
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
#include "xprintf.h"


uint16_t frequency_setting;
uint8_t band_setting;
uint8_t tx_power_setting;
uint8_t datarate_setting;
uint8_t rssi_setting;
uint8_t bandwidth_setting;
uint8_t fsk_setting;

#define BAND_433 0
#define BAND_915 1
#define BAND_868 2

void set_band(){
	switch(band_setting){
		case BAND_433:
			rfm12_set_band (RFM12_BAND_433);
			break;
		case BAND_915:
			rfm12_set_band (RFM12_BAND_915);
			break;
		case BAND_868:
			rfm12_set_band (RFM12_BAND_868);
			break;
	}
}

void save_settings(){
	eeprom_write_byte((void*)0, 0x55);
	eeprom_write_byte((void*)1, band_setting);
	eeprom_write_word((void*)2, frequency_setting);
	eeprom_write_byte((void*)4, tx_power_setting);
	eeprom_write_byte((void*)5, datarate_setting);
	eeprom_write_byte((void*)6, rssi_setting);
	eeprom_write_byte((void*)7, bandwidth_setting);
	eeprom_write_byte((void*)8, fsk_setting);
		
}

void load_settings(){
	if(0x55 == eeprom_read_byte((void*)0)){
		band_setting = eeprom_read_byte((void*)1);
		frequency_setting = eeprom_read_word((void*)2);
		tx_power_setting = eeprom_read_byte((void*)4);
		datarate_setting = eeprom_read_byte((void*)5);
		rssi_setting = eeprom_read_byte((void*)6);
		bandwidth_setting = eeprom_read_byte((void*)7);
		fsk_setting = eeprom_read_byte((void*)8);
	}else{
		//load defaults
		band_setting = BAND_433;
		frequency_setting = RFM12_FREQUENCY_CALC_433(433170000UL);
		tx_power_setting = 0;
		datarate_setting = DATARATE_VALUE;
		rssi_setting = RFM12_RXCTRL_RSSI_79;
		bandwidth_setting = RFM12_RXCTRL_BW_400;
		fsk_setting = RFM12_TXCONF_FS_CALC(FSK_SHIFT);
	}

	set_band();
	rfm12_set_frequency (frequency_setting);
	rfm12_set_tx_power (tx_power_setting );
	rfm12_set_rate (datarate_setting);
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
	
	//868:
	//f0 = 860 + F * (2 /400)  [MHz]
	//f0 = 860000 + F * 5 [kHz]
	
	
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
	}else if(band_setting == BAND_868){
		val *= 2;
		mhz = 860 + val / 400;
		khz = ((val % 400) * 5) / 2;
	}else{
		mhz = 0;
		khz = 0;
	}
	char s[18];
	#if (DISP_LEN == 8)
		xsprintf_P(s, PSTR("%3d.%03d"), mhz, khz); 
	#else
		xsprintf_P(s, PSTR("%3d.%03d MHz"), mhz, khz);
	#endif
	
	draw_parameter(s);
}

void handle_frequency_setting(uint16_t * var){
	show_frequency_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(*var > 96){
						(*var)-= 4;
						show_frequency_setting(var);
						rfm12_set_frequency (*var);
					}
					break;
				case KEY_DOWN:
					if(*var < 3903){
						(*var)+= 4;
						show_frequency_setting(var);
						rfm12_set_frequency (*var);
					}
					break;
				case KEY_ENTER:
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
		case BAND_868:
		draw_parameter_P(PSTR("868MHz"));
		break;
	}
}

void handle_band_setting(uint8_t * var){
	show_band_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(*var > 0){
						(*var)--;
						show_band_setting(var);
					}
					break;
				case KEY_DOWN:
					if(*var < 2){
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
	xsprintf_P(s, PSTR("%3d dBm"), val); 
	
	draw_parameter(s);
}

void handle_tx_power_setting(uint8_t * var){
	show_tx_power_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > 0){
						(*var)--;
						show_tx_power_setting(var);
						rfm12_set_tx_power (*var);
					}
					break;
				case KEY_UP:
					if(*var < 7){
						(*var)++;
						show_tx_power_setting(var);
						rfm12_set_tx_power (*var);
					}
					break;
				case KEY_ENTER:
					
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_data_rate_setting(uint8_t * var){
	/*
		4. Data Rate Command
		Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
		1 1 0 0 0 1 1 0 cs r6 r5 r4 r3 r2 r1 r0 C623h
		The actual bit rate in transmit mode and the expected bit rate of the received data stream in receive mode is determined by the 7-bit
		parameter R (bits r6 to r0) and bit cs.
		BR = 10000 / 29 / (R+1) / (1+cs*7) [kbps]
	*/
	
	uint8_t val = *var;
	uint16_t n;
	
	if(val & 0x80){
		//low bitrate
		n = 29 * 8;
	}else{
		//high bitrate
		n = 29;
	}
	val &= 0x7f;
	
	n *= val;
	
	uint32_t bitrate = 10000000ul / n;
		
	char s[18];
	xsprintf_P(s, PSTR("%4ld bps"), bitrate); 
	
	draw_parameter(s);
}

void handle_data_rate_setting(uint8_t * var){
	show_data_rate_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > 1){
						(*var)--;
						show_data_rate_setting(var);
						rfm12_set_rate (*var);
					}
					break;
				case KEY_UP:
					if(*var < 255){
						(*var)++;
						show_data_rate_setting(var);
						rfm12_set_rate (*var);
					}
					break;
				case KEY_ENTER:
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_rssi_setting(uint8_t * var){
	uint8_t val = *var;
		
	char s[18];
	xsprintf_P(s, PSTR("%4d dBm"), -61 - (7-val) * 6); 
	
	draw_parameter(s);
}

void handle_rssi_setting(uint8_t * var){
	show_rssi_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > 0){
						(*var)--;
						show_rssi_setting(var);
						rfm12_set_rssi (*var);
					}
					break;
				case KEY_UP:
					if(*var < 7){
						(*var)++;
						show_rssi_setting(var);
						rfm12_set_rssi (*var);
					}
					break;
				case KEY_ENTER:
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_bandwidth_setting(uint8_t * var){
	
	uint16_t val = *var;
	val >>= 5; //right adjust
	
	val = ((7 - val) * 200) / 3;
	
	char s[18];
	xsprintf_P(s, PSTR("%3d kHz"), val); 
	
	draw_parameter(s);
}

void handle_bandwidth_setting(uint8_t * var){
	show_bandwidth_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > RFM12_RXCTRL_BW_400){
						(*var) -= 0x20;
						show_bandwidth_setting(var);
						rfm12_set_bandwidth (*var);
					}
					break;
				case KEY_UP:
					if(*var < RFM12_RXCTRL_BW_67){
						(*var) += 0x20;
						show_bandwidth_setting(var);
						rfm12_set_bandwidth (*var);
					}
					break;
				case KEY_ENTER:
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_fsk_setting(uint8_t * var){
	
	uint16_t val = *var;
	val >>= 4; //right adjust
	
	val = (val + 1)*15;
	
	char s[18];
	xsprintf_P(s, PSTR("+-%d kHz"), val); 
	
	draw_parameter(s);
}

void handle_fsk_setting(uint8_t * var){
	show_fsk_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_DOWN:
					if(*var > 0){
						(*var) -= 0x10;
						show_fsk_setting(var);
						rfm12_set_fsk_shift (*var);
					}
					break;
				case KEY_UP:
					if(*var < 0x70){
						(*var) += 0x10;
						show_fsk_setting(var);
						rfm12_set_fsk_shift (*var);
					}
					break;
				case KEY_ENTER:
					save_settings();
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

