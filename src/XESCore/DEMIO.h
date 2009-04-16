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
#ifndef DEMIO_H
#define DEMIO_H

#include "IODefs.h"
#include "DEMDefs.h"
#include "EnumSystem.h"

/*****************************************************************************
 * BASIC DEM IO FOR XES FILE FORMAT
 *****************************************************************************/

// These DEM IO Routines write the 'DEM format' that is part of an XES file.
// They do NOT write the atom container that holds the DEMs, just the contents.
void	WriteDEM(		DEMGeo& inMap, IOWriter * inWriter);
void	ReadDEM (		DEMGeo& inMap, IOReader * inReader);

// Translate the values of the DEM as enums.  Useful when loading an enum-based
// DEM like land use or climate.
void	RemapEnumDEM(	DEMGeo& ioMap, const TokenConversionMap& inMap);

/*****************************************************************************
 * DEM IMPORTERS
 *****************************************************************************/

// 16-bit signed meter heights in geo projection with -32768 = NO_DATA.
// DEM location taken from file name in the N42W073 format.  Works with raw SRTM data.
// This is a big-endian file.
bool	ReadRawHGT(DEMGeo& inMap, const char * inFileName);
bool	WriteRawHGT(const DEMGeo& inMap, const char * inFileName);
// 16-bit signed meter heights in geo projection with -32768 = NO_DATA.
// DEM location taken from file name in the N42W073 format.  Works with raw SRTM data.
// This is a little-endian file.
bool	ReadRawBIL(DEMGeo& inMap, const char * inFileName);
// 32-bit floating point with a 5-byte header - an Austin-invented format, but useful
// because we have the entire US NED dataset in this form.  DEM position is taken from
// the header in +42-073 format.
bool	ReadFloatHGT(DEMGeo& inMap, const char * inFileName);
// "Oz" file - same as above but no header and 16-bit big endian
bool	ReadShortOz(DEMGeo& inMap, const char * inFileName);
bool	WriteFloatHGT(const DEMGeo& inMap, const char * inFileName);
// Raw IMG files - a giant 8-bit-per-sample worldwide image at 30 arc-seconds.  Used to
// get data out of GTOPO30 land use.
bool	ExtractRawIMGFile(DEMGeo& inMap, const char * inFileName, int inWest, int inSouth, int inEast, int inNorth);
// IDA - a proprietary and rather weird old GIS raster format that we use for climate data.
// Files contain their location.
bool	ExtractIDAFile(DEMGeo& inMap, const char * inFileName);
// USGS natural files - ASCII dems type A and B records.  Files contain their bounds.
bool	ExtractUSGSNaturalFile(DEMGeo& inMap, const char * inFileName);
// GeoTiff - must be projceted
bool	ExtractGeoTiff(DEMGeo& inMap, const char * inFileName);
// DTED - contains its own geo info
bool	ExtractDTED(DEMGeo& inMap, const char * inFileName);

/*****************************************************************************
 * DEM TRANSLATION SYSTEM
 *****************************************************************************/

// Load a translation specification.  We can also optionally get a reverse mapping
// and color lookup table based on the enum colors in the config files.
bool	LoadTranslationFile(const char * 		inFileName,
						vector<int>& 			outForwardMap,
						hash_map<int, int> * 	outReverseMap,
						vector<char> *			outCLUT);
// Translate a DEM by the given mappings.
bool	TranslateDEMForward(DEMGeo& ioDEM, const vector<int>& inForwardMap);
bool	TranslateDEMReverse(DEMGeo& ioDEM, const hash_map<int, int>& inReverseMap);
// One-step - load the filter, translate the DEM.
bool	TranslateDEM(DEMGeo& ioDEM, const char * inFileName);

#endif
