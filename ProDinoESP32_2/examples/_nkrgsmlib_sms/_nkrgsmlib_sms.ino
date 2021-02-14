/*
 SMS receiver

 This sketch waits for a SMS message
 and displays it through the Serial port.

 created 1 Dec 2020
 by Dimitar Antonov
*/


#include "KMPProDinoESP32.h"
#include "KMPCommon.h"



#define SECRET_PINNUMBER     ""
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[] = SECRET_PINNUMBER;

// initialize the library instances
GSM gsmAccess(false);
GSM_SMS sms;

// Array to hold the number a SMS is retreived from
char senderNumber[20];
////////////////////////////  | ETH  |  3G  | LoRa |LoRaRFM| LED |
const BoardConfig_t MyBoard = { false, false, false, false ,  1  };

void setup() {
	// initialize serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	Serial.println("SMS Messages Receiver");

	KMPProDinoESP32.begin(MyBoard);

	// connection state
	bool connected = false;

	// Start GSM connection
	while (!connected) {
		if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
			connected = true;
		}
		else {
			Serial.println("Not connected");
			delay(1000);
		}
	}

	KMPProDinoESP32.setStatusLed(blue);

	Serial.println("GSM initialized");
	Serial.println("Waiting for messages");
}

void loop() {
	int c;

	// If there are any SMSs available()
	if (sms.available()) {
		Serial.println("Message received from:");

		// Get remote number
		sms.remoteNumber(senderNumber, 20);
		Serial.println(senderNumber);

		// An example of message disposal
		// Any messages starting with # should be discarded
		if (sms.peek() == '#') {
			Serial.println("Discarded SMS");
			sms.flush();
		}

		// Read message bytes and print them
		while ((c = sms.read()) != -1) {
			Serial.print((char)c);
		}

		Serial.println("\nEND OF MESSAGE");

		// Delete message from modem memory
		sms.flush();
		Serial.println("MESSAGE DELETED");
	}

	delay(1000);

}