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
// Version: 1.3.0
// Start date: 13.09.2017
// Last version date: 10.10.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "FanCoilHelper.h"
#include <KMPDinoWiFiESP.h>       // Our library. https://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/howtoinstall.aspx
#include <KMPCommon.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>         // Install with Library Manager. "PubSubClient by Nick O'Leary" https://pubsubclient.knolleary.net/
#include <DHT.h>                  // Install with Library Manager. "DHT sensor library by Adafruit" https://github.com/adafruit/DHT-sensor-library
#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager
#include <DallasTemperature.h>    // Install with Library Manager. "DallasTemperature by Miles Burton, ..." https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <OneWire.h>			  // Install with Library Manager. "One Wire by Jim Studt, ..."

DeviceSettings _settings;

WiFiClient _wifiClient;
PubSubClient _mqttClient;
DHT _dhtSensor(DHT_SENSORS_PIN, DHT_SENSORS_TYPE, 11);

OneWire _oneWire(ONEWIRE_SENSORS_PIN);
DallasTemperature _oneWireSensors(&_oneWire);

// Text buffers for topic and payload.
char _topicBuff[128];
char _payloadBuff[32];

float _desiredTemperature = 22.0;

DeviceState _bypassState = Off;
DeviceState _bypassNewState = Off;
bool _bypassStateIsChanging = false;
unsigned long _bypassChangeStateInterval;

uint8_t _fanDegree = 0;
Mode _mode = Cold;
DeviceState _deviceState = Off;
DeviceState _lastDeviceState = Off;
unsigned long _checkPingInterval;

bool _isConnected = false;
bool _isStarted = false;
bool _isDHTExists = true;
bool _isDS18b20Exists = true;

void publishData(DeviceData deviceData, bool sendCurrent = false)
{
	_checkPingInterval = millis() + PING_INTERVAL_MS;

	if (!_isConnected || !_isStarted)
	{
		return;
	}

	if (CHECK_ENUM(deviceData, Temperature))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_TEMPERATURE);

		mqttPublish(_topicBuff, valueToStr(&TemperatureData, sendCurrent));
	}

	if (CHECK_ENUM(deviceData, Humidity))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_HUMIDITY);

		mqttPublish(_topicBuff, valueToStr(&HumidityData, sendCurrent));
	}

	if (CHECK_ENUM(deviceData, InletPipe))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_INLET_TEMPERATURE);

		mqttPublish(_topicBuff, valueToStr(&InletData, sendCurrent));
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

	if (CHECK_ENUM(deviceData, BypassState))
	{
		strConcatenate(_topicBuff, 3, _settings.BaseTopic, TOPIC_SEPARATOR, TOPIC_BYPASS_STATE);

		const char * mode = _bypassState == On ? PAYLOAD_ON : PAYLOAD_OFF;

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

	// Processing base topic - command sends all data from the device.
	if (strlen(topic) == baseTopicLen && length == 0)
	{
		publishAllData();
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
		setDeviceMode((char*)payload, length);
		return;
	}

	// Processing topic basetopic/desiredtemp/set: 22.5
	if (isEqual(topic, TOPIC_DESIRED_TEMPERATURE))
	{
		memcpy(_topicBuff, payload, length);
		_topicBuff[length] = CH_NONE;

		float temp = atof(_topicBuff);
		setDesiredTemperature(temp);

		return;
	}

	// Processing topic basetopic/state/set: on, off
	if (isEqual(topic, TOPIC_DEVICE_STATE))
	{
		if (isEqual((char*)payload, PAYLOAD_ON, length))
		{
			if (setDeviceState(On))
			{
				_lastDeviceState = On;
			}
		}

		if (isEqual((char*)payload, PAYLOAD_OFF, length))
		{
			if (setDeviceState(Off))
			{
				_lastDeviceState = Off;
			}
		}

		return;
	}
}

