#include <Arduino.h>
#include <SoftwareSerial.h>

#define SLEEP_PIN 2

int value = 0;
SoftwareSerial xbee(2,3);

void setup() {
  pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(SLEEP_PIN, HIGH);
  Serial.println("Setting sleep OFF");
  Serial.begin(9600);
  xbee.begin(57600);
}

void loop() {
  delay(1000);
  //Serial.print(value);
  //Serial.print(",");
  //value += 2;

  Serial.println("Hello Monkey");
  xbee.println("Hello World");
}