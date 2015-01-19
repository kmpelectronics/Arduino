// Processing.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Watchdog.
// Description:
//		Process actions and Html pages.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#ifndef _PROCESSING_H
#define _PROCESSING_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Structures.h"
#include <EthernetClient.h>

class Processing
{
 private:

 public:
	/**
	 * \brief Initialize data. Read from EEPROM.
	 * 
	 * 
	 * \return void
	 */
	void init();
    
    /**
     * \brief Reset settings by default.
     * 
     * 
     * \return void
     */
    void resetSettings();
    
    /**
     * \brief Processes occurring operations.
     * 
     * \return void
     */
    void processOperaions();
	
	/**
	 * \brief Read client request parse and write needed response.
	 * 
	 * \param client Ethernet client.
	 * 
	 * \return void
	 */
	void processRequest(EthernetClient& client);
    
    /**
     * \brief Check is addresses (IP, Subnet, Gateway) changed.
     * 
     * 
     * \return bool If result equal True - addresses changed, else no change.
     */
    bool IsAddressesChanged();
    
    /**
     * \brief Set value to AddressesChanged.
     * 
     * \param b New value.
     * 
     * \return void
     */
    void SetAddressesChanged(bool b);
};

#endif