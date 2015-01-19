// HtmlPages.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Watchdog.
// Description:
//		Build html responses.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "HtmlPages.h"
#include <avr/pgmspace.h>
#include "KMPCommon.h"

const char BG_GR[] PROGMEM = "bgGr";
const char BG_RD[] PROGMEM = "bgRd";
const char BG_YL[] PROGMEM = "bgYl";
const char W_SEC[] PROGMEM = "sec";
const char W_TIMES[] PROGMEM = "times";
const char DEVICE_VERSION[] PROGMEM = "1.0";
const char DEVICE_COPYRIGHT[] PROGMEM = "Copyright 2015 by ";

const char WATCHDOG_NAME[] PROGMEM = "Dog";
const char T_STATUS [] PROGMEM = "Status:";
const char T_IP[] PROGMEM = "IP:";
const char T_TEST_PERIOD[] PROGMEM = "Test period:";
const char T_HOST_LOST_AFTER[] PROGMEM = "Host lost after:";
const char T_WAIT_AFTER_RESTART[] PROGMEM = "Wait after restart:";
const char T_FORM_METHOD_POST_DIV[] PROGMEM = "<form method=\"post\"><div>";
const char T_TITLE[] PROGMEM = "<title>";
const char T_P_INPUT_SUBMIT_SAVE_SETTINGS[] PROGMEM = "<p><input type=\"submit\" value=\"Save Settings\" /></p></div></form>";

const char DEVICE_NAME[] PROGMEM = "ProDiNo Watchdog";
const char FIRM_NAME[] PROGMEM = "KMP Electronics Ltd.";
const char FIRM_WEB_ADDRESS[] PROGMEM = "http://www.kmpelectronics.eu";

const char* WEB_PAGE_NAME[WEB_PAGE_LEN] = {"Home", "Settings", "User and IP"};
const char* WEB_PAGE_LINK[WEB_PAGE_LEN] = {"/", "/private/settings", "/private/userandip"};

const uint8_t SEND_BUFF_LEN = 128;
uint8_t _buff[SEND_BUFF_LEN];
uint8_t _pos = 0;
const uint8_t CHAR_BUFF_LEN = 32;
char _charBuff[CHAR_BUFF_LEN];

EthernetClient _eth;

