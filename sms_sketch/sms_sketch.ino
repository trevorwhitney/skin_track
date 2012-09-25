/*  Example 26.3 GSM shield sending a SMS text message
http://tronixstuff.com/tutorials > chapter 26 */
#include <SoftwareSerial.h>

SoftwareSerial cell(2,3);  // We need to create a serial port on D2/D3 to talk to the GSM module
char mobilenumber[] = "2037676789";  // Replace xxxxxxxx with the recipient's mobile number

void setup()
{  //Initialize serial ports for communication.
  Serial.begin(9600);
  cell.begin(9600);
  Serial.println("Starting SM5100B Communication...");
  delay(35000); // give the GSM module time to initialise, locate network etc.
  // this delay time varies. Use example 26.1 sketch to measure the amount
  // of time from board reset to SIND: 4, then add five seconds just in case
}

void loop()
{
  int quote = 34;
  int endline = 26;
  cell.println("AT+CMGF=1"); // set SMS mode to text
  Serial.println("AT+CMGF=1"); // set SMS mode to text
  cell.print("AT+CMGS=");  // now send message...
  cell.write((byte)quote); // ASCII equivalent of "
  cell.print(mobilenumber);
  cell.write((byte)quote);  // ASCII equivalent of "
  cell.println("");
  
  Serial.print("AT+CMGS=");  // now send message...
  Serial.write((byte)quote); // ASCII equivalent of "
  Serial.print(mobilenumber);
  Serial.write((byte)quote);  // ASCII equivalent of "
  Serial.println("");
  
  delay(500); // give the module some thinking time
  cell.print("They call me the count... because I like to count! Ah ha ha ha");   // our message to send
  cell.write((byte)endline);  // ASCII equivalent of Ctrl-Z
  cell.println("");
  
  Serial.print("They call me the count... because I like to count! Ah ha ha ha");   // our message to send
  Serial.write((byte)endline);  // ASCII equivalent of Ctrl-Z
  Serial.println("");
  
  delay(15000); // the SMS module needs time to return to OK status
  do // You don't want to send out multiple SMSs.... or do you?
  {
    delay(1);
  } while (1>0);
}
