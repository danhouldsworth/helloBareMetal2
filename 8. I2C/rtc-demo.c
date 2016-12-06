/*
    11-23-04
    Copyright Spark Fun Electronics 2004

    Nathan Seidle
    spark@sparkfun.com

    This is the test bed firmware that we use to program the RTC Module using a PIC 16F88 at 20MHz.
    It should give you some insight on how to deal with binary coded decimal as well as some software driven
    I2C routines.

    Something interesting to note is that there is no external pull-up resistor on the SDA line (normally required
    for I2C communication). Instead, we turn on the internal pull-up resistors on the 16F88 PORTB pins every
    time we do an I2C read. We don't use any hardware module on the 16F88, I just bit-bang I2C on any-ol port pins.

    This code is fairly bulky because of all the pretty-print menus. There is no checking during the programming routine.
    It is up to the user to select valid date ranges/month/times before hitting 'p' to program the RTC Module.

    We used Bloader/Screamer exclusively to design this code. That's why you won't see any config commands/bits.

*/
#define Clock_20MHz
#define Baud_9600

#include "\Pics\c\16F88.h"  //Device dependent definitions

#pragma origin 4

#include "\Pics\code\stdio.c"   //Software based Basic Serial IO
#include "\Pics\code\16F88\RTC-Demo\I2c.c"   //Software based I2C routines

void boot_up(void);
void rtc_programming(void);
void read_rtc(void);

void main(void)
{
    uns8 choice;

    boot_up();

    while(1)
    {
        printf("\n\r\n\r========RTC DEMO========\n\r", 0);
        printf("Main Menu:\n\r", 0);
        printf(" 1) Program RTC Module\n\r", 0);
        printf(" 2) Check RTC Time\n\r", 0);
        printf("\n\r : ", 0);

        choice = getc();

        if (choice == '1')
        {
            rtc_programming();
        }
        else if (choice == '2')
        {
            read_rtc();
        }
        else
        {
            printf("choice = %d", choice);
        }
    }

    while(1);
}//End Main

//Read current RTC - Converts BCD bytes to printable numerals
void read_rtc(void)
{
    //SCL is connected to RB0
    //SDA is connected to RB1

    //All numbers are in BCD form

    uns8 x, temp;

    printf("\n\n\rCurrent Time and Date:\n\n\r  ", 0);

    //=======================
    x = read_eeprom(2); //Read hours register
    temp = x & 0b.0001.0000; //Avoid the hour settings
    temp >>= 4;
    temp += '0';
    putc(temp);

    temp = x & 0b.0000.1111; //Get hours - low number
    temp += '0';
    putc(temp);
    //=======================

    putc(':');

    //=======================
    x = read_eeprom(1); //Minutes
    temp = x & 0b.1111.0000;
    temp >>= 4;
    temp += '0';
    putc(temp);

    temp = x & 0b.0000.1111;
    temp += '0';
    putc(temp);
    //=======================

    putc(':');

    //=======================
    x = read_eeprom(0); //Seconds
    temp = x & 0b.0111.0000; //Avoid the CH bit
    temp >>= 4;
    temp += '0';
    putc(temp);

    temp = x & 0b.0000.1111;
    temp += '0';
    putc(temp);
    //=======================

    putc(' ');

    x = read_eeprom(2); //Read hours register for AM/PM
    if(x.5 == 1) printf("PM" , 0);
    else printf("AM", 0);

    //=======================

    printf(" - ", 0);

    //=======================
    x = read_eeprom(3); //Read day
    if(x == 1) printf("Sunday", 0);
    if(x == 2) printf("Monday", 0);
    if(x == 3) printf("Tuesday", 0);
    if(x == 4) printf("Wednesday", 0);
    if(x == 5) printf("Thursday", 0);
    if(x == 6) printf("Friday", 0);
    if(x == 7) printf("Saturday", 0);

    putc(' ');
    //=======================
    x = read_eeprom(5); //Read month

    temp = x & 0b.1111.0000; //Decode to month number
    x = x & 0b.0000.1111;
    temp >>= 4;
    temp *= 10;
    x = x + temp; //We now have a month number in x
    if(x == 1) printf("January", 0);
    if(x == 2) printf("February", 0);
    if(x == 3) printf("March", 0);
    if(x == 4) printf("April", 0);
    if(x == 5) printf("May", 0);
    if(x == 6) printf("June", 0);
    if(x == 7) printf("July", 0);
    if(x == 8) printf("August", 0);
    if(x == 9) printf("September", 0);
    if(x == 10) printf("October", 0);
    if(x == 11) printf("November", 0);
    if(x == 12) printf("December", 0);

    putc(' ');
    //=======================

    //=======================
    x = read_eeprom(4); //Read date

    temp = x & 0b.1111.0000; //Decode date to a number
    temp >>= 4;
    temp += '0';
    putc(temp);

    temp = x & 0b.0000.1111;
    temp += '0';
    putc(temp);

    putc(',');
    putc(' ');
    //=======================

    //=======================
    x = read_eeprom(6); //Read year

    printf("20", 0);

    temp = x & 0b.1111.0000; //Decode year to a number
    temp >>= 4;
    temp += '0';
    putc(temp);

    temp = x & 0b.0000.1111;
    temp += '0';
    putc(temp);
    //=======================

    x = read_eeprom(7);
    printf("\n\r  SQW Settings : %h ", x);
}

