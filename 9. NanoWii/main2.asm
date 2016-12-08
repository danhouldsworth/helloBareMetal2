; **** Delays ****
; Note : All instructions take clock cycles :
;       rcall   : 3 cycles
;       nop     : 1 cycle
;       ret     : 4 cycles
; So
; The finest resolution we can delay is to do a number nops, we can then recursively nest them to get larger numbers
;                                                     nop                                                                                                       =  0x3 + 1*3 + 0*4 = 1
; "rcall 1nop"  [===                            rcall nop ret                                                                                                   =  1*3 + 1*3 + 1*4 = 10
; "rcall 2nops" [===                      rcall rcall nop ret nop ret                                                                                           =  2x3 + 2*3 + 2*4 = 20
; "rcall 4nops" [===                rcall rcall rcall nop ret nop ret rcall nop ret nop ret                                                                     =  4x3 + 4*3 + 4*4 = 40
; "rcall 8nops" [===          rcall rcall rcall rcall nop ret nop ret rcall nop ret nop ret rcall rcall nop ret nop ret rcall nop ret nop ret                   =  8x3 + 8*3 + 8*4 = 100
; So... rcall fullSec is ~1s @8MHz clock :-)
; ****************

;       1us = 1tik @ 1MHz       1ms = 1000tik
;       1us = 8tik @ 8MHz       1ms = 8000tik
;       1us = 16tik @ 16MHz     1ms = 16000tik


;    DDRD = PORTD |=  (1 << 5); // Green LED  : LOW = ON
 ;   DDRC = PORTC |=  (1 << 7); // YELLOW LED  : HIGH = ON
.equ    DDRD    = 0x0a
.equ    PORTD   = 0x0b

.org 0x0000


start:
        sei
        sbi     DDRD, 5

loop:
        rcall   LEDon
        rcall   LEDoff
        rjmp    loop

LEDon:
        cbi     PORTD, 5
        rcall   delaysecond
        ret
LEDoff:
        sbi     PORTD, 5
        rcall   delaysecond
        ret

delaysecond:
        rcall ms512
        rcall ms256
        rcall ms128
        rcall ms64
        rcall ms32
        rcall ms8
        ret
delayTenthSec:
        rcall ms64
        rcall ms32
        rcall ms4
        ret

ms512:                  rcall ms256
ms256:                  rcall ms128
ms128:                  rcall ms64
ms64:                   rcall ms32
ms32:                   rcall ms16
ms16:                   rcall ms8
ms8:                    rcall ms4
ms4:                    rcall ms2
ms2:                    rcall delayms
delayms:                                        ; comment out the frequencies above our rating so this label falls through at appropriate point
millisecAt16MHz:        rcall millisecAt8MHz
millisecAt8MHz:         rcall millisecAt4MHz
millisecAt4MHz:         rcall millisecAt2MHz
millisecAt2MHz:         rcall millisecAt1MHz
millisecAt1MHz:
kilotik:
                        rcall tiks640
                        rcall tiks320
                        rcall tiks20
                        rcall tiks10
                        nop
                        nop
                        nop
                        ret

tiks640:                rcall tiks320
tiks320:                rcall tiks160
tiks160:                rcall tiks80
tiks80:                 rcall tiks40
tiks40:                 rcall tiks20
tiks20:                 rcall tiks10
tiks10:                 nop
                        nop
                        nop
                        ret
