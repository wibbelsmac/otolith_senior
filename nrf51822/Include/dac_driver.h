#ifndef DAC_DRIVER_h
#define DAC_DRIVER_h
#define DAC_WRITE 0x7000
#define DAC_SHUTDOWN 0x6000
int dac_init(void);
int write_voltage(uint16_t value);

#endif
