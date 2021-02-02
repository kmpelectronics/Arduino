// WiFiWebOptoInSrv.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Web server example present how we can work with Opto inputs. 
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebdhtserver.aspx
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
	
	WiFi.mode(WIFI_STA);

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
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_WIFI) + " - Opto inputs</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_WIFI) + " - Opto inputs example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>";

	String tableBody = "<tbody><tr>";
	String tableHeader = "<thead><tr>";
	for (uint8_t i = 0; i < OPTOIN_COUNT; i++)
	{
		String optoInNumber = String(i + 1);
		tableHeader += "<th>In " + optoInNumber + "</th>";

		char* cellColor;
		char* cellStatus;
		if (KMPDinoWiFiESP.GetOptoInState(i))
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

	tableHeader += "</tr></thead>";
	tableBody += "</tr></tbody>";

	return page + tableHeader + tableBody +
		+ "</table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_WIFI) + "' target='_blank'>Information about " + String(PRODINO_WIFI) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}