// WiFiWebRS485.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Web server RS485 example. 
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.1.0
// Date: 02.02.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//  You have to fill your credentials in arduino_secrets.h file
//		Connect RS485 (make echo, and configured 19200, 8N1) a device in ProDino RS485 port.
// Attention:
//		The Serial (debug port) and RS485 port are the same.

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "arduino_secrets.h"

//#define DEBUG

ESP8266WebServer _server(80);

// If Post request is valid, read data from RS485.
bool _isValidPost = false;

/**
* @brief Execute first after start device. Initialize hardware.
*
* @return void
*/
void setup()
{
	// Init KMP Dino WiFi board.
	KMPDinoWiFiESP.init();

	// Start RS485 with baud 19200 and 8N1.
	KMPDinoWiFiESP.RS485Begin(19200);

	// Connect to WiFi network
	WiFi.begin(SSID_NAME, SSID_PASSWORD);
	Serial.print("\n\r \n\rWorking to connect");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("KMP RS485 Server");
	Serial.print("Connected to ");
	Serial.println(SSID_NAME);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	_server.on("/", HandleRootPage);
	_server.begin();
}


/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	_server.handleClient();
}

/**
* @brief Handle root page "/".
*
* @return void
*/
void HandleRootPage()
{
	_isValidPost = false;

#ifdef DEBUG
	Serial.println("Request send.");
#endif

	// Read POST request. data=Test&btn=Transmit
	if (_server.method() == HTTP_POST && _server.args() > 1)
	{
		_isValidPost = true;

		// Get value first argument.
		String argValue = _server.arg(0);

#ifdef DEBUG
		Serial.println("Data to send:");
		Serial.println(argValue);
#endif
		// Send data with RS485.
		KMPDinoWiFiESP.RS485Write((char *)argValue.c_str());
	}

	_server.send(200, TEXT_HTML, BuildPage());
}

/**
* @brief Build HTML page.
*
* @return void
*/
String BuildPage()
{
	String page =
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_WIFI) + " - RS485</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_WIFI) + " - RS485 example</h1>"
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
		while ((i = KMPDinoWiFiESP.RS485Read()) > -1)
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
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_WIFI) + "' target='_blank'>Information about " + String(PRODINO_WIFI) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}