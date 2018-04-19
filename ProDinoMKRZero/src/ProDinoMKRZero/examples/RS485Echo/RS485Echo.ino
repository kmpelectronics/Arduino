// EthWebRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino MKR Zero (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		RS485 echo example. It works as buffers all received data and transmit it back.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebdhtserver.aspx
// Version: 1.0.0
// Date: 19.04.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

const uint8_t BUFF_MAX = 255;

uint8_t _dataBuffer[BUFF_MAX];

/**
* \brief Setup void. Arduino executed first. Initialize DiNo board and prepare Ethernet connection.
*
*
* \return void
*/
void setup()
{
	delay(5000);
#ifdef DEBUG
	SerialUSB.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init(false);
	// Start RS485 with bound 19200 and 8N1.
	KMPProDinoMKRZero.RS485Begin(19200);

#ifdef DEBUG
	SerialUSB.println("The server is starting.");
#endif
}

/**
* \brief Loop void. Arduino executed second.
*
*
* \return void
*/
void loop() {
	int i = KMPProDinoMKRZero.RS485Read();

	// if i = -1 not data to read.
	if (i == -1)
	{
		return;
	}

	// If data send - On status led.
	//OnStatusLed();

	uint8_t buffPos = 0;

	while (i > -1 && buffPos < BUFF_MAX)
	{
		_dataBuffer[buffPos++] = i;
#ifdef DEBUG
		Serial.write((char)i);
#endif
		// Read next char.
		i = KMPProDinoMKRZero.RS485Read();
	}

	uint8_t sendPos = 0;
	while (sendPos < buffPos)
	{
		KMPProDinoMKRZero.RS485Write(_dataBuffer[sendPos++]);
	}

	// Off status led.
	//OffStatusLed();
}