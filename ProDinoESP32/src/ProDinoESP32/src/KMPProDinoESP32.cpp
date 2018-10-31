// KMPProDinoESP32.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards: 
//		KMP ProDino WiFi-ESP32
// Description:
//		Source file for KMP Dino WiFi ESP32 board.
// Version: 0.0.1
// Date: 30.10.2016
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#include "KMPProDinoESP32.h"

// Relay outputs pins.
#define Rel1Pin  32 // GPIO32
#define Rel2Pin  33 // GPIO32
#define Rel3Pin  25 // GPIO25
#define Rel4Pin  26 // GPIO26

#define OptoIn1Pin 35  // GPI 35
#define OptoIn2Pin 34  // GPI 34
#define OptoIn3Pin 39  // GPI 39
#define OptoIn4Pin 36  // GPI 36

//// Status led pin.
//#define	StatusLedPin  6 // PA20

// W5500 pins.
#define W5500ResetPin 7  // GPIO7
#define W5500CSPin    5  // GPIO5

// RS485 pins. Serial.
#define RS485Pin      12  // GPIO12
#define RS485RxPin    27  // GPIO27
#define RS485TxPin    14  // GPIO14
#define RS485Transmit HIGH
#define RS485Receive  LOW
HardwareSerial RS485Serial(1);

#define GSMRxPin    4   // GPIO4
#define GSMTxPin    16  // GPIO16
#define GSMResetPin 2   // GPIO2
#define GSMCTSPin   13  // GPIO13
#define GSMRTSPin   15  // GPIO15

#define LoraRxPin    13  // GPIO13
#define LoraTxPin    16  // GPIO16
//#define LoraBoot0    ? // It isn't connected at the moment
#define LoraResetPin 0   // GPIO0
#define LoraRTSPin   15  // GPIO15
HardwareSerial SerialModem(2);

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

KMPProDinoESP32Class KMPProDinoESP32;
BoardType _board;

uint32_t _TxFlushDelayuS;

void KMPProDinoESP32Class::init(BoardType board)
{
	init(board, true, true);
}

void KMPProDinoESP32Class::init(BoardType board, bool startEthernet, bool startModem)
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
	//pinMode(StatusLedPin, OUTPUT);
	//digitalWrite(StatusLedPin, LOW);

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Receive);

	InitEthernet(startEthernet);
	InitGSM(startModem);
	InitLoRa(startModem);
}

void KMPProDinoESP32Class::InitEthernet(bool startEthernet)
{
	if (_board == ProDino_ESP32_Ethernet || _board == ProDino_ESP32_GSM_Ethernet)
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

void KMPProDinoESP32Class::InitGSM(bool startGSM)
{
	if (_board == ProDino_ESP32_GSM || _board == ProDino_ESP32_GSM_Ethernet)
	{
		// Start serial communication with the GSM modem.
		SerialModem.begin(115200, SERIAL_8N1, GSMRxPin, GSMTxPin);

		// Turn on the GSM module by triggering GSM_RESETN pin.
		pinMode(GSMResetPin, OUTPUT);
		if (startGSM)
		{
			RestartGSM();
		}
		else
		{
			ResetGSMOn();
		}

		// The GSM pin is output.
		pinMode(GSMCTSPin, INPUT);
		
		// RTS pin should be in LOW to the GSM modem works.
		pinMode(GSMRTSPin, OUTPUT);
		digitalWrite(GSMRTSPin, LOW);
	}
}

void KMPProDinoESP32Class::InitLoRa(bool startLora)
{
	if (_board == ProDino_ESP32_LoRa || _board == ProDino_ESP32_Lora_Ethernet)
	{
		// Start serial communication with the GSM modem.
		SerialModem.begin(19200, SERIAL_8N1, LoraRxPin, LoraTxPin);

		// Turn on the Lora module by triggering LoraResetPin pin.
		pinMode(LoraResetPin, OUTPUT);
		if (startLora)
		{
			RestartLora();
		}
		else
		{
			ResetLoraOn();
		}

		pinMode(LoraRTSPin, OUTPUT);
		digitalWrite(LoraRTSPin, LOW);

		//SerialModem.setTimeout(1);
	}
}

void KMPProDinoESP32Class::RestartLora()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS.
	ResetLoraOn();
	delay(200);
	ResetLoraOff();
	delay(200);
}

void KMPProDinoESP32Class::ResetLoraOn()
{
	digitalWrite(LoraResetPin, LOW);
}

void KMPProDinoESP32Class::ResetLoraOff()
{
	digitalWrite(LoraResetPin, HIGH);
}