/**
* @brief Execute first after start the device. Initialize hardware.
*
* @return void
*/
void setup(void)
{
	initializeSensorData();

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
		DEBUG_FC_PRINTLN(F("Resetting WiFi configuration...\r\n"));
		wifiManager.resetSettings();
		DEBUG_FC_PRINTLN(F("WiFi configuration was reseted.\r\n"));
	}

	// Set save configuration callback.
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	if (!mangeConnectParamers(&wifiManager, &_settings))
	{
		return;
	}

	// Start sensors.
	_dhtSensor.begin();

	// Initialize MQTT.
	_mqttClient.setClient(_wifiClient);
	uint16_t port = atoi(_settings.MqttPort);
	_mqttClient.setServer(_settings.MqttServer, port);
	_mqttClient.setCallback(callback);

	FunCoilHelper.SetExpanderDirection(BYPASS_OFF_PIN, OUTPUT);
	FunCoilHelper.SetExpanderDirection(BYPASS_ON_PIN, OUTPUT);
	// Switch off bypass.
	_bypassState = On;
	setBypassState(Off);
}

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
{
	// For a normal work on device, need it be connected to WiFi and MQTT server.
	_isConnected = connectWiFi() && connectMqtt();

	if (!_isConnected)
	{
		// Turn off a device
		setDeviceState(Off);
		uint8_t degree = processFanDegree();
		setFanDegree(degree);

		return;
	}

	_mqttClient.loop();

	bool isDHTExists = getTemperatureAndHumidity();
	processDHTStatus(isDHTExists);

	bool isDS18B20Exists = getPipesTemperature();
	processDS18B20Status(isDS18B20Exists);

	if (!_isStarted)
	{
		setArrayValues(&TemperatureData);
		setArrayValues(&HumidityData);
		setArrayValues(&InletData);
	}

	processData(&TemperatureData);
	processData(&HumidityData);
	processData(&InletData);

	uint8_t degree = processFanDegree();
	setFanDegree(degree);

	if (millis() > _checkPingInterval)
	{
		publishData(DevicePing);
	}

	if (!_isStarted)
	{
		_isStarted = true;
		publishData(DeviceIsReady);
	}

	processByPassState();
}

bool getTemperatureAndHumidity()
{
	bool result = _dhtSensor.read();
	if (result)
	{
		TemperatureData.Current = _dhtSensor.readTemperature();
		HumidityData.Current = _dhtSensor.readHumidity();
	}

	TemperatureData.IsExists = result;
	HumidityData.IsExists = result;

	return result;
}

bool getPipesTemperature()
{
	if (!InletData.IsExists)
	{
		findPipeSensors();
	};

	if (InletData.IsExists)
	{
		// Send the command to get temperatures.
		_oneWireSensors.requestTemperatures();
		float temp = _oneWireSensors.getTempC(InletData.Address);
		if (temp != DEVICE_DISCONNECTED_C)
		{
			InletData.Current = _oneWireSensors.getTempC(InletData.Address);
		}
		else
		{
			InletData.IsExists = false;
		}
	}

	return InletData.IsExists;
}

/**
* @brief
*/
bool setDeviceState(DeviceState state)
{
	DeviceState shouldBe = state;

	if (!_isConnected || !_isDHTExists)
	{
		shouldBe = Off;
	}

	_deviceState = shouldBe;
	publishData(CurrentDeviceState);

	return _deviceState == state;
}

void processDHTStatus(bool isExists)
{
	bool sendData = false;

	if (isExists)
	{
		sendData = !_isDHTExists;
		_isDHTExists = true;
		if (_deviceState != _lastDeviceState)
		{
			setDeviceState(_lastDeviceState);
		}
	}
	else
	{
		if (_isDHTExists)
		{
			_isDHTExists = false;
			_lastDeviceState = _deviceState;
			// TODO: Maybe send an error
			sendData = true;
			// Turn off a device
			setDeviceState(Off);
		}
	}

	if (sendData)
	{
		publishData(DeviceData(Temperature | Humidity));
	}
}

void processDS18B20Status(bool isExists)
{
	bool sendData = false;

	if (isExists)
	{
		sendData = !_isDS18b20Exists;
		_isDS18b20Exists = true;

	}
	else
	{
		if (_isDS18b20Exists)
		{
			// TODO: Maybe send warning
			_isDS18b20Exists = false;
			sendData = true;
		}
	}

	if (sendData)
	{
		publishData(InletPipe);
	}
}

void processData(SensorData* data)
{
	if (millis() > data->CheckInterval)
	{
		if (data->CurrentCollectPos >= data->DataCollectionLen)
		{
			data->CurrentCollectPos = 0;
		}

		if (data->IsExists)
		{
			data->DataCollection[data->CurrentCollectPos++] = roundF(data->Current, data->Precision);
		}

		// Process collected data
		float result = calcAverage(data->DataCollection, data->DataCollectionLen, data->Precision);

		if (data->Average != result)
		{
			data->Average = result;
			publishData(data->DataType);
		}

		// Set next time to read data.
		data->CheckInterval = millis() + data->CheckDataIntervalMS;
	}
}

