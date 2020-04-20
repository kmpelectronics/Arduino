// AllE.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino ESP32 Ethernet V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet GSM V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet LoRa V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet LoRa RFM V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
// Description:
//		This Ethernet example combine all tests: Relays, inputs, RS485 and GROVE connector.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 20.04.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install:
//		Install library: Sketch\Include library\Menage Libraries... find ... and click Install.
//         - SimpleDHT by Winlin
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. Use pins: 
//			- sensor 1 GROVE_D0, Vcc+, Gnd(-);
//			- sensor 2 GROVE_D1, Vcc+, Gnd(-); - it is not mandatory.

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include <SimpleDHT.h>

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x72, 0xE7, 0x40 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 197);

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Define text colors.
const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

// Define sensors structure.
typedef struct
{
	// Enable sensor - true.
	bool IsEnable;
	// Name of sensor. Example: "First sensor".
	String Name;
	// SimpleDHT22 object.
	SimpleDHT22 dht;
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
	{ true, "Sensor 1", SimpleDHT22(GROVE_D0), NAN, NAN },
	{ true, "Sensor 2", SimpleDHT22(GROVE_D1), NAN, NAN }
};
// Gray color.
const char GRAY[] = "#808080";
// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 5000;
// Store last measure time.
unsigned long _mesureTimeout = 0;

/**
* @brief Setup void. It is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);

	// Init Dino board. Set pins, start W5500.
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
	KMPProDinoESP32.setStatusLed(blue);
	
	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoESP32.rs485Begin(19200);

	// Start the Ethernet connection and the server.
	//Ethernet.begin(_mac, _ip);
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}
	_server.begin();

	Serial.println("Ethernet IP:");
	Serial.print(Ethernet.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);

	KMPProDinoESP32.offStatusLed();
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

	// Waiting for a client.
	_client = _server.available();

	if (!_client.connected())
	{
		return;
	}

	Serial.println(">> Client connected.");

	// If client connected switch On status led.
	KMPProDinoESP32.setStatusLed(yellow);

	// Read client request.
	ReadClientRequest();
	WriteClientResponse();

	// Close the client connection.
	_client.stop();

	// If client disconnected switch Off status led.
	KMPProDinoESP32.offStatusLed();

	Serial.println(">> Client disconnected.");
	Serial.println();
}

/**
* @brief ReadClientRequest void. Read and parse client request.
* 	First row of client request is similar to:
*		GET / HTTP/1.1
*  -or-
*       POST / HTTP/1.1
*       Host: 192.168.0.177
*       Connection: keep-alive
*       Content-Length: 5
*
*       r1=On
* You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
*
* @return bool Returns true if the request was expected.
*/
bool ReadClientRequest()
{
	Serial.println(">> Starts client request.");

	// Loop while read all request.
	// Read first and last row from request.
	String firstRow;
	String lastRow;
	if (ReadHttpRequestLine(&_client, &firstRow))
	{
		while (ReadHttpRequestLine(&_client, &lastRow));
	}

	Serial.println("--firstRow--");
	Serial.println(firstRow);
	Serial.println("--lastRow--");
	Serial.println(lastRow);

	// If the request is GET we write only response.
	if (GetRequestType(firstRow.c_str()) == GET)
	{
		return true;
	}

	// Invalid request type.
	if (GetRequestType(firstRow.c_str()) != POST || lastRow.length() == 0)
	{
		Serial.println(">> Invalid request type.");
		return false;
	}

	// Relay request.
	if (lastRow[0] == 'r')
	{
		// From POST parameters we get relay number and new status.
		uint8_t relay = CharToInt(lastRow[1]) - 1;
		bool newState = lastRow.endsWith(W_ON);

		KMPProDinoESP32.setRelayState(relay, newState);
	}

	// RS485
	if (lastRow.startsWith("data"))
	{
		// From POST parameters we get data should be send.
		String dataToSend = GetValue(lastRow, "data");

		Serial.print("RS485 data to send: ");
		Serial.println(dataToSend);

		// Transmit data.
		KMPProDinoESP32.rs485Write(dataToSend);
	}

	Serial.println(">> End client request.");
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

	// Add web page HTML.
	BuildPage();
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
				measureHT->Temperature = NAN;
				measureHT->Humidity = NAN;

				measureHT->dht.read2(&measureHT->Temperature, &measureHT->Humidity, NULL);

				Serial.print("Sensor ");
				Serial.print(i);
				Serial.print(" Humidity: ");
				Serial.print(measureHT->Humidity);
				Serial.print(" Temperature: ");
				Serial.println(measureHT->Temperature);
			}
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

