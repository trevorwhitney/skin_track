void setup() {
  Serial.begin(9600);

  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  Serial.println("Starting up...");
}

void loop() {
  int RED = digitalRead(4);
  int BLUE = digitalRead(5);
  int WHITE = digitalRead(6);

  if (RED == HIGH) {
    Serial.println("Red was pressed");
  }
  if (BLUE == HIGH) {
    Serial.println("Blue was pressed");
  }
  if (WHITE == HIGH) {
    Serial.println("White was pressed");
  }
  delay(100);
}