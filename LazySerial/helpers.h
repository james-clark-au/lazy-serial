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

// Helper macros

#define LAZY_STRINGIFY(X) #X
#define LAZY_KEYVAL(X) " " #X "=" LAZY_STRINGIFY(X)

#ifndef MIN
  #define MIN(a, b) (a) < (b) ? (a) : (b)
#endif
#ifndef MAX
  #define MAX(a, b) (a) < (b) ? (b) : (a)
#endif

