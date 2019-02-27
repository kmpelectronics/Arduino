// KmpDinoEthernet.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards: 
//		KMP DiNo II NETBOARD V1.0 (http://kmpelectronics.eu/en-us/products/dinoii.aspx)
// Description:
//		Header for KMP Dino Ethernet board.
// Version: 1.0.0
// Date: 17.01.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>
// Warning! RS485 don't work Arduino version 1.5.6 and next. This is version optimized for SAM microprocessors.
//          Please use the version 1.5.5 or 1.0.6 or latest (this versions only for ARM microprocessors).
#ifndef	KMPDINOETHERNET_H_INCLUDED
#define	KMPDINOETHERNET_H_INCLUDED

#include <Arduino.h>
#include "Ethernet/Ethernet.h"

// Count of inputs and outputs.
#define RELAY_COUNT  4
#define OPTOIN_COUNT 4

// Output pins.
#define Relay1Pin 0x04
#define Relay2Pin 0x0C // 12
#define Relay3Pin 0x0B // 11
#define Relay4Pin 0x07

// Input pins.
#define OptoIn1Pin  A2
#define OptoIn2Pin	A3
#define OptoIn3Pin	A4
#define OptoIn4Pin	A5

// 1 Wire pin.
#define OneWirePin 0X05

// Status led pin.
#define	StatusLedPin 0x0D // 13

// W5200 pins.
#define W5200PowerPin 0x09
#define W5200ResetPin 0x0A // 10

// RS485 pins.
#define RS485TXControlPin 0x03
#define RS485TXPin        0x01
#define RS485RXPin        0x00
#define RS485Transmit     HIGH
#define RS485Receive      LOW

/**
 * \brief Store relay pins for easy find.
 */
const int RelayPins[RELAY_COUNT] = 
    {Relay1Pin, Relay2Pin, Relay3Pin, Relay4Pin};

/**
 * \brief Relays enum.
 */
enum Relay {
    Relay1 = 0x00,
    Relay2 = 0x01,
    Relay3 = 0x02,
    Relay4 = 0x03,
};

/**
 * \brief For a simple enumerate.
 */
const Relay RelayList[RELAY_COUNT] =
    {Relay1, Relay2, Relay3, Relay4};

/**
 * \brief Relay or input status.
 */
enum Status {
    NONE = -1,
    OFF  =  0,
    ON   =  1
};

/**
 * \brief Store input pins for easy find.
 */
const int OptoInPins[OPTOIN_COUNT] = 
    {OptoIn1Pin, OptoIn2Pin, OptoIn3Pin, OptoIn4Pin};

/**
 * \brief Input enum.
 */
enum OptoIn {
    OptoIn1 = 0x00,
    OptoIn2 = 0x01,
    OptoIn3 = 0x02,
    OptoIn4 = 0x03,
};

/**
 * \brief For a simple enumerate.
 */
const OptoIn OptoInList[RELAY_COUNT] =
    {OptoIn1, OptoIn2, OptoIn3, OptoIn4};

/**
 * \brief Initialize Dino board. Set pins, start W5200.
 * 
 * 
 * \return void
 */
void DinoInit();

/**
 * \brief Initialize Dino board. Set pins, start W5200.
 * 
 * \param startEthernet If equal true - start W5200, if equal false - not start Ethernet.
 * 
 * \return void
 */
void DinoInit(bool startEthernet);

/**
 * \brief Start Ethernet chip W5200.
 * 
 * 
 * \return void
 */
void StartEthernet();

/**
 * \brief Stop Ethernet chip W5200.
 * 
 * 
 * \return void
 */
void StopEthernet();

/**
 * \brief Restart (Stop & Start) Ethernet chip W5200.
 * 
 * 
 * \return void
 */
void RestartEthernet();

/**
 * \brief Set relay new status.
 * 
 * \param relayNumber Relay number from 0 to RELAY_COUNT - 1
 * \param status New status relay.
 * 
 * \return void
 */
void SetRelayStatus(int relayNumber, Status status);

/**
 * \brief Set relay new status.
 * 
 * \param relay NumberRelay number from 0 to RELAY_COUNT - 1
 * \param status New status relay. true - On, false = Off.
 * 
 * \return void
 */
void SetRelayStatus(int relayNumber, bool status);

/**
 * \brief Set relay new status.
 * 
 * \param relay Relay from enum. Relay1, Relay2 ...
 * \param status New status relay.
 * 
 * \return void
 */
void SetRelayStatus(Relay relay, Status status);

/**
 * \brief Set relay new status.
 * 
 * \param relay Relay from enum. Relay1, Relay2 ...
 * \param status New status relay. true - On, false = Off.
 * 
 * \return void
 */
void SetRelayStatus(Relay relay, bool status);

/**
 * \brief Set all relays On.
 * 
 * 
 * \return void
 */
void SetAllRelaysOn();

/**
 * \brief Set all relays Off.
 * 
 * 
 * \return void
 */
void SetAllRelaysOff();

/**
 * \brief Get relay status.
 * 
 * \param relayNumber Relay number from 0 to RELAY_COUNT - 1
 * 
 * \return bool If result equal - true relay is On, else Off.
 * \except "Assert" if relayNumber out of range.
 */
