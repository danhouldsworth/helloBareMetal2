#include <avr/interrupt.h>

int main(void){
    PCICR  = (1 << PCIE0);          // Enables PC interrupt #0
    PCMSK0 = (1 << PCINT1);         // Enable PCINT1 to fire PC INT #0
    sei();

    DDRB  = PORTB |=  (1 << 4) | (1 << 5) | (1 << 7);
    DDRC  = PORTC |=  (1 << 7);
    DDRD  |=  (1 << 5);
    PORTD &= ~(1 << 5);

    for(;;){}
	return 0;
}

ISR(PCINT0_vect){
    PORTB = PORTC = 0;
    PORTD |=  (1 << 5);
}
