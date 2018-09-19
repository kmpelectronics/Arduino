// TCPInput.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		- KMP ProDino MKR Zero Ethernet V1 (https://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/)
//		- KMP ProDino MKR GSM Ethernet V1  (https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/)
// Description:
//		TCP Input reads isolated input example. It works as receive a command and execute it.
//      Commands:
//        FFI - sending current inputs statuses
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 19.09.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"

// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
#define DEBUG

const int CMD_PREFFIX_LEN = 3;
const char CMD_PREFFIX[CMD_PREFFIX_LEN + 1] = "FFI";

const uint8_t BUFF_MAX = 16;

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0xC5, 0x56, 0x0E };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 198);

// Local port.
const uint16_t LOCAL_PORT = 1111;

char _resultBuffer[BUFF_MAX];

// Initialize the Ethernet server library.
// with the IP address and port you want to use.
EthernetServer _server(LOCAL_PORT);

// Client.
EthernetClient _client;

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
#ifdef DEBUG
	Serial.begin(115200);
#endif

	// Init Dino board. Set pins, start W5500.
	KMPProDinoMKRZero.init(ProDino_MKR_Zero_Ethernet);

	// Start the Ethernet connection and the server.
	Ethernet.begin(_mac, _ip);
	_server.begin();

#ifdef DEBUG
	Serial.println("The example TCPInput is started.");
	Serial.println("IPs:");
	Serial.print(Ethernet.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);
	Serial.println(Ethernet.gatewayIP());
	Serial.println(Ethernet.subnetMask());
#endif
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop()
{
	// Waiting for a client.
	_client = _server.available();

	if (!_client.connected())
	{
		return;
	}

#ifdef DEBUG
	Serial.println(">> Client connected.");
#endif

	// Read client request.
	if (ReadClientRequest())
	{
		WriteClientResponse();

		// If client disconnected switch Off status led.
		KMPProDinoMKRZero.OffStatusLed();
	}

#ifdef DEBUG
	Serial.println(">> Client disconnected.");
	Serial.println();
#endif
}

/**
 * @brief ReadClientRequest void. Read and parse client request.
 *	  First row of client request is similar to:
 *    Prefix Command
 *         | |
 *         FFI
 *    You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 *
 * @return bool Returns true if the request was expected.
 */
bool ReadClientRequest()
{
	// Loop while read all request.
	String data;
	while (ReadHttpRequestLine(&_client, &data));

	if (data.length() == 0)
	{
		return false;
	}

	// If client connected switch On status led.
	KMPProDinoMKRZero.OnStatusLed();

#ifdef DEBUG
	Serial.println(data);
#endif

	// Validate input data.
	if (data.length() < CMD_PREFFIX_LEN || !data.startsWith(CMD_PREFFIX))
	{
		KMPProDinoMKRZero.OffStatusLed();
#ifdef DEBUG
		Serial.println("Command is not valid.");
#endif

		return false;
	}

	return true;
}

/**
* @brief WriteClientResponse void. Write a client response.
*
* @return void
*/
void WriteClientResponse()
{
	// Prepare relays statuses.
	strcpy(_resultBuffer, CMD_PREFFIX);
	int relayState = 0;
	for (int j = CMD_PREFFIX_LEN; j < CMD_PREFFIX_LEN + OPTOIN_COUNT; j++)
	{
		_resultBuffer[j] = KMPProDinoMKRZero.GetOptoInState(relayState++) ? CH_1 : CH_0;
	}

	if (_client.connected())
	{
		_client.write(_resultBuffer, CMD_PREFFIX_LEN + OPTOIN_COUNT);
	}
}