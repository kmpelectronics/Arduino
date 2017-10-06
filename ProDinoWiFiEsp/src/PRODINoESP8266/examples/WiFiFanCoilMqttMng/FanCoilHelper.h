// FanCoilHelper.h

#ifndef _FANCOILHELPER_h
#define _FANCOILHELPER_h

#include <FS.h>
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>          // Install with Library Manager. "WiFiManager by tzapu" https://github.com/tzapu/WiFiManager

// Uncomment to enable printing out nice debug messages.
#define WIFIFCMM_DEBUG

// Define where debug output will be printed.
#define DEBUG_FC Serial

// Setup debug printing macros.
#ifdef WIFIFCMM_DEBUG
#define DEBUG_FC_PRINT(...) { DEBUG_FC.print(__VA_ARGS__); }
#define DEBUG_FC_PRINTLN(...) { DEBUG_FC.println(__VA_ARGS__); }
#else
#define DEBUG_FC_PRINT(...) {}
#define DEBUG_FC_PRINTLN(...) {}
#endif

#define FAN_SWITCH_LEVEL_LEN 3

#define MQTT_SERVER_LEN 40
#define MQTT_PORT_LEN 8
#define MQTT_CLIENT_ID_LEN 32
#define MQTT_USER_LEN 16
#define MQTT_PASS_LEN 16
#define BASE_TOPIC_LEN 32
#define INLET_SENSOR_CRC_LEN 16

#define TEMPERATURE_ARRAY_LEN 10
#define TEMPERATURE_PRECISION 1
#define CHECK_TEMP_INTERVAL_MS 5000

#define HUMIDITY_ARRAY_LEN 5
#define HUMIDITY_PRECISION 0
#define CHECK_HUMIDITY_INTERVAL_MS 10000

#define INLET_ARRAY_LEN 5
#define INLET_PRECISION 1
#define CHECK_INLET_INTERVAL_MS CHECK_TEMP_INTERVAL_MS

#define PING_INTERVAL_MS 30000

#define MIN_DIFFERENCE_TEMPERATURE 5

#define MIN_DESIRED_TEMPERATURE 15.0
#define MAX_DESIRED_TEMPERATURE 30.0

#define DHT_SENSORS_PIN EXT_GROVE_D0
#define DHT_SENSORS_TYPE DHT22

// Thermometer Resolution in bits. http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf page 8. 
// Bits - CONVERSION TIME. 9 - 93.75ms (0.5°C), 10 - 187.5ms (0.25°C), 11 - 375ms (0.125°C), 12 - 750ms (0.0625°C). 
#define ONEWIRE_TEMPERATURE_PRECISION 10
#define ONEWIRE_SENSORS_PIN EXT_GROVE_D1

const char MQTT_SERVER_KEY[] = "mqttServer";
const char MQTT_PORT_KEY[] = "mqttPort";
const char MQTT_CLIENT_ID_KEY[] = "mqttClientId";
const char MQTT_USER_KEY[] = "mqttUser";
const char MQTT_PASS_KEY[] = "mqttPass";
const char BASE_TOPIC_KEY[] = "baseTopic";
const char INLET_SENSOR_KEY[] = "inletSensorCRC";
const char MODE_KEY[] = "mode";
const char CONFIG_FILE_NAME[] = "/config.json";

const char TOPIC_SEPARATOR[] = "/";
const char TOPIC_HUMIDITY[] = "humidity";
const char TOPIC_DESIRED_TEMPERATURE[] = "desiredtemp";
const char TOPIC_TEMPERATURE[] = "temperature";
const char TOPIC_SET[] = "set";
const char TOPIC_MODE[] = "mode";
const char TOPIC_DEVICE_STATE[] = "state";
const char TOPIC_FAN_DEGREE[] = "fandegree";
const char TOPIC_INLET_TEMPERATURE[] = "inlettemp";
const char PAYLOAD_HEAT[] = "heat";
const char PAYLOAD_COLD[] = "cold";
const char PAYLOAD_ON[] = "on";
const char PAYLOAD_OFF[] = "off";
const char PAYLOAD_READY[] = "ready";
const char PAYLOAD_PING[] = "ping";

const char EVERY_ONE_LEVEL_TOPIC[] = "+";
const char NOT_AVILABLE[] = "N/A";

const char MUST_BE_ONE[] = "Must be one";

const float FAN_SWITCH_LEVEL[FAN_SWITCH_LEVEL_LEN] = { 0, 0.5, 1.0 };

enum Mode
{
	Heat = 0,
	Cold = 1
};

enum DeviceState
{
	Off = 0,
	On  = 1
};

enum DeviceData
{
	Temperature = 1,
	DesiredTemp = 2,
	InletPipe = 4,
	FanDegree = 8,
	CurrentMode = 16,
	CurrentDeviceState = 32,
	Humidity = 64,
	DeviceIsReady = 128,
	DevicePing = 256
};

struct DeviceSettings
{
	char MqttServer[MQTT_SERVER_LEN] = "x.cloudmqtt.com";
	char MqttPort[MQTT_PORT_LEN] = "1883";
	char MqttClientId[MQTT_CLIENT_ID_LEN] = "ESP8266Client";
	char MqttUser[MQTT_USER_LEN];
	char MqttPass[MQTT_PASS_LEN];
	char BaseTopic[BASE_TOPIC_LEN] = "flat/bedroom1";
};

struct SensorData
{
	// Current measured value
	float Current;
	// Average value calculated from all collected data in DataCollection
	float Average;
	// An array which stores collected data
	float* DataCollection;
	// The array length
	uint DataCollectionLen;
	// Current array position
	uint8_t CurrentCollectPos = 0;
	// Next time for check value
	unsigned long CheckInterval;
	// Precision with which round measured value
	uint Precision;
	// Check value interval
	uint CheckDataIntervalMS;
	// Stored in this structure data type
	DeviceData DataType;
	// DS18B20 device address
	uint8_t Address[8];
	// Is true, if the sensor exists
	bool IsExists;
};

#ifdef WIFIFCMM_DEBUG
void printTopicAndPayload(const char* operationName, const char* topic, char* payload, unsigned int length);
#endif

bool connectWiFi();

float calcAverage(float* data, uint8 dataLength, uint8 precision);

void ReadConfiguration(DeviceSettings* settings);
bool mangeConnectParamers(WiFiManager* wifiManager, DeviceSettings* settings);
void SaveConfiguration(DeviceSettings* settings);
void saveConfigCallback();

void setArrayValues(SensorData* sensor);

#endif
