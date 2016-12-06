
;                        328@8MHz     328@1MHz      Nano    32u4
;    BLUE    : B4 = HIGH     *           *           *       *
;    RED     : B5 = HIGH     *           *           *       GRN
;    WHITE   : B7 = HIGH     *           *                   *
;    YELLOW  : C7 = HIGH                                     *
;    GREEN   : D5 = LOW      *           *           *       *

.DEVICE ATmega328p
.cseg
.org 	0x0000
start:
        ldi     r16, (1 << 4) | (1 << 5) | (1 << 7)
        out     0x04,r16
        out     0x05,r16

        sbi     0x07, 7
        sbi     0x08, 7

        sbi     0x0A, 5
        cbi     0x0B, 5