#include <Arduino.h>

#if defined(USB_SERIAL)
extern "C" {
  #include "USB_usb.h"
  #include "USB_usb_xmega.h"
  USB_ENDPOINTS(4); // number of endpoints to create table for!
}
#endif

int main(void)
{
	init();

#if defined(USB_SERIAL)
  usb_init();
  usb_attach();
  Serial.begin(0); // ugly, but I can't figure out why it doesn't work without this...
#endif

	setup();

	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}

	return 0;
}

