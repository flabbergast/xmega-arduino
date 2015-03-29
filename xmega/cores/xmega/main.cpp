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
  usb_configure_clock();
  // Enable USB interrupts
  USB.INTCTRLA = USB_SOFIE_bm | USB_BUSEVIE_bm | USB_INTLVL_MED_gc;
  USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
  usb_init();
  sei();
  usb_attach();
#endif

	setup();

	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}

	return 0;
}

