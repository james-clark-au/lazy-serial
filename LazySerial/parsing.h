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

/**
 * Helper for parsing. Hex or Dec is fine.
 */
inline
bool
is_int_digit(char ch) {
  return (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f') || ch == 'x');
}

/**
 * Helper for parsing.
 */
inline
bool
is_space(char ch) {
  return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}


/**
 * Check for '0x'
 */
inline
bool
is_hex_sigil(char *pos) {
  return (pos[0] == '0' && pos[1] == 'x');
}


