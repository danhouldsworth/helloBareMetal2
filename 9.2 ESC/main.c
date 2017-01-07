#include <avr/interrupt.h>

// --- PORT B -----
#define	BpFET 2
#define	CpFET 1
#define BpFET_on 	PORTB &= ~(1 << BpFET)
#define BpFET_off	PORTB |=  (1 << BpFET)
#define CpFET_on 	PORTB &= ~(1 << CpFET)
#define CpFET_off	PORTB |=  (1 << CpFET)
// ----------------

// --- PORT C -----
#define RED_ON 		PORTC &= ~(1 << PORTC3)
#define RED_OFF		PORTC |=  (1 << PORTC3)
#define GRN_ON 		PORTC &= ~(1 << PORTC2)
#define GRN_OFF		PORTC |=  (1 << PORTC2)
// ----------------

// --- PORT D -----
#define	CnFET 5
#define	BnFET 4
#define	AnFET 3
#define	ApFET 2
#define AnFET_off 	PORTD &= ~(1 << AnFET)
#define AnFET_on	PORTD |=  (1 << AnFET)
#define BnFET_off 	PORTD &= ~(1 << BnFET)
#define BnFET_on	PORTD |=  (1 << BnFET)
#define CnFET_off 	PORTD &= ~(1 << CnFET)
#define CnFET_on	PORTD |=  (1 << CnFET)
#define ApFET_on 	PORTD &= ~(1 << ApFET)
#define ApFET_off	PORTD |=  (1 << ApFET)
// ----------------

uint16_t startTime 	= 0;
uint16_t pulseWidth 	= 0;

uint8_t waitingForT0 	= 0;
uint8_t T2hb 		= 2;

uint8_t PWM_nFET 	= '';

ISR(TIMER0_OVF_vect){
	if (waitingForT0) waitingForT0--;
}

void wait16ms(uint8_t count){
	waitingForT0 = count;
	TCNT0 = 0;
	while (waitingForT0){}
}

ISR(TIMER1_CAPT_vect){
	uint16_t timer = ICR1;
	uint8_t risingEdge = TCCR1B & (1 << ICES1);
	if (risingEdge) {
		startTime = timer;
		TCCR1B &= ~(1<<ICES1); // Set next int to falling edge
	} else {
		pulseWidth = timer - startTime;
		TCCR1B |= (1<<ICES1); // Set next int to rising edge
	}
}

ISR(TIMER2_COMP_vect){
	switch (PWM_nFET){
		case 'A' : AnFET_off; break;
		case 'B' : BnFET_off; break;
		case 'C' : CnFET_off; break;
		default : AnFET_off;BnFET_off;CnFET_off;
	}
}
ISR(TIMER2_OVF_vect){
	if (T2hb++ < 2) {return;}
	T2hb = 2;

	switch (PWM_nFET){
		case 'A' : AnFET_on; break;
		case 'B' : BnFET_on; break;
		case 'C' : CnFET_on; break;
		default : AnFET_off;BnFET_off;CnFET_off;
	}
}
void wait128us(){
	waitingForT0 = 1;
	TCNT0 = 0;
	while(waitingForT0){}
	return;
}
void waitForEdge(uint8_t edgeCompType){
	uint8_t targetEdge = edgeCompType << ACO;
	while ( (ACSR & (1 << ACO)) != targetEdge){}
}

void comparator_A(){
	AnFET_off; ApFET_off;
	ADMUX = 0; 					// set comparator multiplexer to phase A
	ADCSRA&= ~(1<<ADEN); 		// Disable ADC if we enabled it to get AIN1
}
void comparator_B(){
	BnFET_off; BpFET_off;
	ADMUX = 1; 					// set comparator multiplexer to phase B
	ADCSRA&= ~(1<<ADEN); 		// Disable ADC if we enabled it to get AIN1
}
void comparator_C(){
	CnFET_off; CpFET_off;
	ADCSRA|= (1<<ADEN); 		// Enable ADC to effectively disable ACME
}
/*
See Table72
ACME 		ADEN 		MUX2..0 	Analog Comparator Negative Input
0		 	x		 	xxx 		AIN1
1		 	1		 	xxx 		AIN1
1		 	0		 	000 		ADC0 	[AFRO_mux_a]
1		 	0		 	001 		ADC1 	[AFRO_mux_b]
1		 	0		 	010 		ADC2
1		 	0		 	011 		ADC3
1		 	0		 	100 		ADC4
1		 	0		 	101 		ADC5
1		 	0		 	110 		ADC6
1		 	0		 	111 		ADC7

*/

