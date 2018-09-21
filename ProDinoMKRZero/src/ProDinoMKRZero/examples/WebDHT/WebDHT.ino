// WebDHT.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino MKR Zero Ethernet V1 (https://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/)
//		- KMP ProDino MKR GSM Ethernet V1  (https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/)
// Description:
//		Read temperature and humidity from DHT sensors and show them in a WEB page.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 21.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//		Before starts this example you need to install DHT library: https://github.com/adafruit/DHT-sensor-library
//		Connect DHT22 sensor to GROVE connector. Use pins: 
//			- first  sensor GROVE_D0, Vcc+, Gnd(-);
//			- second sensor GROVE_D1, Vcc+, Gnd(-);

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"
#include <DHT.h>

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xDA, 0x1E, 0xB3 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 198);

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

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
 * @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
 *
 * @return void
 */
void setup(void)
{
	delay(5000);
#ifdef DEBUG
	Serial.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init(ProDino_MKR_Zero_Ethernet);

	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	Serial.println("The example WebDHT is started.");
	Serial.println("IPs:");
	Serial.println(Ethernet.localIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
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
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() 
{
	GetDataFromSensors();

	// Waiting for a client.
	_client = _server.available();

	if (!_client.connected())
	{
		return;
	}

#ifdef DEBUG
	Serial.println(">> Client connected.");
#endif

	// If client connected switch On status led.
	KMPProDinoMKRZero.OnStatusLed();

	// Read client request.
	ReadClientRequest();
	WriteClientResponse();

	// Close the client connection.
	_client.stop();

	// If client disconnected switch Off status led.
	KMPProDinoMKRZero.OffStatusLed();

#ifdef DEBUG
	Serial.println(">> Client disconnected.");
	Serial.println();
#endif
}

/**
* @brief ReadClientRequest void. Read and parse client request.
* 	First row of client request is similar to:
*		GET / HTTP/1.1
* You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
*
* @return bool Returns true if the request was expected.
*/
bool ReadClientRequest()
{
	// Loop while read all request.
	String row;
	while (ReadHttpRequestLine(&_client, &row));

	return true;
}


/**
* @brief WriteClientResponse void. Write html response.
*
*
* @return void
*/
void WriteClientResponse()
{
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
	// Add table rows, relay information.
	String rows = "";
	for (uint8_t i = 0; i < SENSOR_COUNT; i++)
	{
		// Row i, cell 1
		MeasureHT_t* measureHT = &_measureHT[i];
		rows += "<tr><td"+ (measureHT->IsEnable ? "" : " bgcolor='" + String(GRAY) + "'") + ">" + measureHT->Name + "</td>";

		// Cell i,2
		rows += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Temperature) + "</td>";

		// Cell i,3
		rows += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Humidity) + "</td></tr>";
	}

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - Web DHT</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - Web DHT example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th style='width:30%'></th><th style='width:35%'>Temperature C&deg;</th><th>Humidity</th></tr></thead><tbody>"
		+ rows
		+ "</tbody></table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_MKRZERO) + "' target='_blank'>Information about " + String(PRODINO_MKRZERO) + " board</a></h3>"
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