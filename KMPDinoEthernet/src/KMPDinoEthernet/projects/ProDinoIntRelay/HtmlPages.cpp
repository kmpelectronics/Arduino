// HtmlPages.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Internet relay.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "HtmlPages.h"
#include <avr/pgmspace.h>
#include "KMPCommon.h"

const char BG_GR[] PROGMEM = "bgGr";
const char BG_RD[] PROGMEM = "bgRd";
const char BG_YL[] PROGMEM = "bgYl";
const char CLS_W20P[] PROGMEM = "w20p";
const char CLS_TD_ON[] PROGMEM = "cCl bgRd";
const char CLS_TD_OFF[] PROGMEM = "cCl bgGr";
const char W_SEC[] PROGMEM = "sec";
const char W_TIMES[] PROGMEM = "times";
const char DEVICE_VERSION[] PROGMEM = "1.0";
const char DEVICE_COPYRIGHT[] PROGMEM = "Copyright 2014 by KMPElectronics Ltd.";

const char RELAY_NAME[] PROGMEM = "Relay";
const char T_STATUS [] PROGMEM = "Status:";
const char T_IP[] PROGMEM = "IP:";
const char T_TEST_PERIOD[] PROGMEM = "Test period:";
const char T_HOST_LOST_AFTER[] PROGMEM = "Host lost after:";
const char T_WAIT_AFTER_RESTART[] PROGMEM = "Wait after restart:";
const char T_FORM_METHOD_POST_DIV[] PROGMEM = "<form method=\"post\"><div>";

const char DEVICE_NAME[] PROGMEM = "KMP Electronics Ltd.";
//const char FIRM_NAME[] PROGMEM = "KMP Electronics Ltd.";
const char FIRM_WEB_ADDRESS[] PROGMEM = "http://kmpelectronics.eu";
const char FIRM_LOGO_IMAGE_WEB_ADDRESS[] PROGMEM = "http://kmpelectronics.eu/Portals/0/Images/Logos/KMPLogo_CM_NoLink150.png";

const char* WEB_PAGE_NAME[WEB_PAGE_LEN] = {"Home", "Settings", "User and IP"};
const char* WEB_PAGE_LINK[WEB_PAGE_LEN] = {"/", "/settings", "/userandip"};
	
const uint8_t SEND_BUFF_LEN = 128;
uint8_t _buff[SEND_BUFF_LEN];
uint8_t _pos = 0;
char _intToCharBuff[10];

EthernetClient _eth;

void flush()
{
	if (_pos > 0)
	{
#ifdef DEBUG_RESPONSE
		Serial.print("sendToEthernet() len:");
		Serial.println(_pos);
		for (int i = 0; i < _pos; i++)
		{
			Serial.print((char)_buff[i]);
		}
		Serial.println("--end sTE--");
#endif
		_eth.write(_buff, _pos);
		_pos = 0;
	}
}

void addHtml(const char c)
{
	if (_pos == SEND_BUFF_LEN)
	{ flush(); }
	
	_buff[_pos++] = c;
}

void addHtml(const char * s)
{
	char c;
	while ((c = *(s++)) != 0)
	{ addHtml(c); }
}

void addHtmlP(PGM_P s)
{
	char c;
	while ((c = pgm_read_byte(s++)) != 0)
	{ addHtml(c); }
}

void doctype()
{
	addHtmlP(PSTR("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"));
}

void htmlHead()
{
    addHtmlP(PSTR("<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>"));
}

void HtmlPages::init(EthernetClient& ec)
{
    _eth = ec;
}

void htmlHeader(PGM_P reponseType, bool authenticateTag = false)
{
	addHtmlP(PSTR("HTTP/1.1 "));
	addHtmlP(reponseType);
	addHtml(CR_LF);
	if (authenticateTag)
	{
		//wwwAuthenticate();
		addHtmlP(PSTR("WWW-Authenticate: Basic realm=\""));
		addHtmlP(DEVICE_NAME);
		addHtml(CH_SPACE);
		addHtmlP(DEVICE_VERSION);
		addHtml(CH_DQUOTE);
		addHtml(CR_LF);
	}
	addHtmlP(PSTR("Content-Type: text/html"));
	addHtml(CR_LF);
	
	addHtml(CR_LF);
}

void htmlHeader200OK(bool authenticateTag = false)
{
    htmlHeader(PSTR("200 OK"), authenticateTag);
}

