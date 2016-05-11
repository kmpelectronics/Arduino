// Processing.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Watchdog.
// Description:
//		Process actions and Html pages.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "Processing.h"
#include <avr/eeprom.h>
#include "Base64.h"
#include "ICMPProtocol.h"
#include "HtmlPages.h"
#include "KMPCommon.h"
#include <avr/wdt.h>

/**
* \brief Request types.
*/
enum RequestType
{
    RT_NONE = -1, RT_GET = 0, RT_POST = 1
};

const char* RT_TEXT[] = {"GET", "POST"};

const char W_AUTHORIZATION[] = "Authorization";

const char W_BASIC[] = "Basic ";

const uint8_t MAX_BASE64_LEN = 24;

// If first byte in EEPROM shows if need to initialized data in EEPROM.
const uint8_t INITIALIZED_EEPROM = 0x55;

const uint8_t MAX_HOST_LOST_AFTER = 10;

const uint8_t MAX_REQUEST_LINE_LEN = 254;

char _requestLine[MAX_REQUEST_LINE_LEN + 1];

/**
* \brief List stored position data in EEPROM.
*        0 - INITIALIZED_EEPROM start (0), 1 - DeviceData start (INITIALIZED_EEPROM length), Dogs start (INITIALIZED_EEPROM + DeviceData length).
*/
const uint8_t EE_DATA_POS[] = {0, sizeof(INITIALIZED_EEPROM), sizeof(INITIALIZED_EEPROM) + sizeof(DeviceData)};

EthernetClient _etClient;

HtmlPages _html;

bool _addressesChanged;

ICMPProtocol _icmp;

void ReadEEPROMData()
{
    eeprom_read_block(&DeviceData, (void *)EE_DATA_POS[1], sizeof(DeviceData));
    eeprom_read_block(DogList, (void *)EE_DATA_POS[2], sizeof(DogList));
    
    _addressesChanged = true;
}

void WriteEEPROMDeviceData(DeviceSetting deviceSettings)
{
	eeprom_write_block(&deviceSettings, (void *)EE_DATA_POS[1], sizeof(DeviceData));
}

void WriteEEPROMDogData(DogSetting * dogList)
{
    eeprom_write_block((const void*)dogList, (void *)EE_DATA_POS[2], sizeof(DogList));
}

void WriteEEPROMAllData()
{
    eeprom_write_byte((uint8_t*)EE_DATA_POS[0], INITIALIZED_EEPROM);
    WriteEEPROMDeviceData(DeviceData);
    WriteEEPROMDogData(DogList);
}

/**
* \brief Set next time to ping in ProcessList.
*
* \param itemNo Item to set next time to ping.
* \param time Time to ping in seconds.
*
* \return void
*/
void SetNextTimeToOperate(uint8_t itemNo, uint8_t time)
{
    unsigned long t = time;
    ProcessList[itemNo].NextTimeToOperate = millis() + t * 1000;
}

/**
* \brief Encode to base64 User : pass and prepare to check.
*
* \param result Encoded string.
*
* \return void
*/
void encodeUserPass(char * result)
{
    // + 1 for ':' + 1 for '\0'.
    char userPass[USER_PAS_LEN * 2 + 1 + 1];
    userPass[0] = CH_NONE;
    strcpy(userPass, DeviceData.User);
    strcat_P(userPass, PSTR(":"));
    strcat(userPass, DeviceData.Password);

    base64_encode(result, userPass, strlen(userPass));
}

/**
* \brief Check if user and password encoded in base64 is valid.
*
* \param userPass user and password encoded in base64 to check.
*
* \return uint8_t result is equals - 1, if error - 0.
*/
uint8_t baUserPassIsValid( char * userPass )
{
    char buf[MAX_BASE64_LEN + 1];
    encodeUserPass(buf);
    
    return strcmp(buf, userPass) == 0 ? 1 : 0;
}

