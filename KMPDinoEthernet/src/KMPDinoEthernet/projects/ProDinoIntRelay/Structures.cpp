// Structures.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Internet relay.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "Structures.h"
#include "KMPCommon.h"

const char DEF_User[] = "admin";
const char DEF_Password[] = "admin";
const char DEF_RelayName[] = "Relay ";
const char DEF_DeviceName[] = "KMP Device";
const uint8_t DEF_IPAddress[] = { 192, 168, 0, 100 };
const uint8_t DEF_SubnetMask[] = { 255, 255, 255, 0 };
const uint8_t DEF_Gateway[] = { 192, 168, 0, 1 };

DeviceSetting DeviceData;
RelayData RelayDataList[RELAY_COUNT];

RelayData RelayDataNewList[RELAY_COUNT];
ValidationData ValidationForRelayDataNewList[VALID_DATA_LEN];

DeviceSetting DeviceDataNew;
ValidationData ValidUserAndIP[VALID_USER_AND_IP_LEN];

void DEF_DeviceSetting(DeviceSetting & ds)
{
    strNCopy(ds.DeviceName, DEF_DeviceName, sizeof(DEF_DeviceName));
	
    strNCopy(ds.User, DEF_User, sizeof(DEF_User));

    strNCopy(ds.Password, DEF_Password, sizeof(DEF_Password));

    for (uint8_t i = 0; i < IP_LEN; i++)
    {
        ds.IP[i] = DEF_IPAddress[i];
    }

    for (uint8_t i = 0; i < IP_LEN; i++)
    {
        ds.SubnetMask[i] = DEF_SubnetMask[i];
    }

    for (uint8_t i = 0; i < IP_LEN; i++)
    {
        ds.Gateway[i] = DEF_Gateway[i];
    }
}

void DEF_RelayData(RelayData & data, uint8_t relay)
{
    data.Active = 1; // Active.
	
	uint8_t len = sizeof(DEF_RelayName);
	strNCopy(data.RelayName, DEF_RelayName, len);
	
	data.RelayName[len - 1] = ONE_TO_FOUR[relay];
}

void DEF_RelayDataList()
{
    for (uint8_t i = 0; i < RELAY_COUNT; i++)
    {
        DEF_RelayData(RelayDataList[i], i);
    }
}