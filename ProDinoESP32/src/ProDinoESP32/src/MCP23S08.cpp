// MCP23S08.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported hardware: 
//		Expander MCP23S08
// Description:
//		Source file for work with expander.
// Version: 0.0.1
// Date: 14.12.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "MCP23S08.h"

#define READ_CMD  0x41
#define WRITE_CMD 0x40

#define IODIR   0x00
#define IPOL    0x01
#define GPINTEN 0x02
#define DEFVAL  0x03
#define INTCON  0x04
#define IOCON   0x05
#define GPPU    0x06
#define INTF    0x07
#define INTCAP  0x08
#define GPIO    0x09
#define OLAT    0x0A

#define MAX_PIN_POS 7

int _cs;

uint8_t  _expTxData[16]  __attribute__((aligned(4)));
uint8_t  _expRxData[16]  __attribute__((aligned(4)));

void MCP23S08Class::init(int cs)
{
	_cs = cs;
	// Expander settings.
	SPI.begin();
	SPI.setHwCs(true);
#ifndef ESP32
	SPI.setFrequency(1000000);
	SPI.setDataMode(SPI_MODE0);
#endif // ESP32

	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);
}

/**
 * @brief Set a pin state.
 *
 * @param pinNumber The number of pin to be set.
 * @param state The pin state, true - 1, false - 0.
 *
 * @return void
 */
void MCP23S08Class::SetPinState(uint8_t pinNumber, bool state)
{
	if (pinNumber > MAX_PIN_POS)
	{
		return;
	}

	uint8_t registerData = ReadRegister(OLAT);

	if (state)
	{
		registerData |= (1 << pinNumber);
	}
	else
	{
		registerData &= ~(1 << pinNumber);
	}

	WriteRegister(OLAT, registerData);
}

/**
 * @brief Get a pin state.
 *
 * @param pinNumber The number of pin to be get.
 *
 * @return State true - 1, false - 0.
 */
bool MCP23S08Class::GetPinState(uint8_t pinNumber)
{
	if (pinNumber > MAX_PIN_POS)
	{
		return false;
	}

	uint8_t registerData = ReadRegister(GPIO);

	return registerData & (1 << pinNumber);
}

/**
 * @brief Read an expander MCP23S08 a register.
 *
 * @param address A register address.
 *
 * @return The data from the register.
 */
uint8_t MCP23S08Class::ReadRegister(uint8_t address)
{
	_expTxData[0] = READ_CMD;
	_expTxData[1] = address;

	TransferBytes();

	return _expRxData[2];
}

/**
 * @brief Write data in expander MCP23S08 register.
 *
 * @param address A register address.
 * @param data A byte for write.
 *
 * @return void.
 */
void MCP23S08Class::WriteRegister(uint8_t address, uint8_t data)
{
	_expTxData[0] = WRITE_CMD;
	_expTxData[1] = address;
	_expTxData[2] = data;

	TransferBytes();
}

void MCP23S08Class::TransferBytes()
{
	digitalWrite(_cs, LOW);
	SPI.transferBytes(_expTxData, _expRxData, 3);
	digitalWrite(_cs, HIGH);
}

/**
 * @brief Set the expander MCP23S08 a pin direction.
 *
 * @param pinNumber Pin number for set.
 * @param mode direction mode. 0 - INPUT, 1 - OUTPUT.
 *
 * @return void
 */
void MCP23S08Class::SetPinDirection(uint8_t pinNumber, uint8_t mode)
{
	if (pinNumber > MAX_PIN_POS)
	{
		return;
	}

	uint8_t registerData = ReadRegister(IODIR);

	if (INPUT == mode)
	{
		registerData |= (1 << pinNumber);
	}
	else
	{
		registerData &= ~(1 << pinNumber);
	}

	WriteRegister(IODIR, registerData);
}

MCP23S08Class MCP23S08;