bool GetRelayStatus(int relayNumber);

/**
 * \brief Get relay status.
 * 
 * \param relay Relay from enum. Relay1, Relay2 ...
 * 
 * \return bool If result equal - true relay is On, else Off.
 */
bool GetRelayStatus(Relay relay);

/**
 * \brief Get optical input status.
 * 
 * \param optoInNumber Optical input number from 0 to OPTOIN_COUNT - 1
 * 
 * \return bool If result equal - true optical input is On, else Off.
 * \except "Assert" if optoInNumber out of range.
 */ 
bool GetOptoInStatus(int optoInNumber);

/**
 * \brief Get optical input status.
 * 
 * \param optoIn Optical input enum. OptoIn1, OptoIn2...
 * 
 * \return bool If result equal - true optical input is On, else Off.
 */
bool GetOptoInStatus(OptoIn optoIn);

/**
 * \brief Get status status LED.
 * 
 * 
 * \return bool If result equal - true LED On, else Off.
 */
bool GetStatusLed();

/**
 * \brief Set status LED new status.
 * 
 * \param on New status LED. true - On, false = Off.
 * 
 * \return void
 */
void StatusLed(bool on);

/**
 * \brief Set status LED On.
 * 
 * 
 * \return void
 */
void OnStatusLed();

/**
 * \brief Set status LED Off.
 * 
 * 
 * \return void
 */
void OffStatusLed();

/**
 * \brief Set status LED inverse state. If On set Off, if Off set On.
 * 
 * 
 * \return void
 */
void NotStatusLed();

/**
 * \brief Set power UP Ethernet chip W5200. StandBy -> PowerUp.
 * 
 * 
 * \return void
 */
void W5200PowerUp();

/**
 * \brief Set Ethernet chip W5200 to Stand by. PowerUp -> StandBy.
 * 
 * 
 * \return void
 */
void W5200PowerStandBy();

/**
 * \brief Set Reset Up Ethernet chip W5200. No reset -> Reset.
 * 
 * 
 * \return void
 */
void W5200ResetUp();

/**
 * \brief Set release Reset Ethernet chip W5200. Reset -> Start chip.
 * 
 * 
 * \return void
 */
void W5200ResetDown();

/**
 * \brief Connect to RS485. With default configuration SERIAL_8N1.
 * 
 * \param boud Boud rate.
 *     Values: 75, 110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200 bit/s.
 * 
 * \return void
 */
void RS485Begin(unsigned long boud);

/**
 * \brief Start connect to RS485.
 * 
 * \param boud Boud rate. 
 *             Values: 75, 110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200 bit/s.
 * \param config Configuration - data bits, parity, stop bits.
 *               Values: SERIAL_5N1, SERIAL_6N1, SERIAL_7N1, SERIAL_8N1 (the default), SERIAL_5N2, SERIAL_6N2, SERIAL_7N2, SERIAL_8N2, SERIAL_5E1, SERIAL_6E1, SERIAL_7E1, SERIAL_8E1, SERIAL_5E2, SERIAL_6E2, SERIAL_7E2, SERIAL_8E2, SERIAL_5O1, SERIAL_6O1, SERIAL_7O1, SERIAL_8O1, SERIAL_5O2, SERIAL_6O2, SERIAL_7O2, SERIAL_8O2
 * 
 * \return void
 */
void RS485Begin(unsigned long baud, byte config);

/**
 * \brief Close connection to RS485.
 * 
 * 
 * \return void
 */
void RS485End();

/**
 * \brief Transmit one byte data to RS485.
 * 
 * \param data Data to transmit.
 * 
 * \return size_t Count of transmitted - one byte.
 */
size_t RS485Write(uint8_t data);

/**
 * \brief Transmit one char data to RS485.
 * 
 * \param data Data to transmit.
 * 
 * \return size_t Count of transmitted - one char.
 */
size_t RS485Write(char data);

/**
 * \brief Transmit text to RS485.
 * 
 * \param data Text data to transmit.
 * 
 * \return size_t Count of transmitted chars.
 */
size_t RS485Write(char* data);

/**
 * \brief Send array of bytes to RS485.
 * 
 * \param data Array in bytes to be send.
 * \param dataLen Array length.
 * 
 * \return size_t Count of transmitted bytes.
 */
size_t RS485Write(uint8_t* data, uint8_t dataLen);

/**
 * \brief Read received data from RS485.
 * 
 * 
 * \return int Received byte.<para></para>
 *   If result = -1 - buffer is empty, no data
 *   if result > -1 - valid byte to read.
 */
int RS485Read();

/**
 * \brief Read received data from RS485. Reading data with delay and repeating the operation for all data to arrive.
 * 
 * \param delayWait Wait delay if not available to read byte in milliseconds. Default 10.
 * \param repeatTime Repeat time if not read bytes. Default 10. All time = delayWait * repeatTime.
 * 
 * \return int Received byte.
 *   If result = -1 - buffer is empty, no data<para></para>
 *   if result > -1 - valid byte to read.
 */
int RS485Read(unsigned long delayWait, uint8_t repeatTime);

#endif