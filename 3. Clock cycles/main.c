#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    DDRD  |=  (1 << PORTD5);

    for (;;) {
        PORTD |=  (1 << PORTD5);
        _delay_ms(500);
        PORTD &= ~(1 << PORTD5);
        _delay_ms(500);
    }

    return 0;
}