// SimpleAPI.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		PRODINo ESP32 https://kmpelectronics.eu/shop/prodino-esp32/
//		PRODINo ESP32 Ethernet https://kmpelectronics.eu/shop/prodino-esp32-ethernet/
//		PRODINo ESP32 3G https://kmpelectronics.eu/shop/prodino-esp32-3g/
//		PRODINo ESP32 LoRa WAN https://kmpelectronics.eu/shop/prodino-esp32-lora-wan/
//		PRODINo ESP32 LoRa RFM95W https://kmpelectronics.eu/shop/prodino-esp32-lora-rfm95w/
//		PRODINo ESP32 3G Ethernet https://kmpelectronics.eu/shop/prodino-esp32-3g-ethernet/
//		PRODINo ESP32 LoRa WAN Ethernet https://kmpelectronics.eu/shop/prodino-esp32-lora-wan-ethernet/ 
//		PRODINo ESP32 LoRa RFM95W Ethernet https://kmpelectronics.eu/shop/prodino-esp32-lora-rfm95w-ethernet/
// Description:
//      With this example you can manage the board and get information through web browser for: relays, inputs, GROVE connector.
//      URI: (please, change IP address [xxx.xxx.xxx.xxx] with yours [you can get it from debug console - example: 192.168.1.202])
//		  http://xxx.xxx.xxx.xxx/?r1=0 or r1=1, r2=0 or r2=1 ... r4=0 or r4=1 - set a relay status
//		  http://xxx.xxx.xxx.xxx/relays - get all relay statuses
//		  http://xxx.xxx.xxx.xxx/inputs - get all input statuses
//		  http://xxx.xxx.xxx.xxx/temp - get temperature from connected DHT22 sensor
//		  http://xxx.xxx.xxx.xxx/hum - get humidity from connected DHT22 sensor
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 20.12.2020
// Author: KMP Electronics Ltd <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install:
//		Installing library: Sketch\Include library\Menage Libraries... find ... and click Install.
//         - SimpleDHT by Winlin
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. Use pins: 
//			- sensor GROVE_D0, Vcc+, Gnd(-);
//		You have to fill fields in arduino_secrets.h file.

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include <SimpleDHT.h>
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WebServer.h>

WebServer _server(80);
SimpleDHT22 _dht(GROVE_D0);

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
	Serial.println("Simple API example is starting...");

	// Init PRODINo board...
	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);

	KMPProDinoESP32.setStatusLed(blue);

	// Reset Relay status.
	KMPProDinoESP32.setAllRelaysOff();

	// Connect to WiFi network
	WiFi.mode(WIFI_MODE_STA);
	WiFi.begin(SSID_NAME, SSID_PASSWORD);
	Serial.print("\n\r \n\rWorking to connect");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println();
	Serial.print("WiFi IP: ");
	Serial.print(WiFi.localIP());
	Serial.print(":");
	Serial.println("80");

	_server.on("/", HTTP_GET, handleChangeRelay);
	_server.on("/relays", HTTP_GET, handleRelays);
	_server.on("/inputs", HTTP_GET, handleInputs);
	_server.on("/temp", HTTP_GET, handleTemperature);
	_server.on("/hum", HTTP_GET, handleHumidity);
	_server.onNotFound([]() {
		_server.send(404, "text/plain", "Not found");
		});

	_server.begin();

	KMPProDinoESP32.offStatusLed();
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop(void)
{
	_server.handleClient();

	KMPProDinoESP32.processStatusLed(green, 1000);
}

void handleChangeRelay()
{
	if (_server.args() != 1)
	{
		_server.send(400, "text/plain", "Bad Request");
	}

	String argName = _server.argName(0);
	String argVal = _server.arg(0);

	// Set relay state: http://192.168.1.202/?r1=0 or r1=1, r2=0 or r2=1 ... r4=0 or r4=1
	if (argName.length() == 2 && argName[0] == 'r' && argName[1] >= '1' && argName[1] <= '4' &&
		argVal.length() == 1 && argVal[0] >= '0' && argVal[0] <= '1')
	{
		int relay = argName[1] - '1';
		
		Serial.print("argVal[0]: ");
		Serial.println(argVal[0]);

		KMPProDinoESP32.setRelayState(relay, argVal[0] == '1');

		_server.send(200, "text/plain", KMPProDinoESP32.getRelayState(relay) ? "1" : "0");

		return;
	}

	_server.send(400, "text/plain", "Bad Request");
}

// Get all relays state: http://192.168.1.202/relays
void handleRelays()
{
	char relayStatus[5]{};

	for (size_t i = 0; i < RELAY_COUNT; i++)
	{
		relayStatus[i] = KMPProDinoESP32.getRelayState(i) ? '1' : '0';
	}

	_server.send(200, "text/plain", relayStatus);
}

// Get all inputs state: http://192.168.1.202/inputs
void handleInputs()
{
	char relayStatus[5]{};

	for (size_t i = 0; i < OPTOIN_COUNT; i++)
	{
		relayStatus[i] = KMPProDinoESP32.getOptoInState(i) ? '1' : '0';
	}

	_server.send(200, "text/plain", relayStatus);
}

float readTempAndHum(bool getTemp)
{
	float temperature;
	float humidity;

	if (_dht.read2(&temperature, &humidity, NULL) != 0)
	{
		return NAN;
	}

	if (getTemp)
	{
		return temperature;
	}

	return humidity;
}

// Get temperature: http://192.168.1.202/temp
void handleTemperature()
{
	_server.send(200, "text/plain", String(readTempAndHum(true)));
}

// Get humidity: http://192.168.1.202/hum
void handleHumidity()
{
	_server.send(200, "text/plain", String(readTempAndHum(false)));
}