void KMPProDinoESP32Class::RestartGSM()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS.
	// In our device this pin is inverted.
	ResetGSMOn();
	delay(20);
	ResetGSMOff();
}

void KMPProDinoESP32Class::ResetGSMOn()
{
	digitalWrite(GSMResetPin, HIGH);
}

void KMPProDinoESP32Class::ResetGSMOff()
{
	digitalWrite(GSMResetPin, LOW);
}

void KMPProDinoESP32Class::RestartEthernet()
{
	// RSTn Pull-up Reset (Active low) RESET should be held low at least 500 us for W5500 reset.
	digitalWrite(W5500ResetPin, LOW);
	delay(600);
	digitalWrite(W5500ResetPin, HIGH);
}

//bool KMPProDinoESP32Class::GetStatusLed()
//{
//	return digitalRead(StatusLedPin);
//}
//
//void KMPProDinoESP32Class::SetStatusLed(bool state)
//{
//	digitalWrite(StatusLedPin, state);
//}
//
//void KMPProDinoESP32Class::OnStatusLed()
//{
//	SetStatusLed(true);
//}
//
//void KMPProDinoESP32Class::OffStatusLed()
//{
//	SetStatusLed(false);
//}
//
//void KMPProDinoESP32Class::NotStatusLed()
//{
//	SetStatusLed(!GetStatusLed());
//}

/* ----------------------------------------------------------------------- */
/* Relays methods. */
/* ----------------------------------------------------------------------- */

void KMPProDinoESP32Class::SetRelayState(uint8_t relayNumber, bool state)
{
	// Check if relayNumber is out of range - return.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return;
	}

	digitalWrite(Relay_Pins[relayNumber], state);
}

void KMPProDinoESP32Class::SetRelayState(Relay relay, bool state)
{
	SetRelayState((uint8_t)relay, state);
}

void KMPProDinoESP32Class::SetAllRelaysState(bool state)
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		SetRelayState(i, state);
	}
}

void KMPProDinoESP32Class::SetAllRelaysOn()
{
	SetAllRelaysState(true);
}

void KMPProDinoESP32Class::SetAllRelaysOff()
{
	SetAllRelaysState(false);
}

bool KMPProDinoESP32Class::GetRelayState(uint8_t relayNumber)
{
	// Check if relayNumber is out of range - return false.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return false;
	}

	return digitalRead(Relay_Pins[relayNumber]);
}

bool KMPProDinoESP32Class::GetRelayState(Relay relay)
{
	return GetRelayState((uint8_t)relay);
}

/* ----------------------------------------------------------------------- */
/* Opto input methods. */
/* ----------------------------------------------------------------------- */

bool KMPProDinoESP32Class::GetOptoInState(uint8_t optoInNumber)
{
	// Check if optoInNumber is out of range - return false.
	if (optoInNumber > OPTOIN_COUNT - 1)
	{
		return false;
	}

	return !digitalRead(OPTOIN_PINS[optoInNumber]);
}

bool KMPProDinoESP32Class::GetOptoInState(OptoIn optoIn)
{
	return GetOptoInState((uint8_t)optoIn);
}

/* ----------------------------------------------------------------------- */
/* RS485 methods. */
/* ----------------------------------------------------------------------- */

void KMPProDinoESP32Class::RS485Begin(unsigned long baud)
{
	RS485Begin(baud, SERIAL_8N1);
}

void KMPProDinoESP32Class::RS485Begin(unsigned long baud, uint32_t config)
{
	RS485Serial.begin(baud, config, RS485RxPin, RS485TxPin);
	_TxFlushDelayuS = (uint32_t)((1000000 / baud) * 15);
}

void KMPProDinoESP32Class::RS485End()
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
	// Allowing pin should delay for 50 nS
	delayMicroseconds(70);
}

/**
* @brief End write data to RS485.
*
* @return void
*/
void RS485EndWrite()
{
	RS485Serial.flush();
	delayMicroseconds(_TxFlushDelayuS);
	digitalWrite(RS485Pin, RS485Receive);
}

size_t KMPProDinoESP32Class::RS485Write(const uint8_t data)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data);

	RS485EndWrite();

	return result;
}

size_t KMPProDinoESP32Class::RS485Write(const uint8_t* data, size_t dataLen)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data, dataLen);

	RS485EndWrite();

	return result;
}

int KMPProDinoESP32Class::RS485Read()
{
	return RS485Read(10, 10);
}

int KMPProDinoESP32Class::RS485Read(unsigned long delayWait, uint8_t repeatTime)
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