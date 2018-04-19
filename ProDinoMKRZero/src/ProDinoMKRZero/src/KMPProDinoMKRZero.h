// KMPDinoZeroEth.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards: 
//		KMP ProDino Zero Eth (http://www.kmpelectronics.eu/en-us/products/prodinozero-eth.aspx)
// Description:
//		Header for KMP Dino Zero Eth board.
// Version: 1.0.0
// Date: 15.03.2017
// Authors: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#ifndef _KMPPRODINOMKRZERO_H
#define _KMPPRODINOMKRZERO_H

#include <Arduino.h>
#include <Ethernet2.h>
#include <HardwareSerial.h>

// Inputs and outputs count.
#define RELAY_COUNT  4
#define OPTOIN_COUNT 4

/**
 * @brief Grove 1 connector
 */
#define	GROVE_D0	12 // PA09
#define	GROVE_D1	11 // PA08

/**
 * @brief Relays.
 */
enum Relay {
	Relay1 = 0x00,
	Relay2 = 0x01,
	Relay3 = 0x02,
	Relay4 = 0x03
};

/**
 * @brief Inputs.
 */
enum OptoIn {
	OptoIn1 = 0x00,
	OptoIn2 = 0x01,
	OptoIn3 = 0x02,
	OptoIn4 = 0x03
};

const char TEXT_HTML[] = "text/html; charset=utf-8";
const char PRODINO_ZERO_ETH[] = "ProDino Zero";
const char URL_KMPELECTRONICS_EU_DINO_ZERO[] = "http://www.kmpelectronics.eu/en-us/products/prodinozero-eth.aspx";

class KMPProDinoMKRZeroClass
{
 public:
	void init();
	void init(bool startEthernet);

	void ResetEthernet();

	void SetRelayState(uint8_t relayNumber, bool state);
	void SetRelayState(Relay relay, bool state);
	void SetAllRelaysState(bool state);
	void SetAllRelaysOn();
	void SetAllRelaysOff();
	bool GetRelayState(uint8_t relayNumber);
	bool GetRelayState(Relay relay);

	bool GetOptoInState(uint8_t optoInNumber);
	bool GetOptoInState(OptoIn optoIn);

	void RS485Begin(unsigned long baud);
	void RS485Begin(unsigned long baud, uint16_t config);
	void RS485End();
	size_t RS485Write(uint8_t data);
	size_t RS485Write(char data);
	size_t RS485Write(const char* data);
	size_t RS485Write(String data) { return RS485Write(data.c_str()); }
	size_t RS485Write(uint8_t* data, uint8_t dataLen);
	int RS485Read();
	int RS485Read(unsigned long delayWait, uint8_t repeatTime);
};

extern KMPProDinoMKRZeroClass KMPProDinoMKRZero;

#endif