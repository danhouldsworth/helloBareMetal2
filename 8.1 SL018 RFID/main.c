/*
            328p        1284p
    SDA     PC4         PC1
    SCL     PC5         PC0
    Board   SL018       DC1338
    Trigger PC3         -
    SQW     -           PD7
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#define SLVADD      0b10100000 // SL018 does not respond to general call, must address directly

//============================================
//  global variable define
//============================================
unsigned long int   g_iTimer    = 0;
uint8_t             g_bOverTime = 0;
uint8_t             RxBuffer[60];

//============================================
//  Command List, preamble + length + command
//============================================
uint8_t     ComSelectCard[] = {1, 0x01};
uint8_t     RED_ON[]        = {2, 0x40, 1};
uint8_t     RED_OFF[]       = {2, 0x40, 0};

void wait(unsigned long int ms) {
    g_iTimer        = ms;
    g_bOverTime     = 0;
    while(g_bOverTime == 0){}
    g_iTimer = 0;
}
ISR(TIMER0_OVF_vect) {
    TCNT0 = 125;
    if (g_iTimer != 0) {
        g_iTimer--;
        if (g_iTimer == 0) {g_bOverTime = 1;}
    }
}

void error(){
    for (uint8_t i = 0; i < 20; i++){PORTB = 1; wait(25);PORTB = 0; wait(25);}
}

void Init_Hardware() {
        cli();
        DDRB    = PORTB = 1;                                        // Debug GREEN LED
        DDRC    = 0;                                                // Clock | SDA can be in / out
        PORTC  |= (1 << PORTC5) | (1 << PORTC4);                    // Clock / SDA must start High
        TWSR    = 0;   // Set both prescaler bits for 0 === /1
        TWBR    = 2;   // BitRate = 400kHz @ 8MHz /1 prescaler

        // COM0A    = 0 (off)
        // COM0B    = 0 (off)
        // WGM      = 0 (Normal)
        // CS       = 3 (Prescaler = /64)
        TCCR0A  = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
        TCCR0B  = (0 << WGM02)  | (0 << CS02)   | (1 << CS01)   | (1 << CS00);
        TIMSK0  = (0 << OCIE0B) | (0 << OCIE0A) | (1 << TOIE0);
        TCNT0    = 130;      // Set the counter to leave 125 clks which takes 1ms @ 8MHz
        sei();
}

void SendBuf_I2C(uint8_t *data) {
    uint8_t TxLength = data[0] + 1;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start

    TWDR = SLVADD | 0;  // Address & Write
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    for(uint8_t TxIndex = 0; TxIndex < TxLength; TxIndex++){
        TWDR = data[TxIndex];
        TWCR = (1 << TWINT) | (1 << TWEN);              while((TWCR & (1 << TWINT)) == 0){} // I2C TX
    }

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
    wait(50); // <20ms and SL018 doesn't response to subsequent read / writes
}

void ReadBuf_I2C() {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){} // I2C Start

    TWDR = SLVADD | 1;  // Address & Read
    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){} // I2C TX

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}; // ACK (won't send until byte in TWDR)
    RxBuffer[0] = TWDR;  // 1st Rx Byte is length of transmission

    for(uint8_t RxIndex = 1; RxIndex <= RxBuffer[0]; RxIndex++) {
        if (RxIndex < RxBuffer[0])  {TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);   while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[RxIndex] = TWDR;}
        else                        {TWCR = (1 << TWINT) | (1 << TWEN);                 while((TWCR & (1 << TWINT)) == 0){}; RxBuffer[RxIndex] = TWDR;}
    }

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);                                       // I2C STOP
    wait(50); // <20ms and SL018 doesn't response to subsequent read / writes
}

void chkResponse(char RxCMD, char RxSTATUS){
    ReadBuf_I2C();
    if (RxBuffer[1] != RxCMD || RxBuffer[2] != RxSTATUS) error();
}

void confirmPulse(){
    PORTB = 1;
    SendBuf_I2C(RED_ON);
    chkResponse(0x40, 0);
    wait(500);
    PORTB = 0;
    SendBuf_I2C(RED_OFF);
    chkResponse(0x40, 0);
    wait(500);
}

int main() {
    Init_Hardware();
    confirmPulse();
    while(1) {
        if ( (PINC & (1 << PINC3)) == 0) {
            SendBuf_I2C(ComSelectCard);
            chkResponse(0x01, 0x00);
            if (RxBuffer[0] != 7)           error();                                                                // Alert if not 7 byte response as expected for 4byte UID
            if (RxBuffer[RxBuffer[0]] != 1) error();                                                                // Alert if not Mifare 1k 4byte UID
            if (RxBuffer[3]==0x7F && RxBuffer[4]==0x76 && RxBuffer[5]==0x62 && RxBuffer[6]==0x8b) confirmPulse();   // Confirm if UID matches my Credit Card
        }
    }
}