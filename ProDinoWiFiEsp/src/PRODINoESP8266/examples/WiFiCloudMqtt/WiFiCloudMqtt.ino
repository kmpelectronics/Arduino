// WiFiCloudMqtt.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//    Cloud MQTT example. In this example we show how to connect KMP ProDino WiFi-ESP WROOM-02 with Amazon cloudmqtt.com service.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebrelayserverap.aspx
// Version: 1.0.0
// Date: 04.06.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi authentication settings.
const char* SSID = "xxxx";
const char* SSID_PASSWORD = "xxxx";

// MQTT server settings. 
const char* MQTT_SERVER = "xxx.cloudmqtt.com"; // xxx should be change with true value.
const int MQTT_PORT = 13161;
const char* MQTT_CLIENT_ID = "ESP8266Client";
const char* MQTT_USER = "xxxxx"; // xxxxx should be change with true value.
const char* MQTT_PASS = "xxxxx"; // xxxxx should be change with true value.

const char* TOPIC_INFO = "/KMP/ProDinoWiFi/Info";
const char* TOPIC_COMMAND = "/KMP/ProDinoWiFi/Cmd";
const char* CMD_REL = "rel";
const char* CMD_OPTOIN = "optoIn";
const char CMD_SEP = ':';

// Declares a ESP8266WiFi client.
WiFiClient _wifiClient;
// Declare a MQTT client.
PubSubClient _mqttClient(MQTT_SERVER, MQTT_PORT, _wifiClient);

// There arrays store last states by relay and optical isolated inputs.
bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };

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
	Serial.print("Topic [");
	Serial.print(topic);
	Serial.println("]");

	// Check topic.
	if (strncmp(TOPIC_COMMAND, topic, strlen(TOPIC_COMMAND)) != 0)
	{
		Serial.println("Is not valid.");
		return;
	}

	Serial.println("Payloud:");

	for (uint i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println();

	// Command structure: [command (rel):relay number (0..3):relay state (0 - Off, 1 - On)]. Example: rel:0:0 
	size_t cmdRelLen = strlen(CMD_REL);

	if (strncmp(CMD_REL, (const char*)payload, cmdRelLen) == 0 && length >= cmdRelLen + 4)
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
	PublishInformation();
}

/**
* @brief Publish information in MQTT server.
*
* @return void
*/
void PublishInformation()
{
	// Get current Opto input and relay statuses.
	for (byte i = 0; i < RELAY_COUNT; i++)
	{
		bool rState = KMPDinoWiFiESP.GetRelayState(i);
		if (_lastRelayStatus[i] != rState)
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
		bool rState = KMPDinoWiFiESP.GetOptoInState(i);
		if (_lastOptoInStatus[i] != rState)
		{
			_lastOptoInStatus[i] = rState;
			buildPayload(_payload, CMD_OPTOIN, CMD_SEP, i, rState);
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
	if (!_mqttClient.connected())
	{
		Serial.println("Attempting MQTT connection...");

		if (_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
		{
			Serial.println("Connected.");
			_mqttClient.subscribe(TOPIC_COMMAND);
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
}