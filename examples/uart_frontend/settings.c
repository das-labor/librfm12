
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



void save_settings(){
	uint8_t x;	
	uint16_t checksumm = 0;
	
	for(x=0; x < NUM_LIVECTRL_CMDS; x++){
		uint16_t val = livectrl_cmds[x].current_value;
		checksumm += val;
		eeprom_write_word((void*)(2*x), val);
	}
	
	eeprom_write_word((void*)(2*x), checksumm);
}


void load_settings(){

	uint8_t x;
	uint16_t val;	
	uint16_t checksumm = 0;
	
	for(x=0; x < NUM_LIVECTRL_CMDS; x++){
		val = eeprom_read_word((void*)(2*x));
		checksumm += val;
	}
	
	val = eeprom_read_word((void*)(2*x));
	if( val != checksumm) return; //eeprom invalid, keep default values from array

	//set the settings if eeprom valid
	for(x=0; x < NUM_LIVECTRL_CMDS; x++){
		val = eeprom_read_word((void*)(2*x));
		rfm12_livectrl(x, val);
	}
}

static void show_parameter(uint8_t cmd){
	char s[16];
	rfm12_livectrl_get_parameter_string(cmd, s);
	
	terminal_set_cursor(cmd, 20);
	terminal_clear_line_to_end();
	
	xputs(s);
}

static void show_name(uint8_t cmd){
	terminal_set_cursor(cmd, 0);
	terminal_clear_line_to_end();
	xputs(livectrl_cmds[cmd].name);
}

static void handle_setting(uint8_t cmd){
	livectrl_cmd_t  * livectrl_cmd = &livectrl_cmds[cmd];
	
	uint16_t var = livectrl_cmd->current_value;
	uint16_t step = livectrl_cmd->step;
	uint16_t min_val = livectrl_cmd->min_val;
	uint16_t max_val = livectrl_cmd->max_val;
	
	show_parameter(cmd);
	
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(var > min_val){
						var -= step;
						rfm12_livectrl(cmd, var);
						show_parameter(cmd);
					}
					break;
				case KEY_DOWN:
					if(var < max_val){
						var += step;
						rfm12_livectrl(cmd, var);
						show_parameter(cmd);
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

static void draw_settings_menu(){
	uint8_t x;
	
	for(x=0; x < NUM_LIVECTRL_CMDS; x++){
		show_name(x);
		show_parameter(x);
	}
}

static void draw_cursor_at_pos(uint8_t pos){
	terminal_set_cursor(pos, 0);	
}

void handle_settings_menu(){
	uint8_t menu_pos = 0;
	uint8_t menu_size = NUM_LIVECTRL_CMDS;
	handle_menu_start:

	draw_settings_menu();
	draw_cursor_at_pos(menu_pos);
	
	
	while(1){
		uint16_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(menu_pos > 0){
						menu_pos--;
						draw_cursor_at_pos(menu_pos);
					}
					break;
				case KEY_DOWN:
					if(menu_pos < menu_size - 1){
						menu_pos++;
						draw_cursor_at_pos(menu_pos);
					}
					break;
				case KEY_ENTER:
					handle_setting(menu_pos);
					goto handle_menu_start;
					
					break;
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}
