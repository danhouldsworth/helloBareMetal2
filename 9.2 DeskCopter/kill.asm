; ======= Minimal subset of defsheet
.equ    PORTC   = 0x08
.equ    DDRC    = 0x07
.equ    PINC    = 0x06
.equ    PORTB   = 0x05
.equ    DDRB    = 0x04
.org 0x0000
        cli
        sbi     DDRB,   7
        sbi     PORTB,  7        ; PB7 / RED  - HIGH=ON
        sbi     DDRC,   7
        sbi     PORTC,  7        ; PC7 / GREEN - HIGH=ON
        sbi     DDRC,   6
        sbi     PORTC,  6        ; PC6 / YELLOW - HIGH=ON

loopForever:
        rjmp    loopForever
