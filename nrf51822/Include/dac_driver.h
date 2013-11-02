#ifdef DAC_DRIVER_h
void update_acc_data(acc_data_t *);
int write_register(uint8_t register_address, uint8_t value);
int read_register(uint8_t register_address, int num_bytes, uint8_t* values);
int acc_init(void);

#endif