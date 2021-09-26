#include "feu_rouge.h"
#include "tools/tools.hpp"

#ifdef MODULE_FEU_ROUGE

// Functionning mode
static FEU_ROUGE_MODE_FCT_E curModeFct;
static trafic_light_data_t  traficLightData;
static door_data_t          doorData;

extern uint32_t tick;

// CONFIG
#define SETTING_DOOR_CLEANNING_DURATION  (5 * TIME_SECOND)
#define SETTING_DOOR_MAX_INSIDE_DURATION (10 * TIME_SECOND)

/**
 * @brief Define the color of the traffic light
 *
 * @param color See COLOR_MASK_*
 */
static void feu_rouge_set_color(uint32_t color)
{
	static uint32_t prevColor = UINT32_MAX;

	if (color == prevColor) {
		return; // nothing to do
	}

	prevColor = color;

	output_set(OUTPUTS_TRAFFIC_LIGHT_RED, _isset(color, COLOR_MASK_RED));
	output_set(OUTPUTS_TRAFFIC_LIGHT_YELLOW, _isset(color, COLOR_MASK_YELLOW));
	output_set(OUTPUTS_TRAFFIC_LIGHT_GREEN, _isset(color, COLOR_MASK_GREEN));
}

void feu_rouge_mode_fct_trafic_light(TRAFIC_LIGHT_CMD_E cmd)
{
	// Detect changes of mode
	if (curModeFct != MODE_FCT_TRAFFIC_LIGHT) {
		curModeFct                          = MODE_FCT_TRAFFIC_LIGHT;
		traficLightData.prevState           = TRAFIC_LIGHT_STATE_NONE;
		traficLightData.curState            = TRAFIC_LIGHT_STATE_HS;
		traficLightData.tickLastStateChange = tick;
	}

	traficLightData.curCmd = cmd;
}

void feu_rouge_mode_fct_door(DOOR_CMD_E cmd)
{
	// Detect changes of mode
	if (curModeFct != MODE_FCT_DOOR) {
		curModeFct                   = MODE_FCT_DOOR;
		doorData.prevState           = DOOR_STATE_NONE;
		doorData.curState            = DOOR_STATE_WAIT_SOMEONE;
		doorData.tickLastStateChange = tick;
	}

	doorData.curCmd = cmd;
}

static void run_state_machine_trafic_light(void)
{
	// Manage state change
	switch (traficLightData.curState) {
	case TRAFIC_LIGHT_STATE_OPENED:
		if (traficLightData.curCmd == TRAFIC_LIGHT_CMD_CLOSE) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_CLOSING;
		}
		break;
	case TRAFIC_LIGHT_STATE_CLOSING:
		if ((tick - traficLightData.tickLastStateChange) >= 3000) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_CLOSED;
		}
		break;
	case TRAFIC_LIGHT_STATE_CLOSED:
		if (traficLightData.curCmd == TRAFIC_LIGHT_CMD_OPEN) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_OPENED;
		}
		break;
	case TRAFIC_LIGHT_STATE_HS_ON:
		if ((tick - traficLightData.tickLastStateChange) >= 500) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_HS;
		}
		break;
	case TRAFIC_LIGHT_STATE_HS:
		if ((tick - traficLightData.tickLastStateChange) >= 500) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_HS_ON;
		} else if (traficLightData.curCmd == TRAFIC_LIGHT_CMD_OPEN) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_OPENED;
		} else if (traficLightData.curCmd == TRAFIC_LIGHT_CMD_CLOSE) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_CLOSING;
		}
		break;
	case TRAFIC_LIGHT_STATE_NONE:
	default:
		traficLightData.curState = TRAFIC_LIGHT_STATE_HS;
		break;
	}

	// HS cmd has priority over all
	if (traficLightData.curCmd == TRAFIC_LIGHT_CMD_HS) {
		if ((traficLightData.curState != TRAFIC_LIGHT_STATE_HS) && (traficLightData.curState != TRAFIC_LIGHT_STATE_HS_ON)) {
			traficLightData.curState = TRAFIC_LIGHT_STATE_HS;
		}
	}

	// We managed this command
	traficLightData.curCmd = TRAFIC_LIGHT_CMD_NONE;

	// Detect new state
	if (traficLightData.prevState != traficLightData.curState) {
		traficLightData.prevState           = traficLightData.curState;
		traficLightData.tickLastStateChange = tick;

		// Apply state change
		switch (traficLightData.curState) {
		case TRAFIC_LIGHT_STATE_OPENED:
			feu_rouge_set_color(COLOR_MASK_GREEN);
			break;
		case TRAFIC_LIGHT_STATE_CLOSING:
			feu_rouge_set_color(COLOR_MASK_YELLOW);
			break;
		case TRAFIC_LIGHT_STATE_CLOSED:
			feu_rouge_set_color(COLOR_MASK_RED);
			break;
		case TRAFIC_LIGHT_STATE_HS_ON:
			feu_rouge_set_color(COLOR_MASK_YELLOW);
			break;
		case TRAFIC_LIGHT_STATE_HS:
		default:
			feu_rouge_set_color(COLOR_MASK_BLACK);
			break;
		}
	}
}

