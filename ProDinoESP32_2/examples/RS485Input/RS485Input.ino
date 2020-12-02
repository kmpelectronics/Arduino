// RS485Input.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		ProDino ESP32 V1 https://kmpelectronics.eu/products/prodino-esp32-v1/
//		ProDino ESP32 Ethernet V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/
//		ProDino ESP32 GSM V1 https://kmpelectronics.eu/products/prodino-esp32-gsm-v1/
//		ProDino ESP32 LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-lora-v1/
//		ProDino ESP32 LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-lora-rfm-v1/
//		ProDino ESP32 Ethernet GSM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
//		ProDino ESP32 Ethernet LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-v1/
//		ProDino ESP32 Ethernet LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-rfm-v1/
//
// Description:
//		RS485 reads isolated input example. It works as receive a command and execute it.
//      Command:
//        FFI - sending current inputs statuses
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 16.03.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

const int CMD_PREFFIX_LEN = 3;
const char CMD_PREFFIX[CMD_PREFFIX_LEN + 1] = "FFI";

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
	Serial.begin(115200);
	Serial.println("The example RS485Input is starting...");

	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
	KMPProDinoESP32.setStatusLed(blue);

	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoESP32.rs485Begin(19200);

	Serial.println("The example RS485Input is started.");
	delay(1000);

	KMPProDinoESP32.offStatusLed();
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() {
	KMPProDinoESP32.processStatusLed(green, 1000);
	// Waiting for a data.
	int i = KMPProDinoESP32.rs485Read();

	if (i == -1)
	{
		return;
	}

	Serial.println("Receiving data...");

	// If in RS485 port has any data - Status led is in Yellow
	KMPProDinoESP32.setStatusLed(yellow);

	uint8_t buffPos = 0;

	// Reading data from the RS485 port.
	while (i > -1 && buffPos < BUFF_MAX)
	{
		// Adding received data in a buffer.
		_dataBuffer[buffPos++] = (char)i;
		Serial.write((char)i);
		// Reading a next char.
		i = KMPProDinoESP32.rs485Read();
	}

	_dataBuffer[buffPos] = CH_NONE;
	Serial.println();
	ProcessData();

	KMPProDinoESP32.offStatusLed();
}

void ProcessData()
{
	int len = strlen(_dataBuffer);

	// Validate input data.
	if (len < CMD_PREFFIX_LEN || !startsWith(_dataBuffer, CMD_PREFFIX))
	{
		Serial.print("Command is not valid.");
		return;
	}

	// Prepare Inputs statuses.
	strcpy(_resultBuffer, CMD_PREFFIX);
	int inputState = 0;
	for (int j = CMD_PREFFIX_LEN; j < CMD_PREFFIX_LEN + OPTOIN_COUNT; j++)
	{
		_resultBuffer[j] = KMPProDinoESP32.getOptoInState(inputState++) ? CH_1 : CH_0;
	}
	
	_resultBuffer[CMD_PREFFIX_LEN + OPTOIN_COUNT] = CH_NONE;
	Serial.println("Transmiting isolated inputs statuses...");
	Serial.println(_resultBuffer);

	// Transmit result.
	KMPProDinoESP32.rs485Write(_resultBuffer);
}