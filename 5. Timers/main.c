/*
	8-bit timer overflow interupt to generate 1ms tick
	16-bit timer to generate PWM output with accurate duty cycle for full servo range 800us-2200us
	Smoothly sweep the duty cycle every second
*/

#include <avr/io.h>
#include <avr/interrupt.h>

int direction = 1;

int main(void){
	// Enable OC1B on PB2 as output / Note also SPI SS
	// DDRB 	= PORTB = (1 << PORTB2);
	DDRD 	= PORTD = (1 << PORTD4);
	// COM0A 	= 0 (off)
	// COM0B 	= 0 (off)
	// WGM 		= 0 (Normal)
	// CS 		= 3 (Prescaler = /64)
	TCCR0A 	= (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
	TCCR0B 	= (0 << WGM02) 	| (0 << CS02) 	| (1 << CS01) 	| (1 << CS00);
	TIMSK0 	= (0 << OCIE0B) | (0 << OCIE0A) | (1 << TOIE0);
	// TCNT0 	= 130; 		// Set the counter to leave 125 clks which takes 1ms @ 8MHz
	TCNT0 	= 0; 		// 16MHz

	// COM1A 	= 0 (off)
	// COM1B 	= 2 (Clear on Compare Match "Non-inverted PWM")
	// WGM 		= 15 (FastPWM)
	// CS 		= 2 (Prescaler = /8)
	TCCR1A 	= (0 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (1 << WGM10);
	TCCR1B 	= (1 << WGM13) 	| (1 << WGM12) 	| (0 << CS12) 	| (1 << CS11) 	| (0 << CS10);
	TIMSK1 	= (0 << OCIE1B) | (0 << OCIE1A) | (0 << TOIE1);
	TCNT1 	= 0; 		// Reset the counter
	OCR1A 	= 2*20000; 	// Set TOP for 50Hz @ 8MHz /8 prescaler
	OCR1B 	= 2*1000;

	// sei();
	while(1){}
}

ISR(TIMER0_OVF_vect){
	if (OCR1B > 2*2300 || OCR1B < 2*700) direction *= -1;
	OCR1B += 2*direction;
	// TCNT0 	= 130;
}
