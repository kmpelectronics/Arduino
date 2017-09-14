// WiFiFanCoilMqttMng.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
// Example link: 
// Prerequisites: 
//    You should install libraries described in #include section below.
// Version: 0.0.1
// Date: 13.09.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Attention: The project has not finished yet!

#include <FS.h>
#include <KMPDinoWiFiESP.h>       // Our library. https://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/howtoinstall.aspx
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <stdarg.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiFanCoilMqttMngParams.h"
#include <PubSubClient.h>         // Install with Library Manager. "PubSubClient by Nick O'Leary" https://pubsubclient.knolleary.net/
#include <DHT.h>                  // Install with Library Manager. "DHT sensor library by Adafruit" https://github.com/adafruit/DHT-sensor-library
#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Install with Library Manager. "ArduinoJson by Benoit Blanchon" https://github.com/bblanchon/ArduinoJson

char _mqttServer[MQTT_SERVER_LEN] = "x.cloudmqtt.com";
char _mqttPort[MQTT_PORT_LEN] = "1883";
char _mqttClientId[MQTT_CLIENT_ID_LEN] = "ESP8266Client";
char _mqttUser[MQTT_USER_LEN];
char _mqttPass[MQTT_PASS_LEN];
char _baseTopic[BASE_TOPIC_LEN] = "flat/bedroom1";

WiFiClient _wifiClient;
PubSubClient _mqttClient;
DHT _dhtSensor(EXT_GROVE_D0, DHT22, 11);
// Contains last measured humidity and temperature from the sensor.
float _dht[2];
// Store last measure time.
unsigned long _mesureTimeout;

// Text buffers for topic and payload.
char _topic[128];
char _payload[16];

// Flags for saving data
bool _shouldSaveConfig = false;
bool _forceSendAllData = true;

Mode _mode = Cold;

/**
* @brief Execute first after start the device. Initialize hardware.
*
* @return void
*/
void setup(void)
{
	// You can open the Arduino IDE Serial Monitor window to see what the code is doing
	Serial.begin(115200);
	// Init KMP ProDino WiFi-ESP board.
	KMPDinoWiFiESP.init();

	Serial.println("KMP fan coil management with Mqtt.\r\n");

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

	// Set save configuration callback.
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
	publishRelayOptoData(_forceSendAllData);
	publishDHTSensorData(_forceSendAllData);
	
	_forceSendAllData = false;
}

/**
* @brief Build topic from parameters.
* @param num Numbers of parameters.
* @param ... values, separated with comma. The values should be of type char*
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
* @brief Build topic from parameters.
* @param buffer In it combine parameters.
* @param params Parameters list.
* @param count Parameters count.
*
* @return void
*/
void buildTopic(char* buffer, const char* fragments[], const uint8_t count)
{
	for (uint8_t i = 0; i < count; i++)
	{
		int paramLen = strlen(fragments[i]);
		memcpy(buffer, fragments[i], paramLen);
		buffer += paramLen;
	}
	buffer++;
	*buffer = '\0';
}

