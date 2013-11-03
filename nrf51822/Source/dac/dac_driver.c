#include <spi_master.h>
#include "dac_driver.h"
#include "util.h"

int write_register(uint8_t register_address, uint8_t value);

static uint32_t* spi_base = 0;

int dac_init() {
  // Use SPI1, mode3 with lsb shifted as requested  PLACE AT TOP	
  volatile uint32_t counter = 0;
  spi_base = spi_master_init(SPI1, SPI_MODE3, false);
  if (spi_base == 0) {
    return -1;
  }
	
	uint8_t foo = 0xff;
  //while(true) {
    shutdown_voltage();
    volatile int i = 0;
    for(i = 0; i < 5000; i++);
  //}

  return 0;
}

int write_voltage(uint16_t value)
{
  return write_to_dac(DAC_WRITE, value);
}

int write_to_dac(uint16_t config, uint16_t value) {
	// shifting by 4 for 8 bit resolution
	uint16_t data = config | (value << 4);
	uint8_t tx[2];
	uint8_t rx[2];
	tx[1] = data & 0x00FF;
	tx[0] = data >> 8;

  if(!spi_master_tx_rx(spi_base, 2, tx, rx)) 
  {
    return -1;
  }

  return 0;
}

int shutdown_voltage()
{
  return write_to_dac(DAC_SHUTDOWN, 0x0);
}


