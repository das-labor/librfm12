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


 /** \file rfm12_core.h
  * \brief rfm12 constants and macros
  * \author Hans-Gert Dahmen
  * \author Peter Fuhrmann
  * \author Soeren Heisrath
  * \version 0.9.1
  * \date 2019/09/10
  *
  * This header contains rfm12 specific constants
  * and macros that help operate the radio.
  */

#ifndef _RFM12_CORE_H
#define _RFM12_CORE_H

/************************
* VARIOUS RFM RELATED DEFINES FOR INTERNAL USE	
*(defines which shall be visible to the user are located in rfm12.h)
*/

//default preamble (altrernating 1s and 0s)
#define PREAMBLE 0xAA

//default synchronization pattern
#define SYNC_MSB 0x2D
#define SYNC_LSB 0xD4

//these are the states for the receive/transmit state machine
#define STATE_RX_IDLE 0
#define STATE_RX_ACTIVE 1
#define STATE_TX 2
#define STATE_POWER_DOWN 3


//packet header length in bytes
#define PACKET_OVERHEAD 3

/************************
* HELPER MACROS
*/

//macros to turn the int on and off
//if polling is used, just define these macros as empty
#if !(RFM12_USE_POLLING)
	#define RFM12_INT_ON() RFM12_INT_MSK |= (1<<RFM12_INT_BIT)
	#define RFM12_INT_OFF() RFM12_INT_MSK &= ~(1<<RFM12_INT_BIT)
#else
	#define RFM12_INT_ON()
	#define RFM12_INT_OFF()
#endif /* !(RFM12_USE_POLLING) */

/*
 * the following macros help to manage the rfm12 fifo
 * default fiforeset is as follows:
 * 2 Byte Sync Pattern, disable sensitive reset, fifo filled interrupt at 8 bits
 */
//default fiforeset register value to accept data
#define ACCEPT_DATA (RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR | (8<<4) | RFM12_FIFORESET_FF)
#define ACCEPT_DATA_INLINE (RFM12_FIFORESET_DR | (8<<4) | RFM12_FIFORESET_FF)
//default fiforeset register value to clear fifo
#define CLEAR_FIFO (RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR | (8<<4))
#define CLEAR_FIFO_INLINE (RFM12_FIFORESET_DR | (8<<4))

//this macro helps to encapsulate the return values, when noreturn is set to on
#if (RFM12_NORETURNS)
	#define TXRETURN(a)
#else
	#define TXRETURN(a) (a)
#endif

#endif /* _RFM12_CORE_H */
