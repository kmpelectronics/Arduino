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
#define	StatusLedPin  6 // PA20

// W5500 pins.
#define W5500ResetPin 5  // PB11
#define W5500CSPin    4  // PB10

// RS485 pins. Serial.
#define RS485Pin      3  // PA11
#define RS485Serial	  Serial1
#define RS485Transmit HIGH
#define RS485Receive  LOW

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
BoardType _board;

void KMPProDinoMKRZeroClass::init(BoardType board)
{
	init(board, true);
}

void KMPProDinoMKRZeroClass::init(BoardType board, bool startEthernet)
{
	_board = board;

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
	pinMode(StatusLedPin, OUTPUT);
	digitalWrite(StatusLedPin, LOW);

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Receive);

	InitEthernet(startEthernet);
}

void KMPProDinoMKRZeroClass::InitEthernet(bool startEthernet)
{
	if (_board == ProDino_MKR_Zero_Ethernet || _board == ProDino_MKR_GSM_Ethernet)
	{
		// W5500 pin init.
		pinMode(W5500ResetPin, OUTPUT);

		if (startEthernet)
		{
			RestartEthernet();
			Ethernet.init(W5500CSPin);
		}
		else
		{
			digitalWrite(W5500ResetPin, LOW);
		}
	}
}

void KMPProDinoMKRZeroClass::RestartEthernet()
{
	// RSTn Pull-up Reset (Active low) RESET should be held low at least 500 us for W5500 reset.
	digitalWrite(W5500ResetPin, LOW);
	delay(600);
	digitalWrite(W5500ResetPin, HIGH);
}

bool KMPProDinoMKRZeroClass::GetStatusLed()
{
	return digitalRead(StatusLedPin);
}

void KMPProDinoMKRZeroClass::SetStatusLed(bool state)
{
	digitalWrite(StatusLedPin, state);
}

void KMPProDinoMKRZeroClass::OnStatusLed()
{
	SetStatusLed(true);
}

void KMPProDinoMKRZeroClass::OffStatusLed()
{
	SetStatusLed(false);
}

void KMPProDinoMKRZeroClass::NotStatusLed()
{
	SetStatusLed(!GetStatusLed());
}

/* ----------------------------------------------------------------------- */
/* Relays methods. */
/* ----------------------------------------------------------------------- */

void KMPProDinoMKRZeroClass::SetRelayState(uint8_t relayNumber, bool state)
{
	// Check if relayNumber is out of range - return.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return;
	}
	
	digitalWrite(Relay_Pins[relayNumber], state);
}

void KMPProDinoMKRZeroClass::SetRelayState(Relay relay, bool state)
{
	SetRelayState((uint8_t)relay, state);
}

void KMPProDinoMKRZeroClass::SetAllRelaysState(bool state)
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		SetRelayState(i, state);
	}
}

void KMPProDinoMKRZeroClass::SetAllRelaysOn()
{
	SetAllRelaysState(true);
}

void KMPProDinoMKRZeroClass::SetAllRelaysOff()
{
	SetAllRelaysState(false);
}

bool KMPProDinoMKRZeroClass::GetRelayState(uint8_t relayNumber)
{
	// Check if relayNumber is out of range - return false.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return false;
	}

	return digitalRead(Relay_Pins[relayNumber]);
}

bool KMPProDinoMKRZeroClass::GetRelayState(Relay relay)
{
	return GetRelayState((uint8_t)relay);
}

/* ----------------------------------------------------------------------- */
/* Opto input methods. */
/* ----------------------------------------------------------------------- */

bool KMPProDinoMKRZeroClass::GetOptoInState(uint8_t optoInNumber)
{
	// Check if optoInNumber is out of range - return false.
	if (optoInNumber > OPTOIN_COUNT - 1)
	{
		return false;
	}

	return !digitalRead(OPTOIN_PINS[optoInNumber]);
}

bool KMPProDinoMKRZeroClass::GetOptoInState(OptoIn optoIn)
{
	return GetOptoInState((uint8_t)optoIn);
}

/* ----------------------------------------------------------------------- */
/* RS485 methods. */
/* ----------------------------------------------------------------------- */

void KMPProDinoMKRZeroClass::RS485Begin(unsigned long baud)
{
	RS485Begin(baud, SERIAL_8N1);
}

void KMPProDinoMKRZeroClass::RS485Begin(unsigned long baud, uint16_t config)
{
	RS485Serial.begin(baud, config);
}

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
	digitalWrite(RS485Pin, RS485Transmit);
}

/**
* @brief End write data to RS485.
*
* @return void
*/
void RS485EndWrite()
{
	RS485Serial.flush();
	digitalWrite(RS485Pin, RS485Receive);
}

size_t KMPProDinoMKRZeroClass::RS485Write(uint8_t data)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data);

	RS485EndWrite();

	return result;
}

size_t KMPProDinoMKRZeroClass::RS485Write(char data)
{
	return RS485Write((uint8_t)data);
}

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

int KMPProDinoMKRZeroClass::RS485Read()
{
	return RS485Read(10, 10);
}

int KMPProDinoMKRZeroClass::RS485Read(unsigned long delayWait, uint8_t repeatTime)
{
	// If the buffer is empty, wait until the data arrives.
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