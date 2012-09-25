#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial cell(2,3);
char mobilenumber[] = "2037676789";

void setup()
{
  Serial.begin(9600); //Serial for debugging messages
  cell.begin(9600); //Set up modem for communication
  
  pinMode(10, INPUT);  //RED button
  pinMode(11, INPUT);  //BLUE button
  pinMode(12, INPUT);  //WHITE button
  pinMode(13, OUTPUT); //LED for status
  
  Serial.println("Starting SM5100B Communication...");
  
  delay(35000); //Give the GSM modem time to initialize
}

void startSMS() // function to start a text message transmission
{
  int quote = 34;
  
  digitalWrite(13, HIGH); // turn on LED for status
  cell.println("AT+CMGF=1"); // set SMS mode to text
  cell.print("AT+CMGS=");
  cell.write((byte)quote); // ASCII equivalent of "
  cell.print(mobilenumber);
  cell.write((byte)quote);  // ASCII equivalent of "
  cell.println("");
  delay(500); // give the module some thinking time
}

void endSMS() // function to end a text message transmission
{
  int endline = 26;
  
  cell.write((byte)endline);  // ASCII equivalent of Ctrl-Z
  cell.println("");
  delay(15000); // the SMS module needs time to return to OK status
  digitalWrite(13, LOW); // turn off LED when message has been sent
}

void loop()
{
  int RED = digitalRead(10);
  int BLUE = digitalRead(11);
  int WHITE = digitalRead(12);
 
  if(RED == HIGH){
    Serial.println("RED was pressed...Sending txt...");
    startSMS();
    cell.print("RED was pressed!");
    endSMS();
  } 
 
  if(BLUE == HIGH){
    Serial.println("BLUE was pressed...Sending txt...");
    startSMS();
    cell.print("BLUE was pressed!");
    endSMS();
  } 
 
  if(WHITE == HIGH){
    Serial.println("WHITE was pressed...Sending txt...");
    startSMS();
    cell.print("WHITE was pressed!");
    endSMS();
  } 
  
  delay(500); // let everything think for a bit
}
