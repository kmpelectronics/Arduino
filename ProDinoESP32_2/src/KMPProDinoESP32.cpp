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


const BoardConfig_t BoardConfig[BOARDS_COUNT] = {
//	| ETH  |  3G  | LoRa |LoRaRFM| LED |
	{ false, false, false, false ,  1  },//ProDino_ESP32
	{ true , false, false, false ,  1  },//ProDino_ESP32_Ethernet
	{ false, true , false, false ,  1  },//ProDino_ESP32_GSM   
	{ false, false, true , false ,  1  },//ProDino_ESP32_LoRa
	{ false, false, false, true  ,  1  },//ProDino_ESP32_LoRa_RFM
	{ true , true , false, false ,  1  },//ProDino_ESP32_Ethernet_GSM
	{ true , false, true , false ,  1  },//ProDino_ESP32_Ethernet_LoRa
	{ true , false, false, true  ,  1  },//ProDino_ESP32_Ethernet_LoRa_RFM
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
#define StatusLedPin 0
NeoPixelBus<NeoGrbFeature, NeoEsp32BitBang800KbpsMethod> _statusLed(MaxStatusLedPixelCount, StatusLedPin);

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

uint32_t _TxFlushDelayuS;

unsigned long _blinkIntervalTimeout[MaxStatusLedPixelCount];
uint8_t _ledState[MaxStatusLedPixelCount];

void KMPProDinoESP32Class::begin(BoardType board)
{
	if (BOARDS_COUNT < board)return;
	begin(BoardConfig[board]);
}
void KMPProDinoESP32Class::begin(BoardConfig_t bConfig)
{
	_boardConfig = bConfig;

	bool isBoardInitialized = false;

	if (_boardConfig.Ethernet)
	{
		beginEthernet(true);
	}

	if (_boardConfig.GSM)
	{
		beginGSM(true);
	}

	if (_boardConfig.LoRa)
	{
		beginLoRa(true);
	}

	isBoardInitialized = true;

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

	for (uint8_t i = 0; i < _boardConfig.StatusLedCnt; i++)
	{
		_blinkIntervalTimeout[i] = 0;
		_ledState[i] = 0;
	}

	_statusLed.Begin();

	// RS485 pin init.
	pinMode(RS485Pin, OUTPUT);
	digitalWrite(RS485Pin, RS485Receive);
}

void KMPProDinoESP32Class::beginEthernet(bool startEthernet)
{
	// W5500 pin init.
	pinMode(W5500ResetPin, OUTPUT);

	if (startEthernet)
	{
		restartEthernet();
		Ethernet.init(W5500CSPin);
	}
	else
	{
		digitalWrite(W5500ResetPin, LOW);
	}
}

void KMPProDinoESP32Class::beginGSM(bool startGSM)
{
	// Start serial communication with the GSM modem.
	SerialModem.begin(115200, SERIAL_8N1, GSMRxPin, GSMTxPin);

	// Turn on the GSM module by triggering GSM_RESETN pin.
	pinMode(GSMResetPin, OUTPUT);
	if (startGSM)
	{
		restartGSM();
	}
	else
	{
		resetGSMOn();
	}

	// The GSM pin is output.
	pinMode(GSMCTSPin, INPUT);

	// RTS pin should be in LOW to the GSM modem works.
	pinMode(GSMRTSPin, OUTPUT);
	digitalWrite(GSMRTSPin, LOW);
}

void KMPProDinoESP32Class::beginLoRa(bool startLora)
{
	// Start serial communication with the GSM modem.
	SerialModem.begin(19200, SERIAL_8N1, LoRaRxPin, LoRaTxPin);

	// Turn on the Lora module by triggering LoraResetPin pin.
	pinMode(LoRaResetPin, OUTPUT);
	if (startLora)
	{
		restartLoRa();
	}
	else
	{
		resetLoRaOn();
	}

	// LoRa low pin.
	pinMode(LoRaLowPin, OUTPUT);
	digitalWrite(LoRaLowPin, LOW);
}

void KMPProDinoESP32Class::restartLoRa()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS.
	resetLoRaOn();
	delay(200);
	resetLoRaOff();
	delay(200);
}

void KMPProDinoESP32Class::resetLoRaOn()
{
	digitalWrite(LoRaResetPin, LOW);
}

void KMPProDinoESP32Class::resetLoRaOff()
{
	digitalWrite(LoRaResetPin, HIGH);
}

void KMPProDinoESP32Class::restartGSM()
{
	// Reset occurs when a low level is applied to the RESET_N pin, which is normally set high by an internal pull-up, for a valid time period min 10 mS.
	// In our device this pin is inverted.
	resetGSMOn();
	delay(20);
	resetGSMOff();
}

void KMPProDinoESP32Class::resetGSMOn()
{
	digitalWrite(GSMResetPin, HIGH);
}

