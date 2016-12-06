/*
    5/29/02
    Copyright Spark Fun Electronics 2004

    Nathan Seidle
    spark@sparkfun.com

    I2C Interface with 24LC04B
    Started on 5-19
    12:22am 5-29 First Working I2C - DAH! 10 $#@%ing days
    12:51am 5-29 Cleaned up - 4MHz Operation

    7-20 Added page selection

    9-23 Started working with SOIC 24LC16B - 16k baby!
         Need to increase page_num to full byte.
         Bits 2 1 0 control up to 8 256byte pages.


    7-23-03 Touched up.

    8-7-03 Moved to 24LC32A - 4k bytes. But now we have to have three byte transmission
           for each read/write. The A0,A1,A2 must also be tied high/low.

    9-7-03 Added Ack_polling to both read and write routines. This fixed many small
           graphical errors on the display.

    11-23-04 Tweaked for use with the DS1307 RTC Module
             Can we use the internal pull-up resistors on the PIC for the required
             resistors on the I2C bus? Yep - neat!

*/


//=================
//Make sure the bits agree with the TRISB statements
#pragma bit scl_IIC @ PORTB.0
#pragma bit sda_IIC @ PORTB.1

#define  WRITE_sda() TRISB = TRISB & 0b.1111.1101 //SDA must be output when writing
#define  READ_sda() {TRISB = TRISB | 0b.0000.0010; OPTION.7 = 0;} //SDA must be input when reading - Enable pull-up resistor on SDA

#define DEVICE_ADDRESS  0xD0
//=================

void start(void);
void stop(void);
int  read_byte(void);
void send_byte(uns8);
int  read_eeprom(uns16);
void write_eeprom(uns16, uns8);

void ack_polling(void)
{
    while(sda_IIC != 0)
    {
        start();
        send_byte(DEVICE_ADDRESS);
    }
    stop();
}

void write_eeprom(uns16 address, uns8 thing)
{
    ack_polling();

    start();
    send_byte(DEVICE_ADDRESS);

    //send_byte(address.high8); //Uppder Address - Needed for >= 32k EEProms
    send_byte(address.low8);
    send_byte(thing);
    stop();
}

int read_eeprom(uns16 address)
{
    ack_polling();

    start();
    send_byte(DEVICE_ADDRESS);
    //send_byte(address.high8); //Uppder Address - Needed for >= 32k EEProms
    send_byte(address.low8);
    stop();

    start();
    send_byte(DEVICE_ADDRESS | 0b.0000.0001); //Read bit must be set
    address = read_byte();
    stop();

    return(address);
}

void start(void)
{
    WRITE_sda();
    sda_IIC = 0;
}

void stop(void)
{
    scl_IIC = 0;

    WRITE_sda();

    sda_IIC = 0;
    nop();
    nop();
    nop();
    nop();
    scl_IIC = 1;
    nop();
    nop();
    nop();
    sda_IIC = 1;
}

int read_byte(void)
{
    int j, in_byte;

    scl_IIC = 0;

    READ_sda();

    for(j = 0 ; j < 8 ; j++)
    {
        scl_IIC = 0;
        nop();
        nop();
        nop();
        nop();
        scl_IIC = 1;

        in_byte = rl(in_byte);
        in_byte.0 = sda_IIC;
    }

    return(in_byte);
}

void send_byte(uns8 nate)
{
    int i;

    WRITE_sda();

    for( i = 0 ; i < 8 ; i++ )
    {
        nate = rl(nate);
        scl_IIC = 0;
        sda_IIC = Carry;
        scl_IIC = 1;
        nop();
    }

    //read ack.
    scl_IIC = 0;
    READ_sda();
    scl_IIC = 1;
}