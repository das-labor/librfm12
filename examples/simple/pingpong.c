#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "rfm12.h"
#include "../uart_lib/uart.h"

int main ( void )
{
	uint8_t *bufptr;
	uint8_t i;
	uint8_t tv[] = "foobar";

	uint16_t ticker = 0;
	
	#ifdef LED_PORT
		LED_DDR |= _BV(LED_BIT); //enable LED if any
	#endif

	uart_init();

	_delay_ms(100);  //little delay for the rfm12 to initialize properly
	rfm12_init();    //init the RFM12
	
	sei();           //interrupts on

	uart_putstr ("\r\n" "RFM12 Pingpong test\r\n");

	while (42) //while the universe and everything
	{
		if (rfm12_rx_status() == STATUS_COMPLETE)
		{
			//so we have received a message

			//blink the LED
			#ifdef LED_PORT
				LED_PORT ^= _BV(LED_BIT);
			#endif

			uart_putstr ("new packet: \"");

			bufptr = rfm12_rx_buffer(); //get the address of the current rx buffer

			// dump buffer contents to uart			
			for (i=0;i<rfm12_rx_len();i++)
			{
				uart_putc ( bufptr[i] );
			}
			
			uart_putstr ("\"\r\n");
			
			// tell the implementation that the buffer
			// can be reused for the next data.
			rfm12_rx_clear();
		}


		ticker ++;
		if(ticker == 3000){
			ticker = 0;
			uart_putstr (".\r\n");
			rfm12_tx (sizeof(tv), 0, tv);
		}

		//rfm12 needs to be called from your main loop periodically.
		//it checks if the rf channel is free (no one else transmitting), and then
		//sends packets, that have been queued by rfm12_tx above.
		rfm12_tick();
		
		_delay_us(100); //small delay so loop doesn't run as fast
	}
}
