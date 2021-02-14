// WebSocketE1.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		KMP ProDino ESP32 Ethernet V1 (https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet GSM V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet LoRa V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
//		KMP ProDino ESP32 Ethernet LoRa RFM V1 (https://kmpelectronics.eu/products/prodino-esp32-GSM-ethernet-v1/)
// Description:
//		With this example you can test connection to the web socket.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-esp32-versions-examples/
// Version: 1.0.0
// Date: 14.02.2021
// Author: Plamen Kovandjiev <contact@kmpelectronics.eu>
// --------------------------------------------------------------------------------
// Prerequisites:
//	Before start this example you need to install:
//		Install library: Sketch\Include library\Menage Libraries... find ... and click Install.
//         - WebSockets2_Generic

#define WEBSOCKETS_USE_ETHERNET           true
#define _WS_CONFIG_NO_SSL

#ifdef  ESP32
#define BOARD_TYPE      "ESP32"
#else
#error This code is intended to run only on the ESP32 boards ! Please check your Tools->Board setting.
#endif

#ifndef BOARD_NAME
#define BOARD_NAME    BOARD_TYPE
#endif

#define DEBUG_WEBSOCKETS_PORT     Serial

#include <WebSockets2_Generic.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

// Enter a MAC address and IP address for your controller below.
byte _mac[] = { 0x00, 0x08, 0xDC, 0x72, 0xE7, 0x40 };

using namespace websockets2_generic;

WebsocketsClient client;

void onEventsCallback(WebsocketsEvent event, String data)
{
	if (event == WebsocketsEvent::ConnectionOpened)
	{
		Serial.println("Connection Opened");
	}
	else if (event == WebsocketsEvent::ConnectionClosed)
	{
		Serial.println("Connection Closed");
	}
	else if (event == WebsocketsEvent::GotPing)
	{
		Serial.println("Got a Ping!");
	}
	else if (event == WebsocketsEvent::GotPong)
	{
		Serial.println("Got a Pong!");
	}
}

void setup()
{
	delay(5000);
	Serial.begin(115200);

	// Init Dino board. Set pins, start W5500.
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
	KMPProDinoESP32.setStatusLed(blue);

	// Start the Ethernet connection and the server.
	// Getting IP from DHCP
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}

	Serial.println("Ethernet IP: ");
	Serial.print(Ethernet.localIP());

	KMPProDinoESP32.offStatusLed();

	Serial.println("\nStarting ESP32-Client on " + String(ARDUINO_BOARD));
	Serial.println(WEBSOCKETS2_GENERIC_VERSION);

	// run callback when messages are received
	client.onMessage([&](WebsocketsMessage message)
	{
		Serial.print("Got Message: ");
		Serial.println(message.data());
	});

	// run callback when events are occuring
	client.onEvent(onEventsCallback);

	// try to connect to Websockets server
	bool connected = client.connect("ws://echo.websocket.org");

	if (connected)
	{
		Serial.println("Connected!");
		String WS_msg = String("Hello to Server from ") + BOARD_NAME;
		client.send(WS_msg);
	}
	else
	{
		Serial.println("Not Connected!");
	}
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

	// let the websockets client check for incoming messages
	if (client.available())
	{
		client.poll();
	}
}