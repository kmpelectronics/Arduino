// Relays1.ino
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
//		Test relays.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 01.12.2020
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
	Serial.println("Relay 1 example is starting...");
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
	swichRelay1();
	
	swichRelay2();

	swichRelay3();

	swichRelay4();

	swichRelay5();

	swichRelay6();
}

void swichRelay1()
{
	// Switch off all relays
	KMPProDinoESP32.setAllRelaysOff();
	delay(1000);

	// Switch relay 1 On
	KMPProDinoESP32.setRelayState(0, true);
	delay(1000);

	// Switch relay 2 On
	KMPProDinoESP32.setRelayState(1, true);
	delay(1000);

	// Switch relay 3 On
	KMPProDinoESP32.setRelayState(2, true);
	delay(1000);

	// Switch relay 4 On
	KMPProDinoESP32.setRelayState(3, true);
	delay(1000);

	// Switch relay 1 Off
	KMPProDinoESP32.setRelayState(0, false);
	delay(1000);

	// Switch relay 2 Off
	KMPProDinoESP32.setRelayState(1, false);
	delay(1000);

	// Switch relay 3 Off
	KMPProDinoESP32.setRelayState(2, false);
	delay(1000);

	// Switch relay 4 Off
	KMPProDinoESP32.setRelayState(3, false);
	delay(1000);
}

void swichRelay2()
{
	// Switch every relay to On
	for (size_t i = 0; i < RELAY_COUNT; i++)
	{
		KMPProDinoESP32.setRelayState(i, true);
		delay(1000);
	}

	// Switch every relay to Off
	for (size_t i = 0; i < RELAY_COUNT; i++)
	{
		KMPProDinoESP32.setRelayState(i, false);
		delay(1000);
	}
}

void swichRelay3()
{
	// Switch on all relays
	KMPProDinoESP32.setAllRelaysOn();
	delay(1000);

	// Switch off all relays
	KMPProDinoESP32.setAllRelaysOff();
	delay(1000);
}

void swichRelay4()
{
	// Switch On all relays
	KMPProDinoESP32.setAllRelaysState(true);
	delay(1000);

	// Switch Off all relays
	KMPProDinoESP32.setAllRelaysState(false);
	delay(1000);
}

void swichRelay5()
{
	// Switch relay 1 On
	KMPProDinoESP32.setRelayState(Relay1, true);
	delay(1000);

	// Switch relay 2 On
	KMPProDinoESP32.setRelayState(Relay2, true);
	delay(1000);

	// Switch relay 3 On
	KMPProDinoESP32.setRelayState(Relay3, true);
	delay(1000);

	// Switch relay 4 On
	KMPProDinoESP32.setRelayState(Relay4, true);
	delay(1000);

	// Switch relay 1 Off
	KMPProDinoESP32.setRelayState(Relay1, false);
	delay(1000);

	// Switch relay 2 Off
	KMPProDinoESP32.setRelayState(Relay2, false);
	delay(1000);

	// Switch relay 3 Off
	KMPProDinoESP32.setRelayState(Relay3, false);
	delay(1000);

	// Switch relay 4 Off
	KMPProDinoESP32.setRelayState(Relay4, false);
	delay(1000);
}

void swichRelay6()
{
	// Switch relay 1 and 3 On
	KMPProDinoESP32.setRelayState(Relay1, true);
	KMPProDinoESP32.setRelayState(Relay3, true);
	delay(1000);

	// Switch relay 1 and 3 Off
	KMPProDinoESP32.setRelayState(Relay1, false);
	KMPProDinoESP32.setRelayState(Relay3, false);
	delay(1000);

	// Switch relay 2 and 4 On
	KMPProDinoESP32.setRelayState(Relay2, true);
	KMPProDinoESP32.setRelayState(Relay4, true);
	delay(1000);

	// Switch relay 2 and 4 Off
	KMPProDinoESP32.setRelayState(Relay2, false);
	KMPProDinoESP32.setRelayState(Relay4, false);
	delay(1000);
}