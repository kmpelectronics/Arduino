// 
// 
// 

#include "FanCoilBypass.h"

DeviceState _bypassNewState = Off;
bool _bypassStateIsChanging = false;
unsigned long _bypassChangeStateInterval;
callBackPublishData _publishData;

void FanCoilBypassClass::init(callBackPublishData publishData)
{
	_publishData = publishData;

	ExpanderSetDirection(BYPASS_OFF_PIN, OUTPUT);
	ExpanderSetDirection(BYPASS_ON_PIN, OUTPUT);
}

DeviceState FanCoilBypassClass::state()
{
	return _bypassState;
}

void FanCoilBypassClass::setBypassPin(DeviceState state, bool isEnable)
{
	if (state == On)
	{
		ExpanderSetPin(BYPASS_ON_PIN, isEnable);
	}
	else
	{
		ExpanderSetPin(BYPASS_OFF_PIN, isEnable);
	}
}

/**
* @brief Set bypass new state.
* @param state new state of the bypass
* @desc if room temperature is higher than desired (+ 1 BYPASS_OFF_TEMPERTURE_DIFFERENCE) Off,
*         is close (+ 0,2 BYPASS_ON_TEMPERTURE_DIFFERENCE) to desired or below On
*         Antifreeze On: if room temperature below 7 (BYPASS_ON_MIN_ANTI_FREEZE_TEMPERTURE) degrees.
*         Switch fun-coil Off -> and bypass Off. Only antifreeze functionality will be working.
*
* @return void
**/
void FanCoilBypassClass::setBypassState(DeviceState state, bool forceState)
{
	if (!forceState)
	{
		if (state == _bypassState || _bypassStateIsChanging)
		{
			return;
		}
	}

	// Start bypass state changing
	_bypassNewState = state;

	_bypassStateIsChanging = true;
	_bypassChangeStateInterval = millis() + BYPASS_CHANGE_STATE_INTERVAL_MS;

	setBypassPin(_bypassNewState, true);
}

void FanCoilBypassClass::processByPassState()
{
	if (!_bypassStateIsChanging)
	{
		return;
	}

	// End bypass state changing.
	if (_bypassChangeStateInterval < millis())
	{
		_bypassStateIsChanging = false;
		_bypassState = _bypassNewState;

		setBypassPin(_bypassNewState, false);
		if (_publishData != NULL)
		{
			_publishData(BypassState, false);
		}
	}
}

FanCoilBypassClass FanCoilBypass;