void pageHead(WebPages page)
{
    doctype();
    htmlHead();
	addHtmlP(PSTR("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
    // Add favicon.ico
    addHtmlP(PSTR("<link rel='shortcut icon' href='http://kmpelectronics.eu/Portals/0/Projects/Arduino/Lib/icons/WebRelay.ico' type='image/x-icon'>"));
	addHtmlP(PSTR("<title>"));
	addHtmlP(DEVICE_NAME);
	addHtml(" - ");
	addHtml(WEB_PAGE_NAME[page]);
	addHtmlP(PSTR("</title><style type=\"text/css\">"
		".bgGr{background-color: #C0C0C0}.bgRd{background-color: Red}"
		".bgBl{background-color: #9CF}.bgLG{background-color: #90EE90}"
		".fL{float: left}.fR{float: right}"
		".w20p{width: 20%}"
		"a{text-decoration: none}a:hover{color: #D00}"
		"body,table{text-align:center;font-family: Arial, Helvetica, sans-serif}"
		"table{width: 100%;border-collapse: collapse}"
		"em{color: #a6a6a6}em span{font-size: 8pt}"
		".mLR20{margin-left: 20px;margin-right: 20px}"
		"td input[type=\"submit\"]{width: 100%;font-size: xx-large}"
		".cCl{width: 40%;font-size: xx-large} .fSEL{font-size: xx-large}"
		"p{font-size: large}"
        "</style></head><body>"));
}

void addH1(const char * text)
{
    if (text == NULL)
    { return; }

    addHtmlP(PSTR("<h1>"));
    addHtml(text);
    addHtmlP(PSTR("</h1>"));
}

void addParagraph(const char * text)
{
	if (text == NULL)
	{ return; }

	addHtmlP(PSTR("<p>"));
	addHtml(text);
	addHtmlP(PSTR("</p>"));
}

void addDiv(const char * text)
{
	if (text == NULL)
	{ return; }

	addHtmlP(PSTR("<div>"));
	addHtml(text);
	addHtmlP(PSTR("</div>"));
}

void menuAndTitle(const char * pageTitle, const char * linkName, const char * linkAddress, const char * addlinkName = NULL, const char * addlinkAddress = NULL)
{
    addHtmlP(PSTR("<div class=\"bgBl\"><div class=\"mLR20\"><a href=\""));
    addHtmlP(FIRM_WEB_ADDRESS);
	addHtmlP(PSTR("\" class=\"fL\"><img alt=\"Logo\" src=\""));
    addHtmlP(FIRM_LOGO_IMAGE_WEB_ADDRESS);
	addHtmlP(PSTR("\"/></a><h1>"));
	addHtmlP(DEVICE_NAME);
	addHtmlP(PSTR("</h1><a href=\""));
    addHtml(linkAddress);
    addHtmlP(PSTR("\" class=\"fR\">"));
    addHtml(linkName);
    addHtmlP(PSTR("</a><br /></div></div>"));
    
    if (addlinkName != NULL)
    {
        addHtmlP(PSTR("<a href=\""));
        addHtml(addlinkAddress);
        addHtml(CH_DQUOTE);
        addHtml(CH_GT);
        addHtml(addlinkName);
        addHtmlP(PSTR("</a> <br />"));
    }
    
    addHtmlP(PSTR("<hr />"));
	addParagraph(pageTitle);
}

void pageFooter()
{
	addHtmlP(PSTR("<br /><hr /><div><em>"));
	addHtmlP(DEVICE_COPYRIGHT);
	addHtmlP(PSTR("</em></div></body></html>"));
}

void addTDStart(PGM_P cls = NULL)
{
	addHtmlP(PSTR("<td"));
	if (cls != NULL)
	{
		addHtmlP(PSTR(" class=\""));
		addHtmlP(cls);
		addHtml('"');
	}
	addHtml('>');
}

void addTDEnd()
{
	addHtmlP(PSTR("</td>"));
}

void addTDValueDescrP(PGM_P ps, PGM_P descr)
{
    addTDStart();
    addHtmlP(ps);
    addHtmlP(PSTR("<br /><em><span>"));
    addHtmlP(descr);
    addHtmlP(PSTR("</span></em>"));
    addTDEnd();
}

void addTDValuesP(PGM_P ps, PGM_P cls = NULL)
{
	addTDStart(cls);
	addHtmlP(ps);
	addTDEnd();
}

void addTDValues(const char * s, PGM_P cls = NULL)
{
	addTDStart(cls);
	addHtml(s);
	addTDEnd();
}

void addTDSubmitBtn(char relayNum, const char* btnValalue, PGM_P cls = NULL)
{
	addTDStart(cls);
	addHtmlP(PSTR("<input type=\"submit\" value=\""));
	addHtml(btnValalue);
	addHtmlP(PSTR("\" name=\""));
	addHtmlP(PREFF_RN);
	addHtml(relayNum);
    addHtmlP(PSTR("\" />"));
	addTDEnd();
}

void addTDComboBox(PGM_P namePreff, char nameSuffix, uint8_t isChecked, PGM_P text)
{
    addTDStart();
    addHtmlP(PSTR("<input name=\""));
    addHtmlP(namePreff);
    addHtml(nameSuffix);
    addHtmlP(PSTR("\" type=\"checkbox\" value=\"1\""));
    addHtml(CH_SPACE);
    if (isChecked)
    {
        addHtmlP(PSTR("checked=\"checked\""));
        addHtml(CH_SPACE);
    }
    addHtmlP(PSTR("/>"));
    addHtmlP(text);
    addTDEnd();
}

void addTDTextBox(PGM_P namePreff, char nameSuffix, char * text, uint8_t maxLenght, bool isValidData, bool isPassType = false)
{
    addTDStart(isValidData ? NULL : BG_RD);
    addHtmlP(PSTR("<input type=\""));
    addHtmlP(isPassType ? PSTR("password") : PSTR("text"));
    addHtmlP(PSTR("\" name=\""));
    addHtmlP(namePreff);
    addHtml(nameSuffix);
    addHtml(CH_DQUOTE);
    addHtml(CH_SPACE);
    
    if (text != NULL)
    {
        addHtmlP(PSTR("value=\""));
        //dblQuotesStr(text);
        addHtml(text);
        addHtml(CH_DQUOTE);
        addHtml(CH_SPACE);
    }
    
    if (maxLenght > 0)
    {
        addHtmlP(PSTR("maxlength=\""));
        //dblQuotesStr(IntToChars(maxLenght));
        IntToChars(maxLenght, _intToCharBuff);
        addHtml(_intToCharBuff);
        addHtml(CH_DQUOTE);
        addHtml(CH_SPACE);
    }
	
    addHtmlP(PSTR(" />"));
    addTDEnd();
}

void tableRelayHead()
{
    addHtmlP(PSTR("<table cellpadding=\"5\" border=\"1\" cellspacing=\"0\"><thead><tr>"));
    for (uint8_t i = 0; i < RELAY_COUNT + 1; i++)
    {
        addHtmlP(PSTR("<th class=\"w20p\">"));
        
        if (i > 0)
        {
            addHtmlP(RELAY_NAME);
            addHtml(CH_SPACE);
            addHtml(ONE_TO_FOUR[i - 1]);
        }
        
        addHtmlP(PSTR("</th>"));
    }
}

void indexPageContent()
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		RelayData ri = RelayDataList[i];
		if (ri.Active == 0)
		{ continue; }
		
		addDiv(ri.RelayName);
		addHtmlP(PSTR("<form method=\"post\">"));
		addHtmlP(PSTR("<table cellpadding=\"5\" border=\"1\" cellspacing=\"0\"><tbody><tr>"));
		addTDSubmitBtn(ONE_TO_FOUR[i], W_OFF, CLS_W20P);
		
		if(GetRelayStatus(i))
		{ addTDValues(W_ON, CLS_TD_ON); }
		else
		{ addTDValues(W_OFF, CLS_TD_OFF); }
			
		addTDSubmitBtn(ONE_TO_FOUR[i], W_ON, CLS_W20P);
		addHtmlP(PSTR("</tr></tbody></table></form>"));
	}
}

