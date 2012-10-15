#include <SoftwareSerial.h>
#include <SdFat.h>

#define SLEEP_PIN 9

//Variables for radio
int counter = 0;
int awake = true;
SoftwareSerial xbee(2,3);


//Variables for SD Card
const uint8_t spiSpeed = SPI_HALF_SPEED;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile log_file;
SdFile csv_file;

int chipSelect = 8;
char log_name[] = "log.txt";
char csv_name[] = "data.csv";
char contents[256];
char in_char=0;
int index=0;

//Function prototypes
void setup(void);
void loop(void);
char* printDigits(int);
void wake_radio(void);
void sleep_radio(void);


void setup() {
  //initialize SD card
  card.init(spiSpeed, chipSelect);
  volume.init(&card);
  root.openRoot(&volume);

  //initialize radio
  pinMode(SLEEP_PIN, OUTPUT);
  xbee.begin(57600);
  //wake_radio();

  //Serial for debugging
  Serial.begin(9600);
}

void loop() {
  delay(1000);
  if (counter < 5 && awake) {
    //If radio is awake, print the counter
    xbee.print("\r\nHello from XBee! Counter is: ");
    xbee.print(counter);
    csv_file.open(root, csv_name, O_CREAT | O_APPEND | O_WRITE);
    sprintf(contents, "Counter: %d, ", counter);
    csv_file.print(contents);
    csv_file.close();
    counter += 1;
  }
  else if (counter < 5 && !awake) {
    //If radio is asleep and counter is less than 5, 
    //sleep another second
    counter += 1;
  }
  else if (counter >= 5 && awake) {
    //If we counted to 5, it's time to sleep
    counter = 0;
    xbee.print("\r\nI'm gonna take a nap now, k?");

    csv_file.open(root, csv_name, O_CREAT | O_APPEND | O_WRITE);
    csv_file.print("\r\n");
    csv_file.close();

    sleep_radio();
    awake = false;
  }
  else {
    //If we waited 5 seconds, it's time to wake up
    wake_radio();

    xbee.print("\r\nI'm awake already! What do you want?");
    awake = true;
    counter = 0;
  }
}

char* printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  char digit_string[3];
  digit_string[0] = ':';
  if(digits < 10) {
    digit_string[1] = '0';
  }
  else {
    digit_string[1] = digits / 10;
  }
  digit_string[2] = digits % 10;
  return digit_string;
}

void sleep_radio() {
  digitalWrite(SLEEP_PIN, HIGH);
  log_file.open(root, log_name, O_CREAT | O_APPEND | O_WRITE);
  sprintf(contents, "%d - Putting XBee to sleep\r\n", millis());
  log_file.print(contents);
  log_file.close();
}

void wake_radio() {
  digitalWrite(SLEEP_PIN, LOW);
  log_file.open(root, log_name, O_CREAT | O_APPEND | O_WRITE);
  sprintf(contents, "%d - Waking XBee Up\r\n", millis());
  log_file.print(contents);
  log_file.close();
  delay(100);
}