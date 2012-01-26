
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "menu.h"
#include "settings.h"

static const char STR_EXIT[] PROGMEM = "Exit";

static const char STR_FREQUENCY[] PROGMEM = "Frequency";
static const char STR_BAND[] PROGMEM = "Band";
static const char STR_TX_POWER[] PROGMEM = "Transmit Power";
static const char STR_DATA_RATE[] PROGMEM = "Data Rate";
static const char STR_RSSI_THRESHOLD[] PROGMEM = "RSSI Threshold";
static const char STR_FILTER_BW[] PROGMEM = "Filter BW";
static const char STR_FSK_SHIFT[] PROGMEM = "FSK Shift";


menu_entry_t rf_menu_entries[] = {
	{STR_BAND,            (hdlr_t)handle_band_setting,        (hdlr_t)show_band_setting,        &band_setting },
	{STR_FREQUENCY,       (hdlr_t)handle_frequency_setting,   (hdlr_t)show_frequency_setting,   &frequency_setting },
	{STR_TX_POWER,        (hdlr_t)handle_tx_power_setting,    (hdlr_t)show_tx_power_setting,    &tx_power_setting },
	{STR_DATA_RATE,       (hdlr_t)handle_data_rate_setting,   (hdlr_t)show_data_rate_setting,   &datarate_setting },
	{STR_RSSI_THRESHOLD,  (hdlr_t)handle_rssi_setting,        (hdlr_t)show_rssi_setting,        &rssi_setting },
	{STR_FILTER_BW,       (hdlr_t)handle_bandwidth_setting,   (hdlr_t)show_bandwidth_setting,   &bandwidth_setting },
	{STR_FSK_SHIFT,       (hdlr_t)handle_fsk_setting,         (hdlr_t)show_fsk_setting,         &fsk_setting },
};


menu_t rf_menu = {
	7,                //uint8_t num_entries;
	rf_menu_entries,  //menu_entry_t * entries;
	0,
};
