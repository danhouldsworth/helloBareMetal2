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
#define I2C_START   TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){}
#define I2C_TX      TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){}
#define I2C_STOP    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
#define I2C_ACK     TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){};
#define I2C_NACK    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){};

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
const char Graphics[]           PROGMEM = {0x00, 0xff};
const char OLEDbootSequence[]   PROGMEM = {0xAe, 0xD5, 0x80, 0x8D, 0x14, 0x20, 0x00, 0x81, 0xFF, 0xAF};
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
char nibbleToASCIIhex(char nibble){return (nibble <= 9) ? ('0' + nibble) : ('A' + nibble - 10);}
void printfOLEDasHEX(const char singleByte){
    writeChar(nibbleToASCIIhex((singleByte & (0xf0)) >> 4));
    writeChar(nibbleToASCIIhex((singleByte & (0x0f)) >> 0));
}
void displaySignedWordAsBar(uint8_t highByte, uint8_t lowByte, char scaleFactor){
    uint16_t word = (highByte << 8) | lowByte;
    if (word > 0x8000) {
        word = (0xffff - word) >> scaleFactor; // -1G --> 64
        for (uint16_t x = 64; x > 0; x--) {
            writePROGMEMtoSPI(&Graphics[0],1,(x < word));
        }
        for (uint16_t x = 0; x < 64; x++) writePROGMEMtoSPI(&Graphics[0],1,0); // blank
    } else {
        word = word >> scaleFactor; // 1G --> 64
        for (uint16_t x = 0; x < 64; x++) writePROGMEMtoSPI(&Graphics[0],1,0); // blank
        for (uint16_t x = 0; x < 64; x++) {
            writePROGMEMtoSPI(&Graphics[0],1,(x < word));
        }
    }
}
void printfAsSignedWORD(const char highByte, const char lowByte){ // Allow 5 spaces
    uint16_t word = (highByte << 8) | lowByte;
    if (word > 0x8000) {
        word = 0xffff - word; // Forget the minus sign for now
        writeChar('-');
    } else {
        writeChar(' ');
    }
    printfOLEDasHEX((uint8_t)(word >> 8));
    printfOLEDasHEX((uint8_t)(word & 0xff));
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
        // TWBR    = 2;   // BitRate = 400kHz @ 8MHz /1 prescaler
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
void I2C_sendByte(uint8_t singleByte){
    TWDR = singleByte;
    I2C_TX;
}
uint8_t I2C_readByte(uint8_t ack){
    if (ack)    {I2C_ACK;}
    else        {I2C_NACK;}
    return TWDR;
}
uint8_t MPUreadReg(uint8_t addr){
    I2C_START;
    I2C_sendByte(SLVADD | 0);       // Address / W
    I2C_sendByte(addr);
    I2C_START;
    I2C_sendByte(SLVADD | 1);       // Address / R
    uint8_t value = I2C_readByte(0); // NACK
    I2C_STOP;
    wait(50);
    return value;
}
void MPUwriteReg(uint8_t addr, uint8_t value){
    I2C_START;
    I2C_sendByte(SLVADD | 0);
    I2C_sendByte(addr);
    I2C_sendByte(value);
    I2C_STOP;
    wait(50);
}
void MPUconfigure(){
    MPUwriteReg(0x6b, 0b10000000);  // RESET
    wait(50);
    MPUwriteReg(0x6b, 0b00000011);  // CLKSEL = 3 : PLL with Z Gyro Ref, more stable than internal clock
    wait(50);
    MPUwriteReg(0x1a, 0b00000110);  // Max DLPF ~5Hz 0 => 256Hz, 1 => 188Hz, 2 => 98Hz, 3 => 42Hz, 4 => 20Hz, 5 => 10Hz, 6 => 5Hz
    MPUwriteReg(0x1b, 0b00011000);  // 2000 deg/s
    MPUwriteReg(0x1c, 0b00011000);  // 16g ==> 1G = 2048 (0x800)
}
void MPUburstReadRAW() {
    I2C_START;
    I2C_sendByte(SLVADD | 0);       // Address / W
    I2C_sendByte(0x3b);             // starting with register 0x3B (ACCEL_XOUT_H)
    I2C_START;
    I2C_sendByte(SLVADD | 1);       // Address / R
    RxBuffer[0]     = I2C_readByte(1);
    RxBuffer[1]     = I2C_readByte(1);
    RxBuffer[2]     = I2C_readByte(1);
    RxBuffer[3]     = I2C_readByte(1);
    RxBuffer[4]     = I2C_readByte(1);
    RxBuffer[5]     = I2C_readByte(1);
    RxBuffer[6]     = I2C_readByte(1);
    RxBuffer[7]     = I2C_readByte(1);
    RxBuffer[8]     = I2C_readByte(1);
    RxBuffer[9]     = I2C_readByte(1);
    RxBuffer[10]    = I2C_readByte(1);
    RxBuffer[11]    = I2C_readByte(1);
    RxBuffer[12]    = I2C_readByte(1);
    RxBuffer[13]    = I2C_readByte(0); // NACK
    I2C_STOP;
    wait(5); // <20ms and SL018 doesn't response to subsequent read / writes
}
void readDisplayConfig(){
    // printfOLED("WHO_AM_I :              "); printfOLEDasBinary(MPUreadReg(0x75));
    printfOLED("PWR_MGMT_1:             "); printfOLEDasBinary(MPUreadReg(0x6b));
    printfOLED("                ");
    printfOLED("GYRO_CONFIG :           "); printfOLEDasBinary(MPUreadReg(0x1b));
    printfOLED("                ");
    printfOLED("ACCEL_CONFIG :          "); printfOLEDasBinary(MPUreadReg(0x1c));
}
void displayAccsOnly(){
    printfOLED("MSB.....LSB.....");
    printfOLEDasBinary(RxBuffer[ 0]); printfOLEDasBinary(RxBuffer[ 1]);
    printfOLEDasBinary(RxBuffer[ 2]); printfOLEDasBinary(RxBuffer[ 3]);
    printfOLEDasBinary(RxBuffer[ 4]); printfOLEDasBinary(RxBuffer[ 5]);
    printfOLED("16bit val >> 5  ");
    displaySignedWordAsBar(RxBuffer[ 0], RxBuffer[ 1], 5);
    displaySignedWordAsBar(RxBuffer[ 2], RxBuffer[ 3], 5);
    displaySignedWordAsBar(RxBuffer[ 4], RxBuffer[ 5], 5);
}

void displayAsBinary(){
    printfOLED("Gyro X    Y    Z");
    printfOLEDasBinary(RxBuffer[ 8]); printfOLEDasBinary(RxBuffer[ 9]);
    printfOLEDasBinary(RxBuffer[10]); printfOLEDasBinary(RxBuffer[11]);
    printfOLEDasBinary(RxBuffer[12]); printfOLEDasBinary(RxBuffer[13]);
    printfOLED("Accels :        ");
    printfOLEDasBinary(RxBuffer[ 0]); printfOLEDasBinary(RxBuffer[ 1]);
    printfOLEDasBinary(RxBuffer[ 2]); printfOLEDasBinary(RxBuffer[ 3]);
    printfOLEDasBinary(RxBuffer[ 4]); printfOLEDasBinary(RxBuffer[ 5]);
}
void displayAsBars(){
    printfOLED("Gyro X    Y    Z");
    displaySignedWordAsBar(RxBuffer[ 8], RxBuffer[ 9], 5);
    displaySignedWordAsBar(RxBuffer[10], RxBuffer[11], 5);
    displaySignedWordAsBar(RxBuffer[12], RxBuffer[13], 5);
    printfOLED("Accels :        ");
    displaySignedWordAsBar(RxBuffer[ 0], RxBuffer[ 1], 5);
    displaySignedWordAsBar(RxBuffer[ 2], RxBuffer[ 3], 5);
    displaySignedWordAsBar(RxBuffer[ 4], RxBuffer[ 5], 5);
}
void displayFilteredResults(){
    printfOLED("     X    Y    Z");
    printfOLED("Gyro :          ");
    // printfOLED("  "); printfOLEDasHEX(RxBuffer[8+0]);    printfOLEDasHEX(RxBuffer[8+1]); printfOLED(" ");printfOLEDasHEX(RxBuffer[8+2]);    printfOLEDasHEX(RxBuffer[8+3]);printfOLED(" ");printfOLEDasHEX(RxBuffer[8+4]);    printfOLEDasHEX(RxBuffer[8+5]);
    printfOLED(" "); printfAsSignedWORD(RxBuffer[8], RxBuffer[9]); printfAsSignedWORD(RxBuffer[10], RxBuffer[11]); printfAsSignedWORD(RxBuffer[12], RxBuffer[13]);
    printfOLED("                ");
    printfOLED("Accels :        ");
    // printfOLED("  "); printfOLEDasHEX(RxBuffer[0]);    printfOLEDasHEX(RxBuffer[1]); printfOLED(" ");printfOLEDasHEX(RxBuffer[2]);    printfOLEDasHEX(RxBuffer[3]);printfOLED(" ");printfOLEDasHEX(RxBuffer[4]);    printfOLEDasHEX(RxBuffer[5]);
    printfOLED(" "); printfAsSignedWORD(RxBuffer[0], RxBuffer[1]); printfAsSignedWORD(RxBuffer[2], RxBuffer[3]); printfAsSignedWORD(RxBuffer[4], RxBuffer[5]);
    printfOLED("                ");
    int16_t temp = (RxBuffer[6] << 8) | RxBuffer[7];
    float temp2 = 36.53 + (float)(temp/340);
    printfOLED("Temp : x");printfOLEDasHEX((uint8_t)temp2);printfOLED(" degsC");
}
int main() {
    Init_Hardware();
    MPUconfigure();
    printfOLED("MPU 6050 : Test1");wait(25);printfOLED("MPU 6050 : Test2");wait(25);printfOLED("MPU 6050 : Test3");wait(25);printfOLED("MPU 6050 : Test4");wait(25);printfOLED("MPU 6050 : Test5");wait(25);printfOLED("MPU 6050 : Test6");wait(25);printfOLED("MPU 6050 : Test7");wait(25);printfOLED("MPU 6050 : Test8");
    while(1) {
        MPUburstReadRAW();
        // displayFilteredResults();
        displayAsBars();
        // displayAsBinary();
        // displayAccsOnly();
        // readDisplayConfig();
        // wait(25); // Ideally should be interupt driven from SQW PINC
    }
}