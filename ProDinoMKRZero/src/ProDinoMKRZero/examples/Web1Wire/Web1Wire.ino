// Web1Wire.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino MKR Zero Ethernet V1 (https://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/)
//		- KMP ProDino MKR GSM Ethernet V1  (https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/)
// Description:
//		Read temperature from 1 Wire sensor and show it in a WEB page.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 21.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// Prerequisites:
//		Before start this example you need to install following libraries:
//			- One wire: https://github.com/PaulStoffregen/OneWire
//			- DallasTemperature library: https://github.com/milesburton/Arduino-Temperature-Control-Library
//		Connect DS18B20 sensor(s) to GROVE connector. Use pins: 
//			- GROVE_D0, VCC+, GND(-);

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG
#define SENSORS_PIN GROVE_D0
// Thermometer Resolution in bits. http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf page 8. Bits - CONVERSION TIME. 9 - 93.75ms, 10 - 187.5ms, 11 - 375ms, 12 - 750ms. 
#define TEMPERATURE_PRECISION 9

const char WHITE[] = "white";
const char BLUE[] = "blue";
const char GREEN[] = "green";
const char RED[] = "red";
const char GRAY[] = "#808080";
const char NA[] = "N/A";
const char DEGREE_SYMBOL[] = "&deg;";
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
char _buffer[8*2 + 1];
// Store last measure time.
unsigned long _mesureTimeout;				

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x37, 0x49, 0xF7 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 198);

// Local port. The port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

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
	Serial.println("The example Web1Wire is started.");
	Serial.println("IPs:");
	Serial.println(Ethernet.localIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
#endif

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
		const char* cellColor = GRAY;
		float temp = 0;
		bool sensorAvaible = _sensors.getAddress(_tempDeviceAddress, i);
		String sensorName = "Ghost device";
		String sensorId = NA;
		if (sensorAvaible)
		{
			// Get temperature in Celsius.
			temp = _sensors.getTempC(_tempDeviceAddress);

			// Select cell background.
			if (0.0 >= temp)
			{
				cellColor = BLUE;
			}
			else if (22.0 >= temp)
			{
				cellColor = GREEN;
			}
			else
			{
				cellColor = RED;
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

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - Web 1 Wire</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - Web 1 Wire example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th></th><th>C&deg;</th><th>F&deg;</th></tr></thead><tbody>"
		+ rows
		+ "</tbody></table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_MKRZERO) + "' target='_blank'>Information about " + String(PRODINO_MKRZERO) + " board</a></h3>"
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
		result += String(NA);
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
		_sensors.setResolution(TEMPERATURE_PRECISION);

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_DEVICE_INTERVAL_MS;
	}
}