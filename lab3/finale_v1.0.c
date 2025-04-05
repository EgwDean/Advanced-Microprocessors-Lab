/*
 * lab3.c
 *
 * Created: 20/03/2025 16:23:54
 * Author : Theo & Dean
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

//counter thresholds
#define value1 20
#define value2 40
#define value3 60

//distance sensor threshold
#define DISTANCE_1 8
#define DISTANCE_2 9

int x; //loop control flag
int z; //ADC ISR control flag
int inverted = 0; //clockwise or counter clockwise indicator
int left = 0; //left turn counter
int right = 0; //right turn counter


int main() {
	
	//led setup
	PORTD.DIR |= PIN0_bm | PIN1_bm | PIN2_bm; //first three pins outputs
	PORTD.OUT |= PIN0_bm | PIN2_bm; //PIN0 and PIN2 leds initially turned off
	PORTD.OUT &= ~PIN1_bm; //PIN1 led initially turned on
	
	//ADC setup
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc | ADC_ENABLE_bm; //10-bit resolution and enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //bit 7 analog input
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //enable Debug Mode
	ADC0.CTRLA |= ADC_FREERUN_bm; //free-running mode enabled
	ADC0.WINLT |= DISTANCE_1; //set threshold for forward sensor
	ADC0.WINHT |= DISTANCE_2; //set threshold for side sensor
	ADC0.INTCTRL |= ADC_WCMP_bm; //enable interrupts for WCM
	ADC0.CTRLE = ADC_WINCM_ABOVE_gc; //interrupt when RESULT > WINLT	
	
	//counter setup
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; //normal mode
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm; //prescaler value 1024 and counter enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; //enable compare channel 0 interrupt
	ADC0.COMMAND |= ADC_STCONV_bm; //start conversion	
	
	//button setup
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; //pullup enable and sense on both edges
	
	sei(); // start accepting interrupts

	x = 1; //side sensor works first (insert breakpoint)
	
	
	while(1) { // main loop
		
		if (x == 1) { //side sensor
				
				//ADC0.CTRLA ^= (1 << 1);//Free-Running mode disabled ?
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value1; //assign value1 to the counter compare
				z = 1; //side sensor part of the ISR should run
				x = 0; //don't use the ADC until interrupt
				ADC0.CTRLE = ADC_WINCM_ABOVE_gc; //interrupt when RESULT > WINLT (insert breakpoint)
		}
		
		if (x == 2) { //forward sensor
			
				ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value2; //assign value1 to the counter compare
				z = 2; //forward sensor part of the ISR should run
				x = 0; //don't use the ADC until interrupt
				ADC0.CTRLE = ADC_WINCM_BELOW_gc; //interrupt when RESULT < WINLT (insert breakpoint)
		}
		
		if ( (left == (right + 4)) || (right == (left + 4))) //termination condition if not inverted
			break; //terminate
		
		if ((inverted == 1) && ((right == -1) || (left == -1))) //termination condition if inverted
			break; //terminate
			
		PORTD.OUT |= 0b00000000; // NOP (insert breakpoint)
	}
}

ISR(ADC0_WCOMP_vect) { //ADC interrupt service routine
	
	if (z == 1 && inverted == 0) { //turn right
		
		PORTD.OUT |= PIN1_bm; //turn off led 1
		PORTD.OUT &= ~PIN0_bm; //turn on led 0
		right = right + 1; //CHANGE VALUE (insert breakpoint)
		PORTD.OUT |= PIN0_bm; //turn off led 0
		PORTD.OUT &= ~PIN1_bm; //turn on led 1
	}
	
	if (z == 2 && inverted == 0) { //turn left
		
		PORTD.OUT |= PIN1_bm; //turn off led 1
		PORTD.OUT &= ~PIN2_bm; //turn on led 2
		left = left + 1; //CHANGE VALUE (insert breakpoint)
		PORTD.OUT |= PIN2_bm; //turn off led 2
		PORTD.OUT &= ~PIN1_bm; //turn on led 1
	}
	
	if ((inverted == 1) && (z == 1)) { //inverted turn left
		
		PORTD.OUT |= PIN1_bm; //turn off led 1
		PORTD.OUT &= ~PIN2_bm; //turn on led 2
		right = right - 1;	// CHANGE VALUE (insert breakpoint)
		PORTD.OUT |= PIN2_bm; //turn off led 2
		PORTD.OUT &= ~PIN1_bm; //turn on led 1
	}
	
	if ((inverted == 1) && (z == 2)) { //inverted turn right
		
		PORTD.OUT |= PIN1_bm; //turn off led 1
		PORTD.OUT &= ~PIN0_bm; //turn on led 0
		left = left - 1; // CHANGE VALUE (insert breakpoint)
		PORTD.OUT |= ~PIN0_bm; //turn off led 0
		PORTD.OUT &= PIN1_bm; //turn on led 1
	}
	
	int intflags = ADC0.INTFLAGS; //reset the interrupt flags of the ADC
	ADC0.INTFLAGS = intflags;
}

ISR(TCA0_CMP0_vect) { //timer interrupt service routine
	
	if (z == 1) { //side sensor was used
		x = 2; // forward sensor is about to be used
		ADC0.CTRLE = 0b00000000; // disable ADC comparisons	(insert breakpoint)
	}
	
	if (z == 2) { //forward sensor was used
		x = 1; // side sensor is about to be used
		ADC0.CTRLE = 0b00000000; // disable ADC comparisons	(insert breakpoint)
	}
	
	if (inverted == 2) { // the device is about to be inverted
		
		inverted = 1; //the device is inverted
		PORTD.OUT |= PIN0_bm | PIN2_bm; //led 0 and 2 are turned off
		x = 1; //side sensor is about to be used
	}
	
	int intflags = TCA0.SINGLE.INTFLAGS; //reset the interrupt flags of the counter
	TCA0.SINGLE.INTFLAGS=intflags;
}


ISR(PORTF_PORT_vect) {
	int y = PORTF.INTFLAGS; //reset the interrupt flags of the buttons
	PORTF.INTFLAGS=y; 
	inverted = 2; //the device is about to be inverted (insert breakpoint)
	x = 0; //don't use the ADC until interrupt
	z = 0; //don't use the ADC until interrupt
	PORTD.OUT &= ~PIN0_bm & ~PIN1_bm & ~PIN2_bm; //the first three leds are turned on
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CMP0 = value3; //assign value3 to the counter compare
	ADC0.CTRLE = 0b00000000; // disable ADC comparisons	(insert breakpoint)
}
