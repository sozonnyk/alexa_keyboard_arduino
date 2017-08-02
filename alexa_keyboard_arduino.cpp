#include "alexa_keyboard_arduino.h"
#include <Keyboard.h>
#include <SoftwareSerial.h>
#include "Arduino.h"

// Change USBCore.cpp SendDescriptor(USBSetup& setup), line 531
//			char name[ISERIAL_MAX_LEN] = "esp-kbd";
//			//PluggableUSB().getShortName(name);

#define DEBUG
#ifdef DEBUG
#define DEBUG_SPEED 115200
#define D(input) {Serial1.print(input); Serial1.flush();}
#define Dln(input) {Serial1.println(input); Serial1.flush();}
#else
#define D(input)
#define Dln(input)
#endif

#define SOFT_RX 8
#define SOFT_TX 9
#define SOFT_SPEED 57600

#define INPUT_BLOCK_SEPAR ";"
#define INPUT_COMBINE_SEPAR "+"
#define INPUT_STR_BASE 16

#define TYPE_DELAY 25

#define CR 0x0A
#define LF 0x0D

SoftwareSerial serial(SOFT_RX, SOFT_TX);
String data;

/*
 * Device listens on soft serial port open for 57600
 * Data format: <byte>+<byte>;<byte><cr or lf>
 * byte - is hexadecimal character code
 * ; separates entries, + presses several keys together
 */
void setup() {
#ifdef DEBUG
	Serial1.begin(DEBUG_SPEED);
#endif
	serial.begin(SOFT_SPEED);
	Keyboard.begin();
	Dln("Start");
}

void print_str(char *str) {
	Dln(data);
}

void process_string() {
	D("Received: ");
	Dln(data);

	char *str = data.c_str();
	char *token;
	char *rest = str;

	while ((token = strtok_r(rest, INPUT_BLOCK_SEPAR, &rest))) {
		D("Block: ");
		Dln(token);

		char *str2 = token;
		char *token2;
		char *rest2 = str2;
		bool pressed = false;

		while ((token2 = strtok_r(rest2, INPUT_COMBINE_SEPAR, &rest2))) {
			int c = strtol(token2, NULL, INPUT_STR_BASE);
			if (c >= 0x20 && c <= 0xDA) {
				D("Press: ");
				Dln(c);
				Keyboard.press(c);
				pressed = true;
			} else {
				Dln("Garbage input");
			}
		}
		if (pressed) {
			Dln("Wait 25");
			delay(TYPE_DELAY);
			Dln("Release");
			Keyboard.releaseAll();
		}
	}

	data = "";
}

void loop() {
	if (serial.available() > 0) {
		char character = serial.read();
		if (character == CR || character == LF) {
			process_string();
		} else {
			data += character;
		}
	}
}
