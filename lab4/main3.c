/*
 * lab4.c
 *
 * Created: 09/04/2025 18:06:38
 * Author : Theo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define T1 19
#define T2 39

int x = 0; // Loop control variable

int main(){
	
	PORTD.DIR |= PIN0_bm | PIN1_bm; // PINS 1 and 2 are outputs
	PORTD.OUT |= PIN0_bm | PIN1_bm; // Both disabled
	TCA0.SPLIT.CTRLD = TCA_SINGLE_SPLITM_bm; // Enable split mode 
	TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // Prescaler = 1024
	TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP0EN_bm | TCA_SPLIT_HCMP0EN_bm;
	// Pullup enable and sense on both edges
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	// High and low underflow interrupt enable
    TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm | TCA_SPLIT_HUNF_bm;
		
	sei(); // Start accepting interrupts (breakpoint)
	
		
	while (1) { // Main loop

		if (x == 1) { // First button press
		
			x = x + 1; // Change logic flag to get out of this block
		
			TCA0.SPLIT.LPER = T1; // Wings period
			TCA0.SPLIT.HPER = T2; // Base period
				
			TCA0.SPLIT.LCMP0 = 0.6 * TCA0.SPLIT.LPER; // Wings duty cycle	
			TCA0.SPLIT.HCMP0 = 0.4 * TCA0.SPLIT.HPER; // Base duty cycle

			TCA0.SPLIT.LCNT = 1; // Minor offset so they don't interrupt together
			TCA0.SPLIT.HCNT = 0;
		
			TCA0.SPLIT.CTRLA |= TCA_SPLIT_ENABLE_bm; //Enable TCA (breakpoint)
		
		}
	
		if (x == 3) { // Second button press
		
			PORTD.OUT |= PIN0_bm | PIN1_bm; // Both PINS are disabled
			
			x = x + 1; // Change logic flag to get out of this block
		
			// The wings period becomes half the original
			TCA0.SPLIT.LPER = (T1 / 2);
			
			// The duty cycle becomes 50%
			TCA0.SPLIT.LCMP0 = 0.5 * TCA0.SPLIT.LPER;

			TCA0.SPLIT.LCNT = 0; // // Minor offset so they don't interrupt together
			TCA0.SPLIT.HCNT = 1; // (breakpoint)
		
		}
	
		if (x == 5) { // Third button press
		
			PORTD.OUT |= PIN0_bm | PIN1_bm; // Both PINS are disabled
			
			x = x + 1; // Change logic flag to get out of this block
		
			TCA0.SPLIT.CTRLA &= ~TCA_SPLIT_ENABLE_bm; // Disable TCA (breakpoint)
			
		}
		
		PORTD.OUT |= 0b00000000; // NOP (breakpoint)
	
	}
}

ISR(TCA0_LUNF_vect) { // Every 1 ms, wings
	
	cli();
	
	PORTD.OUT ^= PIN0_bm; // Complement the wings led
	
	int intflags = TCA0.SINGLE.INTFLAGS; // Clear the flags
	TCA0.SINGLE.INTFLAGS = intflags; // (breakpoint)

	sei();
}

ISR(TCA0_HUNF_vect) { // Every 2 ms, base

	cli();

	PORTD.OUT ^= PIN1_bm;  // Complement the base led

	int intflags = TCA0.SINGLE.INTFLAGS; // Clear the flags
	TCA0.SINGLE.INTFLAGS = intflags; // (breakpoint)
	sei();

}


ISR(PORTF_PORT_vect) { // Button press interrupt
	
	cli();
	
	int intflags = PORTF.INTFLAGS; // Clear the flags
	PORTF.INTFLAGS = intflags; 
	
	x = x + 1; // Change logic flag to get out of loop (breakpoint) 
	
	if (x == 7) { // Reset
		x = 1; // (breakpoint)
	}
	sei();
	
}
