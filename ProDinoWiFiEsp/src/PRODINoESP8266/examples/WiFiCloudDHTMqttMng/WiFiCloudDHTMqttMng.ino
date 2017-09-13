// WiFiCloudDHTMqttMng.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//    Cloud MQTT example with DHT support. In this example we show how to connect KMP ProDino WiFi-ESP WROOM-02 with some MQTT server and measure humidity and temperature with DHT22 sensor.
//    In the example you can set settings through web page. How it works: if device can't find WiFi network automatic change to AP. In web page you can set WiFi and MQTT configuration.
//    If you wish remote management (example from a phone) you can use Amazon cloudmqtt.com service.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wificlouddhtmqttmng.aspx
// Prerequisites: you should install libraries described in #include section below.
// Version: 1.0.1
// Date: 13.09.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <FS.h>
#include <KMPDinoWiFiESP.h>       // Our library. https://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/howtoinstall.aspx
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <stdarg.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>         // Install with Library Manager. "PubSubClient by Nick O'Leary" https://pubsubclient.knolleary.net/
#include <DHT.h>                  // Install with Library Manager. "DHT sensor library by Adafruit" https://github.com/adafruit/DHT-sensor-library
#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Install with Library Manager. "ArduinoJson by Benoit Blanchon" https://github.com/bblanchon/ArduinoJson

const uint8_t MQTT_SERVER_LEN = 40;
const uint8_t MQTT_PORT_LEN = 8;
const uint8_t MQTT_CLIENT_ID_LEN = 32;
const uint8_t MQTT_USER_LEN = 16;
const uint8_t MQTT_PASS_LEN = 16;
const long CHECK_HT_INTERVAL_MS = 10000;

const char* MQTT_SERVER_KEY = "mqttServer";
const char* MQTT_PORT_KEY = "mqttPort";
const char* MQTT_CLIENT_ID_KEY = "mqttClientId";
const char* MQTT_USER_KEY = "mqttUser";
const char* MQTT_PASS_KEY = "mqttPass";
const char* CONFIG_FILE_NAME = "/config.json";

char _mqttServer[MQTT_SERVER_LEN] = "x.cloudmqtt.com";
char _mqttPort[MQTT_PORT_LEN] = "1883";
char _mqttClientId[MQTT_CLIENT_ID_LEN] = "ESP8266Client";
char _mqttUser[MQTT_USER_LEN];
char _mqttPass[MQTT_PASS_LEN];

const char* TOPIC_SEPARATOR = "/";
const char* MAIN_TOPIC = "kmp/prodinowifi";
const char* HUMIDITY_SENSOR = "humidity";
const char* TEMPERATURE_SENSOR = "temperature";
const char* RELAY = "relay";
const char* OPTO_INPUT = "optoin";
const char* SET_COMMAND = "set";

WiFiClient _wifiClient;
PubSubClient _mqttClient;
DHT _dhtSensor(EXT_GROVE_D0, DHT22, 11);
// Contains last measured humidity and temperature from sensor.
float _dht[2];
// Store last measure time.
unsigned long _mesureTimeout;

// There arrays store last states by relay and optical isolated inputs.
bool _lastRelayStatus[4] = { false };
bool _relayStatusChanged[4] = { false };
bool _lastOptoInStatus[4] = { false };

// Buffer by send output state.
char _payload[16];

// Flags for saving data
bool _shouldSaveConfig = false;
bool _forceSendData = true;

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

	Serial.println("KMP Mqtt cloud client example.\r\n");

	//WiFiManager
	//Local initialization. Once it's business is done, there is no need to keep it around.
	WiFiManager wifiManager;

	// Is OptoIn 4 is On the board is resetting WiFi configuration.
	if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
	{
		Serial.println("Resetting WiFi configuration...\r\n");
		//reset saved settings
		wifiManager.resetSettings();
		Serial.println("WiFi configuration was reseted.\r\n");
	}

	//set config save notify callback
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	if (!mangeConnectParamers(&wifiManager))
	{
		return;
	}

	// Initialize MQTT.
	_mqttClient.setClient(_wifiClient);
	uint16_t port = atoi(_mqttPort);
	_mqttClient.setServer(_mqttServer, port);
	_mqttClient.setCallback(callback);
}

/**
* @brief Build topic from parameters.
* @param num Numbers of parameters.
* @param ... values, separated with comma. The values should be from type char*
*
* @return combined text
*/
String buildTopic(int num, ...)
{
	String result = "";
	va_list valist;
	int i;

	/* initialize valist for num number of arguments */
	va_start(valist, num);

	/* access all the arguments assigned to valist */
	for (i = 0; i < num; i++) {
		result += va_arg(valist, char*);
	}

	/* clean memory reserved for valist */
	va_end(valist);

	return result;
}

