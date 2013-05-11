#include <QueueArray.h>
#include <SdFat.h>
#include <SoftwareSerial.h>
#include <SkinTrackRecord.h>
#define IR_ENTER_PIN 7
#define IR_EXIT_PIN 6
#define BCA_PIN 5
#define GATE_ENTER 0
#define GATE_EXIT 1
#define GATE_UNKNOWN -1
#define XBEE_SLEEP_PIN 9

#define GATE_TIMER 30
#define IR_RESET_TIMER 10

void setup();
void loop();
boolean checkGate();
void readIRSensors();
void readEnterSensor();
void readBeacon();
void saveToSD(SkinTrackRecord);
void logState(char*);
void printMsg(char*);
void sendRecord(SkinTrackRecord);

int enter_gate_status = 0; // 0 is off, 1 is triggered
int exit_gate_status = 0; // 0 is off, 1 is triggered
int process_enter = 0;
int process_exit = 0;
int beacon_status = 0;
int process_beacon = 0;
int enter_gate_timer = 0; // 10 = 1 second @ delay of 100
int exit_gate_timer = 0;
int radio_timer = 0;

//for ir timing
int ir_a_timer = 0;
int ir_b_timer = 0;

int state = 0;
int print_state = 1;

QueueArray<SkinTrackRecord> recordQueue;

//Variables needed to initialize SD card
const uint8_t spiSpeed = SPI_HALF_SPEED;
SdFat sd;
SdFile file;
int chipSelect = 8;
int cardInitialized = 1;

//Variables for our SD output
char name[] = "access_record.csv";
char contents[10];
char in_char=0;
int index=0;

//variables for XBee
boolean xbee_awake = true;
SoftwareSerial xbee(2,3);

//TODO
//FIXME: If a beacon is detected it should override the
//IR sensors, and record a finding

//FIXME: case of beacon and no IR?
//need a timer so this isn't preemptively triggered

void setup() {
  //Sensors
  pinMode(IR_ENTER_PIN, INPUT); //Channel A IR, we'll call it enter
  pinMode(IR_EXIT_PIN, INPUT); //Channel B IR, we'll call it exit
  pinMode(BCA_PIN, INPUT); //BCA Beacon Checker

  //XBee
  pinMode(XBEE_SLEEP_PIN, OUTPUT);

  //Serial.begin(9600);

  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    printMsg("Unable to initialize SD card, data will not be saved.");
    cardInitialized = 0;
  }

  //initialize XBee
  xbee.begin(57600);
  digitalWrite(XBEE_SLEEP_PIN, LOW);

  printMsg("Starting up...");
}