void KMPProDinoESP32Class::resetGSMOff()
{
	digitalWrite(GSMResetPin, LOW);
}

void KMPProDinoESP32Class::restartEthernet()
{
	// RSTn Pull-up Reset (Active low) RESET should be held low at least 500 us for W5500 reset.
	digitalWrite(W5500ResetPin, LOW);
	delay(600);
	digitalWrite(W5500ResetPin, HIGH);
}

RgbColor KMPProDinoESP32Class::getStatusLed(uint8_t num)
{
	if (_boardConfig.StatusLedCnt <= num)return black;
	return _statusLed.GetPixelColor(num);
}

void KMPProDinoESP32Class::setStatusLed(RgbColor color, uint8_t num)
{
	if (_boardConfig.StatusLedCnt <= num)return;
	_statusLed.SetPixelColor(num, color);
	_statusLed.Show();
}

void KMPProDinoESP32Class::offStatusLed()
{
	for (uint8_t i = 0; i < _boardConfig.StatusLedCnt; i++)
	{
		_statusLed.SetPixelColor(i, black);
	}

	_statusLed.Show();
}

void KMPProDinoESP32Class::processStatusLed(RgbColor color, int blinkInterval, uint8_t num)
{
	if (_boardConfig.StatusLedCnt <= num)return;

	if (millis() > _blinkIntervalTimeout[num])
	{
		_ledState[num] = !_ledState[num];

		if (_ledState[num])
		{
			// Here you can check statuses: is WiFi connected, is there Ethernet connection and other...
			setStatusLed(color, num);
		}
		else
		{
			_statusLed.SetPixelColor(num, black);
			_statusLed.Show();
		}

		// Set next time to read data.
		_blinkIntervalTimeout[num] = millis() + blinkInterval;
	}
}

/* ----------------------------------------------------------------------- */
/* Relays methods. */
/* ----------------------------------------------------------------------- */

void KMPProDinoESP32Class::setRelayState(uint8_t relayNumber, bool state)
{
	// Check if relayNumber is out of range - return.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return;
	}

	MCP23S08.SetPinState(RELAY_PINS[relayNumber], state);
}

void KMPProDinoESP32Class::setRelayState(Relay relay, bool state)
{
	setRelayState((uint8_t)relay, state);
}

void KMPProDinoESP32Class::setAllRelaysState(bool state)
{
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		setRelayState(i, state);
	}
}

void KMPProDinoESP32Class::setAllRelaysOn()
{
	setAllRelaysState(true);
}

void KMPProDinoESP32Class::setAllRelaysOff()
{
	setAllRelaysState(false);
}

bool KMPProDinoESP32Class::getRelayState(uint8_t relayNumber)
{
	// Check if relayNumber is out of range - return false.
	if (relayNumber > RELAY_COUNT - 1)
	{
		return false;
	}

	return MCP23S08.GetPinState(RELAY_PINS[relayNumber]);
}

bool KMPProDinoESP32Class::getRelayState(Relay relay)
{
	return getRelayState((uint8_t)relay);
}

/* ----------------------------------------------------------------------- */
/* Opto input methods. */
/* ----------------------------------------------------------------------- */
bool KMPProDinoESP32Class::getOptoInState(uint8_t optoInNumber)
{
	// Check if optoInNumber is out of range - return false.
	if (optoInNumber > OPTOIN_COUNT - 1)
	{
		return false;
	}

	return !MCP23S08.GetPinState(OPTOIN_PINS[optoInNumber]);
}

bool KMPProDinoESP32Class::getOptoInState(OptoIn optoIn)
{
	return getOptoInState((uint8_t)optoIn);
}

/* ----------------------------------------------------------------------- */
/* RS485 methods. */
/* ----------------------------------------------------------------------- */
void KMPProDinoESP32Class::rs485Begin(unsigned long baud)
{
	rs485Begin(baud, SERIAL_8N1);
}

void KMPProDinoESP32Class::rs485Begin(unsigned long baud, uint32_t config)
{
	RS485Serial.begin(baud, config, RS485RxPin, RS485TxPin);
	_TxFlushDelayuS = (uint32_t)((1000000 / baud) * 15);
}

void KMPProDinoESP32Class::rs485End()
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

size_t KMPProDinoESP32Class::rs485Write(const uint8_t data)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data);

	RS485EndWrite();

	return result;
}

size_t KMPProDinoESP32Class::rs485Write(const uint8_t* data, size_t dataLen)
{
	RS485BeginWrite();

	size_t result = RS485Serial.write(data, dataLen);

	RS485EndWrite();

	return result;
}

int KMPProDinoESP32Class::rs485Read()
{
	return rs485Read(10, 10);
}

int KMPProDinoESP32Class::rs485Read(unsigned long delayWait, uint8_t repeatTime)
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