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


/** \file rfm12.c
 * \brief rfm12 library main file
 * \author Hans-Gert Dahmen
 * \author Peter Fuhrmann
 * \author Soeren Heisrath
 * \version 1.2
 * \date 2012/01/22
 *
 * All core functionality is implemented within this file.
 */


/************************
 * standard includes
*/
#ifdef __PLATFORM_AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif

#include <string.h>



/************************
 * library internal includes
 * the order in which they are included is important
*/
#include "include/rfm12_hw.h"
#include "include/rfm12_defaults.h"
#include "include/rfm12_core.h"
#include "rfm12.h"

//for uart debugging
#if RFM12_UART_DEBUG
	#include "../examples/uart_lib/uart.h"
#endif

#if RFM12_USE_RX_CALLBACK
	volatile static (*rfm12_rx_callback_func)(uint8_t, uint8_t *) = (void *)0x0000;
	void rfm12_set_callback ((*in_func)(uint8_t, uint8_t *)) {
		rfm12_rx_callback_func = in_func;
	}
#endif


/************************
 * library internal globals
*/

//! Buffer and status for packet transmission.
rf_tx_buffer_t rf_tx_buffer;

//if receive mode is not disabled (default)
#if !(RFM12_TRANSMIT_ONLY)
	//! Buffers and status to receive packets.
	rf_trx_buffer_t rf_rx_buffers[2];
#endif /* RFM12_TRANSMIT_ONLY */

//! Global control and status.
rfm12_control_t ctrl;


/************************
 * load other core and external components
 * (putting them directly into here allows GCC to optimize better)
*/

/* include spi functions into here */
#include "include/rfm12_spi.c"
#include "include/rfm12_spi_linux.c"

/*
 * include control / init functions into here
 * all of the stuff in there is optional, so there's no code-bloat.
*/
#define RFM12_LIVECTRL_HOST 1//if we are buliding for the microcontroller, we are the host.
#include "include/rfm12_livectrl.c"

/*
 * include extra features here
 * all of the stuff in there is optional, so there's no code-bloat..
*/
#include "include/rfm12_extra.c"


/************************
 * Begin of library
*/


