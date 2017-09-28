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
#include "FanCoilHelper.h"
#include <KMPDinoWiFiESP.h>       // Our library. https://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/howtoinstall.aspx
#include "KMPCommon.h"
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
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

// Text buffers for topic and payload.
char _topicBuff[128];
char _payloadBuff[32];

bool _shouldSaveConfig = false;
bool _isSensorExist = true;

Mode _mode = Cold;
DeviceState _deviceState = Off;
DeviceState _lastDeviceState;

float _desiredTemperature = 22.0;
float _currentTemperature;
float _averageTemperature;
float _tempCollection[TEMPERATURE_ARRAY_LEN];
uint8_t _tempCollectPos = 0;
unsigned long _checkTempInterval;

float _currentHumidity;
float _averageHumidity;
float _humidityCollection[HUMIDITY_ARRAY_LEN];
uint8_t _humidityCollectPos = 0;
unsigned long _checkHumidityInterval;

unsigned long _checkPingInterval;

// Store last time intervals. 0 -  measure Temp, 1 - ping
uint8_t _fanDegree = 0;

bool _deviceIsConnected = false;

bool getTemperatureAndHumidity(bool processStatus = true)
{
	_currentTemperature = _dhtSensor.readTemperature();
	_currentHumidity = _dhtSensor.readHumidity();

	bool result = _currentHumidity != NAN && _currentTemperature != NAN;

	if (processStatus)
	{
		processDeviceStatus(result);
	}

	return result;
}

/**
* @brief Execute first after start the device. Initialize hardware.
*
* @return void
*/
void setup(void)
{
	// You can open the Arduino IDE Serial Monitor window to see what the code is doing
	DEBUG_FC.begin(115200);
	// Init KMP ProDino WiFi-ESP board.
	KMPDinoWiFiESP.init();
	KMPDinoWiFiESP.SetAllRelaysOff();

	DEBUG_FC_PRINTLN(F("KMP fan coil management with Mqtt.\r\n"));

	//WiFiManager
	//Local initialization. Once it's business is done, there is no need to keep it around.
	WiFiManager wifiManager;

	// Is OptoIn 4 is On the board is resetting WiFi configuration.
	//if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
	//{
	//	DEBUG_FC_PRINTLN("Resetting WiFi configuration...\r\n");
	//	//reset saved settings
	//	wifiManager.resetSettings();
	//	DEBUG_FC_PRINTLN("WiFi configuration was reseted.\r\n");
	//}

	// Set save configuration callback.
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	if (!mangeConnectParamers(&wifiManager))
	{
		return;
	}

	_dhtSensor.begin();

	// Initialize arrays.
	if (getTemperatureAndHumidity(false))
	{
		for (size_t i = 0; i < TEMPERATURE_ARRAY_LEN; i++)
		{
			_tempCollection[i] = _currentTemperature;
		}

		for (size_t i = 0; i < HUMIDITY_ARRAY_LEN; i++)
		{
			_humidityCollection[i] = _currentHumidity;
		}
	}

	// Initialize MQTT.
	_mqttClient.setClient(_wifiClient);
	uint16_t port = atoi(_mqttPort);
	_mqttClient.setServer(_mqttServer, port);
	_mqttClient.setCallback(callback);
}

void publishData(DeviceData deviceData, bool sendCurrent = false)
{
	if (CHECK_ENUM(deviceData, Temperature))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_CURRENT_TEMPERATURE);

		float val = sendCurrent ? _currentTemperature : _averageTemperature;

		mqttPublish(_topicBuff, floatToStr(val, TEMPERATURE_PRECISION));
	}

	if (CHECK_ENUM(deviceData, Humidity))
	{
		strConcatenate(_topicBuff, 3, _baseTopic, TOPIC_SEPARATOR, TOPIC_HUMIDITY);

		float val = sendCurrent ? _currentHumidity : _averageHumidity;

		mqttPublish(_topicBuff, floatToStr(val, HUMIDITY_PRECISION));
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

	if (CHECK_ENUM(deviceData, DeviceIsReady))
	{
		mqttPublish(_baseTopic, (char*)PAYLOAD_READY);
	}

	if (CHECK_ENUM(deviceData, DevicePing))
	{
		mqttPublish(_baseTopic, (char*)PAYLOAD_PING);
	}

	_checkPingInterval = millis() + PING_INTERVAL_MS;
}

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	// For a normal work on device, need it be connected to WiFi and MQTT server.
	bool isConnected = connectWiFi() && connectMqtt();

	processConnectionStatus(isConnected);

	if (!isConnected)
	{
		return;
	}

	_mqttClient.loop();
	
	collectTemperature();
	processTemperature();

	collectHumidity();
	processHumidity();

	fanCoilControl();

	if (millis() > _checkPingInterval)
	{
		publishData(DevicePing);
	}
}

