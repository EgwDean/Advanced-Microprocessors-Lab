/*
 * lab2.c
 *
 * Created: 15/03/2025 15:44:32
 * Author : Theo & Dean
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#define value1 200 // Tram counter counts down from this value
#define value2 30 // After the press of the button, the counter counts down to value2 and changes the state of the traffic light
#define value3 55 // The pedestrian cannot press the button until the counter counts down to this value

int allow_button = 1; // Pedestrians are allowed to press the button
int int_type; // Indicates which type of interrupt is allowed
int x = 0; // Loop control logic flag

int main() {
	
	// Initialize the traffic light
	PORTD.DIR |= 0b00000111; // Pins 0, 1 and 2 of port D are outputs
	PORTD.OUT |= 0b00000011; // Red light for pedestrians and tram does not pass
	PORTD.OUT &= 0b11111011; // Green light for cars
	
	// Initialize the counter
	TCA0.SPLIT.CTRLD = 1; // Enable split mode 
	TCA0.SPLIT.HCNT = value1; // Start from value1 until you reach zero
	TCA0.SPLIT.CTRLA = 0x7<<1 | 1; // Prescaler value = 1024 and the peripheral is enabled
	TCA0.SPLIT.INTCTRL |= 0b00010010; // Compare interrupt enable (low) and underflow interrupt enable (high)
	
	// Initialize the button
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Pullup enable and interrupt enable with sense on both edges
	
	sei(); 	// Start accepting interrupts
	
	while (1) { // Main loop
		
		if (x == 1) { // This block runs after the press of the button
			
			x = 0; // Break out of the conditional block after this iteration
			TCA0.SPLIT.LCNT = value3; // Initialize counter to value3
			TCA0.SPLIT.LCMP0 = 0; // Pedestrian can use the button after the counter counts down from value3 (insert breakpoint)
			int_type = 2; // Button time interrupt

		}
		
		if (x == 2) { // This block runs after tram passes
			
			x = 0; // Break out of the conditional block after this iteration
			TCA0.SPLIT.HCNT = value1; // Reset tram counter
			TCA0.SPLIT.LCNT = value2; // Clear traffic light counter
			TCA0.SPLIT.LCMP0 = 0; // (insert breakpoint)
			int_type = 3; // Reset phase after tram passes
		}
		
		
		PORTD.OUT |= 0b00000000; // NOP (insert breakpoint)
	}
	cli(); // Stop accepting interrupts
}

ISR(PORTF_PORT_vect) { // Is triggered after the press of the button
	
	if (allow_button == 1) { // If pedestrian is allowed to press the button
		
		allow_button = 0; // Pedestrian cannot press the button anymore
		PORTD.OUT |= 0b00000100; // Red light for cars
		PORTD.OUT &= 0b11111110; // Green light for pedestrians
		int_type = 1; // traffic light interrupt

		TCA0.SPLIT.LCNT = value2; // Clear counter
		TCA0.SPLIT.LCMP0 = 0; // (insert breakpoint)
	}
	
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS = y; // Clear the interrupt flag
}

ISR(TCA0_LCMP0_vect) { // Runs after the low counter counts to zero
	
	if (int_type == 1) { // traffic light changes state
		
		PORTD.OUT |= 0b00000001; // Red light for pedestrians
		PORTD.OUT &= 0b11111011; // Green light for cars
		x = 1; // Change flag to get out of the loop (insert breakpoint)
	}
	
	if (int_type == 2) { // After time passes after the press of the button
		
		allow_button = 1; // Allow the press of the button (insert breakpoint)
		int_type = 0; // None of the low counter interrupts should happen

	}
	
	if (int_type == 3) { // Reset phase after tram passes
		
		int_type = 0; // None of the counter interrupts should happen
		PORTD.OUT &= 0b11111011; // Green light for cars
		PORTD.OUT |= 0b00000011; // Red light for pedestrians and tram does not pass
		x = 1; // (insert breakpoint)
	}
	
	int y = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS = y; // Clear the interrupt flag
}

ISR(TCA0_HUNF_vect) {  // Tram interrupt
	
	PORTD.OUT |= 0b00000100; // Red light for cars
	PORTD.OUT &= 0b11111100; // Green light for pedestrians and tram passes
	
	allow_button = 0; // Do not allow the press of the button
	
	x = 2; // (insert breakpoint)
	
		int y = TCA0.SPLIT.INTFLAGS;
		TCA0.SPLIT.INTFLAGS = y; // Clear the interrupt flag
}
