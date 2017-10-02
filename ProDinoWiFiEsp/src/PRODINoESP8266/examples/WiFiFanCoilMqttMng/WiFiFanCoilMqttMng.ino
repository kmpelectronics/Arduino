// WiFiFanCoilMqttMng.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
// Example link: 
// Prerequisites: 
//    You should install following libraries:
//		Install with Library Manager. "ArduinoJson by Benoit Blanchon" https://github.com/bblanchon/ArduinoJson
// Version: 0.0.1
// Date: 13.09.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Attention: The project has not finished yet!

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
#include <DallasTemperature.h>    // Install with Library Manager. "DallasTemperature by Miles Burton, ..." https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <OneWire.h>			  // This library Installed together with a DallasTemperature library.

DeviceSettings _settings;

WiFiClient _wifiClient;
PubSubClient _mqttClient;
DHT _dhtSensor(DHT_SENSORS_PIN, DHT_SENSORS_TYPE, 11);

OneWire _oneWire(ONEWIRE_SENSORS_PIN);
DallasTemperature _oneWireSensors(&_oneWire);

// Text buffers for topic and payload.
char _topicBuff[128];
char _payloadBuff[32];

bool _isSensorExist = true;

Mode _mode = Cold;
DeviceState _deviceState = Off;
DeviceState _lastDeviceState;

float _desiredTemperature = 22.0;
SensorData _temperatureData;
float _tempCollection[TEMPERATURE_ARRAY_LEN];

SensorData _humidityData;
float _humidityCollection[HUMIDITY_ARRAY_LEN];

SensorData _inletData;
float _inletCollection[INLET_ARRAY_LEN];

SensorData _outletData;
float _outletCollection[OUTLET_ARRAY_LEN];

uint _pipeSensorCount;

unsigned long _checkPingInterval;

// Store last time intervals. 0 -  measure Temp, 1 - ping
uint8_t _fanDegree = 0;

bool _deviceIsConnected = false;

bool getTemperatureAndHumidity(bool processStatus = true)
{
	_temperatureData.Current = _dhtSensor.readTemperature();
	_humidityData.Current = _dhtSensor.readHumidity();

	bool result = _temperatureData.Current != NAN && _humidityData.Current != NAN;

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
	_temperatureData.DataCollection = _tempCollection;
	_temperatureData.DataCollectionLen = TEMPERATURE_ARRAY_LEN;
	_temperatureData.Precision = TEMPERATURE_PRECISION;
	_temperatureData.CheckDataIntervalMS = CHECK_TEMP_INTERVAL_MS;
	_temperatureData.DataType = Temperature;

	_humidityData.DataCollection = _humidityCollection;
	_humidityData.DataCollectionLen = HUMIDITY_ARRAY_LEN;
	_humidityData.Precision = HUMIDITY_PRECISION;
	_humidityData.CheckDataIntervalMS = CHECK_HUMIDITY_INTERVAL_MS;
	_humidityData.DataType = Humidity;

	_inletData.DataCollection = _inletCollection;
	_inletData.DataCollectionLen = INLET_ARRAY_LEN;
	_inletData.Precision = INLET_PRECISION;
	_inletData.CheckDataIntervalMS = CHECK_INLET_INTERVAL_MS;
	_inletData.DataType = InletPipe;

	_outletData.DataCollection = _outletCollection;
	_outletData.DataCollectionLen = OUTLET_ARRAY_LEN;
	_outletData.Precision = OUTLET_PRECISION;
	_outletData.CheckDataIntervalMS = CHECK_OUTLET_INTERVAL_MS;
	_outletData.DataType = OutletPipe;

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
	if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
	{
		DEBUG_FC_PRINTLN("Resetting WiFi configuration...\r\n");
		//reset saved settings
		wifiManager.resetSettings();
		DEBUG_FC_PRINTLN("WiFi configuration was reseted.\r\n");
	}

	_oneWireSensors.begin();
	_pipeSensorCount = _oneWireSensors.getDeviceCount();

	// Set save configuration callback.
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	
	if (!mangeConnectParamers(&wifiManager, &_settings))
	{
		return;
	}

	_dhtSensor.begin();

	// Initialize arrays.
	if (getTemperatureAndHumidity(false))
	{
		for (size_t i = 0; i < TEMPERATURE_ARRAY_LEN; i++)
		{
			_tempCollection[i] = _temperatureData.Current;
		}

		for (size_t i = 0; i < HUMIDITY_ARRAY_LEN; i++)
		{
			_humidityCollection[i] = _humidityData.Current;
		}
	}

	// TODO: Add resolution per sensor.
	//sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);

	// Initialize MQTT.
	_mqttClient.setClient(_wifiClient);
	uint16_t port = atoi(_settings.MqttPort);
	_mqttClient.setServer(_settings.MqttServer, port);
	_mqttClient.setCallback(callback);
}

