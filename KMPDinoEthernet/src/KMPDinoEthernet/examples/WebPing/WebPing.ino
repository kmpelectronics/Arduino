// WebPing.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		KMP ProDiNo NETBOARD V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Web server ping IP address example.
// Example link: http://kmpelectronics.eu/en-us/examples/dinoii/webpingipaddress.aspx
// Version: 1.1.0
// Date: 21.11.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <SPI.h>
#include "./Ethernet.h"
#include "KmpDinoEthernet.h"
#include "KMPCommon.h"
#include "ICMPProtocol.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
//#define DEBUG
// If have ping range of IP's uncomment.
//#define PING_RANGE

// Enter a MAC address and IP address for your controller below.
byte     _mac[] = { 0x80, 0x9B, 0x20, 0x52, 0x95, 0x28 };
// The IP address will be dependent on your local network.
byte      _ip[] = { 192, 168, 0, 199 };
// Gateway.
byte _gateway[] = { 192, 168, 0, 1 };
// Sub net mask
byte  _subnet[] = { 255, 255, 255, 0 };

// Local port.
// Port 80 is default for HTTP
const unsigned int LOCAL_PORT = 80;

// If Post request is valid, read data from RS485.
bool _isValidPost = false;

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Ethernet client.
EthernetClient _client;

// Buffer to read IP from post request. Format XXX.XXX.XXX.XXX\0 - 16
const uint8_t MAX_IPSTR_LEN = 16;
char _ipStrByffer[MAX_IPSTR_LEN];
uint8_t _ipBuff[4];
char _buffer[10];

// Create instance ICMPProtocol class.
ICMPProtocol _icmp;

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

    // Initialize Dino board. Set pins, start W5200.
    DinoInit();
    _ipStrByffer[0] = '\0';

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

    // If not client - exit.
    if(!_client)
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
 * \brief ReadClientRequest void. Read client request.
 * 	Client request is similar to:
 * 		GET / HTTP/1.1
 *      |    |
 *      1    6
 *      POST / HTTP/1.1
 *      |     |
 *      1     7
 *      Other post data...
 *      data=192.168.0.15&btn=Ping
 * 
 *  You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 *    or read debug information from Serial Monitor (Tools > Serial Monitor).
 *
 * 
 * \return Returns true if the request was expected.
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
    uint8_t ipStrPos = 0;
    uint8_t dataCount = 0;

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
        
        // Valid GET. Position 6 is ' '. Skip GET /favicons.ico HTTP/1.1
        if(isGet && requestLen == 6)
            result = c == CH_SPACE;
        
        // Find end line, after this start Post data.
        if(prevChar == CH_CR && c == CH_LF)
        {
            // Empty line after this post data.
            if(!isGet && lineLen == 0)
            {
                //readData = true;
                result = true;
            }
            lineLen = 0;
        }

        // Start read data from POST. Key1=Value1&Key2=Value2&...
        if(!isGet && result == true && prevChar == '=')
        {
            readData = true;
            _isValidPost = true;
            dataCount++;
        }

        // End valid data to read. End value.
        if(readData && c == '&')
            readData = false;

        // Read first data.
        if(readData && dataCount == 1 && ipStrPos < MAX_IPSTR_LEN)
        {
            _ipStrByffer[ipStrPos++] = c;
            _ipStrByffer[ipStrPos] = '\0';
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
    _client.write("<head><title>KMP Electronics Ltd. ProDiNo Ethernet - Web ping IP address</title>");

    // Body
    _client.write("<body><div style='text-align: center'>");
    _client.write("<br><hr />");
    _client.write("<h1 style='color: #0066FF;'>ProDiNo Ethernet - Ping Test example</h1>");
    _client.write("<hr /><br><br>");

    // Table
    _client.write("<table border='1' width='500' cellpadding='5' cellspacing='0'"); 
    _client.write("align='center' style='text-align: center; font-size: large; font-family: Arial, Helvetica, sans-serif;'>");

    // Add table header.
    _client.write("<thead><tr><th width='80%'>IP</th><th>Action</th></tr></thead>");

    // Add table rows.
    // Row 1.
    _client.write("<tbody><tr><form method='post'><td><input type='text' name='data' value='");
    _client.write(_ipStrByffer);
    _client.write("' style='width: 100%'></td>");
    _client.write("<td><input type='submit' name='btn' value='Ping'/ ></td></form></tr>");
    // Row 2.
    _client.write("<td colspan='2' style='text-align: left'>Result:<br />");
	_client.write("<textarea name='res' rows='5' style='width: 100%; margin-top: 10px;'>");

    if(_isValidPost)
    {
#ifdef PING_RANGE
        // If ping range of IP's - uncomment.
		for(uint8_t i = 0; i < 10/*255*/; i++)
		{
            _ipBuff[0] = 192;
            _ipBuff[1] = 168;
            _ipBuff[2] = 0;
            _ipBuff[3] = i;
			PingIP(_ipBuff);
		}
#else
        
        if(!atoip(_ipStrByffer, _ipBuff))
        {
#ifdef DEBUG
            Serial.print("IP is invalid.");
#endif
            _client.write("IP is invalid.");
        }
        else
            PingIP(_ipBuff);
#endif
    }

    _client.write("</textarea></tr></tbody>");

    _client.write("</table>");
    _client.write("<br><br><hr />");
    _client.write("<h1><a href='http://kmpelectronics.eu/' target='_blank'>Visit KMPElectronics.eu!</a></h1>");
    _client.write("<h3><a href='http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx' target='_blank'>Information about ProDiNo Ethernet board</a></h3>");
    _client.write("<hr />");

    _client.write("</div></body>");
    _client.write("</html>");
}

