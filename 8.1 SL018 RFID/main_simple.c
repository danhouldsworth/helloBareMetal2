/*
                328p        1284p
    SDA         PC4         PC1
    SCL         PC5         PC0
    Board       SL018       DC1338
    Trigger     PC3         -
    SQW         -           PD7

==> Super simple communication over I2C.
==> Send a single (LED ON) command to a slave (SL018 RFID) at 400kHz
*/

#include <avr/io.h>
#include <avr/interrupt.h>

int main() {
    PORTC |= (1 << PORTC5) | (1 << PORTC4);         // BOTH SDA & SDL can be IN/OUT but must start HIGH

    TWSR = 0;               // Set both prescaler bits for 0 === /1
    TWBR = 2;               // BitRate = 400kHz @ 8MHz /1 prescaler

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start

    TWDR = 0b10100000;      // SL018 slave address : WRITE
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWDR = 2;               // SL018 - 2 bytes follow
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWDR = 0x40;            // SL018 - CMD == RED LED
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWDR = 1;               // SL018 - "ON"
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
}
