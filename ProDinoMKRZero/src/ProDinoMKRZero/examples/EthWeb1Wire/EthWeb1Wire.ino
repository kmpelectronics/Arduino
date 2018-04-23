// EthWeb1Wire.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino MKR Zero (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		Web server 1 Wire measure temperature with DS18B20 example.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiweb1wiresrv.aspx
// Version: 1.0.0
// Date: 23.04.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//		Before start this example you need to install following libraries:
//			- One wire: https://github.com/PaulStoffregen/OneWire
//			- DallasTemperature library: https://github.com/milesburton/Arduino-Temperature-Control-Library
//		Connect DS18B20 sensor(s) to GROVE connector. Use pins: 
//			- GROVE_D0, VCC+, GND(-);

#include <KMPProDinoMKRZero.h>
#include <KMPCommon.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
// Local port. The port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire _oneWire(SENSORS_PIN);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature _sensors(&_oneWire);

// Store last measure time.
unsigned long _checkDeviceTimeout;				
// Temp device address.
DeviceAddress _tempDeviceAddress;
uint8_t _getDeviceCount;
// Buffer to Hex bytes.
char _buffer[8*2 + 1];

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x7D, 0x15, 0x30 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 177);
EthernetServer _server(LOCAL_PORT);
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
	KMPProDinoMKRZero.init(true);

	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	SerialUSB.println("The server is starting.");
	SerialUSB.println(Ethernet.localIP());
	SerialUSB.println(Ethernet.gatewayIP());
	SerialUSB.println(Ethernet.subnetMask());
#endif

	_checkDeviceTimeout = 0;

	// Select available connected to board One Wire devices.
	GetOneWireDevices();
}

/**
 * @brief Main method.
 *
 * @return void
 */
void loop(void)
{
	CheckDevices();
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
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - Web 1Wire</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - Web 1Wire example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='450' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th></th><th>C&deg;</th><th>F&deg;</th></tr></thead>";

	int deviceCount = _sensors.getDeviceCount();

	_sensors.requestTemperatures();

	// Add table rows.
	String tableBody = "<tbody>";
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
		tableBody += "<tr><td>" + sensorName + "<br><font size='2'>Id: " + sensorId + "</font></td>";

		// Add cell i,2
		tableBody += AddTemperatureCell(sensorAvaible, temp, cellColor);

		// Add cell i,3
		tableBody += AddTemperatureCell(sensorAvaible, _sensors.toFahrenheit(temp), cellColor);

		// End row.
		tableBody += "</tr>";
	}

	tableBody += "</tbody>";

	return page + tableBody
		+ "</table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
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
void CheckDevices()
{
	if (millis() > _checkDeviceTimeout)
	{
		GetOneWireDevices();

		// Set next time to read data.
		_checkDeviceTimeout = millis() + CHECK_DEVICE_INTERVAL_MS;
	}
}

/**
 * @brief Get all available One Wire devices.
 *
 * @return void
 */
void GetOneWireDevices()
{
	// Start the One Wire library. Get one wire devices.
	_sensors.begin();

	// Set precision.
	_sensors.setResolution(TEMPERATURE_PRECISION);
}