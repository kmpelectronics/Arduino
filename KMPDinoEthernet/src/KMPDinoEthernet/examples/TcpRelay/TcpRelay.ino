// TcpRelay.ino
// Company: KMP Electronics Ltd, Bulgaria.
// Web: https://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		- KMP ProDiNo Ethernet V2 https://kmpelectronics.eu/products/prodino-ethernet-v2/
// Description:
//		Tcp server relay manipulation example. 
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-ethernet-examples/
// Version: 1.3
// Date: 27.02.2019
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Description: Compatibilie Arduinio version >= 1.6.5

#include "KmpDinoEthernet.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
//#define DEBUG

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x2B, 0x82, 0xC0 };

// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 199);

// Local port.
const uint16_t LOCAL_PORT = 1111;

// Define constants.
const char PREFIX[] = "FF";
char _commandSend = '\0';

const uint8_t BUFF_SIZE = 24;

// Buffer for receiving data.
char _packetBuffer[BUFF_SIZE];

char _buffer[10];

// Initialize the Ethernet server library
// with the IP address and port you want to use 
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

    ReadClientRequest();

    WriteClientResponse();

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
 *	  First row of client request is similar to:
 *    Prefix Command
 *         | |
 *         FFR0011
 *            ||||
 *            3  6 relay position. First is 1, second - 2...
 *    You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 * 
 * 
 * \return void
 */
void ReadClientRequest()
{
#ifdef DEBUG
    Serial.println("Read client request.");
#endif

    _commandSend = '\0';

    int requestLen = 0;
    int readSize;
    // Read all sended data into pieces BUFF_SIZE.
    while ((readSize = _client.read((uint8_t*)_packetBuffer, BUFF_SIZE)) > 0)
    {
        requestLen += readSize;
#ifdef DEBUG
        Serial.print(_packetBuffer);
#endif
        // If packet is not valid - read to end.
        if(requestLen != 7)
        {
            continue;
        }
                
        // Check first 2 chars is prefix.
        // If Prefix not equals first 2 chars not checked any more.
        if(strncmp(_packetBuffer, PREFIX, 2) > 0)
        {
            continue; 
        }
        
        // Command R for relays.
        if(_packetBuffer[2] == CH_R)
        {
            _commandSend = CH_R;
            int statusPos = 3; // To check new statuses.
            char dataChar;
            // Read chars for On or Off for all relays.
            for(int i = 0; i < RELAY_COUNT; i++)
            {
                dataChar = _packetBuffer[statusPos];
                bool status;
                if(dataChar == CH_0 || dataChar == CH_1)
                {
                    // if c == 1 - true. if c == 0 false.
                    status = dataChar == CH_1;
                    SetRelayStatus(i, status);
                }
                else
                    break; // Char is not (0, 1) valid.
                    
                statusPos++;
            }
        }
    }

#ifdef DEBUG
    Serial.println();
    if(_commandSend == CH_R)
        Serial.println("Relay command sended.");
    else
        Serial.println("Command is not valid.");
#endif
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
    _client.write(PREFIX);
    if(_commandSend == CH_R)
    {
        _client.write(CH_R);
        // Add relay data
        for (int i = 0; i < RELAY_COUNT; i++)
        {
#ifdef DEBUG
            Serial.print("Relay ");
			IntToChars(i + 1, _buffer);
			Serial.print(_buffer);
            Serial.print(" ");
            Serial.println(GetRelayStatus(i) ? W_ON : W_OFF);
#endif
            _client.write(GetRelayStatus(i) ? CH_1 : CH_0);
        }
    }
}