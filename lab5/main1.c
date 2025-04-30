/*
 * lab5.c
 *
 * Created: 30/04/2025 16:16:52
 * Author : null
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int x = 0;

int main(){
	
	
	PORTD.DIR |= PIN1_bm; //PIN is output
	//initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLE = 0b00000100; // outisde threshold
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	
	//Window Comparator Mode
	ADC0.WINLT |= 8; //Set threshold
	ADC0.WINHT |= 64;	
	
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	
	
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	
	sei();	
	
	while(1){
					
		PORTD.OUT |= 0b00000011; // BREAKPOINT command
		
	}
}


ISR(ADC0_WCOMP_vect){
	
	if (ADC0.RES < ADC0.WINLT) {
	
		PORTD.OUT &= 0b11111110;// open led0
		PORTD.OUT |= 0b00000011; // BREAKPOINT command

	} else  {
			
		PORTD.OUT &= 0b11111101; // open led1
		PORTD.OUT |= 0b00000011; // BREAKPOINT command
		
	}
	
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
}


ISR(PORTF_PORT_vect){
	
	ADC0.CTRLA &= 0b11111110; //disable ADC		
	
	
	uint8_t go_up = PORTF.INTFLAGS & PIN5_bm; // Equals PIN5_bm if pin 5 is pressed, else it's all 0s
	uint8_t go_down = PORTF.INTFLAGS & PIN6_bm; //
	
	int y = PORTF.INTFLAGS; //Procedure to
	PORTF.INTFLAGS=y; //clear the interrupt flag
	x=1; //change logic flag to get out of loop
}
