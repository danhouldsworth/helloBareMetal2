#include <avr/interrupt.h>

#define	CnFET 5
#define	BnFET 4
#define	AnFET 3
#define	ApFET 2
#define	BpFET 2
#define	CpFET 1

#define AnFET_off 	PORTD &= ~(1 << AnFET)
#define AnFET_on	PORTD |=  (1 << AnFET)
#define BnFET_off 	PORTD &= ~(1 << BnFET)
#define BnFET_on	PORTD |=  (1 << BnFET)
#define CnFET_off 	PORTD &= ~(1 << CnFET)
#define CnFET_on	PORTD |=  (1 << CnFET)
#define ApFET_on 	PORTD &= ~(1 << ApFET)
#define ApFET_off	PORTD |=  (1 << ApFET)
#define BpFET_on 	PORTB &= ~(1 << BpFET)
#define BpFET_off	PORTB |=  (1 << BpFET)
#define CpFET_on 	PORTB &= ~(1 << CpFET)
#define CpFET_off	PORTB |=  (1 << CpFET)

#define RED_ON 		PORTC &= ~(1 << PORTC3)
#define RED_OFF		PORTC |=  (1 << PORTC3)
#define GRN_ON 		PORTC &= ~(1 << PORTC2)
#define GRN_OFF		PORTC |=  (1 << PORTC2)

uint16_t startTime = 0;
uint16_t pulseWidth = 0;

// uint8_t  ZC = 0;
uint8_t  waitingForT0 = 1;
uint8_t  waitingForT2 = 1;

ISR(TIMER0_OVF_vect){
	waitingForT0 = 0; // ~64us per tick / ~16ms per full range
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
// ISR(TIMER1_OVF_vect){
// 	ZC = 1; // if haven't detected ZC break out of waitForEdge()
// }

ISR(TIMER2_OVF_vect){
	waitingForT2 = 0; // ~4us per tick / ~1ms per full range
}

// Global settings

// Timer 1 = 16MHz		Commutation timeing / RC pulse measurement
// Timer 2 = 16MHz 		FET PWM
void wait_16ms(){
	waitingForT0 = 1;
	TCNT0 = 0;
	while (waitingForT0){};
}

void wait_4us(uint8_t ticks){
	waitingForT2 = 1;
	TCNT2 = 254-ticks;
	// if (TCNT2 < 128) TCNT2 = 128;
	while (waitingForT2){};
}

void waitForEdge(uint8_t edgeCompType){
	uint8_t targetEdge = edgeCompType << ACO;
	RED_ON;
	while ((ACSR & (1 << ACO)) != targetEdge){}
	RED_OFF;
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
	TCCR0 	= (1<<CS02) | (0<<CS01) | (1<<CS00); // clk/1024 ~ 16ms per OVF
	TCCR1B 	= (1<<CS10) | (1<<ICNC1); 	// 16MHz with h/w noise reduction on Input Capture
	TCCR2 	= (1<<CS21) | (1<<CS20); 	// clk/64 = 1/4MHz each tick ~4us
	TIMSK 	= (1<<TOIE0)| (0<<TOIE1) | (1<<TOIE2) | (1<<TICIE1); // Enable InputCapture Interrupt : Note -(1<<OCIE1A)
	TIFR 	= (1<<TOIE0)| (1<<TOIE1) | (1<<TOIE2) | (1<<TICIE1); // Clear pending interupts

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

	while(1){

		if (pulseWidth < 16000 || pulseWidth > 32000) {
			RED_ON;
		} else if (pulseWidth > 24000) {
			RED_OFF;
			GRN_ON;
			// uint8_t duty_on = (pulseWidth-24000) >> 5; // 0 --> 1ms | 50% --> 8% |
			AnFET_off;BnFET_off;CnFET_off;
			ApFET_off;BpFET_off;CpFET_off;

			// anti clockwise
			// BpFET_on; CnFET_on; wait_4us(15);BpFET_off;CnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();
			// ApFET_on; CnFET_on; wait_4us(15);ApFET_off;CnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();
			// ApFET_on; BnFET_on; wait_4us(15);ApFET_off;BnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();
			// CpFET_on; BnFET_on; wait_4us(15);CpFET_off;BnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();
			// CpFET_on; AnFET_on; wait_4us(15);CpFET_off;AnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();
			// BpFET_on; AnFET_on; wait_4us(15);BpFET_off;AnFET_off;wait_4us(100);wait_16ms();wait_16ms();wait_16ms();wait_16ms();

			comparator_A();BpFET_on;CnFET_on; wait_4us(15);CnFET_off;wait_4us(100);waitForEdge(1);RED_OFF;
			comparator_B();ApFET_on;CnFET_on; wait_4us(15);CnFET_off;wait_4us(100);waitForEdge(0);RED_OFF;
			comparator_C();ApFET_on;BnFET_on; wait_4us(15);BnFET_off;wait_4us(100);waitForEdge(1);RED_OFF;
			comparator_A();CpFET_on;BnFET_on; wait_4us(15);BnFET_off;wait_4us(100);waitForEdge(0);RED_OFF;
			comparator_B();CpFET_on;AnFET_on; wait_4us(15);AnFET_off;wait_4us(100);waitForEdge(1);RED_OFF;
			comparator_C();BpFET_on;AnFET_on; wait_4us(15);AnFET_off;wait_4us(100);waitForEdge(0);RED_OFF;
		} else {
			AnFET_off;BnFET_off;CnFET_off;
			ApFET_off;BpFET_off;CpFET_off;
			GRN_OFF;
		}

	}
}

		// +     -     Cmp   	Edge
		// B --> C 	Waiting A 	HIGH
		// A --> C 	Waiting B 	LOW
		// A --> B 	Waiting C 	HIGH
		// C --> B 	Waiting A 	LOW
		// C --> A 	Waiting B 	HIGH
		// B --> A 	Waiting C 	LOW


/*
	12v
	--> 1.2v @ 10% duty
	--> 2580rpm ~43r/s
	--> 12N14P (guess?) (it has 12 coils)
	--> 7x43 = 301eRPS
	--> ~3.3ms per loop
	--> ~550ms per commutation section
	--> ~137ticks
*/
