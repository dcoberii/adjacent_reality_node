// Based on USB Debug Channel Example from http://www.pjrc.com/teensy/

// All of the avr usb stuff is rather tiresome
// I think a better approach would be to use auto code generation for this

#include <avr/interrupt.h>
#include <avr/io.h>
#include "usb_core.h"

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void){ 	while (!(UEINTX & (1<<TXINI))) ; }
static inline void usb_send_in(void){	UEINTX = ~(1<<TXINI); }
static inline void usb_wait_receive_out(void){	while (!(UEINTX & (1<<RXOUTI))) ; }
static inline void usb_ack_out(void){	UEINTX = ~(1<<RXOUTI);}

// Local persistant:
// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;
uint8_t (*unhandled_endpoint_interrupt_callback)(uint8_t);
uint8_t (*unhandled_setup_packet_callback)(uint8_t,uint8_t,uint16_t,uint16_t,uint16_t);
uint8_t (*get_descriptor_callback)(uint16_t,uint16_t,uint16_t);
uint8_t (*set_configuration_callback)(uint16_t);

//Getters/Setters:
uint8_t get_usb_configuration(void){ return usb_configuration; }
void set_unhandled_endpoint_interrupt_callback(uint8_t (*func)(uint8_t)){unhandled_endpoint_interrupt_callback=func;}
void set_unhandled_setup_packet_callback(uint8_t (*func)(uint8_t,uint8_t,uint16_t,uint16_t,uint16_t)){unhandled_setup_packet_callback=func;}
void set_get_descriptor_callback(uint8_t (*func)(uint16_t,uint16_t,uint16_t)){get_descriptor_callback=func;}
void set_set_configuration_callback(uint8_t (*func)(uint16_t)){set_configuration_callback=func;}

//device specific initialization
void usb_init(void){
	
	#if (defined (__AVR_AT90USB162__) && F_CPU == 8000000UL)
		USBCON = ((1<<USBE)|(1<<FRZCLK));	//enable USB but turn off the clock
		PLLCSR = ((1<<PLLE));				//turn on the PLL (no prescaler for 8MHz system)
		while (!(PLLCSR & (1<<PLOCK))) ;	//wait for PLL lock
		USBCON = (1<<USBE);					//turn on the usb clock
		
	#elif (defined (__AVR_ATmega32U4__) && F_CPU == 16000000UL)
		UHWCON = (1<<UVREGE);				//turn on VREG
		USBCON = ((1<<USBE)|(1<<FRZCLK));	//enable USB but turn off the clock
		PLLFRQ = (1<<PDIV2);				//set the prescale = 1/2
		PLLCSR = ((1<<PINDIV)|(1<PLLE));	//turn on the PLL and prescaler
		while (!(PLLCSR & (1<<PLOCK))) ;	//wait for PLL lock
		USBCON = ((1<<USBE)|(1<<OTGPADE));	//turn on the usb clock and enter device mode
		
	#elif (defined (__AVR_ATmega32U4__) && F_CPU == 8000000UL)
		UHWCON = (1<<UVREGE);				//turn on VREG
		USBCON = ((1<<USBE)|(1<<FRZCLK));	//enable USB but turn off the clock
		PLLCSR = 0x12;//((1<<PINDIV)|(1<PLLE));		//turn on the PLL (no prescaler for 8MHz system) ???
		while (!(PLLCSR & (1<<PLOCK))) ;	//wait for PLL lock
		USBCON = ((1<<USBE)|(1<<OTGPADE));	//turn on the usb clock and enter device mode
		
	#else
		#error "Particular MCU and Clock not yet supported"
	#endif

	UDCON = (0<<DETACH);				//enable attach resistor
	UDIEN = (1<<EORSTE);				//enable USB_GEN_vect End-Of-Reset interrupt	
	sei();								//enable global interrupts

}

// USB Device Interrupts
ISR(USB_GEN_vect){

	//Check for End-Of-Reset
	if (UDINT & (1<<EORSTI)) {
		//Once the device comes out of reset, this event is triggered
		//All we need to do here is configure the control endpoint

		UDINT = 0;							//Clear interrupt
		
		UENUM = 0;							//Select Endpoint 0
		UECONX = (1<<EPEN);					//Enable selected endpoint
		UECFG0X = 0;						//Set selected endpoint to CONTROL type, OUT direction
		UECFG1X = (1<<EPSIZE1)|(1<<ALLOC);	//Endpoint is 32 bytes, alloc single buffer				//FIXME violates once and only once, see descriptor
		UEIENX = (1<<RXSTPE);				//Enable Setup recieved interrupt
		
		usb_configuration = 0;				
	}
	
}

