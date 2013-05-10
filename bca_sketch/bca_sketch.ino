int beacon_status = 0;
int print_state = 0;

void setup() {
  Serial.begin(57600);

  pinMode(5, INPUT); //BCA Beacon Checker

  Serial.println("Starting up...");
}

void loop() {
  readBeacon();


  if (print_state) {
    print_state = 0;
    if (beacon_status) {
      Serial.println("Beacon present");
    }
    else {
      Serial.println("Beacon absent");
    }
  }

  delay(100);
}

void readBeacon() {
  int BCA = digitalRead(5);

  if (BCA == HIGH && beacon_status == 0) {
    beacon_status = 1;
    print_state = 1;
    return;
  }

  if (BCA == LOW && beacon_status == 1) {
    beacon_status = 0;
    print_state = 1;
    return;
  }
}