static void run_state_machine_door(void)
{
	// Manage state change
	switch (doorData.curState) {
	case DOOR_STATE_WAIT_SOMEONE:
		if (doorData.curCmd == DOOR_CMD_SOMEONE_COME_IN) {
			doorData.curState = DOOR_STATE_PEOPLE_INSIDE;
		}
		break;
	case DOOR_STATE_PEOPLE_INSIDE:
		if (doorData.curCmd == DOOR_CMD_SOMEONE_COME_OUT) {
			doorData.curState = DOOR_STATE_CLEANNING;
		} else if ((tick - doorData.tickLastStateChange) >= SETTING_DOOR_MAX_INSIDE_DURATION) {
			// Abort this state, it means previous guy closes the door after leaving and no one is inside
			doorData.curState = DOOR_STATE_WAIT_SOMEONE;
		}
		break;
	case DOOR_STATE_CLEANNING:
		if (doorData.curCmd == DOOR_CMD_SOMEONE_COME_IN) {
			doorData.curState = DOOR_STATE_PEOPLE_INSIDE;
		} else if ((tick - doorData.tickLastStateChange) >= SETTING_DOOR_CLEANNING_DURATION) {
			doorData.curState = DOOR_STATE_WAIT_SOMEONE;
		}
		break;
	case DOOR_STATE_NONE:
	default:
		doorData.curState = DOOR_STATE_WAIT_SOMEONE;
		break;
	}

	// We managed this command
	doorData.curCmd = DOOR_CMD_NONE;

	// Detect new state
	if (doorData.prevState != doorData.curState) {
		doorData.prevState           = doorData.curState;
		doorData.tickLastStateChange = tick;

		// Apply state change
		switch (doorData.curState) {
		case DOOR_STATE_WAIT_SOMEONE:
			feu_rouge_set_color(COLOR_MASK_GREEN);
			break;
		case DOOR_STATE_PEOPLE_INSIDE:
			feu_rouge_set_color(COLOR_MASK_RED);
			break;
		case DOOR_STATE_CLEANNING:
			feu_rouge_set_color(COLOR_MASK_YELLOW);
			break;
		case DOOR_STATE_NONE:
		default:
			feu_rouge_set_color(COLOR_MASK_BLACK);
			break;
		}
	}
}

int feu_rouge_init(void)
{
	// Set initial functionning mode
	switch (FEU_ROUGE_INIT_FCT_MODE) {
	case MODE_FCT_TRAFFIC_LIGHT:
		feu_rouge_mode_fct_trafic_light(TRAFIC_LIGHT_CMD_NONE);
		break;
	case MODE_FCT_DOOR:
		feu_rouge_mode_fct_door(DOOR_CMD_NONE);
		break;
	case MODE_FCT_NONE:
	default:
		curModeFct = FEU_ROUGE_INIT_FCT_MODE;
		break;
	}
	return 0;
}

void feu_rouge_main(void)
{
	// Manage mode
	switch (curModeFct) {
	case MODE_FCT_TRAFFIC_LIGHT:
		run_state_machine_trafic_light();
		break;
	case MODE_FCT_DOOR:
		run_state_machine_door();
		break;
	case MODE_FCT_NONE:
	default:
		feu_rouge_set_color(COLOR_MASK_BLACK);
		break;
	}
}

#endif // MODULE_FEU_ROUGE