void flush()
{
	if (_pos > 0)
	{
        // Write to W5200.
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
// Optimize code size.
//	addHtmlP(PSTR("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"));
}

void htmlHead()
{
    addHtmlP(PSTR("<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>"));
}

void htmlHeader(PGM_P reponseType, bool authenticateTag = false)
{
	addHtmlP(PSTR("HTTP/1.1 "));
	addHtmlP(reponseType);
	addHtml(CR_LF);
	if (authenticateTag)
	{
		addHtmlP(PSTR("WWW-Authenticate: Basic realm=\""));
		addHtmlP(DEVICE_NAME);
		addHtml(CH_SPACE);
		addHtmlP(DEVICE_VERSION);
		addHtml("\"\r\n");
	}
	addHtmlP(PSTR("Content-Type: text/html"));
	addHtml("\r\n\r\n");
}

void htmlHeader200OK(bool authenticateTag = false)
{
    htmlHeader(PSTR("200 OK"), authenticateTag);
}

void pageHead(WebPages page)
{
// Optimize code size.
//    doctype();
    htmlHead();

	if (page == WP_INDEX)
	{
		addHtmlP(PSTR("<meta http-equiv=\"refresh\" content=\""));
        IntToChars(INDEX_PAGE_REFRESH_RATE, _charBuff);
		addHtml(_charBuff);
        addHtmlP(PSTR("\" />"));
	}
	addHtmlP(T_TITLE);
	addHtmlP(DEVICE_NAME);
	addHtml(" - ");
	addHtml(WEB_PAGE_NAME[page]);
	addHtmlP(PSTR("</title><style type=\"text/css\">"
    ".bgLG{background-color:#90EE90}"
        ".bgGr{background-color:#C0C0C0}.bgRd{background-color:Red}"
        ".bgYl{background-color:Yellow}.bgBl{background-color:#9CF}"
        ".bgLG{background-color:#90EE90}.fR{float:right}.w20p{width:20%}"
        "a{text-decoration:none}a:hover{color:#D00}"
        "body,table{text-align:center;font-family:Arial, Helvetica, sans-serif}"
        "table{width:100%;border-collapse:collapse}em{color:#a6a6a6}em span{font-size:8pt}"
        ".mLR20{margin-left:20px;margin-right:20px}"
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

void menuAndTitle(const char * pageTitle, const char * linkName, const char * linkAddress, const char * addlinkName = NULL, const char * addlinkAddress = NULL)
{
    addHtmlP(PSTR("<div class=\"bgBl\"><div class=\"mLR20\"><h1><a href=\""));
    addHtmlP(FIRM_WEB_ADDRESS);
    addHtml("\">");
    addHtmlP(FIRM_NAME);
    addHtmlP(PSTR("</a> "));
	addHtmlP(DEVICE_NAME);
    addHtmlP(PSTR(" Ver: "));
	addHtmlP(DEVICE_VERSION);
    addHtmlP(PSTR("</h1><a href=\""));
    addHtml(linkAddress);
    addHtmlP(PSTR("\" class=\"fR\">"));
    addHtml(linkName);
    addHtmlP(PSTR("</a><br /></div></div>"));
    
    if (addlinkName != NULL)
    {
        addHtmlP(PSTR("<a href=\""));
        addHtml(addlinkAddress);
        addHtml("\">");
        addHtml(addlinkName);
        addHtmlP(PSTR("</a> <br />"));
    }
    
    addHtmlP(PSTR("<hr />"));
    addH1(pageTitle);
}

void pageFooter()
{
	addHtmlP(PSTR("<hr /><p><em>"));
	addHtmlP(DEVICE_COPYRIGHT);
	addHtmlP(FIRM_NAME);
	addHtmlP(PSTR("</em></p></body></html>"));
}

void addTRStart()
{
    addHtmlP(PSTR("<tr>"));
}

void addTREnd()
{
    addHtmlP(PSTR("</tr>"));
}

void addTDStart(PGM_P cls = NULL)
{
	addHtmlP(PSTR("<td"));
	if (cls != NULL)
	{
		addHtmlP(PSTR(" class=\""));
		addHtmlP(cls);
		addHtml(CH_DQUOTE);
	}
	addHtml(CH_GT);
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

void addTDValues(char * s, PGM_P cls = NULL)
{
	addTDStart(cls);
	addHtml(s);
	addTDEnd();
}

void addTDValues(uint8_t b, PGM_P str, PGM_P cls = NULL)
{
	addTDStart(cls);
    IntToChars(b, _charBuff);
	addHtml(_charBuff);
    addHtml(CH_SPACE);
	addHtmlP(str);
	addTDEnd();
}

void addTDCheckBox(PGM_P namePreff, char nameSuffix, uint8_t isChecked, PGM_P text)
{
    addTDStart();
    addHtmlP(PSTR("<input name=\""));
    addHtmlP(namePreff);
    addHtml(nameSuffix);
    addHtmlP(PSTR("\" type=\"checkbox\" value=\"1\" "));
    if (isChecked)
    {
        addHtmlP(PSTR("checked=\"checked\" "));
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
    addHtml("\" ");
    
    if (text != NULL)
    {
        addHtmlP(PSTR("value=\""));
        addHtml(text);
        addHtml("\" ");
    }
    
    if (maxLenght > 0)
    {
        addHtmlP(PSTR("maxlength=\""));
        IntToChars(maxLenght, _charBuff);
        addHtml(_charBuff);
        addHtml("\" ");
    }
	
    addHtmlP(PSTR(" />"));
    addTDEnd();
}

void tableDogHead()
{
    addHtmlP(PSTR("<table cellpadding=\"5\" border=\"1\" cellspacing=\"0\"><thead><tr>"));
    for (uint8_t i = 0; i < DOG_COUNT + 1; i++)
    {
        addHtmlP(PSTR("<th class=\"w20p\">"));
        
        if (i > 0)
        {
            addHtmlP(WATCHDOG_NAME);
            addHtml(CH_SPACE);
            addHtml(ONE_TO_FOUR[i - 1]);
        }
        
        addHtmlP(PSTR("</th>"));
    }
}

void AddUpTime()
{
    // Add up time.
    addHtmlP(PSTR("<div><em><span>Up time: "));
        
    TimeSpan upTime;
    
    MillisToTime(millis(), upTime);
    // Days.
    IntToChars(upTime.AllDays, _charBuff);
    addHtml(_charBuff);
    addHtml(" day(s) ");
    // Hours.
    IntToChars(upTime.Hours, _charBuff);
    addHtml(_charBuff);
    addHtml(" h. ");

    // Minutes.
    IntToChars(upTime.Minutes, _charBuff);
    addHtml(_charBuff);
    addHtml(" min. ");

    // Seconds.
    IntToChars(upTime.Seconds, _charBuff);
    addHtml(_charBuff);
    addHtml(" sec.");

    addHtmlP(PSTR("</span></em></div>"));
}

void indexPageContent()
{
	tableDogHead();
	addHtmlP(PSTR("</tr></thead><tbody>"));
	
	// Add rows information.
	for (uint8_t row = 0; row < 7; row++)
	{
		addTRStart();
		switch (row)
		{
			case 0: addTDValuesP(T_STATUS); break;
			case 1: addTDValuesP(T_IP); break;
			case 2: addTDValuesP(T_TEST_PERIOD);  break;
			case 3: addTDValuesP(T_HOST_LOST_AFTER);break;
			case 4: addTDValuesP(T_WAIT_AFTER_RESTART); break;
			case 5: addTDValuesP(PSTR("Lost times:")); break;
			case 6: addTDValuesP(PSTR("Restarts:")); break;
		}
		
		// Add cells in row.
		for (uint8_t i = 0; i < DOG_COUNT; i++)
		{
			DogSetting& dog = DogList[i];

			switch (row)
			{
				case 0: dog.Active ? addTDValuesP(PSTR("Active")) : addTDValuesP(PSTR("Disabled"), BG_GR); break;
				case 1: 
					iptoa(dog.IP, _charBuff); 
					addTDValues(_charBuff, dog.Active ? NULL : BG_GR); 
				break;
				case 2: addTDValues(dog.TestPeriod, W_SEC, dog.Active ? NULL : BG_GR); break;
				case 3: addTDValues(dog.HostLostAfter, W_TIMES, dog.Active ? NULL : BG_GR); break;
				case 4: addTDValues(dog.WaitAfterRestart, W_SEC, dog.Active ? NULL : BG_GR); break;
				case 5: 
                    IntToChars(ProcessList[i].LostTimes, _charBuff);
                    addTDValues(_charBuff, dog.Active ? (ProcessList[i].LostTimes == 0 ?  NULL : BG_YL) : BG_GR); 
                break;
				case 6: 
                    IntToChars(ProcessList[i].Restarts, _charBuff);
                    addTDValues(_charBuff, dog.Active ? (ProcessList[i].Restarts == 0 ?  NULL : BG_RD) : BG_GR); 
                break;
			}
		}
		
		addTREnd();
	}
	
	addHtmlP(PSTR("</tbody></table><div><br /></div>"));
    
    // Add up time.
    AddUpTime();
}

void ShowMessageInfo(uint8_t showError, uint8_t showSuccess)
{
    if (showError)
    { addHtmlP(PSTR("<p class=\"bgRd\">Invalid data</p>")); }
    if (showSuccess)
    { addHtmlP(PSTR("<p class=\"bgLG\">Data saved successfully</p>")); }
}

void privateSettingsContent(DogSetting* dogData, ValidationData* validData, uint8_t showError, uint8_t showSuccess)
{
    ShowMessageInfo(showError, showSuccess);
    addHtmlP(T_FORM_METHOD_POST_DIV);
    tableDogHead();
    addHtmlP(PSTR("</tr></thead><tbody>"));
    
    uint8_t vdPos = 0;
    bool isValidData;
    // Add rows information.
    for (uint8_t row = 0; row < DATA_ROW_COUNT; row++)
    {
        addTRStart();
        switch (row)
        {
            case 0: addTDValueDescrP(T_STATUS, PSTR("Checked - Active / Unchecked - Disabled")); break;
            case 1: addTDValueDescrP(T_IP, PSTR("Example: 192.168.1.199")); break;
            case 2: addTDValueDescrP(T_TEST_PERIOD, PSTR("In sec. From 1 to 255. Default 30."));  break;
            case 3: addTDValueDescrP(T_HOST_LOST_AFTER, PSTR("Times. From 1 to 10. Default 3.")); break;
            case 4: addTDValueDescrP(PSTR("Restart wait time:"), PSTR("In sec. From 1 to 255. Default 10.")); break;
            case 5: addTDValueDescrP(T_WAIT_AFTER_RESTART, PSTR("In sec. From 1 to 255. Default 60.")); break;
        }
        
        // Add cells in row.
        for (uint8_t col = 0; col < DOG_COUNT; col++)
        {
            DogSetting& dog = dogData[col];
            char colNum = ONE_TO_FOUR[col];
            isValidData = true;
            
			if (validData != NULL)
			{ 
                ValidationData vData = validData[vdPos++];
                if(!vData.IsValid)
                {
                    memcpy(_charBuff, vData.Value, vData.Length);
                    _charBuff[vData.Length] = CH_NONE;
                    isValidData = false;
                }
            }
            
			switch (row)
            {
                case 0: addTDCheckBox(PREFF_CB, colNum, dog.Active, PSTR("Active")); break;
                case 1:
                    if (isValidData)
                    { iptoa(dog.IP, _charBuff); }
                    addTDTextBox(PREFF_IP, colNum, _charBuff, 15, isValidData);
                break;
                case 2: 
                    if (isValidData)
                    { IntToChars(dog.TestPeriod, _charBuff); }                    
                    addTDTextBox(PREFF_TP, colNum, _charBuff, 3, isValidData); 
                break;
                case 3: 
                    if (isValidData)
                    { IntToChars(dog.HostLostAfter, _charBuff); }                    
                    addTDTextBox(PREFF_HL, colNum, _charBuff, 2, isValidData); 
                break;
                case 4: 
                    if (isValidData)
                    { IntToChars(dog.RestartWaitTime, _charBuff); }                    
                    addTDTextBox(PREFF_RW, colNum, _charBuff, 3, isValidData); 
                break;
                case 5: 
                    if (isValidData)
                    { IntToChars(dog.WaitAfterRestart, _charBuff); }                    
                    addTDTextBox(PREFF_WA, colNum, _charBuff, 3, isValidData); 
                break;
            }
        }
        
        addTREnd();
    }
    
    addHtmlP(PSTR("</tbody></table>"));
    addHtmlP(T_P_INPUT_SUBMIT_SAVE_SETTINGS);
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
    addH1("User");
    
    char str[MAX_IP_STR_LEN + 1];
    uint8_t vdPos = 0;
    char * text;
    bool isValidData;
    
    PGM_P min3Max8 = PSTR("Min: 3 max 8 char.");
    PGM_P tblStart = PSTR("<table cellpadding=\"5\" border=\"1\" cellspacing=\"0\" style=\"width:auto;\" align=\"center\"><tbody>");
    PGM_P tblEnd = PSTR("</tbody></table>");

    addHtmlP(tblStart);
    // Add rows information.
    for (uint8_t row = 0; row < 2; row++)
    {
        text = NULL;
        isValidData = true;
            
        if (validData != NULL)
        { isValidData = CheckValidData(validData[vdPos++], str, text); }

        addTRStart();
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
        addTREnd();
    }
    addHtmlP(tblEnd);

    addH1("IP");
    addHtmlP(tblStart);
    // Add rows information.
    for (uint8_t row = 0; row < 3; row++)
    {
        text = NULL;
        isValidData = true;
        
        if (validData != NULL)
        { isValidData = CheckValidData(validData[vdPos++], str, text); }

        addTRStart();
        switch (row)
        {
            case 0:
                addTDValueDescrP(T_IP, PSTR("Example: 192.168.1.199"));
                iptoa(deviceSettings.IP, _charBuff);
                addTDTextBox(PREFF_IP, CH_1, isValidData ? _charBuff : text, 15, isValidData);
            break;
            case 1:                 
                addTDValueDescrP(PSTR("Subnet mask:"), PSTR("Example: 255.255.255.0"));
                iptoa(deviceSettings.SubnetMask, _charBuff);
                addTDTextBox(PREFF_SN, CH_1, isValidData ? _charBuff : text, 15, isValidData);
            break;
            case 2:
                addTDValueDescrP(PSTR("Gateway:"), PSTR("Example: 192.168.1.1"));
                iptoa(deviceSettings.Gateway, _charBuff);
                addTDTextBox(PREFF_GW, CH_1, isValidData ? _charBuff : text, 15, isValidData);
            break;
        }
        addTREnd();
    }
    addHtmlP(tblEnd);
    addHtmlP(T_P_INPUT_SUBMIT_SAVE_SETTINGS);
}

void HtmlPages::init(EthernetClient& ec)
{
    _eth = ec;
}

void HtmlPages::index()
{
    htmlHeader200OK();
    pageHead(WP_INDEX);
    
    menuAndTitle("Current status", WEB_PAGE_NAME[WP_PRIVATE_SETTINGS], WEB_PAGE_LINK[WP_PRIVATE_SETTINGS]);
    indexPageContent();
    
    pageFooter();

    flush();
}

void HtmlPages::notFound()
{
    htmlHeader(PSTR("404 Not Found"));

    // Optimize code size.
    //    doctype();
    htmlHead();
    addHtmlP(T_TITLE);
    addHtmlP(DEVICE_NAME);
    addHtmlP(PSTR(" - 404 page not found"));
    addHtmlP(PSTR("</title></head><body><h1>Error: 404!</h1><h2>Requested page not found!</h2></body></html>"));

    flush();
}

void HtmlPages::unAuthorized()
{
    htmlHeader(PSTR("401 Unauthorized"), true);

    // Optimize code size.
    //    doctype();
    htmlHead();
    addHtmlP(T_TITLE);
    addHtmlP(DEVICE_NAME);
    addHtmlP(PSTR(" - 401 unauthorized"));
    addHtmlP(PSTR("</title></head><body><h1>Error: 401!</h1><h2>Access denied!</h2></body></html>"));

    flush();
}

void HtmlPages::privateSettings(DogSetting* dogData, ValidationData* validData, uint8_t showError, uint8_t showSuccess)
{
    htmlHeader200OK(true);

    pageHead(WP_PRIVATE_SETTINGS);
    menuAndTitle(WEB_PAGE_NAME[WP_PRIVATE_SETTINGS], WEB_PAGE_NAME[WP_INDEX], WEB_PAGE_LINK[WP_INDEX], WEB_PAGE_NAME[WP_PRIVATE_USERANDIP], WEB_PAGE_LINK[WP_PRIVATE_USERANDIP]);
    privateSettingsContent(dogData, validData, showError, showSuccess);
    pageFooter();

    flush();
}

void HtmlPages::privateUserAndIP(DeviceSetting& deviceSettings, ValidationData* validData, uint8_t showError, uint8_t showSuccess)
{
	htmlHeader200OK(true);

	pageHead(WP_PRIVATE_USERANDIP);
	menuAndTitle(CH_NONE/*WEB_PAGE_NAME[WP_PRIVATE_USERANDIP]*/, WEB_PAGE_NAME[WP_INDEX], WEB_PAGE_LINK[WP_INDEX], WEB_PAGE_NAME[WP_PRIVATE_SETTINGS], WEB_PAGE_LINK[WP_PRIVATE_SETTINGS]);
    ShowMessageInfo(showError, showSuccess);
    privateUserAndIPContent(deviceSettings, validData);
	pageFooter();

	flush();
}