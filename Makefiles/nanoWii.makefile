AVRDUDE = avrdude -p ATmega32u4 -c avr109 -P /dev/tty.usbmodem11

all : build

build:
	avr-gcc 	-mmcu=atmega32u4 -Wall -DF_CPU=16000000 	main.c -o 	main.elf
	avr-objcopy -O ihex 									main.elf 	main.hex

asm:
	avra -fI 												main.asm

read:
	@$(AVRDUDE) -b 57600									-U efuse:r:-:b 	-U hfuse:r:-:b 	-U lfuse:r:-:b

flash:
	@$(AVRDUDE) -b 57600 									-U flash:w:main.hex:i

clean:
	@rm -f *.elf
	@rm -f *.hex
	@rm -f *.cof

