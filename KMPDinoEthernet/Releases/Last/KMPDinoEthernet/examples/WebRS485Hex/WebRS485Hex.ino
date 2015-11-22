// WebRS485Hex.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		ProDiNo NetBoard V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Web server RS485 HEX (send and receive HEX data) test example.
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinoii/webrs485hextest.aspx
// Version: 1.1.0
// Date: 03.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Warning! RS485 don't work Arduino version 1.5.6 and next. This is version optimized for SAM microprocessors.
//          Please use the version 1.5.5 or 1.0.6 or latest (this versions only for ARM microprocessors).

// Headers for version before 1.6.6
#include <SPI.h>
#include "./Ethernet.h"
#include "KmpDinoEthernet.h"
#include "KMPCommon.h"
// Headers for version >= 1.6.6
/*
#include <Base64.h>
#include <DallasTemperature.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <ICMPProtocol.h>
#include <KMPCommon.h>
#include <KmpDinoEthernet.h>
#include <OneWire.h>
#include <util.h>
#include <w5200.h>
*/

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte     _mac[] = { 0x80, 0x9B, 0x20, 0x83, 0x37, 0x4A };
// The IP address will be dependent on your local network.
byte      _ip[] = { 192, 168, 1, 199};                  
// Gateway.
byte _gateway[] = { 192, 168, 1, 1 };
// Sub net mask
byte  _subnet[] = { 255, 255, 255, 0 };

// Buffer to Hex bytes.
char _hex[3];

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// If Post request is valid, read data from RS485.
bool _isValidPost = false;

const char _kmpURL[] = "http://kmpelectronics.eu/";

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
    // Start RS485 with boud 19200 and 8N1.
    RS485Begin(19200);

    // Start the Ethernet connection and the server.
    Ethernet.begin(_mac, _ip, _gateway, _subnet);
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
    // listen for incoming clients
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

    // Read client request to prevent overload input buffer.
    if(ReadClientRequest())
    {
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
    Serial.println("#Client disconnected.");
    Serial.println("---");
#endif
}

/**
 * \brief  ReadClientRequest void. Read client request.
 *         Client request is similar to:
 *          GET / HTTP/1.1
 *          |    |
 *          1    6
 *          POST / HTTP/1.1
 *          |     |
 *          1     7
 *          Other post data...
 *          Needed data.
 *              data=Test&btn=Transmit
 *                  |   |
 *                  6   End symbol.
 *         You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 * 
 * 
 * \return bool Returns true if the request was expected.
 */
bool ReadClientRequest()
{
#ifdef DEBUG
    Serial.println("#Read client request.");
#endif
    _isValidPost = false;
    int requestLen = 0;
    int lineLen = 0;
    bool result = false;
    bool isGet = false;
    bool readData = false;
    char prevChar = '/0';
    bool evenByte = false;
    
    // Loop while read all request.
    while (_client.available()) 
    {
        requestLen++;
        char c = _client.read();

        // Calculate line len only for chars, exclude CR LF.
        if(c != CH_CR && c != CH_LF)
        {
            lineLen++;
        }            

#ifdef DEBUG
        Serial.write(c);
#endif
        // Check request is GET or POST.
        if(requestLen == 1)
        {
            isGet = c == 'G';
        }            
        
        // Valid GET. Position 6 is ' '.
        if(isGet && requestLen == 6)
        {
            result = c == CH_SPACE;
        }            
        
        // Find end line, after this start Post data.
        if(prevChar == CH_CR && c == CH_LF)
        {
            // Empty line after this post data.
            if(!isGet && lineLen == 0)
            {
                readData = true;
                result = true;
            }
            lineLen = 0;
        }

        // End valid data to read.
        if(readData && c == '&')
            readData = false;

        // Transmit data to RS485.
        if(readData && lineLen > 5)
        {
            // Write byte to RS485.
            if (evenByte)
            {
                int hex = HexToByte(prevChar, c);

                // if hex is valid - send.
                if (hex != -1)
                {
                    RS485Write((uint8_t)hex);
                }
            }
            
            _isValidPost = true;
            evenByte = !evenByte;
        }

        prevChar = c;
    }

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
    Serial.println();
    Serial.println("#Write client response.");
#endif

    if (!_client.connected())
        return;

    // Response write.
    // Send a standard http header.
    _client.writeln("HTTP/1.1 200 OK");
    _client.writeln("Content-Type: text/html");
    _client.writeln("");

    // Add web page HTML.
    _client.write("<html>");

    // Header
    _client.write("<head>");

    // Add favicon.ico
    _client.write("<link rel='shortcut icon' href='");
    _client.write(_kmpURL);
    _client.write("Portals/0/Projects/Arduino/Lib/icons/WebRelay.ico' type='image/x-icon'>");

    _client.write("<title>KMP Electronics Ltd. DiNo Ethernet - Web RS485 Hex Test</title></head>");

    // Body
    _client.write("<body><div style='text-align: center'>");
    _client.write("<br><hr />");
    _client.write("<h1 style='color: #0066FF;'>DiNo Ethernet - Web RS485 Hex Test example</h1>");
    _client.write("<hr /><br><br>");

    // Table
    _client.write("<form method='post'><table border='1' width='400' cellpadding='5' cellspacing='0'"); 
    _client.write("align='center' style='text-align: center; font-size: large; font-family: Arial, Helvetica, sans-serif;'>");
    
    // Add table header.
    _client.write("<thead><tr><th width='80%'>Data</th><th>Action</th></tr></thead>");

    // Add table rows.
    // Row 1.
    _client.write("<tbody><tr><td><input type='text' name='data' style='width: 100%'></td>");
    _client.write("<td><input type='submit' name='btn' value='Transmit'/ ></td></tr>");
    // Row 2.
    _client.write("<tr><td>");

    if(_isValidPost)
    {
#ifdef DEBUG
        Serial.println("Read data from RS485.");
        int readBytes = 0;
#endif
        // Read RS485.
        int i;
    
        // if i = -1 not data to read.
        while((i = RS485Read()) > -1)
        {
            ByteToHexStr(i, _hex);
#ifdef DEBUG
            Serial.print(_hex);
            //Serial.print(' ');
            readBytes++;
#endif
            // Convert unsigned char to hex.
            _client.write(_hex);
        }
#ifdef DEBUG
        Serial.println();
        Serial.print("Bytes read: ");
        Serial.println(readBytes);
#endif
    }

    _client.write("</td><td>Received</td></tr></tbody>");

    _client.writeln("</table></form>");
    _client.writeln("<br><br><hr />");
    _client.write("<h1><a href='http://kmpelectronics.eu/' target='_blank'>Visit KMPElectronics.eu!</a></h1>");
    _client.write("<h3><a href='http://www.kmpelectronics.eu/en-us/products/prodinoethernet.aspx' target='_blank'>Information about ProDino Ethernet board</a></h3>");
    _client.writeln("<hr />");

    _client.writeln("</div></body>");
    _client.writeln("</html>");
}