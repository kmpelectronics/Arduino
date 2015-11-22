// Structures.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Internet relay.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#ifndef _STRUCTURES_h
#define _STRUCTURES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "KmpDinoEthernet.h"
#include "KMPCommon.h"

// User, password length
#define USER_PAS_LEN 8
#define DEVICE_NAME_LEN 32
#define RELAY_NAME_LEN 16

const uint8_t WAN_PORT = 80;
const uint8_t MAC_ADDRESS[] =  { 0x80, 0x9B, 0x20, 0xBF, 0x91, 0xF6 };

const char PREFF_CB[]PROGMEM = "cb";
const char PREFF_RN[]PROGMEM = "rn";
const char PREFF_TP[]PROGMEM = "tp";
const char PREFF_HL[]PROGMEM = "hl";
const char PREFF_RW[]PROGMEM = "rw";
const char PREFF_WA[]PROGMEM = "wa";
const char PREFF_DN[]PROGMEM = "DN";
const char PREFF_US[]PROGMEM = "US";
const char PREFF_PS[]PROGMEM = "PS";
const char PREFF_SN[]PROGMEM = "SN";
const char PREFF_GW[]PROGMEM = "GW";

const char ONE_TO_FOUR[] = "1234";

const uint8_t DATA_ROW_COUNT = 2;

const uint8_t VALID_DATA_LEN = RELAY_COUNT * DATA_ROW_COUNT;

const uint8_t VALID_USER_AND_IP_LEN = 6;

const uint8_t MAX_IP_STR_LEN = 16;

const uint8_t INDEX_PAGE_REFRESH_RATE = 60;

/**
* \brief Web pages.
*/
enum WebPages
{
    WP_NONE = -1, WP_INDEX, WP_SETTINGS, WP_USERANDIP
};


/**
* \brief Max web pages.
*/
const uint8_t WEB_PAGE_LEN = 3;

/**
 * \brief Structure with device settings information.
 */
struct DeviceSetting
{
	uint8_t IP[IP_LEN];
    uint8_t SubnetMask[IP_LEN];
    uint8_t Gateway[IP_LEN];
    char DeviceName[DEVICE_NAME_LEN + 1];
	char User[USER_PAS_LEN + 1];
	char Password[USER_PAS_LEN + 1];
};

void DEF_DeviceSetting(DeviceSetting & data);
		
/**
 * \brief Structure with relay settings information.
 */
struct RelayData
{
    uint8_t Active;
    char RelayName[RELAY_NAME_LEN + 1];
};

/**
 * \brief Set default values DogSetting data.
 * 
 * \param data Point to data well set default values.
 * \param relay Relay number.
 * 
 * \return void
 */
void DEF_RelayData(RelayData & data, uint8_t relay);

/**
 * \brief Structure stored data and check is valid.
 */
struct ValidationData
{
    uint8_t IsValid;
    char* Value;
    uint8_t Length;
};

/**
 * \brief List stored relay settings information.
 */
extern RelayData RelayDataList[RELAY_COUNT];

/**
 * \brief List stored new relay data from POST request.
 */
extern RelayData RelayDataNewList[RELAY_COUNT];
/**
 * \brief List stored validation data for new relay data from POST request.
 */
extern ValidationData ValidationForRelayDataNewList[VALID_DATA_LEN];

/**
 * \brief Stored current device setting. User, pass, IP, subnet, gateway.
 */
extern DeviceSetting DeviceData;

extern DeviceSetting DeviceDataNew;
extern ValidationData ValidUserAndIP[VALID_USER_AND_IP_LEN];

/**
 * \brief Set Relay list default values.
 * 
 * \return void
 */
void DEF_RelayDataList();

#endif
