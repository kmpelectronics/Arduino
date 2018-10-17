// KMPProDinoESP32.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported board: 
//		KMP ProDino ESP32 https://kmpelectronics.eu/products/prodino-esp32-v1/
// Description:
//		Header for KMP Dino WiFi ESP32 board.
// Version: 0.0.1
// Date: 05.10.2018
// Authors: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#ifndef _KMPPRODINOESP32_H
#define _KMPPRODINOESP32_H

#include <Arduino.h>
#include <Ethernet2.h>
#include <HardwareSerial.h>

// Inputs and outputs count.
#define RELAY_COUNT  4
#define OPTOIN_COUNT 4

/**
 * @brief A Grove connector pins
 */
#define	GROVE_D0	22 // GPIO22
#define	GROVE_D1	21 // GPIO21

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
	None = 0,
	ProDino_ESP32 = 1,
	ProDino_ESP32_Ethernet = 2,
	ProDino_ESP32_GSM = 3,
	ProDino_ESP32_GSM_Ethernet = 4
};

const char TEXT_HTML[] = "text/html; charset=utf-8";
const char PRODINO_ESP32[] = "ProDino ESP32";
const char URL_KMPELECTRONICS_EU_PRODINO_ESP32[] = "https://kmpelectronics.eu/product-category/arduino-esp32/";

extern HardwareSerial SerialGSM;

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
	* @param startEthernet If board has a Ethernet we can stop it if it isn't necessary. If true - starts Ethernet W5500 or false - the Ethernet stays stop.
	* @param startGSM If board has a GSM we can stop it if it isn't necessary. If true - starts GSM module or false - the GSM stays stop.
	*
	* @return void
	*/
	void init(BoardType board, bool startEthernet, bool startGSM);

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

	///**
	//* @brief Get current status LED status.
	//*
	//* @return bool If equals - true LED On, else Off.
	//*/
	//bool GetStatusLed();
	///**
	//* @brief Set status LED new state.
	//*
	//* @param on A new status: true - On, false - Off.
	//*
	//* @return void
	//*/
	//void SetStatusLed(bool state);
	///**
	//* @brief Set status LED to On.
	//*
	//* @return void
	//*/
	//void OnStatusLed();
	///**
	//* @brief Set status LED to Off.
	//*
	//* @return void
	//*/
	//void OffStatusLed();
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
};

extern KMPProDinoESP32Class KMPProDinoESP32;

#endif