//! Interrupt handler to handle all transmit and receive data transfers to the rfm12.
/** The receiver will generate an interrupt request (IT) for the
* microcontroller - by pulling the nIRQ pin low - on the following events:
* - The TX register is ready to receive the next byte (RGIT)
* - The FIFO has received the preprogrammed amount of bits (FFIT)
* - Power-on reset (POR)
* - FIFO overflow (FFOV) / TX register underrun (RGUR)
* - Wake-up timer timeout (WKUP)
* - Negative pulse on the interrupt input pin nINT (EXT)
* - Supply voltage below the preprogrammed value is detected (LBD)
*
* The rfm12 status register is read to determine which event has occured.
* Reading the status register will clear the event flags.
*
* The interrupt handles the RGIT and FFIT events by default.
* Upon specific configuration of the library the WKUP and LBD events
* are handled additionally.
*
* \see rfm12_control_t, rf_rx_buffer_t and rf_tx_buffer_t
*/
//if polling is used, do not define an interrupt handler, but a polling function
#if (RFM12_USE_POLLING)
void rfm12_poll(void)
#else
ISR(RFM12_INT_VECT, ISR_NOBLOCK)
#endif
{
	uint8_t interrupt_high_inactive = nIRQ_PIN & _BV(BIT_nIRQ); // Masked input of interrupt pin for PCINT
	if(interrupt_high_inactive)return;
	RFM12_INT_OFF();
	uint8_t status;
	uint8_t recheck_interrupt  = 1;
	//if receive mode is not disabled (default)
	#if !(RFM12_TRANSMIT_ONLY)
		static uint8_t checksum; //static local variables produce smaller code size than globals
	#endif /* !(RFM12_TRANSMIT_ONLY) */

	//if we use at least one of the status bits, we need to check the status again
	//for the case in which another interrupt condition occured *while* we were handeling
	//the first one.
	while(recheck_interrupt){
		//clear AVR int flag
#ifdef __PLATFORM_AVR__
		RFM12_INT_FLAG = (1<<RFM12_FLAG_BIT);
#endif

		//first we read the first byte of the status register
		//to get the interrupt flags
		status = rfm12_read_int_flags_inline();

		// This is set for any interrupt handled in this ISR
		// It will cause it to recheck before exiting the function
		recheck_interrupt = status &
				(RFM12_STATUS_LBD | RFM12_STATUS_WKUP | RFM12_STATUS_FFIT) >> 8;

        UART_DEBUG_PUTC('S');
        UART_DEBUG_PUTC(status);

		//low battery detector feature
		#if RFM12_LOW_BATT_DETECTOR
			if (status & (RFM12_STATUS_LBD >> 8)) {
				//debug
                UART_DEBUG_PUTC('L');

				//set status variable to low battery
				ctrl.low_batt = RFM12_BATT_LOW;
			}
		#endif /* RFM12_LOW_BATT_DETECTOR */

		//wakeup timer feature
		#if RFM12_USE_WAKEUP_TIMER
			if (status & (RFM12_STATUS_WKUP >> 8)) {
				//debug
                UART_DEBUG_PUTC('W');

				ctrl.wkup_flag = 1;
			}
			if (status & ((RFM12_STATUS_WKUP | RFM12_STATUS_FFIT) >> 8) ) {
				//restart the wakeup timer by toggling the bit on and off
				rfm12_data(ctrl.pwrmgt_shadow & ~RFM12_PWRMGT_EW);
				rfm12_data(ctrl.pwrmgt_shadow);
			}
		#endif /* RFM12_USE_WAKEUP_TIMER */

		//check if the fifo interrupt occurred
		if (status & (RFM12_STATUS_FFIT>>8)) {
			uint8_t checksum_fail = 0;
	        /********************
	        ** NEXT STATE LOGIC *
	        ********************/
	        //Uses the current flags and buffer information to decide what to do
			switch (ctrl.rfm12_state) {
			case STATE_RX_IDLE: //Same next state logic as RX_ACTIVE
			case STATE_RX_ACTIVE:
#ifndef DISABLE_CHECKSUMM
				//check header against checksum
				if (ctrl.bytecount == 3 && checksum != 0xff) {
					//if the checksum does not match, reset the fifo
					checksum_fail = 1;
				}
#endif
				//Check there is space to accept AND that is not a checksum failure
				if (rf_rx_buffers[ctrl.buffer_in_num].status == STATUS_FREE
						&& !checksum_fail) {
					ctrl.rfm12_state = STATE_RX_ACTIVE;
				}
				else{
					ctrl.rfm12_state = STATE_RX_IDLE;
				}
				break;
			case STATE_TX_RESET:
				ctrl.rfm12_state = STATE_RX_IDLE;
				break;
			case STATE_TX_END:
				ctrl.rfm12_state = STATE_TX_RESET;
				break;
			case STATE_TX:
				ctrl.rfm12_state = STATE_TX_END;
				if (ctrl.bytecount < ctrl.num_bytes && ctrl.bytecount <RFM12_TX_BUFFER_SIZE+6) {
					//Stay in TX mode if there are more bytes to TX
					ctrl.rfm12_state = STATE_TX;
				}
				break;
			default:
				ctrl.rfm12_state = STATE_RX_IDLE;
			}

			/********************
			** STATE OPERATION **
			********************/
			switch (ctrl.rfm12_state) {
			//if receive mode is not disabled (default)
			#if !(RFM12_TRANSMIT_ONLY)
			case STATE_RX_ACTIVE:
				//Read a byte from the radio and take checksum
				uint8_t data;
				data = rfm12_read(RFM12_CMD_READ);
				checksum ^= data;

				//debug
				UART_DEBUG_PUTC('R');
				UART_DEBUG_PUTC(data);

				//Write the byte into the receive data structure with overflow check
				if (ctrl.bytecount == 0){
					//Specially check length byte on write
					if(data>RFM12_TRX_FRAME_SIZE + RFM12_TRX_OVERHEAD){
						rf_rx_buffers[ctrl.buffer_in_num].len=RFM12_TRX_FRAME_SIZE + RFM12_TRX_OVERHEAD;
					}
					else{
						rf_rx_buffers[ctrl.buffer_in_num].len = data;
					}
				}
				else{
					rf_rx_buffers[ctrl.buffer_in_num].buffer[ctrl.bytecount] = data;
				}
				ctrl.bytecount++;
				//Check to see if bytecount pos is at the length, if so, finished
				if(rf_rx_buffers[ctrl.buffer_in_num].len <= ctrl.bytecount){
					/* if we're here, receiving is done */
					/* the FIFO will need to be be reset by idle state */
					//debug
                    UART_DEBUG_PUTC('D');

					rf_rx_buffers[ctrl.buffer_in_num].status = STATUS_OCCUPIED;
					//switch to other buffer
					ctrl.buffer_in_num ^= 1;

					//Set to the idle state so the FIFO is reset.
					ctrl.rfm12_state = STATE_RX_IDLE;
				}
				break;
			#endif /* !(RFM12_TRANSMIT_ONLY) */
			case STATE_TX:
                //Send byte to radio
				rfm12_data( RFM12_CMD_TX | rf_tx_buffer.sync[ctrl.bytecount]);
				ctrl.bytecount++;

                UART_DEBUG_PUTC('T');
				break;
			case STATE_TX_END:
				// Load a dummy byte to allow radio to send what is in TX double buffer
				rfm12_data( RFM12_CMD_TX | 0xaa);
				break;
			case STATE_TX_RESET:
				//Finished transmitting the bytes so get ready to go back to RX_IDLE mode
				//Transmitter on RFM12BP off
				#ifdef TX_LEAVE_HOOK
					TX_LEAVE_HOOK;
				#endif

				//flag the buffer as free again
				ctrl.txstate = STATUS_FREE;

				//turn off the transmitter and enable receiver
				//the receiver is not enabled in transmit only mode (by PWRMGT_RECEIVE makro)
				#if RFM12_PWRMGT_SHADOW
					ctrl.pwrmgt_shadow &= ~(RFM12_PWRMGT_ET); /* disable transmitter */
					ctrl.pwrmgt_shadow |= (PWRMGT_RECEIVE);   /* activate predefined receive mode */
					rfm12_data(ctrl.pwrmgt_shadow);
				#else /* no RFM12_PWRMGT_SHADOW */
					rfm12_data( PWRMGT_RECEIVE );
				#endif /* RFM12_PWRMGT_SHADOW */

				//Receiver on RFM12BP on
				#ifdef RX_ENTER_HOOK
					RX_ENTER_HOOK;
				#endif
				//load a dummy byte to clear int status
				rfm12_data( RFM12_CMD_TX | 0xaa);

				//Set to the idle state so the FIFO is reset.
				ctrl.rfm12_state = STATE_RX_IDLE;
				break;
			}

			// The state may be set to IDLE by other states.
			// So run it after the others.
			if(ctrl.rfm12_state == STATE_RX_IDLE){
				// Reset buffer Variable
				ctrl.bytecount = 0;
				ctrl.num_bytes = 0;

				//reset the receiver fifo, if receive mode is not disabled (default)
				#if !(RFM12_TRANSMIT_ONLY)
					// Reset checksum
					checksum = 0;

					// Reset FIFO
					UART_DEBUG_PUTC('F');
					rfm12_data( RFM12_CMD_FIFORESET | CLEAR_FIFO_INLINE);
					rfm12_data( RFM12_CMD_FIFORESET | ACCEPT_DATA_INLINE);
				#endif /* !(RFM12_TRANSMIT_ONLY) */
			}
		}
	}

    UART_DEBUG_PUTC('E');

	//turn the int back on
	RFM12_INT_ON();
}


