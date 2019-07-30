/*
NanoWii
GREEN PortD5 ON@LOW
*/


#include <avr/interrupt.h>
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


void Init_Hardware() {
        cli();
        UDIEN = 0;
        // DDRB = 1 << 7;
        //============================================
        // Setup I2C at 400kHz
        //============================================
        DDRD    = 1 << 5;                                           // Clock | SDA can be in / out
        PORTD  |= (1 << PORTD0) | (1 << PORTD1);                    // ATmega32u4 Clock / SDA must start High
        TWSR    = 0;   // Set both prescaler bits for 0 === /1
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
void displaySignedWordAsBlink(uint8_t highByte, uint8_t lowByte, char scaleFactor){
    uint16_t word = (highByte << 8) | lowByte;
    if (word > 0x8000) {
        word = (0xffff - word) >> scaleFactor; // -1G --> 64
    } else {
        word = word >> scaleFactor; // 1G --> 64
    }
    PORTD ^=  (1 << 5);
    // PORTB ^=  (1 << 7);
    if (word>1000) word = 1000;
    if (word<5) word = 5;
    wait(word);
}


int main() {
    UDIEN = 0;
    Init_Hardware();
    MPUconfigure();
    while(1) {
        MPUburstReadRAW();
        displaySignedWordAsBlink(RxBuffer[0], RxBuffer[1],5);
        wait(10);
    }
}