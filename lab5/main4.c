/*
 * lab5.c
 *
 * Created: 30/04/2025 16:16:52
 * Author : Theo & Dean
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

int loop_control = 0; // Chooses the block to be executed in the loop
int rising_edges = 0; // PWM rising edges counter
int humidity = 2; // 0 -> humidity low, 1 -> humidity high, 2 -> humidity ok
int error = 0; // Error indicator

int main() {
	
	// Leds setup
	PORTD.DIR |= PIN0_bm | PIN1_bm | PIN2_bm; // Pins 0, 1 and 2 are outputs
	PORTD.OUT |= PIN0_bm | PIN1_bm | PIN2_bm; // All initially turned off
	
	// Setup ADC for free running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; // Free-Running mode enabled
	ADC0.CTRLE = ADC_WINCM_OUTSIDE_gc; // Interrupt outside of the thresholds
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // Pin 7 used as input for ADC
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; // Enable debug mode
	
	// Window comparator mode setup
	ADC0.WINLT |= 8; // Low threshold
	ADC0.WINHT |= 64; // High threshold
	
	ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
	
	// Pullup enable and sense on both edges for pins 5 and 6 (setup buttons)
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
			
	// Timer/counter setup
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // 1024 prescaler value
	
	sei(); // Start accepting interrupts
	
	while(1) { // Main loop
		
		PORTD.OUT |= 0b00000000; // BREAKPOINT command
		switch (humidity) { // Reset pins
			case 0:
				PORTD.OUT &= ~PIN0_bm; // Turn on led 0 for low humidity
				break;
			case 1:
				PORTD.OUT &= ~PIN1_bm; // Turn on led 1 for high humidity
				break;
			default:
				break;
		}
		
		if (loop_control == 0) { // Reset, enable ADC again
			
			loop_control = 3; // Don't execute any block in the next iteration
			
			if (error == 1) { // Reset the pins after error
				PORTD.OUT |= PIN0_bm | PIN1_bm | PIN2_bm; // Pins 0, 1 and 2 are turned off
				error = 0; // Reset error variable
			}
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
			ADC0.CTRLA |= ADC_ENABLE_bm; // Enable ADC
			ADC0.COMMAND |= ADC_STCONV_bm; // Start Conversion
		}
		
		if (loop_control == 1) { // Handle low humidity
			
			loop_control = 3; // Don't execute any block in the next iteration

			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Compare channel 0 interrupt enable
			TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
			TCA0.SINGLE.CNT = 0; // Clear counter

			int wait_time = ADC0.WINLT - ADC0.RES; // Time window to water the plants
			TCA0.SINGLE.CMP0 = wait_time; // When CMP0 reaches this value -> interrupt 	BREAKPOINT command
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // Enable timer/counter
		}
		
		if (loop_control == 2) { // Handle high humidity
			
			PORTD.OUT &= ~PIN1_bm & ~PIN2_bm; // Pins 1 and 2 initially turned on
			
			loop_control = 3; // Don't execute any block in the next iteration
			
			TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // Single slope PWM
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm; // Timer overflow/underflow interrupt enable
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm; // Compare channel 0 interrupt enable
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // Enable timer/counter
			
			TCA0.SINGLE.PER = 20; // (Approximately 1ms period)
			TCA0.SINGLE.CMP0 = 0.5 * TCA0.SINGLE.PER; // 50% Duty cycle		BREAKPOINT command
			
		}
	}
	
	cli(); // Stop accepting interrupts
}

ISR(ADC0_WCOMP_vect) { // Humidity interrupt from the ADC
	
		if (ADC0.RES < ADC0.WINLT) { // Humidity below threshold
	
			PORTD.OUT &= 0b11111110; // Turn on Led 0
		
			humidity = 0; // Low humidity
		
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
		}

		if (ADC0.RES > ADC0.WINLT) { // Humidity above threshold
			
			PORTD.OUT &= 0b11111101; // Turn on led 1
		
			humidity = 1; // High humidity
		
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
				
		}
	
	int intflags = ADC0.INTFLAGS; // Clear interrupt flags
	ADC0.INTFLAGS = intflags;
	ADC0.CTRLA &= ~ADC_ENABLE_bm; // Disable the ADC		
	
}

ISR(PORTF_PORT_vect) { // Button interrupt from port f
	
	uint8_t humidity_low = PORTF.INTFLAGS & PIN5_bm; // Equals PIN5_bm if pin 5 is pressed, else it's all 0s
	uint8_t humidity_high = PORTF.INTFLAGS & PIN6_bm; // Equals PIN6_bm if pin 6 is pressed, else it's all 0s
	
	if (humidity == 0 && humidity_low) { // Pin 5 was used and humidity is low
		
		loop_control = 1; // Handle low humidity in the next iteration
		PORTD.OUT |= 0b00000000; // BREAKPOINT command
	}
	
	if (humidity == 1 && humidity_high) { // Pin 6 was used and humidity is high
			
		loop_control = 2; // Handle high humidity in the next iteration
		PORTD.OUT |= 0b00000000; // BREAKPOINT command
	}
	
	if ((humidity == 0 && humidity_high) || (humidity == 1 && humidity_low) || (humidity == 2)) { // Wrong button pressed
		
		loop_control = 0; // Reset the ADC in the next iteration
		PORTD.OUT &= ~PIN0_bm & ~PIN1_bm & ~PIN2_bm; // Pins 0, 1 and 2 are turned on (error indicator)		BREAKPOINT command
		error = 1; // Set error variable		BREAKPOINT command
	}
	
	int intflags = PORTF.INTFLAGS; // Clear the interrupt flags
	PORTF.INTFLAGS = intflags;
}


ISR(TCA0_CMP0_vect) { // Time to stop watering the plants
	if (humidity == 0) {
		TCA0.SINGLE.CTRLA = ~TCA_SINGLE_ENABLE_bm; // Disable the timer/counter
		loop_control = 0; // Reset the ADC in the next iteration
		PORTD.OUT |= PIN0_bm; // Pin 0 is turned off
		humidity = 2; // Humidity is ok
	
		int intflags = TCA0.SINGLE.INTFLAGS; // Clear the interrupt flags
		TCA0.SINGLE.INTFLAGS = intflags;
	
	}
}

ISR(TCA0_OVF_vect) { // Interrupt in rising edges
	
	PORTD.OUT ^= PIN2_bm; // Complement the value of pin 2
	rising_edges = rising_edges + 1; // increment the rising edge counter    BREAKPOINT command
	
	if (rising_edges == 4) { // Max number of rising edges reached
		TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Disable timer/counter
		PORTD.OUT |= PIN1_bm | PIN2_bm; // Pins 1 and 2 are turned off		BREAKPOINT command
		loop_control = 0; // Reset the ADC in the next iteration
		rising_edges = 0; // Reset the rising edges counter
		humidity = 2; // Humidity is ok
	}
	
	int intflags = TCA0.SINGLE.INTFLAGS; // Clear the interrupt flags
	TCA0.SINGLE.INTFLAGS = intflags;
}
