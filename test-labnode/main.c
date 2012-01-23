
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "rfm12.h"
#include "uart/uart.h"



static void pingpong_test(){
	uint8_t *bufcontents;
	uint8_t i;
	uint8_t tv[] = "foobar";

	uint16_t ticker = 0;
	
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
		if ((ticker == 500 ))
		{
			ticker = 0;
			printf_P(PSTR(".\r\n"));
			rfm12_tx (sizeof(tv), 0, tv);
		}

		rfm12_tick();
		_delay_ms(1);
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

	pingpong_test();



}