#ifndef TEMP_TEMP_H
#define TEMP_TEMP_H

#include "global.hpp"

#define DEVICE_INDEX_0          0
#define DEVICE_INDEX_1          1

float temp_get_value(uint8_t deviceIndex);
void temp_init(void);
void temp_main(void);

#endif /* TEMP_TEMP_H */