//Allow user to input the current Calendar data and time into the DS1307 RTC
void rtc_programming(void)
{
    //SCL is connected to RB0
    //SDA is connected to RB1

    uns8 temp, cell = 1, data_byte_in;

    uns8 hours = 1, minutes = 1, seconds = 1;
    bit am_pm;
    #define PM 1
    #define AM 0

    //Time configuration
    //=========================================================
    printf("\n\r a/s change cell | +/- increase/decrease | p to program\n\r", 0);

    //Display delay and adjust as we go...
    while(1)
    {
        printf("%d:", hours);
        printf("%d:", minutes);
        printf("%d ", seconds);

        if (am_pm == PM) printf("PM", 0);
        if (am_pm == AM) printf("AM", 0);

        data_byte_in = getc();
        if(data_byte_in == 'a' && cell > 1) cell--;
        if(data_byte_in == 's' && cell < 4) cell++;

        if(data_byte_in == '+')
        {
            if(cell == 1) hours++;
            if(cell == 2) minutes++;
            if(cell == 3) seconds++;
            if(cell == 4) am_pm ^= 1;
        }
        if(data_byte_in == '-')
        {
            if(cell == 1) hours--;
            if(cell == 2) minutes--;
            if(cell == 3) seconds--;
            if(cell == 4) am_pm ^= 1;
        }

        if(data_byte_in == 'p') break;

        putc('\r');

    }

    temp = seconds / 10; //Convert seconds into BCD
    temp <<= 4;
    printf(" --%h-- ", seconds);
    seconds %= 10;
    printf(" --%h-- ", seconds);
    seconds = temp | seconds;
    seconds &= 0b.0111.1111; //CH Bit is bit 7 - 0 to enable clock.
    write_eeprom(0, seconds); //Load seconds register

    temp = minutes / 10; //Convert minutes into BCD
    temp <<= 4;
    minutes %= 10;
    minutes = temp | minutes;
    write_eeprom(1, minutes); //Load minutes register

    /*
    Hour settings :

    Bit # - Programmed Setting : Description
    7 - 0 :
    6 - 1 : 12Hr or 24Hr Mode - High for AM/PM 12Hr. Mode
    5 - 1 : AM/PM bit - Set to PM
    4 - 0 :

    3 - 1 :
    2 - 0 :
    1 - 0 :
    0 - 0 : Set to 8 hours
    */
    temp = hours / 10; //Convert hours into BCD
    temp <<= 4;
    hours %= 10;
    hours = temp | hours;
    hours |= 0b.0100.0000; //Force 12hr mode
    if(am_pm == PM) hours |= 0b.0010.0000; //Force AM/PM bit
    if(am_pm == AM) hours &= 0b.1101.1111; //Force AM/PM bit
    write_eeprom(2, hours); //Write hour configuration
    //=========================================================

    //Date configuration
    //=========================================================
    cell = 1;
    uns8 day = 1, month = 1, date = 1, year = 4;

    printf("\n\r a/s change cell | +/- increase/decrease | p to program\n\r", 0);

    while(1)
    {
        if(day == 1) printf("Sunday", 0);
        if(day == 2) printf("Monday", 0);
        if(day == 3) printf("Tuesday", 0);
        if(day == 4) printf("Wednesday", 0);
        if(day == 5) printf("Thursday", 0);
        if(day == 6) printf("Friday", 0);
        if(day == 7) printf("Saturday", 0);

        putc(' ');

        if(month == 1) printf("January", 0);
        if(month == 2) printf("February", 0);
        if(month == 3) printf("March", 0);
        if(month == 4) printf("April", 0);
        if(month == 5) printf("May", 0);
        if(month == 6) printf("June", 0);
        if(month == 7) printf("July", 0);
        if(month == 8) printf("August", 0);
        if(month == 9) printf("September", 0);
        if(month == 10) printf("October", 0);
        if(month == 11) printf("November", 0);
        if(month == 12) printf("December", 0);

        printf(" %d,", date);

        printf(" 200%d    ", year);

        data_byte_in = getc();
        if(data_byte_in == 'a' && cell > 1) cell--;
        if(data_byte_in == 's' && cell < 4) cell++;

        if(data_byte_in == '+')
        {
            if(cell == 1) day++;
            if(cell == 2) month++;
            if(cell == 3) date++;
            if(cell == 4) year++;
        }
        if(data_byte_in == '-')
        {
            if(cell == 1) day--;
            if(cell == 2) month--;
            if(cell == 3) date--;
            if(cell == 4) year--;
        }

        if(data_byte_in == 'p') break;

        putc('\r');

    }

    write_eeprom(3, day); //Load day register

    temp = month / 10; //Convert month into BCD
    temp <<= 4;
    month %= 10;
    month = temp | month;
    printf(" --%h-- ", month);
    write_eeprom(5, month); //Load month register

    temp = date / 10; //Convert date into BCD
    temp <<= 4;
    date %= 10;
    date = temp | date;
    write_eeprom(4, date); //Load date register

    temp = year / 10; //Convert year into BCD
    temp <<= 4;
    year %= 10;
    year = temp | year;
    write_eeprom(6, year); //Load year register

    read_rtc(); //Print it out for double checking

}

void boot_up(void)
{
    //Setup Ports
    ANSEL = 0b.0000.0000; //Turn off A/D

    PORTA = 0b.0000.0000;
    TRISA = 0b.1111.1111;

    PORTB = 0b.0000.0000;
    TRISB = 0b.0000.0100;   //0 = Output, 1 = Input RX on RB2

    //Setup the hardware UART module
    //=============================================================
    //SPBRG = 51; //8MHz for 9600 inital communication baud rate
    SPBRG = 129; //20MHz for 9600 inital communication baud rate

    TXSTA = 0b.0010.0100; //8-bit asych mode, high speed uart enabled
    RCSTA = 0b.1001.0000; //Serial port enable, 8-bit asych continous receive mode
    //=============================================================

}