void publishData(DeviceData deviceData, bool sendCurrent = false)
{
	if (CHECK_ENUM(deviceData, Temperature))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_CURRENT_TEMPERATURE);

		float val = sendCurrent ? _temperatureData.Current : _temperatureData.Average;

		mqttPublish(_topicBuff, floatToStr(val, TEMPERATURE_PRECISION));
	}

	if (CHECK_ENUM(deviceData, Humidity))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_HUMIDITY);

		float val = sendCurrent ? _humidityData.Current : _humidityData.Average;

		mqttPublish(_topicBuff, floatToStr(val, HUMIDITY_PRECISION));
	}

	if (CHECK_ENUM(deviceData, FanDegree))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_FAN_DEGREE);
		IntToChars(_fanDegree, _payloadBuff);

		mqttPublish(_topicBuff, _payloadBuff);
	}

	if (CHECK_ENUM(deviceData, DesiredTemp))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_DESIRED_TEMPERATURE);
		FloatToChars(_desiredTemperature, TEMPERATURE_PRECISION, _payloadBuff);

		mqttPublish(_topicBuff, _payloadBuff);
	}

	if (CHECK_ENUM(deviceData, CurrentMode))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_MODE);

		const char * mode = _mode == Cold ? PAYLOAD_COLD : PAYLOAD_HEAT;

		mqttPublish(_topicBuff, (char*)mode);
	}

	if (CHECK_ENUM(deviceData, CurrentDeviceState))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_DEVICE_STATE);

		const char * mode = _deviceState == On ? PAYLOAD_ON : PAYLOAD_OFF;

		mqttPublish(_topicBuff, (char*)mode);
	}

	if (CHECK_ENUM(deviceData, DeviceIsReady))
	{
		mqttPublish(_settings.BaseTopic, (char*)PAYLOAD_READY);
	}

	if (CHECK_ENUM(deviceData, DevicePing))
	{
		mqttPublish(_settings.BaseTopic, (char*)PAYLOAD_PING);
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
	
	collectData(&_temperatureData);
	processData(&_temperatureData);

	collectData(&_humidityData);
	processData(&_humidityData);
	
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

	size_t baseTopicLen = strlen(_settings.BaseTopic);

	if (!startsWith(topic, _settings.BaseTopic))
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

void collectData(SensorData* data)
{
	if (millis() > data->CheckInterval)
	{
		if (data->CurrentCollectPos >= data->DataCollectionLen)
		{
			data->DataCollectionLen = 0;
		}

		if (_isSensorExist)
		{
			data->DataCollection[data->CurrentCollectPos++] = roundF(data->Current, data->Precision);
		}

		// Set next time to read data.
		data->CheckInterval = millis() + data->CheckDataIntervalMS;
	}
}

void processData(SensorData* data)
{
	float result = calcAverage(data->DataCollection, data->DataCollectionLen, data->Precision);

	if (data->Average != result)
	{
		data->Average = result;
		publishData(data->DataType);
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

	float averageTemp = _temperatureData.Average;

	float diffTemp = _mode == Cold ? averageTemp - _desiredTemperature /* Cold */ : _desiredTemperature - averageTemp /* Heat */;

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
		FloatToChars(value, precision, _payloadBuff);
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

		if (_mqttClient.connect(_settings.MqttClientId, _settings.MqttUser, _settings.MqttPass))
		{
			DEBUG_FC_PRINTLN(F("MQTT connected. Subscribe for topics:"));
			// Subscribe for topics:
			//  basetopic
			_mqttClient.subscribe(_settings.BaseTopic);
			DEBUG_FC_PRINTLN(_settings.BaseTopic);

			//  basetopic/+/set. This pattern include:  basetopic/mode/set, basetopic/desiredtemp/set, basetopic/state/set
			strConcatenate(_topicBuff, 5, _settings.BaseTopic, TOPIC_SEPARATOR, EVERY_ONE_LEVEL_TOPIC, TOPIC_SEPARATOR, TOPIC_SET);
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