/**
* @brief Callback method. It is fire when has information in subscribed topics.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {

	printTopicAndPayload("Subscribe", topic, (char*)payload, length);

	size_t mainTopicLen = strlen(MAIN_TOPIC);
	// Topic kmp/prodinowifi - command send all data from device.
	if (strlen(topic) == mainTopicLen &&  strcmp(MAIN_TOPIC, topic) == 0)
	{
		_forceSendData = true;
		return;
	}

	String topicName = String(topic);

	String topicStart = buildTopic(4, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR);
	String topicEnd = buildTopic(2, TOPIC_SEPARATOR, SET_COMMAND);

	// Set new realy status.
	// Topic kmp/prodinowifi/relay/+/set - command which send relay status.
	// + is relay number (0..3), payload is (0 - Off, 1 - On).
	if (topicName.startsWith(topicStart) && topicName.endsWith(topicEnd))
	{
		int relayNumber = CharToInt(topic[topicStart.length()]);
		if (length == 1)
		{
			int relayState = CharToInt(payload[0]);
			KMPDinoWiFiESP.SetRelayState(relayNumber, relayState == 1);
			_relayStatusChanged[relayNumber] = true;
		}
	}
}

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	// By the normal device work need connected with WiFi and MQTT server.
	if (!connectWiFi() || !connectMqtt())
	{
		return;
	}

	_mqttClient.loop();
	
	// Publish information into MQTT.
	publishRelayOptoData(_forceSendData);
	publishDHTSensorData(_forceSendData);
	
	_forceSendData = false;
}

char getState(bool state)
{
	return state ? '1' : '0';
}

/**
* @brief Publish information in the MQTT server.
* @param isForceSend is set to true send all information from device, if false send only changed information.
* 
* @return void
*/
void publishRelayOptoData(bool isForceSend)
{
	char state[2];
	state[1] = '\0';
	// Get current Opto input and relay statuses.
	for (byte i = 0; i < RELAY_COUNT; i++)
	{
		bool rState = KMPDinoWiFiESP.GetRelayState(i);
		if (_lastRelayStatus[i] != rState || _relayStatusChanged[i] || isForceSend)
		{
			_lastRelayStatus[i] = rState;
			_relayStatusChanged[i] = false;

			state[0] = IntToChar(i);
			String topic = buildTopic(5, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR, state); // kmp/prodinowifi/relay/0
			
			state[0] = getState(rState);
			publish(topic.c_str(), state);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool oiState = KMPDinoWiFiESP.GetOptoInState(i);
		if (_lastOptoInStatus[i] != oiState || isForceSend)
		{
			_lastOptoInStatus[i] = oiState;

			state[0] = IntToChar(i);
			String topic = buildTopic(5, MAIN_TOPIC, TOPIC_SEPARATOR, OPTO_INPUT, TOPIC_SEPARATOR, state); // kmp/prodinowifi/optoin/0

			state[0] = getState(oiState);
			publish(topic.c_str(), state);
		}
	}
}

/**
* @brief Read data from sensors a specified time.
* @param isForceSend is set to true send all information from device, if false send only changed information.
*
* @return void
*/
void publishDHTSensorData(bool isForceSend)
{
	// Publish humidity or temperature if is isForceSend or time to send ocurred and value is changed.
	if (millis() > _mesureTimeout || isForceSend)
	{
		_dhtSensor.read(true);
		float humidity = _dhtSensor.readHumidity();
		float temperature = _dhtSensor.readTemperature();

		if (_dht[0]	!= humidity || isForceSend)
		{
			_dht[0] = humidity;
			FloatToChars(humidity, 1, _payload);
			String topic = buildTopic(3, MAIN_TOPIC, TOPIC_SEPARATOR, HUMIDITY_SENSOR); // kmp/prodinowifi/humidity
			publish(topic.c_str(), _payload);
		}

		if (_dht[1] != temperature || isForceSend)
		{
			_dht[1] = temperature;
			FloatToChars(temperature, 1, _payload);
			String topic = buildTopic(3, MAIN_TOPIC, TOPIC_SEPARATOR, TEMPERATURE_SENSOR); // kmp/prodinowifi/temperature
			publish(topic.c_str(), _payload);
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

/**
* @brief Publish topic.
* @param topic title.
* @param payload data to send
*
* @return void
*/
void publish(const char* topic, char* payload)
{
	printTopicAndPayload("Publish", topic, payload, strlen(payload));
	
	_mqttClient.publish(topic, (const char*)payload);
}

/**
* @brief Print debug information about topic and payload.
* @operationName operation which use this data.
* @param topic title.
* @param payload data
*
* @return void
*/
void printTopicAndPayload(const char* operationName, const char* topic, char* payload, unsigned int length)
{
	Serial.print(operationName);
	Serial.print(" topic [");
	Serial.print(topic);
	Serial.print("] payload [");
	for (uint i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println("]");
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

		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	return true;
}

/**
* @brief Connect to MQTT server.
*
* @return bool true - success.
*/
bool connectMqtt()
{
	if (!_mqttClient.connected())
	{
		Serial.println("Attempting MQTT connection...");

		if (_mqttClient.connect(_mqttClientId, _mqttUser, _mqttPass))
		{
			Serial.println("Connected.");
			// Subscribe for topics:
			//  kmp/prodinowifi
			//  kmp/prodinowifi/relay/+/set
			_mqttClient.subscribe(MAIN_TOPIC);
			String topic = buildTopic(7, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR, "+", TOPIC_SEPARATOR, SET_COMMAND);
			_mqttClient.subscribe(topic.c_str());
		}
		else
		{
			Serial.print("failed, rc=");
			Serial.print(_mqttClient.state());
			Serial.println(" try again after 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}

	return _mqttClient.connected();
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
		Serial.println("Mounted file system");

		if (SPIFFS.exists(CONFIG_FILE_NAME))
		{
			//file exists, reading and loading
			Serial.println("reading config file");
			File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
			if (configFile)
			{
				Serial.println("Opening config file");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success())
				{
					Serial.println("\nparsed json");

					strcpy(_mqttServer, json[MQTT_SERVER_KEY]);
					strcpy(_mqttPort, json[MQTT_PORT_KEY]);
					strcpy(_mqttClientId, json[MQTT_CLIENT_ID_KEY]);
					strcpy(_mqttUser, json[MQTT_USER_KEY]);
					strcpy(_mqttPass, json[MQTT_PASS_KEY]);
				}
				else
				{
					Serial.println("failed to load json config");
				}
			}
		}
	}

	// The extra parameters to be configured (can be either global or just in the setup)
	// After connecting, parameter.getValue() will get you the configured value
	// id/name placeholder/prompt default length
	WiFiManagerParameter customMqttServer("server", "MQTT server", _mqttServer, MQTT_SERVER_LEN);
	WiFiManagerParameter customMqttPort("port", "MQTT port", String(_mqttPort).c_str(), MQTT_PORT_LEN);
	WiFiManagerParameter customClientName("clientName", "Client name", _mqttClientId, MQTT_CLIENT_ID_LEN);
	WiFiManagerParameter customMqttUser("user", "MQTT user", _mqttUser, MQTT_USER_LEN);
	WiFiManagerParameter customMqttPass("password", "MQTT pass", _mqttPass, MQTT_PASS_LEN);

	//add all your parameters here
	wifiManager->addParameter(&customMqttServer);
	wifiManager->addParameter(&customMqttPort);
	wifiManager->addParameter(&customClientName);
	wifiManager->addParameter(&customMqttUser);
	wifiManager->addParameter(&customMqttPass);

	//fetches ssid and pass from eeprom and tries to connect
	//if it does not connect it starts an access point with the specified name
	//auto generated name ESP + ChipID
	if (!wifiManager->autoConnect())
	{
		Serial.println("Doesn't connect");
		return false;
	}

	//if you get here you have connected to the WiFi
	Serial.println("Connected.");

	if (_shouldSaveConfig)
	{
		Serial.println("Saving config");

		//read updated parameters
		strcpy(_mqttServer, customMqttServer.getValue());
		strcpy(_mqttPort, customMqttPort.getValue());
		strcpy(_mqttClientId, customClientName.getValue());
		strcpy(_mqttUser, customMqttUser.getValue());
		strcpy(_mqttPass, customMqttPass.getValue());

		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.createObject();

		json[MQTT_SERVER_KEY] = _mqttServer;
		json[MQTT_PORT_KEY] = _mqttPort;
		json[MQTT_CLIENT_ID_KEY] = _mqttClientId;
		json[MQTT_USER_KEY] = _mqttUser;
		json[MQTT_PASS_KEY] = _mqttPass;

		File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
		if (!configFile) {
			Serial.println("failed to open config file for writing");
		}

		json.prettyPrintTo(Serial);
		json.printTo(configFile);
		configFile.close();
	}

	return true;
}