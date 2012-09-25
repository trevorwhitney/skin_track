#!/bin/sh

ARDUINO_PORT="/dev/ttyACM0"

stty -F $ARDUINO_PORT cs8 9600 ignbrk \
	-brkint -icrnl -imaxbel -opost -onlcr \
	-isig -icanon -iexten -echo -echoe -echok \
	-echoctl -echoke noflsh -ixon -crtscts \
	&& screen $ARDUINO_PORT 9600