// USB Endpoint Interrupts
ISR(USB_COM_vect){
	
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;

	//Normally we would want to ask the AVR which endpoint triggered the vect, but
	//our only endpoint that has interrupts enabled is the control endpoint
	UENUM = 0;	//Select control endpoint (endpoint 0) for subsequent operations
	
	//Check that we have recieved a setup packet on endpoint 0:
	if ( (UEINTX & (1<<RXSTPI)) && UENUM == 0) {

		//Read the packet:
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);

		UEINTX = 0;			//Clear the Interrupt (empties the buffer to ready for next packet)

		if(bmRequestType & DEVICE_TO_HOST){
					
			switch(bRequest){
			
				case GET_DESCRIPTOR: 	//USB 2.0 spec 9.4.3, page 253
					// Returns a requested descriptor (device, config, string, etc)
					// The device descriptor is requested first, it points to the config descriptors and so on						
					if(get_descriptor_callback(wValue, wIndex, wLength)){ return; }		//call to non-core
					break;
			
				case GET_CONFIGURATION:	//USB 2.0 spec 9.4.2, page 253
					//Returns a single byte that lists the configuration in use
					usb_wait_in_ready();
					UEDATX = usb_configuration;
					usb_send_in();		
					return;
				
				case GET_STATUS:		//USB 2.0 spec 9.4.5, page 254
					// Returns status of Endpoint/Configuration/Device in 2 bytes
					usb_wait_in_ready();
					if(bmRequestType & DEVICE_REQUEST){
						UEDATX = 0;		//Not self-powered, not accepting remote wakeup
						UEDATX = 0;		//RESERVED
					}
					if(bmRequestType & INTERFACE_REQUEST){
						UEDATX = 0;		//RESERVED
						UEDATX = 0;		//RESERVED
					}
					if(bmRequestType & ENDPOINT_REQUEST){
						UENUM = wIndex;					//Switch to interrupt
						if (UECONX & (1<<STALLRQ)){		//Check Halt staus
							UENUM = 0;					//Switch back to control endpoint
							UEDATX = 1;					//Return 1 if halted
						}else{
							UENUM = 0;					//Switch back to control endpoint
							UEDATX = 0;					//Return 0 if not halted
						}
						UEDATX = 0;		//RESERVED
					}
					usb_send_in();
					return;
						
			}
		}else{
					
			switch(bRequest){
				
				case SET_ADDRESS:		//USB 2.0 spec 9.4.6, page 256
					//Sets the USB address (0-255 per bus), is 0 by default (general call address)
					//Usually the first packet we recieve	
					usb_send_in();					//Send an empty packet to ACK as status stage
					usb_wait_in_ready();			//Wait for transaction to complete
					UDADDR = wValue | (1<<ADDEN);	//Set and enable address (only request to take effect after transaction)
					return;
					
				case SET_CONFIGURATION:	//USB 2.0 spec 9.4.7, page 257
					//Selects a configuration from the list returned by the get descriptor (almost aways #1)
					usb_configuration=set_configuration_callback(wValue);
					if(usb_configuration){ return; } //call to non-core
					break;			
					
				case CLEAR_FEATURE:		//USB 2.0 spec 9.4.1, page 252
					if((bmRequestType & ENDPOINT_REQUEST) && wValue == ENDPOINT_HALT){
							usb_send_in();								//Send an empty packet to ACK as status stage
							UENUM = wIndex;								//Switch to requested endpoint
							UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);//Unhalt
							UERST = (1 << wIndex);						//Reset FIFO
							UERST = 0;
							return;
					}
					break;
				case SET_FEATURE:		//USB 2.0 spec 9.4.9, page 258
					if((bmRequestType & ENDPOINT_REQUEST) && wValue == ENDPOINT_HALT){
							usb_send_in();					//Send an empty packet to ACK as status stage
							UENUM = wIndex;					//Switch to requested endpoint
							UECONX = (1<<STALLRQ)|(1<<EPEN);//Halt
							return;
					}
					break;
			}
		}

		//Fall through to non-core function
		if(unhandled_setup_packet_callback(bmRequestType, bRequest, wValue, wIndex, wLength)){ 
			return; 
		}
	}

	//Fall through to non-core function
	if(unhandled_endpoint_interrupt_callback(UENUM)){ 
		return; 
	}	
	
	//If we completely fell through then signal a problem by stalling the bus
	UECONX = (1<<STALLRQ) | (1<<EPEN);
	
}


