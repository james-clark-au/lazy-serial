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

#define BUF_SIZE 256	// Max size of any command string.

#include <Arduino.h>


/**
 * The function pointer signature used for callbacks.
 */
typedef void (*CallbackFunction)(/*####Stream? Partial Buffer?*/);

/**
 * The struct we store our callbacks in.
 */
struct Callback
{
	const char *name;
	CallbackFunction *callback;
	Callback *next;	// considering this is Arduino, we may as well use a simple linked list...
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
		CallbackFunction *callback);
		
private:
	/**
	 * What stream we are reading from / writing to.
	 */
	Stream &d_stream;	

	/**
	 * Command Buffer, and our current position within it.
	 */
	char d_buf[BUF_SIZE];
	int  d_pos;
	
	/**
	 * Where we store all the callbacks we have registered.
	 * Lack of dynamic storage is killing me.
	 */
	
	
	void
	clear_buffer()
	{
		d_pos = 0;
		d_buf[d_pos] = '\0';
	}
};


#endif
