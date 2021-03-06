// MqttSimple.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDiNo Ethernet V2 https://kmpelectronics.eu/products/prodino-ethernet-v2/
// Description:
//		In this example you can see how to work with MQTT server. Through it you can manage relays, gets data from inputs, temperature and humidity sensor.
//      If you found full functionality you would see our example MqttBase.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-ethernet-examples/
// Version: 1.1
// Date: 27.02.2019
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install (Sketch\Include library\Menage Libraries... find ... and click Install):
//         - Simple DHT by Winlin
//         - PubSubClient by Nick O'Leary
//		Connect DHT22 sensor(s) to GROVE connector. Only one we use in this example. 
//			- sensor GROVE_D0, Vcc+, Gnd(-);
//  You should have account in https://www.cloudmqtt.com/ or https://www.cloudamqp.com/ or other MQTT server (your RaspberryPI for example)
// --------------------------------------------------------------------------------
// Topics* (these topics we send to MQTT server):
//   kmp/prodinoeth:[] - the device publishes all data from device:
//     kmp/prodinoeth/isready:[OK]
//     kmp/prodinoeth/relay/1:[off] kmp/prodinoeth/relay/2:[off] kmp/prodinoeth/relay/3:[off] kmp/prodinoeth/relay/4:[off]
//     kmp/prodinoeth/input/1:[off] kmp/prodinoeth/input/2:[off] kmp/prodinoeth/input/3:[off] kmp/prodinoeth/input/4:[off]
//     kmp/prodinoeth/temperature/1[22.0] kmp/prodinoeth/humidity/1[50]
//   kmp/prodinoeth/relay/1:[] - the device publishes data per relay 1:
//     kmp/prodinoeth/relay/1:[]
//   kmp/prodinoeth/relay/1/set:[on] - we use this topic to set relay 1..4 status payload could be "on" or "off":
//     kmp/prodinoeth/relay/1:[on]
//   kmp/prodinoeth/temperature:[] - the device publishes data from first temperature sensor:
//     kmp/prodinoeth/temperature/1:[22.0]
//   kmp/prodinoeth/humidity:[] - the device publishes data from first humidity sensor:
//     kmp/prodinoeth/humidity/1:[50]
// *Legend: every message includes topic (as string) and payload (as binary array). 
//  By easy describe them we use following pattern: "topic:[payload]". If payload was empty we use [].

#include "KmpDinoEthernet.h"
#include "KMPCommon.h"
#include "MqttTopicHelper.h"
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xBD, 0x55, 0x1D };

enum DataType {
	NoneData, AllData, RelayState, AllRelaysState, InputState, AllInputsState, Temperature, Humidity, DeviceIsReady
};

// MQTT server settings. 
//const char* MQTT_SERVER = "xxx.cloudmqtt.com"; // Change it with yours data.
//const int MQTT_PORT = 12345; // Change it with yours data.
//const char* MQTT_USER = "xxxxx"; // Change it with yours data.
//const char* MQTT_PASS = "xxxxx"; // Change it with yours data.
const char* MQTT_SERVER = "m21.cloudmqtt.com"; // Change it with yours data.
const int MQTT_PORT = 13161; // Change it with yours data.
const char* MQTT_USER = "lqnxfvdl"; // Change it with yours data.
const char* MQTT_PASS = "wRvBhKRm55PD"; // Change it with yours data.
const char* MQTT_CLIENT_ID = "ProDinoEthClient";

const char* BASE_TOPIC = "kmp"; // Base topic for all devices in this network. It can use for broadcast devices
const char* DEVICE_TOPIC = "prodinoeth"; // Current device name
const char* RELAY_TOPIC = "relay";
const char* INPUT_TOPIC = "input";
const char* HUMIDITY_TOPIC = "humidity";
const char* TEMPERATURE_TOPIC = "temperature";
// A buffer to send output information.
char _topicBuff[128];

// Ethernet client.
EthernetClient _client;
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
	{ true, SimpleDHT22(OneWirePin), NAN, NAN }
};

const long CHECK_HT_INTERVAL_MS = 5000;
unsigned long _mesureTimeout = 0;

/**
* @brief Setup method. It Arduino executed first and initialize board.
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.println("The example MqttSimple is starting...");

#ifdef DEBUG
	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only. If need debug setup() void.
	}
#endif

	// Init Dino board. Set pins, start W5200.
	DinoInit();

	// Initialize MQTT helper
	MqttTopicHelper.init(BASE_TOPIC, DEVICE_TOPIC, &Serial);

	// Start the Ethernet and takes an IP address form DHCP.
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}

	// Set MQTT callback method
	_mqttClient.setCallback(callback);

	Serial.println("The example MqttSimple is started.");
}

/**
* @brief Loop method. Arduino executed second.
*
* @return void
*/
void loop()
{
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
		// kmp/prodinoeth:NULL
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
		// kmp/prodinoeth/relay/1:On
		MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, RELAY_TOPIC, numBuff);
		topic = _topicBuff;
		payload = GetRelayStatus(num) ? W_ON_S : W_OFF_S;
		break;
	case AllInputsState:
		for (size_t i = 0; i < OPTOIN_COUNT; i++)
			publishTopic(InputState, i, false);
		break;
	case InputState:
		IntToChars(num + 1, numBuff);
		// kmp/prodinoeth/input/1:On
		MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, INPUT_TOPIC, numBuff);
		topic = _topicBuff;
		payload = GetOptoInStatus(num) ? W_ON_S : W_OFF_S;
		break;
	case Temperature:
		IntToChars(num + 1, numBuff);
		// kmp/prodinoeth/temperatura/1:22.1
		MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, TEMPERATURE_TOPIC, numBuff);
		topic = _topicBuff;
		FloatToChars(_measureHT[num].Temperature, 1, payloadBuff);
		payload = payloadBuff;
		break;
	case Humidity:
		IntToChars(num + 1, numBuff);
		// kmp/prodinomkrzero/humidity/1:55
		MqttTopicHelper.buildTopicWithMT(_topicBuff, 2, HUMIDITY_TOPIC, numBuff);
		topic = _topicBuff;
		FloatToChars(_measureHT[num].Humidity, 0, payloadBuff);
		payload = payloadBuff;
		break;
	case DeviceIsReady:
		topic = MqttTopicHelper.getIsReadyTopic();
		payload = W_OK;
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
* @brief Callback method. It executes when has information from subscribed topics: kmp and kmp/prodinoeth/#
*
* @return void
*/
void callback(char* topics, byte* payload, unsigned int payloadLen)
{
	bool payloadEmpty = payloadLen == 0;

	// If the topic doesn't start with kmp/prodinoeth it doesn't need to do.
	if (!MqttTopicHelper.startsWithMainTopic(topics))
		return;

	// Publishing all information: kmp/prodinoeth:[]
	if (MqttTopicHelper.isMainTopic(topics) && payloadEmpty)
	{
		printSubscribeTopic(topics, payload, payloadLen);
		publishTopic(AllData);
		return;
	}

	char nextTopic[32];
	char* otherTopics = nullptr;
	// Get topic after  kmp/prodinoeth/...
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
				if (GetRelayStatus(relNum) != isOn)
					SetRelayStatus(relNum, isOn);
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
		bool relayState = GetRelayStatus(i);
		if (_lastRelayStatus[i] != relayState)
		{
			_lastRelayStatus[i] = relayState;

			publishTopic(RelayState, i);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool inputState = GetOptoInStatus(i);
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
		// Building topic with wildcard symbol: kmp/prodinoeth/#
		// With this topic we are going to subscribe for all topics per device. All topics started with: kmp/prodinoeth
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
