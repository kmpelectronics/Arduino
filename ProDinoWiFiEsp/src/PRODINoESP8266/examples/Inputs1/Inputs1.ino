// Inputs1.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Test inputs. When we add power to the some input the example switch on the same relay.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.0.0
// Date: 26.01.2021
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//		Nothing

#include <KMPDinoWiFiESP.h>

void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("Inputs1 example is starting...");

	// Init Dino board.
	KMPDinoWiFiESP.init();
}

void loop()
{
	for (size_t i = 0; i < OPTOIN_COUNT; i++)
	{
		// We read input state KMPDinoWiFiESP.GetOptoInState(i) and set relay with input state.
		KMPDinoWiFiESP.SetRelayState(i, KMPDinoWiFiESP.GetOptoInState(i));
	}
}