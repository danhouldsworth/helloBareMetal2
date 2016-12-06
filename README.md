# helloBareMetal2
A rewrite based on coming back in cold after 18months

This will be written out from scratch, long hand to ensure concepts understood.

I'll modify the original layout where it feels a better approach would make revision better.

==> Read the data sheet for the MCUs ATmega328, ATmega32u4, ATmega8, ATtiny
==> Ensure version with active Contents tab to navigate :-)
==> Peruse the pin out, peripherals, special features, memory map, register summary, instruction set.

Questions
1. What are the differences - specific and general.
2. Which applications are they used in and why?
3. What / where is the Stack?
4. What are the differences between PROGMEM, SRAM, EEPROM?
5. Does the memory map duplicate the 32 GPR or 64 I/O registers? Does this effect how C or assembly addresses hard coded addresses?

AT48/88/168/328p Family
* 4		/ 	8 	/	16	/	32K Program memory
* 0.5 	/ 	1 	/ 	1 	/ 	2K 	SRAM
* 0.25 	/ 	0.5 / 	0.5 / 	1K 	EEPROM
* 10,000 / 100,000 Flash / EEPROM write / erase cycles
* 2x 8-bit timer
* 1x 16-bit timer
* 8 channel 10-bit ADC (!* only 6 channel PDIP *!)
* USART
* SPI (master / slave)
* I2C
* Watchdog
* Internal calibrated oscillator
* 23 I/O lines
* 20MHz @ 1.8-5.5v
* Low consumption at 1MHz @ 1.8v

Fraught with frustrations :
- If don't wire both power rails, then will forget at the worst time, compounding confusion.
- A PDIP chip plugged into breadboard will only work when FULLY SEATED (adult strength not 8 year old)
- Label chips so don't confuse settings.
- Use LEDs that know work and not blown, and if something not working don't overlook a working LED may have since blown.
- Interupts can be set at boot if not cleared(!)
- LEDs can cause brown out reset!

Header
	.DEVICE			Defines the type of the target processor and the applicable set of instructions (illegal instructions for that type trigger an error message, syntax: .DEVICE AT90S8515)
	.DEF			Defines a synonym for a register (e.g. .DEF MyReg = R16)
	.EQU			Defines a symbol and sets its value (later changes of this value remain possible, syntax: .EQU test = 1234567, internal storage of the value is 4-byte- Integer)
	.SET			Fixes the value of a symbole (later redefinition is not possible)
	.INCLUDE		Includes a file and assembles its content, just like its content would be part of the calling file (typical e.g. including the header file: .INCLUDE "C:\avrtools\appnotes\8515def.inc")
Code
	.CSEG			Start of the code segment (all that follows is assembled to the code segment and will go to the program space)
	.DB				Inserts one or more constant bytes in the code segment (could be numbers from 0..255, an ASCII-character like 'c', a string like 'abcde' or a combination like 1,2,3,'abc'. The number of inserted bytes must be even, otherwise an additional zero byte will be inserted by the assembler.)
	.DW				Insert a binary word in the code segment (e.g. produces a table within the code)
	.LISTMAC		Macros will be listed in the .LST-file. (Default is that macros are not listed)
	.MACRO			Beginning of a macro (no code will be produced, call of the macro later produces code, syntax: .MACRO macroname parameters, calling by: macroname parameters)
	.ENDMACRO		End of the macro
EEPROM
	.ESEG			Assemble to the EEPROM-segment (the code produced will go to the EEPROM section, the code produces an .EEP-file)
	.DB				Inserts one or more constant bytes in the EEPROM segment (could be numbers from 0..255, an ASCII-character like 'c', a string like 'abcde' or a combination like 1,2,3,'abc'.)
	.DW				Inserts a binary word to the EEPROM segment (the lower byte goes to the next adress, the higher byte follows on the incremented address)
SRAM
	.DSEG			Assemble to the data segment (here only BYTE directives and labels are valid, during assembly only the labels are used)
	.BYTE			Reserves one or more bytes space in the data segment (only used to produce correct labels, does not insert any values!)
Everywhere
	.ORG			Defines the address within the respective segment, where the assembler assembles to (e.g. .ORG 0x0000)
	.LIST			Switches the listing to the .LST-file on (the assembled code will be listet in a readable text file .LST)
	.NOLIST			Switches the output to the .LST-file off, suppresses listing.
	.INCLUDE		Inserts the content of another source code file, as if its content would be part of the source file (typical e.g. including the header file: .INCLUDE "C:\avrtools\appnotes\8515def.inc")
	.EXIT 			End of the assembler-source code (stops the assembling process)


