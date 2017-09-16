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
#include "WiFiFanCoilMqttMngHelper.h"
#include <PubSubClient.h>         // Install with Library Manager. "PubSubClient by Nick O'Leary" https://pubsubclient.knolleary.net/
#include <DHT.h>                  // Install with Library Manager. "DHT sensor library by Adafruit" https://github.com/adafruit/DHT-sensor-library
#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Install with Library Manager. "ArduinoJson by Benoit Blanchon" https://github.com/bblanchon/ArduinoJson

float FAN_SWITCH_LEVEL[FAN_SWITCH_LEVEL_LEN] = { 0, 0.5, 1.0 };

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

// Text buffers for topic and payload.
char _topicBuff[128];
char _payloadBuff[32];

// Flags for saving data
bool _shouldSaveConfig = false;
bool _initialSendData = true;

Mode _mode = Cold;
DeviceState _deviceState = Off;

float _desiredTemperature = 22.0;
float _currentTemperature;

float _tempCollection[TEMPERATURE_ARRAY_LEN] = { 22.0 };
uint8_t _tempCollectPos = 0;
// Store last measure time.
unsigned long _mesureTempTime;
uint8_t _fanDegree = 0;

/**
* @brief Execute first after start the device. Initialize hardware.
*
* @return void
*/
void setup(void)
{
	// You can open the Arduino IDE Serial Monitor window to see what the code is doing
	DEBUG_PRINTER.begin(115200);
	// Init KMP ProDino WiFi-ESP board.
	KMPDinoWiFiESP.init();

	DEBUG_PRINTLN("KMP fan coil management with Mqtt.\r\n");

	//WiFiManager
	//Local initialization. Once it's business is done, there is no need to keep it around.
	WiFiManager wifiManager;

	// Is OptoIn 4 is On the board is resetting WiFi configuration.
	if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
	{
		DEBUG_PRINTLN("Resetting WiFi configuration...\r\n");
		//reset saved settings
		wifiManager.resetSettings();
		DEBUG_PRINTLN("WiFi configuration was reseted.\r\n");
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
	
	collectTemperature();
	processTemperature();
	fanCoilControl();

	// Publish information into MQTT.
	if (_initialSendData)
	{
		DeviceData deviceData = (DeviceData)(CurrentTemp | DesiredTemp | FanDegree | CurrentMode | CurrentDeviceState | DeviceIsStarted);
		publishData(deviceData);
		_initialSendData = false;
	}
}

/**
* @brief Callback method. It is fire when has information in subscribed topics.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {

	printTopicAndPayload("Subscribe", topic, (char*)payload, length);

	size_t baseTopicLen = strlen(_baseTopic);

	if (!startsWith(topic, _baseTopic))
	{
		return;
	}

	// Processing basetopic - command send all data from device.
	if (strlen(topic) == baseTopicLen && length == 0)
	{
		DeviceData deviceData = (DeviceData)(CurrentTemp | DesiredTemp | FanDegree | CurrentMode | CurrentDeviceState );
		publishData(deviceData);
		return;
	}

	// Remove prefix basetopic/
	removeStart(topic, baseTopicLen + 1);

	// All other topics finished with /set
	strConcatenate(_topicBuff, 2, TOPIC_SEPARATOR, TOPIC_SET);

	if (!endsWith(topic, _topicBuff))
	{
		return;
	}

	// Remove /set
	removeEnd(topic, strlen(_topicBuff));

	// Processing topic basetopic/mode/set: heat/cold
	if (isEqual(topic, TOPIC_MODE))
	{
		if (isEqual((char*)payload, PAYLOAD_HEAT))
		{
			_mode = Heat;
			publishData(CurrentMode);
			//SaveConfiguration();
			return;
		}

		if (isEqual((char*)payload, PAYLOAD_COLD))
		{
			_mode = Cold;
			publishData(CurrentMode);
			//SaveConfiguration();
			return;
		}
	}

	// Processing topic basetopic/desiredtemp/set: 22.5
	if (isEqual(topic, TOPIC_DESIRED_TEMPERATURE))
	{
		memcpy(_topicBuff, payload, length);
		_topicBuff[length] = CH_NONE;
		float temp = atoff(_topicBuff);
		_desiredTemperature = roundF(temp, TEMPERATURE_PRECISION);
		
		publishData(CurrentMode);
		return;
	}

	// Processing topic basetopic/state/set: on, off
	if (isEqual(topic, TOPIC_DEVICE_STATE))
	{
		memcpy(_topicBuff, payload, length);
		_topicBuff[length] = CH_NONE;

		if (isEqual(_topicBuff, PAYLOAD_ON))
		{
			_deviceState = On;
			publishData(CurrentDeviceState);
			//SaveConfiguration();
			return;
		}
		
		if (isEqual(_topicBuff, PAYLOAD_OFF))
		{
			_deviceState = Off;
			publishData(CurrentDeviceState);
			//SaveConfiguration();
			return;
		}
	}
}

void collectTemperature()
{
	if (millis() > _mesureTempTime)
	{
		if (_tempCollectPos == TEMPERATURE_ARRAY_LEN)
		{
			_tempCollectPos = 0;
		}

		_tempCollection[_tempCollectPos++] = _dhtSensor.readTemperature();

		// Set next time to read data.
		_mesureTempTime = millis() + CHECK_HT_INTERVAL_MS;
	}
}

void processTemperature()
{
	// Get average value.
	double temp = 0.0;
	for (uint8_t i = 0; i < TEMPERATURE_ARRAY_LEN; i++)
	{
		temp += _tempCollection[i];
	}

	temp /= TEMPERATURE_ARRAY_LEN;

	temp = roundF(temp, TEMPERATURE_PRECISION);

	if (_currentTemperature != temp)
	{
		_currentTemperature = temp;
		publishData(CurrentTemp);
	}
}

void fanCoilControl()
{
	if (_deviceState == Off)
	{
		if (_fanDegree != 0)
		{
			setFanDegree(0);
		}

		return;
	}

	float diffTemp = _mode == Cold ? _currentTemperature - _desiredTemperature /* Cold */ : _desiredTemperature - _currentTemperature /* Heat */;

	// Bypass the fan coil.
	if (diffTemp < -1.0)
	{
		// TODO: Add bypass logic.
	}

	uint8_t degree = FAN_SWITCH_LEVEL_LEN;
	for (uint8_t i = 0; i < FAN_SWITCH_LEVEL_LEN; i++)
	{
		if (diffTemp <= FAN_SWITCH_LEVEL[i])
		{
			degree = i;
			break;
		}
	}

	setFanDegree(degree);
}

