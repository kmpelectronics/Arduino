// EthWebRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino MKR Zero (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		Web server over Ethernet Relay manipulation.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebdhtserver.aspx
// Version: 1.0.0
// Date: 19.04.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

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

// Define text.
const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
* \brief Setup void. Arduino executed first. Initialize DiNo board and prepare Ethernet connection.
*
*
* \return void
*/
void setup()
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
}

/**
* \brief Loop void. Arduino executed second.
*
*
* \return void
*/
void loop() {
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
	KMPProDinoMKRZero.OnStatusLed();

	// Read client request.
	if (ReadClientRequest())
	{
		// Write client response - html page.
		//WriteClientResponse();
	}
#ifdef DEBUG
	else
	{
		SerialUSB.println("#Request not processed.");
	}
#endif
	
	WriteClientResponse();

	// Close the connection.
	_client.stop();

	// If client disconnected Off status led.
	KMPProDinoMKRZero.OffStatusLed();

#ifdef DEBUG
	SerialUSB.println("Client disconnected.");
	SerialUSB.println("---");
#endif
}

bool ReadHttpRequestLine(EthernetClient* client, String* line)
{
	*line = "";

	if (client == NULL)
	{
		return false;
	}

	bool isCRLF = false;
	int c;
	while ((c = client->peek()) > -1)
	{
		if (c == CH_CR || c == CH_LF)
		{
			isCRLF = true;
		}
		else
		{
			// The line finished and next char isn't CH_CR or CH_LF. 
			if (isCRLF)
			{
				return true;
			}

			*line += (char)c;
		}
#ifdef DEBUG
		SerialUSB.write(c);
#endif
		client->read();
	}

	// Nothing for read.
	return false;
}

/**
* \brief ReadClientRequest void. Read and parse client request.
* 	First row of client request is similar to:
*		GET / HTTP/1.1
*  -or-
*		GET /?r1=On HTTP/1.1
*		GET /?r1=Off HTTP/1.1
*      |    | | | |
*      1    6 8 10 12
*               relay status
* You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
*
* \return bool Returns true if the request was expected.
*/

/*
POST / HTTP/1.1
Host: 192.168.0.177
Connection: keep-alive
Content-Length: 5

r1=On
*/
bool ReadClientRequest()
{
#ifdef DEBUG
	SerialUSB.println(">> Starts client request.");
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
	SerialUSB.println("--firstRow--");
	SerialUSB.println(firstRow);
	SerialUSB.println("--lastRow--");
	SerialUSB.println(lastRow);
#endif

	// Check is post request.
	if (!firstRow.startsWith("POST") || lastRow.length() < 2)
	{
#ifdef DEBUG
		SerialUSB.println("!! Does not prosess client request.");
#endif
		return false;
	}

	uint8_t relay = CharToInt(lastRow[1]) - 1;
	bool newState = lastRow.endsWith(W_ON);

#ifdef DEBUG
	SerialUSB.println("relay");
	SerialUSB.println(relay);
	SerialUSB.println(newState);
#endif

	KMPProDinoMKRZero.SetRelayState(relay, newState);

#ifdef DEBUG
	SerialUSB.println("<< End client request.");
#endif

	return true;
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
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - Web Relay</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - Web Relay example</h1>"
		+ "<hr /><br><br>"
		+ "<form method='post'>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>";

	// Add table rows, relay information.
	String rows = "";
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		// Row i, cell 1
		String relayNumber = String(i + 1);
		rows += "<tr><td>Relay " + relayNumber + "</td>";

		char* cellColor;
		char* cellStatus;
		char* nextRelayStatus;
		if (KMPProDinoMKRZero.GetRelayState(i))
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

	return page + rows
		+ "</table></form><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_ZERO) + "' target='_blank'>Information about " + String(PRODINO_MKRZERO) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}
