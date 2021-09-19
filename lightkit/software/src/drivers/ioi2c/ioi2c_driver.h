#ifndef IOI2C_DRIVER
#define IOI2C_DRIVER

#include <Arduino.h>
#include <Wire.h>

#define IOI2C_OUTPUT_COUNT 6
#define IOI2C_INPUT_COUNT  2
#define IOI2C_INPUT_OFFSET 6

#define IOI2C_0_ADDR 0x20
#define IOI2C_1_ADDR 0x21
#define IOI2C_2_ADDR 0x22
#define IOI2C_3_ADDR 0x23
#define IOI2C_4_ADDR 0x24
#define IOI2C_5_ADDR 0x25
#define IOI2C_6_ADDR 0x26
#define IOI2C_7_ADDR 0x27

class IOI2CDriver {
	public:
	enum Status
	{
		STATUS_NONE,
		STATUS_OK,
		STATUS_COMM_ERROR
	};

	public:
	IOI2CDriver(uint32_t sda, uint32_t scl, uint8_t addr7bits);
	~IOI2CDriver();

	void begin(void);

	void                output_write(uint8_t number, uint8_t value);
	void                update_outputs(void);
	int8_t              input_read(uint8_t number);
	IOI2CDriver::Status get_status(void);
	uint8_t             get_address(void);

	private:
	uint8_t mAddr7bits;
	uint8_t mOutValue; // Bit 0: S1, Bit 1: S2...
};

#endif // IOI2C_DRIVER