#ifndef DAC_DRIVER_h
#define DAC_DRIVER_h
#define DAC_WRITE 0x3000
#define DAC_SHUTDOWN 0x2000
int dac_init(void);
int write_voltage(uint16_t value);
int write_to_dac(uint16_t config, uint16_t value);
int shutdown_voltage(void);

#endif
