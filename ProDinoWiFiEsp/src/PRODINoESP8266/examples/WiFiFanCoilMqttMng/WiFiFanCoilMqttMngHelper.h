// WiFiFanCoilMqttMngHelper.h

#ifndef _WIFIFANCOILMQTTMNGHELPER_h
#define _WIFIFANCOILMQTTMNGHELPER_h

#include "Arduino.h"

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

const uint8_t FAN_SWITCH_LEVEL_LEN = 3;

const uint8_t MQTT_SERVER_LEN = 40;
const uint8_t MQTT_PORT_LEN = 8;
const uint8_t MQTT_CLIENT_ID_LEN = 32;
const uint8_t MQTT_USER_LEN = 16;
const uint8_t MQTT_PASS_LEN = 16;
const uint8_t BASE_TOPIC_LEN = 32;
const uint8_t TEMPERATURE_ARRAY_LEN = 10;
const uint8_t TEMPERATURE_PRECISION = 1;

const long CHECK_HT_INTERVAL_MS = 5000;
const long PING_INTERVAL_MS = 30000;

const char* MQTT_SERVER_KEY = "mqttServer";
const char* MQTT_PORT_KEY = "mqttPort";
const char* MQTT_CLIENT_ID_KEY = "mqttClientId";
const char* MQTT_USER_KEY = "mqttUser";
const char* MQTT_PASS_KEY = "mqttPass";
const char* BASE_TOPIC_KEY = "baseTopic";
const char* MODE_KEY = "mode";
const char* CONFIG_FILE_NAME = "/config.json";

const char* TOPIC_SEPARATOR = "/";
const char* TOPIC_HUMIDITY = "humidity";
const char* TOPIC_DESIRED_TEMPERATURE = "desiredtemp";
const char* TOPIC_CURRENT_TEMPERATURE = "currenttemp";
const char* TOPIC_SET = "set";
const char* TOPIC_MODE = "mode";
const char* PAYLOAD_HEAT = "heat";
const char* PAYLOAD_COLD = "cold";
const char* TOPIC_DEVICE_STATE = "state";
const char* PAYLOAD_ON = "on";
const char* PAYLOAD_OFF = "off";
const char* TOPIC_FAN_DEGREE = "fandegree";
const char* PAYLOAD_STARTED = "started";

const char* EVERY_ONE_LEVEL_TOPIC = "+";
const char* NOT_AVILABLE = "N/A";

const float FAN_SWITCH_LEVEL[FAN_SWITCH_LEVEL_LEN] = { 0, 0.5, 1.0 };

const float MIN_DESIRED_TEMPERATURE = 15;
const float MAX_DESIRED_TEMPERATURE = 30;

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
	CurrentTemp = 1,
	DesiredTemp = 2,
	FanDegree = 4,
	CurrentMode = 8,
	CurrentDeviceState = 16,
	DeviceIsStarted = 32,
	CurrentHumidity = 64
};

void test();

#endif