//! The tick function implements collision avoidance and initiates transmissions.
/** This function has to be called periodically.
* It will read the rfm12 status register to check if a carrier is being received,
* which would indicate activity on the chosen radio channel. \n
* If there has been no activity for long enough, the channel is believed to be free.
*
* When there is a packet waiting for transmission and the collision avoidance
* algorithm indicates that the air is free, then the interrupt control variables are
* setup for packet transmission and the rfm12 is switched to transmit mode.
* This function also fills the rfm12 tx fifo with a preamble.
*
* \warning Warning, if you do not call this function periodically, then no packet will get transmitted.
* \see rfm12_tx() and rfm12_queue_tx()
*/
void rfm12_tick(void) {
	//collision detection is enabled by default
	#if !(RFM12_NOCOLLISIONDETECTION)
		uint16_t status;

		//start with a channel free count of 16, this is necessary for the ASK receive feature to work
		static uint8_t channel_free_count = 16; //static local variables produce smaller code size than globals
	#endif

	//debug
	#if RFM12_UART_DEBUG
		static uint8_t oldstate;
		uint8_t state = ctrl.rfm12_state;
		if (oldstate != state) {
			uart_putstr ("mode change: ");
			switch (state) {
				case STATE_RX_IDLE:
					uart_putc ('i');
					break;
				case STATE_RX_ACTIVE:
					uart_putc ('r');
					break;
				case STATE_TX:
					uart_putc ('t');
					break;
				default:
					uart_putc ('?');
			}
			uart_putstr ("\r\n");
			oldstate = state;
		}
	#endif

	//don't disturb RFM12 if transmitting or receiving
	if (ctrl.rfm12_state != STATE_RX_IDLE) {
		return;
	}

	//collision detection is enabled by default
	#if !(RFM12_NOCOLLISIONDETECTION)
		//disable the interrupt (as we're working directly with the transceiver now)
		//hint: we could be losing an interrupt here, because we read the status register.
		//this applys for the Wakeup timer, as it's flag is reset by reading.
		RFM12_INT_OFF();
		status = rfm12_read(RFM12_CMD_STATUS);
		RFM12_INT_ON();

		//wakeup timer workaround (if we don't restart the timer after timeout, it will stay off.)
		#if RFM12_USE_WAKEUP_TIMER
			if (status & (RFM12_STATUS_WKUP)) {
				ctrl.wkup_flag = 1;

				RFM12_INT_OFF();
				//restart the wakeup timer by toggling the bit on and off
				rfm12_data(ctrl.pwrmgt_shadow & ~RFM12_PWRMGT_EW);
				rfm12_data(ctrl.pwrmgt_shadow);
				RFM12_INT_ON();
			}
		#endif /* RFM12_USE_WAKEUP_TIMER */

		//check if we see a carrier
		if (status & RFM12_STATUS_RSSI) {
			//yes: reset free counter and return
			channel_free_count = CHANNEL_FREE_TIME;
			return;
		}
		//no

		//is the channel free long enough ?
		if (channel_free_count != 0) {
			//no:
			channel_free_count--; // decrement counter
			return;
		}
		//yes: we can begin transmitting
	#endif

	//do we have something to transmit?
	if (ctrl.txstate == STATUS_OCCUPIED) { //yes: start transmitting
		rfm12_start_tx();
	}
}

