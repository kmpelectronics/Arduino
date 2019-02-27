// WebTemperature.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		- KMP ProDiNo Ethernet V2 https://kmpelectronics.eu/products/prodino-ethernet-v2/
// Description:
//		Web server check One Ware temperature sensors DS18B20 or DS18S20. Sensors (one or many) must connect to board. 
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-ethernet-examples/
// Version: 1.3
// Date: 29.02.2019
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Description: Compatibilie Arduinio version >= 1.6.5

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
#define TEMPERATURE_PRECISION 9

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x41, 0xBA, 0x71 };

// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 199);

// Buffer to Hex bytes.
char _buffer[10];

// Local port.
// Port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;

// Define text.
char white[] = "white";
char blue[] = "blue";
char green[] = "green";
char red[] = "red";
char NA[] = "N/A";
char degreeSymbol[] = "&deg;";
const char _kmpURL[] = "https://kmpelectronics.eu/";

// Initialize the Ethernet server library.
// with the IP address and port you want to use. 
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire _oneWire(OneWirePin);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature _sensors(&_oneWire);

// Number of one wire temperature devices found.
int _oneWireDeviceCount; 

// Temp device address.
DeviceAddress _tempDeviceAddress; 

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
    
    // Start the One Wire library.
    _sensors.begin();

    // Set precision.
    _sensors.setResolution(TEMPERATURE_PRECISION);

    // Select available connected to board One Wire devices.
    GethOneWireDevices();
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
    ReadClientRequest();

    // Write client response - html page.
    WriteClientResponse();

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
 * \brief ReadClientRequest void. Read client request. Not need any operation.
 *        First row of client request is similar to:
 *        GET / HTTP/1.1
 *
 *        You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 *        or read debug information from Serial Monitor (Tools > Serial Monitor).
 *
 * \return void
 */
void ReadClientRequest()
{
#ifdef DEBUG
    Serial.println("Read client request.");
#endif

    // Loop while read all request.
    while (_client.available()) 
    {
        char c = _client.read();

#ifdef DEBUG
        Serial.write(c);
#endif
    }

#ifdef DEBUG
    Serial.println();
    Serial.println("End client request.");
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
        return;

    // Send the command to get temperatures.
    _sensors.requestTemperatures(); 

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

    // Add a meta refresh tag, so the browser pulls again every 10 seconds.
    _client.write("<meta http-equiv='refresh' content='10'>");
    _client.write("<title>KMP Electronics Ltd. DiNo Ethernet - Web One Wire</title></head>");

    // Body
    _client.write("<body><div style='text-align: center'><br><hr />");
    _client.write("<h1 style='color: #0066FF;'>DiNo Ethernet - Web One Wire Temperature example</h1><hr /><br><br>");

    // Table
    _client.write("<table border='1' width='300' cellpadding='5' cellspacing='0'"); 
    _client.write("align='center' style='text-align: center; font-size: large; font-family: Arial, Helvetica, sans-serif;'>");

    // Add table header.
    _client.write("<thead><tr><th></th><th>C</th><th>F</th></tr></thead>");

    // Add table rows
    _client.write("<tbody>");
    for (int i = 0; i < _oneWireDeviceCount; i++)
    {
        // Default color.
        char* cellColor = white;
        float temp;
        bool sensorAvaible = _sensors.getAddress(_tempDeviceAddress, i);
        if(sensorAvaible)
        {
            // Get temperature in Celsius.
            temp = _sensors.getTempC(_tempDeviceAddress);

#ifdef DEBUG
            Serial.print("Device ");
            Serial.print(i);
            Serial.print(" with address: ");
            PrintAddress(_tempDeviceAddress);
            Serial.print("Temperature in C: ");
            Serial.println(temp);
#endif

            // Select cell background.
            if(0.0 >= temp)
            {
                cellColor = blue;
            }
            else if (22.0 >= temp)
            {
                cellColor = green;
            }                
            else
            {
                cellColor = red;
            }                
        } 

        // Row i, cell 1
        _client.write("<tr><td>Sensor ");
        _client.write(IntToChar(i + 1));
        _client.write("</td>");

        // Add cell i,2
        AddTemperatureCell(sensorAvaible, temp, cellColor);

        // Add cell i,3
        AddTemperatureCell(sensorAvaible, _sensors.toFahrenheit(temp), cellColor);

        // End row.
        _client.write("</tr>");
    }

    _client.write("<tbody></table>");
    _client.write("<br><br><hr />");
    _client.write("<h1><a href='https://kmpelectronics.eu/' target='_blank'>Visit KMPElectronics.eu!</a></h1>");
    _client.write("<h3><a href='http://www.kmpelectronics.eu/en-us/products/prodinoethernet.aspx' target='_blank'>Information about ProDino Ethernet board</a></h3>");
    _client.write("<h5>Data refresh every 10 seconds.</h5>");
    _client.write("<hr />");

    _client.write("</div></body>");
    _client.write("</html>");
}

/**
 * \brief Add cell in table, include temperature data.
 * 
 * \param sensorAvaible Sensor available. If true - available, else not available.
 * \param temperature Temperature.
 * \param cellColor Cell background color in text.
 * 
 * \return void
 */
void AddTemperatureCell(bool sensorAvaible, double temperature, char* cellColor)
{
    _client.write("<td bgcolor='");
    _client.write(cellColor);
    _client.write("'>");
    if(sensorAvaible)
    {
        FloatToChars(temperature, 1, _buffer);
        _client.write(_buffer);
        _client.write(degreeSymbol);
    }
    else
        _client.write(NA);
    _client.write("</td>");
}

/**
 * \brief Get all available One Wire devices.
 * 
 * 
 * \return void
 */
void GethOneWireDevices()
{
    // Grab a count of devices on the wire.
    _oneWireDeviceCount = _sensors.getDeviceCount();

#ifdef DEBUG
    Serial.print("Number of One Wire devices: ");
    Serial.println(_oneWireDeviceCount);

    // Loop through each device, print out address
    for(int i = 0; i < _oneWireDeviceCount; i++)
    {
        // Search the wire for address
        if(_sensors.getAddress(_tempDeviceAddress, i))
        {
            Serial.print("Device ");
            Serial.print(i);
            Serial.print(" with address: ");
            PrintAddress(_tempDeviceAddress);
        }
        else
        {
            Serial.print("Found ghost device at ");
            Serial.print(i);
            Serial.print(" but could not detect address. Check power and cabling");
        }
    }
#endif
}

#ifdef DEBUG
/**
 * \brief Print device address to Serial.
 * 
 * \param deviceAddress Device address.
 * 
 * \return void
 */
void PrintAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        ByteToHexStr(deviceAddress[i], _buffer);
        Serial.print(_buffer);
    }
    Serial.println();
}
#endif