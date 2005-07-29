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
#ifndef XESIO_H
#define XESIO_H

/*

	XES Format Basics - 
	
	An XES file is a file containing atoms.  The main atoms are:
	
	- An atom for the integrated vector map for the XES file.
	- An atom for each raster plane in the XES file.
	- An atom that contains a directory locating the raster planes.
	- An atom storing the token dictionary for this XES file.

 */

#include "AptDefs.h"
#include "MapIO.h"
#include "DEMIO.h"
#include "MemFileUtils.h"

class CDT;

void	WriteXESFile(
				const char *	inFileName,
				const Pmwx&		inMap,
					  CDT&		inMesh,
				DEMGeoMap&		inDEM,
				const AptVector& inApts,
				ProgressFunc	inFunc);
				
void	ReadXESFile(
				MFMemFile *		inFile,
				Pmwx&			inMap,
				CDT&			inCDt,
				DEMGeoMap&		inDEM,
				AptVector&		inApts,
				ProgressFunc	inFunc);
				
#endif
