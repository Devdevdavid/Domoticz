#ifndef FEU_ROUGE_HPP
#define FEU_ROUGE_HPP

#include "drivers/ioi2c/ioi2c_group.h"
#include "global.hpp"

// I2C interface to use to communicate with IOI2C boards
#define IOI2C_SDA 4
#define IOI2C_SCL 5

// Color available on a traffic light
#define COLOR_MASK_BLACK  0x00
#define COLOR_MASK_RED    0x01
#define COLOR_MASK_YELLOW 0x02
#define COLOR_MASK_GREEN  0x04

typedef enum
{
	STATE_HS = 0,
	STATE_OPENED,
	STATE_CLOSING,
	STATE_CLOSED
} FEU_ROUGE_STATE_E;

typedef enum
{
	CMD_HS = 0,
	CMD_OPEN,
	CMD_CLOSE
} FEU_ROUGE_CMD_E;

void feu_rouge_set_color(uint32_t color);
void feu_rouge_command(FEU_ROUGE_CMD_E cmd);

int  feu_rouge_init(void);
void feu_rouge_main(void);

#endif // FEU_ROUGE_HPP