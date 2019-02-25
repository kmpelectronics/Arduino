// KMPProDinoESP32.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards: 
//		KMP ProDino ESP32 V1 https://kmpelectronics.eu/products/prodino-esp32-v1/
//		KMP ProDino ESP32 Ethernet V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/
//		KMP ProDino ESP32 GSM V1 https://kmpelectronics.eu/products/prodino-esp32-gsm-v1/
//		KMP ProDino ESP32 LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-lora-v1/
//		KMP ProDino ESP32 LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-lora-rfm-v1/
//		KMP ProDino ESP32 Ethernet GSM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
//		KMP ProDino ESP32 Ethernet LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-v1/
//		KMP ProDino ESP32 Ethernet LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-rfm-v1/
// Description:
//		Header for KMP ProDino ESP32 boards.
// Version: 0.6.5
// Date: 20.12.2018
// Authors: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#ifndef _KMPPRODINOESP32_H
#define _KMPPRODINOESP32_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "MCP23S08.h"
#include "Ethernet2/Ethernet2.h"
#include "NeoPixel/NeoPixelBus.h"

// Relays count
#define RELAY_COUNT  4
// Inputs count
#define OPTOIN_COUNT 4

// Expander interrupt pin 
#define MCP23S08IntetuptPin 36  // IO36

/**
 * @brief A Grove connector pins
 */
#define	GROVE_D0	22 // IO22
#define	GROVE_D1	21 // IO21

 /**
  * @brief J14 connector
  */
//#define	J14_1	GND
//#define	J14_2	5V
//#define	J14_3	GND
//#define	J14_4	3V3
#define	J14_5	35 // I35
#define	J14_6	27 // IO27
#define	J14_7	34 // I34
#define	J14_8	25 // IO25
#define	J14_9	26 // IO26
#define	J14_10	14 // IO14
#define	J14_11	13 // IO13
#define	J14_12	15 // IO15


/**
 * @brief Relays.
 */
enum Relay {
	Relay1 = 0,
	Relay2 = 1,
	Relay3 = 2,
	Relay4 = 3
};

/**
 * @brief Inputs.
 */
enum OptoIn {
	OptoIn1 = 0,
	OptoIn2 = 1,
	OptoIn3 = 2,
	OptoIn4 = 3
};

/**
 * @brief All available bards.
 */
enum BoardType {
	ProDino_ESP32,
	ProDino_ESP32_Ethernet,
	ProDino_ESP32_GSM,
	ProDino_ESP32_LoRa,
	ProDino_ESP32_LoRa_RFM,
	ProDino_ESP32_Ethernet_GSM,
	ProDino_ESP32_Ethernet_LoRa,
	ProDino_ESP32_Ethernet_LoRa_RFM
};

#define BOARDS_COUNT ProDino_ESP32_Ethernet_LoRa_RFM + 1

const char TEXT_HTML[] = "text/html; charset=utf-8";
const char PRODINO_ESP32[] = "ProDino ESP32";
const char URL_KMPELECTRONICS_EU_PRODINO_ESP32[] = "https://kmpelectronics.eu/products/prodino-esp32/";

extern RgbColor yellow;
extern RgbColor orange;
extern RgbColor red;
extern RgbColor green;
extern RgbColor blue;
extern RgbColor white;
extern RgbColor black;

extern HardwareSerial SerialModem;

class KMPProDinoESP32Class
{
 public:
	/**
	* @brief Initialize KMP ProDino MKR Zero board. The Ethernet doesn't start.
	*		  Micro controller Arduino Zero compatible, GSM module, relays and opto inputs.
	* @param board Initialize specific bard. Mandatory.
	*
	* @return void
	*/
	void init(BoardType board);
	/**
	* @brief Initialize KMP ProDino MKR Zero board.
	*		  Micro controller Arduino Zero compatible, GSM module, relays and opto inputs.
	* @param board Initialize specific bard. Mandatory.
	* @param startEthernet If board has a Ethernet we can stop it if it isn't necessary. If true - starts Ethernet W5500 or false - the Ethernet stays stopped.
	* @param startModem If board has GSM or LoRa module we can stop it if it isn't necessary. If true - starts module or false - the module stays stopped.
	*
	* @return void
	*/
	void init(BoardType board, bool startEthernet, bool startModem);

	/**
	* @brief Restarts (Stop & Start) GSM module.
	*
	* @return void
	*/
	void RestartGSM();

	/**
	* @brief Restarts (Stop & Start) Ethernet.
	*
	* @return void
	*/
	void RestartEthernet();

	/**
	* @brief Get status LED current color.
	*
	* @return RgbColor RGB color.
	*/
	RgbColor GetStatusLed();
	/**
	* @brief Set status LED new color.
	*
	* @param color A new RGB color.
	*
	* @return void
	*/
	void SetStatusLed(RgbColor color);
	///**
	//* @brief Set status LED to On.
	//*
	//* @return void
	//*/
	//void OnStatusLed();
	/**
	* @brief Set status LED to Off.
	*
	* @return void
	*/
	void OffStatusLed();
	///**
	//* @brief Invert status LED state. If it is On set to Off or inverse.
	//*
	//* @return void
	//*/
	//void NotStatusLed();

