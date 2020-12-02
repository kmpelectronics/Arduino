// BlynkWE.ino
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
//		This Blynk example communicate through WiFi or Ethernet.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 20.04.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install:
//		Install Blynk library: Sketch\Include library\Menage Libraries... find ... and click Install.
//         - Blynk
//         - TinyGSM
//         - SimpleDHT by Winlin
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. Use pins: 
//			- sensor GROVE_D0, Vcc+, Gnd(-);
//		You have to fill fields in arduino_secrets.h file.
//  ProDino ESP32 -> Blynk pins map:
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

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <SimpleDHT.h>

// if this define uncommented example supports only boards with Ethernet
#define ETH_TEST

#ifdef ETH_TEST
	#include "Ethernet/EthernetClient.h"
	#include <../Blynk/src/Adapters/BlynkEthernet.h>
	static EthernetClient _blynkEthernetClient;
	static BlynkArduinoClient _blynkTransport(_blynkEthernetClient);
	BlynkEthernet Blynk(_blynkTransport);
	#include <BlynkWidgets.h>
#else
	#include <BlynkSimpleEsp32.h>
#endif // ETH_TEST

//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

// Define sensors structure.
struct MeasureHT_t
{
	// Enable sensor - true, disable - false.
	bool IsEnable;
	// Name of sensor. Example: "First sensor".
	String Name;
	// DHT object with settings. Example: DHT(GROVE_D0 /* connected pin */, DHT22 /* sensor type */, 11 /* Constant */)
	SimpleDHT22 dht;
	// Store, read humidity from sensor.
	float Humidity;
	// Store, read temperature from sensor.
	float Temperature;
};

// Sensors count. 
#define SENSOR_COUNT 1

// Define an array with 1 sensors.
MeasureHT_t _measureHT[SENSOR_COUNT] =
{
	{ true, "Sensor 1", SimpleDHT22(GROVE_D0), NAN, NAN }
};

// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 10000;
// Store last measure time.
unsigned long _mesureTimeout;				

// Opto input structure.
struct OptoIn_t
{
	OptoIn Input;
	WidgetLED Widget;
	bool Status;
};

// Store opto input data, settings and processing objects.
OptoIn_t _optoInputs[OPTOIN_COUNT] =
{
	{ OptoIn1, WidgetLED(V5), false },
	{ OptoIn2, WidgetLED(V6), false },
	{ OptoIn3, WidgetLED(V7), false },
	{ OptoIn4, WidgetLED(V8), false }
};

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example BlynkWE is starting...");

	// Init Dino board. Set pins, start GSM.
#ifndef ETH_TEST
	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
#else
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
#endif
	KMPProDinoESP32.setStatusLed(blue);

	_mesureTimeout = 0;

#ifdef ETH_TEST
	Blynk.begin(AUTH_TOKEN);
	// You can also specify server:
	//Blynk.begin(AUTH_TOKEN, "blynk-cloud.com", 80);
	//Blynk.begin(AUTH_TOKEN, IPAddress(192,168,1,100), 8080);
#else
	Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD);
	// You can also specify server:
	//Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD, "blynk-cloud.com", 80);
	//Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD, IPAddress(192,168,1,100), 8080);
#endif

	KMPProDinoESP32.offStatusLed();

	Serial.println("The example BlynkWE is started.");
}

/**
* @brief Loop void. Arduino executed second.
*
* @return void
*/
void loop(void)
{
	KMPProDinoESP32.processStatusLed(green, 1000);

	ProcessDHTSensors(false);
	ProcessOptoInputs(false);

	Blynk.run();
}

/**
 * @brief Reading temperature and humidity from DHT sensors every X seconds and if data is changed send it to Blynk.
 *
 * @return void
 */
void ProcessDHTSensors(bool force)
{
	// Checking if time to measure is occurred
	if (millis() > _mesureTimeout)
	{
		int firstFreeVirtualPin = V9;

		for (uint8_t i = 0; i < SENSOR_COUNT; i++)
		{
			// Get sensor structure.
			MeasureHT_t* measureHT = &_measureHT[i];
			// Is enable - read data from sensor.
			if (measureHT->IsEnable)
			{
				float humidity = NAN;
				float temperature = NAN;
				measureHT->dht.read2(&temperature, &humidity, NULL);

				if (measureHT->Humidity != humidity || measureHT->Temperature != temperature || force)
				{
					measureHT->Humidity = humidity;
					measureHT->Temperature = temperature;

					// Write pair of data in pins V9, V10. If have second write V11, V12.
					Blynk.virtualWrite(firstFreeVirtualPin++, measureHT->Temperature);
					Blynk.virtualWrite(firstFreeVirtualPin++, measureHT->Humidity);
				}
			}
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

/**
* @brief Processing data from isolated inputs. It will send data to Blynk if only the input statuses were changed.
*
* @return void
*/
void ProcessOptoInputs(bool force)
{
	for (int i = 0; i < OPTOIN_COUNT; i++)
	{
		OptoIn_t* optoInput = &_optoInputs[i];
		bool currentStatus = KMPProDinoESP32.getOptoInState(optoInput->Input);
		if (optoInput->Status != currentStatus || ((bool)optoInput->Widget.getValue()) != currentStatus || force)
		{
			Serial.println("Opto input " + String(i + 1) + " status changed to -> \"" + currentStatus + "\". WidgetLED value: " + optoInput->Widget.getValue());

			currentStatus ? optoInput->Widget.on() : optoInput->Widget.off();
			optoInput->Status = currentStatus;
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
	KMPProDinoESP32.setRelayState(relay, status == 1);
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

	ProcessDHTSensors(true);
	ProcessOptoInputs(true);
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
* @brief Set Relay 1 state.
*			On virtual pin 1.
*/
BLYNK_READ(V1)
{
	Blynk.virtualWrite(V1, KMPProDinoESP32.getRelayState(Relay1));
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
BLYNK_WRITE(V3)
{
	SetRelay(Relay3, param.asInt());
}

/**
 * @brief Set Relay 4 state.
 *			On virtual pin 4.
 */
BLYNK_WRITE(V4)
{
	SetRelay(Relay4, param.asInt());
}