uint8_t PingIP(uint8_t * ip)
{
    // Ping id = 0x0400 is for windows.
    ICMPEchoReply echoReply = _icmp.Ping(ip, 0x0400, 3);
    
    return echoReply.status == SUCCESS ? 1 : 0;
}

void operationPing(DogSetting& ds, ProcessInfo& pr, uint8_t dogNumber)
{
    // If ping success - exit.
    bool pingResult = PingIP(ds.IP);
    SetNextTimeToOperate(dogNumber, ds.TestPeriod);
    
    // If ping OK - return.
    if (pingResult)
    { 
		// Reset lost times if ping success.
		pr.LostTimes = 0;
		return; 
	}
    
    if (++pr.LostTimes > ds.HostLostAfter)
    {
        // Reset lost times.
        pr.LostTimes = 0;
        // Switch relay to ON.
        SetRelayStatus(dogNumber, ON);
        // Increment restarts.
        ++pr.Restarts;
        // Set current operation to RESTART.
        pr.NextOpertion = OT_RESTART;
        // Prepare restart wait time.
        SetNextTimeToOperate(dogNumber, ds.RestartWaitTime);
    }
}

void operationRestart(DogSetting& ds, ProcessInfo& pr, uint8_t dogNumber)
{
    // Switch relay to OFF.
    SetRelayStatus(dogNumber, OFF);
    // Next operation is ping.
    pr.NextOpertion = OT_PING;
    // Prepare time to next ping.
    SetNextTimeToOperate(dogNumber, ds.WaitAfterRestart);
}

void Processing::processOperaions()
{
    for (uint8_t i = 0; i < DOG_COUNT; i++)
    {
        if (!DogList[i].Active)
        { continue; }

        // Reset the WDT timer.
        wdt_reset();
        
        // Check it is time for action.
        if (millis() < ProcessList[i].NextTimeToOperate)
        { continue; }
        
        switch (ProcessList[i].NextOpertion)
        {
            case OT_PING:
            operationPing(DogList[i], ProcessList[i], i);
            break;
            case OT_RESTART:
            operationRestart(DogList[i], ProcessList[i], i);
            break;
            default: ;
        }
    }
}

bool ReadRequestLine()
{
    _requestLine[0] = CH_NONE;
    uint8_t pos = 0;
    char prevChar = CH_NONE;
    bool isRead = false;
    
    while (_etClient.available())
    {
        isRead = true;
        // Can not read more - buffer overflow return null.
        if (pos == MAX_REQUEST_LINE_LEN - 1)
        { return false; }
        
        char c = _etClient.read();
#ifdef DEBUG
        Serial.write(c);
#endif
        // Add only chars, exclude CR LF.
        if(c != CH_CR && c != CH_LF)
        { _requestLine[pos++] = c; }
        
        // Find end line, after this start Post data.
        if(prevChar == CH_CR && c == CH_LF)
        { break; }

        prevChar = c;
    }
	
	// Add \0.
	_requestLine[pos] = CH_NONE;
    
    // Is not read - return false.
    if (!isRead)
    { return false; }
    
    return true;
}

void ReadRequestToEnd()
{
    char c;
    // Fast flush request.
    while (_etClient.available())
    {
        c = _etClient.read();
#ifdef DEBUG
        Serial.print(c);
#endif
    }
#ifdef DEBUG
    Serial.println();
#endif
}

void GetRequestType(char* txt, RequestType& rt)
{
    // Valid request types.
    //GET HEAD POST PUT DELETE TRACE OPTIONS CONNECT PATCH
    rt = RT_NONE;
    
    // Process only get or post request.
    uint8_t isGet =	startsWith(txt, RT_TEXT[RT_GET]);
    uint8_t isPost = startsWith(txt, RT_TEXT[RT_POST]);
    
    if (!isGet && !isPost)
    { return; }
    
    // Set request type.
    rt = isGet ? RT_GET : RT_POST;
}

