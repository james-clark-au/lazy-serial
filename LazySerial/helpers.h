/*
 * This file is part of the LazySerial library.
 * Copyright (C) 2025 Lazy Cat Software <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
 */
#pragma once

// Helper macros

#define LAZY_STRINGIFY(X) #X
#define LAZY_KEYVAL(X) " " #X "=" LAZY_STRINGIFY(X)

#define LAZY_RETURN_IF(X) if (X) { return; }
#define LAZY_RETURN_UNLESS(X) if (!(X)) { return; }

#define LAZY_RETURN_TRUE_IF(X) if (X) { return true; }
#define LAZY_RETURN_TRUE_UNLESS(X) if (!(X)) { return true; }
#define LAZY_RETURN_FALSE_IF(X) if (X) { return false; }
#define LAZY_RETURN_FALSE_UNLESS(X) if (!(X)) { return false; }

#ifndef MIN
  #define MIN(a, b) (a) < (b) ? (a) : (b)
#endif
#ifndef MAX
  #define MAX(a, b) (a) < (b) ? (b) : (a)
#endif

