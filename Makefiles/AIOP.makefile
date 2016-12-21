#NOTE - could only get it working (flashing) by disabling the erase cycle with -D
AVRDUDE = avrdude -D -p ATmega2560 -c stk500v2 -P /dev/tty.usbserial-A502D20H

all : build

build:
	avr-gcc 	-mmcu=atmega2560 -Wall -DF_CPU=16000000 	main.c -o 	main.elf
	avr-objcopy -O ihex 									main.elf 	main.hex

asm:
	avra -fI 												main.asm

read:
	@$(AVRDUDE) -b 115200									-U efuse:r:-:b 	-U hfuse:r:-:b 	-U lfuse:r:-:b

flash:
	@$(AVRDUDE) -b 115200 									-U flash:w:main.hex:i

clean:
	@rm -f *.elf
	@rm -f *.hex
	@rm -f *.cof

