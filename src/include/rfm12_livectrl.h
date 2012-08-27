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

/** \file rfm12_ctrl_structure.h
 * \brief rfm12 library live control data structure
 * \author Soeren Heisrath
 * \author Hans-Gert Dahmen
 * \author Peter Fuhrmann
 * \version 1.2.0
 * \date 27.01.12
 *
 * defines data structure form live ctrl functions. The data structure can
 * be used by the microcontroller and the host program to implement livecontrol
 * over a communication link, where the microcontroller sets the settings,
 * while the host displays and sends the values  
 *
 */

typedef struct {
	uint16_t rfm12_hw_command;         //actual SPI command for rfm12
	uint16_t rfm12_hw_parameter_mask;  //mask that selects valid bits for this parameter
	uint16_t *shadow_register;         //pointer to the shadow register to be used for this command
	uint16_t current_value;

#if RFM12_LIVECTRL_CLIENT
	//these are for the client
	uint16_t min_val;
	uint16_t max_val;
	int16_t  step;
	char    *name;
	void (*to_string)(char *str, uint16_t value);
#endif
} livectrl_cmd_t;

extern livectrl_cmd_t livectrl_cmds[];

//these constants have to be in the same order as the elemts in the table
//livectrl_cmds
#define RFM12_LIVECTRL_BASEBAND   0
#define RFM12_LIVECTRL_XTAL_LOAD  1
#define RFM12_LIVECTRL_FREQUENCY  2
#define RFM12_LIVECTRL_DATARATE   3
#define RFM12_LIVECTRL_TX_POWER   4
#define RFM12_LIVECTRL_FSK_SHIFT  5
#define RFM12_LIVECTRL_LNA        6
#define RFM12_LIVECTRL_RSSI       7
#define RFM12_LIVECTRL_FILTER_BW  8

#define NUM_LIVECTRL_CMDS         9

void rfm12_livectrl(uint8_t cmd, uint16_t value);

void rfm12_save_settings();
void rfm12_load_settings();

#if RFM12_LIVECTRL_CLIENT
	void rfm12_livectrl_get_parameter_string(uint8_t cmd, char *str);
#endif
