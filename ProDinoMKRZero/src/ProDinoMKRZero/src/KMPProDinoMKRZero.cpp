// KMPDinoZero.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards: 
//		KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//		Source file for KMP Dino WiFi board.
// Version: 1.0.1
// 	Fix RS485Read operation
// Date: 30.10.2016
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#include "KMPProDinoMKRZero.h"

// Relay outputs pins.
#define Rel1Pin  21 // PA07
#define Rel2Pin  20 // PA06
#define Rel3Pin  19 // PA05
#define Rel4Pin  18 // PA04

#define OptoIn1Pin 16 // PB02
#define OptoIn2Pin 7  // PA21
#define OptoIn3Pin 0  // PA22
#define OptoIn4Pin 1  // PA23

// Status led pin.
#define	StatusLedPin 6 // PA20

// W5500 pins.
#define W5500ResetPin 5  // PB11
#define W5500CSPin    4  // PB10

// RS485 pins. Serial.
#define RS485Pin 3  // PA11
#define RS485Serial Serial1
#define RS485Transmit     LOW
#define RS485Receive      HIGH

/**
 * @brief Relay pins.
 */
const uint8_t Relay_Pins[RELAY_COUNT] =
{ Rel1Pin, Rel2Pin, Rel3Pin, Rel4Pin };

/**
 * @brief Input pins.
 */
const int OPTOIN_PINS[OPTOIN_COUNT] =
{ OptoIn1Pin, OptoIn2Pin, OptoIn3Pin, OptoIn4Pin };

KMPProDinoMKRZeroClass KMPProDinoMKRZero;

/**
 * @brief Initialize KMP Dino WiFi board.
 *		   WiFi module ESP8266, Expander MCP23S17, relays and opto inputs.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::init()
{
	init(true);
}

void KMPProDinoMKRZeroClass::init(bool startEthernet)
{
	// Relay pins init.
	pinMode(Rel1Pin, OUTPUT);
	pinMode(Rel2Pin, OUTPUT);
	pinMode(Rel3Pin, OUTPUT);
	pinMode(Rel4Pin, OUTPUT);

	// Opto inputs pins init.
	pinMode(OptoIn1Pin, INPUT);
	pinMode(OptoIn2Pin, INPUT);
	pinMode(OptoIn3Pin, INPUT);
	pinMode(OptoIn4Pin, INPUT);
	
	// Set status led output pin.
	//pinMode(StatusLedPin, OUTPUT);
	//digitalWrite(StatusLedPin, LOW);

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Transmit);

	// W5500 pin init.
	pinMode(W5500ResetPin, OUTPUT);

	if (startEthernet) 
	{
		ResetEthernet();
		Ethernet.init(W5500CSPin);
	}
	else
	{
		digitalWrite(W5500ResetPin, LOW);
	}
}

void KMPProDinoMKRZeroClass::ResetEthernet()
{
	// RSTn Pull-up Reset (Active low) RESET should be held low at least 500 us for W5500 reset.
	digitalWrite(W5500ResetPin, LOW);
	delay(600);
	digitalWrite(W5500ResetPin, HIGH);
}


/* ----------------------------------------------------------------------- */
/* Relays methods. */
/* ----------------------------------------------------------------------- */

/**
 * @brief Set relay new state.
 *
 * @param relayNumber Number of relay from 0 to RELAY_COUNT - 1. 0 - Relay1, 1 - Relay2 ...
 * @param state New state of relay, true - On, false = Off.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::SetRelayState(uint8_t relayNumber, bool state)
{
	// Check if relayNumber is out of range - return.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return;
	}
	
	digitalWrite(Relay_Pins[relayNumber], state);
}

/**
 * @brief Set relay new state.
 *
 * @param relay Relays - Relay1, Relay2 ...
 * @param state New state of relay, true - On, false = Off.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::SetRelayState(Relay relay, bool state)
{
	SetRelayState((uint8_t)relay, state);
}

/**
 * @brief Set all relays new state.
 *
 * @param state New state of relay, true - On, false = Off.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::SetAllRelaysState(bool state)
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		SetRelayState(i, state);
	}
}

/**
 * @brief Set all relays in ON state.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::SetAllRelaysOn()
{
	SetAllRelaysState(true);
}

/**
 * @brief Set all relays in ON state.
 *
 * @return void
 */
void KMPProDinoMKRZeroClass::SetAllRelaysOff()
{
	SetAllRelaysState(false);
}

/**
 * @brief Get relay state.
 *
 * @param relayNumber Relay number from 0 to RELAY_COUNT - 1
 *
 * @return bool true relay is On, false is Off. If number is out of range - return false.
 */
bool KMPProDinoMKRZeroClass::GetRelayState(uint8_t relayNumber)
{
	// Check if relayNumber is out of range - return false.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return false;
	}

	return digitalRead(Relay_Pins[relayNumber]);
}

