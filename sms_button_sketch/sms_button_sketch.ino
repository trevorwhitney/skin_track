#include <SoftwareSerial.h>

#define SOP '+'
#define EOP '\n'

SoftwareSerial cell(2,3);
char incoming_char = 0;

void setup() {
  digitalWrite(13, HIGH);
  Serial.begin(9600); //Serial for debugging messages
  cell.begin(9600); //Set up modem for communication
  
  pinMode(10, INPUT);  //RED button
  pinMode(11, INPUT);  //BLUE button
  pinMode(12, INPUT);  //WHITE button
  pinMode(13, OUTPUT); //LED for status
  
  Serial.println("Starting SM5100B Communication...");
  
  delay(35000); //Give the GSM modem time to initialize
  
  cell.println("AT+CMGF=1"); // set SMS mode to text
  cell.println("AT+CPMS=\"SM\",\"SM\""); // set message storage to SIM
  cell.println("AT+CMGD=1,4"); // delete all SMS's from SIM card
  
  Serial.println("Ready for testing...");
  digitalWrite(13, LOW);
}

void startSMS(char mobile_number[10]) {
  int quote = 34;
  int cr = 13;
  int nl = 10;
  
  digitalWrite(13, HIGH); // turn on LED for status
  cell.print("AT+CMGS=");
  cell.write((byte)quote); // ASCII equivalent of "
  cell.print(mobile_number);
  cell.write((byte)quote);  // ASCII equivalent of "
  cell.println("");
  delay(500); // give the module some thinking time
}

void endSMS() {
  int endline = 26;
  int cr = 13;
  int nl = 10;
  
  cell.write((byte)endline);  // ASCII equivalent of Ctrl-Z
  delay(15000); // the SMS module needs time to return to OK status
  digitalWrite(13, LOW); // turn off LED when message has been sent
}

void sendSMS(char mobile_number[], char message[]) {
  startSMS(mobile_number);
  cell.print(message);
  endSMS();
}

void getSignalStrength(char signal_strength[10]) {
  clearBuffer();
  
  // query for signal, response should start with CSQ
  cell.println("AT+CSQ");
  delay(1000);
  int data_pos = 0;
  while(cell.available() > 0) {
    incoming_char = cell.read();
    if (incoming_char == 'C') {
      incoming_char = cell.read();
      if (incoming_char == 'S') {
        incoming_char = cell.read();
        if (incoming_char = 'Q') {
          int counter = 0;
          while (counter < 3) {
            incoming_char = cell.read();
            counter++;
          }
          while (incoming_char != '\n') {
            signal_strength[data_pos++] = incoming_char;
            incoming_char = cell.read();
          }
        }
      }
    }
  }
  signal_strength[data_pos] = '\0';
}

//This function doesn't reall work, and I'm not sure why
void listSMS(char message[]) {
  Serial.println("Printing ALL SMS");
  cell.println("AT+CMGL=\"ALL\"");
  message[0] = '\0';
  char buffer = 0;
  int data_pos = 0;
  delay(1000);
  while(1) {
    if(cell.available() >0) {
      incoming_char = cell.read();    //Get the character from the cellular serial port.
      if (incoming_char == 'O') {
        buffer = incoming_char;
        incoming_char = cell.read();
        if (incoming_char = 'K') {
          break;
        }
        else {
          message[data_pos++] = buffer;
          message[data_pos++] = incoming_char;
          message[data_pos] = '\0';
        }
      } else {
        if (data_pos < 1000) {
          Serial.print(incoming_char);
          if (incoming_char = '+') {
            message[data_pos++] = '\r';
            message[data_pos++] = '\n';
            message[data_pos++] = incoming_char;
            message[data_pos] = '\0';
          }
          else {
            message[data_pos++] = incoming_char;  //Print the incoming character to the terminal.
            message[data_pos] = '\0';
          }
        }
        else {
          break;
        }
      }
    }
  }
}

void readSMS(char message[]) {
  cell.println("AT+CMGR=1");
  char data_pos = 0;
  message[0] = '\0';
  while(1) {
    if(cell.available() >0) {
      incoming_char = cell.read();    //Get the character from the cellular serial port.
      if (incoming_char == '\r') {
        incoming_char = cell.read();
        if (incoming_char == '\n') {
          incoming_char = cell.read();
          if (incoming_char == '\r') {
            incoming_char = cell.read();
            if (incoming_char = '\n') {
              break;
            }
          }
        }
      } else {
        if (data_pos < 200) {
          message[data_pos++] = incoming_char;  //Print the incoming character to the terminal.
          message[data_pos] = '\0';
        }
      }
    }
  }
}

void clearBuffer() {
  while (cell.available() > 0) {
    cell.read();
  }
}

void loop() {
  int RED = digitalRead(10);
  int BLUE = digitalRead(11);
  int WHITE = digitalRead(12);
 
  /*Send message on Red and Blue, get signal
    strength on White  */

  if(RED == HIGH) {
    //Serial.println("RED was pressed...Sending txt...");
    //sendSMS("2037676789", "RED was pressed!");
    Serial.println("Red pressed");
    Serial.println("Lets try to get the time from Google");
    char message[200];
    //char all_sms[1000];
    digitalWrite(13, HIGH);
    //listSMS(all_sms);
    sendSMS("466453", "time denver");
    clearBuffer();
    readSMS(message);
    digitalWrite(13, LOW);
    Serial.println(message);
    //Serial.println(all_sms);
  } 
 
  if(BLUE == HIGH) {
    /* Set the time from Google */
    Serial.println("Sending Test SMS");
    sendSMS("2037676789", "BLUE was pressed!");
  } 
 
  //Get signal strength when white is pressed
  if(WHITE == HIGH) {
    Serial.println("WHITE was pressed...Polling signal strength");
    char signal_strength[10];
    getSignalStrength(signal_strength);
    Serial.print("Signal strength is: ");
    Serial.println(signal_strength);
  } 
  
  delay(200); // let everything think for a bit
}
