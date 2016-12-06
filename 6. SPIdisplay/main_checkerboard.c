// ;               328p          1284p
// ; Clock         16MHz           16MHz
// ; OLED D/C      B1              B1

// ; SS            B2              B4
// ; MOSI          B3              B5
// ; MISO          B4              B6
// ; SCK           B5              B7

#include <avr/io.h>

void bootOLED(uint8_t *ptr){
    for (uint8_t i = 0; i < 10; i++){
        SPDR = *ptr++;
        while(!(SPSR & (1<<SPIF)));
    }
}

int main(void){
    // MOSI SCK SS Must all be set to output for SPI to work in Master
    PORTB = DDRB = (1 << DDB5) | (1 << DDB7) | (1 << DDB4) | (1 << DDB1);   // ATmega1284P
    // PORTB = DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2) | (1 << DDB1);   // ATmega328P
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);

    PORTB &= ~(1 << PORTB1); // OLED 'Commands'
    PORTB &= ~(1 << PORTB4);
    bootOLED((uint8_t[]){0xAE,0xD5,0x80,0x8D,0x14,0x20,0x00,0x81,0xFF,0xAF});
    PORTB |=  (1 << PORTB4);

    PORTB |=  (1 << PORTB1); // OLED 'Data'
    PORTB &= ~(1 << PORTB4);

    for(uint16_t frame = 0;;frame++)
        for (uint8_t col = 8; col > 0; col--)
            for (uint8_t x = 128; x > 0; x--)
                SPDR = (( (( ( x-(frame>>col) )&8 )>>3) + (col&1) )&1 )*255; // Multi speed lines
}

