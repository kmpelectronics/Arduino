// TcpTemperature.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards:
//		KMP DiNo II NETBOARD V1.0. Web: http://kmpelectronics.eu/en-us/products/dinoii.aspx
//		ProDiNo NetBoard V2.1. Web: http://kmpelectronics.eu/en-us/products/prodinoethernet.aspx
// Description:
//		Tcp server read temperature example. 
// Example link: http://www.kmpelectronics.eu/en-us/examples/dinoii/tcpoptoinputscheck.aspx
// Version: 1.2.0
// Date: 28.01.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Description: Compatibility Arduino version >= 1.6.5

#include <SPI.h>
#include <Ethernet.h>
#include "KmpDinoEthernet.h"
#include "KMPCommon.h"

// One Wire headers
#include <OneWire.h>
#include <DallasTemperature.h>

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

// Thermometer Resolution in bits. http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf page 8. 
// Bits - CONVERSION TIME. 9 - 93.75ms, 10 - 187.5ms, 11 - 375ms, 12 - 750ms. 
#define TEMPERATURE_PRECISION 10

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x14, 0x1E, 0xF1 };

// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 199);

// Local port.
const unsigned int _localPort = 1111;

// Define constants.
const char PREFIX[] = "FF";
char _commandSend = '\0';

const uint8_t BUFF_SIZE = 24;

// Buffer for receiving data.
char _packetBuffer[BUFF_SIZE];

// Initialize the Ethernet server library
// with the IP address and port you want to use.
EthernetServer _server(_localPort);

// Client.
EthernetClient _client;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(OneWirePin);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature _sensors(&oneWire);

/**
* \brief Setup void. Arduino executed first. Initialize DiNo board and prepare Ethernet connection.
*
*
* \return void
*/
void setup()
{
#ifdef DEBUG
    // Open serial communications and wait for port to open.
    Serial.begin(9600);
    while (!Serial) {
    	; // wait for serial port to connect. Needed for Leonardo only. If need debug setup() method.
    }
#endif

    // Init Dino board. Set pins, start W5200.
    DinoInit();

    // Start the Ethernet connection and the server.
    Ethernet.begin(_mac, _ip);
    _server.begin();

#ifdef DEBUG
	Serial.println("KMP Electronics Ltd. TCP temperature example.");
	Serial.println("The server is started.");
    Serial.print(Ethernet.localIP());
	Serial.print(':');
	Serial.println(_localPort);
    Serial.println(Ethernet.gatewayIP());
    Serial.println(Ethernet.subnetMask());
#endif

	// Start the One Wire library.
	_sensors.begin();

	// Set precision.
	_sensors.setResolution(TEMPERATURE_PRECISION);
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
*         FFT
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
        Serial.println(_packetBuffer);
        Serial.println(readSize);
#endif
        // If packet is not valid - read to end.
        if(requestLen != 3)
        {
            continue;
        }
        
        // Check first 2 chars is prefix.
        // If Prefix not equals first 2 chars not checked any more.
        if(strncmp(_packetBuffer, PREFIX, 2) > 0)
        {
            continue;
        }

        // Command I for opto inputs.
        if(_packetBuffer[2] == CH_T)
        {
            _commandSend = CH_T;
        }
    }

#ifdef DEBUG
    Serial.println();
    if(_commandSend == CH_T)
    {
        Serial.println("Check temperature command sent.");
    }
    else
    {
        Serial.println("Command is not valid.");
    }
#endif
}

/**
* \brief WriteClientResponse void. Write response.
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
    if(_commandSend == CH_T)
    {
		// Send the command to get temperatures.
		_sensors.requestTemperatures();

		// Grab a count of devices on the wire.
		int numberOfOneWireDevices = _sensors.getDeviceCount();
#ifdef DEBUG
		Serial.print("Number of One Wire devices: ");
		Serial.println(numberOfOneWireDevices);
#endif
		// Temp device address.
		DeviceAddress tempDeviceAddress;
		char buffer[15];

		// Add temperature data
        for (int i = 0; i < numberOfOneWireDevices; i++)
        {
			if (i > 0)
			{
				_client.write(CR_LF);
			}

			float temp;
			bool sensorAvaible = _sensors.getAddress(tempDeviceAddress, i);
			if (sensorAvaible)
			{
				_client.write(PREFIX);
				_client.write(CH_T);
				temp = _sensors.getTempC(tempDeviceAddress);
#ifdef DEBUG
				Serial.print("Temperature ");
				Serial.print(i);
				Serial.print(" sensor: ");
				Serial.println(temp);
#endif
				FloatToChars(temp, 1, buffer);
				_client.write(buffer);
			}
        }
    }
}