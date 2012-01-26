
uint8_t tx_power_setting;
extern uint16_t frequency_setting;
extern uint8_t band_setting;
extern uint8_t datarate_setting;
extern uint8_t rssi_setting;
extern uint8_t bandwidth_setting;
extern uint8_t fsk_setting;

void load_settings();


extern void show_frequency_setting(uint16_t * var);
void handle_frequency_setting(uint16_t * var);

void show_band_setting(uint8_t * var);
void handle_band_setting(uint8_t * var);

void show_tx_power_setting(uint8_t * var);
void handle_tx_power_setting(uint8_t * var);

void show_data_rate_setting(uint8_t * var);
void handle_data_rate_setting(uint8_t * var);

void show_rssi_setting(uint8_t * var);
void handle_rssi_setting(uint8_t * var);

void show_bandwidth_setting(uint8_t * var);
void handle_bandwidth_setting(uint8_t * var);

void show_fsk_setting(uint8_t * var);
void handle_fsk_setting(uint8_t * var);
