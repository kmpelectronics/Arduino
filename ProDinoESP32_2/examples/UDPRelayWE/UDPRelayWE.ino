// UDPRelay.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		ProDino ESP32 V1 https://kmpelectronics.eu/products/prodino-esp32-v1/
//		ProDino ESP32 Ethernet V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/
//		ProDino ESP32 GSM V1 https://kmpelectronics.eu/products/prodino-esp32-gsm-v1/
//		ProDino ESP32 LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-lora-v1/
//		ProDino ESP32 LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-lora-rfm-v1/
//		ProDino ESP32 Ethernet GSM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
//		ProDino ESP32 Ethernet LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-v1/
//		ProDino ESP32 Ethernet LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-rfm-v1/
// Description:
//		UDP Relay management example. It works as receive a command and execute it.
//      Commands:
//        FFR - sending current relays statuses
//        FFRxxxx - set relay state. x should be 0 - for Off and 1 - for On. If you send different char x, y, #, 2 and etc. the relay doesn't change its status.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 18.03.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WiFiUdp.h>

// if this define is uncommented example supports both Ethernet and WiFi
#define ETH_TEST

const int CMD_PREFFIX_LEN = 3;
const char CMD_PREFFIX[CMD_PREFFIX_LEN + 1] = "FFR";

const uint8_t BUFF_MAX = 16;
char _resultBuffer[BUFF_MAX];

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x44, 0x4E, 0x7D };
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 197);

// Local port.
const uint16_t LOCAL_PORT = 1111;
// An EthernetUDP instance to let us send and receive packets over UDP.
EthernetUDP _ethUdp;
WiFiUDP _wiFiUdp;

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example UDPRelay is starting...");

	//KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
#ifdef ETH_TEST
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
#endif // ETH_TEST

	KMPProDinoESP32.setStatusLed(blue);

	// Connect to WiFi network
	WiFi.begin(SSID, SSID_PASSWORD);
	Serial.print("\n\r \n\rWiFi is connecting");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	Serial.print("WiFi IP: ");
	Serial.print(WiFi.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);
	
	_wiFiUdp.begin(LOCAL_PORT);

#ifdef ETH_TEST
	// Start the Ethernet connection and the server.
	//Ethernet.begin(_mac, _ip);
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}
	_ethUdp.begin(LOCAL_PORT);

	Serial.println("Ethernet IP:");
	Serial.print(Ethernet.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);
#endif // ETH_TEST

	KMPProDinoESP32.offStatusLed();

#ifdef DEBUG
	Serial.println("The example UDPRelay is started.");
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
	KMPProDinoESP32.processStatusLed(green, 1000);

	UDP * udp = NULL;
	// Waiting for a client.
#ifdef ETH_TEST
	if (_ethUdp.parsePacket() != 0)
	{
		udp = &_ethUdp;
	}
#endif

	if (_wiFiUdp.parsePacket() != 0)
	{
		udp = &_wiFiUdp;
	}

	if (udp == NULL)
	{
		return;
	}

	Serial.println(">> Client connected.");

	KMPProDinoESP32.setStatusLed(yellow);
	// Read client request.
	if (ReadClientRequest(udp))
	{
		WriteClientResponse(udp);
	}
	Serial.println(">> Client disconnected.");
	Serial.println();

	KMPProDinoESP32.offStatusLed();
}

/**
 * @brief This method reads and parses the client request.
 *	  First row of the client request is similar to:
 *    Prefix Command
 *         | |
 *         FFR0011
 *            ||||
 *            3  6 relay position. First is 1, second - 2...
 *    You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
 *
 * @return bool Returns true if the request is expected.
 */
bool ReadClientRequest(Stream *udp)
{
	// Loop while read all request.
	String data;
	while (ReadHttpRequestLine(udp, &data));

	if (data.length() == 0)
	{
		return false;
	}

	Serial.println(data);

	// Validate input data.
	if (data.length() < CMD_PREFFIX_LEN || !data.startsWith(CMD_PREFFIX))
	{
		Serial.println("Command is not valid.");
		return false;
	}

	// Command set relay status FFRxxxx
	if (data.length() == CMD_PREFFIX_LEN + RELAY_COUNT)
	{
		int relayNum = 0;
		for (int i = CMD_PREFFIX_LEN; i < CMD_PREFFIX_LEN + RELAY_COUNT; i++)
		{
			// Set relay status if only chars are 0 or 1.
			if (data[i] == CH_0 || data[i] == CH_1)
			{
				KMPProDinoESP32.setRelayState(relayNum, data[i] == CH_1);
			}

			++relayNum;
		}
	}

	return true;
}

/**
* @brief Writing the client response.
*
* @return void
*/
void WriteClientResponse(UDP *udp)
{
	// Prepare relays statuses.
	strcpy(_resultBuffer, CMD_PREFFIX);
	int relayState = 0;
	for (int j = CMD_PREFFIX_LEN; j < CMD_PREFFIX_LEN + RELAY_COUNT; j++)
	{
		_resultBuffer[j] = KMPProDinoESP32.getRelayState(relayState++) ? CH_1 : CH_0;
	}

	// Send output packet
	udp->beginPacket(udp->remoteIP(), udp->remotePort());
	udp->write((uint8_t*)_resultBuffer, CMD_PREFFIX_LEN + RELAY_COUNT);
	udp->endPacket();
}