	/**
	* @brief Set a relay new state.
	*
	* @param relayNumber Number of relay from 0 to RELAY_COUNT - 1. 0 - Relay1, 1 - Relay2 ...
	* @param state New state of relay, true - On, false = Off.
	*
	* @return void
	*/
	void SetRelayState(uint8_t relayNumber, bool state);
	/**
	* @brief Set relay new state.
	*
	* @param relay Relays - Relay1, Relay2 ...
	* @param state New state of relay, true - On, false = Off.
	*
	* @return void
	*/
	void SetRelayState(Relay relay, bool state);
	/**
	* @brief Set all relays new state.
	*
	* @param state New state of relay, true - On, false = Off.
	*
	* @return void
	*/
	void SetAllRelaysState(bool state);
	/**
	* @brief Set all relays in ON state.
	*
	* @return void
	*/
	void SetAllRelaysOn();
	/**
	* @brief Set all relays in ON state.
	*
	* @return void
	*/
	void SetAllRelaysOff();
	/**
	* @brief Get relay state.
	*
	* @param relayNumber Relay number from 0 to RELAY_COUNT - 1
	*
	* @return bool true relay is On, false is Off. If number is out of range - return false.
	*/
	bool GetRelayState(uint8_t relayNumber);
	/**
	* @brief Get relay state.
	*
	* @param relay Relay1, Relay2 ...
	*
	* @return bool true relay is On, false is Off. If number is out of range - return false.
	*/
	bool GetRelayState(Relay relay);

	/**
	* @brief Get opto in state.
	*
	* @param optoInNumber OptoIn number from 0 to OPTOIN_COUNT - 1
	*
	* @return bool true - opto in is On, false is Off. If number is out of range - return false.
	*/
	bool GetOptoInState(uint8_t optoInNumber);
	/**
	* @brief Get opto in state.
	*
	* @param relay OptoIn1, OptoIn2 ...
	*
	* @return bool true - opto in is On, false is Off. If number is out of range - return false.
	*/
	bool GetOptoInState(OptoIn optoIn);

	/**
	* @brief Connect to RS485. With default configuration SERIAL_8N1.
	*
	* @param baud Speed.
	*     Values: 75, 110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200 bit/s.
	*
	* @return void
	*/
	void RS485Begin(unsigned long baud);
	/**
	* @brief Start connect to RS485.
	*
	* @param baud Speed.
	*             Values: 75, 110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200 bit/s.
	* @param config Configuration - data bits, parity, stop bits.
	*               Values: SERIAL_5N1, SERIAL_6N1, SERIAL_7N1, SERIAL_8N1, SERIAL_5N2, SERIAL_6N2, SERIAL_7N2, SERIAL_8N2, SERIAL_5E1, SERIAL_6E1, SERIAL_7E1, SERIAL_8E1, SERIAL_5E2,
	SERIAL_6E2, SERIAL_7E2, SERIAL_8E2, SERIAL_5O1, SERIAL_6O1, SERIAL_7O1, SERIAL_8O1, SERIAL_5O2, SERIAL_6O2, SERIAL_7O2, SERIAL_8O2
	*
	* @return void
	*/
	void RS485Begin(unsigned long baud, uint32_t config);
	/**
	* @brief Close connection to RS485.
	*
	* @return void
	*/
	void RS485End();
	/**
	* @brief Transmit one byte data to RS485.
	*
	* @param data Transmit data.
	*
	* @return size_t Count of transmitted - one byte.
	*/
	size_t RS485Write(const uint8_t data);
	/**
	* @brief Transmit one char data to RS485.
	*
	* @param data Transmit data.
	*
	* @return size_t Count of transmitted - one char.
	*/
	size_t RS485Write(const char data) { return RS485Write((uint8_t)data); }
	/**
	* @brief Transmit the text to RS485.
	*
	* @param data Text data to transmit.
	* @param dataLen Array length.
	*
	* @return size_t Count of transmitted chars.
	*/
	size_t RS485Write(const char* data, size_t dataLen) { return RS485Write((const uint8_t*)data, dataLen); }
	/**
	* @brief Transmit the text to RS485.
	*
	* @param data Text data to transmit.
	*
	* @return size_t Count of transmitted chars.
	*/
	size_t RS485Write(const String data) { return RS485Write(data.c_str(), data.length()); }
	/**
	* @brief Send array of bytes to RS485.
	*
	* @param data Array in bytes to be send.
	* @param dataLen Array length.
	*
	* @return size_t Count of transmitted bytes.
	*/
	size_t RS485Write(const uint8_t* data, size_t dataLen);
	/**
	* @brief Read received data from RS485.
	*
	*
	* @return int Received byte.<para></para>
	*   If result = -1 - buffer is empty, no data
	*   if result > -1 - valid byte to read.
	*/
	int RS485Read();
	/**
	* @brief Read received data from RS485. Reading data with delay and repeating the operation while all data to arrive.
	*
	* @param delayWait Wait delay if not available to read byte in milliseconds. Default 10.
	* @param repeatTime Repeat time if not read bytes. Default 10. All time = delayWait * repeatTime.
	*
	* @return int Received byte.
	*   If result = -1 - buffer is empty, no data<para></para>
	*   if result > -1 - valid byte to read.
	*/
	int RS485Read(unsigned long delayWait, uint8_t repeatTime);

	private:
		void InitEthernet(bool startEthernet);
		void InitGSM(bool startGSM);
		void ResetGSMOn();
		void ResetGSMOff();
		void InitLoRa(bool startLora);
		void RestartLoRa();
		void ResetLoRaOn();
		void ResetLoRaOff();
};

extern KMPProDinoESP32Class KMPProDinoESP32;

#endif