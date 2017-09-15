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
#define	WED_VERSION				1.7.0git
#define	WED_VERSION_STRING		"1.7.0git"
#define	WED_VERSION_STRING_SHORT	"1.7.0"			// omit beta  number
#define	WED_VERSION_RES			WED_VERSION_STRING
#define	WED_VERSION_BIN			1,7,0,0

// This numeric is used by the gateway to understand if our WED is up-to-date.
#define WED_VERSION_NUMERIC		17000

#endif /* WED_Version_H */
