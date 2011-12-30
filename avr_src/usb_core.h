// Based on USB Debug Channel Example from http://www.pjrc.com/teensy/

#ifndef USB_CORE_H
#define USB_CORE_H

//types?

uint8_t get_usb_configuration(void);
void set_unhandled_endpoint_interrupt_callback(uint8_t (*func)(uint8_t));
void set_unhandled_setup_packet_callback(uint8_t (*func)(uint8_t,uint8_t,uint16_t,uint16_t,uint16_t));
void set_get_descriptor_callback(uint8_t (*func)(uint16_t,uint16_t,uint16_t));
void set_set_configuration_callback(uint8_t (*func)(uint16_t));

void usb_init(void);					// initialize everything

#define CONTROL_EP_SIZE 	32
#define MAX_ENDPOINT		4

// standard control endpoint request types
#define  DEVICE_TO_HOST		0x80
#define  HOST_TO_DEVICE		0x00
#define  DEVICE_REQUEST		0x00
#define  INTERFACE_REQUEST	0x01
#define  ENDPOINT_REQUEST	0x02

#define GET_STATUS			0
#define CLEAR_FEATURE		1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR		6
#define GET_CONFIGURATION	8
#define SET_CONFIGURATION	9
#define GET_INTERFACE		10
#define SET_INTERFACE		11

#define ENDPOINT_HALT		0x00


#endif
