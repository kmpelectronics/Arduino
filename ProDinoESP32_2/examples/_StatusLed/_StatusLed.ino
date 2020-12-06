/*
    Name:       StatusLed.ino
    Created:	12/1/2020 4:19:55 PM
    Author:     Dimitar Antonov
*/


#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

///////////////////////////	  | ETH  |  3G  | LoRa |LoRaRFM| LED |
const BoardConfig_t MyBoard = { false, false, false, false ,  3   };

// The setup() function runs once each time the micro-controller starts
void setup()
{
	KMPProDinoESP32.begin(MyBoard);
}

// Add the main program code into the continuous loop() function
void loop()
{
	KMPProDinoESP32.processStatusLed(red, 500, 0);
	KMPProDinoESP32.processStatusLed(green, 700, 1);
	KMPProDinoESP32.processStatusLed(blue, 900, 2);
}
