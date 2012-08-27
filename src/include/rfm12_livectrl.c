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

/** \file rfm12_livectrl.c
 * \brief rfm12 library live control feature source
 * \author Soeren Heisrath
 * \author Hans-Gert Dahmen
 * \author Peter Fuhrmann
 * \version 1.2.0
 * \date 28.01.12
 *
 *
 * \note This file is included directly by rfm12.c, for performance reasons.
 */

/******************************************************
 *	THIS FILE IS BEING INCLUDED DIRECTLY		*
 *		(for performance reasons)				*
 ******************************************************/

#if RFM12_LIVECTRL

#if RFM12_LIVECTRL_CLIENT

	#if __AVR__
		#include <avr/pgmspace.h>
		//yes, we include the c file because of ease of configuration this way
		//the C preprocessor can decide wether we need the file or not.
		#include "../xprintf/xprintf.c"
	#else
		#define PSTR(s)    s
		#define strcpy_P   strcpy
		#define xsprintf_P sprintf
	#endif

	void baseband_to_string(char *s, uint16_t var) {
		switch (var) {
			case RFM12_BAND_315:
				strcpy_P(s, PSTR("315MHz"));
				break;
			case RFM12_BAND_433:
				strcpy_P(s, PSTR("433MHz"));
				break;
			case RFM12_BAND_868:
				strcpy_P(s, PSTR("868MHz"));
				break;
			case RFM12_BAND_915:
				strcpy_P(s, PSTR("915MHz"));
				break;
			default:
				*s = 0;
				break;
		}
	}


	void frequency_to_string(char *s, uint16_t val) {
		//Band [MHz] C1 C2
		//315         1 31
		//433         1 43
		//868         2 43
		//915         3 30
		//f0 = 10 * C1 * (C2 + F/4000) [MHz]
		//f0 = 10 * C1 * (C2*1000 + F/4)    [kHz]

		//433:
		//f0 = 430 + F/400  [MHz]
		//f0 = 430000 + F * 2.5 [kHz]

		//915:
		//f0 = 900 + F * (3 /400)  [MHz]
		//f0 = 900000 + F * 7.5 [kHz]

		//868:
		//f0 = 860 + F * (2 /400)  [MHz]
		//f0 = 860000 + F * 5 [kHz]


		uint16_t mhz;
		uint16_t khz;
		uint16_t band_setting = livectrl_cmds[RFM12_LIVECTRL_BASEBAND].current_value;

		if (band_setting == RFM12_BAND_433) {
			mhz = 430 + val / 400;
			khz = ((val % 400) * 5) / 2;
		} else if (band_setting == RFM12_BAND_915) {
			val *= 3;
			mhz = 900 + val / 400;
			khz = ((val % 400) * 5) / 2;
		} else if (band_setting == RFM12_BAND_868) {
			val *= 2;
			mhz = 860 + val / 400;
			khz = ((val % 400) * 5) / 2;
		} else {
			mhz = 0;
			khz = 0;
		}
		#if (DISP_LEN == 8)
			xsprintf_P(s, PSTR("%3d.%03d"), mhz, khz); 
		#else
			xsprintf_P(s, PSTR("%3d.%03d MHz"), mhz, khz);
		#endif
	}

	void datarate_to_string(char *s, uint16_t val) {
		/*
			4. Data Rate Command
			Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
			1 1 0 0 0 1 1 0 cs r6 r5 r4 r3 r2 r1 r0 C623h
			The actual bit rate in transmit mode and the expected bit rate of the received data stream in receive mode is determined by the 7-bit
			parameter R (bits r6 to r0) and bit cs.
			BR = 10000 / 29 / (R+1) / (1+cs*7) [kbps]
		*/

		uint16_t n;

		if (val & 0x80) {
			//low bitrate
			n = 29 * 8;
		} else {
			//high bitrate
			n = 29;
		}
		val &= 0x7f;

		n *= val;

		uint32_t bitrate = 10000000ul / n;

		xsprintf_P(s, PSTR("%4ld bps"), bitrate);
	}


	#define FULLSCALE_TX_POWER 8;

	void tx_power_to_string(char *s, uint16_t var) {
		int16_t val = var;
		val *= -3;
		val += FULLSCALE_TX_POWER;

		xsprintf_P(s, PSTR("%3d dBm"), val);
	}

	void fsk_shift_to_string(char *s, uint16_t val) {
		val >>= 4; //right adjust

		val = (val + 1) * 15;

		xsprintf_P(s, PSTR("+-%d kHz"), val);
	}

	void lna_to_string(char *s, uint16_t var) {
		uint8_t val = var;
		val >>= 3; //right adjust

		switch (val) {
			case 1: val = 6; break;
			case 2: val = 14; break;
			case 3: val = 20; break;
		}

		xsprintf_P(s, PSTR("%3d dB"), -val);
	}

	void rssi_to_string(char *s, uint16_t val) {
		xsprintf_P(s, PSTR("%4d dBm"), -61 - (7 - val) * 6);
	}

	void filter_bw_to_string(char *s, uint16_t val) {
		val >>= 5; //right adjust

		val = ((7 - val) * 200) / 3;

		xsprintf_P(s, PSTR("%3d kHz"), val);
	}

	void xtal_load_to_string(char *s, uint16_t val) {
		val += 1;
		uint8_t pf = val / 2 + 8;
		uint8_t n = 0;
		if(val & 0x01) n = 5;
		xsprintf_P(s, PSTR("%2d.%dpF"), pf, n);
	}


	#define IFCLIENT(a,b,c,d,e) a,b,c,d,e
#else // RFM12_LIVECTRL_CLIENT
	#define IFCLIENT(a,b,c,d,e)
#endif // RFM12_LIVECTRL_CLIENT

