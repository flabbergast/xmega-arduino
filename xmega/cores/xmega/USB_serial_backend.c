#include <avr/io.h>

#if defined(USB)

#include <avr/pgmspace.h>
#include "USB_usb_xmega.h"
#include "USB_serial_backend.h"

/**************************************************************************
 *
 *  Configurable Options
 *
 **************************************************************************/

// You can change these to give your code its own name.  On Windows,
// these are only used before an INF file (driver install) is loaded.
#define STR_MANUFACTURER L"flabbergast"
#define STR_PRODUCT      L"X-A4U serial"

// All USB serial devices are supposed to have a serial number
// (according to Microsoft).  On windows, a new COM port is created
// for every unique serial/vendor/product number combination.  If
// you program 2 identical boards with 2 different serial numbers
// and they are assigned COM7 and COM8, each will always get the
// same COM port number because Windows remembers serial numbers.
//
// On Mac OS-X, a device file is created automatically which
// incorperates the serial number, eg, /dev/cu-usbmodem12341
//
// Linux by default ignores the serial number, and creates device
// files named /dev/ttyACM0, /dev/ttyACM1... in the order connected.
// Udev rules (in /etc/udev/rules.d) can define persistent device
// names linked to this serial number, as well as permissions, owner
// and group settings.
#define STR_SERIAL_NUMBER  L"12345"

// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define VENDOR_ID               0x03EB
#define PRODUCT_ID              0x206F

// When you write data, it goes into a USB endpoint buffer, which
// is transmitted to the PC when it becomes full, or after a timeout
// with no more writes.  Even if you write in exactly packet-size
// increments, this timeout is used to send a "zero length packet"
// that tells the PC no more data is expected and it should pass
// any buffered data to the application that may be waiting.  If
// you want data sent immediately, call usb_serial_flush_output().
#define TRANSMIT_FLUSH_TIMEOUT  40   /* in 1/8 milliseconds */

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

// These buffer sizes are best for most applications, but perhaps if you
// want more buffering on some endpoint at the expense of others, this
// is where you can make such changes.  The AT90USB162 has only 176 bytes
// of DPRAM (USB buffers) and only endpoints 3 & 4 can double buffer.

#define CDC_ACM_ENDPOINT        2 // was 2
#define CDC_RX_ENDPOINT         4 // was 3
#define CDC_TX_ENDPOINT         3 // was 4
#define CDC_ACM_SIZE            16 // was 8
#define CDC_RX_SIZE             32 // was 16
#define CDC_TX_SIZE             32 // was 16

/**************************************************************************
 *
 *  Descriptors
 *
 **************************************************************************/

#define INTERFACE_ID_CDC_CCI 0
#define INTERFACE_ID_CDC_DCI 1

// just a note: avr-gcc/xmega is little endian, so the two byte
// values below really appear in memory in reverse order
// here's a working one (lufa) 12 01 10 01 EF 02 01 08 EB 03 6F 20 01 00 01 02 DC 01
const USB_DeviceDescriptor PROGMEM device_descriptor = {
  .bLength = sizeof(USB_DeviceDescriptor), // 0x12
  .bDescriptorType = USB_DTYPE_Device, // 0x01

  .bcdUSB                 = 0x0200, // was 0110 (valid 0200 (2.0), 0100 (1.0) and 0110 (1.1))
  .bDeviceClass           = 0xEF, // CSCP_IADDeviceClass; was 2
  .bDeviceSubClass        = 0x02, // CSCP_IADDeviceSubclass; was 0
  .bDeviceProtocol        = 0x01, // CSCP_IADDeviceProtocol; was 0

  .bMaxPacketSize0        = USB_EP0_SIZE, // was 0x08
  .idVendor               = VENDOR_ID, // 0x03EB
  .idProduct              = PRODUCT_ID, // 0x206F
  .bcdDevice              = 0x0100, // 0x0100

  .iManufacturer          = 1, // 0x01
  .iProduct               = 2, // 0x02
  .iSerialNumber          = 3, // 0xDC

  .bNumConfigurations     = 1 // 0x01
};

