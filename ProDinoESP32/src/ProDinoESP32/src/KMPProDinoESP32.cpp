// KMPProDinoESP32.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported boards: 
//		KMP ProDino ESP32 V1 https://kmpelectronics.eu/products/prodino-esp32-v1/
//		KMP ProDino ESP32 Ethernet V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-v1/
//		KMP ProDino ESP32 GSM V1 https://kmpelectronics.eu/products/prodino-esp32-gsm-v1/
//		KMP ProDino ESP32 LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-lora-v1/
//		KMP ProDino ESP32 LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-lora-rfm-v1/
//		KMP ProDino ESP32 Ethernet GSM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-gsm-v1/
//		KMP ProDino ESP32 Ethernet LoRa RFM V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-rfm-v1/
//		KMP ProDino ESP32 Ethernet LoRa V1 https://kmpelectronics.eu/products/prodino-esp32-ethernet-lora-v1/
// Description:
//		Source file for KMP Dino WiFi ESP32 board.
// Version: 0.6.5
// Date: 20.12.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu> & Dimitar Antonov <d.antonov@kmpelectronics.eu>

#include "KMPProDinoESP32.h"

struct BoardConfig_t {
	BoardType Board;
	bool Ethernet;
	bool GSM;
	bool LoRa;
	bool LoRaRFM;
};

const BoardConfig_t BoardConfig[BOARDS_COUNT] = {
	{ProDino_ESP32, false, false, false, false},
	{ProDino_ESP32_Ethernet, true, false, false, false},
	{ProDino_ESP32_GSM, false, true, false, false},
	{ProDino_ESP32_LoRa, false, false, true, false},
	{ProDino_ESP32_LoRa_RFM, false, false, false, true},
	{ProDino_ESP32_Ethernet_GSM, true, true, false, false},
	{ProDino_ESP32_Ethernet_LoRa, true, false, true, false},
	{ProDino_ESP32_Ethernet_LoRa_RFM, true, false, false, true},
};

// Relay pins
#define REL1PIN 7
#define REL2PIN 6
#define REL3PIN 5
#define REL4PIN 4
/**
 * @brief Relay pins.
 */
const uint8_t RELAY_PINS[RELAY_COUNT] = { REL1PIN, REL2PIN, REL3PIN, REL4PIN };

// Input pins
#define IN1PIN 3
#define IN2PIN 2
#define IN3PIN 1
#define IN4PIN 0
/**
 * @brief Input pins.
 */
const int OPTOIN_PINS[OPTOIN_COUNT] = { IN1PIN, IN2PIN, IN3PIN, IN4PIN };

// Expander CS pin.
#define MCP23S08CSPin 32  // IO32

// Status RGB LED.
#define StatusLedPixelCount 1
#define StatusLedPin 0
#define StatusLedPixelNumber 0
NeoPixelBus<NeoGrbFeature, NeoEsp32BitBang800KbpsMethod> _statusLed(StatusLedPixelCount, StatusLedPin);

// W5500 pins.
#define W5500ResetPin 12  // I12
#define W5500CSPin    33 // IO33

// RS485 pins. Serial1.
#define RS485Pin      2   // IO12
#define RS485RxPin    4   // IO4
#define RS485TxPin    16  // IO16
#define RS485Transmit HIGH
#define RS485Receive  LOW
HardwareSerial RS485Serial(1);

// GSM module pins for Serial2
#define GSMCTSPin   J14_5
#define GSMRTSPin   J14_6
#define GSMRxPin    J14_7
#define GSMTxPin    J14_8
#define GSMDTRPin   J14_9
#define GSMResetPin J14_10

// LoRa module pins for Serial2
#define LoRaRxPin    J14_5
#define LoRaLowPin   J14_6
#define LoRaTxPin    J14_8
#define LoRaBootPin  J14_11
#define LoRaResetPin J14_12
HardwareSerial SerialModem(2);

#define colorSaturation 32 // Max 255 but light is too sharp.
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation / 2, 0);
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

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

	bool isBoardInitialized = false;
	for (size_t i = 0; i < BOARDS_COUNT; i++)
	{
		BoardConfig_t boardConfig = BoardConfig[i];
		if (board == boardConfig.Board)
		{
			isBoardInitialized = true;

			if (boardConfig.Ethernet)
			{
				InitEthernet(startEthernet);
			}

			if (boardConfig.GSM)
			{
				InitGSM(startModem);
			}

			if (boardConfig.LoRa)
			{
				InitLoRa(startModem);
			}

			break;
		}
	}

	if (!isBoardInitialized)
	{
		Serial.println(F("The board is not initialized!"));
		while (1) {}
	}

	// Init expander pins.
	pinMode(MCP23S08IntetuptPin, INPUT);

	// Set expander pins direction.
	MCP23S08.init(MCP23S08CSPin);
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		MCP23S08.SetPinDirection(RELAY_PINS[i], OUTPUT);
	}

	for (uint8_t i = 0; i < OPTOIN_COUNT; i++)
	{
		MCP23S08.SetPinDirection(OPTOIN_PINS[i], INPUT);
	}

	// Status led.
	_statusLed.Begin();

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Receive);
}

void KMPProDinoESP32Class::InitEthernet(bool startEthernet)
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

void KMPProDinoESP32Class::InitGSM(bool startGSM)
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

void KMPProDinoESP32Class::InitLoRa(bool startLora)
{
	// Start serial communication with the GSM modem.
	SerialModem.begin(19200, SERIAL_8N1, LoRaRxPin, LoRaTxPin);

	// Turn on the Lora module by triggering LoraResetPin pin.
	pinMode(LoRaResetPin, OUTPUT);
	if (startLora)
	{
		RestartLoRa();
	}
	else
	{
		ResetLoRaOn();
	}

	// LoRa low pin.
	pinMode(LoRaLowPin, OUTPUT);
	digitalWrite(LoRaLowPin, LOW);
}

void KMPProDinoESP32Class::RestartLoRa()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS.
	ResetLoRaOn();
	delay(200);
	ResetLoRaOff();
	delay(200);
}

void KMPProDinoESP32Class::ResetLoRaOn()
{
	digitalWrite(LoRaResetPin, LOW);
}

void KMPProDinoESP32Class::ResetLoRaOff()
{
	digitalWrite(LoRaResetPin, HIGH);
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

RgbColor KMPProDinoESP32Class::GetStatusLed()
{
	return _statusLed.GetPixelColor(StatusLedPixelNumber);
}

void KMPProDinoESP32Class::SetStatusLed(RgbColor color)
{
	_statusLed.SetPixelColor(StatusLedPixelNumber, color);
	_statusLed.Show();
}

//void KMPProDinoESP32Class::OnStatusLed()
//{
//	SetStatusLed(true);
//}

void KMPProDinoESP32Class::OffStatusLed()
{
	_statusLed.SetPixelColor(StatusLedPixelNumber, black);
	_statusLed.Show();
}

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

	MCP23S08.SetPinState(RELAY_PINS[relayNumber], state);
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

	return MCP23S08.GetPinState(RELAY_PINS[relayNumber]);
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

	return !MCP23S08.GetPinState(OPTOIN_PINS[optoInNumber]);
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