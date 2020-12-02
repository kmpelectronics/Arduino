// Web1WireWE.ino
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
//		With this example you can read temperature from 1 Wire sensor and show it in the WEB page.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 06.04.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//		Before start this example you need to install following libraries:
//			- One wire: https://github.com/PaulStoffregen/OneWire
//			- DallasTemperature library: https://github.com/milesburton/Arduino-Temperature-Control-Library
//		Connect DS18B20 sensor(s) to GROVE connector. Use pins: 
//			- GROVE_D0, VCC+, GND(-);

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// if this define is uncommented example supports both Ethernet and WiFi
#define ETH_TEST

#define SENSORS_PIN GROVE_D0

// Check sensor data, interval in milliseconds.
const long CHECK_DEVICE_INTERVAL_MS = 30000;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire _oneWire(SENSORS_PIN);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature _sensors(&_oneWire);

// Temp device address.
DeviceAddress _tempDeviceAddress;
uint8_t _getDeviceCount;
// Buffer to Hex bytes.
char _buffer[8 * 2 + 1];
// Store last measure time.
unsigned long _mesureTimeout;

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xC3, 0x82, 0x35 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 197);

// Local port. The port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Initialize the Ethernet server library.
EthernetServer _ethServer(LOCAL_PORT);
WiFiServer _wifiServer(LOCAL_PORT);

/**
 * @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
 *
 * @return void
 */
void setup(void)
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example Web1Wire is starting...");

	//KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
#ifdef ETH_TEST
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
#endif // ETH_TEST

	KMPProDinoESP32.setStatusLed(blue);

	// Connect to WiFi network
	WiFi.begin(SSID, SSID_PASSWORD);
	Serial.print("\n\r \n\rWiFi is connecting");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	Serial.print("WiFi IP: ");
	Serial.print(WiFi.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);

	_wifiServer.begin();

#ifdef ETH_TEST
	// Start the Ethernet connection and the server.
	//Ethernet.begin(_mac, _ip);
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}
	_ethServer.begin();

	Serial.println("Ethernet IP:");
	Serial.print(Ethernet.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);
#endif // ETH_TEST

	KMPProDinoESP32.offStatusLed();
	_mesureTimeout = 0;

	Serial.println("The example Web1Wire is started.");
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
	GetDataFromSensors();

	Client* client = NULL;
	// Check if a client has connected
	WiFiClient wifiClient = _wifiServer.available();
	if (wifiClient)
	{
		KMPProDinoESP32.setStatusLed(yellow);

		Serial.println("-- wifiClient --");
		// Wait for data from the client that become available
		while (wifiClient.connected() && !wifiClient.available()) {
			delay(1);
		}

		client = &wifiClient;
	}
#ifdef ETH_TEST
	EthernetClient ethClient = _ethServer.available();
	if (ethClient && ethClient.connected())
	{
		Serial.println("-- ethClient --");
		client = &ethClient;
	}
#endif
	if (client == NULL)
	{
		return;
	}

	KMPProDinoESP32.setStatusLed(yellow);

	Serial.println(">> Client connected.");

	// Read client request.
	if (ReadClientRequest(client))
	{
		WriteClientResponse(client);
	}

	// Close the client connection.
	client->stop();
	Serial.println(">> Client disconnected.");
	Serial.println();

	// If client disconnected switch Off status led.
	KMPProDinoESP32.offStatusLed();
}

/**
* @brief ReadClientRequest void. Read and parse client request.
* 	First row of client request is similar to:
*		GET / HTTP/1.1
* You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
*
* @return bool Returns true if the request was expected.
*/
bool ReadClientRequest(Stream* client)
{
	// Loop while read all request.
	String row;
	while (ReadHttpRequestLine(client, &row));

	return true;
}

/**
* @brief WriteClientResponse void. Write html response.
*
*
* @return void
*/
void WriteClientResponse(Client* client)
{
	if (!client->connected())
	{
		return;
	}

	// Response write.
	// Send a standard http header.
	client->print(HEADER_200_TEXT_HTML);

	// Add web page HTML.
	String content = BuildPage();
	client->print(content.c_str());
}

/**
 * @brief Build HTML page.
 *
 * @return void
 */
String BuildPage()
{
	int deviceCount = _sensors.getDeviceCount();
#ifdef DEBUG
	Serial.print("Device count: ");
	Serial.println(deviceCount);
#endif

	_sensors.requestTemperatures();

	// Add table rows, relay information.
	String rows = "";
	for (int i = 0; i < deviceCount; i++)
	{
		// Default color.
		const char* cellColor = W_GRAY;
		float temp = 0;
		bool sensorAvaible = _sensors.getAddress(_tempDeviceAddress, i);
		String sensorName = "Ghost device";
		String sensorId = W_NA;

		if (sensorAvaible)
		{
			// Get temperature in Celsius.
			temp = _sensors.getTempC(_tempDeviceAddress);

			// Select cell background.
			if (0.0 >= temp)
			{
				cellColor = W_BLUE;
			}
			else if (22.0 >= temp)
			{
				cellColor = W_GREEN;
			}
			else
			{
				cellColor = W_RED;
			}

			sensorName = "Sensor " + String(IntToChar(i + 1));
			BytesToHexStr(_tempDeviceAddress, 8, _buffer);
			sensorId = _buffer;
		}

		// Row i, cell 1
		rows += "<tr><td>" + sensorName + "<br><font size='2'>Id: " + sensorId + "</font></td>"
			// Add cell i,2
			+ AddTemperatureCell(sensorAvaible, temp, cellColor)
			// Add cell i,3
			+ AddTemperatureCell(sensorAvaible, _sensors.toFahrenheit(temp), cellColor)
			+ "</tr>";
	}

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ESP32) + " - Web 1 Wire</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Web 1 Wire example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th></th><th>C&deg;</th><th>F&deg;</th></tr></thead><tbody>"
		+ rows
		+ "</tbody></table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_ESP32) + "' target='_blank'>Information about " + String(PRODINO_ESP32) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}

/**
* @brief Add cell in table, include temperature data.
*
* @param sensorAvaible Sensor available. If true - available, else not available.
* @param temperature To add.
* @param cellColor Cell background color in text.
*
* @return String formated cell.
*/
String AddTemperatureCell(bool sensorAvaible, double temperature, const char* cellColor)
{
	String result = "<td bgcolor='" + String(cellColor) + "'>";
	if (sensorAvaible)
	{
		FloatToChars(temperature, 1, _buffer);
		result += String(_buffer);
	}
	else
	{
		result += String(W_NA);
	}

	result += "</td>";

	return result;
}

/**
 * @brief Read data from sensors a specified time.
 *
 * @return void
 */
void GetDataFromSensors()
{
	if (millis() > _mesureTimeout)
	{
		// Start the One Wire library. Get one wire devices.
		_sensors.begin();

		// Set precision.
		// Thermometer Resolution in bits. http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf page 8. Bits - CONVERSION TIME. 9 - 93.75ms, 10 - 187.5ms, 11 - 375ms, 12 - 750ms. 
		_sensors.setResolution(9);

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_DEVICE_INTERVAL_MS;
	}
}