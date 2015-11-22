// HtmlPages.h
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Project:
//		ProDino Watchdog.
// Description:
//		Build html responses.
// Version: 1.0.0
// Date: 10.12.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#ifndef _HTMLPAGES_h
#define _HTMLPAGES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <EthernetClient.h>
#include "Structures.h"

class HtmlPages
{
 private:
	

 public:
	void init(EthernetClient& ec);
	
	void index();
	void notFound();
    void unAuthorized();
    void privateSettings(DogSetting* dogData, ValidationData* validData = NULL, uint8_t showError = 0, uint8_t showSuccess = 0);
    void privateUserAndIP(DeviceSetting& deviceSettings, ValidationData* validData = NULL, uint8_t showError = 0, uint8_t showSuccess = 0);
};

#endif

