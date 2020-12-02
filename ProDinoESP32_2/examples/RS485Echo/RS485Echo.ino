// RS485Echo.ino
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
// Description:
//		RS485 an echo example. It works as received data stored in a buffer and transmit it back.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 06.03.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

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

	Serial.begin(115200);
	Serial.println("The example RS485Echo is starting...");

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

	Serial.println("The example RS485Echo is started");
	delay(1000);
	KMPProDinoESP32.offStatusLed();
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() 
{
	KMPProDinoESP32.processStatusLed(green, 1000);

	// Waiting for a data.
	int i = KMPProDinoESP32.rs485Read();

	if (i == -1)
	{
		return;
	}

	Serial.println("Receiving data...");

	// If in RS485 port has any data - Status led is ON
	KMPProDinoESP32.setStatusLed(yellow);

	uint8_t buffPos = 0;

	// Reading data from the RS485 port.
	while (i > -1 && buffPos < BUFF_MAX)
	{
		// Adding received data in a buffer.
		_dataBuffer[buffPos++] = i;
		Serial.write((char)i);
		// Reading a next char.
		i = KMPProDinoESP32.rs485Read();
	}

	// All data has been read. Off status led. 
	KMPProDinoESP32.offStatusLed();

	Serial.println();
	Serial.println("Transmit received data.");

	KMPProDinoESP32.rs485Write(_dataBuffer, buffPos);
}