/**
* @brief Callback method. It is fire when has information in subscribed topics.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {
#ifdef WIFIFCMM_DEBUG
	printTopicAndPayload("Call back", topic, (char*)payload, length);
#endif

	size_t baseTopicLen = strlen(_baseTopic);

	if (!startsWith(topic, _baseTopic))
	{
		return;
	}

	// Processing base topic - command send all data from device.
	if (strlen(topic) == baseTopicLen && length == 0)
	{
		DeviceData deviceData = (DeviceData)(Temperature | DesiredTemp | FanDegree | CurrentMode | CurrentDeviceState | Humidity);
		publishData(deviceData, true);
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
		if (isEqual((char*)payload, PAYLOAD_HEAT, length))
		{
			_mode = Heat;
		}

		if (isEqual((char*)payload, PAYLOAD_COLD, length))
		{
			_mode = Cold;
		}

		publishData(CurrentMode);
		//SaveConfiguration();
		return;
	}

	// Processing topic basetopic/desiredtemp/set: 22.5
	if (isEqual(topic, TOPIC_DESIRED_TEMPERATURE))
	{
		memcpy(_topicBuff, payload, length);
		_topicBuff[length] = CH_NONE;
		
		float temp = atof(_topicBuff);
		float roundTemp = roundF(temp, TEMPERATURE_PRECISION);
		if (roundTemp >= MIN_DESIRED_TEMPERATURE && roundTemp <= MAX_DESIRED_TEMPERATURE)
		{
			_desiredTemperature = roundTemp;
		}
		
		publishData(DesiredTemp);
		return;
	}

	// Processing topic basetopic/state/set: on, off
	if (isEqual(topic, TOPIC_DEVICE_STATE))
	{
		if (isEqual((char*)payload, PAYLOAD_ON, length))
		{
			_deviceState = _lastDeviceState = On;
		}
		
		if (isEqual((char*)payload, PAYLOAD_OFF, length))
		{
			_deviceState = _lastDeviceState = Off;
		}

		publishData(CurrentDeviceState);
		//SaveConfiguration();
		return;
	}
}

/**
* @brief Check if DHT sensor exist. It is implement plug and play functionality.
* If can't find sensor switch device Off. If the sensor appears again, return device in a previous state.
*/
void processDeviceStatus(bool isSensorExist)
{
	if (isSensorExist)
	{
		_isSensorExist = true;

		if (_lastDeviceState != _deviceState)
		{
			_deviceState = _lastDeviceState;
			publishData(CurrentDeviceState);
		}

		return;
	}

	// Check two times.
	if (_isSensorExist)
	{
		_isSensorExist = false;
		return;
	}

	if (_deviceState == On)
	{
		_lastDeviceState = On;

		// If sensor doesn't exist 2 time, switch device to Off.
		_deviceState = Off;
		publishData(CurrentDeviceState);
	}
}

void processConnectionStatus(bool isConnected)
{
	if (isConnected)
	{
		if (_deviceIsConnected != isConnected)
		{
			_deviceIsConnected = isConnected;
			publishData((DeviceData)(Temperature | DesiredTemp | FanDegree | CurrentMode | CurrentDeviceState | Humidity | DeviceIsReady));
		}
		
		return;
	}

	_deviceIsConnected = false;

	if (_deviceState == On)
	{
		_deviceState = Off;

		// Switch off fan.
		setFanDegree(0);
	}
}


void collectTemperature()
{
	if (millis() > _checkTempInterval)
	{
		if (_tempCollectPos >= TEMPERATURE_ARRAY_LEN)
		{
			_tempCollectPos = 0;
		}

		if (_isSensorExist)
		{
			_tempCollection[_tempCollectPos++] = roundF(_currentTemperature, TEMPERATURE_PRECISION);
		}

		// Set next time to read data.
		_checkTempInterval = millis() + CHECK_TEMP_INTERVAL_MS;
	}
}