// CDC descriptors typedefs
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint16_t  CDCSpecification;
} __attribute__ ((packed)) USB_CDC_Header_FunctionalDescriptor;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
  uint8_t bDataInterface;
} __attribute__ ((packed)) USB_CDC_CallManagement_FunctionalDescriptor;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
} __attribute__ ((packed)) USB_CDC_ACM_FunctionalDescriptor;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bMasterInterface;
  uint8_t bSlaveInterface0;
} __attribute__ ((packed)) USB_CDC_Union_FunctionalDescriptor;

// General descriptor typedefs
typedef struct ConfigDesc {
  USB_ConfigurationDescriptor Config;
  USB_InterfaceAssociationDescriptor CDC_IAD;
  USB_InterfaceDescriptor CDC_CCI_Interface;
  USB_CDC_Header_FunctionalDescriptor CDC_Header_FunctionalDescriptor;
  //USB_CDC_CallManagement_FunctionalDescriptor CallManagement_FunctionalDescriptor;
  USB_CDC_ACM_FunctionalDescriptor ACM_FunctionalDescriptor;
  USB_CDC_Union_FunctionalDescriptor Union_FunctionalDescriptor;
  USB_EndpointDescriptor CDC_Notification_Endpoint;
  USB_InterfaceDescriptor CDC_DCI_Interface;
  USB_EndpointDescriptor CDC_DataOut_Endpoint;
  USB_EndpointDescriptor CDC_DataIn_Endpoint;
} ConfigDesc;