void GetRequestPage(char* txt, WebPages& wp)
{
    const char* wp_link[WEB_PAGE_LEN] = {"/", "/private/settings", "/private/userandip"};
        
    wp = WP_NONE;
    // Get target page.
    char buff[32];
    for (uint8_t i = 0; i < WEB_PAGE_LEN; i++)
    {
        buff[0] = CH_SPACE;
        buff[1] = CH_NONE;

        strcat(buff, wp_link[i]);
        strcat(buff, " ");
        
        if (strstr(txt, buff))
        {
            wp = (WebPages)i;
            break;
        }
    }
}

bool GetRequestTypeAndPage(RequestType& rt, WebPages& wp)
{
	if (!ReadRequestLine())
	{ return false;	}
    
    GetRequestType(_requestLine, rt);
    
    if(rt == RT_NONE)
    { return false; }

    GetRequestPage(_requestLine, wp);
    
    if(wp == WP_NONE)
    { return false; }
    
    return true;
}

bool CheckRequestIsAuthorized()
{
    while (ReadRequestLine())
    {
        // Have Authorization data, check user and pass is valid.
        if (startsWith(_requestLine, W_AUTHORIZATION))
        {
            char *result = strstr(_requestLine, W_BASIC);
            
			if (result == NULL)
            { return false; }
            
            char* userPass = result + strlen(W_BASIC);

            return baUserPassIsValid(userPass);
        }
    }
    
    return false;
}

bool FindParameter(char* text, PGM_P preffName, char suffName, char*& val, uint8_t& len)
{
    char paramName[5] = { pgm_read_byte(preffName++), pgm_read_byte(preffName), suffName, CH_EQUAL, CH_NONE};
    
    char* result = strstr(text, paramName);

    if (result == NULL)
    { return false; }
    
    // paramName XXY=\0 len 4 bytes.
    val = result + 4;
    
	char * findLen = val;
    // Calculate value length.
    char c;
    len = 0;
    while (true)
    {
		c = *findLen++;
		if (c == CH_AMPERSAND || c == CH_NONE)
		{ break; }
				
        len++;
    }
    
    return true;
}

void GetPostParameterLine()
{
    // Find parameters line.
    while (ReadRequestLine())
    {
        // Find first empty line after this fallowed post data.
        if (_requestLine[0] == CH_NONE)
        {
            ReadRequestLine();
            break;
        }
    }
}

bool GetPrivateSettingsData()
{
    // Find parameters line.
    GetPostParameterLine();
    
    if (_requestLine[0] == CH_NONE)
    { return false; }
    
    // Process params.
    char * paramVal;
	uint8_t paramLen;
    uint8_t vdPos = 0;
    uint8_t ipBuff[IP_LEN];
	bool findParam;
    bool cnvResult;
    
    
	for (uint8_t row = 0; row < DATA_ROW_COUNT; row++)
    {
        for (uint8_t col = 0; col < DOG_COUNT; col++)
        {
            DogSetting& dog = DogListSettings[col];
            uint8_t val;
            char numChar = ONE_TO_FOUR[col];
			ValidationData& vd = ValidDataSettings[vdPos++];
            
            switch (row)
            {
                // Status.
                case 0:
                {
                    // cbX=1.
                    findParam = FindParameter(_requestLine, PREFF_CB, numChar, paramVal, paramLen);
					
                    dog.Active = (findParam && paramVal[0] == CH_1) ? true : false;
                    vd.IsValid = 1;
                    vd.Value = paramVal;
                    vd.Length = paramLen;
                }
                break;
                // IP.
                case 1:
                {
                    // ipX=0.0.0.0.
                    findParam = FindParameter(_requestLine, PREFF_IP, numChar, paramVal, paramLen);
                    cnvResult = atoip(paramVal, ipBuff);

                    vd.IsValid = findParam && cnvResult ? 1 : 0;
                    vd.Value = paramVal;
                    vd.Length = paramLen;

                    if (cnvResult)
                    { memcpy(dog.IP, ipBuff, IP_LEN); }
                }
                break;
                // Test period.
                case 2:
                {
                    // tpX=30
                    findParam = FindParameter(_requestLine, PREFF_TP, numChar, paramVal, paramLen);
                    cnvResult = atoUint8(paramVal, val);

                    vd.IsValid = (findParam && cnvResult) && val > 0 ? 1 : 0;
                    vd.Value = paramVal;
                    vd.Length = paramLen;

                    dog.TestPeriod = val;
                }
                break;
                // Host lost after.
                case 3:
                {
                    // hlX=3.
                    findParam = FindParameter(_requestLine, PREFF_HL, numChar, paramVal, paramLen);
                    cnvResult = atoUint8(paramVal, val);
                    
                    vd.IsValid = (findParam && cnvResult && val <= MAX_HOST_LOST_AFTER && val > 0) ? 1 : 0;
                    vd.Value = paramVal;
                    vd.Length = paramLen;
                    dog.HostLostAfter = val;
                }
                break;
                // Restart wait time.
                case 4:
                {
                    // rwX=10.
                    findParam = FindParameter(_requestLine, PREFF_RW, numChar, paramVal, paramLen);
                    cnvResult = atoUint8(paramVal, val);
                    
                    vd.IsValid = (findParam && cnvResult && val > 0) ? 1 : 0;
                    vd.Value = paramVal;
                    vd.Length = paramLen;
                    dog.RestartWaitTime = val;
                }
                break;
                // Wait after restart.
                case 5:
                {
                    // waX=60.
                    findParam = FindParameter(_requestLine, PREFF_WA, numChar, paramVal, paramLen);
                    cnvResult = atoUint8(paramVal, val);

                    vd.IsValid = (findParam && cnvResult && val > 0) ? 1 : 0;
                    vd.Value = paramVal;
                    vd.Length = paramLen;

                    dog.WaitAfterRestart = val;
                }
                break;
            }
        }
    }
    
    return true;
}

