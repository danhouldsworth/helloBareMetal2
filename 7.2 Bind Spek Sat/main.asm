.DEVICE ATmega128

.def 	counter 	= r16
.equ 	DDRD 		= 0x0A
.equ 	PORTD 		= 0x0B

start:
		sbi     DDRD, 2
		sbi     DDRD, 7
		sbi 	PORTD,2
		sbi 	PORTD,7 	; Power On Sat Rx
		rcall 	delay32ms 	; wait to boot and listen
		;rcall 	delay32ms 	; 2nd delay for orange. Gets us closed to https://github.com/cleanflight/cleanflight/blob/886e55f7d0182a0cfc32186e13d632b1072556f2/src/main/rx/spektrum.c
		;rcall  delay32ms;
;DSMX/DSM2 Bind Modes:
;	Pulses 	Mode 	Protocol 	Frame	Precision	*Validated
;	---------------------------------------------------------------------------------------------
;	3 	Internal DSM2 		22ms 	1024 (10-bit) 	3
;	5 	Internal DSM2 		11ms 	2048 (11-bit) 	5
;	7 	Internal DSMx 		22ms 	2048 (11-bit) 	7
;	9 	Internal DSMx 		11ms 	2048 (11-bit) 	9
; 	+1 (4,6,8,10) gives the corresponding External mode, which we don't want.
; * Validated with (Sat poweroff during AVR flash / 32ms reboot wait / 0.25ms pulse period)
; 	GPIO PD7 drives 3.3v Reg directly. Sufficient for reboot and bind mode.
; 	Manually add 5v power to Reg 5Vin for binding process. Binding does not complete without this.

		ldi 	counter, 9
pulse:
		sbi     PORTD, 2
		rcall 	millisecAt2MHz
		cbi 	PORTD, 2
		rcall 	millisecAt2MHz
		dec 	counter
		brne	pulse


done: 		rjmp 	done

delay512ms:   		rcall delay256ms
delay256ms:   		rcall delay128ms
delay128ms:   		rcall delay64ms
delay64ms:    		rcall delay32ms
delay32ms:    		rcall delay16ms
delay16ms:    		rcall delay8ms
delay8ms:     		rcall delay4ms
delay4ms:     		rcall delay2ms
delay2ms:     		rcall delayms
delayms:
millisecAt16MHz:        rcall millisecAt8MHz
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
