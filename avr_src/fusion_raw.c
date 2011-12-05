#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usb_hid.h"
#include "twi.h"
#include "l3g4200d.h"
#include "lsm303dlm.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

//For the hid type:
enum {ACCEL, GYRO, MAG};

int main(void){

	unsigned short dt,x, y, z;

	// Turn prescaler off (is always on 1/8 at start)
	CPU_PRESCALE(0);

	// init TWI (clock/ interrupt setup), remember to enable global interrupts later
	twi_init();

	// usb init turns on global interrupts
	usb_init();
	//wait until the host recognizes us
	while(!usb_configured());
	
	l3g4200d_init();
	lsm303dlm_init();
	
	//TODO: timers and pcints and whatnot
	dt = 0;
	 
	while(1){
		
		if(lsm303dlm_a_drdy()){
			lsm303dlm_a_read(&x,&y,&z);
			usb_hid_send_report(ACCEL, dt, x, y, z);
		}
		if(lsm303dlm_m_drdy()){
			lsm303dlm_m_read(&x,&y,&z);
			usb_hid_send_report(MAG, dt, x, y, z);
		}
		if(l3g4200d_drdy()){
			l3g4200d_read(&x,&y,&z);
			usb_hid_send_report(GYRO, dt, x, y, z);
		}
			
	}
	
	return 0;

}
