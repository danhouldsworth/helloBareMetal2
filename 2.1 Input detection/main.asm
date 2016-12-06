.equ 	PINB 	= 0x03
.equ 	DDRB 	= 0x04
.equ 	PORTB 	= 0x05

.DEVICE ATmega328p
        cbi     DDRB, 	1
        sbi     DDRB, 	4

loop:
        cbi     PORTB, 	4
        sbic 	PINB, 	1
        sbi 	PORTB, 	4
        rjmp 	loop
