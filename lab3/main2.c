/*
 * lab3.c
 *
 * Created: 20/03/2025 16:23:54
 * Author : Theo
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define value1 20
#define value2 40

int x;
int z;
int left = 0;
int right = 0;


int main(){
	
	//Led setup
	PORTD.DIR = 0b00000111; 
	PORTD.OUT &= 0b11111000;
	
	//initialize the ADC 
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution

	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	//Window Comparator Mode
	
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINLT	
	
	//Timer setup
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc σελ 207)
	//CLOCK_FREQUENCY/1024
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc σελ 224
	TCA0.SINGLE.CTRLA |= 1; //Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion	
	
	sei();

	x = 1; (insert breakpoint)
	
	
	
	while(1){
		
		
		if (x == 1){ //side sensor
				
	
				ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode disabled
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value1;
				z = 1;
				x = 0;
				ADC0.CTRLE = 0b00000010;//Interrupt when RESULT > WINLT		(insert breakpoint)
					
		}
		
		if (x == 2){ //forward sensor
				


				PORTD.OUT |= 0b00000010;
			
				ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
				TCA0.SINGLE.CNT = 0; //clear counter
				TCA0.SINGLE.CMP0 = value2;
				z = 2;
				x = 0;
				ADC0.CTRLE = 0b00000001; //Interrupt when RESULT < WINLT (insert breakpoint)
				
								
		}
		
		if ( (left == (right + 4)) || (right == (left + 4)))
			break;
		
		PORTD.OUT |= 0b00000000; // NOP 
	}
}

ISR(ADC0_WCOMP_vect){
	
	if (z == 1){
		
		PORTD.OUT &= 0b11111101;
		PORTD.OUT |= 0b00000001;
		right = right + 1; // CHANGE VALUE IDIOT (insert breakpoint)
		PORTD.OUT &= 0b11111110;
		PORTD.OUT |= 0b00000010;
	}
	
	if (z == 2){
		PORTD.OUT &= 0b11111101;
		PORTD.OUT |= 0b00000100;		
		left = left + 1;	// CHANGE VALUE IDIOT (insert breakpoint)
		PORTD.OUT &= 0b11111011;
		PORTD.OUT |= 0b00000010;
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
		PORTD.OUT &= 0b11111101;
		ADC0.CTRLE = 0b00000000; // disable int	(insert breakpoint)	
	}
	
	int intflags = TCA0.SINGLE.INTFLAGS; //Procedure to
	TCA0.SINGLE.INTFLAGS=intflags; //clear interrupt flag

}
