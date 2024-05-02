/*
 * This file is part of the LazyCat LazySerial library.
 * Copyright (C) 2016 James Clark <james@lazycat.com.au>
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

#include "LazySerial.h"

#include <string.h>	// C-style string functions strchr, strtok, etc.

#ifndef MIN
  #define MIN(a, b) (a) < (b) ? (a) : (b)
#endif


namespace LazySerial
{
	//constructor
	LazySerial::LazySerial(
			Stream &stream) :
		d_stream(stream),
		d_help(NULL)
	{
		clear_buffer();
		d_num_commands = 0;
	}
	
	
	void
	LazySerial::loop()
	{
		// Slowly assemble the command buffer byte by byte.
		bool ready = assemble_command();
		if ( ! ready) {
			return;
		}
		run_command();
	}


  // Once the buffer has a command in it, parse out the command and its args and then run it.
	void LazySerial::run_command()
	{
		// Identify the command word. strchr is in <string.h>
		char *end_of_cmd = strchr(d_buf, ' ');
		char *cmd_name = d_buf;
		char *cmd_args = d_buf;
		if (end_of_cmd) {
			// Set the delimiting space to a \0, advance args ptr to one past it.
			end_of_cmd[0] = '\0';	// cmd_name will now be valid
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

	
	void
	LazySerial::register_callback(
		const char* name,
		CallbackFunction callback)
	{
		if (d_num_commands >= LAZYSERIAL_NUM_CMDS) {
			return;		// Nope!
		}
		d_commands[d_num_commands].name = name;
		d_commands[d_num_commands].callback = callback;
		d_num_commands++;
	}


  // built-in help function, can be changed.
	void
	LazySerial::cmd_help()
	{
		d_stream.print(F("ERR Available commands:"));
		for (int i = 0; i < d_num_commands; ++i) {
			d_stream.print(' ');
			d_stream.print(d_commands[i].name);
		}
		d_stream.print(F(".\n"));
	}

	void
	LazySerial::set_help_callback(
		CallbackFunction cmd_help)
	{
		d_help = cmd_help;
	}


  void
	LazySerial::run_script(
		const char *script)
	{
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
				length = MIN(length, LAZYSERIAL_BUF_SIZE-1);
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

	void
	LazySerial::dispatch_command(
			const char *cmd_name,
			char *cmd_args )
	{
		// No-op command, helps in the case we are getting CRLF.
		if (cmd_name[0] == '\0') {
			return;
		}
		// Scan through all registered callbacks.
		for (int i = 0; i < d_num_commands; ++i) {
			if (strcasecmp(cmd_name, d_commands[i].name) == 0) {
				d_commands[i].callback(cmd_args);
				return;
			}
		}
		// Nothing matched. Print some help?
		if (d_help) {
		  d_help(cmd_args);
		} else {
			cmd_help();
		}
	}


	void
	LazySerial::dispatch_command(
			const char *cmd_name,
			const char *cmd_args )
	{
		// Put args somewhere mutable.
		strncpy(d_args_tmp, cmd_args, LAZYSERIAL_BUF_SIZE-1);
		d_args_tmp[LAZYSERIAL_BUF_SIZE-1] = '\0';
		dispatch_command(cmd_name, d_args_tmp);
	}

	bool
	LazySerial::assemble_command()
	{
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
			if (d_pos >= LAZYSERIAL_BUF_SIZE) {
				// But if we're going to overflow, forget the whole damn thing.
				clear_buffer();
				return false;
			}
			d_buf[d_pos] = '\0';	// Just me being paranoid.
		}
		return false;
	}
		
} //namespace

