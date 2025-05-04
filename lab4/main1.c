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

int x = 0;

int main(){
	
	PORTD.DIR |= PIN0_bm | PIN1_bm; //PIN is output
	PORTD.OUT |= 0b00000011;
	//prescaler=1024
	TCA0.SPLIT.CTRLD = 1; // Enable split mode 
	TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
		
	//select Single_Slope_PWM
	//TCA0.SPLIT.CTRLB=0b00010001;
	TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP0EN_bm | TCA_SPLIT_HCMP0EN_bm;
	
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
    TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm |  TCA_SPLIT_HUNF_bm;
	
	//TCA0.SPLIT.CTRLA |= TCA_SPLIT_ENABLE_bm; //Enable TCA
		
	sei(); // breakpoint
	
		
	while (1){
		
		
	if (x == 1){
		
		x = x + 1;
		
		TCA0.SPLIT.LPER = T1; // wings
		TCA0.SPLIT.HPER = T2; // base		
				
		TCA0.SPLIT.LCMP0 = 0.6 * TCA0.SPLIT.LPER;		
		TCA0.SPLIT.HCMP0 = 0.4 * TCA0.SPLIT.HPER;

		TCA0.SPLIT.LCNT = 1;
		TCA0.SPLIT.HCNT = 0;
		
		TCA0.SPLIT.CTRLA |= TCA_SPLIT_ENABLE_bm; //Enable TCA breakpoint	
		
	}
	
	if (x == 3) {
		
		PORTD.OUT |= 0b00000011;  
		x = x + 1;
		
		TCA0.SPLIT.LPER = (T1 / 2);
			
		TCA0.SPLIT.LCMP0 = 0.5 * TCA0.SPLIT.LPER;

		TCA0.SPLIT.LCNT = 0;
		TCA0.SPLIT.HCNT = 1; // breakpoint
		
	}
	
	if (x == 5) {
		
		PORTD.OUT |= 0b00000011;
		x = x + 1;
		TCA0.SPLIT.CTRLA &= ~TCA_SPLIT_ENABLE_bm; //Disable TCA	breakpoint
		
	}
		
		
	PORTD.OUT |= 0b00000000; //breakpoint
	
	
	
	}
}

ISR(TCA0_LUNF_vect){ // every 1 ms, wings
	
	PORTD.OUT ^= PIN0_bm; 
	
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags; // breakpoint

}

ISR(TCA0_HUNF_vect){ // every 2 ms, base

	PORTD.OUT ^= PIN1_bm; 

	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags; // breakpoint

}


ISR(PORTF_PORT_vect){
	int y = PORTF.INTFLAGS; //Procedure to
	PORTF.INTFLAGS=y; //clear the interrupt flag
	x = x + 1; //change logic flag to get out of loop breakpoint
	
	if (x == 7){
		x = 1; // breakpoint
	}
}
