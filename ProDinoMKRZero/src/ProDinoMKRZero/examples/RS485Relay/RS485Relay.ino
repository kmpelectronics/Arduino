// RS485Relay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		All KMP ProDino MKR Zero series (https://kmpelectronics.eu/product-category/arduino-mkr-zero/)
// Description:
//		RS485 Relay management example. It works as receive a command and execute it.
//      Commands:
//        FFR - sending current relays statuses
//        FFRxxxx - set relay state. x should be 0 - for Off and 1 - for On. If you send different char x, y, #, 2 and etc. the relay doesn't change its status.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 13.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG


const int CMD_PREFFIX_LEN = 3;
const char CMD_PREFFIX[CMD_PREFFIX_LEN + 1] = "FFR";

const uint8_t BUFF_MAX = 16;

char _dataBuffer[BUFF_MAX];
char _resultBuffer[BUFF_MAX];

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
	Serial.println("The example RS485Relay is started.");
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
		_dataBuffer[buffPos++] = (char)i;
#ifdef DEBUG
		Serial.write((char)i);
#endif
		// Reading a next char.
		i = KMPProDinoMKRZero.RS485Read();
	}

	_dataBuffer[buffPos] = CH_NONE;

#ifdef DEBUG
	Serial.println();
#endif

	// All data has been read. Off status led. 
	KMPProDinoMKRZero.OffStatusLed();

	ProcessData();
}

void ProcessData()
{
	int len = strlen(_dataBuffer);

	// Validate input data.
	if (len < CMD_PREFFIX_LEN || !startsWith(_dataBuffer, CMD_PREFFIX))
	{
#ifdef DEBUG
		Serial.print("Command is not valid.");
#endif

		return;
	}

	// Command set relay status FFRxxxx
	if (len == CMD_PREFFIX_LEN + RELAY_COUNT)
	{
		int relayNum = 0;
		for (int i = CMD_PREFFIX_LEN; i < CMD_PREFFIX_LEN + RELAY_COUNT; i++)
		{
			// Set relay status if only chars are 0 or 1.
			if (_dataBuffer[i] == CH_0 || _dataBuffer[i] == CH_1)
			{
				KMPProDinoMKRZero.SetRelayState(relayNum, _dataBuffer[i] == CH_1);
			}

			++relayNum;
		}
	}

	// Prepare relays statuses.
	strcpy(_resultBuffer, CMD_PREFFIX);
	int relayState = 0;
	for (int j = CMD_PREFFIX_LEN; j < CMD_PREFFIX_LEN + RELAY_COUNT; j++)
	{
		_resultBuffer[j] = KMPProDinoMKRZero.GetRelayState(relayState++) ? CH_1 : CH_0;
	}
	
	_resultBuffer[CMD_PREFFIX_LEN + RELAY_COUNT] = CH_NONE;

#ifdef DEBUG
	Serial.println("Transmiting realys statuses...");
	Serial.println(_resultBuffer);
#endif

	// Transmit result.
	KMPProDinoMKRZero.RS485Write(_resultBuffer);
}