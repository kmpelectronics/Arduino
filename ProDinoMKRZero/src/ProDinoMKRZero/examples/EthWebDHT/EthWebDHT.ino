// EthWebDHT.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino MKR Zero (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		Web server DHT example.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebdhtserver.aspx
// Version: 1.0.0
// Date: 19.04.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//		Before start this example you need to install DHT library: https://github.com/adafruit/DHT-sensor-library
//		Connect DHT22 sensor to External GROVE connector. Use pins: 
//			- first  sensor GROVE_D0, Vcc+, Gnd(-);
//			- second sensor GROVE_D1, Vcc+, Gnd(-);

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"
#include <DHT.h>

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x7D, 0x15, 0x30 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 177);

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Define sensors structure.
typedef struct
{
	// Enable sensor - true.
	bool IsEnable;
	// Name of sensor. Example: "First sensor".
	String Name;
	// DHT object with settings. Example: DHT(EXT_GROVE_D0 /* connected pin */, DHT22 /* sensor type */, 11 /* Constant for ESP8266 */)
	DHT dht;
	// Store, read humidity from sensor.
	float Humidity;
	// Store, read temperature from sensor.
	float Temperature;
} MeasureHT_t;

// Sensors count. 
#define SENSOR_COUNT 2

// Define array of 2 sensors.
MeasureHT_t _measureHT[SENSOR_COUNT] =
{
	{ true, "Sensor 1", DHT(GROVE_D0, DHT22, 11), NAN, NAN },
	{ true, "Sensor 2", DHT(GROVE_D1, DHT22, 11), NAN, NAN }
};

// Gray color.
const char GRAY[] = "#808080";
// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 5000;
// Store last measure time.
unsigned long _mesureTimeout;				

EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
 * @brief Execute first after start device. Initialize hardware.
 *
 * @return void
 */
void setup(void)
{
#ifdef DEBUG
	SerialUSB.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init();

	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	SerialUSB.println("The server is starting.");
	SerialUSB.println(Ethernet.localIP());
	SerialUSB.println(Ethernet.gatewayIP());
	SerialUSB.println(Ethernet.subnetMask());
#endif

	// Start sensors.
	for (uint8_t i = 0; i < SENSOR_COUNT; i++)
	{
		MeasureHT_t* measureHT = &_measureHT[i];

		Serial.print("Sensor name: \"" + measureHT->Name + "\" - ");
		if (measureHT->IsEnable)
		{
			measureHT->dht.begin();
			Serial.println("Start");
		}
		else
		{
			Serial.println("Disable");
		}
	}

	_mesureTimeout = 0;
}

/**
 * @brief Main method.
 *
 * @return void
 */
void loop(void)
{
	GetDataFromSensors();
	// Listen for incoming clients.
	_client = _server.available();

	if (!_client.connected())
	{
		return;
	}

#ifdef DEBUG
	SerialUSB.println("Client connected.");
#endif

	// If client connected On status led.
	//OnStatusLed();
	int c;
	// Read client request.
	while ((c = _client.peek()) > -1)
	{
		_client.read();
	}

	WriteClientResponse();

	// Close the connection.
	_client.stop();

	// If client disconnected Off status led.
	//OffStatusLed();

#ifdef DEBUG
	SerialUSB.println("Client disconnected.");
	SerialUSB.println("---");
#endif
}

/**
* \brief WriteClientResponse void. Write html response.
*
*
* \return void
*/
void WriteClientResponse()
{
#ifdef DEBUG
	SerialUSB.println("Write client response.");
#endif

	if (!_client.connected())
	{
		return;
	}

	// Response write.
	// Send a standard http header.
	_client.write(HEADER_200_TEXT_HTML);

	// Add web page HTML.
	String content = BuildPage();
	_client.write(content.c_str());
}

/**
 * @brief Build HTML page.
 *
 * @return void
 */
String BuildPage()
{
	String page =
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ZERO_ETH) + " - Web DHT</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ZERO_ETH) + " - Web DHT example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='450' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th style='width:30%'></th><th style='width:35%'>Temperature C&deg;</th><th>Humidity</th></tr></thead>";

	// Add table rows, relay information.
	String tableBody = "<tbody>";
	for (uint8_t i = 0; i < SENSOR_COUNT; i++)
	{
		// Row i, cell 1
		MeasureHT_t* measureHT = &_measureHT[i];
		tableBody += "<tr><td"+ (measureHT->IsEnable ? "" : " bgcolor='" + String(GRAY) + "'") + ">" + measureHT->Name + "</td>";

		// Cell i,2
		tableBody += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Temperature) + "</td>";

		// Cell i,3
		tableBody += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Humidity) + "</td></tr>";
	}
	tableBody += "</tbody>";

	return page + tableBody
		+ "</table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_ZERO) + "' target='_blank'>Information about " + String(PRODINO_ZERO_ETH) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}

/**
 * @brief Prepare sensor result.
 *
 * @return void
 */
String FormatMeasure(bool isEnable, float val)
{
	return isEnable ? String(val) : "-";
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
		for (uint8_t i = 0; i < SENSOR_COUNT; i++)
		{
			// Get sensor structure.
			MeasureHT_t* measureHT = &_measureHT[i];
			// Is enable - read data from sensor.
			if (measureHT->IsEnable)
			{
				measureHT->dht.read(true);
				measureHT->Humidity = measureHT->dht.readHumidity();
				measureHT->Temperature = measureHT->dht.readTemperature();
			}
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}