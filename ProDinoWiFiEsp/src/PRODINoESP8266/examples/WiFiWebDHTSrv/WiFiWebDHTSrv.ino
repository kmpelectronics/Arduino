// WiFiWebDHTSrv.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//		Web server example with DHT sensor.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebdhtserver.aspx
// Version: 1.1.0
// Date: 02.02.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//  You have to fill your credentials in arduino_secrets.h file
//		Before start this example you need to install DHT library: https://github.com/adafruit/DHT-sensor-library
//		Connect DHT22 sensor to External GROVE connector. Use pins: 
//			- first sensor EXT_GROVE_D0, Vcc+, Gnd(-);

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include "arduino_secrets.h"

DHT _dht(EXT_GROVE_D0, DHT22, 11);

ESP8266WebServer _server(80);

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

	// Init KMP ProDino WiFi-ESP board.
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
	Serial.println("KMP DHT Server");
	Serial.print("Connected to ");
	Serial.println(SSID_NAME);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	_server.on("/", HandleRootPage);
	_server.begin();

	_dht.begin();
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
	//KMPDinoWiFiESP.LedOn();

	_server.send(200, TEXT_HTML, BuildPage());

	//KMPDinoWiFiESP.LedOff();
}

/**
 * @brief Build HTML page.
 *
 * @return void
 */
String BuildPage()
{
	String page =
		"<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_WIFI) + " - Web DHT</title></head>"
		+ "<body><div style='text-align: center'>"
		+ "<br><hr />"
		+ "<h1 style = 'color: #0066FF;'>" + String(PRODINO_WIFI) + " - Web DHT example</h1>"
		+ "<hr /><br><br>"
		+ "<table border='1' width='450' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ "<thead><tr><th style='width:30%'></th><th style='width:35%'>Temperature C&deg;</th><th>Humidity</th></tr></thead>";

	// Add table rows, relay information.
	String tableBody = "<tbody>";
	// Row i, cell 1
	_dht.read(true);
	_dht.readHumidity();
	_dht.readTemperature();

	tableBody += "<tr><td></td>";

	// Cell i,2
	tableBody += "<td>" + String(_dht.readTemperature()) + "</td>";

	// Cell i,3
	tableBody += "<td>" + String(_dht.readTemperature()) + "</td></tr>";

	tableBody += "</tbody>";

	return page + tableBody
		+ "</table><br><br><hr /><h1><a href='" + String(URL_KMPELECTRONICS_EU) + "' target='_blank'>Visit " + String(KMP_ELECTRONICS_LTD) + "</a></h1>"
		+ "<h3><a href='" + String(URL_KMPELECTRONICS_EU_DINO_WIFI) + "' target='_blank'>Information about " + String(PRODINO_WIFI) + " board</a></h3>"
		+ "<hr /></div></body></html>";
}