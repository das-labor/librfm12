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
static rfm12_control_t *myctrl = 0x00;
void rfm12_set_control_register (rfm12_control_t *in_p)
{
	myctrl = in_p;
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
	rfm12_data_delayed(0, RFM12_CMD_DATARATE | DATARATE_VALUE );
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
	rfm12_data_delayed(0, RFM12_CMD_FREQUENCY | in_freq );
}

/* set the rssi value. this can be done with either one of the macros defined
 * in rfm12_hw.h (RFM12_RXCTRL_RSSI_*) or an unsigned integer value from 61 to 103.
 */
void rfm12_set_rssi (uint8_t in_rssi)
{
	if (!myctrl) return;
	
	/* handle rssi bitmasks */
	if (in_rssi <= RFM12_RXCTRL_RSSI_MASK)
	{
		myctrl->rxctrl_shadow &= ~(RFM12_RXCTRL_RSSI_MASK);
		myctrl->rxctrl_shadow |= in_rssi;
		rfm12_data (myctrl->rxctrl_shadow);
		return;
	} else if (in_rssi >= 61 && in_rssi <= 103)
	{
		uint8_t tmp = in_rssi - 61;
		myctrl->rxctrl_shadow &= ~(RFM12_RXCTRL_RSSI_MASK);
		myctrl->rxctrl_shadow |= (RFM12_RXCTRL_RSSI_MASK - (tmp / 6));
		rfm12_data (myctrl->rxctrl_shadow);
	} else
	{
		/* invalid value given - don't change anything */
		return;
	}
}

/* set the relative output power in -dB (from 0 to -21dB)
 */
void rfm12_set_txpower (uint8_t in_power)
{
	if (!myctrl) return;

	myctrl->txconf_shadow &= ~(RFM12_TXCONF_POWER_MASK);
	myctrl->txconf_shadow |= (in_power / 3);
	rfm12_data_delayed (0, myctrl->txconf_shadow);
}


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
	rfm12_data_delayed (0, in_cmd | masked_payload);
	return 1;
}

void rfm12_data_delayed (uint8_t in_op, uint16_t in_cmd)
{
	static uint8_t num_cmds = 0;
	static uint16_t cmds[RFM12_NUM_DELAYED_COMMANDS];

	/* execute delayed commands */
	if (in_op == 1)
	{
		while (num_cmds)
		{
			rfm12_data (cmds[num_cmds-1]);
			num_cmds--;
		}
		return;
	}

	/* add command to queue */
	if (num_cmds >= RFM12_NUM_DELAYED_COMMANDS)
		return;	/* queue full */
	
	cmds[num_cmds] = in_cmd;
	num_cmds++;
}

#endif
