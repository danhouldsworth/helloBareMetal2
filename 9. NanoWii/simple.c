#include <avr/io.h>
#include <avr/interrupt.h>

unsigned long int   g_iTimer    = 0;
uint8_t             g_bOverTime = 0;
void wait(unsigned long int ms) {
    g_iTimer        = ms;
    g_bOverTime     = 0;
    while(g_bOverTime == 0){}
    g_iTimer = 0;
}
ISR(TIMER0_OVF_vect) {
    TCNT0 = 0;
    if (g_iTimer != 0) {
        g_iTimer--;
        if (g_iTimer == 0) {
            g_bOverTime = 1;
        }
    }
}

int main() {
    cli();
    UDIEN = 0;
    DDRD = PORTD |=  (1 << 5); // Green LED  : LOW = ON
    DDRC = PORTC |=  (1 << 7); // YELLOW LED  : HIGH = ON

    //============================================
    // Setup TIMER0 as ms clock
    //============================================
    // COM0A    = 0 (off)
    // COM0B    = 0 (off)
    // WGM      = 0 (Normal)
    // CS       = 3 (Prescaler = /64)
    TCCR0A  = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
    TCCR0B  = (0 << WGM02)  | (0 << CS02)   | (1 << CS01)   | (1 << CS00);
    TIMSK0  = (0 << OCIE0B) | (0 << OCIE0A) | (1 << TOIE0);
    TCNT0    = 0;      // Set the counter to leave 125 clks which takes 1ms @ 16MHz
    //============================================
    sei();
    while(1) {
        wait(500);
        PORTC ^= (1 << 7);
        PORTD ^= (1 << 5);
    }
}