void loop() {
  delay(100);
  //xbee.print("\r\nHello from XBee!");

  if (state == 0) {
    //check which one gets triggered first
    logState("Entering starting state...");
    checkGate();
  }

  // Channel A was triggered first, so this is an "enter"
  //FIXME: Add a beacon check, if at the end of the timer there is a
  //beacon present, go to state 8 to record unkown direction
  if (state == 1) {
    //enter was triggered first, check for exit
    readIRSensors();
    logState("State 1: Gate A Triggered (Enter)...");

    char status[256];
    sprintf(status, "Enter status: %d, enter timer: %d, process_enter: %d, process_exit: %d\0",
      enter_gate_status, enter_gate_timer, process_enter, process_exit);
    //printMsg(status);

    if (enter_gate_timer >= GATE_TIMER) {
      //timer hit, reset to 9 if items in queue, else to start
      //FIXME: check for beacon before reset, if beacon
      //go to state 8
      enter_gate_timer = 0;
      process_enter = 0;

      // if (recordQueue.count() > 0) {
      //   state = 9;
      // }
      // else {
      //   state = 0;
      // }

      //if a beacon was captured when the gate was first triggered,
      //they may have never triggered the other gate
      //so record an unknown direction with beacon
      if (process_beacon == 1) {
        state = 8;
        print_state = 1;
        return;
      }

      //if a beacon was not captured when the gate was first triggered,
      //they may have turned it on while in the gate, and they spent too
      //much time in the gate, or only triggered one IR on their exit
      //so get the current status of the BCA and record it
      if (process_beacon == 0) {
        readBeacon();
        if (beacon_status == 1) {
          process_beacon = 1;
          state = 8;
          print_state = 1;
          return;
        }
      }

      state = 0;
      print_state = 1;
      return;
    }

    if (process_exit == 1) {
      enter_gate_timer = 0;
      process_beacon = 0;
      print_state = 1;
      state = 3;
      return;
    }

    if (enter_gate_status == 1) {
      enter_gate_timer = 0;
      return;
    }

    if (process_exit == 0) {
      enter_gate_timer += 1;
      return;
    }
  }

  //Channel B was triggered first, so this is an "exit"
  //FIXME: Add a beacon check, if at the end of the timer there is a
  //beacon present, go to state 8 to record unkown direction
  if (state == 2) {
    //exit was triggered first, check for enter
    readIRSensors();
    logState("State 2: Gate B triggered (Exit)...");

    if (exit_gate_timer > GATE_TIMER) {
      //timer hit, reset to 9 if items in queue, else to start
      //FIXME: check for beacon before resetting, if beacon
      //go to state 8
      exit_gate_timer = 0;
      process_exit = 0;

      // if (recordQueue.count() > 0) {
      //   state = 9;
      // }
      // else {
      //   state = 0;
      // }
      //if a beacon was captured when the gate was first triggered,
      //they may have never triggered the other gate
      //so record an unknown direction with beacon
      if (process_beacon == 1) {
        state = 8;
        print_state = 1;
        return;
      }

      //if a beacon was not captured when the gate was first triggered,
      //they may have turned it on while in the gate, and they spent too
      //much time in the gate, or only triggered one IR on their exit
      //so get the current status of the BCA and record it
      if (process_beacon == 0) {
        readBeacon();
        if (beacon_status == 1) {
          process_beacon = 1;
          state = 8;
          print_state = 1;
          return;
        }
      }

      state = 0;
      print_state = 1;
      return;
    }

    if (process_enter == 1) {
      exit_gate_timer = 0;
      process_beacon = 0;
      state = 4;
      print_state = 1;
      return;
    }

    if (exit_gate_status == 1) {
      exit_gate_timer = 0;
      return;
    }

    if (process_enter == 0) {
      exit_gate_timer += 1;
      return;
    }
  }

  //IR sensors triggered in the order A->B, so enter->exit
  //This is an entrance into the BC
  if (state == 3) {
    logState("State 3: BC Enter detected...");

    process_enter = 0;
    process_exit = 0;

    //Go to state 6, where beacon is read and an entrance is recorded
    state = 6;
    print_state = 1;
    return;
  }

  //IR sensors triggered in the order B->A, so exit->enter
  //This is an exit from the BC
  if (state == 4) {
    logState("State 4: BC exit detected...");

    process_enter = 0;
    process_exit = 0;

    //Go to state 7, where beacon is read and an exit is recorded
    state = 7;
    print_state = 1;
    return;
  }

  //Sensors were triggered at the same time.
  if (state == 5) {
    logState("State 5: IRs triggered at the same time...");

    //Go to state 8, where a beacon is read and an unknown direction
    //is recorded
    state = 8;
    print_state = 1;
    return;
  }

  //An entrance has occured, read beacon and save record
  if (state == 6) {
    logState("State 6: BC Enter recorded, checking beacon...");
    readBeacon();

    SkinTrackRecord record;
    record.beacon = beacon_status;
    record.direction = GATE_ENTER;
    saveToSD(record); //Save to SD
    recordQueue.push(record); //add to radio queue
    state = 0;
    print_state = 1;
    return;
  }

  //An exit has occured, read beacon and save record
  if (state == 7) {
    logState("State 7: BC Exit recorded, checking beacon...");
    readBeacon();

    SkinTrackRecord record;
    record.beacon = beacon_status;
    record.direction = GATE_EXIT;
    saveToSD(record); //Save to SD
    recordQueue.push(record); //add to radio queue
    state = 0;
    print_state = 1;
    return;

  }

  //The sensors were triggered in an unknown order, read beacon and save
  //record
  if (state == 8) {
    logState("State 8: BC direction unknown, checking beacon...");
    if (process_beacon == 1) {
      beacon_status = 1;
      process_beacon = 0;
    }
    else {
      readBeacon();
    }

    SkinTrackRecord record;
    record.beacon = beacon_status;
    record.direction = GATE_UNKNOWN;
    saveToSD(record); //Save to SD
    recordQueue.push(record); //add to radio queue
    state = 0;
    print_state = 1;
    return;

  }

  //Timer state, recording saved and in queue, wait for timeout until
  //starting the radio up so as not to miss people in groups
  if (state == 9) {
    logState("State 9: Records present, starting radio timer");

    if (!checkGate()) {
      // timer waits for a minute before radio communication
      if (radio_timer >= 100) {
        radio_timer = 0;
        state = 10;
        print_state = 1;
        return;
      }
      if (radio_timer < 600) {
        radio_timer += 1;
        return;
      }
    }
    else {
      radio_timer = 0;
    }
  }

  //Begin radio transmission process by turning the radio on
  if (state == 10) {
    logState("State 10: Beginning radio transmission process..");

    if (xbee_awake) {
      //already awake, continue with transmission
      xbee.print("\r\nIm already awake!");
    }
    else {
      //wake it up
      digitalWrite(XBEE_SLEEP_PIN, LOW);
      xbee_awake = true;
      xbee.print("\r\nGood morning!");
    }

    state = 11;
    print_state = 1;
    return;
  }

  //radio is awake, check to make sure there is something to be sent
  //or that the sensor hasn't been triggered
  if (state == 11) {
    logState("State 11: Checking if there's anything to send...");

    if (!checkGate()) {
      if (!recordQueue.isEmpty()) {
        state = 12;
        print_state = 1;
        return;
      }

      //no records in queue, turn radio off and head back to start
      state = 13;
      print_state = 1;
      return;
    }
  }

  //records in accessQueue, send the first one, return to 11
  if (state == 12) {
    logState("State 12: Sending the first record in the queue...");
    sendRecord(recordQueue.pop());

    state = 11;
    print_state = 1;
    return;
  }

  //no records in queue, turn the radio off, and return to start state
  if (state == 13) {
    //temporarily disable sleeping for debugging purposes
    if (xbee_awake && false) {
      digitalWrite(XBEE_SLEEP_PIN, LOW);
      xbee_awake = false;
    }

    state = 0;
    print_state = 1;
    return;
  }
}