String RelayTable()
{
	// Add table rows which includes relays information.
	String rows = "";
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		// Row i, cell 1
		String relayNumber = String(i + 1);
		rows += "<tr><td>Relay " + relayNumber + "</td>";

		char* cellColor;
		char* cellStatus;
		char* nextRelayStatus;
		if (KMPProDinoESP32.getRelayState(i))
		{
			cellColor = (char*)RED;
			cellStatus = (char*)W_ON;
			nextRelayStatus = (char*)W_OFF;
		}
		else
		{
			cellColor = (char*)GREEN;
			cellStatus = (char*)W_OFF;
			nextRelayStatus = (char*)W_ON;
		}

		// Cell i,2
		rows += "<td bgcolor='" + String(cellColor) + "'>" + String(cellStatus) + "</td>";

		// Cell i,3
		rows += "<td><input type='submit' name='r" + String(relayNumber) + "' value='" + String(nextRelayStatus) + "'/ ></td></tr>";
	}

	return "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Web Relay example</h1>\
		<hr /><br><br>\
		<form method = 'post'>\
		<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ rows
		+ "</table>\
		</form><br><br><hr />";
}

String InputTable()
{
	// Add table rows which include input information.
	String tableBody = "";
	String tableHeader = "";
	for (uint8_t i = 0; i < OPTOIN_COUNT; i++)
	{
		tableHeader += "<th>In " + String(i + 1) + "</th>";

		char* cellColor;
		char* cellStatus;
		if (KMPProDinoESP32.getOptoInState(i))
		{
			cellColor = (char*)RED;
			cellStatus = (char*)W_ON;
		}
		else
		{
			cellColor = (char*)GREEN;
			cellStatus = (char*)W_OFF;
		}

		tableBody += "<td bgcolor='" + String(cellColor) + "'>" + String(cellStatus) + "</td>";
	}

	return "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Isolated inputs example</h1>\
		<hr /><br><br><table border='1' width='300' cellpadding='5' cellspacing='0' align='center' \
		 style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'><thead><tr>"
		+ tableHeader + "</tr></thead>"
		+ "<tbody><tr>" + tableBody + "</tr></tbody></table><br><br><hr />";
}

String RS485Table()
{
	String receivedData = "";
	int i;
	// Reading data from the RS485 port.
	while ((i = KMPProDinoESP32.rs485Read()) != -1)
	{
		// Adding received data in a buffer.
		receivedData += (char)i;
		Serial.write((char)i);
	}

	return
		"<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - RS485</h1>\
		<hr /><br><br>\
		<form method='post'>\
		<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'> \
		<thead><tr><th width='80%'>Data</th><th>Action</th></tr></thead> \
		<tbody><tr><td><input type='text' name='data' style='width: 100%'></td> \
		<td><input type='submit' name='btn' value='Transmit'/></td></tr> \
		<tr><td>"
		+ receivedData
		+ "</td><td>Received</td></tr></tbody>\
		</table>\
		</form><br><br><hr />";
}

String DHTTable()
{
	// Add table rows, relay information.
	String rows = "";
	for (uint8_t i = 0; i < SENSOR_COUNT; i++)
	{
		// Row i, cell 1
		MeasureHT_t* measureHT = &_measureHT[i];
		rows += "<tr><td" + (measureHT->IsEnable ? "" : " bgcolor='" + String(GRAY) + "'") + ">" + measureHT->Name + "</td>";

		// Cell i,2
		rows += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Temperature) + "</td>";

		// Cell i,3
		rows += "<td>" + FormatMeasure(measureHT->IsEnable, measureHT->Humidity) + "</td></tr>";
	}

	return "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Web DHT example</h1>\
		<hr /><br><br>\
		<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>\
		<thead><tr><th style='width:30%'></th><th style='width:35%'>Temperature C&deg;</th><th>Humidity</th></tr></thead><tbody>"
		+ rows
		+ "</tbody></table><br><br><hr />";

}

/**
* @brief Build HTML page.
*
* @return void
*/
void BuildPage()
{
	// Response write.
	// Send a standard http header.
	_client.write(HEADER_200_TEXT_HTML);

	_client.write(
		("<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ESP32) + " - Web All</title></head>\
		<body><div style='text-align: center'>\
		<br><hr />").c_str());

	// RS485
	_client.write(RS485Table().c_str());
	// Relays
	_client.write(RelayTable().c_str());
	// Inputs
	_client.write(InputTable().c_str());
	// DHT sensors data
	_client.write(DHTTable().c_str());

	_client.write(
		("<h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>\
		<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_ESP32) + "' target='_blank'>Information about " + String(PRODINO_ESP32) + " board</a></h3>\
		<hr /></div></body></html>").c_str());
}