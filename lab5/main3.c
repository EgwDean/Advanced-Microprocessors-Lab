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
int p = 0;
int humidity;


int main(){
	
	
	PORTD.DIR |= PIN1_bm | PIN2_bm | PIN3_bm; //PIN is output
	PORTD.OUT |= 0b00000111;
	
		//initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLE = 0b00000100; // outisde threshold
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	
	//Window Comparator Mode
	ADC0.WINLT |= 8; //Set threshold
	ADC0.WINHT |= 64;	
	
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
			
	TCA0.SINGLE.CTRLB = 0; //Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc ??? 207)
	//CLOCK_FREQUENCY/1024
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc ??? 224
	
	
	sei();	
	
	while(1){
		
		PORTD.OUT |= 0b00000000; // BREAKPOINT command
		
		if (x == 0) {
			x = 3;
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
			ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
			ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		}
		
		if (x == 1) {
			x = 3;


			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; //Interrupt Enable (=0x10)
			TCA0.SINGLE.CTRLB = 0;
			TCA0.SINGLE.CNT = 0; //clear counter

			int wait_time = ADC0.WINLT - ADC0.RES;
			TCA0.SINGLE.CMP0 = wait_time;//When CMP0 reaches this value -> interrupt 	BREAKPOINT	
			TCA0.SINGLE.CTRLA |= 1; //Enable 
			
		}
		
		if (x == 2) {
			

			PORTD.OUT &= 0b11111001;
			x = 3;
			
			TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
			
			TCA0.SINGLE.CMP0 = 10; //select the duty cycle
			TCA0.SINGLE.PER = 0.5 * TCA0.SINGLE.CMP0; // BREAKPOINT
			
		}
		
	}
}


ISR(ADC0_WCOMP_vect){
	
		if (ADC0.RES < ADC0.WINLT) {
	
			PORTD.OUT &= 0b11111110; // open led0
			PORTD.OUT |= 0b00000010; // close led1
		
			humidity = 0;
		
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
			ADC0.CTRLA &= 0b11111110; //disable ADC
		}

		if (ADC0.RES > ADC0.WINLT) {
			
			PORTD.OUT &= 0b11111101; // open led1
			PORTD.OUT |= 0b00000001; // close led0
		
			humidity = 1;
		
			PORTD.OUT |= 0b00000000; // BREAKPOINT command
			ADC0.CTRLA &= 0b11111110; //disable ADC
				
		}
	
	
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags; // BREAKPOINT
	ADC0.CTRLA &= 0b11111110; //disable ADC		
	
}


ISR(PORTF_PORT_vect){
	
	ADC0.CTRLA &= 0b11111110; //disable ADC		 BREAKPOINT
	
	
	uint8_t humidity_low = PORTF.INTFLAGS & PIN5_bm; // Equals PIN5_bm if pin 5 is pressed, else it's all 0s
	uint8_t humidity_high = PORTF.INTFLAGS & PIN6_bm; //
	
		
	if (humidity == 0 && humidity_low) {
		
		x = 1;	
		PORTD.OUT = 0b00000000; // BREAKPOINT
		
	}
	
	if (humidity == 1 && humidity_high) {
			
		x = 2;
		PORTD.OUT = 0b00000000; // BREAKPOINT
			
	}
	
	if ((humidity == 0 && humidity_high) || (humidity == 1 && humidity_low)) {
		
		x = 0;
		PORTD.OUT |= 0b00000111;	
		PORTD.OUT &= 0b11111000; // BREAKPOINT
		
	}
	
	int y = PORTF.INTFLAGS; //Procedure to
	PORTF.INTFLAGS=y; //clear the interrupt flag
}


ISR(TCA0_CMP0_vect){
		
	TCA0.SINGLE.CTRLA = 0; //Disable
	x = 0;
	PORTD.OUT |= 0b00000111; 
	
	int intflags = TCA0.SINGLE.INTFLAGS; //Procedure to
	TCA0.SINGLE.INTFLAGS=intflags; //clear interrupt flag BREAKPOINT
	
}

ISR(TCA0_OVF_vect){
	p = p + 1; // BREAKPOINT
	
	if (p == 4) {
		TCA0.SINGLE.CTRLA = 0; //Disable
		PORTD.OUT |= 0b00000111; // BREAKPOINT
		x = 0;
		p = 0;
	
	}
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;

}
