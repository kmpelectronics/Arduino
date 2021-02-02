// WiFiBlynk.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Blynk example. For this example need add in Blynk mobile application 4 button (200), 1 LCD Display(400). All 1200 points.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.1.0
// Date: 27.01.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//  You have to fill your credentials in arduino_secrets.h file
//	Before start this example you need to install:
//		Install librarys: Sketch\Include library\Menage Libraries...
//         - Blynk
//         - SimpleDHT by Winlin
//		Connect DHT22 sensor to GROVE connector. Use pins: 
//			- sensor EXT_GROVE_D0, Vcc+, Gnd(-);
// --------------------------------------------------------------------------------
//  Pin maps PRODINo WiFi-ESP -> Blynk:
//		Relay1 -> V1 {Type: "Button", Name: "Relay 1", Color: "Green", Output: "V1", Mode: "Switch" }
//		Relay2 -> V2 {Type: "Button", Name: "Relay 2", Color: "Blue", Output: "V2", Mode: "Switch" }
//		Relay3 -> V3 {Type: "Button", Name: "Relay 3", Color: "LightBlue", Output: "V3", Mode: "Switch" }
//		Relay4 -> V4 {Type: "Button", Name: "Relay 4", Color: "Orange", Output: "V4", Mode: "Switch" }
//		OptoIn1 -> V5 {Type: "LED", Name: "In 1", Color: "Green", Input: "V5" }
//		OptoIn2 -> V6 {Type: "LED", Name: "In 2", Color: "Blue", Input: "V6" }
//		OptoIn3 -> V7 {Type: "LED", Name: "In 3", Color: "LightBlue", Input: "V7" }
//		OptoIn4 -> V8 {Type: "LED", Name: "In 4", Color: "Orange", Input: "V8" }
//		DHT1T -> V9 {Type: "Value Display", Name: "Temperature", Color: "Green", Input: "V9", Min:"-40", Max:"80", ReadingFrecuency: "5sec" }
//		DHT1H -> V10 {Type: "Value Display", Name: "Humidity", Color: "Green", Input: "V10", Min:"0", Max:"100", ReadingFrecuency: "5sec" }

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SimpleDHT.h>
#include "arduino_secrets.h"

#define BLYNK_DEBUG
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#include <BlynkSimpleEsp8266.h>

SimpleDHT22 _dht(EXT_GROVE_D0);

// Check sensor data, interval in milliseconds.
const long CHECK_INTERVAL_MS = 5000;
// Store last measure time.
unsigned long _mesureTimeout = 0;

/**
 * @brief Execute first after start device. Initialize hardware.
 *
 * @return void
 */
void setup(void)
{
	// You can open the Arduino IDE Serial Monitor window to see what the code is doing
	// Serial connection from ESP-01 via 3.3v console cable
	Serial.begin(115200);

	// Init KMP ProDino WiFi-ESP board.
	KMPDinoWiFiESP.init();

	Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD);
}

/**
 * @brief Main method.
 *
 * @return void
 */
void loop(void)
{
	// Process data every CHECK_INTERVAL_MS.
	if (millis() > _mesureTimeout)
	{
		ProcessDHTSensors();
		ProcessOptoInputs();

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_INTERVAL_MS;
	}

	Blynk.run();
}

/**
 * @brief Read data from DHT sensors a specified time and print new data to LCD display Blynk mobile application.
 *
 * @return void
 */
void ProcessDHTSensors()
{
	float temperature = NAN;
	float humidity = NAN;
	_dht.read2(&temperature, &humidity, NULL);

	Blynk.virtualWrite(V9, temperature);
	Blynk.virtualWrite(V10, humidity);
}

/**
* @brief Read data from opto inputs. Print new data to LCD display Blynk mobile application.
*
* @return void
*/
void ProcessOptoInputs()
{
	// Check if any opto input changed and set new value.
	for (int i = 0; i < OPTOIN_COUNT; i++)
	{
		if (KMPDinoWiFiESP.GetOptoInState(i))
		{
			WidgetLED(V5 + i).on();
		}
		else
		{
			WidgetLED(V5 + i).off();
		}
	}
}

/**
 * @brief Set relay state.
 *
 * @return void
 */
void SetRelay(Relay relay, int status)
{
	KMPDinoWiFiESP.SetRelayState(relay, status == 1);
}

/*****************************
* Blynk methods.
*****************************/
/**
 * @brief This function will be run every time when Blynk connection is established.
 *
 */
BLYNK_CONNECTED() {
	// Request Blynk server to re-send latest values for all pins.
	Blynk.syncAll();

	ProcessDHTSensors();
	ProcessOptoInputs();
}

/**
* @brief Set Relay 1 state.
*			On virtual pin 1.
*/
BLYNK_READ(V1)
{
	Blynk.virtualWrite(V1, KMPDinoWiFiESP.GetRelayState(Relay1));
}

/**
 * @brief Set Relay 1 state.
 *			On virtual pin 1.
 */
BLYNK_WRITE(V1)
{
	SetRelay(Relay1, param.asInt());
}

/**
* @brief Set Relay 2 state.
*			On virtual pin 2.
*/
BLYNK_READ(V2)
{
	Blynk.virtualWrite(V2, KMPDinoWiFiESP.GetRelayState(Relay2));
}

/**
 * @brief Set Relay 2 state.
 *			On virtual pin 2.
 */
BLYNK_WRITE(V2)
{
	SetRelay(Relay2, param.asInt());
}

/**
* @brief Set Relay 3 state.
*			On virtual pin 3.
*/
BLYNK_READ(V3)
{
	Blynk.virtualWrite(V3, KMPDinoWiFiESP.GetRelayState(Relay3));
}

/**
 * @brief Set Relay 3 state.
 *			On virtual pin 3.
 */
BLYNK_WRITE(V3)
{
	SetRelay(Relay3, param.asInt());
}

/**
* @brief Set Relay 4 state.
*			On virtual pin 4.
*/
BLYNK_READ(V4)
{
	Blynk.virtualWrite(V4, KMPDinoWiFiESP.GetRelayState(Relay4));
}

/**
 * @brief Set Relay 4 state.
 *			On virtual pin 4.
 */
BLYNK_WRITE(V4)
{
	SetRelay(Relay4, param.asInt());
}