/**
 * \brief Ping IP address.
 * 
 * \param address IP address to send echo ping.
 * 
 * \return void
 */
void PingIP(uint8_t* address)
{
#ifdef DEBUG
    Serial.print("Ping IP address: ");
    Serial.println(address);
#endif
    // Ping id = 0x0400 is for windows.
    ICMPEchoReply echoReply = _icmp.Ping(address, 0x0400, 1);

    if (echoReply.status == SUCCESS)
    {
#ifdef DEBUG
        Serial.print("Reply ");
        Serial.print(echoReply.seq);
        Serial.print(" from ");
        Serial.print(echoReply.addr);
        Serial.print(" bytes=");
        Serial.print(PAYLOAD_DATA_SIZE);
        Serial.print(" time=");
        IntToChars(echoReply.time, _buffer);
        Serial.print(_buffer);
        Serial.print("ms TTL=");
        Serial.println(echoReply.ttl);
#endif
        _client.write("Reply ");
        IntToChars(echoReply.seq, _buffer);
        _client.write(_buffer);
        _client.write(" from ");
        iptoa(echoReply.addr, _buffer);
        _client.write(_buffer);
        _client.write(" bytes=");
        IntToChars(PAYLOAD_DATA_SIZE, _buffer);
        _client.write(_buffer);
        _client.write(" time=");
        IntToChars(echoReply.time, _buffer);
        _client.write(_buffer);
        _client.write("ms");
		_client.write(" TTL=");
        IntToChars(echoReply.ttl, _buffer);
        _client.write(_buffer);
    }
    else
    {
#ifdef DEBUG
        Serial.print("Echo request failed: ");
        Serial.println(echoReply.status);
#endif
        _client.write("IP: ");
        iptoa(address, _buffer);
        _client.write(_buffer);
        _client.write(" Failed: ");

        char* msg;
        switch (echoReply.status)
        {
            case SEND_TIMEOUT:
                msg = "SEND_TIMEOUT";
        	break;
            case NO_RESPONSE:
                msg = "NO_RESPONSE";
            break;
            case BAD_RESPONSE:
                msg = "BAD_RESPONSE";
            break;
            case NO_FREE_SOCKET:
                msg = "NO_FREE_SOCKET";
            break;
            default:
                msg = "Unexpected";
        }

        _client.write(msg);
    }
	_client.writeln("");
}

/// <summary>
/// Convert string to IPAddress.
/// </summary>
/// <param name="str">Char array to convert.</param>
/// <returns>if success result is IP or INADDR_NONE.</returns>
IPAddress atoip(char* str)
{
    uint8_t ip[4];
    uint8_t pos = 0;

    // A pointer to the next digit to process.
    char* start = str;

    for (int i = 0; i < 4; i++) 
	{
        // The digit being processed.
        char c;
        // The value of this byte.
        uint16_t n = 0;

        while (1) 
        {
            c = *start;
            start++;
            if (c >= '0' && c <= '9') 
			{
                n *= 10;
                n += c - '0';
            }
            // We insist on stopping at "." if we are still parsing
            // the first, second, or third numbers. If we have reached
            // the end of the numbers, we will allow any character.
            else
            {
                if ((i < 3 && c == '.') || i == 3) 
                    break;
                else 
                    return INADDR_NONE;
            }
        }

        if (n >= 256)
            return INADDR_NONE;

        ip[pos++] = n;
    }
    
    IPAddress result(ip);

    return result;
}