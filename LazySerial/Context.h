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
#include <stdlib.h>  // strtol, strtof

#include "LazySerial/helpers.h"
#include "LazySerial/parsing.h"


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
      args(nullptr),
      pos(nullptr)  {  }

    Context(
        CallingMode::CallingMode m,
        Stream &s,
        const char *ecn,
        char *a):
      mode(m),
      stream(s),
      entered_command_name(ecn),
      args(a),
      pos(a)  {  }


    /**
     * Consume whitespace from args. Update pos.
     * You may land on \0, check for that.
     */
    void
    parse_space() {
      while (*pos && is_space(*pos)) {
        pos++;
      }
    }
    
    /**
     * Parse into some integer-like variable you supply by reference.
     * Returns if parsing went ok.
     * expect_hex: false if we don't know it's a hex number (we check for 0x), true if we know it must be hex (no 0x needed)
     */
    template<typename T>
    bool
    parse_int(T *var, bool expect_hex = false) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      
      // Test if hex.
      if (is_hex_sigil(pos)) {
        expect_hex = true;
        pos += 2;
      }
      
      char *end = pos;
      long rval = strtol(pos, &end, expect_hex ? 16 : 10);
      LAZY_RETURN_FALSE_UNLESS(end > pos);
      
      *var = rval;
      pos = end;
      return true;
    }

    /**
     * Parse into some integer-like variable you supply by reference.
     * Returns if parsing went ok.
     * min / max: values that are the inclusive range allowed, returns error if outside.
     * expect_hex: false if we don't know it's a hex number (we check for 0x), true if we know it must be hex (no 0x needed)
     */
    template<typename T>
    bool
    parse_int_minmax(T *var, T min, T max, bool expect_hex = false) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      
      // Test if hex.
      if (is_hex_sigil(pos)) {
        expect_hex = true;
        pos += 2;
      }
      
      char *end = pos;
      long rval = strtol(pos, &end, expect_hex ? 16 : 10);
      LAZY_RETURN_FALSE_UNLESS(end > pos);
      LAZY_RETURN_FALSE_UNLESS(rval >= min);
      LAZY_RETURN_FALSE_UNLESS(rval <= max);
      
      *var = rval;
      pos = end;
      return true;
    }

    /**
     * Parse into some float-like variable you supply by reference.
     * Returns if parsing went ok.
     */
    template<typename T>
    bool
    parse_float(T *var) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      
      char *end = pos;
      double rval = strtod(pos, &end);
      LAZY_RETURN_FALSE_UNLESS(end > pos);
      
      *var = rval;
      pos = end;
      return true;
    }

    
    /**
     * Parse into some float-like variable you supply by reference.
     * min / max: values that are the inclusive range allowed, returns error if outside.
     * Returns if parsing went ok.
     */
    template<typename T>
    bool
    parse_float_minmax(T *var, T min, T max) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      
      char *end = pos;
      double rval = strtod(pos, &end);
      LAZY_RETURN_FALSE_UNLESS(end > pos);
      LAZY_RETURN_FALSE_UNLESS(rval >= min);
      LAZY_RETURN_FALSE_UNLESS(rval <= max);
      
      *var = rval;
      pos = end;
      return true;
    }


    CallingMode::CallingMode mode;
    Stream &stream;
    const char *entered_command_name;
    char *args;  // pointer into d_buf
    
    char *pos;   // pointer into args
  }; // struct
} // namespace
