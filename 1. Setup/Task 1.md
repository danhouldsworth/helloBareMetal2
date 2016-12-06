1. Set up hardware, & build / flash tool chains :
* ATmega328p evaluation board
* ATmega32u4 evaluation board
* Atmega328 PDIP with internal osciallator @ 8MHz
* Atmega328 PDIP with internal osciallator @ 1MHz
* Atmega1284 PDIP with external osciallator @ 16MHz

2. Create makefiles that can read & set fuses, and write to the flash, for C and ASM files to these chips
3. Store in root, so we don't have to keep replicating each exercise