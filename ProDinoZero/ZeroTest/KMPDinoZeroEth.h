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

#ifndef _KMPDINOZEROETH_H
#define _KMPDINOZEROETH_H

#include <Arduino.h>
#include <Ethernet2.h>
#include <HardwareSerial.h>

// Inputs and outputs count.
#define RELAY_COUNT  4
#define OPTOIN_COUNT 4

/**
 * @brief Grove 1 connector
 * @inf   The pin 20 is same for GROVE1_D1 and UEXT_6.
 *		  The pin 21 is same for GROVE1_D2 and UEXT_5.
 */
#define	GROVE1_D1	0x14 // 20 SDA PA22
#define	GROVE1_D2	0x15 // 21 SCL PA23

/**
 * @brief Grove 2 connector
 */
#define	GROVE2_A1	0x0E // 14 A0 PA02
#define	GROVE2_A2	0x0F // 15 A1 PB08

 /**
  * @brief UEXT connector
  */
#define	UEXT_3  0x01 // 1 <- TX PA10          
#define	UEXT_4  0x00 // 0 -> RX PA11
#define	UEXT_5  0x15 // 21 SCL PA23
#define	UEXT_6  0x14 // 20 SDA PA22
#define	UEXT_7  0x16 // 22 SPI1 PA12 MISO
#define	UEXT_8  0x17 // 23 SPI4 PB10 MOSI
#define	UEXT_9  0x18 // 24 SPI3 PB11 SCK
#define	UEXT_10 0x03 // 3 PA09 CS

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
const char PRODINO_ZERO_ETH[] = "ProDino Zero Eth";
const char URL_KMPELECTRONICS_EU_DINO_ZERO[] = "http://www.kmpelectronics.eu/en-us/products/prodinozero-eth.aspx";

class KMPDinoZeroEthClass
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

extern KMPDinoZeroEthClass KMPDinoZeroEth;

#endif