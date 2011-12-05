// Based on USB Debug Channel Example from http://www.pjrc.com/teensy/

//TODO: This is all still rather grotty and will need to be refactored a few more times
// All of the avr usb stuff is rather tiresome
// I think a better approach would be to use auto code generation for this

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include "usb_hid.h"

static uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x02,			// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	32,					// bMaxPacketSize0
	0xC0,0x16,			// idVendor
	0x79,0x04,			// idProduct
	0x00, 0x01,			// bcdDevice
	1,					// iManufacturer
	2,					// iProduct	
	0,					// TODO: iSerialNumber
	1					// bNumConfigurations
};

static uint8_t PROGMEM hid_report_descriptor[] = {
	0x06, 0x31, 0xFF,		// Usage Page 0xFF31 (vendor defined)
	0x09, 0x74,				// Usage 0x74
	0xA1, 0x53,				// Collection 0x53
	0x75, 0x08,				// report size = 8 bits
	0x15, 0x00,				// logical minimum = 0
	0x26, 0xFF, 0x00,		// logical maximum = 255
	0x95, 9,				// report count
	0x09, 0x75,				// usage
	0x81, 0x02,				// Input (array)
	0xC0					// end collection
};

#define CONFIG1_DESC_SIZE (9+9+9+7)
#define HID_DESC2_OFFSET  (9+9)

static uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	34,0,				// wTotalLength
	1,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0xC0,				// bmAttributes
	50,					// bMaxPower
// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	0,					// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,				// bInterfaceClass (0x03 = HID)
	0x00,				// bInterfaceSubClass
	0x00,				// bInterfaceProtocol
	0,					// iInterface
// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,				// bDescriptorType
	0x11, 0x01,			// bcdHID
	0,					// bCountryCode
	1,					// bNumDescriptors
	0x22,				// bDescriptorType
	21,					// wDescriptorLength
	0,
// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	3 | 0x80,			// bEndpointAddress
	0x03,				// bmAttributes (0x03=intr)
	32, 0,				// wMaxPacketSize
	1					// bInterval
};

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};

//Set to English
static struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};

#define STR_MANUFACTURER	L"Your Name"
static struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};

#define STR_PRODUCT 	L"Your USB Device"
static struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x2200, 0x0000, hid_report_descriptor, sizeof(hid_report_descriptor)},
	{0x2100, 0x0000, config1_descriptor+HID_DESC2_OFFSET, 9},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)}
};
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))


// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void){ 	while (!(UEINTX & (1<<TXINI))) ; }
static inline void usb_send_in(void){	UEINTX = ~(1<<TXINI); }
static inline void usb_wait_receive_out(void){	while (!(UEINTX & (1<<RXOUTI))) ; }
static inline void usb_ack_out(void){	UEINTX = ~(1<<RXOUTI);}

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;

// return 0 if the USB is not configured, or the configuration number selected by the HOST (linux always selects #1)
uint8_t usb_configured(void){ return usb_configuration; }


// transmit a report, this isn't proper yet 
char usb_hid_send_report(unsigned char type, unsigned short dt,unsigned  short x,unsigned  short y,unsigned  short z)
{
	uint8_t timeout, intr_state;

	if (!usb_configuration) return -1;
	intr_state = SREG;
	cli();

	UENUM = 3;

	// wait for the FIFO to be ready to accept data
	timeout = UDFNUML + 4;
	while (1) {
		// are we ready to transmit?
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		// have we waited too long?
		if (UDFNUML == timeout) { return -1; }
		// has the USB gone offline?
		if (!usb_configuration) { return -1; }
		// get ready to try checking again
		intr_state = SREG;
		cli();
		UENUM = 3;
	}

	// write the bytes into the FIFO
	UEDATX = type;
	UEDATX = dt >> 8;
	UEDATX = dt & 0xFF;
	UEDATX = x >> 8;
	UEDATX = x & 0xFF;
	UEDATX = y >> 8;
	UEDATX = y & 0xFF;
	UEDATX = z >> 8;
	UEDATX = z & 0xFF;
	
	// transmit it
	UEINTX = 0x3A;
	
	SREG = intr_state;
	return 0;
	
}

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
		PLLFRQ = (1<<PDIV2);				// set the prescale = 1/2
		PLLCSR = ((1<<PINDEV)|(1<PLLE));	//turn on the PLL and prescaler
		while (!(PLLCSR & (1<<PLOCK))) ;	//wait for PLL lock
		USBCON = ((1<<USBE)|(1<<OTGPADE));	//turn on the usb clock and enter device mode
	#elif (defined (__AVR_ATmega32U4__) && F_CPU == 8000000UL)
		UHWCON = (1<<UVREGE);				//turn on VREG
		USBCON = ((1<<USBE)|(1<<FRZCLK));	//enable USB but turn off the clock
		PLLCSR = (1<PLLE);					//turn on the PLL (no prescaler for 8MHz system)
		while (!(PLLCSR & (1<<PLOCK))) ;	//wait for PLL lock
		USBCON = ((1<<USBE)|(1<<OTGPADE));	//turn on the usb clock and enter device mode

	#else
		#error "Particular MCU and Clock not yet supported"
	#endif

	UDCON = (0<<DETACH);				//enable attach resistor
	
	usb_configuration = 0;				
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
		UECFG1X = (1<<EPSIZE1)|(1<<ALLOC);	//Endpoint is 32 bytes, alloc single buffer
		UEIENX = (1<<RXSTPE);				//Enable Setup recieved interrupt
		
		usb_configuration = 0;				
	}
	
}

