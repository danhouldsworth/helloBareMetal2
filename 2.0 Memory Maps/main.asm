.DEVICE ATmega128

.def    temp            = r17
.def    RED_BANK        = r18
.def    WHITE_BANK      = r19
.def    BLUE_BANK       = r20

; Primary I/O
.equ    DDRA            = 0x01
.equ    PORTA           = 0x02
.equ    DDRC            = 0x07
.equ    PORTC           = 0x08
.equ    DDRD            = 0x0A
.equ    PORTD           = 0x0B
.equ    SREG            = 0x3f
.equ    SPH             = 0x3e
.equ    SPL             = 0x3d
.equ    SPMCSR          = 0x37
.equ    SIGRD           = 5
.equ    BLBSET          = 3
.equ    SPMEM           = 0
; Extended I/O
.equ    OSCCAL          = 0x66

.org    0x0000
start:
; **************************** SET STACK POINTER ****************************
                ;ldi     r16,            0x02
                ;out     SPL, r16
                ;ldi     r16,            0x02
                ;out     SPH, r16
; ***************************************************************************

; ************************ Set some reg values & PORTS **********************
                ldi     temp,           0xff
                out     DDRA,           temp         ; PORT A all OUTPUT
                out     DDRC,           temp         ; PORT C all OUTPUT
                out     DDRD,           temp         ; PORT C all OUTPUT
; ***************************************************************************

; *************************** Store my user data ****************************
                ;LDI     r16,            0b01010101
                ;STS     myMem, r16
; ***************************************************************************

; **************************** Mess with the stack **************************
                ;push    temp
                ;push    temp
                ;push    RED_BANK
                ;push    temp
; ***************************************************************************

; ******************** Read the primary reg of interest ********************
                IN      RED_BANK,       SREG
; ***************************************************************************

; **************** Read the 16bit SP into Blue / White banks ****************
                IN      WHITE_BANK,     SPH
                IN      BLUE_BANK,      SPL
; ****************** Read the contents of the SP into RED *******************
                POP     RED_BANK
; ***************************************************************************

; **************** Read a 16bit opcode into Blue / White banks **************
                ;LDI     r31,            high(start << 1)
                ;LDI     r30,            low(start << 1)
                ;LPM     BLUE_BANK,      Z+
                ;LPM     WHITE_BANK,     Z+
; ***************************************************************************


; **************** Read LF/HF/EF FUSES into RED / WHITE / BLUE **************
                ;LDI     R31,0x0000
                ;LDI     R30,0x0000
                ;LDI     temp, (1 << BLBSET) + (1 << SPMEM)
;
                ;OUT     SPMCSR, temp
                ;LPM     RED_BANK, Z+    ; LFuse
                ;OUT     SPMCSR, temp
                ;LPM     WHITE_BANK, Z+   ; Lock bits
                ;OUT     SPMCSR, temp
                ;LPM     BLUE_BANK, Z+   ; EFuse
                ;OUT     SPMCSR, temp
                ;LPM     BLUE_BANK, Z+  ; HFuse
; ***************************************************************************

; **************** Read signiture into RED / WHITE / BLUE **************
                ;LDI     R31,0x0000
                ;LDI     R30,0x0000
                ;LDI     temp, (1 << SIGRD) + (1 << SPMEM)
;
                ;OUT     SPMCSR, temp
                ;LPM     RED_BANK, Z+    ; sig 1
                ;OUT     SPMCSR, temp
                ;LPM     WHITE_BANK, Z+    ; RS OSC Calib
                ;OUT     SPMCSR, temp
                ;LPM     WHITE_BANK, Z+   ; sig 2
                ;OUT     SPMCSR, temp
                ;LPM     BLUE_BANK, Z+   ; ??
                ;OUT     SPMCSR, temp
                ;LPM     BLUE_BANK, Z+  ; sig 3
; ***************************************************************************

; *************** Display the address and contents of myMem *****************
                ;LDS     RED_BANK,       myMem
                ;LDI     WHITE_BANK,     high(myMem)
                ;LDI     BLUE_BANK,      low(myMem)
; ***************************************************************************

; ******************* Directly view 3 bytes of any SRAM  ********************
                ;LDI     temp, 1
                STS     OSCCAL,         temp
                LDS     RED_BANK,       OSCCAL
                LDI     WHITE_BANK,     high(OSCCAL)
                LDI     BLUE_BANK,      low(OSCCAL)
; ***************************************************************************

; ********** Read the contents (w/o increment) of the SP into RED ***********
                ;LDS     r31,            0x005e
                ;LDS     r30,            0x005d
                ;LD      RED_BANK, Z
; ***************************************************************************

; ************************* Disaply to my LED banks *************************
                out     PORTA,          RED_BANK
                out     PORTC,          WHITE_BANK
                out     PORTD,          BLUE_BANK
loop:           rjmp    loop
; ***************************************************************************

testOpCode:     out PORTD, r17
myProgMem1:
.dw 0x1234
.dw 0x4321

.dseg
.org    0x0100
myMem: .byte 1