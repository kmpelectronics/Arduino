// MqttTopicHelper.h

#ifndef _MQTTTOPICHELPER_h
#define _MQTTTOPICHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

const char TOPIC_SEPARATOR = '/';
const uint16_t MAIN_TOPIC_MAXLEN = 64;
const char ISREADY_TOPIC[] = "isready";
const char SET_TOPIC[] = "/set";

class MqttTopicHelperClass
{
 public:
	void init(const char* baseTopic, const char* deviceTopic, Print* debugPort = NULL);
	void printTopicAndPayload(const char* topic, const byte* payload, unsigned int length);
	void printTopicAndPayload(const char* topic, const char* payload);
	void addCharToStr(char* str, const char chr);
	void addTopicSeparator(char* str);
	void appendTopic(char * topic, const char * nextTopic);
	const char* getMainTopic();
	const char* getIsReadyTopic();
	bool startsWithMainTopic(const char* str);
	void buildTopicWithMT(char * str, int num, ...);
	void buildTopic(char* str, int num, ...);
	bool isBaseTopic(char* topic);
	bool isMainTopic(char* topic);
	bool isReadyTopic(char* topic);
	bool getNextTopic(const char* topics, char* nextTopic, char** otherTopics, bool skipMainTopic = false);
	bool isTopicSet(const char* topics);
protected:
	bool isOnlyThisTopic(const char* topic1, const char* topic2);
};

extern MqttTopicHelperClass MqttTopicHelper;

#endif

