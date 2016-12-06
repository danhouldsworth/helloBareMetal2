.include "../Reference/Libs/defSheets/m328Pdef.inc"

.def    temp    = r16
.def    xPos    = r17
.def    colPos  = r18
.def    cData   = r19

.cseg
.org 0x0000
jmp    setup

setup:
        sbi DDRB, DDB1                  ; D/C
        sbi PORTB, PORTB1
        sbi DDRB, DDB2                  ; SS
        sbi PORTB, PORTB2
        sbi DDRB, DDB3                  ; SCK
        sbi PORTB, PORTB3
        sbi DDRB, DDB5                  ; MOSI
        sbi PORTB, PORTB5

        ldi temp, (1 << SPE)
        ori temp, (1 << MSTR)
        out SPCR, temp

        ldi temp, (1 << SPI2X)
        out SPSR, temp

        cbi PORTB, PORTB1               ; OLED 'Commands'
        cbi PORTB, PORTB2               ; CS low
        rcall bootOLED
        sbi PORTB, PORTB2               ; CS high

        sbi PORTB, PORTB1               ; OLED 'Data'
        cbi PORTB, PORTB2               ; CS low
        rcall bitmap
        sbi PORTB, PORTB2               ; CS high

loopForever:
        rjmp    loopForever

hwSPI:
        out SPDR, cData
waitSPIF:
        in temp, SPSR
        sbrs temp, SPIF
        brne waitSPIF
        ret

bootOLED:
        ldi cData, 0xAE ; Screen OFF
        rcall hwSPI
        ldi cData, 0xD5 ; Set Fosc = Max, Clock Div = 1
        rcall hwSPI
        ldi cData, 0x80
        rcall hwSPI
        ldi cData, 0x8D ; Enable Charge Pump
        rcall hwSPI
        ldi cData, 0x14
        rcall hwSPI
        ldi cData, 0x20 ; Set page address mode = horizontal
        rcall hwSPI
        ldi cData, 0x00
        rcall hwSPI
        ldi cData, 0x81 ; Set contrast = MAX
        rcall hwSPI
        ldi cData, 0xFF
        rcall hwSPI
        ldi cData, 0xAF ; Screen ON
        rcall hwSPI
        ret

bitmap:
        ldi colPos, 0x8
column:
        ldi xPos, 128
line:
        ldi temp, 0
        sbrc xPos, 3
        inc temp
        sbrc colPos, 0
        inc temp

        ldi cData, 0
        sbrs temp, 0
        ldi cData, 255
        rcall hwSPI

        dec xPos
        brne line
        dec colPos
        brne column

        ret


oledBootSequence:
.dw     0xAE,0xD5,0x80,0x8D,0x14,0x20,0x00,0x81,0xFF,0xAF