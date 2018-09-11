// KMPDinoZeroEth.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards: 
//		ProDino MKR Zero ??? (http://www.kmpelectronics.eu/en-us/products/prodinozero-eth.aspx)
// Description:
//		Header for KMP Dino Zero Eth board.
// Version: 1.0.0
// Date: 15.03.2018
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
 * @brief A Grove connector pins
 */
#define	GROVE_D0	12 // PA09
#define	GROVE_D1	11 // PA08

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
	ProDino_MKR_Zero = 1,
	ProDino_MKR_Zero_Ethernet = 2,
	ProDino_MKR_GSM = 3,
	ProDino_MKR_GSM_Ethernet = 4
};

const char TEXT_HTML[] = "text/html; charset=utf-8";
const char PRODINO_MKRZERO[] = "ProDino MKR Zero series";
const char URL_KMPELECTRONICS_EU_PRODINO_MKRZERO[] = "https://kmpelectronics.eu/product-category/arduino-mkr-zero/";

class KMPProDinoMKRZeroClass
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
	* @param startEthernet If board has a Ethernet we can stop it if it isn't necessary. If true - starts Ethernet W5500 or false - the Ethernet is stopped.
	*
	* @return void
	*/
	void init(BoardType board, bool startEthernet);

	/**
	* @brief Restarts (Stop & Start) Ethernet.
	*
	* @return void
	*/
	void RestartEthernet();

	/**
	* @brief Get current status LED status.
	*
	* @return bool If equals - true LED On, else Off.
	*/
	bool GetStatusLed();
	/**
	* @brief Set status LED new state.
	*
	* @param on A new status: true - On, false - Off.
	*
	* @return void
	*/
	void SetStatusLed(bool state);
	/**
	* @brief Set status LED to On.
	*
	* @return void
	*/
	void OnStatusLed();
	/**
	* @brief Set status LED to Off.
	*
	* @return void
	*/
	void OffStatusLed();
	/**
	* @brief Invert status LED state. If it is On set to Off or inverse.
	*
	* @return void
	*/
	void NotStatusLed();

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
	void RS485Begin(unsigned long baud, uint16_t config);
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
	size_t RS485Write(uint8_t data);
	/**
	* @brief Transmit one char data to RS485.
	*
	* @param data Transmit data.
	*
	* @return size_t Count of transmitted - one char.
	*/
	size_t RS485Write(char data);
	/**
	* @brief Transmit the text to RS485.
	*
	* @param data Text data to transmit.
	*
	* @return size_t Count of transmitted chars.
	*/
	size_t RS485Write(const char* data);
	/**
	* @brief Transmit the text to RS485.
	*
	* @param data Text data to transmit.
	*
	* @return size_t Count of transmitted chars.
	*/
	size_t RS485Write(String data) { return RS485Write(data.c_str()); }
	/**
	* @brief Send array of bytes to RS485.
	*
	* @param data Array in bytes to be send.
	* @param dataLen Array length.
	*
	* @return size_t Count of transmitted bytes.
	*/
	size_t RS485Write(uint8_t* data, uint8_t dataLen);
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
};

extern KMPProDinoMKRZeroClass KMPProDinoMKRZero;

#endif