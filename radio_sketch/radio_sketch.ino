#include <SoftwareSerial.h>

#define SLEEP_PIN 4

int value = 0;

void setup() {
  Serial.begin(57600);
}

void loop() {
  delay(1000);

  Serial.println("Hello Monkey");
}