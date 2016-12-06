.device ATmega128

.def temp = r17
.def data = r16
.def counter = r18
.def byteData = r19
.def specialChar = r0

; Extended I/O
.equ OSCCAL = 0x66
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
		lds 	data, UDR0
		rcall 	sendBinaryString
		ldi 	data, 10
		rcall 	sendByteToUSART0
		ldi 	data, 13
		rcall 	sendByteToUSART0
		reti
.org 0x0100
setup:
		ldi 	temp, 0
		sts 	UBRRH0, temp
		ldi 	temp, 8			; set baud 115200 @16MHz @1x (-3.5% error)
		;ldi 	temp, 16		; set baud 115200 @16MHz @2x (+2.1% error)
		;ldi 	temp, 12		; set baud  76800 @16MHz @1x (+0.2% error)
		;ldi 	temp, 25		; set baud  76800 @16MHz @2x (+0.2% error)
		;ldi 	temp, 3			; set baud 115200 @8MHz @2x (+8.5% error)
		;ldi 	temp, 8			; set baud 115200 @8MHz @2x (-3.5% error)
		;ldi 	temp, 12		; set baud  76800 @8MHz @2x (+0.2% error)
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

		in 		temp, SPL
		push 	temp
		in 		temp, SPH
		push 	temp
		in 		temp, SREG
		push 	temp
		lds 	temp, OSCCAL
		push 	temp
		lds 	temp, UCSR0C
		push 	temp
		lds 	temp, UCSR0B
		push 	temp
		lds 	temp, UCSR0A
		push 	temp

	;	Don't rcall before read stack

sendProgMemString:
        ldi 	r31, high(textMessageStart << 1)
        ldi 	r30, low(textMessageStart << 1)
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
        cpi 	r30, low(textMessageEnd << 1)
        brne    nextByteFromProgMem
        cpi 	r31, high(textMessageEnd << 1)
        brne    nextByteFromProgMem
        ;ret
        sei
waitForRxInterrupt:
	 	rjmp 	waitForRxInterrupt

sendBinaryString:
		mov 	byteData, data
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
.db 	10, 13
.db 	10, 13,"================== Welcome to D4U! (Dans Dataspace Dump & Debug over USART!) ====================="
.db 	10, 13
.db 	10, 13, "USARTn   : 0"
.db 	10, 13, "Baud     : 115,200"
.db 	10, 13, "Data     : 8N1"
.db 	10, 13, "Mode     : Async. 2x = 0"
.db 	10, 13
.db 	10, 13,"============================= Device Setup & Device Information =================================="
.db 	10, 13
.db 	10, 13, "Device   : ATmega1284p"
.db 	10, 13, "Clock    : 16MHz"
.db 	10, 13, "Signiture: 0x1e9705"
.db 	10, 13, "Fuses    : E:0xFF H:0x99 L:0xE2"
.db 	10, 13
.db 	10, 13,"================================= Selection of registers to Peek ================================="
.db 	10, 13
.db 	10, 13, "UCSR0A   : *"
.db 	10, 13, "UCSR0B   : *"
.db 	10, 13, "UCSR0C   : *"
.db 	10, 13, "OSCCAL   : *"
.db 	10, 13, "SREG     : *"
.db 	10, 13, "SPH      : *"
.db 	10, 13, "SPL      : *"
.db 	10, 13, "(Stack)  : *"
.db 	10, 13
.db 	10, 13,"=================================================================================================="
.db 	10, 13
.db 	10, 13, "Binary Echo service...."
textMessageEnd: