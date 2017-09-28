#include "FanCoilHelper.h"
#include "KMPCommon.h"

float calcAverage(float * data, uint8 dataLength, uint8 precision)
{
	// Get average value.
	double temp = 0.0;
	for (uint i = 0; i < dataLength; i++)
	{
		temp += data[i];
	}

	temp /= dataLength;

	float result = roundF(temp, precision);

	return result;
}

/**
* @brief Print debug information about topic and payload.
* @operationName Operation which send this data.
* @param topic The topic name.
* @param payload The payload data.
* @param length A payload length.
*
* @return void
*/
#ifdef WIFIFCMM_DEBUG
void printTopicAndPayload(const char* operationName, const char* topic, char* payload, unsigned int length)
{
	DEBUG_FC_PRINT(operationName);
	DEBUG_FC_PRINT(" topic [");
	DEBUG_FC_PRINT(topic);
	DEBUG_FC_PRINT("] payload [");
	for (uint i = 0; i < length; i++)
	{
		DEBUG_FC_PRINT((char)payload[i]);
	}
	DEBUG_FC_PRINTLN("]");
}
#endif
