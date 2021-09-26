#ifndef FEU_ROUGE_HPP
#define FEU_ROUGE_HPP

#include "drivers/ioi2c/ioi2c_group.h"
#include "global.hpp"
#include "io/outputs.hpp"

// Color available on a traffic light
#define COLOR_MASK_BLACK  0x00
#define COLOR_MASK_RED    0x01
#define COLOR_MASK_YELLOW 0x02
#define COLOR_MASK_GREEN  0x04

// FCT MODE

typedef enum
{
	MODE_FCT_NONE = 0,
	MODE_FCT_TRAFFIC_LIGHT,
	MODE_FCT_DOOR,
	MODE_FCT_DISCO
} FEU_ROUGE_MODE_FCT_E;

// FCT MODE - TRAFIC LIGHT

typedef enum
{
	TRAFIC_LIGHT_STATE_NONE = 0,
	TRAFIC_LIGHT_STATE_HS,
	TRAFIC_LIGHT_STATE_HS_ON,
	TRAFIC_LIGHT_STATE_OPENED,
	TRAFIC_LIGHT_STATE_CLOSING,
	TRAFIC_LIGHT_STATE_CLOSED
} TRAFIC_LIGHT_STATE_E;

typedef enum
{
	TRAFIC_LIGHT_CMD_NONE = 0,
	TRAFIC_LIGHT_CMD_HS,
	TRAFIC_LIGHT_CMD_OPEN,
	TRAFIC_LIGHT_CMD_CLOSE
} TRAFIC_LIGHT_CMD_E;

typedef struct {
	TRAFIC_LIGHT_STATE_E prevState;
	TRAFIC_LIGHT_STATE_E curState;
	TRAFIC_LIGHT_CMD_E   curCmd;
	uint32_t             tickLastStateChange;
} trafic_light_data_t;

// FCT MODE - DOOR

typedef enum
{
	DOOR_STATE_NONE = 0,
	DOOR_STATE_WAIT_SOMEONE,
	DOOR_STATE_PEOPLE_INSIDE,
	DOOR_STATE_CLEANNING,
} DOOR_STATE_E;

typedef enum
{
	DOOR_CMD_NONE = 0,
	DOOR_CMD_SOMEONE_COME_IN,
	DOOR_CMD_SOMEONE_COME_OUT,
} DOOR_CMD_E;

typedef struct {
	DOOR_STATE_E prevState;
	DOOR_STATE_E curState;
	DOOR_CMD_E   curCmd;
	uint32_t     tickLastStateChange;
} door_data_t;

// Functions

void feu_rouge_mode_fct_trafic_light(TRAFIC_LIGHT_CMD_E cmd);
void feu_rouge_mode_fct_door(DOOR_CMD_E cmd);

int  feu_rouge_init(void);
void feu_rouge_main(void);

#endif // FEU_ROUGE_HPP