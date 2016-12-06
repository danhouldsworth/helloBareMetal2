==> Set up a vanilla REGISTER --> LED bank
--> Must avoid any SREG flag change
--> Must avoid any STACK usage (rcall etc)
--> No datasheet definitions (in case sets registers)

What are the power on initialised values of :
1. DDRx, PORTx
	- DDRx 	= 0x00
	- PORTx = 0x00 (as INPUT)
	- PORTx = 0x00 (as OUTPUT)
	- PORTx = 0xFF (as INPUT with PUR) Note: That bank alls shows dim LEDs
2. SREG
	- All clear (& validated with direct SEC, SEI, SEV etc)
3. SP (SPH, SPL)
	- 0x40, 0xFF which sure enough is 16K x 8 and end of SRAM :-)
	- Sure enough, if we include a [RCALL start] at the beginning SP=0x40FD. (after the 16bit PC is pushed onto the top down stack)
	- Whereas [PUSH] as expected has SP=040FE after an 8bit reg has been pushed.
6. SRAM location. Before and after stack push
	- Can correctly see data getting pushed through stack.
	- Can correctly see PC value getting pushed onto the stack with RCALL
	- Nice that we can see 1st hand the pre-increment of the POP command
	- Can write to named registers. Cannot hold values in reserved registers. Reserved registers often move value after another register write.
	- User memory gets overwritten by stack and vice versa.
	- Get weird behaviour as SP < 0x0100. In both SP value as well as contents.
4. All fuses & lock bits
				[LPM only][SPMCSR] 		[datasheet]
	- 0x0000 LF 01000000  11100010 [xe2 correct!]
	- 0x0001 LB 00000111  11111111 [xff correct!]
	- 0x0002 EF 10111111  11111111 [xff correct!]
	- 0x0003 HF 00001101  11011001 [xd9 correct!]
5. Signiture bytes
	- 0x0000 #1 01000000  00011110 [x1e correct!]
	- 0x0002 #2 00001101  10010111 [x97 correct!]
	- 0x0004 #3 00000010  00000101 [x05 correct!]
5. Various program locations. Validate it against the opcodes.
	- NOP 		=== 0000 0000  0000 0000 [correct!]
	- RET 		=== 1001 0101  0000 1000 [correct!]
	- POP R17	=== 1001 000d  dddd 1111 --> 1001 0001 0001 1111 [Correct!]
	- OUT PORTD === 1011 1AAr  rrrr AAAA --> 1011 1001 0001 1011 [Correct!]
7. Various comms registers :
	- UCSRnA [Datasheet says initialised with UDREn (bit 5) set] 	- Correct for both USARTs
	- UCSRnB [all cleared] 											- Correct for both USARTs
	- UCSRnC [bits 1 & 2 set] 										- Correct for both USARTs
	- TWSR [bits 3-7 set] 			- Correct
	- TWDR [bits 0-7 set]			- Correct
	- TWAR [bits 1-7 set]			- Correct
	- SPMCSR 0
