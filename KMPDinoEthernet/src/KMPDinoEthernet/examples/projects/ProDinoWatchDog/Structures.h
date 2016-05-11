// Structures.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Watchdog.
// Description:
//		Project structures.
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

// Dog count.
const uint8_t DOG_COUNT = RELAY_COUNT;

const uint8_t WAN_PORT = 80;
const uint8_t MAC_ADDRESS[] =  { 0x80, 0x9B, 0x20, 0x54, 0x64, 0x68 };

const char PREFF_CB[]PROGMEM = "cb";
const char PREFF_IP[]PROGMEM = "ip";
const char PREFF_TP[]PROGMEM = "tp";
const char PREFF_HL[]PROGMEM = "hl";
const char PREFF_RW[]PROGMEM = "rw";
const char PREFF_WA[]PROGMEM = "wa";
const char PREFF_US[]PROGMEM = "US";
const char PREFF_PS[]PROGMEM = "PS";
const char PREFF_SN[]PROGMEM = "SN";
const char PREFF_GW[]PROGMEM = "GW";

const char ONE_TO_FOUR[] = "1234";

// Session timeout in minutes.
const uint8_t SESSION_TIMEOUT = 10;

const uint8_t DATA_ROW_COUNT = 6;

const uint8_t VALID_DATA_LEN = DOG_COUNT * DATA_ROW_COUNT;

const uint8_t VALID_USER_AND_IP_LEN = 5;

const uint8_t MAX_IP_STR_LEN = 16;

const uint8_t INDEX_PAGE_REFRESH_RATE = 60;

/**
 * \brief Current operation type.
 */
enum OperationType
{
    OT_PING, OT_RESTART
};
/**
* \brief Web pages.
*/
enum WebPages
{
    WP_NONE = -1, WP_INDEX, WP_PRIVATE_SETTINGS, WP_PRIVATE_USERANDIP
};

const uint8_t WEB_PAGE_LEN = 3;

/**
 * \brief Structure with device settings information.
 */
struct DeviceSetting
{
    uint8_t IP[IP_LEN];
    uint8_t SubnetMask[IP_LEN];
    uint8_t Gateway[IP_LEN];
	char User[USER_PAS_LEN + 1];
	char Password[USER_PAS_LEN + 1];
};

void DEF_DeviceSetting(DeviceSetting & data);
		
/**
 * \brief Structure with dog settings information.
 */
struct DogSetting
{
    uint8_t Active;
    uint8_t IP[IP_LEN];
    uint8_t TestPeriod;
    uint8_t HostLostAfter;
    uint8_t RestartWaitTime;
    uint8_t WaitAfterRestart;
};

/**
 * \brief Set default values DogSetting data.
 * 
 * \param data Point to data well set default values.
 * 
 * \return void
 */
void DEF_DogSetting(DogSetting & data);

/**
 * \brief Processing information. Lost times, restarts, next time operation, current operation.
 */
struct ProcessInfo
{
    uint8_t LostTimes;
    uint8_t Restarts;
    unsigned long NextTimeToOperate;
    OperationType NextOpertion;
};

struct ValidationData
{
    uint8_t IsValid;
    char* Value;
    uint8_t Length;
};

/**
 * \brief List stored dog settings information.
 */
extern DogSetting DogList[DOG_COUNT];

/**
 * \brief List stored processing information.
 */
extern ProcessInfo ProcessList[DOG_COUNT];

extern DogSetting DogListSettings[DOG_COUNT];
extern ValidationData ValidDataSettings[VALID_DATA_LEN];

extern DeviceSetting DeviceDataUserAndIP;
extern ValidationData ValidUserAndIP[VALID_USER_AND_IP_LEN];

/**
 * \brief Stored current device setting. User, pass, IP, subnet, gateway.
 */
extern DeviceSetting DeviceData;

/**
 * \brief Set DogList and ProcessList default values.
 * 
 * \return void
 */
void SetLists_DEF();

#endif