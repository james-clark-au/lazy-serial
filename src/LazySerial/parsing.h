/*
 * This file is part of the LazySerial library.
 * Copyright (C) 2025 Lazy Cat Software <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
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