bool GetPrivateUserAndIpData()
{
    // Find parameters line.
    GetPostParameterLine();
    
    if (_requestLine[0] == CH_NONE)
    { return false; }
    
    // Process params.
    char * paramVal;
    uint8_t paramLen;
    uint8_t ipBuff[IP_LEN];
    bool findParam;
    bool cnvResult;
    
    for (uint8_t col = 0; col < VALID_USER_AND_IP_LEN; col++)
    {
        ValidationData& vd = ValidUserAndIP[col];
        switch (col)
        {
            // User name.
            case 0:
            {
                // cbX=1.
                findParam = FindParameter(_requestLine, PREFF_US, CH_1, paramVal, paramLen);
                    
                vd.IsValid = (findParam && paramLen >= 3 && paramLen <= 8) ? true : false;
                vd.Value = paramVal;
                vd.Length = paramLen;
                if (vd.IsValid)
                { strNCopy(DeviceDataUserAndIP.User, paramVal, paramLen); }
            }
            break;
            // Password.
            case 1:
            {
                // cbX=1.
                findParam = FindParameter(_requestLine, PREFF_PS, CH_1, paramVal, paramLen);
                
                vd.IsValid = (findParam && paramLen >= 3 && paramLen <= 8) ? true : false;
                vd.Value = paramVal;
                vd.Length = paramLen;
                if (vd.IsValid)
                { strNCopy(DeviceDataUserAndIP.Password, paramVal, paramLen); }
            }
            break;
            // IP.
            case 2:
            {
                findParam = FindParameter(_requestLine, PREFF_IP, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
                
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataUserAndIP.IP, ipBuff, IP_LEN); }
            }
            break;
            // Subnet mask.
            case 3:
            {
                findParam = FindParameter(_requestLine, PREFF_SN, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
                    
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataUserAndIP.SubnetMask, ipBuff, IP_LEN); }
            }
            break;
            // Subnet mask.
            case 4:
            {
                findParam = FindParameter(_requestLine, PREFF_GW, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
               
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataUserAndIP.Gateway, ipBuff, IP_LEN); }
            }
            break;
        }
    }
    
    return true;
}

