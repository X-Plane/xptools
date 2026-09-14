/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef WED_Version_H
#define WED_Version_H

// This file must be ALL macros - it is included by the MSVC .rc compiler
// so you can't go using const int and other fancy-pants C++ stuff!

// These versions are used in about boxes, resources, info boxes, etc.

// Bump the version here - everything below is derived from these 5 fields, so
// STRING/BIN/NUMERIC/etc. can't drift out of sync with each other anymore.
// Keep these 5 as plain literals (not expressions) - a couple of build scripts
// (cmake/WED.cmake, .github/workflows/build_wed.yml) grep this file as text.
#define WED_VERSION_MAJOR   2
#define WED_VERSION_MINOR   7
#define WED_VERSION_PATCH   3
#define WED_VERSION_BUILD   0      // only bump for another alpha/beta/rc of the same x.y.z
#define WED_VERSION_STAGE   "r1"   // a1, a2, b1, r1, r2, ...

#define WED_VERSION_STR2(x) #x
#define WED_VERSION_STR(x)  WED_VERSION_STR2(x)

#define	WED_VERSION_STRING          WED_VERSION_STR(WED_VERSION_MAJOR) "." WED_VERSION_STR(WED_VERSION_MINOR) "." WED_VERSION_STR(WED_VERSION_PATCH) "-" WED_VERSION_STAGE
#define	WED_VERSION_STRING_SHORT    WED_VERSION_STR(WED_VERSION_MAJOR) "." WED_VERSION_STR(WED_VERSION_MINOR)			// omit beta/release number and trailing zero's

#define WED_VERSION_FILENAME        WED_VERSION_STRING

#define	WED_COPYRIGHT_STRING        "(C) Copyright 2007-2026, Laminar Research."

#define	WED_VERSION_RES	            WED_VERSION_STRING
#define	WED_VERSION_BIN	            WED_VERSION_MAJOR,WED_VERSION_MINOR,WED_VERSION_PATCH,WED_VERSION_BUILD

// Sent to the gateway so it knows if our WED is up-to-date.
// major*10000 + minor*100 + patch*10 + build
#define WED_VERSION_NUMERIC		    (WED_VERSION_MAJOR*10000 + WED_VERSION_MINOR*100 + WED_VERSION_PATCH*10 + WED_VERSION_BUILD)

#endif /* WED_Version_H */
