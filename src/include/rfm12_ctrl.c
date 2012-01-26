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

/** \file rfm12_ctrl.c
 * \brief rfm12 library live control feature source
 * \author Soeren Heisrath
 * \author Hans-Gert Dahmen
 * \author Peter Fuhrmann
 * \version 0.9.0
 * \date 08.09.09
 *
 * This file implements all functions necessary for setting the baud rate and frquency.
 *
 * \note This file is included directly by rfm12.c, for performance reasons.
 * \todo Add more livectrl functions.
 */
 
/******************************************************
 *    THIS FILE IS BEING INCLUDED DIRECTLY		*
 *		(for performance reasons)				*
 ******************************************************/


#if RFM12_LIVECTRL


void rfm12_data_safe(uint16_t d){
	//disable the interrupt (as we're working directly with the transceiver now)
	RFM12_INT_OFF();
	rfm12_data(d);
	RFM12_INT_ON();
}

//! Set the data rate of the rf12.
/** The data rate has to be specified using the following macros:
* - RFM12_DATARATE_CALC_HIGH(x) for rates >= 2700 Baud
* - RFM12_DATARATE_CALC_LOW(x) for rates from 340 to < 2700 Baud
*
* Please refer to the rfm12 library configuration header for a demo macro usage. \n
* The data rate calculation macros can be found in  rfm12_hw.h.
* They are not included as a function for code-size reasons.
*/
void rfm12_set_rate (uint16_t in_datarate)
{
	rfm12_data_safe( RFM12_CMD_DATARATE | in_datarate );
}

//! Set the frequency of the rf12.
/** The frequency has to be specified using the RFM12_FREQUENCY_CALC_433(x) macro.
*
* Please refer to the rfm12 library configuration header for a demo macro usage. \n
* The frequency calculation macro can be found in  rfm12_hw.h.
* It is not included as a function for code-size reasons.
*/
void rfm12_set_frequency (uint16_t in_freq)
{
	rfm12_data_safe( RFM12_CMD_FREQUENCY | in_freq );
}

//! Set the receive RSSI threshold
/** set the rssi value. this can be done with either one of the macros defined
 * in rfm12_hw.h (RFM12_RXCTRL_RSSI_*) or an unsigned integer value from 61 to 103.
 */
void rfm12_set_rssi (uint8_t in_rssi)
{
	/* handle rssi bitmasks */
	if (in_rssi <= RFM12_RXCTRL_RSSI_MASK)
	{
		ctrl.rxctrl_shadow &= ~(RFM12_RXCTRL_RSSI_MASK);
		ctrl.rxctrl_shadow |= in_rssi;
		rfm12_data_safe (ctrl.rxctrl_shadow);
		return;
	} else if (in_rssi >= 61 && in_rssi <= 103)
	{
		uint8_t tmp = in_rssi - 61;
		ctrl.rxctrl_shadow &= ~(RFM12_RXCTRL_RSSI_MASK);
		ctrl.rxctrl_shadow |= (RFM12_RXCTRL_RSSI_MASK - (tmp / 6));
		rfm12_data_safe (ctrl.rxctrl_shadow);
		
	} else
	{
		/* invalid value given - don't change anything */
		return;
	}
}

//! Set the receive filter bandwidth
/** use RFM12_RXCTRL_BW constants from rfm12_hw.h
*/
void rfm12_set_bandwidth (uint8_t in_bw)
{
	ctrl.rxctrl_shadow &= ~(RFM12_RXCTRL_BW_MASK);
	ctrl.rxctrl_shadow |= in_bw & RFM12_RXCTRL_BW_MASK;
	rfm12_data_safe (ctrl.rxctrl_shadow);
}

/* set the relative output power in RFM12_TXCONF_POWER terms
 */
void rfm12_set_tx_power (uint8_t in_power)
{
	ctrl.txconf_shadow &= ~(RFM12_TXCONF_POWER_MASK);
	ctrl.txconf_shadow |= in_power;
	rfm12_data_safe ( ctrl.txconf_shadow);
}

/* set the fsk shift. Use datasheet or RFM12_TXCONF_FS_CALC(f) for values.
 */
void rfm12_set_fsk_shift (uint8_t in_fsk)
{
	ctrl.txconf_shadow &= ~(RFM12_TXCONF_POWER_MASK);
	ctrl.txconf_shadow |= in_fsk & 0xf0;
	rfm12_data_safe ( ctrl.txconf_shadow);
}


/* set the frequency band to use
 * accepted vlaues: RFM12_BAND_433 RFM12_BAND_315 RFM12_BAND_868 RFM12_BAND_915
 */
void rfm12_set_band (uint16_t in_band)
{
	rfm12_data_safe(RFM12_CMD_CFG | RFM12_CFG_EL | RFM12_CFG_EF | in_band | RFM12_XTAL_LOAD);
}

#if 0
/* convenience function to send control commands to the rf12. This function
 * attempts to mask out bits that may've been set accidently.
 */
uint16_t rfm12_sendcommand (uint16_t in_cmd, uint16_t in_payload)
{
	uint16_t msk = ~(in_cmd), masked_payload;
	uint16_t retval = 0;
	
	/* mask out accidental setting of commands */
	masked_payload = in_payload & msk;

	switch (in_cmd)
	{
		/* write-only commands */
		case RFM12_CMD_CFG:
		case RFM12_CMD_PWRMGT:
		case RFM12_CMD_RXCTRL:
		case RFM12_CMD_FREQUENCY:
		case RFM12_CMD_DATARATE:
		case RFM12_CMD_DATAFILTER:
		case RFM12_CMD_FIFORESET:
		case RFM12_CMD_SYNCPATTERN:
		case RFM12_CMD_AFC:
		case RFM12_CMD_TXCONF:
		case RFM12_CMD_PLL:
		case RFM12_CMD_TX:
		case RFM12_CMD_WAKEUP:
		case RFM12_CMD_DUTYCYCLE:
		case RFM12_CMD_LBDMCD:
		case RFM12_RESET: /* power on reset */		
			break;

		/* read/write commands */
		case RFM12_CMD_STATUS:
		case RFM12_CMD_READ:
			retval = rfm12_read (in_cmd | masked_payload);
			return retval;

		default:
			return 0;
	}
	
	/* handle write-only commands */
	rfm12_data ( in_cmd | masked_payload);
	return 1;
}
#endif

#endif