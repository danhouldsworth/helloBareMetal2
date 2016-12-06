#include <avr/interrupt.h>  // ISR(), sei, cli, <avr/io.h> for port defs etc
#include <avr/pgmspace.h>
const uint8_t bootOLED[] PROGMEM = {0xAe, 0xD5,0x80,0x8D,0x14,0x20,0x00,0x81,0xFF,0xAF};

uint16_t channelVals[16] = {0,1,2,4,8,16,32,64,128,16,32,64,128,256,512,1024,2048};
uint8_t satFrame[16]; // allow some RX overrun
uint8_t *satByte = satFrame;
uint8_t direction = 1;

void initUSART1();
void processCurrentFrameBuffer();
void displayAllChannelsAsBars();
void displaySingleChannel11Bits();
void setupServoPWM();

void newFrame(){
    satByte = satFrame;
}

void writeSPIstream(const uint8_t *ptr, uint16_t count){
    PORTB &= ~(1 << PORTB4);
    while(count--){
        SPDR = pgm_read_byte(ptr++);
        while(!(SPSR & (1<<SPIF)));
    }
    PORTB |=  (1 << PORTB4);
}

int main(void){
    setupServoPWM();
    // PORTD |= DDRD = (1 << DDD7); // Power on SatRx

    PORTB = DDRB = (1 << DDB5) | (1 << DDB7) | (1 << DDB4) | (1 << DDB1); // MOSI SCK SS D/C
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);

    PORTB &= ~(1 << PORTB1); // OLED 'Commands'
    writeSPIstream(&bootOLED[0],10);
    initUSART1();
    displayAllChannelsAsBars(); // This helps with sanity to show has flashed

    for (;;) {
        sei();
        if (satByte == satFrame + 16) {
            cli();
            processCurrentFrameBuffer();
            newFrame();
            OCR1B = 2000 + channelVals[0];
        }
    }

}

void initUSART1(){
    UBRR1 = 8; // 111,111 @ 16MHz
    // -- Set the mode (00 = Async)
    UCSR1C &= ~((1 << UMSEL11)| (1 << UMSEL10));
    // -- Set the Parity (00 = OFF)
    UCSR1C &= ~((1 << UPM11)  | (1 << UPM10));
    // -- Set the stop bit (0 = 1stop)
    UCSR1C &= ~( 1 << USBS1);
    // -- Set the Character Size bits (011 = 8bits)
    UCSR1C |=   (1 << UCSZ11) | (1 << UCSZ10);
    // -- Enable the PINs for Rx and the interrupt
    UCSR1B |=   (1 << RXEN1) | (1 << RXCIE1);
 }

// -- Fires once when UDR1 contains a byte from Rx
ISR(USART1_RX_vect){
    // read UCSR1A . FE1 in case this byte experienced a frame error (USART frame of 8N1 for this byte)
    *satByte++  = UDR1;
}

void processCurrentFrameBuffer(){
    if (satFrame[0] != 0b00000000){newFrame(); return;}             // Header #0 should always be 0 if Rx booted before Tx
    if (satFrame[1] != 0b10110010){newFrame(); return;}             // Header #1 should match our known bind state : DSMX 11ms
    // if (satFrame[1] != 0b01110010){newFrame(); return;}             // Header #1 should match our known bind state : DSMX 11ms
    // if (satFrame[1] != 0b00000001){newFrame(); return;}             // Header #1 should match our known bind state : DSM2 22mm / 1024
    // if (satFrame[1] != 0b01100010){newFrame(); return;}             // Header #1 should match our known bind state : DSMX 22mm / 2048

    for (uint8_t frameWord = 1; frameWord <= 7; frameWord++){
        uint8_t wordHB  = satFrame[frameWord * 2 + 0];
        uint8_t wordLB  = satFrame[frameWord * 2 + 1];

        uint8_t wordHeader = wordHB >> 7;
        if (wordHeader != 0) {continue;}

        uint8_t channel = (0b01111000 & wordHB) >> 3; // DSMX / 2048
        // uint8_t channel = (0b00111000 & wordHB) >> 3; // Orange
        // uint8_t channel = (0b00011100 & wordHB) >> 2; // DSM2 / 1024

        // channelVals[channel] =  (uint16_t)0;
        // channelVals[channel] += (uint16_t)(wordHB << 8);
        // channelVals[channel] += (uint16_t)(wordLB);
        uint8_t servoHB = 0b00000111 & wordHB;
        uint8_t servoLB = wordLB;
        channelVals[channel] =  (uint16_t)0;
        channelVals[channel] += (uint16_t)(servoHB << 8);
        channelVals[channel] += (uint16_t)(servoLB);
    }
    displayAllChannelsAsBars();
    // displaySingleChannel11Bits();
}

