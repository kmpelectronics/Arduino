// AllWEG.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		ProDino ESP32 Ethernet GSM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
// Description:
//		The Blynk part of example communicates via GSM module.
//      All other: relays, inputs, RS485 and GROVE connector communicate both WiFi and Ethernet.
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
//  ProDino MKR series -> Blynk pins map:
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
#include <SimpleDHT.h>
#include "arduino_secrets.h"

//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include <BlynkSimpleSIM800.h>

#include <WiFi.h>
#include <WiFiClient.h>

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

// Define array of 2 sensors.
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

// Storing opto input data, settings and processing objects.
OptoIn_t _optoInputs[OPTOIN_COUNT] =
{
	{ OptoIn1, WidgetLED(V5), false },
	{ OptoIn2, WidgetLED(V6), false },
	{ OptoIn3, WidgetLED(V7), false },
	{ OptoIn4, WidgetLED(V8), false }
};

// It supports work with GSM Modem.
TinyGsm modem(SerialModem);

const uint8_t LOCAL_PORT = 80;

// Define text colors.
const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 
const char GRAY[] = "#808080";

// TCP server at port 80 will respond to HTTP requests
WiFiServer _wifiServer(LOCAL_PORT);

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xF9, 0x90, 0x4E };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 197);
EthernetServer _ethServer(LOCAL_PORT);

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
	Serial.println("The example GSM, Ethernet, WiFi is starting...");

	// Init Dino board. Set pins, start GSM.
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	KMPProDinoESP32.setStatusLed(blue);
	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoESP32.rs485Begin(19200);

	Serial.println("Initializing modem...");
	modem.init();

	String modemInfo = modem.getModemInfo();
	if (modemInfo == "")
	{
		Serial.println("Modem is not started!!!");
		while (true) {}
	}
	Serial.print("Modem info: ");
	Serial.println(modemInfo);

	// Unlock your SIM card if it locked with a PIN code. 
	// If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
	if (strlen(SECRET_PINNUMBER) > 0 && !modem.simUnlock(SECRET_PINNUMBER))
	{
		Serial.println("PIN code is not valid! STOP!!!");
		while (true) {}
	}

	_mesureTimeout = 0;

	Blynk.begin(AUTH_TOKEN, modem, SECRET_GPRS_APN, SECRET_GPRS_LOGIN, SECRET_GPRS_PASSWORD);
	// Connect to WiFi network
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
	Serial.println(LOCAL_PORT);

	_wifiServer.begin();

	// Start the Ethernet connection and the server.
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

	KMPProDinoESP32.offStatusLed();

	Serial.println("The example GSM, Ethernet, WiFi is started.");
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop(void)
{
	KMPProDinoESP32.processStatusLed(green, 1000);
	ProcessDHTSensors(false);
	ProcessOptoInputs(false);

	Blynk.run();
	//Serial.println("Blynk.run();");

	Client * client = NULL;
	// Check if a client has connected
	WiFiClient wifiClient = _wifiServer.available();
	//Serial.println("_wifiServer.available()");
	if (wifiClient)
	{
		Serial.println("if (wifiClient) {");
		// Wait for data from client to become available
		while (wifiClient.connected() && !wifiClient.available()) {
			delay(1);
		}

		client = &wifiClient;
	}

	EthernetClient ethClient = _ethServer.available();
	if (ethClient && ethClient.connected())
	{
		Serial.println("if (ethClient && ethClient.connected())");
		client = &ethClient;
	}

	if (client == NULL)
	{
		return;
	}

	Serial.println(">> Client connected.");

	// If client connected switch On status led.
	KMPProDinoESP32.setStatusLed(yellow);

	// Read client request.
	ReadClientRequest(client);
	WriteClientResponse(client);

	// Close the client connection.
	client->stop();

	// If client disconnected switch Off status led.
	KMPProDinoESP32.offStatusLed();

	Serial.println(">> Client disconnected.");
	Serial.println();
}

/**
 * @brief Set relay state.
 * @param relay The relay which status will change.
 * @param status A new status relay.
 *
 * @return void
 */
void SetRelay(uint8_t relayNumber, bool status, bool isBlynk = true)
{
	KMPProDinoESP32.setRelayState(relayNumber, status);

	if (!isBlynk)
	{
		Blynk.virtualWrite(relayNumber + 1, status);
	}
}

bool ReadClientRequest(Stream *client)
{
	Serial.println(">> Starts client request.");

	// Loop while read all request.
	// Read first and last row from request.
	String firstRow;
	String lastRow;
	if (ReadHttpRequestLine(client, &firstRow))
	{
		while (ReadHttpRequestLine(client, &lastRow));
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

		SetRelay(relay, newState, false);
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

void WriteClientResponse(Client *client)
{
	if (!client->connected())
	{
		return;
	}

	// Add web page HTML.
	BuildPage(client);
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
			//Serial.println("Opto input " + String(i + 1) + " status changed to -> \"" + currentStatus + "\". WidgetLED value: " + optoInput->Widget.getValue());

			currentStatus ? optoInput->Widget.on() : optoInput->Widget.off();
			optoInput->Status = currentStatus;
		}
	}
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
void BuildPage(Stream *client)
{
	// Response write.
	// Send a standard http header.
	client->write(HEADER_200_TEXT_HTML);

	client->write(
		("<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ESP32) + " - Web All</title></head>\
		<body><div style='text-align: center'>\
		<br><hr />").c_str());

	// RS485
	client->write(RS485Table().c_str());
	// Relays
	client->write(RelayTable().c_str());
	// Inputs
	client->write(InputTable().c_str());
	// DHT sensors data
	client->write(DHTTable().c_str());

	client->write(
		("<h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>\
		<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_ESP32) + "' target='_blank'>Information about " + String(PRODINO_ESP32) + " board</a></h3>\
		<hr /></div></body></html>").c_str());
}