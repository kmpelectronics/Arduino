// MqttSimple.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- ProDino ESP32 GSM v1          https://kmpelectronics.eu/products/prodino-esp32-gsm-v1/
//		- ProDino ESP32 Ethernet GSM v1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
// Description:
//		In this example you can see how to work with MQTT server. Through it you can manage relays, gets data from inputs, temperature and humidity sensor.
//      If you found full functionality you would see our example MqttBase.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 22.07.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install (Sketch\Include library\Menage Libraries... find ... and click Install):
//         - TinyGSM
//         - Simple DHT by Winlin
//         - PubSubClient by Nick O'Leary
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. 
//			- sensor GROVE_D0, Vcc+, Gnd(-);
//  You should have account in https://www.cloudmqtt.com/ or https://www.cloudamqp.com/ or other MQTT server (your RaspberryPI for example)
//	You have to fill fields in arduino_secrets.h file.
// --------------------------------------------------------------------------------
// Topics* (these topics we send to MQTT server):
//   kmp/prodinoesp32:[] - the device publishes all data from device:
//     kmp/prodinoesp32/isready:[OK]
//     kmp/prodinoesp32/relay/1:[off] kmp/prodinoesp32/relay/2:[off] kmp/prodinoesp32/relay/3:[off] kmp/prodinoesp32/relay/4:[off]
//     kmp/prodinoesp32/input/1:[off] kmp/prodinoesp32/input/2:[off] kmp/prodinoesp32/input/3:[off] kmp/prodinoesp32/input/4:[off]
//     kmp/prodinoesp32/temperature/1[22.0] kmp/prodinoesp32/humidity/1[50]
//   kmp/prodinoesp32/relay/1:[] - the device publishes data per relay 1:
//     kmp/prodinoesp32/relay/1:[]
//   kmp/prodinoesp32/relay/1/set:[on] - we use this topic to set relay 1..4 status payload could be "on" or "off":
//     kmp/prodinoesp32/relay/1:[on]
//   kmp/prodinoesp32/temperature:[] - the device publishes data from first temperature sensor:
//     kmp/prodinoesp32/temperature/1:[22.0]
//   kmp/prodinoesp32/humidity:[] - the device publishes data from first humidity sensor:
//     kmp/prodinoesp32/humidity/1:[50]
// *Legend: every message includes topic (as string) and payload (as binary array). 
//  By easy describe them we use following pattern: "topic:[payload]". If payload was empty we use [].

#include "KMPProDinoESP32.h"

#include "KMPCommon.h"
#include "MqttTopicHelper.h"
#include "arduino_secrets.h"
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>

#include <WiFi.h>
#include <WiFiClient.h>

enum DataType {
	NoneData, AllData, RelayState, AllRelaysState, InputState, AllInputsState, Temperature, Humidity, DeviceIsReady
};

const char* MQTT_CLIENT_ID = "ProDinoESP32Client";

const char* BASE_TOPIC = "kmp"; // Base topic for all devices in this network. It can use for broadcast devices
const char* DEVICE_TOPIC = "prodinoesp32"; // Current device name
const char* RELAY_TOPIC = "relay";
const char* INPUT_TOPIC = "input";
const char* HUMIDITY_TOPIC = "humidity";
const char* TEMPERATURE_TOPIC = "temperature";
// A buffer to send output information.
char _topicBuff[128];

// It supports work with GSM Modem.
TinyGsm _modem(SerialModem);
TinyGsmClient _client(_modem);

// Declare a MQTT client.
PubSubClient _mqttClient(MQTT_SERVER, MQTT_PORT, _client);

// There arrays store last states by relay and optical isolated inputs.
bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };

// Define sensors structure.
struct MeasureHT_t {
	bool IsEnable;
	SimpleDHT22 dht;
	float Humidity;
	float Temperature;
};

#define SENSOR_COUNT 1
MeasureHT_t _measureHT[SENSOR_COUNT] = {
	{ true, SimpleDHT22(GROVE_D0), NAN, NAN }
};

const long CHECK_HT_INTERVAL_MS = 5000;
unsigned long _mesureTimeout = 0;

