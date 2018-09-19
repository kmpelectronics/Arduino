// WebInput.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino MKR Zero Ethernet V1 (https://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/)
//		- KMP ProDino MKR GSM Ethernet V1  (https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/)
// Description:
//		Reading isolated input statuses through web page.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 18.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xCE, 0x13, 0x12 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 198);

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

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
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
	KMPProDinoMKRZero.init(ProDino_MKR_Zero_Ethernet);

	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	Serial.println("The example WebInput is started.");
	Serial.println("IPs:");
	Serial.println(Ethernet.localIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
#endif
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() 
{
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
#ifdef DEBUG
	Serial.println(">> Starting client request...");
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
#ifdef DEBUG
		Serial.println(">> End client request.");
#endif
		return true;
	}
	else
	{
#ifdef DEBUG
		Serial.println(">> Invalid request type.");
#endif
		return false;
	}
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
	String tableBody = "";
	String tableHeader = "";
	for (uint8_t i = 0; i < OPTOIN_COUNT; i++)
	{
		tableHeader += "<th>In " + String(i + 1) + "</th>";

		char* cellColor;
		char* cellStatus;
		if (KMPProDinoMKRZero.GetOptoInState(i))
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

	String rows = "<thead><tr>" + tableHeader + "</tr></thead>"
		+ "<tbody><tr>" + tableBody + "</tr></tbody>";

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - Isolated inputs</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - Isolated inputs example</h1>"
		+ "<hr /><br><br>"
		+ "<form method='post'>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ rows
		+ "</table></form><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_MKRZERO) + "' target='_blank'>Information about " + String(PRODINO_MKRZERO) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}