//! Start the buffered packet transmission
void rfm12_start_tx() {
	//disable the interrupt (as we're working directly with the transceiver now)
	//we won't loose interrupts, as the AVR caches them in the int flag.
	//we could disturb an ongoing reception,
	//if it has just started some cpu cycles ago
	//(as the check for this case is some lines (cpu cycles) above)
	//anyhow, we MUST transmit at some point...
	RFM12_INT_OFF();

	//disable receiver - if you don't do this, tx packets will get lost
	//as the fifo seems to be in use by the receiver

	#if RFM12_PWRMGT_SHADOW
		ctrl.pwrmgt_shadow &= ~(RFM12_PWRMGT_ER); /* disable receiver */
		rfm12_data(ctrl.pwrmgt_shadow);
	#else
		rfm12_data(RFM12_CMD_PWRMGT | PWRMGT_DEFAULT ); /* disable receiver */
	#endif

	//RFM12BP receiver off
	#ifdef RX_LEAVE_HOOK
		RX_LEAVE_HOOK;
	#endif

	//calculate number of bytes to be sent by ISR
	//2 sync bytes + header_overhead + message length
	ctrl.num_bytes =  2 + RFM12_TRX_OVERHEAD + rf_tx_buffer.len;

	//reset byte sent counter
	ctrl.bytecount = 0;

	//set mode for interrupt handler
	ctrl.rfm12_state = STATE_TX;

	//RFM12BP transmitter on
	#ifdef TX_ENTER_HOOK
		TX_ENTER_HOOK;
	#endif

	//fill 2byte 0xAA preamble into data register
	//the preamble helps the receivers AFC circuit to lock onto the exact frequency
	//(hint: the tx FIFO [if el is enabled] is two staged, so we can safely write 2 bytes before starting)
	rfm12_data(RFM12_CMD_TX | PREAMBLE);
	rfm12_data(RFM12_CMD_TX | PREAMBLE);

	//set ET in power register to enable transmission (hint: TX starts now)
	#if RFM12_PWRMGT_SHADOW
		ctrl.pwrmgt_shadow |= RFM12_PWRMGT_ET;
		rfm12_data (ctrl.pwrmgt_shadow);
	#else
		rfm12_data(RFM12_CMD_PWRMGT | PWRMGT_DEFAULT | RFM12_PWRMGT_ET);
	#endif

	//enable the interrupt to continue the transmission
	RFM12_INT_ON();
}
//! Enqueue an already buffered packet for transmission
/** If there is no active transmission, the packet header is written to the
* transmission control buffer and the packet will be enqueued for transmission. \n
* This function is not responsible for buffering the actual packet data.
* The data has to be copied into the transmit buffer beforehand,
* which can be accomplished by the rfm12_tx() function.
*
* \note Note that this function does not start the transmission, it merely enqueues the packet. \n
* Transmissions are started by rfm12_tick().
* \param [type] The packet header type field
* \param [length] The packet data length
* \returns One of these defines: \ref tx_retvals "TX return values"
* \see rfm12_tx() and rfm12_tick()
*/
#if (RFM12_NORETURNS)
void
#else
uint8_t
#endif
rfm12_queue_tx(uint8_t type, uint8_t length) {
	//exit if the buffer isn't free
	if (ctrl.txstate != STATUS_FREE)
		return TXRETURN(RFM12_TX_OCCUPIED);

	//write airlab header to buffer
	rf_tx_buffer.len = length;
	rf_tx_buffer.type = type;
	rf_tx_buffer.checksum = length ^ type ^ 0xff;

	//schedule packet for transmission
	ctrl.txstate = STATUS_OCCUPIED;

	return TXRETURN(RFM12_TX_ENQUEUED);
}


