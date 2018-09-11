// RS485Echo.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		All KMP ProDino MKR Zero series (https://kmpelectronics.eu/product-category/arduino-mkr-zero/)
// Description:
//		RS485 echo example. It works as received data stored in a buffer and transmit it back.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 11.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

const uint8_t BUFF_MAX = 255;

uint8_t _dataBuffer[BUFF_MAX];

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
#ifdef DEBUG
	Serial.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init(ProDino_MKR_Zero);
	// Start RS485 with bound 19200 and 8N1.
	KMPProDinoMKRZero.RS485Begin(19200);

#ifdef DEBUG
	Serial.println("The example RS485Echo is started.");
#endif
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() {
	// Waiting for a data.
	int i = KMPProDinoMKRZero.RS485Read();

	if (i == -1)
	{
		return;
	}

#ifdef DEBUG
	Serial.println("Receiving data...");
#endif

	// If in RS485 port has any data - Status led is ON
	KMPProDinoMKRZero.OnStatusLed();

	uint8_t buffPos = 0;

	// Reading data from the RS485 port.
	while (i > -1 && buffPos < BUFF_MAX)
	{
		// Adding received data in a buffer.
		_dataBuffer[buffPos++] = i;
#ifdef DEBUG
		Serial.write((char)i);
#endif
		// Reading a next char.
		i = KMPProDinoMKRZero.RS485Read();
	}

	// All data has been read. Off status led. 
	KMPProDinoMKRZero.OffStatusLed();

#ifdef DEBUG
	Serial.println();
	Serial.println("Transmit received data.");
#endif

	KMPProDinoMKRZero.RS485Write(_dataBuffer, buffPos);
}