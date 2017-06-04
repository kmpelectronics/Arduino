// WiFiMqttClient.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//    Local Mosquitto MQTT server example. In this example we show how to connect KMP ProDino WiFi-ESP WROOM-02 with Mosquitto (https://mosquitto.org) server worked in local network.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebrelayserverap.aspx
// Version: 1.0.0
// Date: 04.06.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* SSID = "xxxx";
const char* SSID_PASSWORD = "xxxx";

const char* MQTT_SERVER_IP = "192.168.0.4"; // Add local Mosquitto server IP address.

const char* TOPIC_SEND_INFO = "/KMP/ProDinoWiFi/Info";
const char* TOPIC_COMMAND = "/KMP/ProDinoWiFi/Cmd";
const char* CMD_REL = "rel";
const char* CMD_OPTOIN = "optoIn";
const char CMD_SEP = ':';

WiFiClient _wifiClient;
PubSubClient _mqttClient(_wifiClient);

bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };
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

	Serial.println("KMP Mqtt client example.\r\n");
	// Connect to WiFi network
	WiFi.begin(SSID, SSID_PASSWORD);
	Serial.print("\n\rWorking to connect");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("KMP MQTT Client");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	// Initialize MQTT.
	_mqttClient.setServer(MQTT_SERVER_IP, 1883);
	_mqttClient.setCallback(callback);
}

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
	if (!_mqttClient.connected()) {
		reconnect();
	}
	_mqttClient.loop();

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
			_mqttClient.publish(TOPIC_SEND_INFO, (const char*)_payload);
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
			_mqttClient.publish(TOPIC_SEND_INFO, (const char*)_payload);
		}
	}

	//long now = millis();
	//if (now - lastMsg > 2000) {
	//  lastMsg = now;
	//  ++value;
	//  snprintf(msg, 75, "hello world #%ld", value);
	//  Serial.print("Publish message: ");
	//  Serial.println(msg);
	//  _mqttClient.publish("outTopic", msg);
	//}
}

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

void reconnect() {
	// Loop until we're reconnected
	while (!_mqttClient.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (_mqttClient.connect("ESP8266Client")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			//client.publish("outTopic", "hello world");
			// ... and resubscribe
			_mqttClient.subscribe(TOPIC_COMMAND);
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(_mqttClient.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}