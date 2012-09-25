#include <SoftwareSerial.h>

#define SOP '+'
#define EOP '\n'

SoftwareSerial cell(2,3);
char incoming_char = 0;

void setup()
{
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
  //cell.println("AT+CMGD=1,4"); // delete all SMS's from SIM card
  
  Serial.println("Ready for testing...");
  digitalWrite(13, LOW);
}

void startSMS(char mobile_number[10]) // function to start a text message transmission
{
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

void endSMS() // function to end a text message transmission
{
  int endline = 26;
  int cr = 13;
  int nl = 10;
  
  cell.write((byte)endline);  // ASCII equivalent of Ctrl-Z
  delay(15000); // the SMS module needs time to return to OK status
  digitalWrite(13, LOW); // turn off LED when message has been sent
}

void sendSMS(char mobile_number[], char message[])
{
  startSMS(mobile_number);
  cell.print(message);
  endSMS();
}

void getSignalStrength(char signal_strength[10])
{
  // clear the cell's incoming buffer
  while (cell.available() > 0) {
    cell.read();
  }
  
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

void readSMS(char message[]) {

  /*bool started = false;
  bool ended = false;

  char inData[200];
  byte index;
  
  cell.println("AT+CMGR=1");
  delay(1000);
  while(cell.available() > 0)
  {
    incoming_char = cell.read();
    cell.println(incoming_char);
    if(incoming_char == SOP)
    {
       index = 0;
       inData[index] = '\0';
       started = true;
       ended = false;
    }
    else if(incoming_char == EOP)
    {
       ended = true;
       break;
    }
    else
    {
      if(index < 200)
      {
        inData[index++] = incoming_char;
        inData[index] = '\0';
      }
    }
  }

  // We are here either because all pending serial
  // data has been read OR because an end of
  // packet marker arrived. Which is it?
  if(started && ended)
  {
    // The end of packet marker arrived. Process the packet

    // Reset for the next packet
    started = false;
    ended = false;
    index = 0;
    message = inData;
    inData[index] = '\0';
  }*/
  cell.println("AT+CMGR=1");
  char data_pos = 0;
  message[0] = '\0';
  while(1) {
    if(cell.available() >0)
    {
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

void loop()
{
  int RED = digitalRead(10);
  int BLUE = digitalRead(11);
  int WHITE = digitalRead(12);
 
  /*Send message on Red and Blue, get signal
    strength on White  */

  if(RED == HIGH) {
    //Serial.println("RED was pressed...Sending txt...");
    //sendSMS("2037676789", "RED was pressed!");
    Serial.println("Red pressed");
    char message[200];
    digitalWrite(13, HIGH);
    readSMS(message);
    digitalWrite(13, LOW);
    Serial.println(message);
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
