#include "feu_rouge.h"

#ifdef MODULE_FEU_ROUGE

const uint8_t piaAddr[] = {
	IOI2C_0_ADDR
};

// Map color to outputs
#define PIA_OUTPUT_RED    0
#define PIA_OUTPUT_YELLOW 1
#define PIA_OUTPUT_GREEN  2

static IOI2CGroup        pia(IOI2C_SDA, IOI2C_SCL, piaAddr, sizeof(piaAddr));
static FEU_ROUGE_STATE_E prevState;
static FEU_ROUGE_STATE_E curState;
static FEU_ROUGE_CMD_E   curCmd;
static uint32_t          prevColor = UINT32_MAX;

static uint32_t tickLastStateChange = 0;

extern uint32_t tick;

/**
 * @brief Define the color of the traffic light
 *
 * @param color See COLOR_MASK_*
 */
void feu_rouge_set_color(uint32_t color)
{
	if (color == prevColor) {
		return; // nothing to do
	}

	prevColor = color;

	pia.output_write(PIA_OUTPUT_RED, _isset(color, COLOR_MASK_RED));
	pia.output_write(PIA_OUTPUT_YELLOW, _isset(color, COLOR_MASK_YELLOW));
	pia.output_write(PIA_OUTPUT_GREEN, _isset(color, COLOR_MASK_GREEN));
}

void feu_rouge_command(FEU_ROUGE_CMD_E cmd)
{
	curCmd = cmd;
}

int feu_rouge_init(void)
{
	curCmd   = CMD_HS;
	curState = STATE_HS;

	pia.begin();

	return 0;
}

void feu_rouge_main(void)
{
	// Detect new state
	if (prevState != curState) {
		prevState           = curState;
		tickLastStateChange = tick;
	}

	// Do state
	switch (curState) {
	case STATE_OPENED:
		feu_rouge_set_color(COLOR_MASK_GREEN);
		break;
	case STATE_CLOSING:
		feu_rouge_set_color(COLOR_MASK_YELLOW);
		break;
	case STATE_CLOSED:
		feu_rouge_set_color(COLOR_MASK_RED);
		break;
	case STATE_HS:
	default:
		feu_rouge_set_color(COLOR_MASK_BLACK);
		break;
	}

	// Manage state change
	switch (curState) {
	case STATE_OPENED:
		if (curCmd == CMD_CLOSE) {
			curState = STATE_CLOSING;
		}
		break;
	case STATE_CLOSING:
		if ((tick - tickLastStateChange) >= 3000) {
			curState = STATE_CLOSED;
		}
		break;
	case STATE_CLOSED:
		if (curCmd == CMD_OPEN) {
			curState = STATE_OPENED;
		}
		break;
	case STATE_HS:
	default:
		if (curCmd == CMD_OPEN) {
			curState = STATE_OPENED;
		} else if (curCmd == CMD_CLOSE) {
			curState = STATE_CLOSING;
		}
		break;
	}

	// HS cmd has priority over all
	if (curCmd == CMD_HS) {
		curState = STATE_HS;
	}
}

#endif // MODULE_FEU_ROUGE