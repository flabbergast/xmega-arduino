// originally from teensyduino (license below)

/* USB API for Teensy USB Development Board
 * http://www.pjrc.com/teensy/teensyduino.html
 * Copyright (c) 2008 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <stdint.h>
#include "Arduino.h"
extern "C" {
  #include "USB_serial_backend.h"
  #include "USB_usb.h"
}

#if defined(USB)

// Public Methods //////////////////////////////////////////////////////////////

void usb_serial_class::begin(long speed)
{
  usb_serial_clear_constants();
	// make sure USB is initialized
	//usb_init();
  //usb_attach();
	uint16_t begin_wait = (uint16_t)millis();
	while (1) {
		// wait for the host to finish enumeration
		if (usb_configuration) {
			delay(200);  // a little time for host to load a driver
			return;
		}
		// ... or a timeout (powered by a USB power adaptor that
		// wiggles the data lines to keep a USB device charging)
		if ((uint16_t)millis() - begin_wait > 2500) return;
	}
}

void usb_serial_class::end()
{
	usb_disable();
	delay(25);
}

// number of bytes available in the receive buffer
int usb_serial_class::available()
{
  return usb_serial_available();
}

int usb_serial_class::peek()
{
  return usb_serial_peek();
}

// get the next character, or -1 if nothing received
int usb_serial_class::read(void)
{
  return usb_serial_getchar();
}

/* // normally inherited from Stream, just if we want a faster implementation ...
size_t usb_serial_class::readBytes(char *buffer, size_t length)
{
  // lazy implementation
	size_t count=0;
	unsigned long startMillis;

	startMillis = millis();
	if (length <= 0) return 0;
	do {
    if(usb_serial_available()>0) {
      *buffer++ = (char)usb_serial_getchar();
      count++;
    }
    if(count == length) {
      return count;
    }
	} while(millis() - startMillis < _timeout);
	setReadError();
	return count;
}
*/

// discards input, sends off output
void usb_serial_class::flush()
{
  usb_serial_flush_input();
  usb_serial_flush_output();
}

/* pulling slow implementation from Print
// transmit a block of data
size_t usb_serial_class::write(const uint8_t *buffer, uint16_t size)
{
  return usb_serial_write(buffer, size);
}
*/

size_t usb_serial_class::write(uint8_t c)
{
  return usb_serial_putchar_nowait(c);
}

// These are Teensy-like extensions to the Serial object

// immediately transmit any buffered output.
// This doesn't actually transmit the data - that is impossible!
// USB devices only transmit when the host allows, so the best
// we can do is release the FIFO buffer for when the host wants it
void usb_serial_class::send_now(void)
{
  usb_serial_flush_output();
}

uint32_t usb_serial_class::baud(void)
{
	return *(uint32_t *)cdc_line_coding;
}

uint8_t usb_serial_class::stopbits(void)
{
	return cdc_line_coding[4];
}

uint8_t usb_serial_class::paritytype(void)
{
	return cdc_line_coding[5];
}

uint8_t usb_serial_class::numbits(void)
{
	return cdc_line_coding[6];
}

uint8_t usb_serial_class::dtr(void)
{
	return (cdc_line_rtsdtr & USB_SERIAL_DTR) ? 1 : 0;
}

uint8_t usb_serial_class::rts(void)
{
	return (cdc_line_rtsdtr & USB_SERIAL_RTS) ? 1 : 0;
}

usb_serial_class::operator bool()
{
	if (usb_configuration &&
	  (cdc_line_rtsdtr & (USB_SERIAL_DTR | USB_SERIAL_RTS))) {
		return true;
	}
	return false;
}


// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(USB_SERIAL)
usb_serial_class Serial = usb_serial_class();
#endif

#endif // defined(USB)
