// EthWebRS485.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		Web server RS485 example. 
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinowifiesp/wifiwebrelayserver.aspx
// Version: 1.0.0
// Date: 01.07.2016
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//		Before start this example you need:
//		Connect RS485 (make echo, and configured 19200, 8N1) a device in ProDino RS485 port.
// Attention:
//		The Serial (debug port) and RS485 port is same.

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

// If Post request is valid, read data from RS485.
bool _isValidPost = false;
// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
* @brief Execute first after start device. Initialize hardware.
*
* @return void
*/
void setup()
{
#ifdef DEBUG
	Serial.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init(true);

	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoMKRZero.RS485Begin(19200);
	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	Serial.println("The server is starting.");
	Serial.println(Ethernet.localIP());
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
#endif
}

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	// Listen for incoming clients.
	_client = _server.available();

	if (!_client.connected())
	{
		return;
	}

#ifdef DEBUG
	Serial.println("Client connected.");
#endif

	// If client connected On status led.
	KMPProDinoMKRZero.OnStatusLed();

	// Read client request.
	_isValidPost = ReadClientRequest();
	
	// Write client response - html page.
	WriteClientResponse();

	// Close the connection.
	_client.stop();

	// If client disconnected Off status led.
	KMPProDinoMKRZero.OffStatusLed();

#ifdef DEBUG
	Serial.println("Client disconnected.");
	Serial.println("---");
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
		Serial.write(c);
#endif
		client->read();
	}

	// Nothing for read.
	return false;
}

/*
POST / HTTP/1.1
Host: 192.168.0.177
Connection: keep-alive
Content-Length: 5

data=test&btn=Transmit
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

	// Check is post request.
	if (!firstRow.startsWith("POST"))
	{
		return false;
	}
	
	// Get data
	int equalPos = lastRow.indexOf('=');
	int ampersantPos = lastRow.indexOf('&');

	if (equalPos > 0 && ampersantPos > equalPos)
	{
		String data = lastRow.substring(equalPos + 1, ampersantPos);
#ifdef DEBUG
		Serial.println("--data to send--");
		Serial.println(data);
#endif
		// Send data with RS485.
		KMPProDinoMKRZero.RS485Write(data.c_str());
	}

#ifdef DEBUG
	Serial.println("<< End client request.");
#endif

	return true;
}

/**
* \brief WriteClientResponse void. Write html response.
*
* \return void
*/
void WriteClientResponse()
{
#ifdef DEBUG
	Serial.println("Write client response.");
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
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_MKRZERO) + " - RS485</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_MKRZERO) + " - RS485 example</h1>"
		+ "<hr /><br><br>"
		+ "<form method='post'>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th width='80%'>Data</th><th>Action</th></tr></thead>"
		+ "<tbody><tr><td><input type='text' name='data' style='width: 100%'></td>"
		+ "<td><input type='submit' name='btn' value='Transmit'/></td></tr>"
		+ "<tr><td>";

	if (_isValidPost)
	{
#ifdef DEBUG
		Serial.println("Read data from RS485.");
		int readBytes = 0;
#endif
		// Read RS485.
		int i;

		// if i = -1 not data to read.
		while ((i = KMPProDinoMKRZero.RS485Read()) > -1)
		{
#ifdef DEBUG
			Serial.write((char)i);
			readBytes++;
#endif
			// Convert unsigned char to char.
			page += (char)i;
		}
#ifdef DEBUG
		Serial.println();
		Serial.print("Bytes read: ");
		Serial.println(readBytes);
#endif
	}

	return page 
		+ "</td><td>Received</td></tr></tbody>"
		+ "</table></form>"
		+ "</table></form><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_PRODINO_MKRZERO) + "' target='_blank'>Information about " + String(PRODINO_MKRZERO) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}