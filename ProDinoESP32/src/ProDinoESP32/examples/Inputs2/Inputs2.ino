// Inputs2.ino
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
//		Test inputs. When we add power to the some input the example changes RGB status led.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 03.12.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//		Nothing

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("Inputs2 example is starting...");
	KMPProDinoESP32.setStatusLed(blue);

	// Init Dino board. Set pins, start W5500.
	// Init Dino board.
	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);

	KMPProDinoESP32.offStatusLed();
}

void loop()
{
	for (size_t i = 0; i < OPTOIN_COUNT; i++)
	{
		// If input is On
		if (KMPProDinoESP32.getOptoInState(i))
		{
			RgbColor color;
			// Get color
			switch (i)
			{
				case 0: color = yellow;
					break;
				case 1: color = red;
					break;
				case 2: color = green;
					break;
				case 3: color = blue;
					break;
				default:
					color = black;
					break;
			}
			
			// Set status led
			KMPProDinoESP32.setStatusLed(color);
		}
	}
}