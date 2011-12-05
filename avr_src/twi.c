// Based on AVR315, modified to allow for Write then Read transactions

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "twi.h"

static unsigned char TWI_address;
static volatile unsigned char TWI_buf[ TWI_BUFFER_SIZE ];    	// Transceiver buffer (both in and out)
static unsigned char TWI_out_msgSize;                  			// Number of bytes to be transmitted.
static unsigned char TWI_in_msgSize;							// Number of bytes expected to return.

// init, generic to all TWI enabled avrs
// Remember to enable gloabal interrupts 
void twi_init(void){
	
	TWBR = (F_CPU / F_TWI - 16) / 2;               // Set bit rate register
	TWSR = 0x00;                                   // Set TWI prescaler to none.  
	//The TWI prescaler is awkwadly encoded in the status register, ATMEL reccommends not using it in case someone clears it along with the status
	TWDR = 0xFF;                                   // Default content = SDA released.
	TWCR = (1<<TWEN);                              // Enable TWI-interface and release TWI pins.

}

/// Small functions ///

//Block until transmission is finished (return immediatly if no transmission in progress)
void twi_wait(void){ while( ( TWCR & (1<<TWIE) )); }

//Start the transaction, Global interrupt must be enabled
void twi_begin(void){  TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA); }	// Send start bit (The subsequent interrupts will take us through the state machine)

//Fills in some header values
void twi_setup_packet(unsigned char address, unsigned char write_length, unsigned char read_length) {

	twi_wait();							// Don't overwrite an in progress transfer
	TWI_address = address;				// Slave address, LSB should be 0
	TWI_out_msgSize = write_length;		// Number of data to transmit from TWI_buf
	TWI_in_msgSize = read_length;		// Number of data to acquire into TWI_buf

}

/// User functions ///

// Send one byte, block until finished, assume everything went according to plan
void twi_write_byte(unsigned char address, unsigned char data){
	
	twi_setup_packet(address, 1, 0);
	TWI_buf[0] = data;  
	twi_begin();
	twi_wait();
	
}

// Send (length) bytes from (data), block until finished, assume everything went according to plan
void twi_write_block(unsigned char address, unsigned char *data, unsigned char length ){

	unsigned char i;

	twi_setup_packet(address, length, 0);
	for(i=0;i<length;i++){ TWI_buf[i]=data[i]; }
	twi_begin();
	twi_wait();

}

//Ask for a byte, block until transaction is finished, assume success, return the first element in the buffer
/* NOTE: We are assuming success, this may not be true. 
 * While we could actually monitor the status of the TWI transaction and react accordingly,
 * but it might be easier to handle such things on a higher level
 * TWI failures are usually the result poor usuage of the bus at the highest level */
unsigned char twi_read_byte(unsigned char address){

	twi_setup_packet(address, 0, 1);
	twi_begin();
	twi_wait();
	return TWI_buf[0];

}

//Ask for (length) bytes, block until transaction is finished, assume success, copy buffer to (data)
void twi_read_block(unsigned char address, unsigned char *data, unsigned char length ){

	unsigned char i;

	twi_setup_packet(address, 0, length);
	twi_begin();
	twi_wait();
	for(i=0;i<length;i++){ data[i]=TWI_buf[i]; }

}

///Writing and Reading///
//This is a special feature of SMBus devices and uses the restart bit to merge two commands

//Write a byte, then ask for a byte, block until transaction is finished, assume success, return the first element in the buffer
//The TWI_buf holds the outbound data, and then is overwritten with the inbound data
unsigned char twi_write_then_read_byte(unsigned char address, unsigned char data){

	twi_setup_packet(address, 1, 1);
	TWI_buf[0] = data;  		
	twi_begin();
	twi_wait();
	return TWI_buf[0];		// hmm: volatile, I always seem to make this mistake

}

//Write (write_length) bytes from (write_data), then ask for (read_length) bytes, block until transaction is finished,
// assume success,  copy buffer to (read_data)
void twi_write_then_read_block(unsigned char address, unsigned char *write_data, unsigned char write_length, unsigned char *read_data, unsigned char read_length){

	unsigned char i;

	twi_setup_packet(address, write_length, read_length);
	for(i=0;i<write_length;i++){ TWI_buf[i]=write_data[i]; }
	twi_begin();
	twi_wait();
	for(i=0;i<read_length;i++){ read_data[i]=TWI_buf[i]; }

}

/// Interrupts ///

ISR(TWI_vect){									// The main state machine for the I2C transaction is here

	static unsigned char TWI_bufPtr;			// To keep track of where we are in the message (used for both in and out data)

	switch (TWSR){										// Our actions are based on the TWI status register, which is updated by the TWI controller
	
		case TW_START:										// START bit has been transmitted
			if(TWI_out_msgSize>0){								// Are we writing data first?
				TWDR = TWI_address | TW_WRITE;						// SLA + W
			}else{												//
				TWDR = TWI_address | TW_READ;						// SLA + R
			}													//
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);				// Send the byte
			TWI_bufPtr = 0;										// Reset the buffer pointer
			break;											//
			
		case TW_REP_START:									// Repeated START has been transmitted, switch to reading
			TWDR = TWI_address | TW_READ;						// SLA + R
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);				// Send the byte 
			TWI_bufPtr = 0;										// Reset the buffer pointer
			break;											//
			
		case TW_MT_SLA_ACK:									// SLA+W has been tramsmitted and ACK received, fallthru
		case TW_MT_DATA_ACK:								// Data byte has been tramsmitted and ACK received
			if(TWI_bufPtr < TWI_out_msgSize){					// Are we not at the end of the write message?
				TWDR = TWI_buf[TWI_bufPtr++];						// DATA
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);				// Send the byte
			}else{												//
				if(TWI_in_msgSize > 0){								// Will we perform a subsequent read, after sending the last byte?
					TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);	// Send REPEATED START
				}else{												//
					TWCR =(1<<TWEN)|(1<<TWINT)|(1<<TWSTO);				// Send STOP
				}													//
			}													//
			break;											//
			
		case TW_MR_DATA_ACK:								// Data byte has been received and ACK tramsmitted
			TWI_buf[TWI_bufPtr++] = TWDR;						// Read byte, fallthru
		case TW_MR_SLA_ACK:									// SLA+R has been tramsmitted and ACK received
			if (TWI_bufPtr < (TWI_in_msgSize-1) ){				// Detect the last byte
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);	// NACK last byte
			}else{												//
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);				// ACK all others
			}													//
			break;											//
			
		case TW_MR_DATA_NACK:								// Data byte has been received and NACK tramsmitted
			TWI_buf[TWI_bufPtr] = TWDR;							// Read last byte
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);;			// Initiate a STOP condition.
			break;											//
			
		case TW_MR_ARB_LOST:								// Arbitration lost
		case TW_MT_SLA_NACK:								// SLA+W has been tramsmitted and NACK received
		case TW_MR_SLA_NACK:								// SLA+R has been tramsmitted and NACK received    
		case TW_MT_DATA_NACK:								// Data byte has been tramsmitted and NACK received
		case TW_NO_INFO:									// No State, TWIE=0; Wait... how did we even get here?
		case TW_BUS_ERROR:									// Bus error due to an illegal START or STOP condition
		default:											//
			TWCR = (1<<TWEN);									// Reset TWI Interface and release TWI pins.
	}
	
}


