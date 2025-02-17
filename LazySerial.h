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

#ifndef LAZYSERIAL_H
#define LAZYSERIAL_H

#define LAZYSERIAL_VERSION 1.3

// Max size of any command string. Including the \0!
// We currently kludge things a bit and allocate two buffers.
#ifndef LAZYSERIAL_BUF_SIZE
  #define LAZYSERIAL_BUF_SIZE 256
#endif
// Max number of commands we will ever have.
#ifndef LAZYSERIAL_NUM_CMDS
  #define LAZYSERIAL_NUM_CMDS 25
#endif


#include <Arduino.h>


namespace LazySerial
{
	/**
	 * The function pointer signature used for callbacks.
	 * It is necessary to pass the 'args' string as a non-const char *, since you will most likely want to
	 * use strtok() on it to further parse things.
	 */
	typedef void (*CallbackFunction)(char *);

	/**
	 * Function pointer signature for a generic character-reading source, for use with scripts saved to EEPROM.
	 */
	typedef char (*ReaderFunction)(size_t);
	
	/**
	 * The struct we store our callbacks in.
	 */
	struct Callback
	{
		const char *name;
		CallbackFunction callback;
	};


	class LazySerial
	{  
	public:
		/**
		 * Constructor. Pass in the Stream to read and write from/to.
		 * For example, the magic 'Serial' global variable should be usable as a Stream-class object.
		 */
		explicit
		LazySerial(
			Stream &stream);
		
		/**
		 * Call this from your own loop() for LazySerial to poll the Serial device for more data.
		 * Shouldn't delay for too long unless one of your callbacks ends up triggering and taking time to process.
		 */
		void
		loop();
		
		/**
		 * Register a callback for use when a given command is seen.
		 */
		void
		register_callback(
			const char* name,
			CallbackFunction callback);

    /**
		 * The default help function.
		 * The magic HELP command is hard-coded to actually hit this method rather than anything in
		 * the function table, because we want access to our list of commands.
		 */
		void
		cmd_help();

    /**
		 * Set an alternative callback when no command matches.
		 */
    void
    set_help_callback(
			CallbackFunction cmd_help);
		
		/**
		 * Once the buffer is full, identify what command it is, parse and run it.
		 */
		void
		run_command();
		
		/**
		 * Instead of LazySerial polling the supplied Stream for commands, you can also supply a large string of
		 * \n-terminated commands to run in a batch.
		 */
		void
		run_script(
			const char *script);
		
		/**
		 * Or a generic function to be called with an incrementing index until a '\0' is returned.
		 * This lets me read from EEPROM without depending on EEPROM.h here.
		 */
		void
		run_script(
			ReaderFunction read_char_fn);

		/**
		 * Dispatch the command named by 'cmd_name', to whatever callback has been registered by the user.
		 * If none match, cmd_help() will be invoked instead.
		 */
		void
		dispatch_command(
			const char *cmd_name,
			char *cmd_args );

		/**
		 * As dispatch_command() but in the event we only have a const char * for the arguments.
		 */
		void
		dispatch_command(
			const char *cmd_name,
			const char *cmd_args );

	private:
		/**
		 * What stream we are reading from / writing to.
		 */
		Stream &d_stream;	
	
		/**
		 * Command Buffer, and our current position within it.
		 */
		char d_buf[LAZYSERIAL_BUF_SIZE];
		int  d_pos;
		
		/**
		 * Args Buffer, needed just in case client code is using dispatch() with a const char *...
		 * Could be done better but I guess I don't really want to clobber the serial buffer?
		 */
		char d_args_tmp[LAZYSERIAL_BUF_SIZE];
		
		/**
		 * Where we store all the callbacks we have registered.
		 * Lack of dynamic storage is killing me.
		 */
		Callback d_commands[LAZYSERIAL_NUM_CMDS];
		int      d_num_commands;

		/**
		 * Permit cmd_help to be overridden with something custom (and outside of this class).
		 */
		CallbackFunction d_help;

		
		void
		clear_buffer()
		{
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
		assemble_command();
		
	};

} //namespace

#endif
