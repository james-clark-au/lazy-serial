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
      USAGE,     // Used as a 'Return' value if we error out and should print usage, but also as an initial value to the macro to actually do print the usage.
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

    /**
     * Parse by setting the pointer-to-a-char* that you supply to the start of the next 'word'.
     * Like strtok, this _**modifies**_ the args string by inserting a \0 to terminate the 'word'.
     * Thus, it is delimited by space characters only.
     * "" is not considered an 'ok' return value.
     * Returns if parsing went ok.
     */
    bool
    parse_word(char **charstar_ptr) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      char *start = pos;
      
      // Consume non-whitespace.
      while (*pos && ! is_space(*pos)) {
        pos++;
      }
      LAZY_RETURN_FALSE_UNLESS(pos > start);
      
      // Create a \0-delimited string we can 'return' by modifying the args string.
      // However, if we have hit end of string, we cannot advance the parser past that legit \0.
      if (*pos != '\0') {
        *pos = '\0';
        pos++;
      }
      
      *charstar_ptr = start;
      return true;
    }


    /**
     * Parse by setting the pointer-to-a-char* that you supply to the start of the string,
     * Like strtok, this _**modifies**_ the args string by inserting a \0 to terminate it.
     * It is intended to match text between "", but you can specify that a 'bareword' is ok
     * It does *NOT* handle any escape sequences, since we are modifying the args buffer in-place.
     * It *WILL* skip over \" though.
     * "" is considered an 'ok' return value.
     * Returns if parsing went ok.
     */
    bool
    parse_string(char **charstar_ptr, bool bareword_ok = false) {
      // Consume leading whitespace.
      parse_space();
      LAZY_RETURN_FALSE_UNLESS(*pos);
      
      if (*pos != '"' && bareword_ok) {
        // Fallback to bareword.
        return parse_word(charstar_ptr);
      }
      // Move past initial ".
      pos++;
      char *start = pos;
      
      // Consume non-quote.
      while (*pos && *pos != '"') {
        if (*pos == '\\') {
          pos++;
          LAZY_RETURN_FALSE_UNLESS(*pos);  // backslash and then null is not ok, we were in a quoted string.
        }
        pos++;
      }
      LAZY_RETURN_FALSE_UNLESS(*pos);  // found a null is not ok, we were in a quoted string.
      LAZY_RETURN_FALSE_UNLESS(*pos == '"');  // we were in a quoted string.
      
      // Create a \0-delimited string we can 'return' by modifying the args string.
      // However, if we have hit end of string, we cannot advance the parser past that legit \0.
      if (*pos != '\0') {
        *pos = '\0';
        pos++;
      }
      
      *charstar_ptr = start;
      return true;
    }

    
    CallingMode::CallingMode mode;
    Stream &stream;
    const char *entered_command_name;
    char *args;  // pointer into d_buf
    
    char *pos;   // pointer into args
  }; // struct
} // namespace
