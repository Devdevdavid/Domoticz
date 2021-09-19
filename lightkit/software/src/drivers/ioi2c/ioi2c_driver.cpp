#include "ioi2c_driver.h"

IOI2CDriver::IOI2CDriver(uint32_t sda, uint32_t scl, uint8_t addr7bits)
{
	mAddr7bits = addr7bits;
	Wire.begin(sda, scl);
}

IOI2CDriver::~IOI2CDriver(void)
{
}

void IOI2CDriver::begin(void)
{
	// Bit value are inverted - Setting all off
	mOutValue = 0xFF;

	// Write output at begin
	update_outputs();
}

/**
 * @brief Write the new value of output specified by number
 *
 * @param number Output to write
 * @param value Boolean
 */
void IOI2CDriver::output_write(uint8_t number, uint8_t value)
{
	uint8_t bitValue;

	if (number >= IOI2C_OUTPUT_COUNT) {
		return;
	}

	bitValue = (1 << (IOI2C_OUTPUT_COUNT - number - 1));

	// Take inversion into account here
	if (value) {
		mOutValue &= ~bitValue;
	} else {
		mOutValue |= bitValue;
	}

	update_outputs();
}

/**
 * @brief Send the output values stored in the instance
 */
void IOI2CDriver::update_outputs(void)
{
	Wire.beginTransmission(mAddr7bits);
	Wire.write(mOutValue);
	Wire.endTransmission(); // Can't block if SDA is low (like an other Master on the line)
}

/**
 * @brief Read inputs
 *
 * @param number Input to read
 * @return boolean
 */
int8_t IOI2CDriver::input_read(uint8_t number)
{
	uint8_t byteValue;

	if (number >= IOI2C_INPUT_COUNT) {
		return -1;
	}

	if (Wire.requestFrom(mAddr7bits, (uint8_t) 1) != 1) {
		return -1;
	}

	byteValue = Wire.read();

	// Do inversion here
	return (byteValue & (0x80 >> number)) == 0;
}

/**
 * @brief Get the status of the device
 *
 * @return IOI2CDriver::Status
 */
IOI2CDriver::Status IOI2CDriver::get_status(void)
{
	if (Wire.requestFrom(mAddr7bits, (uint8_t) 1) != 1) {
		return IOI2CDriver::STATUS_COMM_ERROR;
	}

	return IOI2CDriver::STATUS_OK;
}

/**
 * @brief Return the I2C address of the device
 * @details [long description]
 * @return [description]
 */
uint8_t IOI2CDriver::get_address(void)
{
	return mAddr7bits;
}
