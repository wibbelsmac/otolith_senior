#include <spi_master.h>
#include "dac_driver.h"

int write_register(uint8_t register_address, uint8_t value);

static uint32_t* spi_base = 0;

int dac_init() {
  // Use SPI1, mode3 with lsb shifted as requested  PLACE AT TOP	
  volatile uint32_t counter = 0;
  spi_base = spi_master_init(SPI1, SPI_MODE3, false);
  if (spi_base == 0) {
    return -1;
  }
	
	uint8_t foo = 0x7f;
  //while(true) {
    write_voltage(foo);
    volatile int i = 0;
    for(i = 0; i < 5000; i++);
  //}

  return 0;
}

int write_voltage(uint16_t value) 
{
  uint8_t rx[2];
  uint8_t tx[2];
	// shifting by 4 for 8 bit resolution
	*((uint16_t*)tx) = DAC_WRITE | (value << 4);

  if(!spi_master_tx_rx(spi_base, 2, tx, rx)) 
  {
    return -1;
  }

  return 0;
}


