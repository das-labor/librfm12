
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "menu.h"
#include "settings.h"

static const char STR_EXIT[] PROGMEM = "Exit";

static const char STR_FREQUENCY[] PROGMEM = "Frequency";
static const char STR_BAND[] PROGMEM = "Band";
static const char STR_TX_POWER[] PROGMEM = "Transmit Power";

menu_entry_t rf_menu_entries[] = {
	{STR_BAND,         (hdlr_t)handle_band_setting,        (hdlr_t)show_band_setting,        &band_setting },
	{STR_FREQUENCY,    (hdlr_t)handle_frequency_setting,   (hdlr_t)show_frequency_setting,   &frequency_setting },
	{STR_TX_POWER,     (hdlr_t)handle_tx_power_setting,    (hdlr_t)show_tx_power_setting,    &tx_power_setting },
	{STR_EXIT,         menu_exit,   0,          0 },
};


menu_t rf_menu = {
	4,                //uint8_t num_entries;
	rf_menu_entries,  //menu_entry_t * entries;
	0,
};
