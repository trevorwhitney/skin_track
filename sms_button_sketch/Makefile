ARDUINO_DIR  = /usr/share/arduino

TARGET       = GSM
ARDUINO_LIBS = SoftwareSerial

BOARD_TAG    = uno
ARDUINO_PORT = /dev/ttyACM0

include /usr/share/arduino/Arduino.mk

monitor: upload
	stty -F $(ARDUINO_PORT) cs8 9600 ignbrk \
	-brkint -icrnl -imaxbel -opost -onlcr \
	-isig -icanon -iexten -echo -echoe -echok \
	-echoctl -echoke noflsh -ixon -crtscts \
	&& screen $(ARDUINO_PORT) 9600
