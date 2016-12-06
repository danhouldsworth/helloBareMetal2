AVRDUDE = avrdude -patmega328p -c usbasp -P usb

all: build
build:
	@avr-gcc 		-mmcu=atmega328p -Wall -DF_CPU=16000000 	main.c -o 	main.elf
	@avr-objcopy 	-O ihex 									main.elf 	main.hex
	@$(AVRDUDE) 												-U flash:w:main.hex:i

asm:
	avra -fI 													main.asm
	@$(AVRDUDE) 												-U flash:w:main.hex:i

read:
	@$(AVRDUDE) 												-U efuse:r:-:b 	-U hfuse:r:-:b 	-U lfuse:r:-:b

clean:
	@rm -f *.elf
	@rm -f *.hex
	@rm -f *.cof

