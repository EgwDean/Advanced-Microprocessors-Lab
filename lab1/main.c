/*
 * lab1.c
 *
 * Created: 21/02/2025 15:44:32
 * Author : Theo
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int main() {
	// Initialize port D (LEDs)
	PORTD.DIR |= 0b00000111; //Pins 0, 1 and 2 are outputs
	PORTD.OUT |= 0b00000111; //LEDs 0, 1 and 2 are turned off
	// initialize port F (Switches)
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Pullup enable and detection on both edges (pin 5)
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Pullup enable and detection on both edges (pin 6)
	
	sei(); // Enable interrupts
	
	// Main loop
	while (1) {
		PORTD.OUT |= 0b00000000; // Idle command to allow breakpoints in the loop (insert breakpoint)
	}

	cli(); // Disable interrupts
}

// Interrupt Service Routine for port F
ISR(PORTF_PORT_vect) {
	uint8_t button_up = PORTF.INTFLAGS & PIN5_bm; // Equals PIN5_bm if pin 5 is pressed, else it's all 0s
	uint8_t button_down = PORTF.INTFLAGS & PIN6_bm; // Equals PIN6_bm if pin 6 is pressed, else it's all 0s
	// If both switches are used, both the variables are all 0s
	
	int floor; // Integer indicating the current floor
	
	if (button_up && button_down) { // Error, both switches used
		PORTD.OUT &= 0b11111110; // First LED of port D gets turned on (insert breakpoint)
		_delay_ms(10); // Wait for 10ms
		PORTD.OUT |= 0b00000001; // First LED of port D gets turned off (insert breakpoint)
	}

	else { // One of the switches used
		if ((PORTD.OUT & 0b00000111) == 0b00000111) // Current level is ground
			floor = 0;
		if ((PORTD.OUT & 0b00000111) == 0b00000101) // Current level is 1
			floor = 1;
		if ((PORTD.OUT & 0b00000111) == 0b00000001) // Current level is 2
			floor = 2;	
		if (button_up) { // Move up switch used
			if (floor == 0) // Go to level 1
				PORTD.OUT = 0b00000101;	
			if (floor == 1) // Go to level 2
				PORTD.OUT = 0b00000001;	
			if (floor == 2) // Stay at level 2
				PORTD.OUT = 0b00000001;	
		}
	
		if (button_down) { // Move down switch used
			if (floor == 0) // Stay at ground level
				PORTD.OUT = 0b00000111;	
			if (floor == 1) // Go to ground level
				PORTD.OUT = 0b00000111;	
			if (floor == 2) // Go to level 1
				PORTD.OUT = 0b00000101;	
		}
	}
	int y = PORTF.INTFLAGS; // Reset the switches
	PORTF.INTFLAGS=y;
}
