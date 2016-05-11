// Processing.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Internet relay.
// Version: 1.0.0
// Date: 21.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "Processing.h"
#include <avr/eeprom.h>
#include "Base64.h"
#include "ICMPProtocol.h"
#include "HtmlPages.h"
#include "KMPCommon.h"
#include <avr/wdt.h>
#include "Structures.h"

//#define DEBUG

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
*        0 - INITIALIZED_EEPROM start (0), 1 - DeviceData start (INITIALIZED_EEPROM length), RelayInfo start (INITIALIZED_EEPROM + DeviceData length).
*/
const uint8_t EE_DATA_POS[] = {0, sizeof(INITIALIZED_EEPROM), sizeof(INITIALIZED_EEPROM) + sizeof(DeviceData)};

EthernetClient _etClient;

HtmlPages _html;

bool _addressesChanged;

void ReadEEPROMData()
{
    eeprom_read_block(&DeviceData, (void *)EE_DATA_POS[1], sizeof(DeviceData));
    eeprom_read_block(RelayDataList, (void *)EE_DATA_POS[2], sizeof(RelayDataList));
    
    _addressesChanged = true;
}

void WriteEEPROMDeviceData(DeviceSetting deviceSettings)
{
	eeprom_write_block(&deviceSettings, (void *)EE_DATA_POS[1], sizeof(DeviceData));
}

void WriteEEPROMRelayInfoData(RelayData * relayList)
{
    eeprom_write_block((const void*)relayList, (void *)EE_DATA_POS[2], sizeof(RelayDataList));
}

void WriteEEPROMAllData()
{
    eeprom_write_byte((uint8_t*)EE_DATA_POS[0], INITIALIZED_EEPROM);
    WriteEEPROMDeviceData(DeviceData);
    WriteEEPROMRelayInfoData(RelayDataList);
}

void Processing::init()
{
    DEF_DeviceSetting(DeviceData);
    DEF_RelayDataList();

    if (eeprom_read_byte((uint8_t*)EE_DATA_POS[0]) == INITIALIZED_EEPROM)
    { ReadEEPROMData(); }
    else
    {
        // First write default data.
        WriteEEPROMAllData();
    }

	// Set all relay off.
	SetAllRelaysOff();
}

