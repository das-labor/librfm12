/**** RFM 12 library for Atmel AVR Microcontrollers *******
 * 
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * @author Peter Fuhrmann, Hans-Gert Dahmen, Soeren Heisrath
 */

/******************************************************
 *                                                    *
 *           C O N F I G U R A T I O N                *
 *                                                    *
 ******************************************************/

/*
	Connect the RFM12 to the AVR as follows:

	RFM12           | AVR
	----------------+------------
	SDO             | MISO
	nIRQ            | INT0
	FSK/DATA/nFFS   | VCC
	DCLK/CFIL/FFIT  |  -
	CLK             |  -
	nRES            |  -
	GND             | GND
	ANT             |  -
	VDD             | VCC
	GND             | GND
	nINT/VDI        | -
	SDI             | MOSI
	SCK             | SCK
	nSEL            | Slave select pin defined below
*/


/************************
 * PIN DEFINITIONS
 */

#include "../config.h"

/************************
 * RFM12 CONFIGURATION OPTIONS
 */

//baseband of the module (either RFM12_BAND_433, RFM12_BAND_868 or RFM12_BAND_912)
#define RFM12_BASEBAND RFM12_BAND_433

//center frequency to use (+- FSK frequency shift)
#define FREQ 433175000UL

//FSK frequency shift in kHz
#define FSK_SHIFT 125000


//Output power relative to maximum (0dB is maximum)
#define RFM12_POWER           RFM12_TXCONF_POWER_0

//crystal load capacitance - the frequency can be verified by measuring the
//clock output of RFM12 and comparing to 1MHz.
//11.5pF seems to be o.k. for RFM12, and 10.5pF for RFM12BP, but this may vary.
#define RFM12_XTAL_LOAD RFM12_XTAL_11_5PF

//use this for datarates >= 2700 Baud
#define DATARATE_VALUE RFM12_DATARATE_CALC_HIGH(9600.0)

//use this for 340 Baud < datarate < 2700 Baud
//#define DATARATE_VALUE RFM12_DATARATE_CALC_LOW(1200.0)

//TX BUFFER SIZE
#define RFM12_TX_BUFFER_SIZE 30

//RX BUFFER SIZE (there are going to be 2 Buffers of this size for double_buffering)
#define RFM12_RX_BUFFER_SIZE 30


/************************
 * INTERRUPT VECTOR
 * set the name for the interrupt vector here
 */
 
//the interrupt vector
#define RFM12_INT_VECT (INT1_vect)

//the interrupt mask register
#define RFM12_INT_MSK GICR

//the interrupt bit in the mask register
#define RFM12_INT_BIT (INT1)

//the interrupt flag register
#define RFM12_INT_FLAG GIFR

//the interrupt bit in the flag register
#define RFM12_FLAG_BIT (INTF1)

//setup the interrupt to trigger on negative edge
#define RFM12_INT_SETUP()   MCUCR |= (1<<ISC11)


/************************
 * FEATURE CONFIGURATION
 */

#define RFM12_LIVECTRL 0
#define RFM12_NORETURNS 0
#define RFM12_NOCOLLISIONDETECTION 0
#define RFM12_TRANSMIT_ONLY 0
#define RFM12_SPI_SOFTWARE 0
#define RFM12_USE_POLLING 0
#define RFM12_RECEIVE_ASK 0
#define RFM12_TRANSMIT_ASK 0
#define RFM12_USE_WAKEUP_TIMER 1
#define RFM12_LOW_POWER 0
#define RFM12_USE_CLOCK_OUTPUT 1


#define RFM12_LBD_VOLTAGE             RFM12_LBD_VOLTAGE_3V0


#define RFM12_CLOCK_OUT_FREQUENCY     RFM12_CLOCK_OUT_FREQUENCY_1_00_MHz

/* use a callback function that is called directly from the
 * interrupt routine whenever there is a data packet available. When
 * this value is set to 1, you must use the function
 * "rfm12_set_callback(your_function)" to point to your
 * callback function in order to receive packets.
 */
#define RFM12_USE_RX_CALLBACK 0


/************************
 * RFM12BP support (high power version of RFM12)
 */

//To use RFM12BP, which needs control signals for RX enable and TX enable,
//use these defines (set to your pinout of course).
//The TX-Part can also be used to control a TX-LED with the nomral RFM12

/*
	#define RX_INIT_HOOK  DDRA |= _BV(PA1)
	#define RX_LEAVE_HOOK PORTA &= ~_BV(PA1)
	#define RX_ENTER_HOOK PORTA |= _BV(PA1)
	
	#define TX_INIT_HOOK  DDRA |= _BV(PA2)
	#define TX_LEAVE_HOOK PORTA &= ~_BV(PA2)
	#define TX_ENTER_HOOK PORTA |= _BV(PA2)
*/

/************************
 * UART DEBUGGING
 * en- or disable debugging via uart.
 */
 
#define RFM12_UART_DEBUG 0