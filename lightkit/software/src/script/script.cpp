/**
  * @file   script.cpp
  * @brief  Interconnection point of all modules
  * @author David DEVANT
  * @date   12/08/2019
  */

#include "script/script.hpp"
#include "buzzer/buzzer.hpp"
#include "cmd/cmd.hpp"
#include "domoticz/domoticz.hpp"
#include "feu_rouge/feu_rouge.h"
#include "io/inputs.hpp"
#include "io/outputs.hpp"
#include "relay/relay.hpp"
#include "telegram/telegram.hpp"
#include "temp/temp.hpp"

extern uint32_t tick;
uint32_t        nextResetTick             = 0;
uint32_t        nextTempCheckTick         = 0;
uint32_t        nextDomoticzUpdateTick    = 0;
uint32_t        nextSecondRelayImpulsTick = UINT32_MAX; // Disabled at startup
uint32_t        nextBuzzerPulseTick       = UINT32_MAX; // Disabled at startup
bool            isInAlertOld              = false;
bool            isAutoTempMsgEnabled      = false;

#if defined(MODULE_TEMPERATURE) && defined(MODULE_TELEGRAM)
uint32_t nextTelegramUpdateTick = SCRIPT_TELEGRAM_UPT_PERIOD; // Skip first call
#endif

#if defined(MODULE_TELEGRAM) && (SCRIPT_TELEGRAM_CONN_OK_NOTIFY_PERIOD > 0)
uint32_t nextTelegramConnOkNotify = SCRIPT_TELEGRAM_CONN_OK_NOTIFY_PERIOD; // Skip first call
#endif

// CONSTANTS
#if defined(SCRIPT_TEMP_ALERT_METHOD) && (SCRIPT_TEMP_ALERT_METHOD == METHOD_THRESHOLD)
// Using extern and const together: https://stackoverflow.com/a/2190981
extern const float sensorThreshold[TEMP_MAX_SENSOR_SUPPORTED];
const float        sensorThreshold[TEMP_MAX_SENSOR_SUPPORTED] = {
    SCRIPT_TEMP_ALERT_SENSOR_0,
    SCRIPT_TEMP_ALERT_SENSOR_1
};
const float sensorThresholdHigh[TEMP_MAX_SENSOR_SUPPORTED] = {
	SCRIPT_TEMP_ALERT_SENSOR_0 + SCRIPT_TEMP_ALERT_HYSTERESIS,
	SCRIPT_TEMP_ALERT_SENSOR_1 + SCRIPT_TEMP_ALERT_HYSTERESIS
};
const float sensorThresholdLow[TEMP_MAX_SENSOR_SUPPORTED] = {
	SCRIPT_TEMP_ALERT_SENSOR_0 - SCRIPT_TEMP_ALERT_HYSTERESIS,
	SCRIPT_TEMP_ALERT_SENSOR_1 - SCRIPT_TEMP_ALERT_HYSTERESIS
};
#endif

/***************************************
            STATIC FUNCTIONS
 ***************************************/

#if defined(MODULE_RELAY)
/**
 * @brief Set the relay ON then turn it OFF after impulseDurationMs milliseconds
 *
 * @param impulseDurationMs
 */
static void script_send_relay_impulse(uint32_t impulseDurationMs)
{
	// Set relay to ON
	relay_set_state(true);
	// Turn it OFF after impulseDurationMs milliseconds
	relay_set_toogle_timeout(impulseDurationMs);
}
#endif

/***************************************
                FUNCTIONS
 ***************************************/

/**
 * @brief Main function of script module
 */