/**
* @brief Callback method. It is fire when has information in subscribed topics.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {

	printTopicAndPayload("Subscribe", topic, (char*)payload, length);

	size_t mainTopicLen = strlen(_baseTopic);
	// Processing topic basetopic - command send all data from device.
	if (strlen(topic) == mainTopicLen &&  strcmp(_baseTopic, topic) == 0)
	{
		_forceSendAllData = true;
		return;
	}

	String topicContent = String(topic);
	// Remove prefix  basetopic/
	if (topicContent.length > mainTopicLen)
	{
		topicContent.remove(0, mainTopicLen + 1);
	}
	// All other topics finished with /set
	String topicEnd = buildTopic(2, TOPIC_SEPARATOR, SET_COMMAND);
	if (!topicContent.endsWith(topicEnd))
	{
		return;
	}
	// Remove /set
	topicContent.remove(topicContent.length - topicEnd.length, topicEnd.length);

	// Processing topic basetopic/mode/set: heat/cold
	if (topicContent.equals(MODE_COMMAND))
	{
		if (strncmp((char*)payload, HEAT_VALUE, strlen(HEAT_VALUE)) == 0)
		{
			_mode = Heat;
			_forceSendAllData = true;
			return;
		}

		if (strncmp((char*)payload, COLD_VALUE, strlen(COLD_VALUE)) == 0)
		{
			_mode = Cold;
			_forceSendAllData = true;
			return;
		}
	}

	// Processing topic basetopic/temperature/set: 22.5
	// Processing topic basetopic/state/set: on, off

	String topicStart = buildTopic(4, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR);

	// Set new realy status.
	// Topic kmp/prodinowifi/relay/+/set - command which send relay status.
	// + is relay number (0..3), payload is (0 - Off, 1 - On).
	if (topicContent.startsWith(topicStart) && topicContent.endsWith(topicEnd))
	{
		int relayNumber = CharToInt(topic[topicStart.length()]);
		if (length == 1)
		{
			int relayState = CharToInt(payload[0]);
			KMPDinoWiFiESP.SetRelayState(relayNumber, relayState == 1);
			_lastStatus[1][relayNumber] = true;
		}
	}
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
		if (_lastStatus[0][i] != rState || _lastStatus[1][i] || isForceSend)
		{
			_lastStatus[0][i] = rState;
			_lastStatus[1][i] = false;

			state[0] = IntToChar(i);
			String topic = buildTopic(5, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR, state); // kmp/prodinowifi/relay/0
			
			state[0] = getState(rState);
			publish(topic.c_str(), state);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool oiState = KMPDinoWiFiESP.GetOptoInState(i);
		if (_lastStatus[2][i] != oiState || isForceSend)
		{
			_lastStatus[2][i] = oiState;

			state[0] = IntToChar(i);
			String topic = buildTopic(5, MAIN_TOPIC, TOPIC_SEPARATOR, OPTO_INPUT, TOPIC_SEPARATOR, state); // kmp/prodinowifi/optoin/0

			state[0] = getState(oiState);
			publish(topic.c_str(), state);
		}
	}
}

/**
* @brief Read data from a sensor for humidity and temperature.
* @param isForceSend Is set to true send humidity and temperature, if false send only changed data on occurred _mesureTimeout.
*
* @return void
*/
void publishDHTSensorData(bool isForceSend)
{
	// Publish humidity or temperature if is isForceSend or time to send occurred and value is changed.
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
* @param topic A topic title.
* @param payload A data to send.
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
* @operationName Operation which use this data.
* @param topic The topic name.
* @param payload The payload data.
* @param length A payload length.
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
			_mqttClient.subscribe(MAIN_TOPIC);
			//  kmp/prodinowifi/relay/+/set
			String topic = buildTopic(7, MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR, "+", TOPIC_SEPARATOR, SET_COMMAND);
			// TODO: New implementation, should be test it.
			const char * params[] = { MAIN_TOPIC, TOPIC_SEPARATOR, RELAY, TOPIC_SEPARATOR, "+", TOPIC_SEPARATOR, SET_COMMAND };
			buildTopicNew(_topic, params, 7);
			Serial.print("buildTopicNew: ");
			Serial.println(_topic);

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
				//json.printTo(Serial);
				if (json.success())
				{
					Serial.println("\nJson is parsed");

					strcpy(_mqttServer, json[MQTT_SERVER_KEY]);
					strcpy(_mqttPort, json[MQTT_PORT_KEY]);
					strcpy(_mqttClientId, json[MQTT_CLIENT_ID_KEY]);
					strcpy(_mqttUser, json[MQTT_USER_KEY]);
					strcpy(_mqttPass, json[MQTT_PASS_KEY]);
				}
				else
				{
					Serial.println("Loading json configuration is failed");
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

	// add all your parameters here
	wifiManager->addParameter(&customMqttServer);
	wifiManager->addParameter(&customMqttPort);
	wifiManager->addParameter(&customClientName);
	wifiManager->addParameter(&customMqttUser);
	wifiManager->addParameter(&customMqttPass);

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

	return true;
}