void collectHumidity()
{
	if (millis() > _checkHumidityInterval)
	{
		if (_humidityCollectPos >= HUMIDITY_ARRAY_LEN)
		{
			_humidityCollectPos = 0;
		}

		if (_isSensorExist)
		{
			_humidityCollection[_humidityCollectPos++] = roundF(_currentHumidity, HUMIDITY_PRECISION);
		}

		// Set next time to read data.
		_checkHumidityInterval = millis() + CHECK_HUMIDITY_INTERVAL_MS;
	}
}

void processTemperature()
{
	float result = calcAverage(_tempCollection, TEMPERATURE_ARRAY_LEN, TEMPERATURE_PRECISION);

	if (_averageTemperature != result)
	{
		_averageTemperature = result;
		publishData(Temperature);
	}
}

void processHumidity()
{
	float result = calcAverage(_humidityCollection, HUMIDITY_ARRAY_LEN, HUMIDITY_PRECISION);

	if (_averageHumidity != result)
	{
		_averageHumidity = result;
		publishData(Humidity);
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

	// Switch last fan degree relay off.
	if (_fanDegree != 0)
	{
		KMPDinoWiFiESP.SetRelayState(_fanDegree - 1, false);
	}

	delay(100);

	if (degree > 0)
	{
		KMPDinoWiFiESP.SetRelayState(degree - 1, true);
	}

	_fanDegree = degree;

	publishData(FanDegree);
}

char* floatToStr(float value, uint precision)
{
	if (_isSensorExist)
	{
		FloatToChars(_currentTemperature, precision, _payloadBuff);
		return _payloadBuff;
	}

	return (char*) NOT_AVILABLE;
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
#ifdef WIFIFCMM_DEBUG
	printTopicAndPayload("Publish", topic, payload, strlen(payload));
#endif
	_mqttClient.publish(topic, (const char*)payload);
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
		DEBUG_FC_PRINT(F("Reconnecting ["));
		DEBUG_FC_PRINT(WiFi.SSID());
		DEBUG_FC_PRINTLN(F("]..."));

		WiFi.begin();

		if (WiFi.waitForConnectResult() != WL_CONNECTED)
		{
			return false;
		}

		DEBUG_FC_PRINT(F("IP address: "));
		DEBUG_FC_PRINTLN(WiFi.localIP());
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
		DEBUG_FC_PRINTLN(F("Attempting MQTT connection..."));

		if (_mqttClient.connect(_mqttClientId, _mqttUser, _mqttPass))
		{
			DEBUG_FC_PRINTLN(F("MQTT connected. Subscribe for topics:"));
			// Subscribe for topics:
			//  basetopic
			_mqttClient.subscribe(_baseTopic);
			DEBUG_FC_PRINTLN(_baseTopic);

			//  basetopic/+/set. This pattern include:  basetopic/mode/set, basetopic/desiredtemp/set, basetopic/state/set
			strConcatenate(_topicBuff, 5, _baseTopic, TOPIC_SEPARATOR, EVERY_ONE_LEVEL_TOPIC, TOPIC_SEPARATOR, TOPIC_SET);
			_mqttClient.subscribe(_topicBuff);
			DEBUG_FC_PRINTLN(_topicBuff);
		}
		else
		{
			DEBUG_FC_PRINT(F("failed, rc="));
			DEBUG_FC_PRINT(_mqttClient.state());
			DEBUG_FC_PRINTLN(F(" try again after 5 seconds"));
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
	DEBUG_FC_PRINTLN("Should save config");
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
	DEBUG_FC_PRINTLN("Mounting FS...");

	ReadConfiguration();

	// The extra parameters to be configured (can be either global or just in the setup)
	// After connecting, parameter.getValue() will get you the configured value
	// id/name placeholder/prompt default length
	WiFiManagerParameter customMqttServer("server", "MQTT server", _mqttServer, MQTT_SERVER_LEN);
	WiFiManagerParameter customMqttPort("port", "MQTT port", String(_mqttPort).c_str(), MQTT_PORT_LEN);
	WiFiManagerParameter customClientName("clientName", "Client name", _mqttClientId, MQTT_CLIENT_ID_LEN);
	WiFiManagerParameter customMqttUser("user", "MQTT user", _mqttUser, MQTT_USER_LEN);
	WiFiManagerParameter customMqttPass("password", "MQTT pass", _mqttPass, MQTT_PASS_LEN);
	WiFiManagerParameter customBaseTopic("baseTopic", "Main topic", _baseTopic, BASE_TOPIC_LEN);

	// add all your parameters here
	wifiManager->addParameter(&customMqttServer);
	wifiManager->addParameter(&customMqttPort);
	wifiManager->addParameter(&customClientName);
	wifiManager->addParameter(&customMqttUser);
	wifiManager->addParameter(&customMqttPass);
	wifiManager->addParameter(&customBaseTopic);

	// fetches ssid and pass from eeprom and tries to connect
	// if it does not connect it starts an access point with the specified name
	// auto generated name ESP + ChipID
	if (!wifiManager->autoConnect())
	{
		DEBUG_FC_PRINTLN("Doesn't connect.");
		return false;
	}

	//if you get here you have connected to the WiFi
	DEBUG_FC_PRINTLN("Connected.");

	if (_shouldSaveConfig)
	{
		//read updated parameters
		strcpy(_mqttServer, customMqttServer.getValue());
		strcpy(_mqttPort, customMqttPort.getValue());
		strcpy(_mqttClientId, customClientName.getValue());
		strcpy(_mqttUser, customMqttUser.getValue());
		strcpy(_mqttPass, customMqttPass.getValue());
		strcpy(_baseTopic, customBaseTopic.getValue());

		SaveConfiguration();
	}

	return true;
}

void ReadConfiguration()
{
	if (!SPIFFS.begin())
	{
		DEBUG_FC_PRINTLN("Failed to mount FS");
	}
	else
	{
		DEBUG_FC_PRINTLN("The file system is mounted.");

		if (SPIFFS.exists(CONFIG_FILE_NAME))
		{
			//file exists, reading and loading
			DEBUG_FC_PRINTLN("Reading configuration file");
			File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
			if (configFile)
			{
				DEBUG_FC_PRINTLN("Opening configuration file");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
#ifdef WIFIFCMM_DEBUG
				json.printTo(DEBUG_FC);
#endif
				if (json.success())
				{
					DEBUG_FC_PRINTLN("\nJson is parsed");

					strcpy(_mqttServer, json[MQTT_SERVER_KEY]);
					strcpy(_mqttPort, json[MQTT_PORT_KEY]);
					strcpy(_mqttClientId, json[MQTT_CLIENT_ID_KEY]);
					strcpy(_mqttUser, json[MQTT_USER_KEY]);
					strcpy(_mqttPass, json[MQTT_PASS_KEY]);
					strcpy(_baseTopic, json[BASE_TOPIC_KEY]);
					// After start device we should set this settings.
					//_mode = (Mode)atoi(json[MODE_KEY]);
					//_deviceState = atoi(json[TOPIC_DEVICE_STATE]) == 1 ? On : Off;
				}
				else
				{
					DEBUG_FC_PRINTLN(F("Loading json configuration is failed"));
				}
			}
		}
	}
}

void SaveConfiguration()
{
	DEBUG_FC_PRINTLN(F("Saving configuration..."));

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	json[MQTT_SERVER_KEY] = _mqttServer;
	json[MQTT_PORT_KEY] = _mqttPort;
	json[MQTT_CLIENT_ID_KEY] = _mqttClientId;
	json[MQTT_USER_KEY] = _mqttUser;
	json[MQTT_PASS_KEY] = _mqttPass;
	json[BASE_TOPIC_KEY] = _baseTopic;
	// We shouldn't save this settings.
	//json[MODE_KEY] = (int)_mode;
	//json[TOPIC_DEVICE_STATE] = _deviceState == On ? 1 : 0;

	File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
	if (!configFile) {
		DEBUG_FC_PRINTLN(F("Failed to open a configuration file for writing."));
	}
	else
	{
		DEBUG_FC_PRINTLN(F("Configuration is saved."));
	}

#ifdef WIFIFCMM_DEBUG
	json.prettyPrintTo(DEBUG_FC);
#endif

	json.printTo(configFile);
	configFile.close();
}