/**
 * @brief Get relay state.
 *
 * @param relay Relay1, Relay2 ...
 *
 * @return bool true relay is On, false is Off. If number is out of range - return false.
 */
bool KMPProDinoMKRZeroClass::GetRelayState(Relay relay)
{
	return GetRelayState((uint8_t)relay);
}

/* ----------------------------------------------------------------------- */
/* Opto input methods. */
/* ----------------------------------------------------------------------- */

/**
 * @brief Get opto in state.
 *
 * @param optoInNumber OptoIn number from 0 to OPTOIN_COUNT - 1
 *
 * @return bool true - opto in is On, false is Off. If number is out of range - return false.
 */
bool KMPProDinoMKRZeroClass::GetOptoInState(uint8_t optoInNumber)
{
	// Check if optoInNumber is out of range - return false.
	if (optoInNumber > OPTOIN_COUNT - 1)
	{
		return false;
	}

	return digitalRead(OPTOIN_PINS[optoInNumber]);
}

/**
 * @brief Get opto in state.
 *
 * @param relay OptoIn1, OptoIn2 ...
 *
 * @return bool true - opto in is On, false is Off. If number is out of range - return false.
 */
bool KMPProDinoMKRZeroClass::GetOptoInState(OptoIn optoIn)
{
	return GetOptoInState((uint8_t)optoIn);
}

/* ----------------------------------------------------------------------- */
/* RS485 methods. */
/* ----------------------------------------------------------------------- */

/**
* @brief Connect to RS485. With default configuration SERIAL_8N1.
*
* @param baud Speed.
*     Values: 75, 110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600 and 115200 bit/s.
*
* @return void
*/
void KMPProDinoMKRZeroClass::RS485Begin(unsigned long baud)
{
	RS485Begin(baud, SERIAL_8N1);
}

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
void KMPProDinoMKRZeroClass::RS485Begin(unsigned long baud, uint16_t config)
{
	RS485Serial.begin(baud, config);
}

/**
* @brief Close connection to RS485.
*
* @return void
*/
void KMPProDinoMKRZeroClass::RS485End()
{
	RS485Serial.end();
}

/**
* @brief Begin write data to RS485. 
*
* @return void
*/
void RS485BeginWrite()
{
	digitalWrite(RS485Pin, RS485Receive);
	delay(1);
}

/**
* @brief End write data to RS485.
*
* @return void
*/
void RS485EndWrite()
{
	RS485Serial.flush();
	delay(1);
	digitalWrite(RS485Pin, RS485Transmit);
}

/**
* @brief Transmit one byte data to RS485.
*
* @param data Transmit data.
*
* @return size_t Count of transmitted - one byte.
*/
size_t KMPProDinoMKRZeroClass::RS485Write(uint8_t data)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data);

	RS485EndWrite();

	return result;
}

/**
* @brief Transmit one char data to RS485.
*
* @param data Transmit data.
*
* @return size_t Count of transmitted - one char.
*/
size_t KMPProDinoMKRZeroClass::RS485Write(char data)
{
	return RS485Write((uint8_t)data);
}

/**
* @brief Transmit the text to RS485.
*
* @param data Text data to transmit.
*
* @return size_t Count of transmitted chars.
*/
size_t KMPProDinoMKRZeroClass::RS485Write(const char* data)
{
	RS485BeginWrite();

	size_t len = strlen(data);
	size_t result = 0;
	while(len > 0)
	{
		result += RS485Serial.write(*data++);
		--len;
	}

	RS485EndWrite();

	return result;
}

/**
* @brief Send array of bytes to RS485.
*
* @param data Array in bytes to be send.
* @param dataLen Array length.
*
* @return size_t Count of transmitted bytes.
*/
size_t KMPProDinoMKRZeroClass::RS485Write(uint8_t* data, uint8_t dataLen)
{
	RS485BeginWrite();

	size_t result = 0;
	for (size_t i = 0; i < dataLen; i++)
	{
		result += RS485Serial.write(data[i]);
	}

	RS485EndWrite();

	return result;
}

/**
* @brief Read received data from RS485.
*
*
* @return int Received byte.<para></para>
*   If result = -1 - buffer is empty, no data
*   if result > -1 - valid byte to read.
*/
int KMPProDinoMKRZeroClass::RS485Read()
{
	return RS485Read(10, 10);
}

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
int KMPProDinoMKRZeroClass::RS485Read(unsigned long delayWait, uint8_t repeatTime)
{
	// If the buffer empty, wait until the data arrive.
	while (!RS485Serial.available())
	{
		delay(delayWait);
		--repeatTime;

		if (repeatTime == 0)
		{
			return -1;
		}
	}

	return RS485Serial.read();
}
