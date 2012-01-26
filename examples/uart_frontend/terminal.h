
#include <avr/pgmspace.h>

void terminal_printline(uint8_t line, char * str);
void terminal_printline_P(uint8_t line, PGM_P str);

#define KEY_UP    'u'//0x0100
#define KEY_DOWN  'd'//0x0101
#define KEY_ENTER 0x0D
#define KEY_EXIT  0x08

uint16_t terminal_get_key_nb();

void terminal_clear_screen();
void terminal_set_cursor_to_line_and_clear(uint8_t line);
void terminal_set_cursor(uint8_t line, uint8_t x);
void terminal_clear_line_to_end();
void terminal_push_cursor();
void terminal_pop_cursor();
