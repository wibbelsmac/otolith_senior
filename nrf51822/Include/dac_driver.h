#ifndef DAC_DRIVER_h

uint16_t const DAC_WRITE = 0x7000;
uint16_t const DAC_SHUTDOWN = 0x6000;
int dac_init(void);
int write_voltage(uint8_t value);

#endif