//! Copy a packet to the buffer and call rfm12_queue_tx() to enqueue it for transmission.
/** If there is no active transmission, the buffer contents will be copied to the
* internal transmission buffer. Finally the buffered packet is going to be enqueued by
* calling rfm12_queue_tx(). If automatic buffering of packet data is not necessary,
* which is the case when the packet data does not change while the packet is enqueued
* for transmission, then one could directly store the data in \ref rf_tx_buffer
* (see rf_tx_buffer_t) and use the rfm12_queue_tx() function.
*
* \note Note that this function does not start the transmission, it merely enqueues the packet. \n
* Transmissions are started by rfm12_tick().
* \param [len] The packet data length
* \param [type] The packet header type field
* \param [data] Pointer to the packet data
* \returns One of these defines: \ref tx_retvals "TX return values"
* \see rfm12_start_tx() and rfm12_tick()
*/
#if !(RFM12_SMALLAPI)
	#if (RFM12_NORETURNS)
	void
	#else
	uint8_t
	#endif
	rfm12_tx(uint8_t len, uint8_t type, uint8_t *data) {
		#if RFM12_UART_DEBUG
			uart_putstr ("sending packet\r\n");
		#endif

		if (len > RFM12_TX_BUFFER_SIZE) return TXRETURN(RFM12_TX_ERROR);

		//exit if the buffer isn't free
		if (ctrl.txstate != STATUS_FREE)
			return TXRETURN(RFM12_TX_OCCUPIED);

		memcpy(rf_tx_buffer.buffer, data, len);

		#if (!(RFM12_NORETURNS))
		return rfm12_queue_tx(type, len);
		#else
		rfm12_queue_tx(type, len);
		#endif
	}
