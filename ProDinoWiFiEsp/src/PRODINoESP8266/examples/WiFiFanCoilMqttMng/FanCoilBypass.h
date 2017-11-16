// FanCoilBypass.h

#ifndef _FANCOILBYPASS_h
#define _FANCOILBYPASS_h

#include "Arduino.h"
#include "FanCoilHelper.h"
#include <KMPDinoWiFiESP.h>

#define BYPASS_CHANGE_STATE_INTERVAL_MS 10000
#define BYPASS_ON_MIN_ANTI_FREEZE_TEMPERTURE 7.0
#define BYPASS_OFF_MIN_ANTI_FREEZE_TEMPERTURE 10.0
#define BYPASS_OFF_TEMPERTURE_DIFFERENCE -1.0
#define BYPASS_ON_TEMPERTURE_DIFFERENCE -0.2
#define BYPASS_OFF_PIN 0x00 // IN1PIN
#define BYPASS_ON_PIN 0x01  // IN2PIN

typedef void(* callBackPublishData) (DeviceData deviceData, bool sendCurrent);

class FanCoilBypassClass : private KMPDinoWiFiESPClass
{
private:
	DeviceState _bypassState = Off;
	void setBypassPin(DeviceState state, bool isEnable);
public:
	void init(callBackPublishData publishData);

	DeviceState state();

	void setBypassState(DeviceState state, bool forceState = false);
	void processByPassState();
};

extern FanCoilBypassClass FanCoilBypass;

#endif

