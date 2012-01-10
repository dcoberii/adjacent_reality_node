#ifndef USB_SERIAL_H
#define USB_SERIAL_H

void usb_serial_init(void);
uint8_t usb_serial_ready();
void usb_serial_tx(const uint8_t*, uint8_t);

#define DEVICE_DESCRIPTOR	0x01
#define CONFIG_DESCRIPTOR	0x02
#define STRING_DESCRIPTOR	0x03
#define INTERFACE_DESCRIPTOR 0x04
#define ENDPOINT_DESCRIPTOR	0x05


#define USB_1_1		0x0110
#define UNUSED		0x00		
#define NONE		0x00
#define BUS_POWERED		0x00
#define SELF_POWERED	0x40
#define mA(n)		n>>1

#define EP_BULK 		0x02
#define EP_INTERRUPT	0x03
#define EP_IN			0x00
#define EP_OUT			0x80

#define ENGLISH 		{0x04,0x09}

#define CDC					0x02
#define CDC_CCI				0x02	
#define CDC_ACM				0x02
#define CDC_ACM_AT			0x01
#define CDC_CDI				0x0A
#define CDC_CS_INTERFACE	0x24
#define CDC_1_1				0x0110
#define CDC_FUNC_HEADER		0x00
#define CDC_FUNC_ACM		0x02
#define CDC_FUNC_UNION		0x06

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} device_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} config_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} interface_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} endpoint_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bData0;
	uint8_t bData1;
} cdc_function_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t *bData;
} string_descriptor_t;

#endif