boolean checkGate() {
  readIRSensors();

  if (process_enter == 1 && process_exit == 0) {
    state = 1;
    readBeacon();
    if (beacon_status == 1) {
      process_beacon = 1;
    }
    print_state = 1;
    return true;
  }

  if (process_exit == 1 && process_enter == 0) {
    state = 2;
    readBeacon();
    if (beacon_status == 1) {
      process_beacon = 1;
    }
    print_state = 1;
    return true;
  }

  if (process_enter == 1 && process_exit == 1) {
    enter_gate_status = 0;
    exit_gate_status = 0;
    process_enter = 0;
    process_exit = 0;
    state = 5;
    print_state = 1;
    return true;
  }

  return false;
}

void readIRSensors() {
  readEnterSensor();
  readExitSensor();
}

void readEnterSensor() {
  int IR_A = digitalRead(7);

  if (IR_A == LOW && enter_gate_status == 0) {
    if (process_enter == 0) {
      enter_gate_status = 1;
      process_enter = 1;
    }
  }
  else if (IR_A == HIGH && enter_gate_status == 1 &&
      ir_a_timer > IR_RESET_TIMER) {
    enter_gate_status = 0;
    ir_a_timer = 0;
  }
  else if (IR_A == HIGH && enter_gate_status == 1) {
    ir_a_timer += 1;
  }
  else {
    ir_a_timer = 0;
  }
}

void readExitSensor() {
  int IR_B = digitalRead(6);

  if (IR_B == LOW && exit_gate_status == 0) {
    if (process_exit == 0) {
      exit_gate_status = 1;
      process_exit = 1;
    }
  }
  else if (IR_B == HIGH && exit_gate_status == 1 &&
      ir_b_timer > IR_RESET_TIMER) {
    exit_gate_status = 0;
    ir_b_timer = 0;
  }
  else if (IR_B == HIGH && exit_gate_status == 1) {
    ir_b_timer += 1;
  }
  else {
    ir_b_timer = 0;
  }
}

void readBeacon() {
  int BCA = digitalRead(5);

  if (BCA == HIGH && beacon_status == 0) {
    beacon_status = 1;
    return;
  }

  if (BCA == LOW && beacon_status == 1) {
    beacon_status = 0;
    return;
  }
}

void saveToSD(SkinTrackRecord record) {
  if (!cardInitialized) {
    return;
  }

  //Open or create the file 'name' in 'root' for writing to the end of the file.
  if (!file.open("data.csv", O_RDWR | O_CREAT | O_AT_END)) {
    printMsg("Unable to open file on SD card for writing.");
    return;
  }
  //Copy CSV record into contents array
  sprintf(contents, "%d, %d\n", record.direction, record.beacon);
  printMsg("Writing contents: ");
  printMsg(contents);
   //Write the record to the end of the file.
  file.print(contents);
  file.close();

}

void logState(char *msg) {
  if (print_state) {
    //Serial.println(msg);
    if (xbee_awake) {
      xbee.print(msg);
      xbee.print("\r\n");
    }
    print_state = 0;
  }
}

void printMsg(char *msg) {
  //Serial.println(msg);
  if (xbee_awake) {
    xbee.print(msg);
    xbee.print("\r\n");
  }
}

void sendRecord(SkinTrackRecord record) {
  sprintf(contents, "%d, %d", record.direction, record.beacon);
  xbee.print("\r\nRecord: ");
  xbee.print(contents);
}
