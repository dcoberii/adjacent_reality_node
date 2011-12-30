
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usb_serial.h"
#include "usb_core.h"

const device_descriptor_t PROGMEM device_descriptor = {
	.bLength = 			sizeof(device_descriptor_t),
	.bDescriptorType = 	DEVICE_DESCRIPTOR,
	.bcdUSB = 			USB_1_1,
	.bDeviceClass = 	CDC,
	.bDeviceSubClass = 	UNUSED,
	.bDeviceProtocol = 	UNUSED,
	.bMaxPacketSize0 = 	32,
	.idVendor = 		0x03EB, 
	.idProduct = 		0x204B,
	.bcdDevice = 		0x0001,
	.iManufacturer = 	NONE,
	.iProduct = 		NONE,
	.iSerialNumber = 	NONE,
	.bNumConfigurations = 1
};

const config_descriptor_t PROGMEM config_descriptor[1] = {{
	.bLength = 			sizeof(config_descriptor_t),
	.bDescriptorType = 	CONFIG_DESCRIPTOR,
	.wTotalLength = 	61,
	.bNumInterfaces = 	2,
	.bConfigurationValue = 1,
	.iConfiguration = 	NONE,
	.bmAttributes = 	0x80|BUS_POWERED,
	.bMaxPower = 		mA(100)
}};

const interface_descriptor_t PROGMEM interface_descriptor[2] = {{
	.bLength = 			sizeof(interface_descriptor_t),
	.bDescriptorType = 	INTERFACE_DESCRIPTOR,
	.bInterfaceNumber = 0,
	.bAlternateSetting = NONE,
	.bNumEndpoints = 	1,
	.bInterfaceClass = 	CDC_CCI,
	.bInterfaceSubClass = CDC_ACM,
	.bInterfaceProtocol = CDC_ACM_AT,
	.iInterface = 		NONE
},{
	.bLength = 			sizeof(interface_descriptor_t),
	.bDescriptorType = 	INTERFACE_DESCRIPTOR,
	.bInterfaceNumber = 1,
	.bAlternateSetting = NONE,
	.bNumEndpoints = 	2,
	.bInterfaceClass = 	CDC_CDI,
	.bInterfaceSubClass = UNUSED,
	.bInterfaceProtocol = NONE,
	.iInterface = 		NONE	
}};

const endpoint_descriptor_t PROGMEM endpoint_descriptor[3] = {{
	.bLength = 			sizeof(endpoint_descriptor_t),
	.bDescriptorType = 	ENDPOINT_DESCRIPTOR,
	.bEndpointAddress = 1 | EP_OUT,
	.bmAttributes = 	EP_INTERRUPT,
	.wMaxPacketSize = 	32,
	.bInterval = 		64,
},{
	.bLength = 			sizeof(endpoint_descriptor_t),
	.bDescriptorType = 	ENDPOINT_DESCRIPTOR,
	.bEndpointAddress = 2 | EP_IN,
	.bmAttributes = 	EP_BULK,
	.wMaxPacketSize = 	32,
	.bInterval = 		0,
},{
	.bLength = 			sizeof(endpoint_descriptor_t),
	.bDescriptorType = 	ENDPOINT_DESCRIPTOR,
	.bEndpointAddress = 3 | EP_OUT,
	.bmAttributes = 	EP_BULK,
	.wMaxPacketSize = 	32,
	.bInterval = 		0,
}};
 
const cdc_function_descriptor_t PROGMEM cdc_function_descriptor[3] ={{
	.bLength = 			4,
	.bDescriptorType = 	CDC_CS_INTERFACE,
	.bDescriptorSubtype = CDC_FUNC_HEADER,
	.bData0 = 0x10,		//bcdCDC
	.bData1 = 0x01,
},{
	.bLength = 			4,
	.bDescriptorType = 	CDC_CS_INTERFACE,
	.bDescriptorSubtype = CDC_FUNC_ACM,
	.bData0 = NONE,		//bmCapabilities
},{
	.bLength = 			5,
	.bDescriptorType = 	CDC_CS_INTERFACE,
	.bDescriptorSubtype = CDC_FUNC_UNION,
	.bData0 = 0,	//MasterInterface
	.bData0 = 1,	//SlaveInterface0
}}; 

#define STR_MANUFACTURER	"ATMEL USB TEST"
#define STR_PRODUCT			"CDC DEVICE"
 /*
const string_descriptor_t PROGMEM string_descriptor[3] ={{
	.bLength = 			4,
	.bDescriptorType =  STRING_DESCRIPTOR,
	.bData = PSTR(STR_MANUFACTURER),
},{
	.bLength = 			 sizeof(STR_MANUFACTURER) + 2,
	.bDescriptorType =  STRING_DESCRIPTOR,
	.bData = PSTR(STR_MANUFACTURER),
},{
	.bLength = 			sizeof(STR_PRODUCT) + 2,
	.bDescriptorType =  STRING_DESCRIPTOR,
	.bData =  PSTR(STR_PRODUCT),
}};
*/
// I wrote this, but now I am not sure what it does
void send_data_p(uint8_t *data, uint16_t length){

	while(length>0){
		while (!(UEINTX & ((1<<TXINI)|(1<<RXOUTI))));	
		while((length>0) && (UEBCX<32)){
			UEDATX = pgm_read_byte(data++);
			length--;
		}
		if((length>0)){
			UEINTX = ~(1<<TXINI);
		}
	}
	
	
}

uint16_t MIN(uint16_t a, uint16_t b){ if (a>b){return b;}else{return a;}}

