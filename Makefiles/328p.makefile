# AVRDUDE = avrdude -patmega328 -c usbasp -P usb -B 4
AVRDUDE = avrdude -patmega328p -c usbasp -P usb

all: build
build:
	@avr-gcc 		-mmcu=atmega328p -Wall -DF_CPU=16000000 	main.c -o 	main.elf
	@avr-objcopy 	-O ihex 									main.elf 	main.hex
	@$(AVRDUDE) -B 4												-U flash:w:main.hex:i

asm:
	avra -fI 													main.asm
	@$(AVRDUDE) -B 4												-U flash:w:main.hex:i

read:
	@$(AVRDUDE) -B 4											-U efuse:r:-:b 	-U hfuse:r:-:b 	-U lfuse:r:-:b

setfuse_fullswing:
	@$(AVRDUDE) -B 4											-U lfuse:w:0xf7:m

setfuse_8MHz:
	@$(AVRDUDE) -B 4											-U lfuse:w:0xe2:m

setfuse_1MHz:
	#
	# WARNING : Need to update Makefile with AVRDUDE -B 4
	#
	@$(AVRDUDE) 												-U lfuse:w:0x62:m

clean:
	@rm -f *.elf
	@rm -f *.hex
	@rm -f *.cof

# Extended / High / Low (eg Section 27.2 in Datasheet for ATm328)
# eg http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=62&HIGH=D9&EXTENDED=FF&LOCKBIT=FF

