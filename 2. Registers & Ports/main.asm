.DEVICE ATmega128

.def 	FULL_OFF 	= r16
.def 	FULL_ON  	= r17
.def 	RED_BANK 	= r18
.def 	WHITE_BANK 	= r19
.def 	BLUE_BANK 	= r20

.equ 	LED_SEQUENCE 	= 0b00000000
.equ 	DDRA 		= 0x01
.equ 	PORTA 		= 0x02
.equ 	DDRC 		= 0x07
.equ 	PORTC 		= 0x08
.equ 	DDRD 		= 0x0A
.equ 	PORTD 		= 0x0B

start:
		ldi 	FULL_OFF, 	0x00
		ldi 	FULL_ON, 	0xff

		out     DDRA, 		FULL_ON 	; PORT A all OUTPUT
		out     DDRC, 		FULL_ON 	; PORT C all OUTPUT
		out     DDRD, 		FULL_ON 	; PORT C all OUTPUT
		out     PORTA, 		FULL_ON 	; PORT A all ON FOR RESET / LED TEST
		out     PORTC, 		FULL_ON 	; PORT C all ON FOR RESET / LED TEST
		out     PORTD, 		FULL_ON 	; PORT C all ON FOR RESET / LED TEST
		rcall 	delay512ms

		ldi 	RED_BANK, 	LED_SEQUENCE
		mov 	WHITE_BANK, 	FULL_ON
		mov 	BLUE_BANK, 	FULL_ON
loop:
		out     PORTA, 		RED_BANK
		out     PORTC, 		WHITE_BANK
		out     PORTD, 		BLUE_BANK
		rcall 	delay32ms
		rcall 	delay32ms
		out     PORTA, 		FULL_OFF
		out     PORTC, 		FULL_OFF
		out     PORTD, 		FULL_OFF

		clc
		rol     RED_BANK
		ror     WHITE_BANK
		ror     BLUE_BANK
		brcc 	loop
		ori 	RED_BANK, 1
		rjmp 	loop

delay512ms:                  rcall delay256ms
delay256ms:                  rcall delay128ms
delay128ms:                  rcall delay64ms
delay64ms:                   rcall delay32ms
delay32ms:                   rcall delay16ms
delay16ms:                   rcall delay8ms
delay8ms:                    rcall delay4ms
delay4ms:                    rcall delay2ms
delay2ms:                    rcall delayms
delayms:
;millisecAt16MHz:        rcall millisecAt8MHz
millisecAt8MHz:         rcall millisecAt4MHz
millisecAt4MHz:         rcall millisecAt2MHz
millisecAt2MHz:         rcall millisecAt1MHz
millisecAt1MHz:		; roughly
tiks1024:               rcall tiks512
tiks512:                rcall tiks256
tiks256:                rcall tiks128
tiks128:                rcall tiks64
tiks64:                 rcall tiks32
tiks32:                 rcall tiks16
tiks16:                 rcall tiks8
tiks8:                 	nop
			ret
