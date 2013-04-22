#include <aJSON.h>

char json_string[255];
char example_string[255];

void setup() {
  strcpy(json_string, 
    "{ \"time\": 1234, \"direction\": 0, \"beacon\": 1 }");
  strcpy(example_string,
    "{ \"name\": \"Ted (\\\"Monkey\\\") Jones\" }");
  Serial.begin(9600);
}

void loop() {
  Serial.print("Original string: ");
  Serial.println(json_string);

  aJsonObject* root = aJson.parse(json_string);
  aJsonObject* time_string = aJson.getObjectItem(root, "time");
  
  Serial.print("Time is: ");
  Serial.println(time_string->valueint);

  aJsonObject* example = aJson.parse(example_string);
  aJsonObject* name = aJson.getObjectItem(example, "name");

  Serial.print("Example string: ");
  Serial.println(example_string);
  Serial.print("Example name: ");
  Serial.println(name->valuestring);

  aJsonObject* dir = aJson.getObjectItem(root, "direction");
  int direction = dir->valueint;
  if (direction == 0) {
    Serial.println("He went north");
  }
  else {
    Serial.println("He went south");
  }

  aJson.deleteItem(root);
  aJson.deleteItem(example);

  //Let's create an object now
  aJsonObject *fmt;
  root = aJson.createObject();

  aJson.addNumberToObject(root, "time", 4321);
  aJson.addNumberToObject(root, "direction", 1);
  aJson.addNumberToObject(root, "beacon", 0);

  char *json_string2 = aJson.print(root);
  Serial.print("New object created: \r\n\t");
  Serial.println(json_string2);

  aJsonObject* time_value = aJson.getObjectItem(root, "time");
  aJsonObject* direction_value = aJson.getObjectItem(root, "direction");
  aJsonObject* beacon_value = aJson.getObjectItem(root, "beacon");

  Serial.print("New object time: ");
  Serial.println(time_value->valueint);
  Serial.print("New object direction: ");
  Serial.println(direction_value->valueint);
  Serial.print("New object beacon: ");
  Serial.println(beacon_value->valueint);
  
  aJson.deleteItem(root);

  delay(2000);
}