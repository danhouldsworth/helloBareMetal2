/*
	Desk Copter

	MCU = AIOP FC
	Programmer via USB bootloader (disabling erase cycle)
	Pinouts :
	TIMER 	|	PORTB 	| 	Connection
	==================================
	OC3B 	|	 PE4 	|	 Pin2
	OC3C 	|	 PE5 	|	 Pin3
	OC1A 	|	 PB5 	|	 Pin11
	OC1B 	|	 PB6 	|	 Pin12

	8-bit Timer0 overflow interupt to generate 1ms tick
	16-bit Timer3 to generate PWM outputs for each motor at 400Hz
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>

#define SLVADD      0xd0 //
#define I2C_START   TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   while((TWCR & (1 << TWINT)) == 0){}
#define I2C_TX      TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){}
#define I2C_STOP    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
#define I2C_ACK     TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);    while((TWCR & (1 << TWINT)) == 0){}
#define I2C_NACK    TWCR = (1 << TWINT) | (1 << TWEN);                  while((TWCR & (1 << TWINT)) == 0){}
#define PI 			3.14159265
#define PIby2 		PI / 2.0
uint8_t   	RxBuffer[16];
uint16_t 	wait_ms = 0;
uint8_t 	waitFinished = 0;
uint16_t 	runtime_ms = 0;
int16_t 	thrust = 0;

void wait(uint16_t ms) {
    wait_ms       = ms;
    waitFinished = 0;
    while(waitFinished == 0){}
}
ISR(TIMER0_OVF_vect){
    if (wait_ms > 0) {
    	wait_ms--;
    	if (wait_ms == 0) waitFinished = 1;
    }
    runtime_ms++;
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
int16_t getMPUvalue(uint8_t highByte, uint8_t lowByte){
    int16_t word = ((highByte << 8) | lowByte);
    return (double)word;
}

// int16_t getMPUvalue(uint8_t highByte, uint8_t lowByte, char scaleFactor){
//     int16_t word = ((highByte << 8) | lowByte) / scaleFactor;
//     if (word >  500) word =  500;
//     if (word < -500) word = -500;
//     return word;
// }

void RED_ON() 		{PORTB 	|= 1 << 7;}
void YELLOW_ON() 	{PORTC 	|= 1 << 6;}
void GREEN_ON() 	{PORTC 	|= 1 << 7;}
void RED_OFF() 		{PORTB 	&= ~(1 << 7);}
void YELLOW_OFF() 	{PORTC 	&= ~(1 << 6);}
void GREEN_OFF() 	{PORTC 	&= ~(1 << 7);}
void motor1(uint16_t duty_us){OCR3B = 2 * duty_us;}
void motor2(uint16_t duty_us){OCR3C = 2 * duty_us;}

void MPUconfigure(){
    MPUwriteReg(0x6b, 0b10000000);  // RESET
    wait(50);
    MPUwriteReg(0x6b, 0b00000011);  // CLKSEL = 3 : PLL with Z Gyro Ref, more stable than internal clock
    wait(50);
    MPUwriteReg(0x1a, 0b00000100);  // Max DLPF ~5Hz 0 => 256Hz, 1 => 188Hz, 2 => 98Hz, 3 => 42Hz, 4 => 20Hz, 5 => 10Hz, 6 => 5Hz
    MPUwriteReg(0x1b, 0b00011000);  // 2000 deg/s
    MPUwriteReg(0x1c, 0b00011000);  // 16g ==> 1G = 2048 (0x800)
}


int main(void){
	// Enable OC3B & OC3C on PortE as output
	DDRE	|= (1 << PORTE4) | (1 << PORTE5);
	PORTE 	|= (1 << PORTE4) | (1 << PORTE5);
	// Enable STATUS LEDs
    DDRB 	|= 1 << 7;
    DDRC 	|= (1 << 6)|(1 << 7);

    //============================================
    // Setup I2C at 400kHz
    //============================================
    PORTD  |= (1 << PORTD0) | (1 << PORTD1);                    // Clock / SDA must start High
    TWSR    = 0;   // Set both prescaler bits for 0 === /1
    TWBR    = 12;   // BitRate = 400kHz @ 16MHz /1 prescaler
    //============================================

	// COM0A 	= 0 (off)
	// COM0B 	= 0 (off)
	// WGM 		= 0 (Normal)
	// CS 		= 3 (Prescaler = /64)
	TCCR0A 	= (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
	TCCR0B 	= (0 << WGM02) 	| (0 << CS02) 	| (1 << CS01) 	| (1 << CS00);
	TIMSK0 	= (0 << OCIE0B) | (0 << OCIE0A) | (1 << TOIE0);
	TCNT0 	= 0; 		// 16MHz

	// COM3A 	= 0 (off)
	// COM3B/C 	= 2 (Clear on Compare Match "Non-inverted PWM")
	// WGM 		= 15 (FastPWM)
	// CS 		= 2 (Prescaler = /8)
	TCCR3A 	= (0 << COM3A1) | (0 << COM3A0) | (1 << COM3B1) | (0 << COM3B0) | (1 << COM3C1) | (0 << COM3C0) | (1 << WGM31) | (1 << WGM30);
	TCCR3B 	= (1 << WGM33) 	| (1 << WGM32) 	| (0 << CS32) 	| (1 << CS31) 	| (0 << CS30);
	TIMSK3 	= (0 << OCIE3B) | (0 << OCIE3A) | (0 << TOIE3);
	TCNT3 	= 0; 		// Reset the counter
	OCR3A 	= 2*2500; 	// Set TOP for 400Hz @ 16MHz /8 prescaler (2 ticks per microsecond)
	RED_OFF();YELLOW_OFF();GREEN_OFF();
	sei();

	motor1(900);
	motor2(900);
	wait(1000);

	MPUconfigure();

	double kP 			= 6000.0 / PIby2;
	// double kP 			= 0;
	double kD 			= 130.0 / PIby2;
	double hover 		= 1500.0;
	double maxThrust    =  100.0;
	// double setPoint  	= PI / 3.0;
	double setPoint  	= 0;
	while(1){

        MPUburstReadRAW(); // Although no waiting - this is a crude blocking I2C protocol

        double ACC_Y = getMPUvalue(RxBuffer[2], RxBuffer[3]);
        double ACC_Z = getMPUvalue(RxBuffer[4], RxBuffer[5]);
        double theta = atan(-ACC_Z / ACC_Y); // -PI/2 --> PI/2

        double theta_dot = (getMPUvalue(RxBuffer[8], RxBuffer[9]) * 2000.0 / 32768.0) * (2 * PI / 360.0); // Radians per second

        double thrust = (theta - setPoint) * kP + theta_dot * kD;

        if (thrust >  maxThrust) thrust =  maxThrust;
        if (thrust < -maxThrust) thrust = -maxThrust;

		motor1((uint16_t)(hover + thrust));
		motor2((uint16_t)(hover - thrust));

		// if (runtime_ms > 2000) {
		// 	if (setPoint > 0.0) setPoint = -PI / 3.0;
		// 	else  				setPoint =  PI / 3.0;
		// 	runtime_ms = 0;
		// }
        // wait(1);
	}
}
