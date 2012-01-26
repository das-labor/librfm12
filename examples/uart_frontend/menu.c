
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include <util/delay.h>
#include "config.h"
#include "menu.h"
#include "run_statemachines.h"
#include "terminal.h"

#define PW(a) (pgm_read_word(&(a)))

//this can be set by a menu handler function if it wants to leave the menu after
//executing the handler. setting it to 1 leaves one menu, 2 two menus ...
uint8_t leave_menu;

void menu_exit(void * foo){
	foo = foo;
	leave_menu = 1;
}

uint8_t menu_pos;
uint8_t old_menu_pos;

void draw_parameter(char * str){
	terminal_set_cursor(menu_pos, 20);
	terminal_clear_line_to_end();
	xputs(str);
}

void draw_parameter_P(PGM_P str){
	terminal_set_cursor(menu_pos, 20);
	terminal_clear_line_to_end();
	uart_putstr_P(str);
}

void draw_menu_entry(menu_t * menu, uint8_t menu_pos){
	void (*draw)(void *);
	menu_entry_t * entries = menu->entries;

	terminal_printline_P(menu_pos, (PGM_P) entries[menu_pos].text );
	
	draw = entries[menu_pos].draw;
	if(draw){
		draw(entries[menu_pos].ref);
	}else{
	}
}

void draw_menu(menu_t * menu){
	old_menu_pos = menu_pos;
	for( menu_pos=0; menu_pos  <  menu->num_entries; menu_pos++ ){
		draw_menu_entry(menu, menu_pos);
	}
	menu_pos = old_menu_pos;
}

void draw_menu_at_pos(menu_t * menu, uint8_t menu_pos){
	terminal_set_cursor(menu_pos, 0);
}

void handle_menu(void * men){
	menu_t * menu = men;
	menu_pos = 0;
	
	handle_menu_start:

	if(leave_menu){
		leave_menu--;
		return;
	}

	draw_menu(menu);
	draw_menu_at_pos(menu, menu_pos);
	
	while(1){
		uint16_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(menu_pos > 0){
						menu_pos--;
						draw_menu_at_pos(menu, menu_pos);
					}
					break;
				case KEY_DOWN:
					if(menu_pos < menu->num_entries - 1){
						menu_pos++;
						draw_menu_at_pos(menu, menu_pos);
					}
					break;
				case KEY_ENTER:
					if(menu->entries[menu_pos].func){
						menu->entries[menu_pos].func(menu->entries[menu_pos].ref);
						goto handle_menu_start;
					}
					break;
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}

void show_uint8_setting(uint8_t * var){
	char s[10];
	itoa(*var, s, 10);
	terminal_printline(1,s);
}

void handle_uint8_setting( uint8_t * var){
	show_uint8_setting(var);
	while(1){
		uint8_t key;
		if((key = terminal_get_key_nb()) != 0){
			switch(key){
				case KEY_UP:
					if(*var > 0){
						(*var)--;
						show_uint8_setting(var);
					}
					break;
				case KEY_DOWN:
					if(*var < 255){
						(*var)++;
						show_uint8_setting(var);
					}
					break;
				case KEY_ENTER:
				case KEY_EXIT:
					return;
					break;
			}
		}
		
		run_statemachines();
	}
}
