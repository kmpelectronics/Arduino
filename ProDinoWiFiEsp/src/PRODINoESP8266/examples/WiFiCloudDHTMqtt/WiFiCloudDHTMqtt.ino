// WiFiCloudDHTMqtt.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//    Cloud MQTT example with DHT support. In this example we show how to connect KMP ProDino WiFi-ESP WROOM-02 with Amazon cloudmqtt.com service 
//		and measure humidity and temperature with DHT22 sensor. For connect with WiFi we use WiFiManager.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebrelayserverap.aspx
// Version: 1.0.0
// Date: 04.06.2017
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

// MQTT server settings.
const char* MQTT_SERVER = "xxx.cloudmqtt.com"; // Your xxx.cloudmqtt.com server.
const int MQTT_PORT = 13161;                   // Your server port.
const char* MQTT_CLIENT_ID = "ESP8266Client";  // Default server name.
const char* MQTT_USER = "xxxxxxxx";            // Your xxxxxxxx MQTT server user.
const char* MQTT_PASS = "xxxxxxxxxxxx";        // Your xxxxxxxxxxxx MQTT server user password.

const char* TOPIC_INFO = "/KMP/ProDinoWiFi/Info";
const char* TOPIC_INFO_DHT_T = "/KMP/ProDinoWiFi/Info/dhtt";
const char* TOPIC_INFO_DHT_H = "/KMP/ProDinoWiFi/Info/dhth";
const char* TOPIC_COMMAND = "/KMP/ProDinoWiFi/Cmd";
const char* CMD_ALL = "all";
const char* CMD_REL = "rel";
const char* CMD_OPTOIN = "optoIn";
const char* CMD_DHT_H = "dht";
const char* CMD_DHT_T = "dht";
const char CMD_SEP = ':';

DHT _dhtSensor(EXT_GROVE_D0, DHT22, 11);
// Contains last measured humidity from sensor.
float _humidity;
// Contains last measured temperature from sensor.
float _temperature;

// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 10000;
// Store last measure time.
unsigned long _mesureTimeout;

// Declares a ESP8266WiFi client.
WiFiClient _wifiClient;
// Declare a MQTT client.
PubSubClient _mqttClient(MQTT_SERVER, MQTT_PORT, _wifiClient);

// There arrays store last states by relay and optical isolated inputs.
bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };

// Buffer by send output state.
char _payload[16];
bool _sendAllData;

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

	//WiFiManager
	//Local initialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;

	// Is OptoIn 4 is On the board is resetting WiFi configuration.
	if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
	{
		Serial.println("Resetting WiFi configuration...\r\n");
		//reset saved settings
		wifiManager.resetSettings();
		Serial.println("WiFi configuration was reseted.\r\n");
	}

	//fetches ssid and pass from eeprom and tries to connect
	//if it does not connect it starts an access point with the specified name
	//auto generated name ESP + ChipID
	wifiManager.autoConnect();

	// Initialize MQTT.
	_mqttClient.setCallback(callback);

	_sendAllData = true;
}

/**
* @brief Callback method. It is fire when has information in subscribed topic.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Subscribed topic [");
	Serial.print(topic);
	Serial.print("]");

	// Check topic.
	if (strncmp(TOPIC_COMMAND, topic, strlen(TOPIC_COMMAND)) != 0)
	{
		Serial.println("Is not valid.");
		return;
	}

	Serial.print(" payload [");

	for (uint i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}
	Serial.println("]");

	// Command send all data.
	if (strncmp((const char*)payload, CMD_ALL, strlen(CMD_ALL)) == 0)
	{
		_sendAllData = true;
		return;
	}

	// Relay command.
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
* @brief Publish information in the MQTT server.
*
* @return void
*/
void PublishInformation()
{
	char state[2];
	state[1] = '\0';
	// Get current Opto input and relay statuses.
	for (byte i = 0; i < RELAY_COUNT; i++)
	{
		bool rState = KMPDinoWiFiESP.GetRelayState(i);
		if (_lastRelayStatus[i] != rState || _sendAllData)
		{
			_lastRelayStatus[i] = rState;
			state[0] = rState ? '1' : '0';
			buildPayload(_payload, CMD_REL, CMD_SEP, i, state);
			Publish(TOPIC_INFO, _payload);
		}
	}

	for (byte i = 0; i < OPTOIN_COUNT; i++)
	{
		bool oiState = KMPDinoWiFiESP.GetOptoInState(i);
		if (_lastOptoInStatus[i] != oiState || _sendAllData)
		{
			_lastOptoInStatus[i] = oiState;
			state[0] = oiState ? '1' : '0';
			buildPayload(_payload, CMD_OPTOIN, CMD_SEP, i, state);

			Publish(TOPIC_INFO, _payload);
		}
	}

	GetDHTSensorData();

	_sendAllData = false;
}

/**
* @brief Read data from sensors a specified time.
*
* @return void
*/
void GetDHTSensorData()
{
	if (millis() > _mesureTimeout || _sendAllData)
	{
		_dhtSensor.read(true);
		float humidity = _dhtSensor.readHumidity();
		float temperature = _dhtSensor.readTemperature();

		if (_humidity != humidity || _sendAllData)
		{
			FloatToChars(humidity, 1, _payload);
			_humidity = humidity;
			Publish(TOPIC_INFO_DHT_H, _payload);
		}

		if (_temperature != temperature || _sendAllData)
		{
			FloatToChars(temperature, 1, _payload);
			_temperature = temperature;
			Publish(TOPIC_INFO_DHT_T, _payload);
		}

		// Set next time to read data.
		_mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
	}
}

/**
* @brief Build publish payload.
* @param buffer where fill payload.
* @param command description
* @param number device number
* @param state device state
*
* @return void
*/
void buildPayload(char* buffer, const char* command, char separator, byte number, const char* state)
{
	int cmdLen = strlen(command);
	memcpy(buffer, command, cmdLen);
	buffer[cmdLen++] = separator;
	buffer[cmdLen++] = IntToChar(number);
	buffer[cmdLen++] = separator;
	buffer += cmdLen;
	int stLen = strlen(state);
	memcpy(buffer, state, stLen);
	buffer[stLen] = '\0';
}

/**
* @brief Publish topic.
* @param topic title.
* @param payload data to send
*
* @return void
*/
void Publish(const char* topic, char* payload)
{
	Serial.print("Publish topic [");
	Serial.print(topic);
	Serial.print("] payload [");
	Serial.print(_payload);
	Serial.println("]");

	_mqttClient.publish(topic, (const char*)_payload);
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
		Serial.print("Reconnecting [");
		Serial.print(WiFi.SSID());
		Serial.println("]...");

		WiFi.begin();
		//WiFi.begin(SSID, SSID_PASSWORD);

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

	return _mqttClient.connected();
}