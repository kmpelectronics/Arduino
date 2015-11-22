// KmpDinoEthernet.cpp
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Supported boards:
//		KMP DiNo II NETBOARD V1.0 (http://kmpelectronics.eu/en-us/products/dinoii.aspx)
// Description:
//		Code for KMP Dino Ethernet board.
// Version: 1.0.0
// Date: 17.01.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "KmpDinoEthernet.h"
#include <Arduino.h>
#include <assert.h>

void DinoInit()
{
	DinoInit(true);
}

void DinoInit( bool startEthernet )
{
    // Set relays output pins.
    pinMode(Relay1Pin, OUTPUT);
    pinMode(Relay2Pin, OUTPUT);
    pinMode(Relay3Pin, OUTPUT);
    pinMode(Relay4Pin, OUTPUT);

    // Set status led output pin.
    pinMode(StatusLedPin, OUTPUT);
    digitalWrite(StatusLedPin, LOW);

    // Set Opto in, input pins.
    pinMode(OptoIn1Pin, INPUT);
    pinMode(OptoIn2Pin, INPUT);
    pinMode(OptoIn3Pin, INPUT);
    pinMode(OptoIn4Pin, INPUT);

    // Prepare W5200
    pinMode(W5200PowerPin, OUTPUT);
    pinMode(W5200ResetPin, OUTPUT);
   
	if (startEthernet)
	{
		StartEthernet();
	}
	else
	{
		StopEthernet();
	}

    // Prepare RS485 pins.
    pinMode(RS485TXControlPin, OUTPUT);
    pinMode(RS485TXPin, OUTPUT);
    pinMode(RS485RXPin, INPUT);
    digitalWrite(RS485TXControlPin, RS485Receive);
}


void StartEthernet()
{
	// Stand by -> Power UP.
	W5200PowerUp();

	// Reset -> Operating mode.
	W5200ResetDown();
}

void StopEthernet()
{
	// Reset.
	W5200ResetUp();

	// Stand by.
	W5200PowerStandBy();
}

void RestartEthernet()
{
    StopEthernet();
    StartEthernet();
}

// Relay voids.
//
void SetRelayStatus(int relayNumber, Status status)
{
    // If relayNumber out of range - return.
    if(0 < relayNumber && relayNumber >= RELAY_COUNT)
    return;

    if (status != NONE)
    digitalWrite(RelayPins[relayNumber], status);
}

void SetRelayStatus(int relayNumber, bool status)
{
    Status rs;

    if(status)
    rs = ON;
    else
    rs = OFF;

    SetRelayStatus(relayNumber, rs);
}

void SetRelayStatus(Relay relay, Status status)
{
    SetRelayStatus(relay, status);
}

void SetRelayStatus(Relay relay, bool status)
{
    SetRelayStatus(relay, status);
}

void SetAllRelaysOn()
{
    for(int i = 0; i < RELAY_COUNT; i++)
    SetRelayStatus(i, ON);
}

void SetAllRelaysOff()
{
    for(int i = 0; i < RELAY_COUNT; i++)
    SetRelayStatus(i, OFF);
}

bool GetRelayStatus(int relayNumber)
{
    // If relayNumber out of range - throw error.
    assert(relayNumber >= 0 && relayNumber < RELAY_COUNT);
    return digitalRead(RelayPins[relayNumber]);
    //if(0 <= relayNumber && relayNumber >= RELAY_COUNT)
    //	return NONE;

    //if(HIGH == digitalRead(RelayPins[relayNumber]))
    //	return ON;
    //else
    //	return OFF;
}

bool GetRelayStatus(Relay relay)
{
    return digitalRead(RelayPins[relay]);
}

// Opto In voids.
//
bool GetOptoInStatus(int optoInNumber)
{
    // If optoInNumber out of range - throw error.
    assert(optoInNumber >= 0 && optoInNumber < OPTOIN_COUNT);
    return digitalRead(OptoInPins[optoInNumber]);

    // If optoInNumber out of range - return NONE.
    //if(0 < optoInNumber && optoInNumber >= OPTOIN_COUNT)
    //	return NONE;

    //if(HIGH == digitalRead(OptoInPins[optoInNumber]))
    //	return ON;
    //else
    //	return OFF;
}

bool GetOptoInStatus(OptoIn optoIn)
{
    return digitalRead(OptoInPins[optoIn]);
}

// Status LED voids.
//
bool GetStatusLed()
{
    return digitalRead(StatusLedPin);
}

void StatusLed(bool on)
{
    digitalWrite(StatusLedPin, on);
}

void OnStatusLed()
{
    StatusLed(true);
}

void OffStatusLed()
{
    StatusLed(false);
}

void NotStatusLed()
{
    StatusLed(!GetStatusLed());
}

// W5200 voids
//
void W5200PowerUp()
{
    digitalWrite(W5200PowerPin, LOW);
}
void W5200PowerStandBy()
{
    digitalWrite(W5200PowerPin, HIGH);
}

void W5200ResetUp()
{
    digitalWrite(W5200ResetPin, HIGH);
}

void W5200ResetDown()
{
    digitalWrite(W5200ResetPin, HIGH);
}

// RS485 voids
//
void RS485Begin(unsigned long boud)
{
    RS485Begin(boud, SERIAL_8N1);
}

void RS485Begin(unsigned long boud, uint8_t config)
{
    Serial1.begin(boud, config);
}

void RS485End()
{
    Serial1.end();
}

void RS485BeginWrite()
{
    digitalWrite(RS485TXControlPin, RS485Transmit);
    delay(1);
}

void RS485EndWrite()
{
    Serial1.flush();
    digitalWrite(RS485TXControlPin, RS485Receive);
}

size_t RS485Write(uint8_t data)
{
    RS485BeginWrite();
    
    size_t result = Serial1.write(data);
    
    RS485EndWrite();
        
    return result;
}

size_t RS485Write(char data)
{
    return RS485Write((uint8_t)data);
}

size_t RS485Write(char* data)
{
    RS485BeginWrite();
    
    size_t len = strlen(data);
    size_t result = 0;
    for(size_t i = 0; i < len; i++)
    {
        result += Serial1.write(data[i]);
    }
    
    RS485EndWrite();
    
    return result;
}

size_t RS485Write(uint8_t* data, uint8_t dataLen)
{
    RS485BeginWrite();
    
    size_t result = 0;
    for(size_t i = 0; i < dataLen; i++)
    {
	    result += Serial1.write(data[i]);
    }
    
    RS485EndWrite();
    
    return result;
}

int RS485Read()
{
    return RS485Read(10, 10);
}

int RS485Read(unsigned long delayWait, uint8_t repeatTime)
{
    digitalWrite(RS485TXControlPin, RS485Receive);

    // Wait before read if buffer is empty.
    while(Serial1.available() == 0 && repeatTime-- > 0)
    {
        delay(delayWait);
    }

    return Serial1.read();
}