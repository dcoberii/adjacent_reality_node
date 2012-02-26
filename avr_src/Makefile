F_CPU = 8000000
MCU = atmega32u4

TARGET = fusion_raw
SRC =	usb_core.c \
		usb_serial.c \
		twi.c \
		l3g4200d.c \
		lsm303dlm.c
	
OBJ := $(SRC:%.c=%.o)
DEP := $(SRC:%.c=%.d)

CC = avr-gcc
CDEFS = -DF_CPU=$(F_CPU)UL
CFLAGS = -Os -Wall -mmcu=$(MCU)


.PHONY: all
all: $(TARGET).hex

%.hex: %.elf
	avr-objcopy -O ihex -R .fuse -R .lock -R .eeprom -R .signature $< $@
%.elf: $(OBJ) %.o	
	$(CC) $(CFLAGS) $(CDEFS) $^ -o $@
%.o : %.c
	$(CC) $(CFLAGS) $(CDEFS) -c $< -o $@ 
%.d : %.c %.o
	$(CC) $(CFLAGS) $(CDEFS) -MM -MG $< > $@
	 
-include $(DEP)

.PHONY: install
install: $(TARGET).hex
	-dfu-programmer $(MCU) erase
	dfu-programmer $(MCU) flash $(TARGET).hex

.PHONY: clean
clean:
	-rm -f *.d *.o *.hex
