/*
 * hello_lazy_serial - example basic serial commands sketch.
 *
 * This file is part of the LazySerial library example code. It is licenced under the MIT Open Source licence.
 * See the file LICENCE for details.
 * Copyright (C) 2025 James Neko <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
 */ 
#include <LazySerial.h>
#include "BlinkyLed.h"

#define BAUD_RATE 9600

// Declare our global LazySerial object; tell it to use the Serial global object to communicate on.
LazySerial::LazySerial<128> lazy(Serial);

// And something to blink an LED on and off.
BlinkyLed::BlinkyLed blinky(LED_BUILTIN, 500);


// ---------------- SERIAL COMMAND CALLBACKS ----------------

void cmd_ohai(LazySerial::Context &context) {
  LAZY_COMMAND("OHAI");
  context.stream.println(F("OHAI hello_lazy_serial " __TIMESTAMP__  ));
}

void cmd_pinout(LazySerial::Context &context) {
  LAZY_COMMAND("PINOUT");
  context.stream.println(F("OK PINOUT" LAZY_KEYVAL(ARDUINO_BOARD) LAZY_KEYVAL(LED_BUILTIN) ));
}

void cmd_blink(LazySerial::Context &context) {
  LAZY_COMMAND("BLINK", "<pinNum> <interval>");
  uint8_t pinNum = 0;
  bool ok = context.parse_int_minmax<uint8_t>(&pinNum, 0, 255);
  if ( ! ok) {
    context.stream.print("OK Blink pin is ");
    context.stream.print(blinky.pin());
    context.stream.print("\n");
    return;
  }

  blinky.changePin(pinNum);
  uint32_t interval_ms = 0;
  ok = context.parse_int(&interval_ms);
  if (ok) {
    blinky.changeInterval(interval_ms);
  }

  context.stream.print("OK BLINK pin ");
  context.stream.print(blinky.pin());
  context.stream.print(" ");
  context.stream.print(blinky.d_interval);
  context.stream.print("ms\n");
}


LazySerial::CallbackFunction commands[] = {
  cmd_ohai,
  cmd_pinout,
  cmd_blink,
};


// ---------------- MAIN ARDUINO FUNCTIONS ----------------

void setup() {
  // We still need to do our own Serial init.
  Serial.begin(BAUD_RATE);
  lazy.set_commands(commands);
}


void loop() {
  // Blink a LED!
  blinky.loop();

  // Process commands via LazySerial!
  lazy.loop();
}



