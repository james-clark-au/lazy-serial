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
#include <Arduino.h>

#include "LazySerial/helpers.h"
#include "LazySerial/Context.h"


#define LAZYSERIAL_VERSION 2.0


#define LAZY_COMMAND(NAME)                                       \
  if (context.mode == LazySerial::CallingMode::IDENTIFY) {       \
    context.stream.print(NAME);                                  \
    return;                                                      \
  } else if (context.mode == LazySerial::CallingMode::INVOKE) {  \
    if (strcasecmp(NAME, context.entered_command_name) != 0) {   \
      return;  /* not us. */                                     \
    }                                                            \
    context.mode = LazySerial::CallingMode::MATCHED;             \
  }



namespace LazySerial
{
  /**
   * The function pointer signature used for callbacks.
   * It is necessary to pass the 'args' string as a non-const char *, since you will most likely want to
   * use strtok() on it to further parse things.
   */
  typedef void (*CallbackFunction)(Context &);

  /**
   * Function pointer signature for a generic character-reading source, for use with scripts saved to EEPROM.
   */
  typedef char (*ReaderFunction)(size_t);
  

  template <size_t BUF_SIZE>
  class LazySerial {  
  public:
    /**
     * Constructor. Pass in the Stream to read and write from/to.
     * For example, the magic 'Serial' global variable should be usable as a Stream-class object.
     */
    explicit
    LazySerial(
        Stream &stream) :
      d_stream(stream),
      d_help(NULL),
      d_callbacks(nullptr),
      d_callbacks_size(0) {
      clear_buffer();
    }
    
    /**
     * Call this during setup to set your array of LazySerial::Command s
     */
    void
    set_callbacks(
        CallbackFunction *callbacks,
        uint8_t callbacks_size) {
      d_callbacks = callbacks;
      d_callbacks_size = callbacks_size;
    }
    
    /**
     * Call this from your own loop() for LazySerial to poll the Serial device for more data.
     * Shouldn't delay for too long unless one of your callbacks ends up triggering and taking time to process.
     */
    void
    loop() {
      // Slowly assemble the command buffer byte by byte.
      bool ready = assemble_command();
      LAZY_RETURN_UNLESS(ready);
      run_command();
    }

    
    /**
     * Once the buffer is full, identify what command it is, parse and run it.
     */
    void
    run_command() {
      // Identify the command word. strchr is in <string.h>
      char *end_of_cmd = strchr(d_buf, ' ');
      char *cmd_name = d_buf;
      char *cmd_args = d_buf;
      if (end_of_cmd) {
        // Set the delimiting space to a \0, advance args ptr to one past it.
        end_of_cmd[0] = '\0';  // cmd_name will now be valid
        end_of_cmd++;
        cmd_args = end_of_cmd;
      } else {
        // No args. Put the 'args' pointer at the trailing \0 of the command itself, making args the empty string.
        cmd_args = d_buf + strlen(d_buf);
      }
      
      // Dispatch command!
      dispatch_command(cmd_name, cmd_args);
      // Clean up our buffer afterwards.
      clear_buffer();
    }
    
    /**
     * Instead of LazySerial polling the supplied Stream for commands, you can also supply a large string of
     * \n-terminated commands to run in a batch.
     */
    void
    run_script(
        const char *script) {
      const char *pos = script;
      const char *end = script;
      while (*pos) {
        // starting from pos, search for a \n or \0.
        end = pos;
        while (*end && *end != '\n') {
          end++;
        }
        // Copy the resulting line into the modifiable command buffer.
        size_t length = end - pos;
        if (length) {
          length = MIN(length, BUF_SIZE-1);
          strncpy(d_buf, pos, length);
          d_buf[length] = '\0';

          // Parse out the command and its arguments and run it!
          run_command();
        }
        // Next line
        if (*end) {
          end++;
        }
        pos = end;
      }
    }
    