/*
* A fan degree: 0 - stopped, 1 - low fan speed, 2 - medium, 3 - high
*/
void setFanDegree(uint degree)
{
	if (degree == _fanDegree)
	{
		return;
	}

	// Switch relay off.
	if (_fanDegree != 0)
	{
		KMPDinoWiFiESP.SetRelayState(_fanDegree - 1, false);
	}

	delay(100);
	
	KMPDinoWiFiESP.SetRelayState(_fanDegree - 1, true);

	_fanDegree = degree;
	publishData(FanDegree);
}

void publishData(DeviceData deviceData)
{
	if (CHECK_ENUM(deviceData, CurrentTemp))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_CURRENT_TEMPERATURE);
		FloatToChars(_currentTemperature, TEMPERATURE_PRECISION, _payloadBuff);

		mqttPublish(_topicBuff, _payloadBuff);
	}

	if (CHECK_ENUM(deviceData, FanDegree))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_FAN_DEGREE);
		IntToChars(_fanDegree, _payloadBuff);

		mqttPublish(_topicBuff, _payloadBuff);
	}

	if (CHECK_ENUM(deviceData, DesiredTemp))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_DESIRED_TEMPERATURE);
		FloatToChars(_desiredTemperature, TEMPERATURE_PRECISION, _payloadBuff);

		mqttPublish(_topicBuff, _payloadBuff);
	}

	if (CHECK_ENUM(deviceData, CurrentMode))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_MODE);

		const char * mode = _mode == Cold ? PAYLOAD_COLD : PAYLOAD_HEAT;

		mqttPublish(_topicBuff, (char*)mode);
	}

	if (CHECK_ENUM(deviceData, CurrentDeviceState))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_DEVICE_STATE);

		const char * mode = _deviceState == On ? PAYLOAD_ON : PAYLOAD_OFF;

		mqttPublish(_topicBuff, (char*)mode);
	}

	if (CHECK_ENUM(deviceData, DeviceIsStarted))
	{
		mqttPublish(_baseTopic, (char*)PAYLOAD_STARTED);
	}
}

