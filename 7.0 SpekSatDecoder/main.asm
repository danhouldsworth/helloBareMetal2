.device ATmega128

.def specialChar 	= r0
.def data 			= r16
.def temp 			= r17
.def channel 		= r18
.def counter 		= r19
.def byteData 		= r20
.def lowByte 		= r21
.def highByte 		= r22
.def ZL 			= r30
.def ZH 			= r31

; Extended I/O
.equ UBRRH0 = 0xc5
.equ UBRRL0 = 0xc4
.equ UCSR0A = 0xc0
.equ UCSR0B = 0xc1
.equ UCSR0C = 0xc2
.equ UDR0 	= 0xc6

.equ SREG 	= 0x3f
.equ SPL 	= 0x3d
.equ SPH 	= 0x3e

.cseg
.org 0x0000
		rjmp 	setup
.org 0x0028
		ldi 	counter, 16
nextFrameByte:
		rcall 	waitUntilRXC0
		lds 	data, UDR0
		push 	data
		dec 	counter
		brne 	nextFrameByte
		jmp 	outputAsChannels

waitUntilRXC0:
		lds 	temp, UCSR0A
		bst 	temp, 7
		brtc 	waitUntilRXC0
clearRXC0:
		lds 	temp, UCSR0A
		ori 	temp, 0b10000000
		sts 	UCSR0A, temp
		ret

setup:
		ldi 	temp, 0
		sts 	UBRRH0, temp
		;ldi 	temp, 7 		; 16000000/(16*(7+1)) = 125000bps
		ldi 	temp, 8			; set baud 115200 @16MHz @1x (-3.5% error)
		;ldi 	temp, 16		; set baud 115200 @16MHz @2x (+2.1% error)
		sts 	UBRRL0, temp

		; RXCn 		| TXCn 		| UDREn 	| FEn 	| DORn 	| UPEn 	| U2Xn 	| MPCMn
		ldi 	temp, 0b11100000 		; 1x speed
		;ldi 	temp, 0b11100010 		; 2x speed
		sts 	UCSR0A, temp

		; RXCIEn 	| TXCIEn 	| UDRIEn 	| RXENn | TXENn | UCSZn2 | RXB8n | TXB8n
		; enable Rx/Tx pins
		ldi 	temp, 0b10011000
		sts 	UCSR0B, temp

    	; UMSELn1 	| UMSELn0 	| UPMn1 	| UPMn0 | USBSn | UCSZn1 | UCSZn0 | UCPOLn
    	; 8bits / Async / 1 stop bit / Disabled parity
		ldi 	temp, 0b00000110
		sts 	UCSR0C, temp

		jmp 	waitForRxInterrupt

outputAsChannels:
		ldi 	counter, 7
nextChannel:
		pop 	lowByte
		pop 	highByte
		mov 	channel, highByte
		andi 	channel, 0b11111000
		lsr		channel
		lsr		channel
		lsr		channel
		ldi 	temp, '0'
		add 	channel, temp
		mov 	data, channel
		rcall 	sendByteToUSART0
		ldi  	data, ':'
		rcall 	sendByteToUSART0
		ror 	highByte
		ror 	lowByte
		ror 	highByte
		ror 	lowByte
		ror 	highByte
		ror 	lowByte
;		mov 	data, highByte
;		andi 	data, 0b00000111
;		rcall 	sendBinaryString
		mov 	data, lowByte
		rcall 	sendBinaryString
		ldi  	data, ' '
		rcall 	sendByteToUSART0
		dec 	counter
		brne 	nextChannel

		jmp 	sendProgMemString

sendProgMemString:
        ldi 	ZH, high(textMessageStart << 1)
        ldi 	ZL, low(textMessageStart << 1)
nextByteFromProgMem:
        lpm 	data, Z+
        cpi    	data, '*'
        brne 	ordinaryChar
        pop		data ; think about whether an rcall has interferred with stack
        rcall 	sendBinaryString
        rjmp 	moveAlongString
ordinaryChar:
        rcall   sendByteToUSART0
moveAlongString:
        cpi 	ZL, low(textMessageEnd << 1)
        brne    nextByteFromProgMem
        cpi 	ZH, high(textMessageEnd << 1)
        brne    nextByteFromProgMem

waitForRxInterrupt:
        sei 	; note: we don't ret or reti as we discarded the 2 byte PC on the stack anyway
	 	rjmp 	waitForRxInterrupt

sendBinaryString:
		mov 	byteData, data
		push 	counter 		; preserve this as might be using
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
		pop 	counter 		; restore the counter
		ret

sendByteToUSART0:
		sts 	UDR0, data
waitUntilTXC0:
		lds 	temp, UCSR0A
		bst 	temp, 6
		brtc 	waitUntilTXC0
clearTXC0:
		lds 	temp, UCSR0A
		ori 	temp, 0b01000000
		sts 	UCSR0A, temp
		ret

textMessageStart:
.db 	"    Hdr#2(BindMode) -->*<--     Hdr#1(Fade) -->*<--", 10, 13
textMessageEnd:
