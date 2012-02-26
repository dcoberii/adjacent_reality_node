#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usb_serial.h"
#include "twi.h"
#include "l3g4200d.h"
#include "lsm303dlm.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

volatile uint64_t tm, tg, ta, tovr;
uint8_t last_pinb;

void send(char c, unsigned short x,unsigned short y,unsigned short z, uint64_t t){
	
	uint8_t buf[16];
	buf[0]='D';
	buf[1]=c;
	
	buf[2]=(x>>8);
	buf[3]=(x & 0xFF);
	buf[4]=(y>>8);
	buf[5]=(y & 0xFF);
	buf[6]=(z>>8);
	buf[7]=(z & 0xFF);
	cli();
	buf[8]=(t>>56);
	buf[9]=(t>>48) & 0xFF;
	buf[10]=(t>>40) & 0xFF;
	buf[11]=(t>>32) & 0xFF;
	buf[12]=(t>>24) & 0xFF;
	buf[13]=(t>>16) & 0xFF;
	buf[14]=(t>>8) & 0xFF;
	buf[15]=(t & 0xFF);
	sei();
	
	/*
	buf[2]=(x>>12) + '0';
	buf[3]=((x>>8) & 0xF) + '0' ;
	buf[4]=((x>>4) & 0xF) + '0' ;
	buf[5]=((x) & 0xF) + '0' ;
	
	buf[6]=(y>>12) + '0';
	buf[7]=((y>>8) & 0xF) + '0' ;
	buf[8]=((y>>4) & 0xF) + '0' ;
	buf[9]=((y) & 0xF) + '0' ;

	buf[10]=(z>>12) + '0';
	buf[11]=((z>>8) & 0xF) + '0' ;
	buf[12]=((z>>4) & 0xF) + '0' ;
	buf[13]=((z) & 0xF) + '0' ;

	
	cli();

	buf[14]=(t>>12) + '0';
	buf[15]=((t>>8) & 0xF) + '0' ;
	buf[16]=((t>>4) & 0xF) + '0' ;
	buf[17]=((t) & 0xF) + '0' ;

	sei();
	buf[18]='\n' ;
	*/
	
	usb_serial_tx(buf, 16);
	
}

int main(void){
	
	unsigned short x,y,z;

	// Turn prescaler off (is always on 1/8 at start)
	CPU_PRESCALE(1);

	tovr =0;
	TCCR1A = 0x00;
	TCCR1B = 0x04;
	TIMSK1 = (1<<TOIE1);

	

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
	 

	while(1){

		if(!(PINB & (1<<5))){
			lsm303dlm_a_read(&x,&y,&z);
			send('A', x, y, z, ta);
		}
		
		if(!(PINB & (1<<4))){
			lsm303dlm_m_read(&x,&y,&z);
			send('M', x, y, z, tm);
		}
	
		if((PINB & (1<<6))){
			l3g4200d_read(&x,&y,&z);
			send('G', x, y, z, tg);
		}

	}
	
	return 0;

}

ISR(TIMER1_OVF_vect){tovr+=0x10000;}


ISR(PCINT0_vect){
	cli();
	uint8_t tmp_pinb = PINB;
	if(((last_pinb ^ tmp_pinb) && (1<<6)) && (tmp_pinb & (1<<6))){	tg=tovr + TCNT1;}
	if(((last_pinb ^ tmp_pinb) && (1<<5)) && !(tmp_pinb & (1<<5))){	ta=tovr + TCNT1;}
	if(((last_pinb ^ tmp_pinb) && (1<<4)) && !(tmp_pinb & (1<<4))){	tm=tovr + TCNT1;}
	last_pinb = tmp_pinb;
	sei();
}
