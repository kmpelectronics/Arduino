// GPRSUdpNtpTest.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards:
//		ProDino MKR GSM V1 (https://kmpelectronics.eu/products/prodino-mkr-gsm-v1/)
//		ProDino MKR GSM Ethernet V1 (https://kmpelectronics.eu/products/prodino-mkr-gsm-ethernet-v1/)
// Description:
//		With this example we can test GSM GPRS communication. 
//		When the GSM/GPRS received valid NTP packet the status led changes it status.
//		If 10 time can't get date the code restarts GPS module.
// Example link: https://kmpelectronics.eu/tutorials-examples/prodino-mkr-versions-examples/
// Version: 1.0.0
// Date: 28.02.2020
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include <KMPProDinoMKRZero.h>
#include <MKRGSM.h>

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[] = SECRET_PINNUMBER;
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

unsigned int localPort = 2390;      // local port to listen for UDP packets

// 0.bg.pool.ntp.org [185.117.82.70] 8ms
// 2.bg.pool.ntp.org [185.117.82.66] 7ms
// 3.bg.pool.ntp.org [195.85.215.215] 8ms
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServer(185, 117, 82, 66);

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// initialize the library instance
GPRS gprs;
GSM gsmAccess;

// A UDP instance to let us send and receive packets over UDP
GSMUDP Udp;

bool _ledStatus;
bool _resetGsm = false;
unsigned long _testNo = 0;
unsigned long _noReceive = 0;
unsigned long _responseTimeout;
unsigned long _restartGsmCount = 0;
unsigned long _restartGprsCount = 0;

void setup()
{
	delay(5000);

	Serial.begin(115200);
	Serial.println("Starting GPRSUdpNtpTest example...");

	KMPProDinoMKRZero.init(ProDino_MKR_GSM);
	//KMPProDinoMKRZero.init(ProDino_MKR_GSM_Ethernet);

	// Enable GSM modem debug
	MODEM.debug();
}

void loop()
{
	if (!isConnected())
	{
		return;
	}
	
	// Waiting 10 sec. before asking for the time again
	delay(10000);

	// Sending an NTP packet to a time server
	sendNTPpacket(timeServer); 
	
	// Set waiting for packet time out
	_responseTimeout = millis() + 1500;

	// Waiting for a time packet. It test for new packet every 100 ms with 1.5 sec timeout.
	while (true)
	{
		if (Udp.parsePacket())
		{
			break;
		}

		// If timeout appears print a message
		if (millis() > _responseTimeout)
		{
			Serial.print(++_noReceive);
			Serial.println(" packet not received! Trying again...");

			printResets();

			_ledStatus = false;
			KMPProDinoMKRZero.OffStatusLed();

			// If the device can't get data 10 consequence times restart GSM
			if (_noReceive > 10)
			{
				_resetGsm = true;
			}

			return;
		}

		delay(100);
	}

	// Print time information
	_noReceive = 0;
	Serial.print("Packet No: ");
	Serial.print(++_testNo);
	Serial.println(" is received");

	// Change status led. On->Off or Off->On.
	_ledStatus = !_ledStatus;
	KMPProDinoMKRZero.SetStatusLed(_ledStatus);

	// We've received a packet, read the data from it
	Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

	//the timestamp starts at byte 40 of the received packet and is four bytes,
	// or two words, long. First, extract the two words:

	unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
	unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
	// combine the four bytes (two words) into a long integer
	// this is NTP time (seconds since Jan 1 1900):
	unsigned long secsSince1900 = highWord << 16 | lowWord;
	Serial.print("Seconds since Jan 1 1900 = ");
	Serial.println(secsSince1900);

	// now convert NTP time into everyday time:
	Serial.print("Unix time = ");
	// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	const unsigned long seventyYears = 2208988800UL;
	// subtract seventy years:
	unsigned long epoch = secsSince1900 - seventyYears;
	// print Unix time:
	Serial.println(epoch);

	// print the hour, minute and second:
	Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
	Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
	Serial.print(':');
	if (((epoch % 3600) / 60) < 10) {
		// In the first 10 minutes of each hour, we'll want a leading '0'
		Serial.print('0');
	}
	Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
	Serial.print(':');
	if ((epoch % 60) < 10) {
		// In the first 10 seconds of each minute, we'll want a leading '0'
		Serial.print('0');
	}
	Serial.println(epoch % 60); // print the second

	printResets();
}

void printResets()
{
	Serial.print("Restarts: GSM: ");
	Serial.print(_restartGsmCount);
	Serial.print(", GPRS: ");
	Serial.println(_restartGprsCount);
}

bool isConnected()
{
	if (gsmAccess.status() != GSM_READY || _resetGsm)
	{
		_resetGsm = true;
		Serial.println("Starting GSM...");

		if (gsmAccess.begin(PINNUMBER) != GSM_READY)
		{
			Serial.println("GSM can not start!");
			delay(1000);
			return false;
		}

		Serial.println("GSM is started");
		++_restartGsmCount;
	}

	if (gprs.status() != GPRS_READY || _resetGsm)
	{
		// After starting the modem with GSM.begin()
		// attach the shield to the GPRS network with the APN, login and password
		Serial.println("Connecting GPRS network with the APN");

		if (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) != GPRS_READY)
		{
			Serial.println("GPRS_APN can not connect!");

			return false;
		}
		Serial.println("GPRS_APN is connected");

		Serial.println("Start connection to the server");
		Udp.begin(localPort);

		_resetGsm = false;
		++_restartGprsCount;
	}

	return true;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address)
{
	//Serial.println("1");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	//Serial.println("2");
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	//Serial.println("3");

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	//Serial.println("4");
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	//Serial.println("5");
	Udp.endPacket();
	//Serial.println("6");
}
