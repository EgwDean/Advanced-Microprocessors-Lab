#include <avr/io.h>
#include <avr/interrupt.h>


// value0 255
#define value2 225 // After the press of the button, the counter counts to value2 and changes the state of the traffic light
#define value3 200 // The pedestrian cannot press the button until the counter counts up to this value

int allow_button = 1; // Pedestrians are allowed to press the button
int int_type; // Indicates which type of interrupt is allowed
int x=0; // Loop control logic flag
int z=0; //Variable to disable the first high counter interrupt. (FF starting value)

int main() {
	
	// Initialize the traffic light
	PORTD.DIR |= 0b00000111; //Pins 0 and 2 of port D are outputs
	PORTD.OUT |= 0b00000011; // Red light for pedestrians 
	PORTD.OUT &= 0b11111011; // Green light for cars 
	
	// Initialize the counter
	TCA0.SPLIT.CTRLD = 1; // Enable split mode
	TCA0.SPLIT.LCNT = 0b11111111; // Clear counter low
	TCA0.SPLIT.HCNT = 0b11111111; // Clear counter high
	TCA0.SPLIT.CTRLA = 0x7<<1 | 1; // Prescaler value = 1024 and the peripheral is disabled
	TCA0.SPLIT.INTCTRL |= 0b00010010; // Interrupt Enable for channel 0
	
	// Initialize the button
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Pullup enable and Interrupt enabled with sense on both edges
	
	sei(); 	// Start accepting interrupts
	
	while (1) { // Main loop
		
		if (x == 1) {
			
			x = 0; // Break out of the conditional block after this iteration 
			TCA0.SPLIT.LCNT = 0; // Clear counter
			TCA0.SPLIT.LCMP0 = value3; // Pedestrian can use the button after the counter counts p to value3 (insert breakpoint)
			int_type = 2; // Button time interrupt

		}
		
		if (x == 2){
			
			x = 0;
			TCA0.SPLIT.LCNT = 0; // Clear counter
			TCA0.SPLIT.LCMP0 = value2; // (insert breakpoint)
			int_type = 3;
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

		TCA0.SPLIT.LCNT = 0; // Clear counter		
		TCA0.SPLIT.LCMP0 = value2; // (insert breakpoint)
	}
	
	PORTF.INTFLAGS = PIN5_bm; // Clear the interrupt flag
}

ISR(TCA0_LCMP0_vect) {
	
	if (int_type == 1) { // traffic light changes state
		
		PORTD.OUT |= 0b00000001; // Red light for pedestrians
		PORTD.OUT &= 0b11111011; // Green light for cars 
		x = 1; // Change flag to get out of the loop (insert breakpoint)
	}
	
	if (int_type == 2) {
		
		allow_button = 1; // Allow the press of the button (insert breakpoint)
		int_type = 0; // None of the counter interrupts should happen 

	}
	
	if (int_type == 3) {
		
		int_type = 0; // None of the counter interrupts should happen
		PORTD.OUT &= 0b11111011;
		PORTD.OUT |= 0b00000011;
		x = 1; // (insert breakpoint)
	}
	
	TCA0.SPLIT.INTFLAGS = TCA_SPLIT_LCMP0_bm; // Clear the interrupt flag
}

ISR(TCA0_HUNF_vect) {
			
		PORTD.OUT |= 0b00000100; // Red light for CARS 
		PORTD.OUT &= 0b11111100; // Green light for pedestrians and tram 
	
		allow_button = 0; // Allow the press of the button 
	
		x = 2; //(insert breakpoint)
	
		TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm; // Clear the interrupt flag	
}