void displayAllChannelsAsBars(){
    PORTB |=  (1 << PORTB1); // OLED 'Data'
    PORTB &= ~(1 << PORTB4);
    for (uint8_t col = 0; col < 8; col++){
        for (uint8_t x = 0; x < 128; x++){
            if (x < (uint8_t)(channelVals[col] >> 4)) {SPDR = 255;}
            else {SPDR = 0;}
            while(!(SPSR & (1<<SPIF)));
        }
    }
    PORTB |=  (1 << PORTB4);
}

void displaySingleChannel11Bits(){
    PORTB |=  (1 << PORTB1); // OLED 'Data'
    PORTB &= ~(1 << PORTB4);

    for (uint8_t channelPr = 0; channelPr < 8; channelPr++){
    // for (uint8_t channelPr = 8; channelPr < 16; channelPr++){
        SPDR = 255;while(!(SPSR & (1<<SPIF)));
        for (uint8_t bit = 0; bit < 16; bit++){
            if (bit == 8){
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 255;while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 255;while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
            }
            if (bit == 11){
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
                SPDR = 255;while(!(SPSR & (1<<SPIF)));
                SPDR = 0;  while(!(SPSR & (1<<SPIF)));
            }
            // SPDR = 17 ;while(!(SPSR & (1<<SPIF)));
            SPDR = 17 ;while(!(SPSR & (1<<SPIF)));
            uint8_t val =   (channelVals[channelPr*2] & (1 << bit)) ? 0b00000111 : 0;
            val +=          (channelVals[channelPr*2+1] & (1 << bit)) ? 0b01110000 : 0;
            SPDR = val; while(!(SPSR & (1<<SPIF)));
            SPDR = val; while(!(SPSR & (1<<SPIF)));
            SPDR = val; while(!(SPSR & (1<<SPIF)));
            SPDR = val; while(!(SPSR & (1<<SPIF)));
            SPDR = val; while(!(SPSR & (1<<SPIF)));
            SPDR = val; while(!(SPSR & (1<<SPIF)));
        }
        SPDR = 255;while(!(SPSR & (1<<SPIF)));
    }
    PORTB |=  (1 << PORTB4);
}

void setupServoPWM(){
    // Enable OC1B on PB2 as output / Note also SPI SS
    DDRD    |= (1 << PORTD4);
    PORTD   |= (1 << PORTD4);
    // COM1A    = 0 (off)
    // COM1B    = 2 (Clear on Compare Match "Non-inverted PWM")
    // WGM      = 15 (FastPWM)
    // CS       = 2 (Prescaler = /8)
    TCCR1A  = (0 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (1 << WGM10);
    TCCR1B  = (1 << WGM13)  | (1 << WGM12)  | (0 << CS12)   | (1 << CS11)   | (0 << CS10);
    TIMSK1  = (0 << OCIE1B) | (0 << OCIE1A) | (0 << TOIE1);
    TCNT1   = 0;        // Reset the counter
    OCR1A   = 2*20000;  // Set TOP for 50Hz @ 8MHz /8 prescaler
    OCR1B   = 2*1000;
}