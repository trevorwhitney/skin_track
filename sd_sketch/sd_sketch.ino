#include <SdFat.h>

//Variables needed to initialize card
const uint8_t spiSpeed = SPI_HALF_SPEED;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;
int chipSelect = 8;

//Variables for our test output
char name[] = "Test.txt";
char contents[256];
char in_char=0;
int index=0; 

void setup() {
  Serial.begin(9600);
  card.init(spiSpeed, chipSelect);
  volume.init(&card);
  root.openRoot(&volume);
}

void loop() {
  //list contents
  //root.ls(LS_R | LS_DATE | LS_SIZE);
  file.open(root, name, O_CREAT | O_APPEND | O_WRITE);    //Open or create the file 'name' in 'root' for writing to the end of the file.
  sprintf(contents, "Millis: %d    ", millis());    //Copy the letters 'Millis: ' followed by the integer value of the millis() function into the 'contents' array.
  file.print(contents);    //Write the 'contents' array to the end of the file.
  file.close();            //Close the file.
    
  file.open(root, name, O_READ);    //Open the file in read mode.
  in_char=file.read();              //Get the first byte in the file.
  //Keep reading characters from the file until we get an error or reach the end of the file. (This will output the entire contents of the file).
  while(in_char >=0){            //If the value of the character is less than 0 we've reached the end of the file.
    Serial.print(in_char);    //Print the current character
    in_char=file.read();      //Get the next character
  }
  file.close();    //Close the file
  delay(1000);     //Wait 1 second before repeating the process
}
