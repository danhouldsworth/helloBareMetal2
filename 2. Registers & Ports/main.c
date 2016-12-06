/*
 						328@8MHz	 328@1MHz 	   Nano	   32u4
	BLUE 	: B4 = HIGH		* 			*			*		*
	RED 	: B5 = HIGH		*			*			*		GRN
	WHITE 	: B7 = HIGH 	*			*					*
	YELLOW 	: C7 = HIGH										*
	GREEN 	: D5 = LOW		*			*			*		*
*/

int main(void){
    // Note : int, uint and char* are 16-bit whereas char is 8-bit
    // The general purpose registers R0-R31 come first in Data memory, next the IO regs from 0x0020
    // Port registers are the same for 328 / 328p / 32u4 chips

    char* DDRB_ptr  = (char*)0x0024;
    char* PORTB_ptr = (char*)0x0025;
    char* DDRC_ptr  = (char*)0x0027;
    char* PORTC_ptr = (char*)0x0028;
    char* DDRD_ptr  = (char*)0x002A;
    char* PORTD_ptr = (char*)0x002B;

    *DDRB_ptr  |=  (1 << 4) | (1 << 5) | (1 << 7);
    *PORTB_ptr |=  (1 << 4) | (1 << 5) | (1 << 7);

    *DDRC_ptr  |=  (1 << 7);
    *PORTC_ptr |=  (1 << 7);

    *DDRD_ptr  |=  (1 << 5);
    *PORTD_ptr |=  (1 << 5);
    *PORTD_ptr &= ~(1 << 5);

	for (;;){
	}
}