#include "config.h"

#define EEPROM_I2C_NO 0
#define EEPROM_SLAVE_ADDR 0b1010010

static void eeprom_read(u8 word_addr, void *buf, size_t size)
{
	i2c_block_read(EEPROM_I2C_NO, EEPROM_SLAVE_ADDR, word_addr, buf, size);
}

static void eeprom_write(u8 word_addr, const void *buf, size_t size)
{
	i2c_block_write(EEPROM_I2C_NO, EEPROM_SLAVE_ADDR, word_addr, buf, size);
}

static void eeprom_test()
{
	i2c_init(EEPROM_I2C_NO);

	u8 wbuf[16] = "hello world abc";
	eeprom_write(32, wbuf, sizeof(wbuf));

	u8 rbuf[16] = {0};
	eeprom_read(32, rbuf, sizeof(rbuf));

	printk("%s: %s\n", __FUNCTION__, rbuf);
}

module_init(eeprom_test);
