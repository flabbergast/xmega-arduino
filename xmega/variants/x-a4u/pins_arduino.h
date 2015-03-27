/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#define USE_RTC // Use RTC for millis etc.

#define REPEAT8(x) x, x, x, x, x, x, x, x
#define BV0TO7 _BV(0), _BV(1), _BV(2), _BV(3), _BV(4), _BV(5), _BV(6), _BV(7)
#define BV7TO0 _BV(7), _BV(6), _BV(5), _BV(4), _BV(3), _BV(2), _BV(1), _BV(0)

#define NUM_DIGITAL_PINS            23
#define NUM_ANALOG_INPUTS           8
#define EXTERNAL_NUM_INTERRUPTS     18

#define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 10 : -1)
#define digitalPinHasPWM(p)         (((p) >= 0 && (p) <= 5) || ((p) >= 19 && (p) <= 21))

static const uint8_t SS    = 4;
static const uint8_t MOSI  = 5;
static const uint8_t MISO  = 6;
static const uint8_t SCK   = 7;

static const uint8_t SDA = 0;
static const uint8_t SCL = 1;
static const uint8_t LED_BUILTIN = 19;

static const uint8_t A0 = 10;
static const uint8_t A1 = 11;
static const uint8_t A2 = 12;
static const uint8_t A3 = 13;
static const uint8_t A4 = 14;
static const uint8_t A5 = 15;
static const uint8_t A6 = 16;
static const uint8_t A7 = 17;

#define Wire xmWireC

#define SPI_PORT	SPIC

// TODO this - what the heck does this do on XMEGA?
#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 22) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7) ? 2 : (((p) <= 13) ? 0 : 1))
#define digitalPinToPCMSK(p)    (((p) <= 7) ? (&PCMSK2) : (((p) <= 13) ? (&PCMSK0) : (((p) <= 21) ? (&PCMSK1) : ((uint8_t *)0))))
#define digitalPinToPCMSKbit(p) (((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : ((p) - 14)))

#ifdef ARDUINO_MAIN

// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_PGM[] = {
	NOT_A_PORT,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
		// PORTLIST
		PC,/* 0 */
		PC,
		PC,
		PC,
		PC,
		PC,
		PC,
		PC,
		PB,/* 8 */
		PB,
		PA,/* 10 A0 */
		PA,
		PA,
		PA,
		PA,
		PA,
		PA,
		PA,
		PE,/* 18 */
		PE,
		PE,
		PE,
		PB,/* 22 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
		// PIN IN PORT
		// -------------------------------------------
		_BV( 0 ), // 0
		_BV( 1 ),
		_BV( 2 ),
		_BV( 3 ),
		_BV( 4 ),
		_BV( 5 ),
		_BV( 6 ),
		_BV( 7 ),
		_BV( 2 ), // 8
		_BV( 3 ),
		_BV( 0 ), // 10
		_BV( 1 ),
		_BV( 2 ),
		_BV( 3 ),
		_BV( 4 ),
		_BV( 5 ),
		_BV( 6 ),
		_BV( 7 ),
		_BV( 0 ), // 18
		_BV( 1 ),
		_BV( 2 ),
		_BV( 3 ),
		_BV( 0 ), // 22
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	// TIMERS
	// -------------------------------------------
	TIMER_C0A	, // 0
	TIMER_C0B	,
	TIMER_C0C	,
	TIMER_C0D	,
	TIMER_C1A	,
	TIMER_C1B	,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, // 8
	NOT_ON_TIMER,
	NOT_ON_TIMER, // 10
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, // 18
	TIMER_E0B	,
	TIMER_E0C	,
	TIMER_E0D	,
	NOT_ON_TIMER	, // 22
};

const TC0_t* const PROGMEM timer_to_tc0_PGM[] = {
	NULL,

	&TCC0,
	&TCC0,
	&TCC0,
	&TCC0,
	NULL,
	NULL,

	&TCD0,
	&TCD0,
	&TCD0,
	&TCD0,
	NULL,
	NULL,

	&TCE0,
	&TCE0,
	&TCE0,
	&TCE0,
	NULL,
	NULL,
	};

const TC1_t* const PROGMEM timer_to_tc1_PGM[] = {
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	&TCC1,
	&TCC1,

	NULL,
	NULL,
	NULL,
	NULL,
	&TCD1,
	&TCD1,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

const TC2_t* const PROGMEM timer_to_tc2_PGM[] = {
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

const uint8_t PROGMEM timer_to_channel_PGM[] = {
	NOT_ON_TIMER,

    0,
    1,
    2,
    3,
    0,
    1,

    0,
    1,
    2,
    3,
    0,
    1,

    0,
    1,
    2,
    3,
    0,
    1,
};

const uint8_t PROGMEM adc_to_channel_PGM[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
};

#endif

#endif