/**
* @brief Setup void. It Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example simple MQTT with GSM communication is starting...");

	// Init Dino board. Set pins, start GSM.
	KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	KMPProDinoESP32.setStatusLed(blue);
	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoESP32.rs485Begin(19200);

	Serial.println("Initializing GSM modem...");
	_modem.init();

	String modemInfo = _modem.getModemInfo();
	if (modemInfo == "")
	{
		Serial.println("GSM modem is not started!!!");
		while (true) {}
	}
	Serial.print("Modem info: ");
	Serial.println(modemInfo);

	// Unlock your SIM card if it locked with a PIN code. 
	// If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
	if (strlen(SECRET_PINNUMBER) > 0 && !_modem.simUnlock(SECRET_PINNUMBER))
	{
		Serial.println("SIM card PIN code is not valid! STOP!!!");
		while (true) {}
	}

	while (!_modem.gprsConnect(SECRET_GPRS_APN, SECRET_GPRS_LOGIN, SECRET_GPRS_PASSWORD))
	{
		Serial.print("Can not connect to APN: ");
		Serial.print(SECRET_GPRS_APN);
		Serial.println(" waiting for 5 sec.");
		delay(5000);
	}
	Serial.println("GSM GPRS is connected.");

	// Initialize MQTT helper
	MqttTopicHelper.init(BASE_TOPIC, DEVICE_TOPIC, &Serial);

	// Set MQTT callback method
	_mqttClient.setCallback(callback);

	Serial.println("The example MqttBase is started.");

	KMPProDinoESP32.offStatusLed();
}

/**
* @brief Loop method. Arduino executed second.
*
* @return void
*/
void loop()
{
	KMPProDinoESP32.processStatusLed(green, 1000);

	// Checking is device connected to MQTT server.
	if (!ConnectMqtt())
	{
		return;
	}

	_mqttClient.loop();

	PublishChangedData();
	PublishSensorData();
}

/**
* @brief This method publishes all data per device.
* @dataType Type of data which will be publish.
* @num device number, if need for publish this topic.
* @isPrintPublish is print Publish. 
*
* @return void
*/
void publishTopic(DataType dataType, int num = 0, bool isPrintPublish = true)
{
	if (isPrintPublish)
	{
		Serial.println("Publish");
	}

	const char * topic = NULL;
	const char * payload = NULL;
	char numBuff[8];
	char payloadBuff[16];

	switch (dataType)
	{
		case AllData:
			// kmp/prodinoesp32:NULL
			publishTopic(DeviceIsReady, 0, false);
			publishTopic(AllRelaysState, 0, false);
			publishTopic(AllInputsState, 0, false);
			publishTopic(Temperature, 0, false);
			publishTopic(Humidity, 0, false);
			break;
		case AllRelaysState:
			for (size_t i = 0; i < RELAY_COUNT; i++)
				publishTopic(RelayState, i, false);
			break;
		case RelayState:
			IntToChars(num + 1, numBuff);
			// kmp/prodinoesp32/relay/1:On
			MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, RELAY_TOPIC, numBuff);
			topic = _topicBuff;
			payload = KMPProDinoESP32.getRelayState(num) ? W_ON_S : W_OFF_S;
			break;
		case AllInputsState:
			for (size_t i = 0; i < OPTOIN_COUNT; i++)
				publishTopic(InputState, i, false);
			break;
		case InputState:
			IntToChars(num + 1, numBuff);
			// kmp/prodinoesp32/input/1:On
			MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, INPUT_TOPIC, numBuff);
			topic = _topicBuff;
			payload = KMPProDinoESP32.getOptoInState(num) ? W_ON_S : W_OFF_S;
			break;
		case Temperature:
			IntToChars(num + 1, numBuff);
			// kmp/prodinoesp32/temperatura/1:22.1
			MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, TEMPERATURE_TOPIC, numBuff);
			topic = _topicBuff;
			FloatToChars(_measureHT[num].Temperature, 1, payloadBuff);
			payload = payloadBuff;
			break;
		case Humidity:
			IntToChars(num + 1, numBuff);
			// kmp/prodinoesp32/humidity/1:55
			MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, HUMIDITY_TOPIC, numBuff);
			topic = _topicBuff;
			FloatToChars(_measureHT[num].Humidity, 0, payloadBuff);
			payload = payloadBuff;
			break;
		case DeviceIsReady:
			topic = MqttTopicHelper.getIsReadyTopic();
			payload = "OK";
			break;
		default:
			break;
	}

	if (topic != NULL)
	{
		MqttTopicHelper.printTopicAndPayload(topic, payload);
		_mqttClient.publish(topic, payload);
	}
}

/**
* @brief Print in debug console Subscribed topic and payload.
*
* @return void
*/
void printSubscribeTopic(char* topic, byte* payload, unsigned int length)
{
	Serial.println("Subscribe");
	MqttTopicHelper.printTopicAndPayload(topic, payload, length);
}