// here's working one (lufa):
// 09 02 46 00 02 01 00 C0 32
// 08 0B 00 02 02 02 01 00
// 09 04 00 00 01 02 02 01 00
// 05 24 00 10 01
// 04 24 02 06
// 05 24 06 00 01
// 07 05 82 03 08 00 FF
// 09 04 01 00 02 0A 00 00 00
// 07 05 04 02 10 00 05
// 07 05 83 02 10 00 05
const ConfigDesc PROGMEM configuration_descriptor = {
  .Config = {
    .bLength = sizeof(USB_ConfigurationDescriptor), // 0x09
    .bDescriptorType = USB_DTYPE_Configuration, // 0x02
    .wTotalLength  = sizeof(ConfigDesc), // 0x0046 (=70)
    .bNumInterfaces = 2, // 0x02
    .bConfigurationValue = 1, // 0x01
    .iConfiguration = 0, // 0x00
    .bmAttributes = USB_CONFIG_ATTR_BUSPOWERED | USB_CONFIG_ATTR_SELFPOWERED, // 0xC0
    .bMaxPower = USB_CONFIG_POWER_MA(100) // 0x32
  },
  .CDC_IAD = {
    .bLength = sizeof(USB_InterfaceAssociationDescriptor), // 0x08
    .bDescriptorType = USB_DTYPE_InterfaceAssociation, // 0x0B
    .bFirstInterface = INTERFACE_ID_CDC_CCI, // CDC CCI Interface
    .bInterfaceCount = 2,
    .bFunctionClass = 0x02, // CDC_CSCP_CDCClass
    .bFunctionSubClass = 0x02, // CDC_CSCP_ACMSubclass
    .bFunctionProtocol = 0x01, // CSC_CSCP_ATCommandProtocol
    .iFunction = 0, // NO_DESCRIPTOR
  },
  .CDC_CCI_Interface = {
    .bLength = sizeof(USB_InterfaceDescriptor), // 0x09
    .bDescriptorType = USB_DTYPE_Interface, // 0x04
    .bInterfaceNumber = INTERFACE_ID_CDC_CCI,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = 0x02, //  CDC_CSCP_CDCClass
    .bInterfaceSubClass = 0x02, // CDC_CSCP_ACMSubclass
    .bInterfaceProtocol = 0x01, // CDC_CSCP_ATCommandProtocol
    .iInterface = 0
  },
  .CDC_Header_FunctionalDescriptor = {
    .bLength = sizeof(USB_CDC_Header_FunctionalDescriptor), // 0x05
    .bDescriptorType = 0x24, // DTYPE_CSInterface
    .bDescriptorSubtype = 0x00, // CDC_DSUBTYPE_CSInterface_Header
    .CDCSpecification = 0x0110 // bcdCDC
  },
  /*
  .CallManagement_FunctionalDescriptor = {
    .bLength = sizeof(USB_CDC_CallManagement_FunctionalDescriptor), // 5
    .bDescriptorType = 0x24,
    .bDescriptorSubtype = 0x01,
    .bmCapabilities = 0x01,
    .bDataInterface = 1,
  },
  */
  .ACM_FunctionalDescriptor = {
    .bLength = sizeof(USB_CDC_ACM_FunctionalDescriptor), // 0x04
    .bDescriptorType = 0x24, // DTYPE_CSInterface
    .bDescriptorSubtype = 0x02, // CDC_DSUBTYPE_CSInterface_ACM
    .bmCapabilities = 0x06
  },
  .Union_FunctionalDescriptor = {
    .bLength = sizeof(USB_CDC_Union_FunctionalDescriptor), // 0x05
    .bDescriptorType = 0x24, // DTYPE_CSInterface
    .bDescriptorSubtype = 0x06, // CDC_DSUBTYPE_CSInterface_Union
    .bMasterInterface = INTERFACE_ID_CDC_CCI, // CDC CCI Interface
    .bSlaveInterface0 = INTERFACE_ID_CDC_DCI // CDC DCI Interface
  },
  .CDC_Notification_Endpoint = {
    .bLength = sizeof(USB_EndpointDescriptor), // 0x07
    .bDescriptorType = USB_DTYPE_Endpoint, // 0x05
    .bEndpointAddress = CDC_ACM_ENDPOINT | USB_IN, // 0x82
    .bmAttributes = (USB_EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), // 0x03
    .wMaxPacketSize = CDC_ACM_SIZE, // 0x0008
    .bInterval = 0xFF // 0xFF
  },
  .CDC_DCI_Interface = {
    .bLength = sizeof(USB_InterfaceDescriptor), // 0x09
    .bDescriptorType = USB_DTYPE_Interface, // 0x04
    .bInterfaceNumber = INTERFACE_ID_CDC_DCI,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0x0A, // CSC_CSCP_CDCDataClass
    .bInterfaceSubClass = 0x00, // CDC_CSCP_NoDataSubclass
    .bInterfaceProtocol = 0x00, // CDC_CSCP_NoDataProtocol
    .iInterface = 0
  },
  .CDC_DataOut_Endpoint = {
    .bLength = sizeof(USB_EndpointDescriptor), // 0x07
    .bDescriptorType = USB_DTYPE_Endpoint, // 0x05
    .bEndpointAddress = CDC_RX_ENDPOINT, // 0x04
    .bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), // 0x02
    .wMaxPacketSize = CDC_RX_SIZE, // 0x0010
    .bInterval = 0x05
  },
  .CDC_DataIn_Endpoint = {
    .bLength = sizeof(USB_EndpointDescriptor), // 0x07
    .bDescriptorType = USB_DTYPE_Endpoint, // 0x05
    .bEndpointAddress = CDC_TX_ENDPOINT | USB_IN, //  0x83
    .bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), // 0x02
    .wMaxPacketSize = CDC_TX_SIZE, // 0x0010
    .bInterval = 0x05
  },
};

