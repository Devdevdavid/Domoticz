#include "script/script.hpp"
#include "temp/temp.hpp"
#include "relay/relay.hpp"
#include "domoticz/domoticz.hpp"
#include "io/inputs.hpp"
#include "io/outputs.hpp"
#include "cmd/cmd.hpp"

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
#ifdef BOARD_TEMP_DOMOTICZ
    if (tick > nextDomoticzUpdateTick) {
        nextTempCheckTick = tick + SCRIPT_DOMOTICZ_UPT_PERIOD;

        // Send both sensor values
        domoticz_send_temperature(DOMOTICZ_SENSOR_ID_INSIDE, temp_get_value(DEVICE_INDEX_0));
        domoticz_send_temperature(DOMOTICZ_SENSOR_ID_OUTSIDE, temp_get_value(DEVICE_INDEX_1));
    }
#endif

#ifdef BOARD_TEMP_DOMOTICZ
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
#ifdef BOARD_TEMP_DOMOTICZ_RELAY
            // Are we in impulsion mode (Jumper set) ? (OPT jumper JP1)
            if (is_input_low(INPUTS_OPT_RELAY_IMPULSION_MODE)) {
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
#ifdef BOARD_TEMP_DOMOTICZ_BUZZER
            output_set(OUTPUTS_BUZZER, isInAlertOld);
#endif
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

#if defined(BOARD_RING) && (DETECTOR_ENABLED != 0)
    static uint32_t detectorEndTick = 0;
    /** PIR Detector */
    #if (DETECTOR_INVERSE_POLARITY == 0)
    if (is_input_rising(INPUTS_PIR_DETECTOR)) {
        reset_input_rising(INPUTS_PIR_DETECTOR);
    #else
    if (is_input_falling(INPUTS_PIR_DETECTOR)) {
        reset_input_falling(INPUTS_PIR_DETECTOR);
    #endif
        // Detection ! Turn on the strip
        detectorEndTick = tick + (DETECTOR_ON_DURATION_MIN * 60 * 1000);
        cmd_set_state(true);
    }

    // Detector duration end
    if (tick > detectorEndTick) {
        cmd_set_state(false);
        detectorEndTick = 0;
    }
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