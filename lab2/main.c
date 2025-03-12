#include <avr/io.h>
#include <avr/interrupt.h>


#define value0 1000
#define value1 20000
#define value2 20 // After the press of the button, the counter counts to value2 and changes the state of the traffic light
#define value3 50 // The pedestrian cannot press the button until the counter counts up to this value

int allow_button = 1; // Pedestrians are allowed to press the button
int int_type; // Indicates which type of interrupt is allowed
int x=0; // Loop control logic flag

int main() {
	
	// Initialize the traffic light
	PORTD.DIR |= 0b00000111; //Pins 0 and 2 of port D are outputs
	PORTD.OUT |= 0b00000011; // Red light for pedestrians (insert breakpoint)
	PORTD.OUT &= 0b11111011; // Green light for cars (insert breakpoint)
	
	// Initialize the counter
	TCA0.SPLIT.CTRLD = 1; // Enable split mode
	TCA0.SPLIT.LCNT = 0; // Clear counter low
	TCA0.SPLIT.HCNT = 0;
	TCA0.SPLIT.HCMP0 = value1;
	TCA0.SPLIT.LCMP0 = value0; // Traffic light changes state after counter counts to value2
	TCA0.SPLIT.CTRLA = 0x7<<1 | 1; // Prescaler value = 1024 and the peripheral is disabled
	TCA0.SPLIT.INTCTRL |= 0b00010010; // Interrupt Enable for channel 0
	
	// Initialize the button
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Pullup enable and Interrupt enabled with sense on both edges
	
	sei(); 	// Start accepting interrupts
	
	while (1) { // Main loop
		
		if (x == 1) {
			
			x = 0; // Break out of the conditional block after this iteration (insert breakpoint)
			TCA0.SPLIT.LCNT = 0; // Clear counter
			TCA0.SPLIT.LCMP0 = value3; // Pedestrian can use the button after the counter counts up to value3
			int_type = 2; // Button time interrupt
			TCA0.SPLIT.CTRLA |= 1; // Counter starts to count
		}
		
		if (x == 2){
			
			x = 0;			
			TCA0.SPLIT.LCMP0 = value2;
			TCA0.SPLIT.CTRLA |= 1; // Counter starts to count
			int_type = 3;
		}
		
		
		PORTD.OUT |= 0b00000000; // NOP (insert breakpoint)
	}
	cli(); // Stop accepting interrupts
}

ISR(PORTF_PORT_vect) { // Is triggered after the press of the button
	
	if (allow_button == 1) { // If pedestrian is allowed to press the button
		allow_button = 0; // Pedestrian cannot press the button anymore
		PORTD.OUT |= 0b00000100; // Red light for cars (insert breakpoint)
		PORTD.OUT &= 0b11111110; // Green light for pedestrians (insert breakpoint)
		int_type = 1; // traffic light interrupt
		TCA0.SPLIT.LCMP0 = value2; 
		TCA0.SPLIT.CTRLA |= 1; // Counter starts to count
	}
	PORTF.INTFLAGS = PIN5_bm; // Clear the interrupt flag
}

ISR(TCA0_LCMP0_vect) {
	
	if (int_type == 1) { // traffic light changes state
		
		PORTD.OUT |= 0b00000001; // Red light for pedestrians (insert breakpoint)
		PORTD.OUT &= 0b11111011; // Green light for cars (insert breakpoint)
		x = 1; // Change flag to get out of the loop
	}
	
	if (int_type == 2) {
		
		allow_button = 1; // Allow the press of the button (insert breakpoint)
		int_type = 0; // None of the counter interrupts should happen
		TCA0.SPLIT.LCNT = 0; // Clear the counter
	}
	
	if (int_type == 3) {
			
		int_type = 0; // None of the counter interrupts should happen
		TCA0.SPLIT.LCNT = 0; // Clear the counter
		PORTD.OUT &= 0b11111011;
		PORTD.OUT |= 0b00000011;
		x = 1;
	}
		
	TCA0.SPLIT.INTFLAGS = TCA_SPLIT_LCMP0_bm; // Clear the interrupt flag
}

ISR(TCA0_HUNF_vect) {
	
	PORTD.OUT |= 0b00000100; // Red light for CARS (insert breakpoint)
	PORTD.OUT &= 0b11111100; // Green light for pedestrians and tram (insert breakpoint)
	
	allow_button = 0; // Allow the press of the button (insert breakpoint)
	
	x = 2;
	
	TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm; // Clear the interrupt flag

	
}
