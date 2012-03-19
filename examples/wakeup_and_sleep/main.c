#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sleep.h>

#include "rfm12.h"
#include "../uart_lib/uart.h"

int main ( void )
{
	uint8_t *bufptr;
	uint8_t i;
	uint8_t tv[] = "foobar";
	
	#ifdef LED_PORT
		LED_DDR |= _BV(LED_BIT); //enable LED if any
	#endif

	uart_init();

	_delay_ms(100);  //little delay for the rfm12 to initialize properly
	rfm12_init();    //init the RFM12
	
	sei();           //interrupts on

	uart_putstr ("\r\n" "RFM12 wakeup and sleep test\r\n");

	//set the wakeup timer to 4 * 256ms = 1.024s
	rfm12_set_wakeup_timer(0x132);

	//set AVR sleep mode
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	while (42) //while the universe and everything
	{
		if (rfm12_rx_status() == STATUS_COMPLETE)
		{
			//so we have received a message

			

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
			_delay_ms(100);
	
		}
		
		if(ctrl.wkup_flag){
			//blink the LED
			#ifdef LED_PORT
				LED_PORT ^= _BV(LED_BIT);
			#endif
		}
		ctrl.wkup_flag = 0;

	}
	
	while (42) //while the universe and everything
	{
		uart_putstr ("t\r\n");
		
		//Transmit something
		rfm12_tx (sizeof(tv), 0, tv);
		
		rfm12_tick();
		
		
		uart_putstr ("s\r\n");
		_delay_ms(100);
		
		//sleep without receiving
		rfm12_power_down();
		while(!ctrl.wkup_flag) {
			sleep_mode();
		}
		ctrl.wkup_flag = 0;
		rfm12_power_up();

		
		uart_putstr ("r\r\n");
		_delay_ms(100);

		//sleep while receiving
		while(!ctrl.wkup_flag) {
			sleep_mode();
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
				_delay_ms(100);
		
			}
		}
		ctrl.wkup_flag = 0;
		
	}
}
