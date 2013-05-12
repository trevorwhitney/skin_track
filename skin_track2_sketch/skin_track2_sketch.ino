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
void logError(char*);
void sendRecord(SkinTrackRecord);
void processRecord(SkinTrackRecord);
void sleepXBee();
void wakeXBee();

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

//Variables needed to initialize SD card
const uint8_t spiSpeed = SPI_HALF_SPEED;
SdFat sd;
SdFile file;
int chipSelect = 8;
boolean cardInitialized = true;

//Variables for our SD output
char name[] = "access_record.csv";
char contents[10];
char in_char=0;
int index=0;

//variables for XBee
boolean xbee_awake = true;
SoftwareSerial xbee(2,3);

void setup() {
  //Sensors
  pinMode(IR_ENTER_PIN, INPUT); //Channel A IR, we'll call it enter
  pinMode(IR_EXIT_PIN, INPUT); //Channel B IR, we'll call it exit
  pinMode(BCA_PIN, INPUT); //BCA Beacon Checker

  //XBee
  pinMode(XBEE_SLEEP_PIN, OUTPUT);

  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    printMsg("Unable to initialize SD card, data will not be saved.");
    cardInitialized = false;
  }

  //initialize XBee
  xbee.begin(57600);
  wakeXBee();
  delay(500);
  printMsg("Starting up...");
  printMsg("XBee initialized");
  sleepXBee();
}

void loop() {
  delay(100);

  if (state == 0) {
    //check which one gets triggered first
    if (checkGate()) {
      wakeXBee();
    }
  }

  // Channel A was triggered first, so this is an "enter"
  if (state == 1) {
    //enter was triggered first, check for exit
    readIRSensors();

    if (enter_gate_timer >= GATE_TIMER) {
      //timer hit, reset to 9 if items in queue, else to start
      enter_gate_timer = 0;
      process_enter = 0;

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
  if (state == 2) {
    //exit was triggered first, check for enter
    readIRSensors();

    if (exit_gate_timer > GATE_TIMER) {
      //timer hit, reset to 9 if items in queue, else to start
      //FIXME: check for beacon before resetting, if beacon
      //go to state 8
      exit_gate_timer = 0;
      process_exit = 0;

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

    process_enter = 0;
    process_exit = 0;

    //Go to state 7, where beacon is read and an exit is recorded
    state = 7;
    print_state = 1;
    return;
  }

  //Sensors were triggered at the same time.
  if (state == 5) {

    //Go to state 8, where a beacon is read and an unknown direction
    //is recorded
    state = 8;
    print_state = 1;
    return;
  }

  //An entrance has occured, read beacon and save record
  if (state == 6) {
    readBeacon();

    SkinTrackRecord record;
    record.beacon = beacon_status;
    record.direction = GATE_ENTER;
    processRecord(record);
    state = 0;
    print_state = 1;
    return;
  }

  //An exit has occured, read beacon and save record
  if (state == 7) {
    readBeacon();

    SkinTrackRecord record;
    record.beacon = beacon_status;
    record.direction = GATE_EXIT;
    processRecord(record);
    state = 0;
    print_state = 1;
    return;
  }

  //The sensors were triggered in an unknown order, read beacon and save
  //record
  if (state == 8) {
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
    processRecord(record);
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
    logError("Error: SD card not initialized");
    return;
  }

  if (!file.open("data.csv", O_RDWR | O_CREAT | O_AT_END)) {
    logError("Error: Unable to open file on SD card for writing.");
    return;
  }
  //Copy CSV record into contents array
  sprintf(contents, "%d, %d\n", record.direction, record.beacon);
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

void logError(char *msg) {
  if (xbee_awake) {
    delay(200);
    printMsg("Clearing buffer...");
    printMsg(msg);
  }
  else {
    wakeXBee();
    delay(500);
    printMsg("Clearing buffer...");
    printMsg(msg);
    sleepXBee();
  }
}

void sendRecord(SkinTrackRecord record) {
  xbee.print("Clearing buffer...");
  xbee.print("\r\n");
  sprintf(contents, "%d, %d", record.direction, record.beacon);
  xbee.print("Record: ");
  xbee.print(contents);
  xbee.print("\r\n");
}

void processRecord(SkinTrackRecord record) {
  saveToSD(record);
  sendRecord(record);
  sleepXBee();
}

void wakeXBee() {
  digitalWrite(XBEE_SLEEP_PIN, LOW);
  xbee_awake = true;
  //printMsg("XBee awake\r\n");
}

void sleepXBee() {
  //printMsg("XBee going to sleep\r\n");
  digitalWrite(XBEE_SLEEP_PIN, HIGH);
  xbee_awake = false;
}
