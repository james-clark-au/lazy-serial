/*
 * This file is part of the LazySerial library.
 * Copyright (C) 2025 James Neko <arduino@neko.stream>
 * 
 * LazySerial is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *          
 * LazySerial is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with LazySerial.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <string.h>  // C-style string functions strchr, strtok, etc.

#include "LazySerial/helpers.h"


namespace LazySerial
{
  /**
   * When callbacks are called, they might be called for a variety of purposes, not just invocation.
   */
  namespace CallingMode {
    enum CallingMode {
      IDENTIFY,  // Print our name to serial
      INVOKE,    // Run, if we match.
      MATCHED,   // 'Return' value - we matched, no need to run help.
    };
  }

  class Context {
  public:
    Context(
        CallingMode::CallingMode m,
        Stream &s):
      mode(m),
      stream(s),
      entered_command_name(nullptr),
      args(nullptr) {  }

    Context(
        CallingMode::CallingMode m,
        Stream &s,
        const char *ecn,
        char *a):
      mode(m),
      stream(s),
      entered_command_name(ecn),
      args(a) {  }
      
    CallingMode::CallingMode mode;
    Stream &stream;
    const char *entered_command_name;
    char *args;  // pointer into d_buf
  }; // struct
} // namespace
