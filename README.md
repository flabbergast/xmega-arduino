# Introduction

This add-on to [Arduino] {at least 1.6.2} adds support for ATMEL's XMEGA
line of microcontrollers.

Most of the hard work has been done by [Xmegaduino] folks. But I just
wanted an add-on to regular [Arduino] IDE.

I've added support for USB enabled atxmega chips (at the moment, two
xmega128A4U boards tested).

## Caveats (mainly for USB boards)

- USB support is alpha quality - so may be quite buggy.
- Only USB support present at the moment is for Serial (e.g. like the
  standard Arduino Leonardo). So no Keyboard, Mouse ... (like
  [Teensyduino]) support yet.
- Only DFU bootloaded boards for now.
- Autoreset is not implemented - you need to put your board into DFU
  mode manually before uploading.

- Various third-party libraries might (and most probably do) need some
  adjustments to work with XMEGAs.

- Because Arduino 1.6.2 is inconsistent as to how does it refer to where
  the compiler resides, you might need to adjust `platform.txt` on 32bit
  linux (if the compiler or avrdude is not found).

# Installation

Download zip (button on the right) and unpack to your
`ARDUINO_SKETCHES_FOLDER/hardware`. Restart IDE.

# Usage

Select one of the Xmega boards in your IDE. Enjoy!

# Remarks and ramblings

- The USB stack is based on my quick-and-dirty XMEGA-USB code, based
  originally on Nonolith Labs' [USB-XMEGA] {so not on
  Dean Camera's excellent [LUFA] library - mainly because I didn't want
  to have to deconstruct [LUFA]'s complicated build system}. So quite
  possibly it's buggy. However it should be good enough for some basic
  Serial support.
- The build settings assume DFU bootloader. The flashing is done with
  [dfu-programmer], supplied in the zip (tested on Mac OS X, Linux (i686
  and x86_64 - you'll need to install dfu-programmer manually on arm).
- I've only tested it on [X-A4U-stick] and [MT-DB-X4] xmega128A4U
  breakout from [MattairTech] - I don't have any other XMEGA boards.
- `dfu-programmer` on linux might claim that 'No device present' even if
  the DFU bootloader shows up on `lsusb`. It's a permissions problem
  then - give your user enough permissions to access that usb device
  (google usb device permissions udev rule to see how can you fix that
  on your particular system).

## Pin assignments for -A4U boards

### MT-DB-X4 from MattairTech

Digital pins go around the board from `0` = `A0`, to `29` = `E3`, with
the exception `30` = `D6` and `31` = `D7`. In particular, the LED is
digital `27` and the jumper is digital `26`.

Analog pins go from `A0` = `A0` to `A7` = `A7` and then `A8` = `B0`, up
to `A11` = `B3`.

### X-A4U stick

The pins numbers are described on the [X-A4U-stick] webpage.



[Teensyduino]: https://www.pjrc.com/teensy/teensyduino.html
[LUFA]: http://www.fourwalledcubicle.com/LUFA.php
[Xmegaduino]: https://github.com/akafugu/Xmegaduino
[Arduino]: http://arduino.cc
[X-A4U-stick]: https://flabbergast.github.io/x-a4u-r2
[dfu-programmer]: https://dfu-programmer.github.io/
[USB-XMEGA]: https://dfu-programmer.github.io/
[MattairTech]: https://www.mattairtech.com/
[MT-DB-X4]: https://www.mattairtech.com/index.php/featured/mt-db-x4.html