uint8_t IsAuthorized()
{
    if(CheckRequestIsAuthorized())
    {
        return 1;
    }

    ReadRequestToEnd();
    _html.unAuthorized();

    return 0;
}

void PrivateSettingsGET()
{
    ReadRequestToEnd();
    _html.privateSettings(DogList);
}

void PrivateSettingsPOST()
{
    bool gsResult = GetPrivateSettingsData();
    ReadRequestToEnd();

    if(gsResult)
    {
        // Check for errors.
        bool isError = false;
        for (uint8_t i = 0; i < VALID_DATA_LEN; i++)
        {
            if(!ValidDataSettings[i].IsValid)
            {
                isError = true;
                break;
            }
        }
                        
        // Save to EEPROM.
        if (!isError)
        {
            WriteEEPROMDogData(DogListSettings);
            ReadEEPROMData();
        }
                        
        _html.privateSettings(DogListSettings, ValidDataSettings, isError ? 1 : 0, isError ? 0 : 1);
    }
    else
    {
        _html.privateSettings(DogList, NULL, 1);
    }
}

void PrivateUserAndIpGET()
{
    ReadRequestToEnd();
    _html.privateUserAndIP(DeviceData);
}

void PrivateUserAndIpPOST()
{
    bool gsResult = GetPrivateUserAndIpData();
    ReadRequestToEnd();

    if(gsResult)
    {
        // Check for errors.
        bool isError = false;
        for (uint8_t i = 0; i < VALID_USER_AND_IP_LEN; i++)
        {
            if(!ValidUserAndIP[i].IsValid)
            {
                isError = true;
                break;
            }
        }

        if (!isError)
        {
			WriteEEPROMDeviceData(DeviceDataUserAndIP);
			ReadEEPROMData();
        }
        
        _html.privateUserAndIP(DeviceDataUserAndIP, ValidUserAndIP, isError ? 1 : 0, isError ? 0 : 1);
    }
    else
    {
        _html.privateUserAndIP(DeviceData, NULL, 1);
    }
}

void Processing::processRequest(EthernetClient& client)
{
    _etClient = client;
    _html.init(client);
    
    RequestType rt;
    WebPages wp;
    GetRequestTypeAndPage(rt, wp);
    
    switch (wp)
    {
        case WP_INDEX:
        {
            ReadRequestToEnd();
            _html.index();
        }
        break;
        case WP_PRIVATE_SETTINGS:
        {
            if(!IsAuthorized())
            { return; }
            // if GET.
            if (rt == RT_GET)
            { PrivateSettingsGET(); }
            
            // if post. Fill dogList with valid data. Fill Validate data list if have not valid.
            if (rt == RT_POST)
            { PrivateSettingsPOST(); }
        }
        break;
        case WP_PRIVATE_USERANDIP:
        {
            if(!IsAuthorized())
            { return; }
            // if GET.
            if (rt == RT_GET)
            { PrivateUserAndIpGET(); }
            if (rt == RT_POST)
            { PrivateUserAndIpPOST(); }
        }
        break;
        default:
        {
            ReadRequestToEnd();
            _html.notFound();
        }
        break;
    }
}

void Processing::init()
{
    DEF_DeviceSetting(DeviceData);
    SetLists_DEF();

    if (eeprom_read_byte((uint8_t*)EE_DATA_POS[0]) == INITIALIZED_EEPROM)
    { ReadEEPROMData(); }
    else
    {
        // First write default data.
        WriteEEPROMAllData();
    }

    // Initialize next time to ping and process type.
    for (uint8_t i = 0; i < DOG_COUNT; i++)
    {
        SetNextTimeToOperate(i, DogList[i].TestPeriod);
        
        // Set all relays OFF.
        SetRelayStatus(i, OFF);
    }
}

void Processing::resetSettings()
{
    DEF_DeviceSetting(DeviceData);
    SetLists_DEF();
    WriteEEPROMAllData();
}

bool Processing::IsAddressesChanged()
{
    return _addressesChanged;
}

void Processing::SetAddressesChanged( bool b )
{
    _addressesChanged = b;
}