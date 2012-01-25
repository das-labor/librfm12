
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/sleep.h>

#include "rfm12.h"
#include "../uart_lib/uart.h"


static void wakeup_timer_test(){
	printf_P(PSTR("wakeup timer test 1 second...\r\n"));
	
	//set the wakeup timer to 10 ms
	rfm12_set_wakeup_timer(10);
	uint8_t x;
	for(x=0;x<100;x++){
		printf_P(PSTR("."));
		while(ctrl.wkup_flag == 0){
			if (rfm12_rx_status() == STATUS_COMPLETE)
			{
				rfm12_rx_clear();
			}
			rfm12_tick();
			//_delay_ms(1);
			//uart_hexdump(ctrl.guard, 10);
			//uart_hexdump(ctrl.guard1, 10);
			
		}
		ctrl.wkup_flag = 0;
	}
}

static void pingpong_test(){
	uint8_t *bufcontents;
	uint8_t i;
	uint8_t tv[] = "foobar";

	uint16_t ticker = 0;
	uint16_t tocker = 5000;
	
	printf_P(PSTR("pingpong test\r\n"));

	while (42)
	{
		if (rfm12_rx_status() == STATUS_COMPLETE)
		{

			printf_P(PSTR("new packet: "));

			bufcontents = rfm12_rx_buffer();

			// dump buffer contents to uart
			for (i=0;i<rfm12_rx_len();i++)
			{
				uart_putc ( bufcontents[i] );
			}
			
			printf_P(PSTR("\r\n"));
			// tell the implementation that the buffer
			// can be reused for the next data.
			rfm12_rx_clear();

		}

		ticker++;
		if ((ticker == 5000 ))
		{
			ticker = 0;
			printf_P(PSTR(".\r\n"));
			rfm12_tx (sizeof(tv), 0, tv);
		}

		rfm12_tick();
		_delay_us(100);
		
		
		
		if(ctrl.wkup_flag){
			ctrl.wkup_flag = 0;
			tocker = 150;
		}
		
		if(tocker == 0){
			PORTA |= 1;
			DDRA |= 1;
		}else{
			PORTA &= ~1;
			DDRA |= 1;
			tocker--;
		}
		
	}
}


int my_putc(char c, FILE * fp){
	uart_putc(c);
	return 0;
}

int main(){


	uart_init();

	fdevopen(my_putc, 0);

	printf_P(PSTR("*** RFM12 Test ***\r\n"));
	
	_delay_ms(500);
	printf_P(PSTR("rfm12_init()\r\n"));

	rfm12_init();
	sei();

	wakeup_timer_test();
	
	//set the wakeup timer to 10 ms
	rfm12_set_wakeup_timer(10);

	pingpong_test();



}