void HtmlPages::index()
{
    htmlHeader200OK();
	pageHead(WP_INDEX);
	
    menuAndTitle(DeviceData.DeviceName, WEB_PAGE_NAME[WP_SETTINGS], WEB_PAGE_LINK[WP_SETTINGS]);
	indexPageContent();
	
	pageFooter();

	flush();
}

void HtmlPages::notFound()
{
	htmlHeader(PSTR("404 Not Found"));

    doctype();
    htmlHead();
 	//addHtmlP(PSTR("<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>"));
	addHtmlP(PSTR("<title>"));
	addHtmlP(DEVICE_NAME);
	addHtmlP(PSTR(" - 404 page not found"));
	addHtmlP(PSTR("</title></head><body><h1>Error: 404!</h1><h2>Requested page not found!</h2></body></html>"));

	flush();
}

void HtmlPages::unAuthorized()
{
    htmlHeader(PSTR("401 Unauthorized"), true);

    doctype();
    htmlHead();
    //addHtmlP(PSTR("<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>"));
    addHtmlP(PSTR("<title>"));
    addHtmlP(DEVICE_NAME);
    addHtmlP(PSTR(" - 401 unauthorized"));
    addHtmlP(PSTR("</title></head><body><h1>Error: 401!</h1><h2>Access denied!</h2></body></html>"));

    flush();
}