#endif /* RFM12_SMALLAPI */


//if receive mode is not disabled (default)
#if !(RFM12_TRANSMIT_ONLY)
	//! Function to clear buffer complete/occupied status.
	/** This function will set the current receive buffer status to free and switch
	* to the other buffer, which can then be read using rfm12_rx_buffer().
	*
	* \see rfm12_rx_status(), rfm12_rx_len(), rfm12_rx_type(), rfm12_rx_buffer() and rf_rx_buffers
	*/
	//warning: without the attribute, gcc will inline this even if -Os is set
	void __attribute__((noinline)) rfm12_rx_clear(void) {
			//mark the current buffer as empty
			rf_rx_buffers[ctrl.buffer_out_num].status = STATUS_FREE;

			//switch to the other buffer
			ctrl.buffer_out_num ^= 1;

	}
#endif /* !(RFM12_TRANSMIT_ONLY) */


//enable internal data register and fifo
//setup selected band
#define RFM12_CMD_CFG_DEFAULT   (RFM12_CMD_CFG | RFM12_CFG_EL | RFM12_CFG_EF | RFM12_BASEBAND | RFM12_XTAL_LOAD)

//set rx parameters: int-in/vdi-out pin is vdi-out,
//Bandwith, LNA, RSSI
#define RFM12_CMD_RXCTRL_DEFAULT (RFM12_CMD_RXCTRL | RFM12_RXCTRL_P16_VDI | RFM12_RXCTRL_VDI_FAST | RFM12_FILTER_BW | RFM12_LNA_GAIN | RFM12_RSSI_THRESHOLD )

//set AFC to automatic, (+4 or -3)*2.5kHz Limit, fine mode, active and enabled
#define RFM12_CMD_AFC_DEFAULT  (RFM12_CMD_AFC | RFM12_AFC_AUTO_KEEP | RFM12_AFC_LIMIT_4 | RFM12_AFC_FI | RFM12_AFC_OE | RFM12_AFC_EN)

//set TX Power, frequency shift
#define RFM12_CMD_TXCONF_DEFAULT  (RFM12_CMD_TXCONF | RFM12_POWER | RFM12_TXCONF_FS_CALC(FSK_SHIFT) )

