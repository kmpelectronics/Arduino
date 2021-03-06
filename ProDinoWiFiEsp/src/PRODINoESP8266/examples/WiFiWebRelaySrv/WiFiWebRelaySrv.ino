// WiFiWebRelaySrv.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Web relay example presents how to manipulate relays through web interface. 
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.1.0
// Date: 02.02.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//  You have to fill your credentials in arduino_secrets.h file

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "arduino_secrets.h"

ESP8266WebServer _server(80);

const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

/**
 * @brief Execute first after start device. Initialize hardware.
 *
 * @return void
 */
void setup(void)
{
	// You can open the Arduino IDE Serial Monitor window to see what the code is doing
	// Serial connection from ESP-01 via 3.3v console cable
	Serial.begin(115200);
	// Init KMP Dino WiFi board.
	KMPDinoWiFiESP.init();

	// Connect to WiFi network
	WiFi.begin(SSID_NAME, SSID_PASSWORD);
	Serial.print("\n\r \n\rWorking to connect");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("KMP Relay Server");
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
	// Read POST request.
	// Have only one argument. r1=On ...
	if (_server.method() == HTTP_POST && _server.args() > 1)
	{
		// Check argument name
		String argName = _server.argName(0);

		// Check is valid value: r1, r2 ...
		if (argName.length() == 2)
		{
			int relayNumber = CharToInt(argName[1]) - 1;
			Serial.print("Relay number: ");
			Serial.println(relayNumber);
			if (relayNumber >= 0 && relayNumber < RELAY_COUNT)
			{
				// Get value first argument.
				String argValue = _server.arg(0);

				Serial.print("Argument value: ");
				Serial.println(argValue);

				if (argValue == W_ON || argValue == W_OFF)
				{
					KMPDinoWiFiESP.SetRelayState((uint8_t)relayNumber, argValue == W_ON);
				}
			}
		}
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
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_WIFI) + " - Web Relay</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_WIFI) + " - Web Relay example</h1>"
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
		if (KMPDinoWiFiESP.GetRelayState(i))
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
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_WIFI) + "' target='_blank'>Information about " + String(PRODINO_WIFI) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}