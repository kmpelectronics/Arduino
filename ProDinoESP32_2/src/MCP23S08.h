// MCP23S08.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: https://kmpelectronics.eu/
// Supported hardware: 
//		Expander MCP23S08
// Description:
//		Header file for work with expander.
// Version: 0.0.1
// Date: 14.12.2018
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#ifndef _MCP23S08_h
#define _MCP2S308_h

#include "arduino.h"
#include <SPI.h>

class MCP23S08Class
{
 protected:
	 uint8_t ReadRegister(uint8_t address);
	 void WriteRegister(uint8_t address, uint8_t data);
	 void TransferBytes();

 public:
	void init(int cs);
	void SetPinState(uint8_t pinNumber, bool state);
	bool GetPinState(uint8_t pinNumber);
	void SetPinDirection(uint8_t pinNumber, uint8_t mode);
};

extern MCP23S08Class MCP23S08;

#endif

