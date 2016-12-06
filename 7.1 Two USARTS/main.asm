
.include "../Reference/Libs/defSheets/m1284Pdef.inc"

.def PCreturn1		= r1
.def PCreturn2 		= r2
.def data 		= r16
.def temp 		= r17

.def counter 		= r19
.def byteData 		= r20

.cseg
.org 0x0000
		rjmp 	setup
.org URXC1addr
		pop 	PCreturn1
		pop 	PCreturn2

		lds 	data, UDR1
		push 	data

		lds 	temp, UCSR1A
		ori 	temp, (1 << RXC1)
		sts 	UCSR1A, temp

		dec 	counter
		breq 	dumpOutBuffer

		push 	PCreturn2
		push 	PCreturn1
		reti

setup:
		ldi 	temp, 0
		sts 	UBRR0H, temp
		sts 	UBRR1H, temp
		ldi 	temp, 16		; USART # 0 @ 115200bps  [16000000/((16/2)*(16+1)) = 117647] 16MHz @2x (+2.1% error)
		sts 	UBRR0L, temp
		ldi 	temp, 7 		; USART # 1 @ 125000bps  [16000000/((16/1)*( 7+1)) = 125000] 16MHz @1x (+-0.0% error)
		;ldi 	temp, 16 		; USART # 1 @ 115200bps  [16000000/((16/2)*(16+1)) = 117647] 16MHz @2x (+2.1% error)
		sts 	UBRR1L, temp

		; RXCn 		| TXCn 		| UDREn 	| FEn 	| DORn 	| UPEn 	| U2Xn 	| MPCMn
		ldi 	temp, (1 << RXC0) | (1 << TXC0) | (1 << UDRE0) | (1 << U2X0) 	; USART #0 @ 2x speed
		sts 	UCSR0A, temp
		ldi 	temp, (1 << RXC1) | (1 << TXC1) | (1 << UDRE1) | (0 << U2X1) 	; USART #1 @ 1x speed
		;ldi 	temp, (1 << RXC1) | (1 << TXC1) | (1 << UDRE1) | (1 << U2X1) 	; USART #1 @ 2x speed
		sts 	UCSR1A, temp

		; RXCIEn 	| TXCIEn 	| UDRIEn 	| RXENn | TXENn | UCSZn2 | RXB8n | TXB8n
		ldi 	temp, (1 << TXEN0) 			; USART #0 enable Tx pin only. No interupts
		sts 	UCSR0B, temp
		ldi 	temp, (1 << RXCIE1) | (1 << RXEN1) 	; USART #1 enable Rx pin, with interupts
		sts 	UCSR1B, temp

	    	; UMSELn1 	| UMSELn0 	| UPMn1 	| UPMn0 | USBSn | UCSZn1 | UCSZn0 | UCPOLn
		ldi 	temp, (1 << UCSZ01) | (1 << UCSZ00) 	; 8bits / Async / 1 stop bit / Disabled parity
		sts 	UCSR0C, temp
		sts 	UCSR1C, temp

	        ldi 	ZH, high(resetMessage << 1)
        	ldi 	ZL, low(resetMessage << 1)
		rcall 	printf

		ldi 	counter, 0x00

listen:        	sei 	; note: we don't ret or reti as we discarded the 2 byte PC on the stack anyway
wait:		rjmp 	wait

dumpOutBuffer:
	        ldi 	ZH, high(dumpStart << 1)
        	ldi 	ZL, low(dumpStart << 1)
		rcall 	printf
		ldi 	counter, 0x00
dumpNextByte:
		pop 	data
		rcall 	sendBinaryString
		dec 	counter
		brne 	dumpNextByte
	        ldi 	ZH, high(dumpEnd << 1)
        	ldi 	ZL, low(dumpEnd << 1)
		rcall 	printf
		rjmp 	wait

sendBinaryString:
		mov 	byteData, data
		push 	counter 		; preserve counter
		ldi 	counter, 8
nextBit:
		ldi 	data, '0'
		lsl 	byteData
		brcc 	is0
		ldi 	data, '1'
is0:
		rcall 	sendByteToUSART0
		dec 	counter
		brne 	nextBit
		pop 	counter 		; restore counter
		ldi 	data, ' '
		rcall 	sendByteToUSART0
		ret

sendByteToUSART0:
		sts 	UDR0, data
waitUntilTXC0:
		lds 	temp, UCSR0A
		bst 	temp, TXC0
		brtc 	waitUntilTXC0
clearTXC0:
		lds 	temp, UCSR0A
		ori 	temp, (1 << TXC0)
		sts 	UCSR0A, temp
		ret

printf:
		lpm 	data, Z+
		cpi    	data, 0
		brne 	processStringByte
		ret
processStringByte:
		cpi    	data, '*'
		brne 	ordinaryChar
		pop	data 			; be careful with always pushing required args
		rcall 	sendBinaryString
		rjmp 	printf
ordinaryChar:
		rcall   sendByteToUSART0
		rjmp 	printf

resetMessage: 	.db 10,13,"ATmega1284p --> Reset", 10, 13, 0
dumpStart: 	.db 10,13,"Commencing dump of 256 bytes (hopefully 16 frames) IN REVERSE....", 10, 13, 0
dumpEnd: 	.db 10,13,"FINISHED dump of 256 bytes (hopefully 16 frames)....", 10, 13, 0

