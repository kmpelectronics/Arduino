// UdpRelay.ino
// Company: KMP Electronics Ltd, Bulgaria.
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		ProDiNo NetBoard V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Udp server relay manipulation example.
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinoii/udprelaycontrol.aspx
// Version: 1.1.0
// Date: 21.11.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

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
byte     mac[] = { 0x80, 0x9B, 0x20, 0xE2, 0xD7, 0xD8 };
// The IP address will be dependent on your local network.
byte      ip[] = { 192, 168, 1, 199 };
// Gateway.
byte gateway[] = { 192, 168, 1, 1 };
// Sub net mask.
byte  subnet[] = { 255, 255, 255, 0 };

// Local port.
uint16_t LOCAL_PORT = 1111;

// Define constants.
char PREFIX[] = "FF";
char _commandSend = '\0';

// Buffer for receiving data.
char _packetBuffer[UDP_TX_PACKET_MAX_SIZE];

char _buffer[10];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP _udp;

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

    // Start the Ethernet connection and Udp listener.
    Ethernet.begin(mac, ip, gateway, subnet);
    _udp.begin(LOCAL_PORT);

#ifdef DEBUG
    Serial.println("The UDP listener is starting.");
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
    // Listen for incoming packets.
    int packetSize = _udp.parsePacket();
    
    if(!packetSize)
	{
        return;
	}

#ifdef DEBUG
    Serial.println("Client connected.");
    Serial.print("Receive packet of size ");
    Serial.println(packetSize);
#endif

    // If client connected On status led.
    OnStatusLed();

    ReadClientPacket();

    WriteClientResponse();

    // If client disconnected Off status led.
    OffStatusLed();

#ifdef DEBUG
    Serial.println("Client disconnected.");
    Serial.println("---");
#endif
}

/**
 * \brief ReadClientPacket void. Read and parse client request.
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
void ReadClientPacket()
{
#ifdef DEBUG
    Serial.print("From ");
    Serial.print(_udp.remoteIP());
    Serial.print(", port ");
    Serial.println(_udp.remotePort());
    Serial.println("Read client packet.");
#endif

    _commandSend = '\0';

    int requestLen = 0;
    int readSize;
    while ((readSize = _udp.read(_packetBuffer, UDP_TX_PACKET_MAX_SIZE)) > 0)
    {
        requestLen += readSize;
#ifdef DEBUG
        Serial.print(_packetBuffer);
#endif
        // Packet is not valid. Read to end.
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
                {
                    break; // Char is not (0, 1) valid.
                }
                                    
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
    Serial.println("Write client packet.");
#endif

    _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
    // Send a reply, to the IP address and port that sent us the packet we received.
    _udp.write(PREFIX);

    if(_commandSend == CH_R)
    {
        _udp.write(CH_R);
        // Add relay data.
        for (int i = 0; i < RELAY_COUNT; i++)
        {
#ifdef DEBUG
            Serial.print("Relay ");
			IntToChars(i + 1, _buffer);
			Serial.print(_buffer);
            Serial.print(" ");
            Serial.println(GetRelayStatus(i) ? W_ON : W_OFF);
#endif
            _udp.write(GetRelayStatus(i) ? CH_1 : CH_0);
        }
    }

    _udp.endPacket();
}