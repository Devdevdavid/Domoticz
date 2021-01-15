/**
  * @file   relay.cpp
  * @brief  Control a relay
  * @author David DEVANT
  * @date   11/06/2020
  */

#include "relay.hpp"
#include "global.hpp"
#include "io/outputs.hpp"
#include "script/script.hpp"

#ifdef MODULE_RELAY

extern uint32_t tick;
uint32_t        nextRelayToggleTick   = UINT32_MAX;
uint32_t        nextRelayCheckTick    = 0;
bool            relayTheoreticalState = false;                    /** true: is close, false: is open */
uint8_t         checkCountBeforeError = RELAY_CHECK_BEFORE_ERROR; /** Soft will try to re-send the command n times before declaring error */

void relay_set_state(bool isClose)
{
#ifdef RELAY_IS_BISTABLE
	uint8_t alias = (isClose) ? RELAY_CMD_1_ALIAS : RELAY_CMD_2_ALIAS;

	output_set(alias, true);
	output_delayed_set(alias, false, RELAY_BISTABLE_ON_TIME_MS);
#else
	output_set(RELAY_CMD_ALIAS, isClose);
#endif
	relayTheoreticalState = isClose;

	// Force to check the relay in 1 sec
	nextRelayCheckTick = tick + 1000;
}

bool relay_get_theoretical_state(void)
{
	return relayTheoreticalState;
}

bool relay_get_state(void)
{
	return digitalRead(RELAY_FEEDBACK_PIN) ? false : true;
}

bool relay_toogle_state(void)
{
	relay_set_state(!relayTheoreticalState);

	// Return new state
	return relayTheoreticalState;
}

void relay_set_toogle_timeout(uint32_t timeoutMs)
{
	nextRelayToggleTick = tick + timeoutMs;
}

int relay_init(void)
{
	pinMode(RELAY_FEEDBACK_PIN, INPUT);

	// Set relay open
	relay_set_state(false);

	return 0;
}

void relay_main(void)
{
	// When timeout expires, toggle the relay state by using theorical state
	if (tick > nextRelayToggleTick) {
		nextRelayToggleTick = UINT32_MAX;
		relay_toogle_state();
	}

	if (tick > nextRelayCheckTick) {
		// Does the relay have the right state ?
		if (relay_get_state() != relayTheoreticalState) {
			// Do nothing if fault is already declared
			if (_isunset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT)) {
				// Try to resend command
				relay_set_state(relayTheoreticalState);

				if (checkCountBeforeError > 0) {
					--checkCountBeforeError;
					log_warn("Bad feedback for relay %d/%d", RELAY_CHECK_BEFORE_ERROR - checkCountBeforeError, RELAY_CHECK_BEFORE_ERROR);
				}

				if (checkCountBeforeError == 0) {
					_set(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT);
					log_error("Relay seems to be broken (Bad feedback)");

					// Inform script module of the event
					script_relay_feedback_event();
				}
			}
		} else {
			// Do nothing if fault is already declared
			if (_isset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT)) {
				_unset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT);

				// Inform script module of the event
				script_relay_feedback_event();
			}

			// Reset error counter
			checkCountBeforeError = RELAY_CHECK_BEFORE_ERROR;
		}
		// EasyFix: Exceptional case, this has to be after relay_set_state() because
		// it overwrites the variable "nextRelayCheckTick" and we don't want that here
		nextRelayCheckTick = tick + RELAY_CHECK_PERIOD;
	}
}

#endif /** MODULE_RELAY */