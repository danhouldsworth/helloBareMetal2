# 1. Short reset - expect green pulse
# 2. ls /dev/tty.usbmodem* - note device
# 3. update Makefile
# 4. Short reset then flash
AVRDUDE = avrdude -p ATmega32u4 -c avr109 -P /dev/tty.usbmodem41

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