void ShowMessageInfo(uint8_t showError, uint8_t showSuccess)
{
    if (showError)
    { addHtmlP(PSTR("<p class=\"bgRd\">Invalid data</p>")); }
    if (showSuccess)
    { addHtmlP(PSTR("<p class=\"bgLG\">Data saved successfully</p>")); }
}

void AddUpTime()
{
    TimeSpan upTime;
    
    MillisToTime(millis(), upTime);
    // Days.
    IntToChars(upTime.AllDays, _intToCharBuff);
    addHtml(_intToCharBuff);
    addHtml(" day(s) ");
    // Hours.
    IntToChars(upTime.Hours, _intToCharBuff);
    addHtml(_intToCharBuff);
    addHtml(" h. ");

    // Minutes.
    IntToChars(upTime.Minutes, _intToCharBuff);
    addHtml(_intToCharBuff);
    addHtml(" min. ");

    // Seconds.
    IntToChars(upTime.Seconds, _intToCharBuff);
    addHtml(_intToCharBuff);
    addHtml(" sec.");
}

void privateSettingsContent(RelayData * relayData, ValidationData* validData, uint8_t showError, uint8_t showSuccess)
{
    ShowMessageInfo(showError, showSuccess);
    addHtmlP(T_FORM_METHOD_POST_DIV);
    tableRelayHead();
    addHtmlP(PSTR("</tr></thead><tbody>"));
    
    char str[RELAY_NAME_LEN + 1];
    uint8_t vdPos = 0;
    char * text;
    bool isValidData;
    // Add rows information.
    for (uint8_t row = 0; row < DATA_ROW_COUNT; row++)
    {
        addHtmlP(PSTR("<tr>"));
        switch (row)
        {
            case 0: addTDValueDescrP(T_STATUS, PSTR("Checked - Active / Unchecked - Disabled")); break;
            case 1: addTDValueDescrP(PSTR("Name:"), PSTR("Min: 3 max 16 char.")); break;
        }
        
        // Add cells in row.
        for (uint8_t col = 0; col < RELAY_COUNT; col++)
        {
            RelayData& relay = relayData[col];
            text = NULL;
            isValidData = true;
            
			if (validData != NULL)
			{ 
                ValidationData& vData = validData[vdPos++];
                if(!vData.IsValid)
                {
                    memcpy(str, vData.Value, vData.Length);
                    str[vData.Length] = CH_NONE;
                    text = str;
                    isValidData = false;
                }
            }

			switch (row)
            {
                case 0: addTDComboBox(PREFF_CB, ONE_TO_FOUR[col], relay.Active, PSTR("Active")); break;
                case 1: addTDTextBox(PREFF_RN, ONE_TO_FOUR[col], isValidData ? relay.RelayName : text, RELAY_NAME_LEN, isValidData); break;
            }
        }
        
        addHtmlP(PSTR("</tr>"));
    }
    
    addHtmlP(PSTR("</tbody></table><p><input type=\"submit\" value=\"Save Settings\" /></p></div></form>"));
    
    // Add up time.
    addHtmlP(PSTR("<div><em><span>Up time: "));
    
    AddUpTime();

    addHtmlP(PSTR("</span></em></div>"));
}

void HtmlPages::privateSettings(RelayData* relayData, ValidationData* validData /*= NULL*/, uint8_t showError /*= 0*/, uint8_t showSuccess /*= 0*/)
{
    htmlHeader200OK(true);

	pageHead(WP_SETTINGS);
    menuAndTitle(WEB_PAGE_NAME[WP_SETTINGS], WEB_PAGE_NAME[WP_INDEX], WEB_PAGE_LINK[WP_INDEX], WEB_PAGE_NAME[WP_USERANDIP], WEB_PAGE_LINK[WP_USERANDIP]);
	privateSettingsContent(relayData, validData, showError, showSuccess);
	pageFooter();

	flush();
}

bool CheckValidData(ValidationData& validData, char * buff, char*& result)
{
    if(!validData.IsValid)
    {
        memcpy(buff, validData.Value, validData.Length);
        buff[validData.Length] = CH_NONE;
        result = buff;
        return false;
    }
    return true;
}

