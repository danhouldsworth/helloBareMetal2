; Problem 1 : SS *MUST* be set as output to remain in SPI master mode
; Problem 2 : If an incomplete frame is written to the OLED it displays what is already there. Appearing that flash hasn't taken

; Simplest possible validater of OLED
;               328p          1284p
; Clock         16MHz           16MHz
; OLED D/C      B1              B1

; SS            B2              B4
; MOSI          B3              B5
; MISO          B4              B6
; SCK           B5              B7

; BreadBoard OLED
; ISP breakout : RST / MOSI / SCK
; Power Rails
; CS=SS | D/C = PB1

.def    temp    = r16
.def    cData   = r19

.equ    DDRB    = 0x04
.equ    PORTB   = 0x05

.equ    SPCR    = 0x2c
.equ            SPE     = 6
.equ            MSTR    = 4
.equ    SPSR    = 0x2d
.equ            SPIF    = 7
.equ            SPI2X   = 0
.equ    SPDR    = 0x2e
; ATmega328 SPI / PORTB Mapping
;.equ    SS      = 2
;.equ    MOSI    = 3
;.equ    SCK     = 5
; ATmega1284 SPI / PORTB Mapping
.equ    SS      = 4
.equ    MOSI    = 5
.equ    SCK     = 7

setup:
        sbi     DDRB,   1                  ; D/C
        sbi     PORTB,  1

        sbi     DDRB,   SS                  ; SS
        sbi     PORTB,  SS
        sbi     DDRB,   MOSI                  ; MOSI
        sbi     PORTB,  MOSI
        sbi     DDRB,   SCK                  ; SCK
        sbi     PORTB,  SCK

        ldi     temp, (1 << SPE)
        ori     temp, (1 << MSTR)
        out     SPCR, temp

        ldi     temp, (1 << SPI2X)
        out     SPSR, temp

        cbi     PORTB,  1               ; OLED 'Commands'
        cbi     PORTB,  SS               ; CS low
        rcall   bootOLED
        sbi     PORTB,  SS               ; CS high
        sbi     PORTB,  1               ; OLED 'Data'
        cbi     PORTB,  SS               ; CS low

solidBitmap:
        ldi     cData, 255
        rcall   hwSPI
        rjmp    solidBitmap

hwSPI:
        out     SPDR, cData
waitSPIF:
        in      temp, SPSR
        sbrs    temp, SPIF
        brne    waitSPIF
        ret

bootOLED:
        ldi     cData, 0xAE ; Screen OFF
        rcall   hwSPI
        ldi     cData, 0xD5 ; Set Fosc = Max, Clock Div = 1
        rcall   hwSPI
        ldi     cData, 0x80
        rcall   hwSPI
        ldi     cData, 0x8D ; Enable Charge Pump
        rcall   hwSPI
        ldi     cData, 0x14
        rcall   hwSPI
        ldi     cData, 0x20 ; Set page address mode = horizontal
        rcall   hwSPI
        ldi     cData, 0x00
        rcall   hwSPI
        ldi     cData, 0x81 ; Set contrast = MAX
        rcall   hwSPI
        ldi     cData, 0xFF
        rcall   hwSPI
        ldi     cData, 0xAF ; Screen ON
        rcall   hwSPI
        ret