#include "script/script.hpp"
#include "temp/temp.hpp"
#include "relay/relay.hpp"
#include "domoticz/domoticz.hpp"

extern uint32_t tick;
uint32_t nextTempCheckTick = 0;
uint32_t nextDomoticzUpdateTick = 0;
uint32_t nextSecondRelayImpulsTick = UINT32_MAX; // Disabled at startup
bool isInAlertOld = false;

// Sub script function prototypes
#ifdef MODULE_RELAY
void script_send_relay_impulse(uint32_t impulseDurationMs);
#endif

void script_execute(void)
{
#if defined(MODULE_DOMOTICZ) && defined(MODULE_TEMPERATURE)
    if (tick > nextDomoticzUpdateTick) {
        nextTempCheckTick = tick + SCRIPT_DOMOTICZ_UPT_PERIOD;

        // Send both sensor values
        domoticz_send_temperature(DOMOTICZ_SENSOR_ID_INSIDE, temp_get_value(DEVICE_INDEX_0));
        domoticz_send_temperature(DOMOTICZ_SENSOR_ID_OUTSIDE, temp_get_value(DEVICE_INDEX_1));
    }
#endif
    // This is only to test buttons 
    if (_isset(STATUS_BUTTON, STATUS_BUTTON_RISING)) {
        _unset(STATUS_BUTTON, STATUS_BUTTON_RISING);
        log_info("Rising edge");
    }
    if (_isset(STATUS_BUTTON, STATUS_BUTTON_FALLING)) {
        _unset(STATUS_BUTTON, STATUS_BUTTON_FALLING);
        log_info("Falling edge");
    }

#ifdef MODULE_TEMPERATURE
    if (tick > nextTempCheckTick) {
        nextTempCheckTick = tick + SCRIPT_TEMP_CHECK_PERIOD;
        float fridgeTemp = temp_get_value(DEVICE_INDEX_0);
        float frezzerTemp = temp_get_value(DEVICE_INDEX_1);

        if (_isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT)) {
            if ((fridgeTemp < (SCRIPT_TEMP_ALERT_FRIDGE - SCRIPT_TEMP_ALERT_HYSTERESIS))
                && (frezzerTemp < (SCRIPT_TEMP_ALERT_FREEZER - SCRIPT_TEMP_ALERT_HYSTERESIS))) {
                _unset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
            }
        } else {
            if ((fridgeTemp > (SCRIPT_TEMP_ALERT_FRIDGE + SCRIPT_TEMP_ALERT_HYSTERESIS))
                || (frezzerTemp > (SCRIPT_TEMP_ALERT_FREEZER + SCRIPT_TEMP_ALERT_HYSTERESIS))) {
                _set(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
            }
        }

        // Detect edges
        if (isInAlertOld != _isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT)) {
            isInAlertOld = _isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
            log_error("Script alert is now %s", isInAlertOld ? "active" : "inactive");
#ifdef MODULE_RELAY
            // Are we in impulsion mode ? (OPT jumper)
            if (_isset(STATUS_BUTTON, STATUS_BUTTON_IS_PRESSED)) {
                script_send_relay_impulse(SCRIPT_RELAY_IMPULSION_DURATION);

                // If 2nd impulsion is activated and the alert is going from OFF to ON then
                // prepare to send a 2nd impulsion
#if (SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION != 0)
                if (isInAlertOld) {
                    nextSecondRelayImpulsTick = tick + SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION;
                }
#endif
            } else {
                // No impulsion, just set the relay ON when alert is ON and vice versa
                relay_set_state(isInAlertOld);
            }
#endif
#ifdef MODULE_BUZZER
            buzzer_set_state(isInAlertOld);
#endif
        }
    }

    // This is the 2nd relay impulsion callback (Activated only when 2nd impulsion duration is > 0)
#if defined(MODULE_RELAY) && (SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION != 0)
    if (tick > nextSecondRelayImpulsTick) {
        nextSecondRelayImpulsTick = UINT32_MAX; // Disable second relay impulse
        script_send_relay_impulse(SCRIPT_RELAY_IMPULSION_DURATION);
    }
#endif
#endif
}

/**
 * @brief Set the relay ON then turn it OFF after impulseDurationMs milliseconds
 * 
 * @param impulseDurationMs 
 */
#ifdef MODULE_RELAY
void script_send_relay_impulse(uint32_t impulseDurationMs)
{
    // Set relay to ON
    relay_set_state(true);
    // Turn it OFF after impulseDurationMs milliseconds
    relay_set_toogle_timeout(impulseDurationMs);
}
#endif