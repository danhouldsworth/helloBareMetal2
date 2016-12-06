
int main(void){

    char* PINB_ptr  = (char*)0x0023;
    char* DDRB_ptr  = (char*)0x0024;
    char* PORTB_ptr = (char*)0x0025;

    *DDRB_ptr = (1 << 4);           // PB4 as output (all others as input)

	for (;;){
        *PORTB_ptr = 0;             // No pull up. Tri Stated (ie we can control it's voltage externally with little current)
        if (*PINB_ptr & (1 << 1)) {
            *PORTB_ptr = (1 << 4);
        }
	}
}