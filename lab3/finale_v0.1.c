/*
 * lab3.c
 *
 * Created: 20/03/2025 16:23:54
 * Author : Theo & Dean
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define value1 20
#define value2 40
#define value3 60

int x;
int z;
int left = 0;
int right = 0;
int inverted = 0;


int main(){
	
	//Led setup
	PORTD.DIR = 0b00000111; 
	PORTD.OUT |= 0b00000101;
	PORTD.OUT &= 0b11111101;
	
	//initialize the ADC 
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	//Window Comparator Mode
	
	ADC0.WINLT |= 8; //Set threshold for forward
	ADC0.WINHT |= 32; //Set threshold for side
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINLT	
	
	//Timer setup
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc ??? 207)
	//CLOCK_FREQUENCY/1024
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc ??? 224
	TCA0.SINGLE.CTRLA |= 1; //Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion	
	
	
	//Button setup
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	sei();

	x = 1; //(insert breakpoint)
	
	
	
	while(1){
		
		
		if (x == 1){ //side sensor
					
				
				//ADC0.CTRLA ^= (1 << 1);//Free-Running mode disabled
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value1;
				z = 1;
				x = 0;
				ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT		(insert breakpoint)
					
		}
		
		if (x == 2){ //forward sensor
				
		
				ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value2;
				z = 2;
				x = 0;
				ADC0.CTRLE = 0b00000001; //Interrupt when RESULT < WINLT (insert breakpoint)
												
		}
		
		if ( (left == (right + 4)) || (right == (left + 4)))
			break;
		
		if ((inverted == 1) && ((right == -1) || (left == -1)))
			break;
			
					
		PORTD.OUT |= 0b00000000; // NOP 
	}
}

ISR(ADC0_WCOMP_vect){
	
	if (z == 1 && inverted == 0){ //right
		
		PORTD.OUT |= 0b00000010;
		PORTD.OUT &= 0b11111110;
		
		right = right + 1; // CHANGE VALUE IDIOT (insert breakpoint)
		
		PORTD.OUT |= 0b00000001;
		PORTD.OUT &= 0b11111101;

	}
	
	if (z == 2 && inverted == 0){ //left
		
		PORTD.OUT |= 0b00000010;
		PORTD.OUT &= 0b11111011;		
		
		left = left + 1;	// CHANGE VALUE IDIOT (insert breakpoint)
		
		PORTD.OUT |= 0b00000100;
		PORTD.OUT &= 0b11111101;
		
	}
	
	if ((inverted == 1) && (z == 1)) { //inverted left
		
		PORTD.OUT |= 0b00000010;		
		PORTD.OUT &= 0b11111011;
		
		right = right - 1;	// CHANGE VALUE IDIOT (insert breakpoint)
		
		PORTD.OUT |= 0b00000100;
		PORTD.OUT &= 0b11111101;
		
	}
		
	if ((inverted == 1) && (z == 2)) { //inverted right
		
		PORTD.OUT &= 0b11111110;
		PORTD.OUT |= 0b00000010;
		
		left = left - 1; // CHANGE VALUE IDIOT (insert breakpoint)

		PORTD.OUT &= 0b11111101;
		PORTD.OUT |= 0b00000001;
		
	}
	
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;

}

ISR(TCA0_CMP0_vect){
	
	if (z == 1){
		x = 2;
		ADC0.CTRLE = 0b00000000; // disable int	(insert breakpoint)
	}
		
	if (z == 2){
		x = 1;
		ADC0.CTRLE = 0b00000000; // disable int	(insert breakpoint)	
	}
		
	if (inverted == 2) {
		
		inverted = 1;
		PORTD.OUT |= 0b00000101;
		x = 1;
	}
	
	int intflags = TCA0.SINGLE.INTFLAGS; //Procedure to
	TCA0.SINGLE.INTFLAGS=intflags; //clear interrupt flag

}


ISR(PORTF_PORT_vect){
	int y = PORTF.INTFLAGS; //Procedure to
	PORTF.INTFLAGS=y; //clear the interrupt flag
	inverted = 2; //change logic flag to get out of loop (insert breakpoint)
	x = 3;
	z = 3;
	PORTD.OUT &= 0b11111000;
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CMP0 = value3;
	ADC0.CTRLE = 0b00000000; // disable int	(insert breakpoint)	

}