// USB Endpoint Interrupts
ISR(USB_COM_vect){
	
	const uint8_t *list;
	uint8_t i, n, len;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;

	//Our only endpoint that has interrupts enabled is the control endpoint
	UENUM = 0;	//Select control endpoint (endpoint 0) for subsequent operations
	
	//Check that we have recieved a setup packet:
	if (UEINTX & (1<<RXSTPI)) {

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

		//Sets the USB address (0-255 per bus), is 0 by default (general call  address)
		//Usually the first packet we recieve
		if (bRequest == SET_ADDRESS) {
			usb_send_in();					//Send an empty packet to ACK
			usb_wait_in_ready();			
			UDADDR = wValue | (1<<ADDEN);	//Set and enable address
			return;
		}
		
		// Returns a requested descriptor (device, config, string, etc)
		// The device descriptor is first, it points to the config descriptors and so on
		if (bRequest == GET_DESCRIPTOR) {
			
			list = (const uint8_t *)descriptor_list;
			
			for (i=0; ; i++) {
				if (i >= NUM_DESC_LIST) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
					return;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue) {
					list += sizeof(struct descriptor_list_struct);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex) {
					list += sizeof(struct descriptor_list_struct)-2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			
			len = (wLength < 256) ? wLength : 255;
			
			if (len > desc_length) len = desc_length;
			
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;	// abort
				// send IN packet
				n = len < 32 ? len : 32;
				for (i = n; i; i--) {
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == 32);
			return;
		}

		//Selects a configuration from the list returned by the get descriptor (almost aways #1)
		if (bRequest == SET_CONFIGURATION && bmRequestType == 0) {

			//Set our configuration, This action may tell the app code that we are now a functional usb device
			usb_configuration = wValue;
			usb_send_in();

			//Setup endpoints as listed in the configuration descriptor
			UENUM = 3;										//Select endpoint 3 ....why 3? is this a HID thing?
			UECONX = (1<<EPEN);	 							//Enable
			UECFG0X = (1<EPTYPE1)|(1<EPTYPE0)|(1<<EPDIR);	//Configure as type INTERRUPT direction IN		
			UECFG1X = (1<<EPSIZE1)|(1<<ALLOC); 				// Buffer is 32 bytes, allocated as double

			UERST = (1<<EPRST3);	//Reset the FIFO
			UERST = 0;
			return;
		}
		
		//Returns a single byte that lists the configuration in use
		if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {
			usb_wait_in_ready();
			UEDATX = usb_configuration;		
			usb_send_in();
			return;
		}

		if (bRequest == GET_STATUS) {
			usb_wait_in_ready();
			i = 0;
			if (bmRequestType == 0x82) {
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
		
		
		// Stall on get/set feature as we have none
		if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
		  && bmRequestType == 0x02 && wValue == 0) {
			i = wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT) {
				usb_send_in();
				UENUM = i;
				if (bRequest == SET_FEATURE) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);
				} else {
					UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}
		}

	}
	
	//If we fell through then signal a problem by stalling the bus
	UECONX = (1<<STALLRQ) | (1<<EPEN);
	
}