//Something to test: descriptor length > wLength = EP_size
uint8_t get_descriptor(uint16_t wValue, uint16_t wIndex, uint16_t wLength){
	
	switch(wValue>>8){
		
		case DEVICE_DESCRIPTOR:
			//Determine maximum length
			//wLength =;
			//Send data from descriptor
			send_data_p( (uint8_t *)&device_descriptor,  MIN(wLength, pgm_read_byte(&device_descriptor.bLength)));
			//terminate transaction
			UEINTX = ~(1<<TXINI);
			return 1;

		case CONFIG_DESCRIPTOR:
			//Determine maximum length
			wLength = MIN(wLength, pgm_read_word(&config_descriptor[0].wTotalLength));
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&config_descriptor[0], MIN(wLength, pgm_read_byte(&config_descriptor[0].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&config_descriptor[0].bLength);
			
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&interface_descriptor[0], MIN(wLength, pgm_read_byte(&interface_descriptor[0].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&interface_descriptor[0].bLength);
						
			//Send data from descriptor
			send_data_p( (uint8_t *)&cdc_function_descriptor[0], MIN(wLength, pgm_read_byte(&cdc_function_descriptor[0].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&cdc_function_descriptor[0].bLength);
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&cdc_function_descriptor[1], MIN(wLength, pgm_read_byte(&cdc_function_descriptor[1].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&cdc_function_descriptor[1].bLength);
						
			//Send data from descriptor
			send_data_p( (uint8_t *)&cdc_function_descriptor[2], MIN(wLength, pgm_read_byte(&cdc_function_descriptor[2].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&cdc_function_descriptor[2].bLength);
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&endpoint_descriptor[0], MIN(wLength, pgm_read_byte(&endpoint_descriptor[0].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&endpoint_descriptor[0].bLength);			
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&interface_descriptor[1], MIN(wLength, pgm_read_byte(&interface_descriptor[1].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&interface_descriptor[1].bLength);
			
			//Send data from descriptor
			send_data_p( (uint8_t *)&endpoint_descriptor[1], MIN(wLength, pgm_read_byte(&endpoint_descriptor[1].bLength)));
			//Adjust length
			wLength -=pgm_read_byte(&endpoint_descriptor[1].bLength);

			//Send data from descriptor
			send_data_p( (uint8_t *)&endpoint_descriptor[2], MIN(wLength, pgm_read_byte(&endpoint_descriptor[2].bLength)));
						
			//terminate transaction
			UEINTX = ~(1<<TXINI);
			return 1;
			/*
		case STRING_DESCRIPTOR:
			if((wValue & 0xFF)<3){
				//Send data from descriptor header
				send_data_p( (uint8_t *)&string_descriptor[(wValue & 0xFF)],  MIN(wLength, 2));
				
				//Send data from descriptor
			//	send_data_p( (uint8_t *)pgm_read_byte(&string_descriptor[(wValue & 0xFF)].bData), MIN(wLength, pgm_read_byte(&string_descriptor[(wValue & 0xFF)].bLength))-2);
				//terminate transaction
				UEINTX = ~(1<<TXINI);
				return 1;
			}
			*/
	}
	
	return 0;
	
}
 
//Called from the USB_COM_vect when a set configuation device request is recieved.
//returns 0 on failure
uint8_t set_configuration(uint16_t wValue){

	if(wValue == 1){
		//Set our configuration, This action may tell the app code that we are now a functional usb device
		UEINTX = ~(1<<TXINI); 	// Once and only once? usb_send_in();

		//Setup endpoints as listed in the configuration descriptor
		UENUM = 1;										//Select endpoint 1
		UECONX = (1<<EPEN);	 							//Enable
		UECFG0X = (1<EPTYPE1)|(1<EPTYPE0)|(0<<EPDIR);	//Configure as type INTERRUPT direction OUT		
		UECFG1X = (1<<EPSIZE1)|(1<<ALLOC); 				//Buffer is 32 bytes, allocated as single

		UENUM = 2;										//Select endpoint 2
		UECONX = (1<<EPEN);	 							//Enable
		UECFG0X = (1<EPTYPE1)|(1<<EPDIR);				//Configure as type BULK direction IN		
		UECFG1X = (1<<EPSIZE1)|(1<<ALLOC); 				//Buffer is 32 bytes, allocated as single		//FIXME: double
		
		UENUM = 3;										//Select endpoint 3
		UECONX = (1<<EPEN);	 							//Enable
		UECFG0X = (1<EPTYPE1)|(0<<EPDIR);				//Configure as type BULK direction OUT
		UECFG1X = (1<<EPSIZE1)|(1<<ALLOC); 				//Buffer is 32 bytes, allocated as single

		UERST = (1<<EPRST1)|(1<<EPRST2)|(1<<EPRST3);	//Reset the FIFO
		UERST = 0;
		
		return 1;
	}
	
	return 0;
	
}

uint8_t unhandled_endpoint_interrupt(uint8_t bEndpoint){
	
	return 0;
}

uint8_t unhandled_setup_packet(uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength){

	//For a CDC application we would see things like get/set line coding features here. However we do not support such fineries
	return 0;	
}

void usb_serial_init(void){

	set_unhandled_endpoint_interrupt_callback(&unhandled_endpoint_interrupt);
	set_unhandled_setup_packet_callback(&unhandled_setup_packet);
	set_get_descriptor_callback(&get_descriptor);
	set_set_configuration_callback(&set_configuration);
	usb_init();

}

uint8_t usb_serial_ready(){return get_usb_configuration();}

void usb_serial_tx(const uint8_t data[], uint8_t length){
	
		cli();
		UENUM = 3;

		for(;length>0;length--){
			UEDATX = *(data++);
		}
		UEINTX = ~(1<<TXINI);
		sei();
	
}

//rx_byte_callback

