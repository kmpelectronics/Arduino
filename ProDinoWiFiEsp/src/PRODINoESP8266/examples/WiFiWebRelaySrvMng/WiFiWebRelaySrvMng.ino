// WiFiWebRelaySrv.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		A Web server which support manipulation relays statuses example.
//      In the example you can set settings through a web page. How it works: if device can't connect WiFi network automatic, change to AP. 
//      In a web page you can set WiFi configuration. If this configuration is valid, the device connect to a WiFi.
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinowifiesp/wifiwebrelayserver.aspx
// Version: 1.0.0
// Date: 21.09.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <FS.h>
#include <KMPDinoWiFiESP.h>          // Our library. https://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/howtoinstall.aspx
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Install with Library Manager. "ArduinoJson by Benoit Blanchon" https://github.com/bblanchon/ArduinoJson

const char* CONFIG_FILE_NAME = "/config.json";
const uint8_t HTTP_PORT = 80;

ESP8266WebServer _server(HTTP_PORT);
WiFiClient _wifiClient;

const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

// Flags for saving data
bool _shouldSaveConfig = false;

/**
 * @brief Execute first after start a device. Initialize hardware.
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

	Serial.println("KMP Mqtt cloud client example.\r\n");

	//WiFiManager
	//Local initialization. Once it's business is done, there is no need to keep it around.
	WiFiManager wifiManager;

	//set config save notify callback
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	if (!mangeConnectParamers(&wifiManager))
	{
		return;
	}
}

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	if (!connectWiFi())
	{
		return;
	}

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

/**
* @brief Starting a server.
*
* @return void
*/
void startServer()
{
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	_server.on("/", HandleRootPage);
	_server.begin();

	Serial.println("HTTP server started");
}

/**
* @brief Connect to WiFi access point.
*
* @return bool true - success.
*/
bool connectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.print("Reconnecting [");
		Serial.print(WiFi.SSID());
		Serial.println("]...");

		WiFi.begin();

		if (WiFi.waitForConnectResult() != WL_CONNECTED)
		{
			return false;
		}
		
		startServer();
	}

	return true;
}

/**
* @brief Callback notifying us of the need to save configuration set from WiFiManager.
*
* @return void.
*/
void saveConfigCallback()
{
	Serial.println("Should save config");
	_shouldSaveConfig = true;
}

/**
* @brief Setting information for connect WiFi and MQTT server. After successful connected this method save them.
* @param wifiManager.
*
* @return bool if successful connected - true else false.
*/
bool mangeConnectParamers(WiFiManager* wifiManager)
{
	//read configuration from FS json
	Serial.println("Mounting FS...");

	if (!SPIFFS.begin())
	{
		Serial.println("Failed to mount FS");
	}
	else
	{
		Serial.println("The file system is mounted.");

		if (SPIFFS.exists(CONFIG_FILE_NAME))
		{
			//file exists, reading and loading
			Serial.println("Reading configuration file");
			File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
			if (configFile)
			{
				Serial.println("Opening configuration file");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success())
				{
					Serial.println("\nJson is parsed");
				}
				else
				{
					Serial.println("Loading json configuration is failed");
				}
			}
		}
	}

	// fetches ssid and pass from eeprom and tries to connect
	// if it does not connect it starts an access point with the specified name
	// auto generated name ESP + ChipID
	if (!wifiManager->autoConnect())
	{
		Serial.println("Doesn't connect.");
		return false;
	}

	//if you get here you have connected to the WiFi
	Serial.println("Connected.");

	if (_shouldSaveConfig)
	{
		Serial.println("Saving configuration...");

		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.createObject();

		File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
		if (!configFile) {
			Serial.println("Failed to open a configuration file for writing.");
		}
		else
		{
			Serial.println("Configuration is saved.");
		}

		json.prettyPrintTo(Serial);
		json.printTo(configFile);
		configFile.close();
	}

	startServer();

	return true;
}