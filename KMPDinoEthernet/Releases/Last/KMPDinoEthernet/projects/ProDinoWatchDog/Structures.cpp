// Structures.cpp
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

#include "Structures.h"
#include "KMPCommon.h"

const char DEF_User[] = "admin";
const char DEF_Password[] = "admin";
const uint8_t DEF_IPAddress[] = { 192, 168, 0, 103 };
const uint8_t DEF_SubnetMask[] = { 255, 255, 255, 0 };
const uint8_t DEF_Gateway[] = { 192, 168, 0, 1 };
const uint8_t DEF_TestPeriod = 30; // Default test period in seconds.
const uint8_t DEF_HostLostAfter = 3; // Default HostLostAfter ... times.
const uint8_t DEF_RestartWaitTime = 10; // Default restart wait time in second. On wait Off.
const uint8_t DEF_WaitAfterRestart = 60; // Default wait after restart in seconds. Wait before start ping.

DeviceSetting DeviceData;
DogSetting DogList[DOG_COUNT];
ProcessInfo ProcessList[DOG_COUNT];

DogSetting DogListSettings[DOG_COUNT];
ValidationData ValidDataSettings[VALID_DATA_LEN];
DeviceSetting DeviceDataUserAndIP;
ValidationData ValidUserAndIP[VALID_USER_AND_IP_LEN];


void DEF_DeviceSetting(DeviceSetting & ds)
{
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

void DEF_DogSetting(DogSetting & data)
{
    data.Active = 0; // Not active.
    
    for (uint8_t i = 0; i < IP_LEN; i++)
    {
        data.IP[i] = 0;
    }
    
    data.TestPeriod = DEF_TestPeriod;
    data.HostLostAfter= DEF_HostLostAfter;
    data.RestartWaitTime = DEF_RestartWaitTime;
    data.WaitAfterRestart = DEF_WaitAfterRestart;
}

void SetLists_DEF()
{
    for (uint8_t i = 0; i < DOG_COUNT; i++)
    {
        DEF_DogSetting(DogList[i]);
        
        ProcessInfo pd = ProcessList[i];
        pd.LostTimes = 0;
        pd.Restarts = 0;
        pd.NextOpertion = OT_PING;
    }
}