#if RFM12_LIVECTRL_HOST
	#define IFHOST(a) a
#else
	#define IFHOST(a) 0
#endif

livectrl_cmd_t livectrl_cmds[] = {
	{ RFM12_CMD_CFG,       RFM12_CFG_BAND_MASK,     IFHOST(&ctrl.cfg_shadow),    RFM12_BASEBAND,                        IFCLIENT(0x00,   0x30, 0x10, "Baseband"  , baseband_to_string  )},
	{ RFM12_CMD_CFG,       RFM12_CFG_XTAL_MASK,     IFHOST(&ctrl.cfg_shadow),    RFM12_XTAL_LOAD,                       IFCLIENT(0x00,   0x0f,    1, "Xtal Load" , xtal_load_to_string  )},
	{ RFM12_CMD_FREQUENCY, RFM12_FREQUENCY_MASK,    0,                           RFM12_FREQUENCY_CALC(RFM12_FREQUENCY), IFCLIENT(0x00, 0x0fff,    4, "Frequency" , frequency_to_string )},
	{ RFM12_CMD_DATARATE,  RFM12_DATARATE_MASK,     0,                           DATARATE_VALUE,                        IFCLIENT(0x03,   0xff,    1, "Data rate" , datarate_to_string  )},
	{ RFM12_CMD_TXCONF,    RFM12_TXCONF_POWER_MASK, IFHOST(&ctrl.txconf_shadow), RFM12_POWER,                           IFCLIENT(0x00,   0x07,    1, "TX Power"  , tx_power_to_string  )},
	{ RFM12_CMD_TXCONF,    RFM12_TXCONF_FSK_MASK,   IFHOST(&ctrl.txconf_shadow), RFM12_TXCONF_FS_CALC(FSK_SHIFT),       IFCLIENT(0x00,   0xf0, 0x10, "FSK Shift" , fsk_shift_to_string )},
	{ RFM12_CMD_RXCTRL,    RFM12_RXCTRL_LNA_MASK,   IFHOST(&ctrl.rxctrl_shadow), RFM12_LNA_GAIN,                        IFCLIENT(0x00,   0x18, 0x08, "LNA"       , lna_to_string      )},
	{ RFM12_CMD_RXCTRL,    RFM12_RXCTRL_RSSI_MASK,  IFHOST(&ctrl.rxctrl_shadow), RFM12_RSSI_THRESHOLD,                  IFCLIENT(0x00,   0x07,    1, "RSSI"      , rssi_to_string      )},
	{ RFM12_CMD_RXCTRL,    RFM12_RXCTRL_BW_MASK,    IFHOST(&ctrl.rxctrl_shadow), RFM12_FILTER_BW,                       IFCLIENT(0x20,   0xC0, 0x20, "Filter BW" , filter_bw_to_string )},
};


#if RFM12_LIVECTRL_LOAD_SAVE_SETTINGS
	#include <avr/eeprom.h>

	void rfm12_save_settings() {
		uint8_t x;
		uint16_t checksumm = 0;

		for (x = 0; x < NUM_LIVECTRL_CMDS; x++) {
			uint16_t val = livectrl_cmds[x].current_value;
			checksumm += val;
			eeprom_write_word((void*)(2 * x), val);
		}

		eeprom_write_word((void*)(2 * x), checksumm);
	}

	void rfm12_load_settings() {

		uint8_t x;
		uint16_t val;
		uint16_t checksumm = 0;

		for (x = 0; x < NUM_LIVECTRL_CMDS; x++) {
			val = eeprom_read_word((void*)(2 * x));
			checksumm += val;
		}

		val = eeprom_read_word((void*)(2 * x));
		if (val != checksumm) return; //eeprom invalid, keep default values from array

		//set the settings if eeprom valid
		for (x = 0; x < NUM_LIVECTRL_CMDS; x++) {
			val = eeprom_read_word((void*)(2 * x));
			rfm12_livectrl(x, val);
		}
	}
#endif

#if RFM12_LIVECTRL_HOST
	void rfm12_data_safe(uint16_t d) {
		//disable the interrupt (as we're working directly with the transceiver now)
		RFM12_INT_OFF();
		rfm12_data(d);
		RFM12_INT_ON();
	}


	void rfm12_livectrl(uint8_t cmd, uint16_t value) {
		uint16_t tmp = 0;
		livectrl_cmd_t  *livectrl_cmd = &livectrl_cmds[cmd];

		livectrl_cmd->current_value = value; //update current value

		//the shadow register is somewhat redundant with the current value,
		//but it makes sense never the less:
		//the current_value only saves the bits for this one setting (for menu,saving,loding settings)
		//while the shadow register keeps track of ALL bits the rfm12 has in that register.
		//the shadow will also be used from rfm12_tick or maybe the interrupt

		if (livectrl_cmd->shadow_register) {
			tmp = *livectrl_cmd->shadow_register;         //load shadow value if any
			tmp &= ~livectrl_cmd->rfm12_hw_parameter_mask;//clear parameter bits
		}
		tmp |= livectrl_cmd->rfm12_hw_command | (livectrl_cmd->rfm12_hw_parameter_mask & value);

		*livectrl_cmd->shadow_register = tmp;

		rfm12_data_safe(tmp);
	}
#endif // RFM12_LIVECTRL_HOST

#if RFM12_LIVECTRL_CLIENT
	void rfm12_livectrl_get_parameter_string(uint8_t cmd, char *str) {
		livectrl_cmd_t *livectrl_cmd = &livectrl_cmds[cmd];

		uint16_t var = livectrl_cmd->current_value;
		livectrl_cmd->to_string(str, var);
	}
#endif // RFM12_LIVECTRL_CLIENT


#endif// RFM12_LIVECTRL