struct usb_string_descriptor_struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        int16_t wString[];
};
static const struct usb_string_descriptor_struct PROGMEM language_string = {
        4,
        3,
        {USB_LANGUAGE_EN_US}
};
static const struct usb_string_descriptor_struct PROGMEM manufacturer_string = {
        sizeof(STR_MANUFACTURER),
        3,
        STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM product_string = {
        sizeof(STR_PRODUCT),
        3,
        STR_PRODUCT
};
static const struct usb_string_descriptor_struct PROGMEM serial_number_string = {
        sizeof(STR_SERIAL_NUMBER),
        3,
        STR_SERIAL_NUMBER
};

/**************************************************************************
 *
 *  USB CDC constants
 *
 **************************************************************************/

    /** Enum for the CDC class specific control requests that can be issued by the USB bus host. */
    enum CDC_ClassRequests_t
    {
      CDC_REQ_SendEncapsulatedCommand = 0x00, /**< CDC class-specific request to send an encapsulated command to the device. */
      CDC_REQ_GetEncapsulatedResponse = 0x01, /**< CDC class-specific request to retrieve an encapsulated command response from the device. */
      CDC_REQ_SetLineEncoding         = 0x20, /**< CDC class-specific request to set the current virtual serial port configuration settings. */
      CDC_REQ_GetLineEncoding         = 0x21, /**< CDC class-specific request to get the current virtual serial port configuration settings. */
      CDC_REQ_SetControlLineState     = 0x22, /**< CDC class-specific request to set the current virtual serial port handshake line states. */
      CDC_REQ_SendBreak               = 0x23, /**< CDC class-specific request to send a break to the receiver via the carrier channel. */
    };

    /** Enum for the CDC class specific notification requests that can be issued by a CDC device to a host. */
    enum CDC_ClassNotifications_t
    {
      CDC_NOTIF_SerialState = 0x20, /**< Notification type constant for a change in the virtual serial port
                                     *   handshake line states, for use with a \ref USB_Request_Header_t
                                     *   notification structure when sent to the host via the CDC notification
                                     *   endpoint.
                                     */
    };


/**************************************************************************
 *
 *  Variables
 *
 **************************************************************************/

// the time remaining before we transmit any partially full
// packet, or send a zero length packet.
volatile uint16_t transmit_countdown=TRANSMIT_FLUSH_TIMEOUT;
// claim access to tx buffer
volatile bool transmit_lock=false;

// serial port settings (baud rate, control signals, etc) set
// by the PC.  These are ignored, but kept in RAM.
uint8_t cdc_line_coding[7]={0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08};
volatile uint8_t cdc_line_rtsdtr=0;

// points to the next character in the received buffer
volatile size_t cdc_rx_buf_next_char_ptr = 0;
// next character in the transmit buffer
volatile size_t cdc_tx_buf_next_char_ptr = 0;

// buffers
uint8_t ep_acm_buf_in[CDC_ACM_SIZE];
uint8_t ep_rx_buf[CDC_RX_SIZE];
uint8_t ep_tx_buf[CDC_TX_SIZE];

// write/transmit timeout (cycle counter)
#define WRITE_TIMEOUT 100000

/**************************************************************************
 *
 *  USB stack callbacks
 *
 **************************************************************************/

uint16_t usb_cb_get_descriptor(uint8_t type, uint8_t index, const uint8_t** ptr, uint16_t offset) {
  const void* address = NULL;
  uint16_t size    = 0;

  switch (type) {
    case USB_DTYPE_Device:
      address = &device_descriptor;
      size    = sizeof(USB_DeviceDescriptor);
      break;
    case USB_DTYPE_Configuration:
      address = &configuration_descriptor;
      size    = sizeof(ConfigDesc);
      break;
    case USB_DTYPE_String:
      switch (index) {
        case 0x00:
          address = &language_string;
          break;
        case 0x01:
          address = &manufacturer_string;
          break;
        case 0x02:
          address = &product_string;
          break;
        case 0x03:
          address = &serial_number_string;
          break;
      }
      size = pgm_read_byte(&((USB_StringDescriptor*)address)->bLength);
      break;
  }

  *ptr = usb_ep0_from_progmem(address+offset, size-offset); // may need to call again if size>USB_EP0_SIZE, so offset
  return size;
}

void usb_cb_reset(void) {
  cdc_line_rtsdtr = 0;
}

bool usb_cb_set_configuration(uint8_t config) {
  if (config <= 1) {
    // set up endpoints here (CTRL, STATUS, DATAPTR)
    usb_ep_enable(CDC_ACM_ENDPOINT|USB_IN, USB_EP_TYPE_BULK_gc, CDC_ACM_SIZE); // Bulk=Interrupt mask
    usb_ep_enable(CDC_RX_ENDPOINT, USB_EP_TYPE_BULK_gc, CDC_RX_SIZE);
    usb_ep_enable(CDC_TX_ENDPOINT|USB_IN, USB_EP_TYPE_BULK_gc, CDC_TX_SIZE);
    // clear the other directions (CTRL = 0)
    usb_ep_disable(CDC_ACM_ENDPOINT);
    usb_ep_disable(CDC_RX_ENDPOINT|USB_IN);
    usb_ep_disable(CDC_TX_ENDPOINT);
    // start OUT endpoints (clear BUSNACK0, assign DATAPTR)
    usb_ep_start_out(CDC_RX_ENDPOINT, ep_rx_buf, CDC_RX_SIZE);
    return true;
  } else {
    return false;
  }
}

void usb_cb_control_setup(void) { // this is probably EVENT_USB_Device_ControlRequest

  if( usb_setup.wIndex == INTERFACE_ID_CDC_CCI ) {
    switch(usb_setup.bRequest) {
      case CDC_REQ_GetLineEncoding:
        if (usb_setup.bmRequestType == (USB_IN | USB_REQTYPE_CLASS | USB_RECIPIENT_INTERFACE)) {
          while( !(usb_ep_in_ready(0x80)) );
          usb_ep_start_in(0x80,cdc_line_coding, 7, true);
          usb_ep0_out();
        }
        break;
      case CDC_REQ_SetLineEncoding:
        if (usb_setup.bmRequestType == (USB_OUT | USB_REQTYPE_CLASS | USB_RECIPIENT_INTERFACE)) {
          usb_ep0_out(); // clear SETUP transaction
          while( !(usb_ep_pending(0)) ); // wait for an incoming OUT
          memcpy(cdc_line_coding, ep0_buf_out, 7);
          usb_ep0_in(0);
          usb_ep0_out();
        }
        break;
      case CDC_REQ_SetControlLineState:
        if (usb_setup.bmRequestType == (USB_OUT | USB_REQTYPE_CLASS | USB_RECIPIENT_INTERFACE)) {
          cdc_line_rtsdtr = usb_setup.wValue;
          usb_ep0_in(0);
          usb_ep0_out();
        }
        break;
      case CDC_REQ_SendBreak:
        if (usb_setup.bmRequestType == (USB_OUT | USB_REQTYPE_CLASS | USB_RECIPIENT_INTERFACE)) {
          // do nothing - add a callback?
          usb_ep0_in(0);
          usb_ep0_out();
        }
        break;
    }
  }
}


bool usb_cb_set_interface(uint16_t interface, uint16_t altsetting) {
  return false;
}

void usb_cb_control_in_completion(void) {

}

void usb_cb_control_out_completion(void) {

}

void usb_cb_completion(void) {

}

void usb_cb_start_of_frame(void) {
  if( (transmit_countdown == 0) ||
     (cdc_tx_buf_next_char_ptr >= CDC_TX_SIZE)) {
    transmit_countdown = TRANSMIT_FLUSH_TIMEOUT;
    usb_serial_flush_output();
  }
  transmit_countdown--;
}

/**************************************************************************
 *
 *  Serial API Functions
 *
 **************************************************************************/

void usb_serial_clear_constants(void) {
  cdc_rx_buf_next_char_ptr=0;
  cdc_tx_buf_next_char_ptr=0;
  cdc_line_rtsdtr=0;
}

// returns -1 when problem
int16_t usb_serial_getchar(void) {
  size_t cur_avail = usb_serial_available();
  if( usb_configuration && cur_avail > 0 ) {
    uint8_t c = ep_rx_buf[cdc_rx_buf_next_char_ptr++];
    // if we consumed the whole buffer, clean up
    if(cur_avail == 1) {
      cdc_rx_buf_next_char_ptr = 0;
      usb_ep_start_out(CDC_RX_ENDPOINT, ep_rx_buf, CDC_RX_SIZE);
    }
    return c;
  }
  return -1;
}

// returns -1 when problem
int16_t usb_serial_peek(void) {
  if( usb_configuration && (usb_serial_available() > 0) ) {
    return (uint8_t)ep_rx_buf[cdc_rx_buf_next_char_ptr];
  }
  return -1;
}

size_t usb_serial_available(void) {
  if( usb_configuration && usb_ep_out_complete(CDC_RX_ENDPOINT) ) {
    return usb_ep_out_length(CDC_RX_ENDPOINT) - cdc_rx_buf_next_char_ptr;
  }
  return 0;
}

void usb_serial_flush_input(void) {
  cdc_rx_buf_next_char_ptr=0;
  usb_ep_reset(CDC_RX_ENDPOINT|USB_OUT); // clear any pending transfers
}

// returns 0 on success, -1 on failure (buffer full)
// gets transmitted within TRANSMIT_FLUSH_TIMEOUT or immediately
//   if the buffer is full
// use usb_serial_putchar to have it flushed right away
int8_t usb_serial_putchar_nowait(uint8_t c) {
  int8_t retval = -1;
  transmit_lock = true;
  if(usb_configuration && cdc_tx_buf_next_char_ptr < CDC_TX_SIZE) {
    ep_tx_buf[cdc_tx_buf_next_char_ptr++] = c;
    retval = 0;
  }
  transmit_lock = false;
  if( cdc_tx_buf_next_char_ptr == CDC_TX_SIZE ) // flush if buffer full
    usb_serial_flush_output();
  return retval;
}

// same as _nowait version, forcibly flushing at the end
int8_t usb_serial_putchar(uint8_t c) {
  int8_t retval = usb_serial_putchar_nowait(c);
  usb_serial_flush_output();
  return retval;
}

// transmit a buffer
size_t usb_serial_write(const uint8_t *buffer, uint16_t size) {
  int8_t retval = 0;
  uint32_t timeout = WRITE_TIMEOUT;
  uint16_t size_next = 0, transmitted = 0;
  if( usb_configuration && !transmit_lock ) { // if USB is connected and nothing is interfering with the buffer
    if( cdc_tx_buf_next_char_ptr > 0 ) { // transmit any previous leftover data
      while( !usb_ep_in_ready(CDC_TX_ENDPOINT|USB_IN) && timeout > 0) // wait for next free IN request
        timeout--;
      if(timeout == 0)
        return 0;
      usb_serial_flush_output();
    }
    transmit_lock = true;
    while( size > 0 ) { // each loop will transmit a chunk of data
      size_next = size < CDC_TX_SIZE ? size : CDC_TX_SIZE;
      timeout = WRITE_TIMEOUT;
      while( !usb_ep_in_ready(CDC_TX_ENDPOINT|USB_IN) && timeout > 0) // wait for next free IN request
        timeout--;
      if(timeout == 0)
        return transmitted;
      usb_ep_start_in(CDC_TX_ENDPOINT|USB_IN, buffer+transmitted, size_next, false);
      transmitted += size_next;
      size -= size_next;
    }
    transmit_lock = false;
    retval = transmitted;
  }
  return retval;
}

// send the current tx buffer in
void usb_serial_flush_output(void) {
  if( !usb_configuration || !usb_ep_in_ready(CDC_TX_ENDPOINT|USB_IN) || transmit_lock)
    return; // don't do anything if disconnected or previous IN packet/event still interferes, or if something's working with the buffer
  transmit_lock = true;
  usb_ep_start_in(CDC_TX_ENDPOINT|USB_IN, ep_tx_buf, cdc_tx_buf_next_char_ptr, false);
  uint32_t timeout = WRITE_TIMEOUT;
  while( !usb_ep_in_ready(CDC_TX_ENDPOINT|USB_IN) && timeout > 0) // wait for next free IN request
    timeout--;
  cdc_tx_buf_next_char_ptr = 0;
  transmit_countdown = TRANSMIT_FLUSH_TIMEOUT;
  transmit_lock = false;
}

// serial controls
uint32_t usb_serial_get_baud(void)
{
	return *(uint32_t *)cdc_line_coding;
}
uint8_t usb_serial_get_stopbits(void)
{
	return cdc_line_coding[4];
}
uint8_t usb_serial_get_paritytype(void)
{
	return cdc_line_coding[5];
}
uint8_t usb_serial_get_numbits(void)
{
	return cdc_line_coding[6];
}
uint8_t usb_serial_get_control(void)
{
	return cdc_line_rtsdtr;
}

#endif // defined(USB)
