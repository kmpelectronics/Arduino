// 
// 
// 

#include "MqttTopicHelper.h"
#include "KMPCommon.h"
#include <stdarg.h>

const char* _baseTopic;
const char* _deviceTopic;
char _mainTopic[MAIN_TOPIC_MAXLEN];
size_t _mainTopicLen;
char _isReadyTopic[MAIN_TOPIC_MAXLEN];
size_t _setTopicLen;
Print* _debugPort;

void MqttTopicHelperClass::init(const char* baseTopic, const char* deviceTopic, Print* debugPort)
{
	_baseTopic = baseTopic;
	_deviceTopic = deviceTopic;
	_debugPort = debugPort;

	// Building main topic.
	strcpy(_mainTopic, baseTopic);
	addCharToStr(_mainTopic, TOPIC_SEPARATOR);
	strcat(_mainTopic, deviceTopic);

	_mainTopicLen = strlen(_mainTopic);

	// Building is ready topic.
	strcpy(_isReadyTopic, _mainTopic);
	addCharToStr(_isReadyTopic, TOPIC_SEPARATOR);
	strcat(_isReadyTopic, ISREADY_TOPIC);

	_setTopicLen = strlen(SET_TOPIC);
}

void MqttTopicHelperClass::printTopicAndPayload(const char * topic, const byte * payload, unsigned int length)
{
	if (_debugPort == NULL)
	{
		return;
	}

	_debugPort->print(F("Topic ["));
	_debugPort->print(topic);
	_debugPort->println(F("]"));

	_debugPort->print(F("Payload ["));
	for (uint i = 0; i < length; i++)
	{
		_debugPort->print((char)payload[i]);
	}
	_debugPort->println(F("]"));
}

void MqttTopicHelperClass::printTopicAndPayload(const char * topic, const char * payload)
{
	printTopicAndPayload(topic, (const byte *) payload, strlen(payload));
}

void MqttTopicHelperClass::addCharToStr(char * str, const char chr)
{
	size_t len = strlen(str);

	str[len++] = chr;
	str[len] = CH_NONE;
}

void MqttTopicHelperClass::addTopicSeparator(char * str)
{
	addCharToStr(str, TOPIC_SEPARATOR);
}

void MqttTopicHelperClass::appendTopic(char * topic, const char * nextTopic)
{
	if (topic[0] == CH_NONE)
	{
		strcpy(topic, nextTopic);
	}
	else
	{
		addTopicSeparator(topic);
		strcat(topic, nextTopic);
	}
}

const char * MqttTopicHelperClass::getMainTopic()
{
	return _mainTopic;
}

const char * MqttTopicHelperClass::getIsReadyTopic()
{
	return _isReadyTopic;
}

bool MqttTopicHelperClass::startsWithMainTopic(const char * str)
{
	return startsWith(str, _mainTopic);
}

void MqttTopicHelperClass::buildTopic(char * str, int num, ...)
{
	str[0] = CH_NONE;

	va_list valist;
	int i;

	/* initialize valist for num number of arguments */
	va_start(valist, num);

	/* access all the arguments assigned to valist */
	for (i = 0; i < num; i++) {
		appendTopic(str, va_arg(valist, char*));
	}

	/* clean memory reserved for valist */
	va_end(valist);
}

bool MqttTopicHelperClass::isBaseTopic(char * topic)
{
	return isOnlyThisTopic(topic, _baseTopic);
}

bool MqttTopicHelperClass::isMainTopic(char * topic)
{
	return isOnlyThisTopic(topic, _mainTopic);
}

bool MqttTopicHelperClass::isReadyTopic(char * topic)
{
	return isOnlyThisTopic(topic, _isReadyTopic);
}

bool MqttTopicHelperClass::getNextTopic(const char * topics, char * nextTopic, char ** otherTopics, bool skipMainTopic)
{
	if (!topics || !nextTopic)
		return false;

	size_t topicLen = strlen(topics);
	if (topicLen == 0)
		return false;

	nextTopic[0] = CH_NONE;

	char * otherTopicResult;

	if (skipMainTopic)
	{
		if (topicLen <= _mainTopicLen ||
			// starts with MT
			strncmp(_mainTopic, topics, _mainTopicLen) != 0 ||
			// MT ends with topic separator
			topics[_mainTopicLen] != TOPIC_SEPARATOR)
		{
			return false;
		}
		// Skip MT + separator.
		otherTopicResult = (char *)topics + _mainTopicLen + 1;
	}
	else
	{
		if (topics[0] == TOPIC_SEPARATOR)
		{
			// includes only separator
			if (topicLen == 1)
				return false;
			
			// Skip separator
			otherTopicResult = (char*)topics + 1;
		}
		else
		{
			otherTopicResult = (char*)topics;
		}
	}

	char * findPos = otherTopicResult;
	size_t length = 0;
	while (*findPos != CH_NONE && *findPos != TOPIC_SEPARATOR)
	{
		findPos++;
		length++;
	}

	if (length == 0)
		return false;
	
	strNCopy(nextTopic, otherTopicResult, length);

	*otherTopics = otherTopicResult + length;

	return true;
}

bool MqttTopicHelperClass::isTopicSet(const char* topics)
{
	return endsWith(topics, SET_TOPIC);
}

bool MqttTopicHelperClass::isOnlyThisTopic(const char * topic1, const char * topic2)
{
	if (!topic1 || !topic2)
	{
		return false;
	}

	return isEqual(topic1, topic2);
}

void MqttTopicHelperClass::buildTopicWithMT(char * str, int num, ...)
{
	str[0] = CH_NONE;
	appendTopic(str, _mainTopic);

	va_list valist;
	int i;

	/* initialize valist for num number of arguments */
	va_start(valist, num);

	/* access all the arguments assigned to valist */
	for (i = 0; i < num; i++) {
		appendTopic(str, va_arg(valist, char*));
	}

	/* clean memory reserved for valist */
	va_end(valist);

}

MqttTopicHelperClass MqttTopicHelper;