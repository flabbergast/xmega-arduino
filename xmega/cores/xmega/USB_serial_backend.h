#ifndef usb_serial_backend_h__
#define usb_serial_backend_h__

#include <stdint.h>

// *** variables ***

// zero when we are not configured, non-zero when enumerated
extern volatile uint8_t usb_configuration;

// serial port settings (baud rate, control signals, etc) set
// by the PC.  These are ignored, but kept in RAM.
extern uint8_t cdc_line_coding[7];
extern volatile uint8_t cdc_line_rtsdtr;

// points to the next character in the received buffer
extern volatile size_t cdc_rx_buf_next_char_ptr;
// next character in the transmit buffer
extern volatile size_t cdc_tx_buf_next_char_ptr;

// *** functions ***

void usb_serial_clear_constants(void);

// receiving data
int16_t usb_serial_getchar(void);	// receive a character (-1 if timeout/error)
int16_t usb_serial_peek(void);	// peek at the next char (-1 if timeout/error)
size_t usb_serial_available(void);	// number of bytes in receive buffer
void usb_serial_flush_input(void);	// discard any buffered input

// transmitting data
int8_t usb_serial_putchar(uint8_t c);	// transmit a character (flush after)
int8_t usb_serial_putchar_nowait(uint8_t c);  // transmit a character, do not wait
size_t usb_serial_write(const uint8_t *buffer, uint16_t size); // transmit a buffer
void usb_serial_flush_output(void);	// immediately transmit any buffered output

// serial parameters
uint32_t usb_serial_get_baud(void);	// get the baud rate
uint8_t usb_serial_get_stopbits(void);	// get the number of stop bits
uint8_t usb_serial_get_paritytype(void);// get the parity type
uint8_t usb_serial_get_numbits(void);	// get the number of data bits
uint8_t usb_serial_get_control(void);	// get the RTS and DTR signal state
// setting signals not implemented
// int8_t usb_serial_set_control(uint8_t signals); // set DSR, DCD, RI, etc

// constants corresponding to the various serial parameters
#define USB_SERIAL_DTR			0x01
#define USB_SERIAL_RTS			0x02
#define USB_SERIAL_1_STOP		0
#define USB_SERIAL_1_5_STOP		1
#define USB_SERIAL_2_STOP		2
#define USB_SERIAL_PARITY_NONE		0
#define USB_SERIAL_PARITY_ODD		1
#define USB_SERIAL_PARITY_EVEN		2
#define USB_SERIAL_PARITY_MARK		3
#define USB_SERIAL_PARITY_SPACE		4
#define USB_SERIAL_DCD			0x01
#define USB_SERIAL_DSR			0x02
#define USB_SERIAL_BREAK		0x04
#define USB_SERIAL_RI			0x08
#define USB_SERIAL_FRAME_ERR		0x10
#define USB_SERIAL_PARITY_ERR		0x20
#define USB_SERIAL_OVERRUN_ERR		0x40

#endif
