#include "relay.hpp"

#ifdef MODULE_RELAY

#ifdef RELAY_IS_BISTABLE
uint32_t nextRelayTurnOffTick = 0;
#endif
extern uint32_t tick;
uint32_t nextRelayCheckTick = 0;
bool relayTheoreticalState = false; /** true: is close, false: is open */
uint8_t checkCountBeforeError = RELAY_CHECK_BEFORE_ERROR;  /** Soft will try to re-send the command n times before declaring error */

void relay_set_state(bool isClose)
{
    #ifdef RELAY_IS_BISTABLE
        digitalWrite((isClose) ? RELAY_CMD_PIN_1 : RELAY_CMD_PIN_2, HIGH);
        nextRelayTurnOffTick = tick + RELAY_BISTABLE_ON_TIME_MS;
    #else
        digitalWrite(RELAY_CMD_PIN, (isClose) ? HIGH : LOW);
    #endif
    relayTheoreticalState = isClose;
}

bool relay_get_state()
{
    return digitalRead(RELAY_FEEDBACK_PIN) ? false : true;
}

void relay_init(void)
{
#ifdef RELAY_IS_BISTABLE
    pinMode(RELAY_CMD_PIN_1, OUTPUT);
    pinMode(RELAY_CMD_PIN_2, OUTPUT);
#else
    pinMode(RELAY_CMD_PIN, OUTPUT);
#endif
    pinMode(RELAY_FEEDBACK_PIN, INPUT);

    // Set relay open
    relay_set_state(false);
}

void relay_main(void)
{
#ifdef RELAY_IS_BISTABLE
    if (tick > nextRelayTurnOffTick) {
        nextRelayTurnOffTick = 0;
        digitalWrite(RELAY_CMD_PIN_1, LOW);
        digitalWrite(RELAY_CMD_PIN_2, LOW);
    }
#endif

    if (tick > nextRelayCheckTick) {
        nextRelayCheckTick = tick + 60*1000; // This period check is not changeable
        
        // Does the relay have the right state ?
        if (relay_get_state() != relayTheoreticalState) {
            if (checkCountBeforeError <= 0) {
                _set(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT);
            } else {
                // Try to resend command
                relay_set_state(relayTheoreticalState);
                --checkCountBeforeError;
            }
        } else {
            _unset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT);
            // Reset error counter
            checkCountBeforeError = RELAY_CHECK_BEFORE_ERROR;
        }
    }
}

#endif /** MODULE_RELAY */