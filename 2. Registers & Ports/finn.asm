
;                        328@8MHz     328@1MHz      Nano    32u4
;    BLUE    : B4 = HIGH     *           *           *       *
;    RED     : B5 = HIGH     *           *           *       GRN
;    WHITE   : B7 = HIGH     *           *                   *
;    YELLOW  : C7 = HIGH                                     *
;    GREEN   : D5 = LOW      *           *           *       *

.DEVICE ATmega328p
        ldi     r16, 0xFF
        out     0x04,r16
        out     0x07,r16
        out     0x0A,r16

loop:
        inc 	r16
        rcall 	del16ms
        out     0x05,r16
        rcall 	del16ms
        out     0x08,r16
        rcall 	del16ms
        out     0x0B,r16
        rjmp 	loop


fullSec:rcall halfSec
halfSec:rcall quart_S
quart_S:rcall tenth_S
tenth_S:rcall del64ms
del64ms:rcall del32ms
del32ms:rcall del16ms
del16ms:rcall del_8ms
del_8ms:rcall del_4ms
del_4ms:rcall del_2ms
del_2ms:rcall delayms
delayms:rcall nops512
nops512:rcall nops256
nops256:rcall nops128
nops128:rcall nops64
nops64: rcall nops32
nops32: rcall nops16
nops16: rcall nops8
nops8:  rcall nops4
nops4:  rcall nops2
nops2:  rcall nops1
nops1:  nop
        ret


