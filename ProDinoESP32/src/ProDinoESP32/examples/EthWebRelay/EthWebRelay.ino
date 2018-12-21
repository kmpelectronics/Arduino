// EthWebRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino ESP32 Ethernet V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/)
//		- KMP ProDino ESP32 Ethernet GSM V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-GSM-v1/)
//		- KMP ProDino ESP32 Ethernet LoRa V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-v1/)
//		- KMP ProDino ESP32 Ethernet LoRa RFM V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-rfm-v1/)
// Description:
//		Relays manipulation web example.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 17.10.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x1A, 0xE4, 0xB8 };
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

const long LED_STATUS_INTERVAL_MS = 1000;
unsigned long _ledStatusTimeout = 0;
bool _ledState = false;

/**
* @brief Setup void. It is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
#ifdef DEBUG
	Serial.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoESP32.init(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.init(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.init(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.init(ProDino_ESP32_Ethernet_LoRa_RFM);
	KMPProDinoESP32.SetStatusLed(blue);

	// Start the Ethernet connection and the server.
	//Ethernet.begin(_mac, _ip);
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}
	_server.begin();

#ifdef DEBUG
	Serial.println("The example Ethernet WebRelay is started.");
	Serial.println("IPs:");
	Serial.println(Ethernet.localIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
#endif

	KMPProDinoESP32.OffStatusLed();
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() 
{
	ShowStatus();
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
	KMPProDinoESP32.SetStatusLed(yellow);

	// Read client request.
	ReadClientRequest();
	WriteClientResponse();

	// Close the client connection.
	_client.stop();

	// If client disconnected switch Off status led.
	KMPProDinoESP32.OffStatusLed();

#ifdef DEBUG
	Serial.println(">> Client disconnected.");
	Serial.println();
#endif
}

void ShowStatus()
{
	if (millis() > _ledStatusTimeout)
	{
		_ledState = !_ledState;

		if (_ledState)
		{
			// Here you can check statuses: is WiFi connected, is there Ethernet connection and other...
			KMPProDinoESP32.SetStatusLed(green);
		}
		else
		{
			KMPProDinoESP32.OffStatusLed();
		}

		// Set next time to read data.
		_ledStatusTimeout = millis() + LED_STATUS_INTERVAL_MS;
	}
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
#ifdef DEBUG
	Serial.println(">> Starts client request.");
#endif

	// Loop while read all request.
	// Read first and last row from request.
	String firstRow;
	String lastRow;
	if (ReadHttpRequestLine(&_client, &firstRow))
	{
		while (ReadHttpRequestLine(&_client, &lastRow));
	}

#ifdef DEBUG
	Serial.println("--firstRow--");
	Serial.println(firstRow);
	Serial.println("--lastRow--");
	Serial.println(lastRow);
#endif

	// If the request is GET we write only response.
	if (GetRequestType(firstRow.c_str()) == GET)
	{
		return true;
	}

	// Invalid request type.
	if (GetRequestType(firstRow.c_str()) != POST || lastRow.length() < 2)
	{
#ifdef DEBUG
		Serial.println(">> Invalid request type.");
#endif
		return false;
	}

	// From POST parameters we get relay number and new status.
	uint8_t relay = CharToInt(lastRow[1]) - 1;
	bool newState = lastRow.endsWith(W_ON);

	KMPProDinoESP32.SetRelayState(relay, newState);

#ifdef DEBUG
	Serial.println(">> End client request.");
#endif

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
		if (KMPProDinoESP32.GetRelayState(i))
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

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ESP32) + " - Web Relay</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Web Relay example</h1>"
		+ "<hr /><br><br>"
		+ "<form method='post'>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ rows
		+ "</table></form><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_ESP32) + "' target='_blank'>Information about " + String(PRODINO_ESP32) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}