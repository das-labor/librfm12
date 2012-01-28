
#include <avr/io.h>
#include "rfm12.h"

//This function is called from all spinloops in the user interface code, so all
//statemachines we have can run while the user interface itself is not
//implemented as statemachine.

extern void display_received_packets();
extern void transmit_packets();

void run_statemachines(){
	rfm12_tick();
	display_received_packets();
	transmit_packets();
}
