
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include <util/delay.h>
#include "terminal.h"
#include "menu.h"
#include "run_statemachines.h"
#include "rfm12.h"
#include "../../src/include/rfm12_hw.h"
#include "settings.h"


uint16_t frequency_setting;
uint8_t band_setting;
uint8_t tx_power_setting;

uint8_t power_mode_setting;

#define BAND_433 0
#define BAND_915 1

void set_band(){
	switch(band_setting){
		case BAND_433:
			rfm12_set_band (RFM12_BAND_433);
			break;
		case BAND_915:
			rfm12_set_band (RFM12_BAND_915);
			break;
	}
}

void save_settings(){
	eeprom_write_byte((void*)0, 0x55);
	eeprom_write_byte((void*)1, band_setting);
	eeprom_write_word((void*)2, frequency_setting);
	eeprom_write_byte((void*)4, tx_power_setting);
}

void load_settings(){
	if(0x55 == eeprom_read_byte((void*)0)){
		band_setting = eeprom_read_byte((void*)1);
		frequency_setting = eeprom_read_word((void*)2);
		tx_power_setting = eeprom_read_byte((void*)4);

		if(band_setting > 1) band_setting = BAND_433;
		if(tx_power_setting > 7) tx_power_setting = 0;
		if(frequency_setting > 3903) frequency_setting = 0x680;
	}

	set_band();
	rfm12_set_frequency (frequency_setting);
	rfm12_set_tx_power (tx_power_setting );
}


void show_frequency_setting(uint16_t * var){
	//Band [MHz] C1 C2
	//315         1 31
	//433         1 43
	//868         2 43
	//915         3 30
	//f0 = 10 * C1 * (C2 + F/4000) [MHz]
	//f0 = 10 * C1 * (C2*1000 + F/4)    [kHz]
	
	//433:
	//f0 = 430 + F/400  [MHz]
	//f0 = 430000 + F * 2.5 [kHz]
	
	//915:
	//f0 = 900 + F * (3 /400)  [MHz]
	//f0 = 900000 + F * 7.5 [kHz]
	
	
	uint16_t val = *var;
	uint16_t mhz;
	uint16_t khz;
	if(band_setting == BAND_433){
		mhz = 430 + val / 400;
		khz = ((val % 400) * 5) / 2;
	}else if(band_setting == BAND_915){
		val *= 3;
		mhz = 900 + val / 400;
		khz = ((val % 400) * 5) / 2;
	}else{
		mhz = 0;
		khz = 0;
	}
	char s[18];
	#if (DISP_LEN == 8)
		sprintf(s, "%3d.%03d", mhz, khz); 
	#else
		sprintf(s, "%3d.%03d MHz", mhz, khz);
	#endif
	
	draw_parameter(s);
}

void handle_frequency_setting(uint16_t * var){
	show_frequency_setting(var);
	while(1){
		uint16_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(*var > 96){
						(*var)-= 4;
						show_frequency_setting(var);
					}
					break;
				case KEY_DOWN:
					if(*var < 3903){
						(*var)+= 4;
						show_frequency_setting(var);
					}
					break;
				case KEY_ENTER:
					rfm12_set_frequency (*var);
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_band_setting(uint8_t * var){
	switch(*var){
		case BAND_433:
		draw_parameter_P(PSTR("433MHz"));
		break;
		case BAND_915:
		draw_parameter_P(PSTR("915MHz"));
		break;
	}
}

void handle_band_setting(uint8_t * var){
	show_band_setting(var);
	while(1){
		uint16_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(*var > 0){
						(*var)--;
						show_band_setting(var);
					}
					break;
				case KEY_DOWN:
					if(*var < 1){
						(*var)++;
						show_band_setting(var);
					}
					break;
				case KEY_ENTER:
					set_band (*var);
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

#define FULLSCALE_TX_POWER 8;

void show_tx_power_setting(uint8_t * var){
	int16_t val = *var;
	val *= -3;
	val += FULLSCALE_TX_POWER;
	
	char s[18];
	sprintf(s, "%3d dBm", val); 
	
	draw_parameter(s);
}

void handle_tx_power_setting(uint8_t * var){
	show_tx_power_setting(var);
	while(1){
		uint16_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > 0){
						(*var)--;
						show_tx_power_setting(var);
					}
					break;
				case KEY_UP:
					if(*var < 7){
						(*var)++;
						show_tx_power_setting(var);
					}
					break;
				case KEY_ENTER:
					rfm12_set_tx_power (*var);
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}
