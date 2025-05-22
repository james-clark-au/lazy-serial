/*
 * This file is part of the LazySerial library example code. It is licenced under the MIT Open Source licence.
 * See the file LICENCE for details.
 * Copyright (C) 2025 James Neko <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <Arduino.h>

// v2, with patterns


namespace BlinkyLed
{
  class BlinkyLed
  {
  public:
    uint8_t  d_pin       = 13;
    uint8_t  d_state     = LOW;
    uint32_t d_interval  = 1000;
    uint16_t d_pattern   = 0xFF00;
    uint32_t d_sleeptime = 0;
    bool     d_inverted  = false;
  
    BlinkyLed(uint8_t pin, uint32_t interval_ms, uint16_t pattern = 0xFF00) {
      d_pin = pin;
      d_interval = interval_ms;
      d_pattern = pattern;
      if (d_pin != 0xFF) {
        pinMode(pin, OUTPUT);
      }
    }
  

    uint8_t
    pin() {
      return d_pin;
    }


    void
    setPin(uint8_t pin) {
      if (d_pin != 0xFF) {
        // Bring old pin low first, maybe even make it an input for safety
        setState(LOW);
        setMode(INPUT_PULLUP);
      }
      // Set new pin
      d_pin = pin;
      if (d_pin != 0xFF) {
        setMode(OUTPUT);
        setState(d_state);
      }
    }


    uint32_t
    interval() {
      return d_interval;
    }
  

    void
    setInterval(uint32_t interval_ms) {
      d_interval = interval_ms;
    }


    uint32_t
    sleepTime() {
      return d_sleeptime;
    }
  

    void
    setSleepTime(uint32_t sleeptime_ms) {
      d_sleeptime = sleeptime_ms;
    }


    uint16_t
    pattern() {
      return d_pattern;
    }
  

    void
    setPattern(uint16_t pattern) {
      d_pattern = pattern;
    }


    bool
    inverted() {
      return d_inverted;
    }


    void
    setInverted(bool invert) {
      d_inverted = invert;
      setState(d_state);
    }


    uint8_t
    state() {
      return d_state;
    }


    void
    setState(uint8_t state) {
      if (d_pin != 0xFF && state != d_state) {
        d_state = state;
        if ( ! d_inverted) {
          digitalWrite(d_pin, d_state);
        } else {
          if (d_state == LOW) {
            digitalWrite(d_pin, HIGH);
          } else {
            digitalWrite(d_pin, LOW);
          }
        }
      }
    }


    void
    setMode(uint8_t mode) {
      if (d_pin != 0xFF) {
        pinMode(d_pin, mode);
      }
    }

  
    void
    loop() {
      uint32_t total_time_ms = d_interval + d_sleeptime;  // How long between a full cycle, if the extra 'sleeptime' padding is included.
      uint32_t beat_position = millis() % total_time_ms;
      if (beat_position < d_interval) {
        beat_position = (beat_position * 16) / d_interval;  // should give us which bit 15..0
        uint16_t testBit = 0x01 << beat_position;
  
        if (d_pattern & testBit) {
          setState(HIGH);
        } else {
          setState(LOW);
        }
      } else {
        setState(LOW);
      }
    }
  };

} // namespace

