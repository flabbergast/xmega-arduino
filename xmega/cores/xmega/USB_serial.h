// originally from teensyduino

#ifndef USB_serial_h_
#define USB_serial_h_

#include <inttypes.h>

#include "Stream.h"

#if ARDUINO < 100
  #error "Only implemented for Arduino >= 1.00"
#endif

class usb_serial_class : public Stream
{
public:
	void begin(long);
	void end();
	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
	virtual size_t write(uint8_t c); //		{ return write(&c, 1); }
  inline size_t write(unsigned long n) { return write((uint8_t)n); }
  inline size_t write(long n) { return write((uint8_t)n); }
  inline size_t write(unsigned int n) { return write((uint8_t)n); }
  inline size_t write(int n) { return write((uint8_t)n); }
  // this would be faster but ...
	// virtual size_t write(const uint8_t *buffer, uint16_t size);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	void send_now(void);
	uint32_t baud(void);
	uint8_t stopbits(void);
	uint8_t paritytype(void);
	uint8_t numbits(void);
	uint8_t dtr(void);
	uint8_t rts(void);
	operator bool();
	//size_t readBytes(char *buffer, size_t length);
private:
};

#if defined(USB_SERIAL)
extern usb_serial_class Serial;
#endif

#endif
