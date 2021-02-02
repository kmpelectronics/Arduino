// WiFiCloudMqtt.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//    KMP PRODINo WIFI-ESP WROOM-02 https://kmpelectronics.eu/products/prodino-wifi-esp-wroom-02-v1/
// Description:
//    Cloud MQTT example. In this example we show how to connect KMP PRODINo WIFI-ESP WROOM-02 with MQTT.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-wifi-examples/
// Version: 1.1.0
// Date: 04.01.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//  You have to fill your credentials in arduino_secrets.h file
//	Before start this example you need to install (Sketch\Include library\Menage Libraries... find ... and click Install):
//         - PubSubClient by Nick O'Leary
//  You should have account in https://www.cloudmqtt.com/ or https://www.cloudamqp.com/ or other MQTT server (your RaspberryPI for example)
// --------------------------------------------------------------------------------
// Topics* (these topics we send to MQTT server):
// kmp/prodinowifi/info [] - the device publishes all data
// -> kmp/prodinowifi/info [rel:0:0] or kmp/prodinowifi/info:[rel:0:1]
// ... 
// -> kmp/prodinowifi/info [rel:3:0] or kmp/prodinowifi/info:[rel:3:1]
// -> kmp/prodinowifi/info [in:0:0] or kmp/prodinowifi/info:[in:0:1]
// ... 
// -> kmp/prodinowifi/info [in:3:0] or kmp/prodinowifi/info:[in:3:1]
// kmp/prodinowifi/cmd [rel:0:0] - set relay 0 to off
// -> kmp/prodinowifi/info [rel:0:0]
// kmp/prodinowifi/cmd [rel:0:1] - set relay 0 to on
// -> kmp/prodinowifi/info [rel:0:1]
// ...
// kmp/prodinowifi/cmd [rel:3:0] - set relay 3 to off
// -> kmp/prodinowifi/info [rel:3:0]
// kmp/prodinowifi/cmd [rel:3:1] - set relay 3 to on
// -> kmp/prodinowifi/info [rel:3:1]
// *Legend: every message includes topic (as string) and payload (as binary array). 
//  For easy description we use follow pattern: "topic [payload]". If the payload is empty we use [].
//  In the first row we describe the command and on the row which started with -> information sent from the device

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

const char* MQTT_CLIENT_ID = "ESP8266Client";

const char* TOPIC_INFO = "kmp/prodinowifi/info";
const char* TOPIC_COMMAND = "kmp/prodinowifi/cmd";
const char* CMD_REL = "rel";
const char* CMD_OPTOIN = "in";
const char CMD_SEP = ':';

bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };

// Declares a ESP8266WiFi client.
WiFiClient _wifiClient;
// Declare a MQTT client.
PubSubClient _mqttClient(MQTT_SERVER, MQTT_PORT, _wifiClient);

// Buffer by send output state.
char _payload[16];

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

	// Initialize MQTT.
	_mqttClient.setCallback(callback);
}

/**
* @brief Callback method. It is fire when has information in subscribed topic.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {
	//Serial.print("Topic [");
	//Serial.print(topic);
	//Serial.println("]");
	//Serial.print("Payload length: ");
	//Serial.print(length);
	//Serial.println();
	//Serial.println("Payload:");
	//for (uint i = 0; i < length; i++)
	//{
	//	Serial.print((char)payload[i]);
	//}
	//Serial.println();

	// Check topic.
	if (strncmp(TOPIC_INFO, topic, strlen(TOPIC_INFO)) == 0)
	{
		if (length == 0)
		{
			sendStatuses(true);
		}

		return;
	}

	if (strncmp(TOPIC_COMMAND, topic, strlen(TOPIC_COMMAND)) != 0)
	{
		return;
	}

	// Command structure: [command (rel):relay number (0..3):relay state (0 - Off, 1 - On)]. Example: rel:0:0 
	uint cmdRelLen = strlen(CMD_REL);
	if (strncmp(CMD_REL, (const char*)payload, strlen(CMD_REL)) == 0 && length >= cmdRelLen + 4)
	{
		KMPDinoWiFiESP.SetRelayState(CharToInt(payload[4]), CharToInt(payload[6]) == 1);
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
	if (!ConnectWiFi() || !ConnectMqtt())
	{
		return;
	}

	_mqttClient.loop();

	// Publish information in MQTT.
	sendStatuses(false);
}

/**
* @brief Publish information in MQTT server.
*
* @return void
*/
void sendStatuses(bool force)
{
	for (byte i = 0; i < RELAY_COUNT; i++)
	{
		bool rState = KMPDinoWiFiESP.GetRelayState(i);
		if (_lastRelayStatus[i] != rState || force)
		{
			_lastRelayStatus[i] = rState;
			buildPayload(_payload, CMD_REL, CMD_SEP, i, rState);
			Serial.print("Publish message: ");
			Serial.println(_payload);
			_mqttClient.publish(TOPIC_INFO, (const char*)_payload);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool inState = KMPDinoWiFiESP.GetOptoInState(i);
		if (_lastOptoInStatus[i] != inState || force)
		{
			_lastOptoInStatus[i] = inState;
			buildPayload(_payload, CMD_OPTOIN, CMD_SEP, i, inState);
			Serial.print("Publish message: ");
			Serial.println(_payload);
			_mqttClient.publish(TOPIC_INFO, (const char*)_payload);
		}
	}
}

/**
* @brief Build publish payload.
* @param buffer where fill payload.
* @param command description
* @param number device number
* @param state device state
* @return void
*/
void buildPayload(char* buffer, const char* command, char separator, byte number, bool state)
{
	int cmdLen = strlen(command);
	memcpy(buffer, command, cmdLen);
	buffer[cmdLen++] = separator;
	buffer[cmdLen++] = IntToChar(number);
	buffer[cmdLen++] = separator;
	buffer[cmdLen++] = state ? '1' : '0';
	buffer[cmdLen] = '\0';
}

/**
* @brief Connect to WiFi access point.
*
* @return bool true - success.
*/
bool ConnectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.print("Connecting [");
		Serial.print(SSID);
		Serial.println("]...");

		WiFi.begin(SSID, SSID_PASSWORD);

		if (WiFi.waitForConnectResult() != WL_CONNECTED)
		{
			return false;
		}

		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());

		return true;
	}
}

/**
* @brief Connect to MQTT server.
*
* @return bool true - success.
*/
bool ConnectMqtt()
{
	if (_mqttClient.connected())
	{
		return true;
	}

	Serial.println("Attempting MQTT connection...");

	bool isConnected = false;
	if (strlen(MQTT_USER) == 0 && strlen(MQTT_PASS))
	{
		isConnected = _mqttClient.connect(MQTT_CLIENT_ID);
	}
	else
	{
		isConnected = _mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
	}

	if (!isConnected)
	{
		Serial.print("failed, rc=");
		Serial.print(_mqttClient.state());
		Serial.println(" try again after 5 seconds");
		// Wait 5 seconds before retrying
		delay(5000);
		return false;
	}

	Serial.println("Connected.");
	_mqttClient.subscribe(TOPIC_INFO);
	_mqttClient.subscribe(TOPIC_COMMAND);

	sendStatuses(true);

	return true;
}