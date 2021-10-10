#include "buzzer.hpp"
#include "io/outputs.hpp"

#ifdef MODULE_BUZZER

struct buzzer_t {
	uint32_t   output;
	uint8_t    noteIndex;
	uint32_t   timeout;
	uint32_t * pMelody;
	uint32_t   melodyLength;
	uint8_t    repeat : 1;
	uint8_t    enabled : 1;
};

// MACROS
#define STATE_MASK           0x80000000
#define STATE_ON             0x80000000
#define STATE_OF             0x00000000
#define NOTE(state, delayMs) (state | delayMs)
#define NOTE(state, delayMs) (state | delayMs)

// VARIABLES
extern uint32_t        tick;
static struct buzzer_t buzzerData[BUZZERS_COUNT] = { 0 };

// MELODY - binary
static uint32_t melody1[] = {
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 900),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 800),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 700),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 600),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 500),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 400),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 300),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 200),
	NOTE(STATE_ON, 100),
	NOTE(STATE_OF, 100),
	NOTE(STATE_ON, 1000),
};
static uint32_t melody2[] = {
	NOTE(STATE_ON, 20),
	NOTE(STATE_OF, 20),
};

int buzzer_init(void)
{
	const uint32_t buzzerOutputs[BUZZERS_COUNT] = BUZZERS_OUTPUTS;

	for (int i = 0; i < BUZZERS_COUNT; ++i) {
		buzzerData[i].output = buzzerOutputs[i];
		buzzer_set_melody(i, 0, false);
	}

	return 0;
}

void buzzer_stop(uint8_t buzzerId)
{
	buzzer_set_melody(buzzerId, 0, false);
}

int buzzer_set_melody(uint8_t i, uint8_t melodyId, bool isRepeatEnabled)
{
	if (i > BUZZERS_COUNT) {
		return -1;
	}
	// 0 means stop
	if (melodyId == 0) {
		buzzerData[i].enabled = false;
		output_set(buzzerData[i].output, false);
		return 0;
	}

	// Check melody id
	switch (melodyId) {
	case 1:
		buzzerData[i].pMelody      = melody1;
		buzzerData[i].melodyLength = sizeof(melody1) / sizeof(uint32_t);
		break;
	case 2:
		buzzerData[i].pMelody      = melody2;
		buzzerData[i].melodyLength = sizeof(melody2) / sizeof(uint32_t);
		break;
	default:
		return -1;
	}

	// Start at begin
	buzzerData[i].noteIndex = 0;
	buzzerData[i].repeat    = isRepeatEnabled;
	buzzerData[i].enabled   = true;

	// Start now
	buzzerData[i].timeout = 0;

	return 0;
}

void buzzer_main(void)
{
	uint32_t note;

	for (int i = 0; i < BUZZERS_COUNT; ++i) {
		if (!buzzerData[i].enabled) {
			continue;
		}

		if (tick >= buzzerData[i].timeout) {
			// Check if this was the last
			if (buzzerData[i].noteIndex >= buzzerData[i].melodyLength) {
				// Detect end of melody
				if (buzzerData[i].repeat == false) {
					buzzer_stop(i);
					continue;
				}

				// Restart to begin
				buzzerData[i].noteIndex = 0;
			}

			// Read note from melody
			note = buzzerData[i].pMelody[buzzerData[i].noteIndex];
			++buzzerData[i].noteIndex;

			// Program note duration
			buzzerData[i].timeout = tick + (note & ~STATE_MASK);

			// Set state
			output_set(buzzerData[i].output, (note & STATE_ON) != 0);
		}
	}
}

#endif /* MODULE_BUZZER */