/**
* @brief Callback method. It executes when has information from subscribed topics: kmp and kmp/prodinoesp32/#
*
* @return void
*/
void callback(char* topics, byte* payload, unsigned int payloadLen)
{
	bool payloadEmpty = payloadLen == 0;

	// If the topic doesn't start with kmp/prodinoesp32 it doesn't need to do.
	if (!MqttTopicHelper.startsWithMainTopic(topics))
		return;

	// Publishing all information: kmp/prodinoesp32:[]
	if (MqttTopicHelper.isMainTopic(topics) && payloadEmpty)
	{
		printSubscribeTopic(topics, payload, payloadLen);
		publishTopic(AllData);
		return;
	}

	char nextTopic[32];
	char* otherTopics = nullptr;
	// Get topic after  kmp/prodinoesp32/...
	if (!MqttTopicHelper.getNextTopic(topics, nextTopic, &otherTopics, true))
		return;

	// relay/...
	if (isEqual(nextTopic, RELAY_TOPIC))
	{
		uint8_t relNum;
		// [relay] /1 or [relay] /1/set
		if (!MqttTopicHelper.getNextTopic(otherTopics, nextTopic, &otherTopics) || !atoUint8(nextTopic, &relNum) || relNum <= 0 || relNum > RELAY_COUNT)
			return;

		--relNum;
		// relay/1
		if (otherTopics[0] == CH_NONE && payloadEmpty)
		{
			printSubscribeTopic(topics, payload, payloadLen);
			publishTopic(RelayState, relNum);
			return;
		}

		// relay/1/set
		if (MqttTopicHelper.isTopicSet(otherTopics))
		{
			char payloadStr[128];
			strNCopy(payloadStr, (const char*)payload, payloadLen > 128 ? 128 : payloadLen);

			// Checking payload is On or Off.
			bool isOn = isEqual(payloadStr, W_ON_S);
			if (isOn || isEqual(payloadStr, W_OFF_S))
			{
				printSubscribeTopic(topics, payload, payloadLen);
				// Set relay new state.
				if (KMPProDinoESP32.getRelayState(relNum) != isOn)
					KMPProDinoESP32.setRelayState(relNum, isOn);
				else
					// Publish current relay state.
					publishTopic(RelayState, relNum);
			}
		}
		return;
	}
}

/**
* @brief Publishing information for relays and inputs if they changed.
*
* @return void
*/
void PublishChangedData()
{
	// Get current Opto input and relay statuses.
	for (byte i = 0; i < RELAY_COUNT; i++)
	{
		bool relayState = KMPProDinoESP32.getRelayState(i);
		if (_lastRelayStatus[i] != relayState)
		{
			_lastRelayStatus[i] = relayState;

			publishTopic(RelayState, i);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool inputState = KMPProDinoESP32.getOptoInState(i);
		if (_lastOptoInStatus[i] != inputState)
		{
			_lastOptoInStatus[i] = inputState;

			publishTopic(InputState, i);
		}
	}
}

/**
* @brief Publishing information for sensors if they changed.
*
* @return void
*/
void PublishSensorData()
{
	if (millis() > _mesureTimeout)
	{
		for (uint8_t i = 0; i < SENSOR_COUNT; i++)
		{
			// Get sensor structure.
			MeasureHT_t* measureHT = &_measureHT[i];
			// Is enable - read data from sensor.
			if (!measureHT->IsEnable)
			{
				continue;
			}

			float humidity = NAN;
			float temperature = NAN;
			measureHT->dht.read2(&temperature, &humidity, NULL);

			humidity = roundF(humidity, 0);
			if (humidity != measureHT->Humidity)
			{
				measureHT->Humidity = humidity;
				publishTopic(Humidity, i);
			}

			if (temperature != measureHT->Temperature)
			{
				measureHT->Temperature = temperature;
				publishTopic(Temperature, i);
			}
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

/**
* @brief Checking if device connected to MQTT server. When it connects add subscribe topics per device.
*
* @return void
*/
bool ConnectMqtt()
{
	if (_mqttClient.connected())
	{
		return true;
	}

	Serial.println("Attempting to connect MQTT...");

	if (_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
	{
		// It is broadcast topic: kmp
		_mqttClient.subscribe(BASE_TOPIC);
		// Building topic with wildcard symbol: kmp/prodinoesp32/#
		// With this topic we are going to subscribe for all topics per device. All topics started with: kmp/prodinoesp32
		MqttTopicHelper.buildTopicWithMT(_topicBuff, 1, "#");
		_mqttClient.subscribe(_topicBuff);

		Serial.println("Connected.");
		publishTopic(DeviceIsReady);

		return true;
	}

	Serial.print("failed, rc=");
	Serial.print(_mqttClient.state());
	Serial.println(" try again after 5 seconds...");
	// Wait 5 seconds before retrying
	delay(5000);

	return false;
}