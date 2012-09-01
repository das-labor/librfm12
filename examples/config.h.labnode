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

#define UART_BAUD_RATE 115200
#define UART_HEXDUMP

/************************
 * Debug LED for examples
 */

#define LED_PORT  PORTB
#define LED_DDR    DDRB
#define LED_BIT     PB0


/************************
 * RFM12 PIN DEFINITIONS
 */

//Pin that the RFM12's slave select is connected to
#define DDR_SS DDRB
#define PORT_SS PORTB
#define BIT_SS 3

//SPI port
#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define PIN_SPI PINB
#define BIT_MOSI 5
#define BIT_MISO 6
#define BIT_SCK  7
#define BIT_SPI_SS 4
//this is the hardware SS pin of the AVR - it 
//needs to be set to output for the spi-interface to work 
//correctly, independently of the CS pin used for the RFM12

#define RFM12_SPI_SOFTWARE 0

/************************
 * RFM12 CONFIGURATION OPTIONS
 */

//baseband of the module (either RFM12_BAND_433, RFM12_BAND_868 or RFM12_BAND_912)
#define RFM12_BASEBAND RFM12_BAND_433

//center frequency to use (+- FSK frequency shift)
#define RFM12_FREQUENCY       433170000UL

//Transmit FSK frequency shift in kHz
#define FSK_SHIFT             125000

//Receive RSSI Threshold
#define RFM12_RSSI_THRESHOLD  RFM12_RXCTRL_RSSI_79

//Receive Filter Bandwidth
#define RFM12_FILTER_BW       RFM12_RXCTRL_BW_400

//Output power relative to maximum (0dB is maximum)
#define RFM12_POWER           RFM12_TXCONF_POWER_0

//Receive LNA gain
#define RFM12_LNA_GAIN        RFM12_RXCTRL_LNA_6

//crystal load capacitance - the frequency can be verified by measuring the
//clock output of RFM12 and comparing to 1MHz.
//11.5pF seems to be o.k. for RFM12, and 10.5pF for RFM12BP, but this may vary.
#define RFM12_XTAL_LOAD       RFM12_XTAL_11_5PF

//use this for datarates >= 2700 Baud
#define DATARATE_VALUE        RFM12_DATARATE_CALC_HIGH(9600.0)

//use this for 340 Baud < datarate < 2700 Baud
//#define DATARATE_VALUE      RFM12_DATARATE_CALC_LOW(1200.0)

//TX BUFFER SIZE
#define RFM12_TX_BUFFER_SIZE  30

//RX BUFFER SIZE (there are going to be 2 Buffers of this size for double_buffering)
#define RFM12_RX_BUFFER_SIZE  30


/************************
 * RFM12 INTERRUPT VECTOR
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
 * RFM12 CLOCK OUTPUT
 */

#define RFM12_USE_CLOCK_OUTPUT        1 
#define RFM12_CLOCK_OUT_FREQUENCY     RFM12_CLOCK_OUT_FREQUENCY_1_00_MHz
