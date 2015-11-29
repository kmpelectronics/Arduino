// WebRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards:
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		ProDiNo NetBoard V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Web server relay manipulation example. 
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinoii/webrelaycontrol.aspx
// Version: 1.2.0
// Date: 29.11.2015
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Description: Compatibilie Arduinio version >= 1.6.5

#include <SPI.h>
#include <Ethernet.h>
#include "KmpDinoEthernet.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
//#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x7D, 0x15, 0x30 };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 199);

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Define text.
const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

char _kmpURL[] = "http://kmpelectronics.eu/";

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
* \brief Setup void. Arduino executed first. Initialize DiNo board and prepare Ethernet connection.
*
*
* \return void
*/
void setup()
{
#ifdef DEBUG
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only. If need debug setup() void.
    }
#endif

    // Init Dino board. Set pins, start W5200.
    DinoInit();

    // Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
    _server.begin();

#ifdef DEBUG
    Serial.println("The server is starting.");
    Serial.println(Ethernet.localIP());
    Serial.println(Ethernet.gatewayIP());
    Serial.println(Ethernet.subnetMask());
#endif
}

/**
* \brief Loop void. Arduino executed second.
*
*
* \return void
*/
void loop(){
    // Listen for incoming clients.
    _client = _server.available();

    if(!_client.connected())
    {
        return;
    }

#ifdef DEBUG
    Serial.println("Client connected.");
#endif

    // If client connected On status led.
    OnStatusLed();

    // Read client request.
    if(ReadClientRequest())
    {
        // Write client response - html page.
        WriteClientResponse();
    }
#ifdef DEBUG
    else
    {
        Serial.println("#Request not processed.");
    }    
#endif

    // Close the connection.
    _client.stop();

    // If client disconnected Off status led.
    OffStatusLed();

#ifdef DEBUG
    Serial.println("Client disconnected.");
    Serial.println("---");
#endif
}

/**
 * \brief ReadClientRequest void. Read and parse client request.
 * 	First row of client request is similar to:
 *		GET / HTTP/1.1
 *  -or-
 *		GET /?r1=On HTTP/1.1
 *		GET /?r1=Off HTTP/1.1
 *      |    | | | |
 *      1    6 8 10 12
 *               relay status
 * You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 * 
 * \return bool Returns true if the request was expected.
 */
bool ReadClientRequest()
{
#ifdef DEBUG
    Serial.println("Read client request.");
#endif

    int requestLen = 0;
    bool result = false;
    bool isGet = false;
    char c;
    int relay = -1;
    char status[4];
    int statusPos = 0;

    // Loop while read all request.
    while (_client.available())
    {
        ++requestLen;
        c = _client.read();

        #ifdef DEBUG
        Serial.write(c);
        #endif
        // Check request is GET.
        if(requestLen == 1 && c != 'G')
        break;
        
        // Check for GET / HTTP/1.1
        if(requestLen == 6)
        result = c == CH_SPACE;

        // Not other valid data to read. Read to end.
        if(requestLen > 12)
        break;

        // Read only needed chars.
        // Read relay.
        if (requestLen == 8)
        {
            // convert char to int.
            relay = CharToInt(c) - 1;
        }

        // Read max 3 chars for On or Off.
        if (requestLen >= 10 && requestLen <= 12)
        {
            status[statusPos] = c;
            statusPos++;
            // End of string.
            status[statusPos] = 0;

            // If On.
            if(strcmp(status, W_ON) == 0)
            {
                SetRelayStatus(relay, true);
                result = true;
                break;
            }

            // If Off.
            if(strcmp(status, W_OFF) == 0)
            {
                SetRelayStatus(relay, false);
                result = true;
                break;
            }
        }
    }

    // Fast flush request.
    while (_client.available())
    {
        c = _client.read();
        #ifdef DEBUG
        Serial.write(c);
        #endif
    }

#ifdef DEBUG
    Serial.println("#End client request.");
#endif

    return result;
}

/**
 * \brief WriteClientResponse void. Write html response.
 * 
 * 
 * \return void
 */
void WriteClientResponse()
{
    #ifdef DEBUG
    Serial.println("Write client response.");
    #endif

    if (!_client.connected())
    {
    	return;
    }

    // Response write.
    // Send a standard http header.
    _client.write("HTTP/1.1 200 OK\r\n");
    _client.write("Content-Type: text/html\r\n");
    _client.write("\r\n");

    // Add web page HTML.
    _client.write("<html>");

    // Header
    _client.write("<head>");

    // Add favicon.ico
    _client.write("<link rel='shortcut icon' href='");
    _client.write(_kmpURL);
    _client.write("Portals/0/Projects/Arduino/Lib/icons/WebRelay.ico' type='image/x-icon'>");
    
    // End header.
    _client.write("<title>KMP Electronics Ltd. DiNo Ethernet - Web Relay</title></head>");

    // Body
    _client.write("<body><div style='text-align: center'>");
    _client.write("<br><hr />");
    _client.write("<h1 style='color: #0066FF;'>DiNo Ethernet - Web Relay example</h1>");
    _client.write("<hr /><br><br>");

    _client.write("<form method='get'>");
    // Table
    _client.write("<table border='1' width='300' cellpadding='5' cellspacing='0'");
    _client.write("align='center' style='text-align: center; font-size: large; font-family: Arial, Helvetica, sans-serif;'>");
    // Add table rows
    for (int i = 0; i < RELAY_COUNT; i++)
    {
        // Row i, cell 1
        _client.write("<tr><td>Relay ");
        char relayNumber = IntToChar(i + 1);
        _client.write(relayNumber);
        _client.write("</td>");

        char* cellColor;
        char* cellStatus;
        char* nextRelayStatus;
        if(GetRelayStatus(i) == true)
        {
            cellColor = (char*)RED;
            cellStatus = (char*)W_ON;
            nextRelayStatus = (char*)W_OFF;
        }
        else
        {
            cellColor = (char*)GREEN;
            cellStatus = (char*)W_OFF;
            nextRelayStatus = (char*)W_ON;
        }

        // Cell i,2
        _client.write("<td bgcolor='");
        _client.write(cellColor);
        _client.write("'>");
        _client.write(cellStatus);
        _client.write("</td>");

        // Cell i,3
        _client.write("<td><input type='submit' name='r");
        _client.write(relayNumber);
        _client.write("' value='");
        _client.write(nextRelayStatus);
        _client.write("'/ ></td></tr>");
    }

    _client.write("</table></form>");
    _client.write("<br><br><hr />");
    _client.write("<h1><a href='");
    _client.write(_kmpURL);
    _client.write("' target='_blank'>Visit KMPElectronics.eu!</a></h1>");
    _client.write("<h3><a href='");
    _client.write(_kmpURL);
    _client.write("en-us/products/dinoii.aspx' target='_blank'>Information about DiNo Ethernet board</a></h3>");
    _client.write("<hr />");

    _client.write("</div></body>");
    _client.write("</html>");
}