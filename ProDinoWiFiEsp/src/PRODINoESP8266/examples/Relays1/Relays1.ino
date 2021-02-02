// Relays1.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Test relays.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.0.0
// Date: 26.01.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//		Nothing

#include <KMPDinoWiFiESP.h>

void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("Relay 1 example is starting...");

	// Init Dino board.
	KMPDinoWiFiESP.init();
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
	KMPDinoWiFiESP.SetAllRelaysOff();
	delay(1000);

	// Switch relay 1 On
	KMPDinoWiFiESP.SetRelayState(0, true);
	delay(1000);

	// Switch relay 2 On
	KMPDinoWiFiESP.SetRelayState(1, true);
	delay(1000);

	// Switch relay 3 On
	KMPDinoWiFiESP.SetRelayState(2, true);
	delay(1000);

	// Switch relay 4 On
	KMPDinoWiFiESP.SetRelayState(3, true);
	delay(1000);

	// Switch relay 1 Off
	KMPDinoWiFiESP.SetRelayState(0, false);
	delay(1000);

	// Switch relay 2 Off
	KMPDinoWiFiESP.SetRelayState(1, false);
	delay(1000);

	// Switch relay 3 Off
	KMPDinoWiFiESP.SetRelayState(2, false);
	delay(1000);

	// Switch relay 4 Off
	KMPDinoWiFiESP.SetRelayState(3, false);
	delay(1000);
}

void swichRelay2()
{
	// Switch every relay to On
	for (size_t i = 0; i < RELAY_COUNT; i++)
	{
		KMPDinoWiFiESP.SetRelayState(i, true);
		delay(1000);
	}

	// Switch every relay to Off
	for (size_t i = 0; i < RELAY_COUNT; i++)
	{
		KMPDinoWiFiESP.SetRelayState(i, false);
		delay(1000);
	}
}

void swichRelay3()
{
	// Switch on all relays
	KMPDinoWiFiESP.SetAllRelaysOn();
	delay(1000);

	// Switch off all relays
	KMPDinoWiFiESP.SetAllRelaysOff();
	delay(1000);
}

void swichRelay4()
{
	// Switch On all relays
	KMPDinoWiFiESP.SetAllRelaysState(true);
	delay(1000);

	// Switch Off all relays
	KMPDinoWiFiESP.SetAllRelaysState(false);
	delay(1000);
}

void swichRelay5()
{
	// Switch relay 1 On
	KMPDinoWiFiESP.SetRelayState(Relay1, true);
	delay(1000);

	// Switch relay 2 On
	KMPDinoWiFiESP.SetRelayState(Relay2, true);
	delay(1000);

	// Switch relay 3 On
	KMPDinoWiFiESP.SetRelayState(Relay3, true);
	delay(1000);

	// Switch relay 4 On
	KMPDinoWiFiESP.SetRelayState(Relay4, true);
	delay(1000);

	// Switch relay 1 Off
	KMPDinoWiFiESP.SetRelayState(Relay1, false);
	delay(1000);

	// Switch relay 2 Off
	KMPDinoWiFiESP.SetRelayState(Relay2, false);
	delay(1000);

	// Switch relay 3 Off
	KMPDinoWiFiESP.SetRelayState(Relay3, false);
	delay(1000);

	// Switch relay 4 Off
	KMPDinoWiFiESP.SetRelayState(Relay4, false);
	delay(1000);
}

void swichRelay6()
{
	// Switch relay 1 and 3 On
	KMPDinoWiFiESP.SetRelayState(Relay1, true);
	KMPDinoWiFiESP.SetRelayState(Relay3, true);
	delay(1000);

	// Switch relay 1 and 3 Off
	KMPDinoWiFiESP.SetRelayState(Relay1, false);
	KMPDinoWiFiESP.SetRelayState(Relay3, false);
	delay(1000);

	// Switch relay 2 and 4 On
	KMPDinoWiFiESP.SetRelayState(Relay2, true);
	KMPDinoWiFiESP.SetRelayState(Relay4, true);
	delay(1000);

	// Switch relay 2 and 4 Off
	KMPDinoWiFiESP.SetRelayState(Relay2, false);
	KMPDinoWiFiESP.SetRelayState(Relay4, false);
	delay(1000);
}