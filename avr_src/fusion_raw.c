#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usb_serial.h"
#include "twi.h"
#include "l3g4200d.h"
#include "lsm303dlm.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

unsigned short t;

int main(void){
	
	uint8_t buf[9];
	unsigned short x,y,z;

	// Turn prescaler off (is always on 1/8 at start)
	CPU_PRESCALE(1);

	TCCR1A = 0x00;
	TCCR1B = 0x04;

	PCMSK0 = 0x70;
	PCICR = (1<<PCIE0);


	// init TWI (clock/ interrupt setup), remember to enable global interrupts later
	twi_init();

	// usb init turns on global interrupts
	usb_serial_init();
	//wait until the host recognizes us
	while(!usb_serial_ready());
	
	l3g4200d_init();
	lsm303dlm_init();
	
	//TODO: timers and pcints and whatnot
//	dt = 0;
	 

	while(1){

		if(!(PINB & (1<<5))){
			lsm303dlm_a_read(&x,&y,&z);
			buf[0]='A';
			buf[2]=x;
			buf[1]=x>>8;
			buf[4]=y;
			buf[3]=y>>8;
			buf[6]=z;
			buf[5]=z>>8;
			buf[8]=t;
			buf[7]=t>>8;
			usb_serial_tx(buf, 9);
		}
		
		if(!(PINB & (1<<4))){
			lsm303dlm_m_read(&x,&y,&z);
			buf[0]='M';
			buf[2]=x;
			buf[1]=x>>8;
			buf[4]=y;
			buf[3]=y>>8;
			buf[6]=z;
			buf[5]=z>>8;
			buf[8]=t;
			buf[7]=t>>8;
			usb_serial_tx(buf, 9);
		}
		
		if((PINB & (1<<6))){
			l3g4200d_read(&x,&y,&z);
			buf[0]='G';
			buf[2]=x;
			buf[1]=x>>8;
			buf[4]=y;
			buf[3]=y>>8;
			buf[6]=z;
			buf[5]=z>>8;
			buf[8]=t;
			buf[7]=t>>8;
			usb_serial_tx(buf, 9);
		}

	}
	
	return 0;

}

ISR(PCINT0_vect){
	t=TCNT1;
}
