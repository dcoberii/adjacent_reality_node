#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usb_serial.h"
#include "twi.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

int main(void){

	CPU_PRESCALE(1);
	twi_init();
	usb_serial_init();
	while(!usb_serial_ready());
	
	uint8_t i;
	
	while(1){
		_delay_ms(1000);
		usb_serial_tx("\n\nSelfTest\n",11);
		if (twi_write_then_read_byte(0xD0, 0x0F) == 0xD4){usb_serial_tx("Found Gyro on  0xD0\n", 20);}
		if (twi_write_then_read_byte(0xD2, 0x0F) == 0xD4){usb_serial_tx("Found Gyro on  0xD2\n", 20);}
		if (twi_write_then_read_byte(0x3C, 0x0F) == 0x3C){usb_serial_tx("Found Mag on   0x3C\n", 20);} 
		if (twi_write_then_read_byte(0x30, 0x20) == 0x37){usb_serial_tx("Found Accel on 0x30\n", 20);} 
		if (twi_write_then_read_byte(0x32, 0x20) == 0x37){usb_serial_tx("Found Accel on 0x32\n", 20);}
	}
}
