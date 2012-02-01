
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include "terminal.h"
#include "menu.h"
#include "settings.h"
#include "../../src/xprintf/xprintf.h"

#include "rfm12.h"
#include "../uart_lib/uart.h"


#if RFM12_USE_WAKEUP_TIMER
static void wakeup_timer_test(){
	xprintf_P(PSTR("wakeup timer test 1 second...\r\n"));
	
	//set the wakeup timer to 10 ms
	rfm12_set_wakeup_timer(10);
	uint8_t x;
	for(x=0;x<100;x++){
		xprintf_P(PSTR("."));
		while(ctrl.wkup_flag == 0){
			if (rfm12_rx_status() == STATUS_COMPLETE)
			{
				rfm12_rx_clear();
			}
			rfm12_tick();			
		}
		ctrl.wkup_flag = 0;
	}
	rfm12_set_wakeup_timer(100);
}
#endif


void transmit_packets(){
	uint8_t tv[] = "foobar";
	static uint16_t ticker = 0;
	static uint16_t count;

	ticker++;
	if ((ticker == 5000 ))
	{
		ticker = 0;
		rfm12_tx (sizeof(tv), 0, tv);

		terminal_push_cursor();

		terminal_set_cursor_to_line_and_clear(11);

		count++;
		xprintf_P(PSTR("TX %3d"), count);

		terminal_pop_cursor();

	}
}


void display_received_packets(){
	uint8_t *buf;
	uint8_t i;
	static uint16_t count;

	if (rfm12_rx_status() == STATUS_COMPLETE)
	{
		terminal_push_cursor();

		terminal_set_cursor_to_line_and_clear(10);

		count++;
		xprintf_P(PSTR("RX %3d: "), count);

		buf = rfm12_rx_buffer();
		uint8_t len = rfm12_rx_len();

		// dump buffer contents to uart
		
		
		#ifdef UART_HEXDUMP
			//as hexdump
			uart_hexdump(buf, len);
		#endif
		
		//as string
		for (i=0;i<len;i++)
		{
			uint8_t c = buf[i];
			if(c >= 0x20) //if printable 
				uart_putc ( c );
			else
				uart_putc (0xff);
		}
		
		terminal_pop_cursor();
				
		// tell the implementation that the buffer
		// can be reused for the next data.
		rfm12_rx_clear();
	}
}


extern menu_t rf_menu;

int main(){


	uart_init();
	xdev_out(uart_putc);

	xprintf_P(PSTR("*** RFM12 Test ***\r\n"));
	
	_delay_ms(500);
	xprintf_P(PSTR("rfm12_init()\r\n"));

	rfm12_init();
	rfm12_load_settings();
	sei();

	#if RFM12_USE_WAKEUP_TIMER
		wakeup_timer_test();
	#endif
	
	terminal_clear_screen();
	
	//handle_menu(&rf_menu);
	handle_settings_menu();

	while(1);

}
