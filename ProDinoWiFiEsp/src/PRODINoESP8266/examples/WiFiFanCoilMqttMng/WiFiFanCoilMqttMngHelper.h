// WiFiFanCoilMqttMngHelper.h

#ifndef _WIFIFANCOILMQTTMNGHELPER_h
#define _WIFIFANCOILMQTTMNGHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

const uint8_t MQTT_SERVER_LEN = 40;
const uint8_t MQTT_PORT_LEN = 8;
const uint8_t MQTT_CLIENT_ID_LEN = 32;
const uint8_t MQTT_USER_LEN = 16;
const uint8_t MQTT_PASS_LEN = 16;
const uint8_t BASE_TOPIC_LEN = 32;
const uint8_t TEMPERATURE_ARRAY_LEN = 10;
const long CHECK_HT_INTERVAL_MS = 5000;

const char* MQTT_SERVER_KEY = "mqttServer";
const char* MQTT_PORT_KEY = "mqttPort";
const char* MQTT_CLIENT_ID_KEY = "mqttClientId";
const char* MQTT_USER_KEY = "mqttUser";
const char* MQTT_PASS_KEY = "mqttPass";
const char* BASE_TOPIC_KEY = "baseTopic";
const char* MODE_KEY = "mode";
const char* CONFIG_FILE_NAME = "/config.json";

const char* TOPIC_SEPARATOR = "/";
const char* HUMIDITY_COMMAND = "humidity";
const char* TEMPERATURE_COMMAND = "temperature";
const char* RELAY = "relay";
const char* OPTO_INPUT = "optoin";
const char* SET_COMMAND = "set";
const char* HEAT_VALUE = "heat";
const char* COLD_VALUE = "cold";
const char* MODE_COMMAND = "mode";
const char* STATE_COMMAND = "state";
const char* ON_STATE = "on";
const char* OFF_STATE = "off";

const float FUN_SWITCH_LEVEL[3] = { 0.5, 1.0, 2.0 };

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

#endif

