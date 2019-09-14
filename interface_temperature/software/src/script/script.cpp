#include "script/script.hpp"
#include "temp/temp.hpp"
#include "relay/relay.hpp"
#include "domoticz/domoticz.hpp"

extern uint32_t tick;
uint32_t nextTempCheckTick = 0;
uint32_t nextDomoticzUpdateTick = 0;

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

#ifdef MODULE_RELAY
        relay_set_state(isInAlert);
#endif
#ifdef MODULE_BUZZER
        buzzer_set_state(isInAlert);
#endif
    }
#endif
}