void Processing::resetSettings()
{
    DEF_DeviceSetting(DeviceData);
    DEF_RelayDataList();
    WriteEEPROMAllData();
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
uint8_t base64UserPassIsValid( char * userPass )
{
    char buf[MAX_BASE64_LEN + 1];
    encodeUserPass(buf);
    
    return strcmp(buf, userPass) == 0 ? 1 : 0;
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
    Serial.println("--end req--");
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
	const char* WEB_PAGE_LINK[WEB_PAGE_LEN] = {"/", "/settings", "/userandip"};
		
    wp = WP_NONE;
    // Get target page.
    char buff[32];
    for (uint8_t i = 0; i < WEB_PAGE_LEN; i++)
    {
        buff[0] = CH_SPACE;
        buff[1] = CH_NONE;

        strcat(buff, WEB_PAGE_LINK[i]);
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

            return base64UserPassIsValid(userPass);
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
	bool findParam;
    
	for (uint8_t row = 0; row < DATA_ROW_COUNT; row++)
    {
        for (uint8_t col = 0; col < RELAY_COUNT; col++)
        {
            RelayData& relay = RelayDataNewList[col];
            char numChar = ONE_TO_FOUR[col];
			ValidationData& vd = ValidationForRelayDataNewList[vdPos++];
            
			switch (row)
            {
                // Status.
                case 0:
                {
                    // cbX=1.
                    findParam = FindParameter(_requestLine, PREFF_CB, numChar, paramVal, paramLen);
					
                    relay.Active = (findParam && paramVal[0] == CH_1) ? true : false;
                    vd.IsValid = 1;
                    vd.Value = paramVal;
                    vd.Length = paramLen;
                }
                break;
                // Name.
                case 1:
                {
                    // rnX=Relay 1.
					findParam = FindParameter(_requestLine, PREFF_RN, numChar, paramVal, paramLen);
                
					vd.IsValid = (findParam && paramLen >= 3 && paramLen <= RELAY_NAME_LEN) ? true : false;
					strReplace(paramVal, paramLen, '+', CH_SPACE);
					vd.Value = paramVal;
					vd.Length = paramLen;
					if (vd.IsValid)
					{ strNCopy(RelayDataNewList[col].RelayName, paramVal, paramLen); }
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
            // Device name.
            case 0:
            {
	            // cbX=1.
	            findParam = FindParameter(_requestLine, PREFF_DN, CH_1, paramVal, paramLen);
	            
	            vd.IsValid = (findParam && paramLen >= 3 && paramLen <= 32) ? true : false;
				strReplace(paramVal, paramLen, '+', CH_SPACE);
	            vd.Value = paramVal;
	            vd.Length = paramLen;
	            if (vd.IsValid)
	            { strNCopy(DeviceDataNew.DeviceName, paramVal, paramLen); }
            }
            break;
            // User name.
            case 1:
            {
                // cbX=1.
                findParam = FindParameter(_requestLine, PREFF_US, CH_1, paramVal, paramLen);
                    
                vd.IsValid = (findParam && paramLen >= 3 && paramLen <= 8) ? true : false;
                vd.Value = paramVal;
                vd.Length = paramLen;
                if (vd.IsValid)
                { strNCopy(DeviceDataNew.User, paramVal, paramLen); }
            }
            break;
            // Password.
            case 2:
            {
                // cbX=1.
                findParam = FindParameter(_requestLine, PREFF_PS, CH_1, paramVal, paramLen);
                
                vd.IsValid = (findParam && paramLen >= 3 && paramLen <= 8) ? true : false;
                vd.Value = paramVal;
                vd.Length = paramLen;
                if (vd.IsValid)
                { strNCopy(DeviceDataNew.Password, paramVal, paramLen); }
            }
            break;
            // IP.
            case 3:
            {
                findParam = FindParameter(_requestLine, PREFF_RN, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
                
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataNew.IP, ipBuff, IP_LEN); }
            }
            break;
            // Subnet mask.
            case 4:
            {
                findParam = FindParameter(_requestLine, PREFF_SN, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
                    
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataNew.SubnetMask, ipBuff, IP_LEN); }
            }
            break;
            // Gateway.
            case 5:
            {
                findParam = FindParameter(_requestLine, PREFF_GW, CH_1, paramVal, paramLen);
                cnvResult = atoip(paramVal, ipBuff);
                
                vd.IsValid = findParam && cnvResult ? 1 : 0;
                vd.Value = paramVal;
                vd.Length = paramLen;

                if (cnvResult)
                { memcpy(DeviceDataNew.Gateway, ipBuff, IP_LEN); }
            }
            break;
        }
    }
    
    return true;
}

uint8_t IsAuthorized()
{
    if(CheckRequestIsAuthorized())
    { return 1; }
		
    ReadRequestToEnd();
    _html.unAuthorized();

    return 0;
}

void IndexPOST()
{
    // Find parameters line.
    GetPostParameterLine();
    
	// if have data, processing.
    if (_requestLine[0] != CH_NONE)
    {
		char * paramVal;
		uint8_t paramLen;
		bool findParam;
    
	    for (uint8_t col = 0; col < RELAY_COUNT; col++)
	    {
		    char numChar = ONE_TO_FOUR[col];

			// rnX=ON or OFF.
			findParam = FindParameter(_requestLine, PREFF_RN, numChar, paramVal, paramLen);
			if (findParam)
			{
				if (startsWith(paramVal, W_OFF))
				{
					SetRelayStatus(col, OFF);
					break;
				}

				if (startsWith(paramVal, W_ON))
				{
					SetRelayStatus(col, ON);
					break;
				}
			}
		}
	}
	
	_html.index();
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
            if(!ValidationForRelayDataNewList[i].IsValid)
            {
                isError = true;
                break;
            }
        }
                        
        // Save to EEPROM.
        if (!isError)
        {
            WriteEEPROMRelayInfoData(RelayDataNewList);
            ReadEEPROMData();
        }
                        
        _html.privateSettings(RelayDataNewList, ValidationForRelayDataNewList, isError ? 1 : 0, isError ? 0 : 1);
    }
    else
    {
        _html.privateSettings(RelayDataList, NULL, 1);
    }
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
			WriteEEPROMDeviceData(DeviceDataNew);
			ReadEEPROMData();
        }
        
        _html.privateUserAndIP(DeviceDataNew, ValidUserAndIP, isError ? 1 : 0, isError ? 0 : 1);
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
    
	// If request page or type is not valid - exit.
    RequestType rt;
    WebPages wp;
    if(!GetRequestTypeAndPage(rt, wp))
	{
		ReadRequestToEnd();
		return;
	}
	
	// All pages should be authorized.
    if(!IsAuthorized())
    { return; }
    
    switch (wp)
    {
        case WP_INDEX:
        {
            if (rt == RT_GET)
            {
				ReadRequestToEnd();
				_html.index();
            }
			if (rt == RT_POST)
			{ IndexPOST(); }			
        }
        break;
        case WP_SETTINGS:
        {
            // if GET.
            if (rt == RT_GET)
            { 
				ReadRequestToEnd();
	            _html.privateSettings(RelayDataList); 
			}
            
            // if post. Fill dogList with valid data. Fill Validate data list if have not valid.
            if (rt == RT_POST)
            { PrivateSettingsPOST(); }
        }
        break;
        case WP_USERANDIP:
        {
            // if GET.
            if (rt == RT_GET)
            {    
				ReadRequestToEnd();
	            _html.privateUserAndIP(DeviceData);
			}
			
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

bool Processing::IsAddressesChanged()
{
    return _addressesChanged;	
}

void Processing::SetAddressesChanged( bool b )
{
	_addressesChanged = b;
}