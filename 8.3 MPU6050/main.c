/*
            328p        1284p
    SDA     PC4         PC1
    SCL     PC5         PC0
    Board   SL018       DC1338
    Trigger PC3         -
    SQW     -           PD7

MPU-6050 Breakout board
*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#define SLVADD      0xd0 //
// #define SLVADD      0b00111010 // ADXL345

//============================================
//  global variable define
//============================================
unsigned long int   g_iTimer    = 0;
uint8_t             g_bOverTime = 0;
uint8_t             RxBuffer[16];
//============================================

//============================================
// OLED display in PRGSPACE
//============================================
const char OLEDbootSequence[]   PROGMEM = {0xAe,0xD5,0x80,0x8D,0x14,0x20,0x00,0x81,0xFF,0xAF};
const char asciiBitmaps[760]    PROGMEM = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x41, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xB7, 0x01, 0xB7, 0x01, 0xB7, 0xFF, 0xFF, 0xFF, 0x37, 0x6B, 0x69, 0x5B, 0x93, 0xFF, 0xFF, 0xFF, 0xFB, 0xA5, 0x4B, 0xAF, 0xF7, 0xFF, 0xFF, 0xFF, 0xBF, 0x57, 0x4B, 0xBB, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x79, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x79, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xE7, 0xF1, 0xE7, 0xFB, 0xFF, 0xFF, 0xFF, 0xDF, 0xDF, 0x07, 0xDF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xDF, 0xDF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xC7, 0xF9, 0xFE, 0xFF, 0xFF, 0xFF, 0x87, 0x7B, 0x7D, 0x7B, 0x87, 0xFF, 0xFF, 0xFF, 0x7D, 0x7D, 0x01, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x7B, 0x3D, 0x5D, 0x6D, 0x33, 0xFF, 0xFF, 0x7F, 0x7D, 0x6D, 0x6D, 0x93, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xD7, 0x5B, 0x01, 0x5F, 0xFF, 0xFF, 0xFF, 0x61, 0x6D, 0x6D, 0x6D, 0x9F, 0xFF, 0xFF, 0xFF, 0x87, 0x5B, 0x6D, 0x6D, 0x9D, 0xFF, 0xFF, 0xFF, 0xF9, 0xFD, 0x3D, 0xC5, 0xF9, 0xFF, 0xFF, 0xFF, 0x93, 0x6D, 0x6D, 0x63, 0x9F, 0xFF, 0xFF, 0xFF, 0x63, 0x5D, 0x6D, 0xAB, 0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0x6F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0x9F, 0xAF, 0x6F, 0x77, 0xFF, 0xFF, 0xFF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xFF, 0xFF, 0xFF, 0x77, 0x6F, 0xAF, 0x9F, 0xDF, 0xFF, 0xFF, 0xFF, 0xFB, 0x7D, 0x4D, 0xED, 0xF3, 0xFF, 0xFF, 0xC7, 0xBB, 0x6D, 0x55, 0x6D, 0x5B, 0xE7, 0xFF, 0x7F, 0x0D, 0xD1, 0xDD, 0xD3, 0x0F, 0x7F, 0xFF, 0x7D, 0x01, 0x6D, 0x6D, 0x6D, 0x93, 0xFF, 0xFF, 0xFF, 0xC7, 0xBB, 0x7D, 0x7D, 0xB1, 0xFF, 0xFF, 0x7D, 0x01, 0x7D, 0x7D, 0xBB, 0xC7, 0xFF, 0xFF, 0x7D, 0x01, 0x6D, 0x6D, 0x65, 0x3D, 0xFF, 0xFF, 0xFF, 0x7D, 0x01, 0x6D, 0xC5, 0xFD, 0xF9, 0xFF, 0xC7, 0xBB, 0x7D, 0x5D, 0x5B, 0x91, 0xDF, 0xFF, 0x7D, 0x01, 0x6D, 0xEF, 0x6D, 0x01, 0x7D, 0xFF, 0xFF, 0x7D, 0x7D, 0x01, 0x7D, 0x7D, 0xFF, 0xFF, 0x9F, 0x7F, 0x7D, 0x7D, 0x81, 0xFD, 0xFF, 0xFF, 0x7D, 0x01, 0x6D, 0xE7, 0x99, 0x7D, 0xFF, 0xFF, 0xFF, 0x7D, 0x01, 0x7D, 0x7F, 0x1F, 0xFF, 0xFF, 0xFF, 0x01, 0xF3, 0xCF, 0xF3, 0x01, 0xFF, 0xFF, 0x7D, 0x01, 0x7B, 0xC7, 0xBD, 0x01, 0xFD, 0xFF, 0xC7, 0xBB, 0x7D, 0x7D, 0xBB, 0xC7, 0xFF, 0xFF, 0xFF, 0x7D, 0x01, 0x5D, 0xDD, 0xE3, 0xFF, 0xFF, 0xC7, 0xBB, 0x7D, 0x7D, 0xBB, 0xC7, 0xFF, 0xFF, 0x7D, 0x01, 0x6D, 0xED, 0x8D, 0x73, 0x7F, 0xFF, 0xFF, 0x33, 0x6D, 0x6D, 0x6D, 0x99, 0xFF, 0xFF, 0xF1, 0xFD, 0x7D, 0x01, 0x7D, 0xFD, 0xF1, 0xFF, 0xFD, 0x81, 0x7D, 0x7F, 0x7D, 0x81, 0xFD, 0xFF, 0xFD, 0xE1, 0x9F, 0x3F, 0xC1, 0xFD, 0xFF, 0xFF, 0xC1, 0x3D, 0xCF, 0xC7, 0x3D, 0xC1, 0xFF, 0xFF, 0x7F, 0x3D, 0xD1, 0xC7, 0x39, 0x7D, 0xFF, 0xFF, 0xFD, 0x79, 0x67, 0x0F, 0x71, 0xFD, 0xFF, 0xFF, 0xFF, 0x31, 0x5D, 0x6D, 0x71, 0x1D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF9, 0xC7, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFC, 0xFD, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFB, 0xFB, 0xFF, 0xFF, 0xFF, 0xBF, 0x57, 0x57, 0x97, 0x0F, 0x7F, 0xFF, 0xFF, 0x7D, 0x01, 0x77, 0x77, 0x77, 0x8F, 0xFF, 0xFF, 0xFF, 0x8F, 0x77, 0x77, 0x77, 0xA7, 0xFF, 0xFF, 0x8F, 0xAF, 0x77, 0x75, 0xAD, 0x01, 0x7F, 0xFF, 0xFF, 0x8F, 0x57, 0x57, 0x57, 0x4F, 0xFF, 0xFF, 0xFF, 0x77, 0x03, 0x75, 0x75, 0xFD, 0xFF, 0xFF, 0x8F, 0xAF, 0x77, 0x77, 0xAF, 0x07, 0xF7, 0xFF, 0x7D, 0x01, 0x6F, 0xF7, 0x77, 0x0F, 0x7F, 0xFF, 0xFF, 0x77, 0x77, 0x05, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xF7, 0xF7, 0xF5, 0x07, 0xFF, 0xFF, 0xFF, 0x7D, 0x01, 0xDF, 0xCF, 0x37, 0x77, 0xFF, 0xFF, 0xFF, 0x7D, 0x7D, 0x01, 0x7F, 0x7F, 0xFF, 0xFF, 0xF7, 0x07, 0xF7, 0x0F, 0xF7, 0x0F, 0x7F, 0xFF, 0x77, 0x07, 0x6F, 0xF7, 0x77, 0x0F, 0x7F, 0xFF, 0x8F, 0xAF, 0x77, 0x77, 0x77, 0x8F, 0xFF, 0xFF, 0xF7, 0x07, 0xAF, 0x77, 0x77, 0x8F, 0xFF, 0xFF, 0x8F, 0xAF, 0x77, 0x77, 0xAF, 0x07, 0xF7, 0xFF, 0xFF, 0x77, 0x07, 0x6F, 0x77, 0xF7, 0xFF, 0xFF, 0xFF, 0x6F, 0x57, 0x57, 0x57, 0xB7, 0xFF, 0xFF, 0xF7, 0x83, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xF7, 0x87, 0x7F, 0x7F, 0xB7, 0x07, 0x7F, 0xFF, 0xF7, 0xC7, 0x3F, 0xBF, 0xC7, 0xF7, 0xFF, 0xFF, 0xE7, 0x97, 0x3F, 0xCF, 0x3F, 0x97, 0xE7, 0xFF, 0x7F, 0x37, 0xA7, 0xDF, 0x27, 0x77, 0xFF, 0xFF, 0xF7, 0xE7, 0x9F, 0x7F, 0x9F, 0xE7, 0xF7, 0xFF, 0xFF, 0x67, 0x37, 0x57, 0x67, 0x37, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xDF, 0xDF, 0xBF, 0xDF, 0xFF, 0xFF};
//============================================
const char commandTest[]  = {7,0,0,0,0,0,0,0};
//============================================
// SPI / OLED functions
//============================================
void writeSPIbyte(char value){SPDR = value; while(!(SPSR & (1<<SPIF)));}
void writePROGMEMtoSPI(const char *ptrToPROGMEM, uint16_t count, char invert){
    PORTB &= ~(1 << PORTB4);
    if (invert) while(count--) writeSPIbyte(~pgm_read_byte(ptrToPROGMEM++));
    else        while(count--) writeSPIbyte( pgm_read_byte(ptrToPROGMEM++));
    PORTB |=  (1 << PORTB4);
}
void writeChar(char charCode){
    uint16_t bitmapIndex = (charCode - 32) * 8;
    writePROGMEMtoSPI(&asciiBitmaps[bitmapIndex], 8, 1); // Invert the text for aesthetics
}
void writeString(const char *ptrToProgmem){
    char nextByte;
    while ((nextByte = pgm_read_byte(ptrToProgmem++))) writeChar(nextByte);
}
void printfOLED(const char *ptrToSRAM){
    char nextByte;
    while ((nextByte = *ptrToSRAM++)) writeChar(nextByte);
}
void printfOLEDasBinary(const char singleByte){
    writeChar('0' + ((singleByte & (1 << 7)) >> 7) );
    writeChar('0' + ((singleByte & (1 << 6)) >> 6) );
    writeChar('0' + ((singleByte & (1 << 5)) >> 5) );
    writeChar('0' + ((singleByte & (1 << 4)) >> 4) );
    writeChar('0' + ((singleByte & (1 << 3)) >> 3) );
    writeChar('0' + ((singleByte & (1 << 2)) >> 2) );
    writeChar('0' + ((singleByte & (1 << 1)) >> 1) );
    writeChar('0' + ((singleByte & (1 << 0)) >> 0) );
}
char nibbleToASCIIhex(char nibble){
    return (nibble <= 9) ? ('0' + nibble) : ('A' + nibble - 10);
}
void printfOLEDasHEX(const char singleByte){
    writeChar(nibbleToASCIIhex((singleByte & (0xf0)) >> 4));
    writeChar(nibbleToASCIIhex((singleByte & (0x0f)) >> 0));
}
//============================================

//============================================
// Timer / counter functions
//============================================
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
//============================================

//============================================
// Pin Chnage Interupt
//============================================
// ISR(PCINT3_vect){
//     printfOLED("PCINT3_vect :   ");
// }
//============================================

void Init_Hardware() {
        cli();
        //============================================
        // Setup OLED / SPI
        //============================================
        PORTB = DDRB = (1 << DDB5) | (1 << DDB7) | (1 << DDB4) | (1 << DDB1); // MOSI SCK SS D/C
        SPCR = (1 << SPE) | (1 << MSTR);
        SPSR = (1 << SPI2X);

        PORTB &= ~(1 << PORTB1); // OLED 'Commands'
        writePROGMEMtoSPI(&OLEDbootSequence[0], 10, 0);
        PORTB |= (1 << PORTB1); // OLED 'Commands'
        //============================================

        //============================================
        // Setup PCINT for SQW
        //============================================
        PCICR  = (1 << PCIE3);                                      // Enables PC interrupt #0
        PCMSK3 = (1 << PCINT31);                                    // Enable PCINT1 to fire PC INT #0
        DDRD    = PORTD = 0;                                        // PD7 INPUT (SQW)
        //============================================

        //============================================
        // Setup I2C at 400kHz
        //============================================
        DDRC    = 0;                                                // Clock | SDA can be in / out
        // PORTC  |= (1 << PORTC5) | (1 << PORTC4);                    // ATmega328p Clock / SDA must start High
        PORTC  |= (1 << PORTC0) | (1 << PORTC1);                    // ATmega1284p Clock / SDA must start High
        TWSR    = 0;   // Set both prescaler bits for 0 === /1
        TWBR    = 2;   // BitRate = 400kHz @ 8MHz /1 prescaler
        TWBR    = 12;   // BitRate = 400kHz @ 16MHz /1 prescaler
        //============================================

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
        // TCNT0    = 130;      // Set the counter to leave 125 clks which takes 1ms @ 8MHz
        TCNT0    = 0;      // Set the counter to leave 125 clks which takes 1ms @ 16MHz
        //============================================
        sei();
}
void wakeMPU6050(){
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start

    TWDR = SLVADD;
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWDR = 0x6b; // PWR_MGMT_1 reg
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWDR = 0x00; // Wake
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
}
void ReadBuf_I2C() {
    // printfOLED("ReadBuf_I2C()   ");
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start
    // printfOLED("I2C Start - Done");
    TWDR = SLVADD | 0;  // Address & Write
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX
    // printfOLED("I2C Address | W ");
    TWDR = 0x3b;  // starting with register 0x3B (ACCEL_XOUT_H)
    // TWDR = 0x00;  // starting with register 0x3B (ACCEL_XOUT_H)
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX
    // printfOLED("I2C 0x3b BYTE.  ");
    // TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
    // printfOLED("I2C STOP CONITIN");
    // wait(50);
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start
    // printfOLED("I2C Start - Done");
    TWDR = SLVADD | 1;  // Address & Read
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX
    // printfOLED("I2C Address | R ");
    // wait(100);
    // printfOLED("Reading Byte 1..");
    // RxBuffer[0] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[0] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[1] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[2] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[3] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[4] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[5] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[6] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[7] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[8] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[9] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[10] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[11] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[12] = TWDR;
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[13] = TWDR;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
    // printfOLED("I2C STOP CONITIN");
    wait(50); // <20ms and SL018 doesn't response to subsequent read / writes
}

int main() {
    Init_Hardware();
    wakeMPU6050();
    printfOLED("MPU 6050 : Test1");wait(100);printfOLED("MPU 6050 : Test2");wait(100);printfOLED("MPU 6050 : Test3");wait(100);printfOLED("MPU 6050 : Test4");wait(100);printfOLED("MPU 6050 : Test5");wait(100);printfOLED("MPU 6050 : Test6");wait(100);printfOLED("MPU 6050 : Test7");wait(100);printfOLED("MPU 6050 : Test8");
    while(1) {
        ReadBuf_I2C();
        printfOLED("ACCEL_XOUT: "); printfOLEDasHEX(RxBuffer[0]);    printfOLEDasHEX(RxBuffer[1]);
        printfOLED("ACCEL_YOUT: "); printfOLEDasHEX(RxBuffer[2]);    printfOLEDasHEX(RxBuffer[3]);
        printfOLED("ACCEL_ZOUT: "); printfOLEDasHEX(RxBuffer[4]);    printfOLEDasHEX(RxBuffer[5]);
        printfOLED("TEMP :      "); printfOLEDasHEX(RxBuffer[6]);    printfOLEDasHEX(RxBuffer[7]);
        printfOLED("GYRO_XOUT : "); printfOLEDasHEX(RxBuffer[8]);    printfOLEDasHEX(RxBuffer[9]);
        printfOLED("GYRO_YOUT : "); printfOLEDasHEX(RxBuffer[10]);   printfOLEDasHEX(RxBuffer[11]);
        printfOLED("GYRO_ZOUT : "); printfOLEDasHEX(RxBuffer[12]);   printfOLEDasHEX(RxBuffer[13]);
        printfOLED("Control "); printfOLEDasBinary(RxBuffer[7]);
        wait(100); // Ideally should be interupt driven from SQW PINC
    }
}