uint8_t processFanDegree()
{
	uint8_t degree = 0;

	if (_deviceState == Off)
	{
		return degree;
	}

	float diffTemp = _mode == Cold ? TemperatureData.Average - _desiredTemperature /* Cold */ : _desiredTemperature - TemperatureData.Average /* Heat */;

	// Bypass the fan coil.
	if (diffTemp > BYPASS_OFF_TEMPERTURE_DIFFERENCE)
	{
		setBypassState(Off);
	}

	if (diffTemp < BYPASS_ON_TEMPERTURE_DIFFERENCE)
	{
		setBypassState(On);
	}

	float pipeDiffTemp;

	if (InletData.IsExists)
	{
		pipeDiffTemp = _mode == Cold ? TemperatureData.Average - InletData.Average /* Cold */ : InletData.Average - TemperatureData.Average /* Heat */;
	}

	// If inlet sensor doesn't exist or difference between inlet pipe temperature and ambient temperature > 5 degree get fan degree.
	if (!InletData.IsExists || pipeDiffTemp >= 5)
	{
		int i = FAN_SWITCH_LEVEL_LEN;
		while (i > 0)
		{
			if (diffTemp > FAN_SWITCH_LEVEL[--i])
			{
				degree = i + 1;
				break;
			}
		}
	}

	return degree;
}

/**
* @brief: Setting the degree of fun.
* The degrees: 0 - stopped, 1 - low fan speed, 2 - medium, 3 - high
**/
void setFanDegree(uint8_t degree)
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

void setDesiredTemperature(float temp)
{
	if (!std::isnan(temp))
	{
		float roundTemp = roundF(temp, TEMPERATURE_PRECISION);
		if (roundTemp >= MIN_DESIRED_TEMPERATURE && roundTemp <= MAX_DESIRED_TEMPERATURE)
		{
			_desiredTemperature = roundTemp;
		}
		publishData(DesiredTemp);
	}
}

/**
* @brief Set bypass new state. 
* @param state new state of the bypass
*
* @return void
**/
// TODO: Add execute this method in appropriate methods
void setBypassState(DeviceState state)
{
	if (state == _bypassState || _bypassStateIsChanging)
	{
		return;
	}

	// Start bypass state changing
	_bypassNewState = state;

	_bypassStateIsChanging = true;
	_bypassChangeStateInterval = millis() + BYPASS_CHANGE_STATE_INTERVAL_MS;

	setBypassPin(_bypassNewState, true);
}

void processByPassState()
{
	if (!_bypassStateIsChanging)
	{
		return;
	}

	// End bypass state changing.
	if (_bypassChangeStateInterval < millis())
	{
		_bypassStateIsChanging = false;
		_bypassState = _bypassNewState;

		setBypassPin(_bypassNewState, false);
		publishData(BypassState);
	}
}

void setBypassPin(DeviceState state, bool isEnable)
{
	if (state == On)
	{
		FunCoilHelper.SetExpanderPin(BYPASS_ON_PIN, isEnable);
	}
	else
	{
		FunCoilHelper.SetExpanderPin(BYPASS_OFF_PIN, isEnable);
	}
}

/**
* @brief Publish topic.
* @param topic A topic title.
* @param payload Data to send.
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

char* valueToStr(SensorData* sensorData, bool sendCurrent)
{
	if (!sensorData->IsExists)
	{
		return (char*)NOT_AVILABLE;
	}

	float val = sendCurrent ? sensorData->Current : sensorData->Average;

	FloatToChars(val, sensorData->Precision, _payloadBuff);
	return _payloadBuff;
}

void findPipeSensors()
{
	_oneWireSensors.begin();
	_oneWireSensors.setResolution(ONEWIRE_TEMPERATURE_PRECISION);

	uint8_t pipeSensorCount = _oneWireSensors.getDeviceCount();
	if (pipeSensorCount > 0)
	{
		DeviceAddress deviceAddress;

		bool isExists = _oneWireSensors.getAddress(deviceAddress, 0);
		InletData.IsExists = isExists;

		if (isExists)
		{
			memcpy(InletData.Address, deviceAddress, 8);
		}
	}
}

void publishAllData()
{
	DeviceData deviceData = (DeviceData)
		(Temperature | DesiredTemp | FanDegree | CurrentMode | CurrentDeviceState | Humidity | InletPipe | BypassState);
	publishData(deviceData, false);
}

void setDeviceMode(char* payload, unsigned int length)
{
	bool isProcessed = false;

	if (isEqual(payload, PAYLOAD_HEAT, length))
	{
		_mode = Heat;
		isProcessed = true;
	}

	if (isEqual(payload, PAYLOAD_COLD, length))
	{
		_mode = Cold;
		isProcessed = true;
	}

	if (isProcessed)
	{
		//SaveConfiguration();
		publishData(CurrentMode);
	}
}