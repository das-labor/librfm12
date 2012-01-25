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
 
/** \file rfm12_ctrl.h
 * \brief rfm12 library live control feature header
 * \author Soeren Heisrath
 * \author Hans-Gert Dahmen
 * \author Peter Fuhrmann
 * \version 0.9.0
 * \date 08.09.09
 *
 * This header declares all functions necessary for setting the baud rate and frquency.
 *
 * \note It is usually not required to explicitly include this header,
 * as this is already done by rfm12.h.
 * \todo Add more livectrl functions.
 */
 
#ifndef _RFM12_CTRL_H
#define _RFM12_CTRL_H


/************************
* function protoypes	
*/
#if RFM12_LIVECTRL
#ifndef RFM12_NUM_DELAYED_COMMANDS
	#define RFM12_NUM_DELAYED_COMMANDS 4 /* defines how many commands to store for delayed execution */
#endif
//see rfm12_ctrl.c for more documentation
void rfm12_set_rate (uint16_t in_datarate);
void rfm12_set_band (uint16_t in_band);
void rfm12_set_frequency (uint16_t in_freq);
void rfm12_set_tx_power (uint8_t in_power);
void rfm12_set_rssi (uint8_t in_val);
void rfm12_set_frequency_khz (uint16_t in_freq);
uint16_t rfm12_sendcommand (uint16_t cmd, uint16_t in_payload);
void rfm12_set_control_register (rfm12_control_t *);
/* called at the end of the interrupt/polling function in order to not interfere with
 * other transactions.
 */
void rfm12_data_delayed (uint8_t in_op, uint16_t in_cmd);
#endif /* RFM12_LIVECTRL */

#endif /* _RFM12_EXTRA_H */