    /**
     * Or a generic function to be called with an incrementing index until a '\0' is returned.
     * This lets me read from EEPROM without depending on EEPROM.h here.
     */
    void
    run_script(
        ReaderFunction read_char_fn) {
      size_t pos = 0;
      size_t this_cmd_pos = 0;
      char ch = read_char_fn(pos);
      while (ch) {
        if (ch == '\n') {
          // Reached newline, run this command rather than append '\n'.
          d_buf[this_cmd_pos] = '\0';
          run_command();
          // Reset.
          this_cmd_pos = 0;
        } else {
          // Copy into command buffer as we go.
          d_buf[this_cmd_pos++] = ch;
        }
        // Read next ch
        ch = read_char_fn(++pos);
      }
      // Reached \0, is there any leftover?
      if (this_cmd_pos) {
        d_buf[this_cmd_pos] = '\0';
        run_command();
      }
    }

    /**
     * Dispatch the command named by 'cmd_name', to whatever callback has been registered by the user.
     * If none match, cmd_help() will be invoked instead.
     */
    void
    dispatch_command(
        const char *cmd_name,
        char *cmd_args ) {
      // No-op command, helps in the case we are getting CRLF.
      LAZY_RETURN_IF (cmd_name[0] == '\0');

      // Scan through all registered callbacks.
      for (uint8_t i = 0; i < d_callbacks_size; ++i) {
        Context context{CallingMode::INVOKE, d_stream, cmd_name, cmd_args};
        d_callbacks[i](context);
        LAZY_RETURN_IF (context.mode == CallingMode::MATCHED);
      }
      // Nothing matched. Print some help?
      if (d_help) {
        Context context{CallingMode::INVOKE, d_stream, "HELP", cmd_args};
        d_help(context);
      } else {
        cmd_help();
      }
    }


    /**
     * The default help function.
     * The magic HELP command is hard-coded to actually hit this method rather than anything in
     * the function table, because we want access to our list of commands.
     */
    void
    cmd_help() {
      d_stream.print(F("ERR Available commands:"));
      for (uint8_t i = 0; i < d_callbacks_size; ++i) {
        d_stream.print(' ');
        // Ask commands to name themselves.
        Context context(CallingMode::IDENTIFY, d_stream);
        d_callbacks[i](context);
      }
      d_stream.print(F(".\n"));
    }
    /**
     * Set an alternative callback when no command matches.
     */
    void
    set_help_callback(
        CallbackFunction cmd_help) {
      d_help = cmd_help;
    }

  private:
    void
    clear_buffer() {
      d_pos = 0;
      d_buf[d_pos] = '\0';
    }
    
    /**
     * Read bytes from the serial until we get a \n.
     * Returns true if we get one and have a completed command (with \0) in the buffer,
     * false if we have yet to get a full command (or hit LAZYSERIAL_BUF_SIZE - no incomplete
     * command will be processed)
     */
    bool
    assemble_command() {
      while (d_stream.available()) {
        // Read new character
        char ch = d_stream.read();
        
        // If it's the \n terminator, don't advance pos but instead write \0 and return success.
        // Arduino seems to (correctly) interpret \n as 10, LF. Which is 'Newline' in the Serial Monitor.
        // Minicom is being weird. Let's just support both CR and LF (and in the event we get both,
        // interpret that as a regular command plus a no-op)
        if (ch == 10 || ch == 13) {
          d_buf[d_pos] = '\0';
          return true;
        }
        
        // For the mundane case, add the character to the buf and advance pos,
        // keeping a \0 terminator just in case.
        d_buf[d_pos] = ch;
        d_pos++;
        if (d_pos >= BUF_SIZE) {
          // But if we're going to overflow, forget the whole damn thing.
          clear_buffer();
          return false;
        }
        d_buf[d_pos] = '\0';  // Just me being paranoid.
      }
      return false;
    }
    
    /**
     * What stream we are reading from / writing to.
     */
    Stream &d_stream;
    
    /**
     * A statically declared list of callback functions.
     */
    CallbackFunction* d_callbacks;
    uint8_t d_callbacks_size;
  
    /**
     * Permit cmd_help to be overridden with something custom (and outside of this class).
     */
    CallbackFunction d_help;

    /**
     * Command Buffer, and our current position within it.
     */
    char d_buf[BUF_SIZE];
    int  d_pos;
    
  }; // class
} //namespace

