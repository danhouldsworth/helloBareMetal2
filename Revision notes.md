# Revision notes on AVR following higher than expected frustration levels

## Three separate memory spaces for SRAM, FLASH & EEPROM

### SRAM [Octet address mapping]:
0x0000 - 0x001F 	32 general purpose registers R0-R31 (of which R26-31 double up as X,Y,Z 16-bit special registers)
0x0020 - 0x005F 	64 I/O registers that can be accessed directly
	(0x0000 - 0x001F) 	[1st half of I/O space] has bit access instructions SBI / CBI / SBIS / SBIC when addressed with its direct mapping
	(0x0000 - 0x003F)   [Full primary I/O space] has byte access instructions IN / OUT when addressed with its direct mapping
0x0060 - 0x00FF 	160 extended I/O registers. Accessed indirectly via normal LDI / STI and Z register
0x0100 - 0x08FF 	2048 bytes of SRAM for stack and user data (.dseg can ONLY address this sub section)

### FLASH [16-bit Word address mapping]:
0x0000 - 0x0032 	26x Interupt Vector (2 instruction words wide, so 32 bit / 4bytes)
0x0000 - 0x3FFF 	16K instruction words (= 32KB) of programmable persistent flash

### EEPROM :
-- I've yet to play with this.

## H/W AVR Notes:
1. The datasheets refer to the IO pins being able to source/sink sufficient current to drive LED displays directly, yet the maximum ratings of each pin is 40mA with the entire Vcc pin rating of 200mA. I have a suspicion (can't find my multimeter) that my LEDs a drawing much beyond this.
2. I've now isolated that both i) a reset, and ii) odd execution behaviour (freeze, pause, unexpected register values), can both be seen from driving LEDs directly.
3. Counter intuitively, LEDs with a *higher* voltage drop cause the biggest issue, so clearly my understanding of current flowing through an LED is not right. Q: Perhaps they have an intrinsic resistance which is inversely proportional to the voltage drop? (eg a single White LED can blow the chip, vs a few Blues, vs many Reds)
4. If a chip has shipped with fuse set for external full swing crystal, then there is simply no way to communicate with it without first giving it such a clock source. Even to read & check whether this is the case.
5. Conversely, if it has shipped set for internal clock with /8Div (ie 1MHz) we must manually override the bit speed of our ISP before it will detect and communicate with the chip.
6. To compound this, (and further to points 1-3) if we're sinking too much current with LEDs on the SPI pins then we will also be unable to detect and communicate with the chip.
7. To replay the above - many of my new chips shipped set for either ext. crystal or 1MHz, or new/existing chips set at internal 8MHz and working on one board but I've then tried to program while hooked up with LEDs on SPI to test the board. *ALL* of these will show as 'error : chip not found/responding' when we try and read / write to it. Which is also the error we get if there is a connection issue anywhere on the board. Careful systematic approach always needed.
8. Just for kicks, a fuse bit is 'programmed' / 'set' when it has logical 0. While I'm sure there's a valid legacy reason, and it shouldn't be difficult once informed, its about as helpful as riding a bike having being told the steering is reversed.

## S/W AVR Notes:
1. Only FLASH and EEPROM can be written 'flashed' during programming. ALL SRAM is set during runtime (although some registers have non-zero initial values). Despite some instructions appearing to use similar indirect address registers (LD r0, Z, and LPM r0, Z) they are have separate memory address space.
2. Peripherals are mapped to SRAM address space in the form of single or double registers.
3. As with ARM assembly, all instructions are either directly manipulating registers, moving bytes / words between SRAM and registers, or changing the PC.
4. *All* SRAM (including GP registers, I/O registers, SP & SREG) can be accessed indirectly using LD / ST and one of the 16-bit X,Y,Z registers)
5. *Some* SRAM (GP registers, and primary I/O registers) can be accessed directly with dedicated instructions.
6. Some instructions can only be used in *a subset* of registers (eg r16-31 only) or data address space.
7. The status register (SREG) [located in SRAM @ [0x3F (0x5F)] is made up of the 8 flags that all conditional execution (comparison and branch instructions) are based around setting / reading. It can additionally be accessed bitwise with BSET / BCLR.
*CAREFUL!!* Some bitwise instructions refer the bit number (BSET, BCLR, SBI, CBI) while others refer to a bit mask (SBR, CBR). Why they couldn't just make the instruction name clear this is an OR mask aka ORI is beyond me??
8. The stack pointer (SP) [located as a double register [SPH 0x3E (0x5E), SPL 0x3D (0x5D)], points to the top of the SRAM where GP registers & PC can be pushed / pulled in a LIFO manner by both software (PUSH / POP) and hardware (RCALL, ICALL, RET, RETI). It is implemented in a descending order. The SP must always remain > 0x0100 (ie SRAM beyond register & I/O space). Q : It's not clear what happens / how it prevents SP < 0x100 during runtime?
*CAREFUL!!* The data sheet has a conflicting view of it's initial value : "SPH=0x00 SPL=0xFF" vs "last address of SRAM" vs "All user programs must initialise the SP in the Reset routine (before subroutines or interrupts are executed)." so I need to explicitly test this.
9. In addition to the stack, application data can make use of SRAM by reserving byte space with .byte directive after .dseg, and as with the stack this must be > 0x0100. Note : this just returns an address label, and I can't see that it's actually 'reserving' anything as usually understood by a malloc() type process, as there's nothing to stop our stack extending through it, or indirectly accessing it via an over indexed LD / ST.
10. The program counter (PC) points to the instruction word in program memory that should be executed next. Some instructions can take multiple clock cycles. Note : The PC is *NOT* stored in the SRAM registers in AVR unlike say in ARM7 architecture, where R13,14,15 are the SP, LR, PC (Q: Why not?) It starts with initial value of 0x0000 which is the interrupt vector for reset.
11. If both the global interupt flag (SREG7) and specific interupt flag (such as timer, SPI byte Rx ready, pin change etc) are set, then in the event of a the internal / external trigger, the 'interupt' will push the PC onto the stack, and then point execution to the corresponding interrupt vector - freely user defined, and typically needs to be a JMP subRoutineLabel as we only have 32bits before the next interupt vector, but in theory could be a single word instruction before returning eg. OUT PORTB, 0xFF; RETI;
11. When program memory is used for application data [PROGMEM] we need to be careful that progmem is addressed in 16-bit words, vs SRAM addressed in octets. Ie. a label in flash memory will be half that of the address space if we address it in bytes - as we will when using LPM Rd, Z.
12. Some of the status flags (eg Pin Change interupt) are cleared by writing a logical one. I'm going to need some clear practise on this as that sounds like we need to SBI SREG(global interupt) but then CBI (pin change interupt) to ensure both are set. This is not what I did when I had a working set up for user input via an input pin. Q: Is this only to do with clearing the interupt mid-flight when we're handling it directly rather than letting the routine pointed to by the interupt vector and RETI instruction?
13. Lastly, within the similar architectures such as mega328 / mega32u4 / mega1284p, many of the instructions, register mapping, interupt vectors are the same. However many are not, and so we still can't avoid the need to write bespoke code & check with the corresponding datasheets for each value. To some degree, the nomenclature in the def.inc sheets helps, but in the long run it would probably have been easier to simply have planned to start from scratch each time. The dream of write once, run everywhere is fraught with disappointment. Far better to have the reward of clean, and error free code.

*!* Careful using the stack while servicing an interupt. We need to pop off the return PC address, and put back on the stack before RETI
*!* Likewise with any registers we want to preserve in subroutine.
*!* Contrary to Datasheet the interupt read of UDRn does NOT clear the RXCn flag.