int main(void){
	TCCR0 	= (0<<CS02) | (1<<CS01) | (0<<CS00); 				// Manual Commutations	 2MHz
	TCCR1B 	= (0<<CS12) | (0<<CS11) | (1<<CS10) | (1<<ICNC1); 		// RC INPUT 	16MHz[h/w noise reduction on Input Capture]
	TCCR2 	= (0<<CS22) | (1<<CS21) | (0<<CS20); 				// FET PWM 		 2MHz
	TIMSK 	= (1<<TOIE0)| (0<<TOIE1) | (1<<TOIE2) | (1<<TICIE1) | (1<<OCIE2);
	TIFR 	= (1<<TOIE0)| (1<<TOIE1) | (1<<TOIE2) | (1<<TICIE1) | (1<<OCIE2);

	OCR2 	= 0;
	TCNT0 	= 0;
	TCNT1 	= 0;
	TCNT2 	= 0;
	SFIOR 	|= (1<<ACME);					// set Analog Comparator Multiplexer Enable

	DDRC 	= (1 << PORTC3) | (1 << PORTC2);

	DDRB	= (1 << BpFET) | (1 << CpFET);
	PORTB	= (1 << BpFET) | (1 << CpFET);

	DDRD 	= (1 << AnFET) | (1 << BnFET) | (1 << CnFET) | (1 << ApFET);
	PORTD	= (1 << ApFET);

	sei();
	RED_ON;
	while(1){
		if (pulseWidth < 16000 || pulseWidth > 32000) {
			AnFET_off;BnFET_off;CnFET_off;
			ApFET_off;BpFET_off;CpFET_off;
			// RED_ON;
			GRN_OFF;
		} else {
			GRN_ON;
			OCR2 = (pulseWidth - 16000) >> 8;
			if (OCR2 < 1) 	OCR2 = 1;
			// if (OCR2 > 10) 	OCR2 = 10;
			// OCR2 = 1;
			TCNT2 = 0;
			AnFET_off;BnFET_off;CnFET_off;
			ApFET_off;BpFET_off;CpFET_off;

			// anti clockwise (on current wiring)
			comparator_A();BpFET_on;PWM_nFET = 'C'; waitForEdge(1);
			comparator_B();ApFET_on;PWM_nFET = 'C'; waitForEdge(0);
			comparator_C();ApFET_on;PWM_nFET = 'B'; waitForEdge(1);
			comparator_A();CpFET_on;PWM_nFET = 'B'; waitForEdge(0);
			comparator_B();CpFET_on;PWM_nFET = 'A'; waitForEdge(1);
			comparator_C();BpFET_on;PWM_nFET = 'A'; waitForEdge(0);
			RED_OFF;
		}
	}
}
/*
Commutations
	+     -     Comparator  Edge
	B --> C 	Waiting A 	HIGH
	A --> C 	Waiting B 	LOW
	A --> B 	Waiting C 	HIGH
	C --> B 	Waiting A 	LOW
	C --> A 	Waiting B 	HIGH
	B --> A 	Waiting C 	LOW

Estimations on crude timing
	12v
	--> 1.2v @ 10% duty
	--> 2150kV => 2580rpm ~43rps
	--> 12N14P (guess?) (it has 12 coils)
	--> 7x43 = 301eRPS
	--> ~3.3ms per loop
	--> ~550us per commutation section

CS12 		CS11 		CS10		Description 		RATE 	Tick 	    Ticks/us 	8-bitOVF 	16-bitOVF
0 		0 		0		No clock
0 		0 		1		clk/1 			 16MHz   60ns  		 16 	  16us	    	4096us
0 		1 		0		clk/8 			  2MHz  0.5us 		  2 	 128us  	  33ms
0 		1 		1		clk/64 		 	250KHz 	  4us 		1/4 	 1.0ms 		 250ms
1 		0 		0		clk/256  		 62KHz 	 16us 		1/16	 4.1ms 		 1.0s
1 		0 		1		clk/1024 		 15KH`   64us 		1/64	16.4ms 		 4.2s
*/