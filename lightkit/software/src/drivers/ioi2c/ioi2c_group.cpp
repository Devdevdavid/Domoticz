#include "ioi2c_group.h"

IOI2CGroup::IOI2CGroup(uint32_t sda, uint32_t scl, const uint8_t * addr7bitsTable, uint8_t deviceCount)
{
	if (deviceCount > IOI2C_GROUP_MAX_DEVICE) {
		deviceCount = IOI2C_GROUP_MAX_DEVICE;
	}

	mDeviceCount = deviceCount;

	for (uint8_t i = 0; i < mDeviceCount; ++i) {
		mDeviceTable[i] = new IOI2CDriver(sda, scl, addr7bitsTable[i]);
	}
}

IOI2CGroup::~IOI2CGroup()
{
}

void IOI2CGroup::begin(void)
{
	for (uint8_t i = 0; i < mDeviceCount; ++i) {
		mDeviceTable[i]->begin();
	}
}

void IOI2CGroup::output_write(uint8_t number, uint8_t value)
{
	uint8_t deviceIndex = number / IOI2C_OUTPUT_COUNT;
	uint8_t outputIndex = number % IOI2C_OUTPUT_COUNT;

	if (deviceIndex >= mDeviceCount) {
		return;
	}

	mDeviceTable[deviceIndex]->output_write(outputIndex, value);
}

int8_t IOI2CGroup::input_read(uint8_t number)
{
	uint8_t deviceIndex = number / IOI2C_INPUT_COUNT;
	uint8_t outputIndex = number % IOI2C_INPUT_COUNT;

	if (deviceIndex >= mDeviceCount) {
		return -1;
	}

	return mDeviceTable[deviceIndex]->input_read(outputIndex);
}