// ProDinoIntRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Internet relay.
// Supported boards:
//		KMP ProDiNo NETBOARD V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		ProDino Internet relay, Bulgaria
// Version: 1.0.0
// Date: 21.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

// Headers for version before 1.6.6
#include <SPI.h>
#include "./Ethernet.h"
#include "KmpDinoEthernet.h"
#include "ICMPProtocol.h"
#include "Structures.h"
#include "Processing.h"
#include <avr/wdt.h>
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
//#define DEBUG

// If Post request is valid, read data from RS485.
bool _isValidPost = false;

// Initialize the Ethernet server library with the IP address and port you want to use.
EthernetServer _server(WAN_PORT);

// Ethernet client.
EthernetClient _client;

Processing _processing;

// Time to set addresses.
unsigned long _timeToSetAddresses = 0;

// Time to refresh addresses. In very rare cases V5200 wasting (changed) addresses and the network becomes unavailable. On 10 seconds addresses refresh.
const uint16_t SET_ADDRESS_TIME = 10 * 1000;

#ifdef DEBUG
char _ipBuff[17];
uint8_t _macBuff[6];
#endif

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

    // If Optical input 4 is On - reset by default settings (Relay names, Device name, IP's, user and password).
    if (GetOptoInStatus(OptoIn4))
    {
        _processing.resetSettings();
    }

    // Wait 30 seconds for help to flash new sketch. If comment flash only in ISP.
    OnStatusLed();
    delay(30000);
    OffStatusLed();

	// Read data from EEPROM.
	_processing.init();

    // Start the Ethernet connection and the server.
    Ethernet.begin((uint8_t *)MAC_ADDRESS, DeviceData.IP, DeviceData.Gateway, DeviceData.SubnetMask);
    _server.begin();

#ifdef DEBUG
    Serial.println("The server is starting.");
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
#endif

    // Start MC Watchdog timer.
    cli(); // disable all interrupts
    wdt_reset(); // reset the WDT timer    
    /*
        WDTCSR configuration:
        WDIE = 1: Interrupt Enable
        WDE = 1 :Reset Enable
        WDP3 WDP2 WDP1 WDP0
        0 0 0 0 2K (2048) cycles 16 ms
        0 0 0 1 4K (4096) cycles 32 ms
        0 0 1 0 8K (8192) cycles 64 ms
        0 0 1 1 16K (16384) cycles 0.125 s
        0 1 0 0 32K (32768) cycles 0.25 s
        0 1 0 1 64K (65536) cycles 0.5 s
        0 1 1 0 128K (131072) cycles 1.0 s
        0 1 1 1 256K (262144) cycles 2.0 s
        1 0 0 0 512K (524288) cycles 4.0 s
        1 0 0 1 1024K (1048576) cycles 8.0 s    */
    /* Start timed sequence */
    WDTCSR |= (1<<WDCE) | (1<<WDE); 
    // Set Watchdog settings - Reset Enable, cycles 8.0 s.
    WDTCSR = (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
    sei();
}

#ifdef DEBUG
void PrintDebugInfo()
{
    IPAddressToA(Ethernet.localIP(), _macBuff);
    iptoa(_macBuff, _ipBuff);
    Serial.println(_ipBuff);
    IPAddressToA(Ethernet.subnetMask(), _macBuff);
    iptoa(_macBuff, _ipBuff);
    Serial.println(_ipBuff);
    IPAddressToA(Ethernet.gatewayIP(), _macBuff);
    iptoa(_macBuff, _ipBuff);
    Serial.println(_ipBuff);

    Ethernet.macAddress(_macBuff);
    for (int i = 0; i < 6; i++)
    {
        ByteToHexStr(_macBuff[i], _ipBuff);
        Serial.print(_ipBuff);
        Serial.print(' ');
    }
    Serial.println();

    for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
        IntToChars(EthernetClass::_server_port[sock], _ipBuff);
        Serial.print(_ipBuff);
        Serial.print(' ');
    }
    Serial.println();
}
#endif

/**
 * \brief Loop void. Arduino executed second.
 * 
 * 
 * \return void
 */
void loop(){
    // Reset the WDT timer.
    wdt_reset();
    
    /*
    // Test WDT is working. Set optoIn in 1.
    if (GetOptoInStatus(OptoIn1))
    {
        // for test WDT start loop.
        while (true)
        {
            
        }
    }
    */
    
    // listen for incoming clients
    _client = _server.available();

    // If not client set IP's and process operation.
    if(!_client)
    {
        // Set addresses. If need refreshAddresses OR every 10 sec - prevent missing IP-s and MAC from W5200 and lost connection.
        if (_timeToSetAddresses < millis() || _processing.IsAddressesChanged())
        {
#ifdef DEBUG
            Serial.println("Rewrite addresses.");
            PrintDebugInfo();
#endif
            // Set IP-s. Prevent missing IP-s from W5200 and lost connection.
            Ethernet.setAddresses(DeviceData.IP, DeviceData.Gateway, DeviceData.SubnetMask, (uint8_t*)MAC_ADDRESS);
 
            // Next time after 10 seconds.            
            _timeToSetAddresses = millis() + SET_ADDRESS_TIME;
            
            _processing.SetAddressesChanged(false);
        }

        return;
    }

#ifdef DEBUG
    Serial.println("Client connected.");
#endif

    // If client connected On status led.
    OnStatusLed();

	// Process request and write response.
	_processing.processRequest(_client);

    // Close the connection.
    _client.stop();

    // If client disconnected Off status led.
    OffStatusLed();

#ifdef DEBUG
    Serial.println("Client disconnected.");
    Serial.println("---");
    Serial.println("After process request.");
    PrintDebugInfo();
#endif
}