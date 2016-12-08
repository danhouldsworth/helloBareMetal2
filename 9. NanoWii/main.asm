; ======= Minimal subset of defsheet
.equ    DDRD    = 0x0a
.equ    PORTD   = 0x0b
.equ    TIFR0   = 0x15
.equ    TOV0    = 0 ; Timer/Counter0 Overflow Flag
.equ    TCNT0   = 0x26
.equ    TCCR0B  = 0x25
.equ    TCCR0A  = 0x24
.equ    MCUCR   = 0x35
.equ    TIMSK0  = 0x6e  ; MEMORY MAPPED
.equ    UDIEN   = 0xe2  ; MEMORY MAPPED
.equ    OVF0addr    = 0x002e    ; Timer/Counter0 Overflow
.equ    USB_GENaddr = 0x0014    ; USB General Interrupt Request
; ===== register use
.def    counter = r16
.def    temp    = r17
.def    mask    = r18
.def    ZH      = r31
.def    ZL      = r30
; ===== interupt vectors
.org 0x0000
        ;rjmp    loopBlocking
        ;rjmp    setupTimer
        rjmp    Move_interrupts

.org USB_GENaddr
        reti
.org 0x002c
        reti
.org OVF0addr
        jmp     TIMER0_OVF_ISR

.org 0x100
        reti ; catch all

Move_interrupts:

        ;ldi temp, 0b00000001 ; Enable change of Interrupt Vectors
        ;ldi mask, 0b00000000 ; Move interrupts to zero
        ;out MCUCR, temp
        ;out MCUCR, mask
        ldi     ZH, HIGH(UDIEN)
        ldi     ZL, LOW(UDIEN)
        ld      temp, Z
        andi     temp, 0b11111110 ; Clear USB interupts
        st      Z, temp

setupTimer:
        ; ******** I/O Registers from 0x00-0x3f can be read/write directly with in / out *******
        ldi     temp, 0
        out     TCNT0, temp             ; Reset the counter

        ; NOTE - This is heavy handed, but means I can simply choose all settings in the header
        ldi     temp, 0b00000000
        out     TCCR0A, temp
        ldi     temp, 0b00000011 ; /64 prescaler
        out     TCCR0B, temp

        ldi     temp, 0b00000001 ; TOIE
        ldi     ZH, HIGH(TIMSK0)
        ldi     ZL, LOW(TIMSK0)
        st      Z, temp                 ; Enable interupts for OCRA / OCRB and Timer overflow
        ; **************************************************************************************
; =====================================
        sbi     DDRD, 5
        sbi     PORTD, 5 ; Off
        ;rjmp    setupTimer
        rjmp    enableIntertups
        rjmp    loopForeverCheckingTimer
; =====================================

enableIntertups:
        sei
waitForInterupts:
        sbi     DDRD, 5
        rjmp    waitForInterupts

loopForeverCheckingTimer:
        sbi     DDRD, 5
        sbic    TIFR0, TOV0
        rcall   TIMER0_OVF_manual
        rjmp    loopForeverCheckingTimer

loopBlocking:
        sbi     DDRD, 5
        rcall   LEDon
        rcall   LEDoff
        rjmp    loopBlocking

; =====================================

LEDon:
        cbi     PORTD, 5
        rcall   delaysecond
        ret
LEDoff:
        sbi     PORTD, 5
        rcall   delaysecond
        ret


TIMER0_OVF_ISR:
        dec     counter
        brne    notYetMs
        ldi     mask, 1<<5
        in      temp, PORTD
        eor     temp, mask
        out     PORTD, temp
notYetMs:
        reti

TIMER0_OVF_manual:
        sbi     TIFR0, TOV0 ; or do we wite to clear?
        dec     counter
        brne    notYetMs2
        ldi     counter, 130
        ldi     mask, 1<<5
        in      temp, PORTD
        eor     temp, mask
        out     PORTD, temp
notYetMs2:
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
