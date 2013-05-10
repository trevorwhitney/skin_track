#include <SoftwareSerial.h>
#define SLEEP_PIN 9

int counter = 0;
int awake = true;

SoftwareSerial xbee(2,3);

void setup() {
  pinMode(SLEEP_PIN, OUTPUT);
  Serial.begin(9600);
  xbee.begin(57600);
  Serial.println("Starting up...");
  digitalWrite(SLEEP_PIN, LOW);
}

void loop() {
  delay(1000);
  if (counter < 5 && awake) {
    //If radio is awake, print the counter
    Serial.print("Awake for ");
    Serial.print(counter);
    Serial.print(" seconds\r\n");
    xbee.print("\r\nHello from XBee!");
    counter += 1;
  }
  else if (counter < 5 && !awake) {
    //If radio is asleep and counter is less than 5,
    //sleep another second
    Serial.print("Asleep for ");
    Serial.print(counter);
    Serial.print(" seconds\r\n");
    counter += 1;
  }
  else if (counter >= 5 && awake) {
    //If we counted to 5, it's time to sleep
    counter = 0;
    Serial.println("Going to sleep");
    xbee.print("\r\nI'm gonna take a nap now, k?");
    digitalWrite(SLEEP_PIN, HIGH);
    awake = false;
  }
  else {
    //If we waited 5 seconds, it's time to wake up
    Serial.println("Waking up");
    digitalWrite(SLEEP_PIN, LOW);
    delay(200);
    xbee.print("\r\nI'm awake already! What do you want?");
    awake = true;
    counter = 0;
  }
}
