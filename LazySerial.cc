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

namespace LazySerial
{
	//constructor
	LazySerial::LazySerial(
			Stream &stream) :
		d_stream(stream)
	{
		clear_buffer();
		d_num_commands = 0;
	}
	
	
	void
	LazySerial::loop()
	{
	}
	
	
	void
	LazySerial::register_callback(
		const char* name,
		CallbackFunction *callback)
	{
		d_commands[d_num_commands].name = name;
		d_commands[d_num_commands].callback = callback;
		d_num_commands++;
	}


	void
	LazySerial::cmd_help()
	{
		d_stream.print("Available commands:");
		for (int i = 0; i < d_num_commands; ++i) {
			d_stream.print(" ");
			d_stream.print(d_commands[i].name);
		}
		d_stream.print(".\n");
	}


	bool
	LazySerial::assemble_command()
	{
		while (d_stream.available()) {
			// Read new character
			char ch = d_stream.read();
			
			// If it's the \n terminator, don't advance pos but instead write \0 and return success.
			if (ch == '\n') {
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
			d_buf[d_pos] = '\0';	// Just me being paranoid.
		}
		return false;
	}
	
} //namespace

