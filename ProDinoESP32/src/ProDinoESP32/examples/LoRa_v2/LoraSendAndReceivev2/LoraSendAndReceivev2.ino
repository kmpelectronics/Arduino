/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN_v2.h>
#include <HardwareSerial.h>

// LoRa module pins for Serial2
#define	J14_5	35 // I35
#define	J14_6	27 // IO27
#define	J14_7	34 // I34
#define	J14_8	25 // IO25
#define	J14_9	26 // IO26
#define	J14_10	14 // IO14
#define	J14_11	13 // IO13
#define	J14_12	15 // IO15

#define LoRaRxPin    J14_5
#define LoRaLowPin   J14_6
#define LoRaTxPin    J14_8
#define LoRaBootPin  J14_11
#define LoRaResetPin J14_12
HardwareSerial SerialModem(2);

LoRaModem modem(SerialModem);

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);

	pinMode(LoRaLowPin, INPUT_PULLUP);

	pinMode(LoRaBootPin, OUTPUT);
	digitalWrite(LoRaBootPin, LOW);

	pinMode(LoRaResetPin, OUTPUT);
	digitalWrite(LoRaResetPin, HIGH);
	delay(200);
	digitalWrite(LoRaResetPin, LOW);
	delay(200);
	digitalWrite(LoRaResetPin, HIGH);
	delay(200);

	SerialModem.begin(9600, SERIAL_8N1, LoRaRxPin, LoRaTxPin);

	// change this to your regional band (eg. US915, AS923, ...)
	if (!modem.begin(EU868)) {
		Serial.println("Failed to start module");
		while (1) {}
	};
	Serial.print("Your module version is: ");
	Serial.println(modem.version());
	Serial.print("Your device EUI is: ");
	Serial.println(modem.deviceEUI());

	int connected = modem.joinOTAA(appEui, appKey);
	if (!connected) {
		Serial.println("Something went wrong; are you indoor? Move near a window and retry");
		while (1) {}
	}

	// Set poll interval to 60 secs.
	modem.minPollInterval(60);
	// NOTE: independently by this setting the modem will
	// not allow to send more than one message every 2 minutes,
	// this is enforced by firmware and can not be changed.
}

void loop() {
	Serial.println();
	Serial.println("Enter a message to send to network");
	Serial.println("(make sure that end-of-line 'NL' is enabled)");

	while (!Serial.available());
	String msg = Serial.readStringUntil('\n');

	Serial.println();
	Serial.print("Sending: " + msg + " - ");
	for (unsigned int i = 0; i < msg.length(); i++) {
		Serial.print(msg[i] >> 4, HEX);
		Serial.print(msg[i] & 0xF, HEX);
		Serial.print(" ");
	}
	Serial.println();

	int err;
	modem.beginPacket();
	modem.print(msg);
	err = modem.endPacket(true);
	if (err > 0) {
		Serial.println("Message sent correctly!");
	}
	else {
		Serial.println("Error sending message :(");
		Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
		Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
	}
	delay(1000);
	if (!modem.available()) {
		Serial.println("No downlink message received at this time.");
		return;
	}
	char rcv[64];
	int i = 0;
	while (modem.available()) {
		rcv[i++] = (char)modem.read();
	}
	Serial.print("Received: ");
	for (unsigned int j = 0; j < i; j++) {
		Serial.print(rcv[j] >> 4, HEX);
		Serial.print(rcv[j] & 0xF, HEX);
		Serial.print(" ");
	}
	Serial.println();
}