void script_main(void)
{
#if defined(MODULE_TEMPERATURE) && defined(MODULE_DOMOTICZ)
	if ((isAutoTempMsgEnabled == true) && (tick > nextDomoticzUpdateTick)) {
		nextDomoticzUpdateTick = tick + SCRIPT_DOMOTICZ_UPT_PERIOD;

		// Send all sensor values
		for (byte i = 0; i < temp_get_nb_sensor(); ++i) {
			uint8_t sensorId = (i == DEVICE_INDEX_0) ? DOMOTICZ_SENSOR_ID_INSIDE : DOMOTICZ_SENSOR_ID_OUTSIDE;
			domoticz_send_temperature(sensorId, temp_get_value(i));
		}
	}
#endif

#if defined(MODULE_TEMPERATURE) && defined(MODULE_TELEGRAM)
	if ((isAutoTempMsgEnabled == true) && (tick > nextTelegramUpdateTick)) {
		nextTelegramUpdateTick = tick + SCRIPT_TELEGRAM_UPT_PERIOD;

		// Send all sensor values
		for (byte i = 0; i < temp_get_nb_sensor(); ++i) {
			telegram_send_msg_temperature(i, temp_get_value(i));
		}
	}
#endif

#if defined(BOARD_TEMP_TELEGRAM)
	/** Send a Telegram message when OPT just changed
     *
     *  Note: Here we use INPUTS_OPT_TEMP_ALARM_EN define
     *  whatever the version BUZZER or RELAY
     */
	if (is_input_falling(INPUTS_OPT_TEMP_ALARM_EN)) {
		reset_input_falling(INPUTS_OPT_TEMP_ALARM_EN);
		telegram_send_opt_changed(true);
		nextTempCheckTick = 0; // Force update of temperature
	} else if (is_input_rising(INPUTS_OPT_TEMP_ALARM_EN)) {
		reset_input_rising(INPUTS_OPT_TEMP_ALARM_EN);
		telegram_send_opt_changed(false);
		nextTempCheckTick = 0; // Force update of temperature
	}
#endif

	// Temperature alarm
#if defined(BOARD_TEMP_DOMOTICZ) || defined(BOARD_TEMP_TELEGRAM)
	if (tick > nextTempCheckTick) {
		nextTempCheckTick = tick + SCRIPT_TEMP_CHECK_PERIOD;

		bool isTempAlarmDisabled = false;

#if defined(BOARD_TEMP_TELEGRAM)
		/** Force alarm off if OPT is enabled
         *  Switch is ON when signal is LOW */
		isTempAlarmDisabled = is_input_low(INPUTS_OPT_TEMP_ALARM_EN);
#endif

#if (SCRIPT_TEMP_ALERT_METHOD == METHOD_THRESHOLD)
		bool atLeastOneIsAbove = false;
		bool allAreBelow       = true;

		// For each sensor
		for (int i = 0; i < temp_get_nb_sensor(); ++i) {
			float sensorTemp = temp_get_value(i);

			// Ignore faulty sensors
			if (_isset(STATUS_TEMP, STATUS_TEMP_1_FAULT << i)) {
				continue;
			}

			if (sensorTemp >= sensorThresholdHigh[i]) {
				atLeastOneIsAbove = true;
			}
			if (sensorTemp >= sensorThresholdLow[i]) {
				allAreBelow = false;
			}
		}

		// Define the new state of the alert
		if (isTempAlarmDisabled) {
			_unset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
		} else if (atLeastOneIsAbove) {
			_set(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
		} else if (allAreBelow) {
			_unset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
		}
#elif (SCRIPT_TEMP_ALERT_METHOD == METHOD_DIFFERENTIAL)

		if (isTempAlarmDisabled) {
			// Force Off
			_unset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
		} else if (temp_get_nb_sensor() < 2) {
			// We need at least 2 sensors to make a difference
		} else if (_isset(STATUS_TEMP, STATUS_TEMP_1_FAULT | STATUS_TEMP_2_FAULT)) {
			// We need both sensor fully operationnal to continue
		} else {
			// Take the absolute value of the difference
			float diffTemp = fabs(temp_get_value(DEVICE_INDEX_1) - temp_get_value(DEVICE_INDEX_0));

			if (diffTemp >= (SCRIPT_TEMP_ALERT_DIFF_THRESHOLD + SCRIPT_TEMP_ALERT_HYSTERESIS)) {
				_set(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
			} else if (diffTemp <= (SCRIPT_TEMP_ALERT_DIFF_THRESHOLD - SCRIPT_TEMP_ALERT_HYSTERESIS)) {
				_unset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
			}
		}
#endif

		// Detect edges
		if (isInAlertOld != _isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT)) {
			isInAlertOld = _isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT);
			log_error("Script alert is now %s", isInAlertOld ? "active" : "inactive");

#ifdef MODULE_TELEGRAM
			// Tell telegram client that alert level changed
			telegram_send_alert(isInAlertOld);
#endif

#if defined(MODULE_RELAY)
			script_relay_set_event(isInAlertOld);
#endif

#if defined(BOARD_TEMP_DOMOTICZ_BUZZER) || defined(BOARD_TEMP_TELEGRAM_BUZZER)
			// Are we configured in impulsion mode ?
			if (is_input_low(INPUTS_OPT_ALARM_IMPULSION_MODE_EN)) {
				// Either stop pulse (UINT32_MAX) or start now (0)
				nextBuzzerPulseTick = isInAlertOld ? 0 : UINT32_MAX;
			} else {
				// Start/Stop the buzzer in continuous mode
				output_set(OUTPUTS_BUZZER, isInAlertOld);
			}
#endif
		}
	}

#if defined(MODULE_TELEGRAM) && (SCRIPT_TELEGRAM_CONN_OK_NOTIFY_PERIOD > 0)
	// Send a brief message to say connection is OK
	if (tick > nextTelegramConnOkNotify) {
		nextTelegramConnOkNotify = tick + SCRIPT_TELEGRAM_CONN_OK_NOTIFY_PERIOD;
		telegram_send_conn_ok();
	}
#endif

#if defined(MODULE_RELAY) && (SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION != 0)
	// This is the 2nd relay impulsion callback (Activated only when 2nd impulsion duration is != 0)
	if (tick > nextSecondRelayImpulsTick) {
		nextSecondRelayImpulsTick = UINT32_MAX; // Disable second relay impulse
		script_send_relay_impulse(SCRIPT_RELAY_IMPULSION_DURATION);
	}
#endif

#if defined(BOARD_TEMP_DOMOTICZ_BUZZER) || defined(BOARD_TEMP_TELEGRAM_BUZZER)
	// Manage the pulses of buzzer
	// It triggers every pulse period (ON + OFF period)
	// until nextBuzzerPulseTick is set to UINT32_MAX
	if (tick > nextBuzzerPulseTick) {
		// Prepare next pulse
		nextBuzzerPulseTick = tick + SCRIPT_BUZZER_PULSE_ON_MS + SCRIPT_BUZZER_PULSE_OFF_MS;

		// Start pulse now and define when ON state stops (after SCRIPT_BUZZER_PULSE_ON_MS ms)
		output_set(OUTPUTS_BUZZER, true);
		output_delayed_set(OUTPUTS_BUZZER, true, SCRIPT_BUZZER_PULSE_ON_MS);
	}
#endif
#endif // End of BOARD_TEMP_DOMOTICZ || BOARD_TEMP_TELEGRAM

#ifdef BOARD_RING
	static uint32_t detectorEndTick = UINT32_MAX;

	/** Is the jumper set to enable the detector ? */
	if (is_input_low(INPUTS_PIR_DETECTOR_ENABLE)) {
/** PIR Detector */
#if (DETECTOR_INVERSE_POLARITY == 0)
		if (is_input_rising(INPUTS_PIR_DETECTOR)) {
			reset_input_rising(INPUTS_PIR_DETECTOR);
#else
		if (is_input_falling(INPUTS_PIR_DETECTOR)) {
			reset_input_falling(INPUTS_PIR_DETECTOR);
#endif
			// Detection ! Turn on the strip
			uint32_t duration = input_analog_read(INPUTS_PIR_DETECTOR_DELAY);
			// Convert to [0; MAX min - 1 min], 12 bits so 4095
			duration = ((duration * ((DETECTOR_MAX_DURATION_MIN - 1) * 60)) / 4095) * 1000;
			// Convert to [1 min; MAX min]
			duration += 1 * 60 * 1000;
			// Set the moment of closure
			detectorEndTick = tick + duration;

			cmd_set_state(true);
			log_info("Detection triggered ! Delay = %dmin", duration / (1000 * 60));
		}
	}

	// Detector duration end
	if (tick > detectorEndTick) {
		cmd_set_state(false);
		detectorEndTick = UINT32_MAX;
	}
#endif

#ifdef BOARD_FEU_ROUGE
	if (is_input_falling(INPUTS_DOOR_SWITCH)) {
		reset_input_falling(INPUTS_DOOR_SWITCH);
		feu_rouge_mode_fct_door(DOOR_CMD_SOMEONE_COME_IN);
	}
	if (is_input_rising(INPUTS_DOOR_SWITCH)) {
		reset_input_rising(INPUTS_DOOR_SWITCH);
		feu_rouge_mode_fct_door(DOOR_CMD_SOMEONE_COME_OUT);
	}
#endif

	// Manage delayed reset
	if (nextResetTick != 0) {
		if (tick >= nextResetTick) {
			nextResetTick = 0;
			log_warn("Reseting due to delayed reset !");
			ESP.restart();
		}
	}
}

#if defined(MODULE_RELAY)
/**
 * @brief Called when RELAY fault is set or cleared
 */
void script_relay_feedback_event(void)
{
#if defined(MODULE_TELEGRAM)
	bool isOk = _isunset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT);
	telegram_send_msg_relay_feedback(isOk);
#endif
}
#endif

#if defined(MODULE_RELAY)
void script_relay_set_event(bool isClosed)
{
	// Are we in impulsion mode (Jumper set) ?
	if (is_input_low(INPUTS_OPT_ALARM_IMPULSION_MODE_EN)) {
		script_send_relay_impulse(SCRIPT_RELAY_IMPULSION_DURATION);

		// If 2nd impulsion is activated and the alert is going from OFF to ON then
		// prepare to send a 2nd impulsion
#if (SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION != 0)
		if (isClosed) {
			nextSecondRelayImpulsTick = tick + SCRIPT_RELAY_MS_BEFORE_2ND_IMPULSION;
		}
#endif
	} else {
		// No impulsion, just set the relay ON when alert is ON and vice versa
		relay_set_state(isClosed);
	}
}
#endif

/**
 * @brief Set a delayed reset occuring in tickCount ticks
 * @note Use 0 to disable the delayed reset
 */
void script_delayed_reset(uint32_t tickCount)
{
	nextResetTick = tick + tickCount;
}
