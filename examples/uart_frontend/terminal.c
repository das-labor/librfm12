
#include <stdio.h>
#include "../../src/xprintf/xprintf.h"
#include "../uart_lib/uart.h"
#include "terminal.h"

void terminal_set_cursor(uint8_t line, uint8_t x){
	//VT100      set cursor pos 
	xprintf_P(PSTR( "\x1b[%d;%dH") , line + 1, x+1);
}

void terminal_clear_line_to_end(){
	xprintf_P(PSTR( "\x1b[K"));
}

void terminal_set_cursor_to_line_and_clear(uint8_t line){
	//VT100      set cursor pos  clear current line
	xprintf_P(PSTR( "\x1b[%d;1H"  "\x1b[2K" ) , line + 1);
}

void terminal_clear_screen(){
	xprintf_P(PSTR( "\x1b[2J" ) );
}

void terminal_push_cursor(){
	xprintf_P(PSTR("\x1b" "7"));
}

void terminal_pop_cursor(){
	xprintf_P(PSTR("\x1b" "8"));
}


void terminal_printline(uint8_t line, char * str){
	terminal_set_cursor_to_line_and_clear(line);
	xputs(str);
}

void terminal_printline_P(uint8_t line, PGM_P str){
	terminal_set_cursor_to_line_and_clear(line);
	xprintf_P(str);
}

uint16_t terminal_get_key_nb(){
	char c;
	static uint8_t esc_code_count;

	if(uart_getc_nb(&c)){
		if(esc_code_count){
			switch(esc_code_count){
				case 1:	if(c == '[') esc_code_count = 2; else esc_code_count = 0; break;
				case 2:
					switch (c){
						case 'A': esc_code_count = 0; return KEY_UP;
						case 'B': esc_code_count = 0; return KEY_DOWN;
						default: esc_code_count = 0; break;
					}break;
				
			}
		}else{
			if(c == 0x1b){
				esc_code_count = 1;
				return 0;
			}else{
				return c;
			}
		}
	}
	return 0;
}
