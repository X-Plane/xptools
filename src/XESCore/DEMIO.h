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

enum {
	dem_want_Post,	// Use pixel=post sampling
	dem_want_Area,	// Use area-pixel sampling!
	dem_want_File	// Use whatever the file has.
};

struct	DEMSpec {
	// ASSUMED: row major, north-west corner is data start.  Nearly all public DEM formats except for LR's are like htis.
	double			mWest;					// Outer bbox of DEM
	double			mSouth;
	double			mEast;
	double			mNorth;					
	int				mPost;					// Post vs. area pixels
	int				mWidth;					// Image dimensions
	int				mHeight;
	int				mBits;					// Bits per post (8, 16 or 32)
	bool			mBigEndian;				// True if big-endian, false for little endian
	bool			mFloat;					// True if floating point, false for integral types
	float			mNoData;				// No-data value, usually -9999 or -32768
	int				mHeaderBytes;			// Pre-data header size - usually 0, except for oz floats
};

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

bool	ReadRawWithHeader(DEMGeo& inMap, const char * inFilename, const DEMSpec& spec);

// SRTM HGT Files
//	16-bit signed meter heights in geo projection with -32768 = NO_DATA.
//	DEM location taken from file name in the N42W073 format.  Works with raw SRTM data.
//	This is a big-endian file.  Origin is NW corner.
bool	ReadRawHGT(DEMGeo& inMap, const char * inFileName);
bool	WriteRawHGT(const DEMGeo& inMap, const char * inFileName);

// IDA - a proprietary and rather weird old GIS raster format that we use for climate data.
// Files contain their location.
bool	ExtractIDAFile(DEMGeo& inMap, const char * inFileName);

// USGS natural files - ASCII dems type A and B records.  Files contain their bounds.
bool	ExtractUSGSNaturalFile(DEMGeo& inMap, const char * inFileName);

// GeoTiff - must be geographic projected for us to use.  Origin is NW corner.
bool	ExtractGeoTiff(DEMGeo& inMap, const char * inFileName, int post_style, int no_geo_needed);
bool	WriteGeoTiff(DEMGeo& inMap, const char * inFileName);

// DTED - contains its own geo info
bool	ExtractDTED(DEMGeo& inMap, const char * inFileName);

// 16-bit signed meter heights in geo projection with some flag as NO_DATA, typically -32768 or -9999.
// DEM location taken from file name in the N42W073 format if bound is NULL or takes explicit bounds.
// File can be either endian, reader guesses.  The reader routine guesses resolution from the tile bounds.  
// Origin is NW corner.
bool	ReadRawBIL(DEMGeo& inMap, const char * inFileName, int bounds[4]);	

// 32-bit floating point with a 5-byte header - an Austin-invented format, but useful
// because we have the entire US NED dataset in this form.  DEM position is taken from
// the header in +42-073 format.
bool	ReadARCASCII(DEMGeo& inMap, const char * inFileName);

// Raw IMG files - a giant 8-bit-per-sample worldwide image at 30 arc-seconds.  Used to
// get data out of GTOPO30 land use.
bool	ExtractRawIMGFile(DEMGeo& inMap, const char * inFileName, int inWest, int inSouth, int inEast, int inNorth);


// Weird X-Plane specific formats...preferably we'll never need to use these again!
// "Oz" file - same as above but no header and 16-bit big endian
bool	ReadFloatHGT(DEMGeo& inMap, const char * inFileName);			// 5 byte header, big endian float, column major, SW corner origin.  Really!
bool	WriteFloatHGT(const DEMGeo& inMap, const char * inFileName);
bool	ReadShortOz(DEMGeo& inMap, const char * inFileName);			// No header, little endian signed short, SW corner origin.

void	ReadHDR(const string& in_real_file, DEMSpec& io_header, bool force_area);

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
