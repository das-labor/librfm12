
extern uint8_t brightnes;
extern uint8_t red;
extern uint8_t green;
extern uint8_t blue;

uint8_t tx_power_setting;


void load_settings();

extern int32_t video_start_frames[16];

extern uint16_t frequency_setting;
extern uint8_t band_setting;

extern void show_frequency_setting(uint16_t * var);
void handle_frequency_setting(uint16_t * var);

void show_band_setting(uint8_t * var);
void handle_band_setting(uint8_t * var);

void show_tx_power_setting(uint8_t * var);
void handle_tx_power_setting(uint8_t * var);

extern uint32_t standby_frame;

extern uint8_t power_mode_setting;
#define POWER_MODE_OFF 0
#define POWER_MODE_AUTO 1
#define POWER_MODE_ALLWAYS_ON 2


void show_power_mode_setting(uint8_t * var);
void handle_power_mode_setting(uint8_t * var);

void handle_standby_time_setting(uint8_t * var);

void handle_status_screen(void * menu);

#define NUM_PANELS 18
