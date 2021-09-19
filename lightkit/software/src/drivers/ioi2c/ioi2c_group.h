#ifndef IOI2C_GROUP_H
#define IOI2C_GROUP_H

#include "ioi2c_driver.h"

#define IOI2C_GROUP_MAX_DEVICE 5

class IOI2CGroup {
public:
	IOI2CGroup(uint32_t sda, uint32_t scl, const uint8_t * addr7bitsTable, uint8_t deviceCount);
	~IOI2CGroup();

	void begin(void);

	void output_write(uint8_t number, uint8_t value);
	int8_t input_read(uint8_t number);

private:
	uint8_t mDeviceCount;
	IOI2CDriver * mDeviceTable[IOI2C_GROUP_MAX_DEVICE];
};

#endif // IOI2C_GROUP_H