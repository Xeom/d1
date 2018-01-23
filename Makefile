d1.elf: com.c com.h cmd.c cmd.h pwm.c pwm.h
	avr-gcc -lm -mmcu=atmega644p -DF_CPU=12000000 -Wpedantic -Wall -Wextra -Os com.c cmd.c pwm.c -o $@
d1.hex: d1.elf
	avr-objcopy -O ihex $< $@
install: d1.hex
	avrdude -c usbasp -p m644p -U flash:w:d1.hex
