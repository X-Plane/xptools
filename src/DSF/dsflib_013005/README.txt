/* 
 * Copyright (c) 2004, Laminar Research.
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
 
---------------------------------------------------------------
DSFLIB
---------------------------------------------------------------
 
DSFLib is Laminar Research's open source C++ library for creating
and reading DSF files.  DSFLib can be used to read existing DSF
files or write new ones.  Before using DSFLib, please see the DSF
file format specification at:

http://www.xsquawkbox.net/scenery/library.php

for an explanation of DSF concepts.

WARNING: while DSFLib is SUPPOSED to be a true C API, it turns 
out it can only be compiled right now by a C++ API due to
a limit in the way XChunkyFileUtils is used in the headers.
This needs future work.  For this reason, DSFLib names
are name mangled!

WARNING: DSFLib has only been built for Mac and Win using
Metrowerks Codewarrior so far.

---------------------------------------------------------------
BUILDING DSFLIB
---------------------------------------------------------------

DSFLib requires a generic platform #define - one of the following
variables must be #defined to 1:

APL - Macintosh Carbon and Mach-O compilation.
IBM - Windows (Win32) compilation
LIN - Linux compilation

Linux compilation is only supported on little-endian architectures.

DSFlib consists of the following files:

Public headers:
   XChunkyFileUtils.h - definitions for chunky file formats and utils.
   DSFDefs.h - common DSF file format definitions
   DSFLib.h  - public DSFLib API
Implementation:
   DSFLib.cpp - DSF parser
   DSFLibWrite.cpp - DSF generator/writer
   DSFPointPool.h 
   DSFPointPool.cpp - Utility class for working with DSF point tuples
   XChunkyFileUtils.cpp - Utility code for dealing with chunky file formats
Test (not needed to use DSFLib):
   DSFLib_Print.cpp - uses reader code to print a DSF file to the console*
   DSFLib_TestGen.cpp - uses writer code to make some simple SDF files.
   
*This code requires #define USE_MEM_FILE 0 to work outside WorldEditor -
MemFileUtils is WorldEditor's memory-mapped I/O package and can be used for
performance.  But MemFileUtils contains too many other dependencies to
include in the the standalone release of DSFLib.

DSFLib must be built as a C++ project right now, and requires an STL 
impelmentation.  We use Metrowerks' STL for Mac and Win.

---------------------------------------------------------------
DSFLIB USAGE - CONCEPTS
---------------------------------------------------------------

DSFLib abstracts the DSF file format in a few ways:

 - DSFLib manages point pools for you - you communicate with
   DSFLib using raw numeric data and DSFLib attempts to create
   the optimal point pools.  DSFLib will provide you with raw
   numbers when reading a DSF file.

 - DSFLib manages commands for you - you communicate with DSFLib
   using higher level commands like "add one triangle", etc. and
   DSFLib will consolidate commands or pick the best encoding
   and similarly unpack commands as needed.  DSFLib manages 
   setting the current primitive and point pool for you and will
   sort your commands to reduce file size.

DSFLib does not form tri-strip or tri-fans for you and requires
you to  build your own mesh patches and construct your own
vector topology (e.g. link intersecting roads where you want 
an intersection).  DSFLib does not change the index numbers of
your definition files, e.g. if you specify grass as terrain #3,
it will remain terrain #3.

FEEDER FUNCTIONS

You communicate with DSFLib via a series of "feeder functions",
or callbacks that are used to communicate data.  When reading a
DSFLib, you provide DSFLib with a series of callbacks.  When you
call the read function, your feeder functions will be called over
and over to "feed" you with data from the DSFLib.

When you write a DSFLib file, you are given a set of feeder 
functions and you call each one to add data to the DSF file.
In practice DSFLib accumulates all of this data in memory, then
sorts and processes it en masse when you actually call 'write'.

The struct DSFCallbacks_t contains the function pointers for
all callbacks - you must fill out one of these structs to read, 
and you can get one of these structs when writing.  All callbacks
take a void * ref - when you call read, the void * you pass is
sent back to you - you can use this to recover an object or variables.
When you call the writer's callbacks you must pass your writer
as the void * to each feeder function.

Most DSFLib feeder callbacks do not return a value.

MULTIPLE PASSES

DSFLib's reader can optionally make multiple passes over the
DSF commands.  You pass in an array of bitwise flags that are a
filter for what kinds of commands you want for each pass.

It is more efficient to instruct DSFLib to make multiple passes 
over the file than to call the reader multiple times because when 
you do a multi-pass read with one call DSFLib will process the 
geometry data in the file only once, saving time and memory 
allocations.  Normally DSFLib calls you back in the order commands
are found in the DSF file, but multiple passes allows you to
pull out all mesh commands first, then all objects, etc.  DSFLib
is very fast at reading the commands section, so doign multiple
passes will only add a few miliseconds to load time.

Each time a pass ends, the NextPass_f callback is called, with 
the pass number of the last completed pass.  This function
takes a return type - you can return false to abort DSF 
processing.

COORDINATES AND POINT POOLS

All DSF coordinates are passed as arrays of doubles.  When you
make a patch you specify the number of coordinates for the patch
(it's "depth) - you must then pass a ptr to this many doubles
for each vertex.  The number of required coordinates varies for
primitive and also with the type of terrain - see the DSF spec
for more info.

When you create a DSF lib file writer, you must specify the
geometric bounds of the file and the number of divisions
(in one axis), e.g. 8 divisions means 8x8 or 64 separate
point pools for a divided primitive.  DSFLib then builds point
pools.  More divisions means that each point pool covers less
area and therefore has more precision, but also means that
you will have more point pools overall, which may increase
file size.

LIMITATIONS

Currently DSFLib's writer is very crude - errors are returned by
printing to the console rather than programmatically.  Make sure
to monitor writing to detect problems!

