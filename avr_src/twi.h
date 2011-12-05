//Based on AVR315

#ifndef TWI_H
#define TWI_H

#define TWI_BUFFER_SIZE 32   // Set this to the largest message size that will be sent including address byte. (32 is the max for SMBUS by default)
#define F_TWI 	400000UL    //400kHz

void twi_init(void);

unsigned char twi_read_byte(unsigned char address);
void twi_write_byte(unsigned char address, unsigned char data);
unsigned char twi_write_then_read_byte(unsigned char address, unsigned char data);

void twi_read_block(unsigned char address, unsigned char *data, unsigned char length );
void twi_write_block(unsigned char address, unsigned char *data, unsigned char length );
void twi_write_then_read_block(unsigned char address, unsigned char *write_data, unsigned char write_length, unsigned char *read_data, unsigned char read_length);

#endif

