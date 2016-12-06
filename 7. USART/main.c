#include <avr/interrupt.h>  // ISR(), sei, cli, <avr/io.h> for port defs etc
// #include <stdio.h>          // sprintf() and standard types

#define USART_BAUDRATE 115200
// Calc UBBR = ((F_CPU / (USART_BAUDRATE * 16UL))) - 1
// However, needs to be an int, and don't want risk of truncation down so easier to set manually (for higher BAUDs)
// (See Datasheet 19.11 Examples of BaudRate) For 16MHz, 115200, normal asyncronous === 8 (== 3 @ 8MHz)
#define UBRR_VALUE 3

int main(void){
    // -- Set BAUD (Let GCC handle the low byte / high byte assigment of 16bit reg)
    UBRR0 = UBRR_VALUE;
    // -- Set the mode (00 = Async)
    UCSR0C &= ~((1 << UMSEL01)| (1 << UMSEL00));
    // -- Set the Parity (00 = OFF)
    UCSR0C &= ~((1 << UPM01)  | (1 << UPM00));
    // -- Set the stop bit (0 = 1stop)
    UCSR0C &= ~( 1 << USBS0);
    // -- Set the Character Size bits (011 = 8bits)
    UCSR0C |=   (1 << UCSZ01) | (1 << UCSZ00);
    // --

    // -- Enable the PINs for Rx and Tx
    UCSR0B |=   (1 << RXEN0) | (1 << TXEN0);
    // -- and the interrupts
    UCSR0B |=   (1 << RXCIE0) | (1 << TXCIE0) | (0 << UDRIE0);
    // --
}