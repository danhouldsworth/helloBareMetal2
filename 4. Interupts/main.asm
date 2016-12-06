.device ATmega328p
; **** Defines a synonym for a register (e.g. .DEF MyReg = R16) ****
.def    ZH      = r31
.def    ZL      = r30
; ************** Defines a symbol and sets its value  **************
.equ    PCIE0   = 0
.equ    PCICR   = 0x68
.equ    PCMSK0  = 0x6b
.equ    PCINT1  = 1
; ******************************************************************

; **** Start of the code segment - assembled into program memory ***
.cseg
.org 0x0000
                        jmp handle_RESET
                        jmp handle_RESET
                        jmp handle_RESET
                        jmp handle_PCINT0
; Note : rjmp & reti are 1 word instructions. Whereas jmp is a 2 word instruction
; So to save address organising each interrupt vector we should use jmp in correct sequential order
; ******************************************************************

.org 0x0034
handle_RESET:
                        ldi     r16, (1 << PCIE0)               ; PCICR = (1 << PCIE0) [enable PinChangeInterupt 0]
                        ldi     ZH, HIGH(PCICR)
                        ldi     ZL, LOW(PCICR)
                        st      Z, r16

                        ldi     r16, (1 << PCINT1)              ; PCMSK0 = (1 << PCINT1) [enable Pin 1 to fire PCINT0]
                        ldi     ZH, HIGH(PCMSK0)
                        ldi     ZL, LOW(PCMSK0)
                        st      Z, r16

                        sei

start:
                        ldi     r16, (1 << 4) | (1 << 5) | (1 << 7)
                        out     0x04,r16
                        out     0x05,r16

                        sbi     0x07, 7
                        sbi     0x08, 7

                        sbi     0x0A, 5
                        cbi     0x0B, 5

loop:
                        rjmp    loop

handle_PCINT0:
                        cbi     0x04, 4
                        cbi     0x04, 5
                        cbi     0x04, 7
                        cbi     0x08, 7
                        sbi     0x0B, 5
                        reti

end:
; ***** Compiler will pad out to fill double byte (word) memory *****
myProgmemData:
.org    0x2FFF
.db                     "Hello, world!!"
.dw                     0x1234, 0x5678
; **** 'data segment' This will be wrtten to DATA MEMORY / SRAM ****
.dseg
.org    0x0100
mySramDataLabel1: .byte 1
mySramDataLabel2: .byte 1
; ******************************************************************