/**
* @brief Publish topic.
* @param topic A topic title.
* @param payload A data to send.
*
* @return void
*/
void mqttPublish(const char* topic, char* payload)
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
	DEBUG_PRINT(operationName);
	DEBUG_PRINT(" topic [");
	DEBUG_PRINT(topic);
	DEBUG_PRINT("] payload [");
	for (uint i = 0; i < length; i++)
	{
		DEBUG_PRINT((char)payload[i]);
	}
	DEBUG_PRINTLN("]");
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
		DEBUG_PRINT("Reconnecting [");
		DEBUG_PRINT(WiFi.SSID());
		DEBUG_PRINTLN("]...");

		WiFi.begin();

		if (WiFi.waitForConnectResult() != WL_CONNECTED)
		{
			return false;
		}

		DEBUG_PRINT("IP address: ");
		DEBUG_PRINTLN(WiFi.localIP());
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
		DEBUG_PRINTLN("Attempting MQTT connection...");

		if (_mqttClient.connect(_mqttClientId, _mqttUser, _mqttPass))
		{
			DEBUG_PRINTLN("Connected.");
			// Subscribe for topics:
			//  basetopic
			_mqttClient.subscribe(_baseTopic);
			
			//  basetopic/+/set. This pattern include:  basetopic/mode/set, basetopic/desiredtemp/set, basetopic/state/set
			strConcatenate(_topicBuff, 4, _baseTopic, TOPIC_SEPARATOR, EVERY_ONE_LEVEL_TOPIC, TOPIC_SET);
			_mqttClient.subscribe(_topicBuff);
		}
		else
		{
			DEBUG_PRINT("failed, rc=");
			DEBUG_PRINT(_mqttClient.state());
			DEBUG_PRINTLN(" try again after 5 seconds");
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
	DEBUG_PRINTLN("Should save config");
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
	DEBUG_PRINTLN("Mounting FS...");

	ReadConfiguration();

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
		DEBUG_PRINTLN("Doesn't connect.");
		return false;
	}

	//if you get here you have connected to the WiFi
	DEBUG_PRINTLN("Connected.");

	if (_shouldSaveConfig)
	{
		//read updated parameters
		strcpy(_mqttServer, customMqttServer.getValue());
		strcpy(_mqttPort, customMqttPort.getValue());
		strcpy(_mqttClientId, customClientName.getValue());
		strcpy(_mqttUser, customMqttUser.getValue());
		strcpy(_mqttPass, customMqttPass.getValue());

		SaveConfiguration();
	}

	return true;
}

void ReadConfiguration()
{
	if (!SPIFFS.begin())
	{
		DEBUG_PRINTLN("Failed to mount FS");
	}
	else
	{
		DEBUG_PRINTLN("The file system is mounted.");

		if (SPIFFS.exists(CONFIG_FILE_NAME))
		{
			//file exists, reading and loading
			DEBUG_PRINTLN("Reading configuration file");
			File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
			if (configFile)
			{
				DEBUG_PRINTLN("Opening configuration file");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
#ifdef WIFIFCMM
				json.printTo(DEBUG_PRINTER);
#endif
				if (json.success())
				{
					DEBUG_PRINTLN("\nJson is parsed");

					strcpy(_mqttServer, json[MQTT_SERVER_KEY]);
					strcpy(_mqttPort, json[MQTT_PORT_KEY]);
					strcpy(_mqttClientId, json[MQTT_CLIENT_ID_KEY]);
					strcpy(_mqttUser, json[MQTT_USER_KEY]);
					strcpy(_mqttPass, json[MQTT_PASS_KEY]);
					// After start device we should set this settings.
					//_mode = (Mode)atoi(json[MODE_KEY]);
					//_deviceState = atoi(json[TOPIC_DEVICE_STATE]) == 1 ? On : Off;
				}
				else
				{
					DEBUG_PRINTLN("Loading json configuration is failed");
				}
			}
		}
	}
}

void SaveConfiguration()
{
	DEBUG_PRINTLN("Saving configuration...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	json[MQTT_SERVER_KEY] = _mqttServer;
	json[MQTT_PORT_KEY] = _mqttPort;
	json[MQTT_CLIENT_ID_KEY] = _mqttClientId;
	json[MQTT_USER_KEY] = _mqttUser;
	json[MQTT_PASS_KEY] = _mqttPass;
	// We shouldn't save this settings.
	//json[MODE_KEY] = (int)_mode;
	//json[TOPIC_DEVICE_STATE] = _deviceState == On ? 1 : 0;

	File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
	if (!configFile) {
		DEBUG_PRINTLN("Failed to open a configuration file for writing.");
	}
	else
	{
		DEBUG_PRINTLN("Configuration is saved.");
	}

	json.prettyPrintTo(Serial);
	json.printTo(configFile);
	configFile.close();
}