#ifdef __PLATFORM_AVR__
static const uint16_t init_cmds[] PROGMEM = {
#else
static const uint16_t init_cmds[] = {
#endif
	//defined above (so shadow register is inited with same value)
	RFM12_CMD_CFG_DEFAULT,

	//set power default state (usually disable clock output)
	//do not write the power register two times in a short time
	//as it seems to need some recovery
	(RFM12_CMD_PWRMGT | PWRMGT_DEFAULT),

	//set frequency
	(RFM12_CMD_FREQUENCY | RFM12_FREQUENCY_CALC(RFM12_FREQUENCY) ),

	//set data rate
	(RFM12_CMD_DATARATE | DATARATE_VALUE ),

	//defined above (so shadow register is inited with same value)
	RFM12_CMD_RXCTRL_DEFAULT,

	//automatic clock lock control(AL), digital Filter(!S),
	//Data quality detector value 3, slow clock recovery lock
	(RFM12_CMD_DATAFILTER | RFM12_DATAFILTER_AL | 3),

	//2 Byte Sync Pattern, Start fifo fill when sychron pattern received,
	//disable sensitive reset, Fifo filled interrupt at 8 bits
	(RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR | (8 << 4)),

	//defined above (so shadow register is inited with same value)
	RFM12_CMD_AFC_DEFAULT,

	//defined above (so shadow register is inited with same value)
	RFM12_CMD_TXCONF_DEFAULT,

	//disable low dutycycle mode
	(RFM12_CMD_DUTYCYCLE),

	//disable wakeup timer
	(RFM12_CMD_WAKEUP),

	//enable rf receiver chain, if receiving is not disabled (default)
	//the magic is done via defines
	(RFM12_CMD_PWRMGT | PWRMGT_RECEIVE),
};

//! This is the main library initialization function
/**This function takes care of all module initialization, including:
* - Setup of the used frequency band and external capacitor
* - Setting the exact frequency (channel)
* - Setting the transmission data rate
* - Configuring various module related rx parameters, including the amplification
* - Enabling the digital data filter
* - Enabling the use of the modules fifo, as well as enabling sync pattern detection
* - Configuring the automatic frequency correction
* - Setting the transmit power
*
* This initialization function also sets up various library internal configuration structs and
* puts the module into receive mode before returning.
*
* \note Please note that the transmit power and receive amplification values are currently hard coded.
* Have a look into rfm12_hw.h for possible settings.
*/
void rfm12_init(void) {
	//initialize spi
#ifdef __PLATFORM_AVR__
	SS_RELEASE();
	DDR_SS |= (1<<BIT_SS);
#endif
	spi_init();

	//typically sets DDR registers for RFM12BP TX/RX pin
	#ifdef TX_INIT_HOOK
		TX_INIT_HOOK;
	#endif

	#ifdef RX_INIT_HOOK
		RX_INIT_HOOK;
	#endif

	//store the syncronization pattern to the transmission buffer
	//the sync pattern is used by the receiver to distinguish noise from real transmissions
	//the sync pattern is hardcoded into the receiver
	rf_tx_buffer.sync[0] = SYNC_MSB;
	rf_tx_buffer.sync[1] = SYNC_LSB;

	//if receive mode is not disabled (default)
	#if !(RFM12_TRANSMIT_ONLY)
		//init buffer pointers
		ctrl.buffer_in_num = 0;
		ctrl.buffer_out_num = 0;
	#endif /* !(RFM12_TRANSMIT_ONLY) */

	//low battery detector feature initialization
	#if RFM12_LOW_BATT_DETECTOR
		ctrl.low_batt = RFM12_BATT_OKAY;
	#endif /* RFM12_LOW_BATT_DETECTOR */

	#if RFM12_PWRMGT_SHADOW
		//set power management shadow register to receiver chain enabled or disabled
		//the define correctly handles the transmit only mode
		ctrl.pwrmgt_shadow = (RFM12_CMD_PWRMGT | PWRMGT_RECEIVE);
	#endif


	#if RFM12_LIVECTRL
		//init shadow registers with values about to be written to rfm12
		ctrl.rxctrl_shadow = RFM12_CMD_RXCTRL_DEFAULT;
		ctrl.afc_shadow = RFM12_CMD_AFC_DEFAULT;
		ctrl.txconf_shadow = RFM12_CMD_TXCONF_DEFAULT;
		ctrl.cfg_shadow =    RFM12_CMD_CFG_DEFAULT;
	#endif

	//write all the initialisation values to rfm12
	uint8_t x;

	#ifdef __PLATFORM_AVR__

		for (x = 0; x < ( sizeof(init_cmds) / 2) ; x++) {
			rfm12_data(pgm_read_word(&init_cmds[x]));
		}
	#else
		for (x = 0; x < ( sizeof(init_cmds) / 2) ; x++) {
			rfm12_data(init_cmds[x]);
		}
	#endif

	#ifdef RX_ENTER_HOOK
		RX_ENTER_HOOK;
	#endif

	#if RFM12_USE_CLOCK_OUTPUT || RFM12_LOW_BATT_DETECTOR
		rfm12_data(RFM12_CMD_LBDMCD | RFM12_LBD_VOLTAGE | RFM12_CLOCK_OUT_FREQUENCY ); //set low battery detect, clock output
	#endif

	//ASK receive mode feature initialization
	#if RFM12_RECEIVE_ASK
		adc_init();
	#endif

	//setup interrupt for falling edge trigger
#ifdef __PLATFORM_AVR__
	RFM12_INT_SETUP();
#endif

	//clear int flag
	rfm12_read(RFM12_CMD_STATUS);

#ifdef __PLATFORM_AVR__
	RFM12_INT_FLAG = (1<<RFM12_FLAG_BIT);
#endif

	//init receiver fifo, we now begin receiving.
	rfm12_data(CLEAR_FIFO);
	rfm12_data(ACCEPT_DATA);

	//activate the interrupt
	RFM12_INT_ON();
}