void privateUserAndIPContent(DeviceSetting& deviceSettings, ValidationData* validData)
{
    addHtmlP(T_FORM_METHOD_POST_DIV);
    
    // IP 4 bytes * 3 (max chars in byte) + 3 dot + 1 (\0) to end. = 16.
    char buff[32];
    char str[MAX_IP_STR_LEN + 1];
    uint8_t vdPos = 0;
    char * text;
    bool isValidData;
    
    PGM_P min3Max8 = PSTR("Min: 3 max 8 char.");
    PGM_P tblStart = PSTR("<table cellpadding=\"5\" border=\"1\" cellspacing=\"0\" style=\"width:auto;\" align=\"center\"><tbody>");
    PGM_P tblEnd = PSTR("</tbody></table>");

	// Device name.
	addH1("Device");
	addHtmlP(tblStart);

	text = NULL;
	isValidData = true;
			
	if (validData != NULL)
	{ isValidData = CheckValidData(validData[vdPos++], str, text); }

	addHtmlP(PSTR("<tr>"));
	addTDValueDescrP(PSTR("Name:"), min3Max8);
	addTDTextBox(PREFF_DN, CH_1, isValidData ? deviceSettings.DeviceName : text, 32, isValidData);
	addHtmlP(PSTR("</tr>"));
	addHtmlP(tblEnd);

	// User information.
    addH1("User");
    addHtmlP(tblStart);
    // Add User rows information.
    for (uint8_t row = 0; row < 2; row++)
    {
        text = NULL;
        isValidData = true;
            
        if (validData != NULL)
        { isValidData = CheckValidData(validData[vdPos++], str, text); }

        addHtmlP(PSTR("<tr>"));
        switch (row)
        {
            case 0: 
            addTDValueDescrP(PSTR("Name:"), min3Max8); 
            addTDTextBox(PREFF_US, CH_1, isValidData ? deviceSettings.User : text, 8, isValidData);
            break;
            case 1: addTDValueDescrP(PSTR("Password:"), min3Max8); 
            addTDTextBox(PREFF_PS, CH_1, isValidData ? deviceSettings.Password : text, 8, isValidData, true);
            break;
        }
        addHtmlP(PSTR("</tr>"));
    }
    addHtmlP(tblEnd);

	// IP information.
    addH1("IP");
    addHtmlP(tblStart);
    // Add IP rows information.
    for (uint8_t row = 0; row < 3; row++)
    {
        text = NULL;
        isValidData = true;
        
        if (validData != NULL)
        { isValidData = CheckValidData(validData[vdPos++], str, text); }

        addHtmlP(PSTR("<tr>"));
        switch (row)
        {
            case 0:
                addTDValueDescrP(T_IP, PSTR("Example: 192.168.1.199"));
                iptoa(deviceSettings.IP, buff);
                addTDTextBox(PREFF_RN, CH_1, isValidData ? buff : text, 15, isValidData);
            break;
            case 1:                 
                addTDValueDescrP(PSTR("Subnet mask:"), PSTR("Example: 255.255.255.0"));
                iptoa(deviceSettings.SubnetMask, buff);
                addTDTextBox(PREFF_SN, CH_1, isValidData ? buff : text, 15, isValidData);
            break;
            case 2:
                addTDValueDescrP(PSTR("Gateway:"), PSTR("Example: 192.168.1.1"));
                iptoa(deviceSettings.Gateway, buff);
                addTDTextBox(PREFF_GW, CH_1, isValidData ? buff : text, 15, isValidData);
            break;
        }
        addHtmlP(PSTR("</tr>"));
    }
    addHtmlP(tblEnd);
    addHtmlP(PSTR("<p><input type=\"submit\" value=\"Save Settings\" /></p></div></form>"));
}

void HtmlPages::privateUserAndIP(DeviceSetting& deviceSettings, ValidationData* validData /*= NULL*/, uint8_t showError /*= 0*/, uint8_t showSuccess /*= 0*/)
{
	htmlHeader200OK(true);
	pageHead(WP_USERANDIP);
	    
	menuAndTitle(CH_NONE/*WEB_PAGE_NAME[WP_PRIVATE_USERANDIP]*/, WEB_PAGE_NAME[WP_INDEX], WEB_PAGE_LINK[WP_INDEX], WEB_PAGE_NAME[WP_SETTINGS], WEB_PAGE_LINK[WP_SETTINGS]);
    ShowMessageInfo(showError, showSuccess);

	privateUserAndIPContent(deviceSettings, validData);
	    
	pageFooter();

	flush();
}