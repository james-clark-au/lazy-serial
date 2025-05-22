/*
 * This file is part of the LazySerial library example code. It is licenced under the MIT Open Source licence.
 * See the file LICENCE for details.
 * Copyright (C) 2025 James Neko <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <Arduino.h>


namespace Ticker
{
  typedef void (*CallbackFunction)();

  class Ticker
  {
  public:
    uint32_t d_delay_ms = 0;
    uint32_t d_last_ms = 0;
    CallbackFunction d_callback_fn = nullptr;

    Ticker(uint16_t tps, CallbackFunction callback) {
      if (tps > 0) {
        d_delay_ms = 1000 / tps;
      } else {
        d_delay_ms = 0;
      }
      d_last_ms = millis();
      d_callback_fn = callback;
    }

    void
    tps(uint16_t tps) {
      if (tps > 0) {
        d_delay_ms = 1000 / tps;
      } else {
        d_delay_ms = 0;
      }
    }

    bool
    running() {
      return d_delay_ms > 0;
    }

    bool
    loop() {
      if (d_delay_ms == 0) {
        return false;
      }
      if (millis() > d_last_ms + d_delay_ms) {
        d_last_ms = millis();
        if (d_callback_fn) {
          d_callback_fn();
        }
        return true;
      }
      return false;
    }
  };
} // namespace
