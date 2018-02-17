all: 
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o main.c
	avr-gcc -mmcu=atmega328p main.o -o main
	avr-objcopy -O ihex -R .eeprom main main.hex

flash: 
	avrdude -F -V -e -c arduino -p ATMEGA328P -P /dev/cu.usbmodemFA131 -b 115200 -Uflash:w:main.hex

clean:
		-rm -rf main.hex
		-rm -rf main.o

super:
	-rm -rf main.hex
	-rm -rf main.o
	avrdude -e -c arduino -p ATMEGA328P -P /dev/cu.usbmodemFD121
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o main.c
	avr-gcc -mmcu=atmega328p main.o -o main
	avr-objcopy -O ihex -R .eeprom main main.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/cu.usbmodemFD121 -b 115200 -Uflash:w:main.hex
	
erase:
	avrdude -e -c arduino -p ATMEGA328P -P /dev/cu.usbmodemFA131 