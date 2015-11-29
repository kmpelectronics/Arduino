// WebOptoInputs.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		ProDiNo NetBoard V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Web server opto inputs show example. 
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinoii/weboptoinputscheck.aspx
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
byte _mac[] = { 0x00, 0x08, 0xDC, 0xCB, 0x65, 0x02 };

// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 199);

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Define text.
const char GREEN[] = "#90EE90"; // LightGreen
const char RED[] = "#FF4500"; // OrangeRed 

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
    //while (!Serial) {
    //	; // wait for serial port to connect. Needed for Leonardo only. If need debug setup() void.
    //}
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
 * \brief Read client request. Not need any operation.
 *	  First row of client request is similar to:
 *		GET / HTTP/1.1
 *      |    |
 *      1    6
 *    You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 *    or read debug information from Serial Monitor (Tools > Serial Monitor).
 *
 * 
 * \return Returns true if the request was expected.
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

    // Loop while read all request.
    while (_client.available()) 
    {
        requestLen++;
        c = _client.read();
#ifdef DEBUG
        Serial.write(c);
#endif

        // Check request is GET.
        if(requestLen == 1)
            isGet = c == 'G';
        
        // Check for valid GET. Position 6 is ' '.
        if(requestLen == 6)
        {
            result = isGet && c == CH_SPACE;
            // Not need any read.
            break;
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
    // Add a meta refresh tag, so the browser pulls again every 5 seconds.
    _client.write("<head><meta http-equiv='refresh' content='5'>");
    _client.write("<title>KMP Electronics Ltd. DiNo Ethernet - Web Opto Inputs</title></head>");

    // Body
    _client.write("<body><div style='text-align: center'>");
    _client.write("<br><hr />");
    _client.write("<h1 style='color: #0066FF;'>DiNo Ethernet - Web Opto Inputs example</h1>");
    _client.write("<hr /><br><br>");

    // Table
    _client.write("<table border='1' width='300' cellpadding='5' cellspacing='0'"); 
    _client.write("align='center' style='text-align: center; font-size: large; font-family: Arial, Helvetica, sans-serif;'>");

    // Add table header.
    _client.write("<thead><tr>");
    for (int i = 0; i < OPTOIN_COUNT; i++)
    {
        // Row i, cell 1
        _client.write("<th>In ");
        _client.write(IntToChar(i));
        _client.write("</th>");
    }
    _client.write("</tr></thead>");

    // Add table rows
    _client.write("<tbody><tr>");
    for (int i = 0; i < OPTOIN_COUNT; i++)
    {
        // Row 2, cell i
        char* cellColor;
        char* cellStatus;
        if(GetOptoInStatus(i) == true)
        {
            cellColor = (char*)RED;
            cellStatus = (char*)W_ON;
        }
        else
        {
            cellColor = (char*)GREEN;
            cellStatus = (char*)W_OFF;
        }

        // Row 2, cell i
        _client.write("<td bgcolor='");
        _client.write(cellColor);
        _client.write("'>");
        _client.write(cellStatus);
        _client.write("</td>");
    }
    _client.write("</tr></tbody>");

    _client.write("</table>");
    _client.write("<br><br><hr />");
    _client.write("<h1><a href='http://kmpelectronics.eu/' target='_blank'>Visit KMPElectronics.eu!</a></h1>");
    _client.write("<h3><a href='http://kmpelectronics.eu/en-us/products/dinoii.aspx' target='_blank'>Information about DiNo Ethernet board</a></h3>");
    _client.write("<h5>Data refresh every 5 seconds.</h5>");
    _client.write("<hr />");

    _client.write("</div></body>");
    _client.write("</html>");
}