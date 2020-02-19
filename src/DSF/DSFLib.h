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
#ifndef DSFLIB_H
#define DSFLIB_H

#include "XChunkyFileUtils.h"
#include "DSFDefs.h"
/*
 * DSFLib Error enumerations.
 *
 * DSFReadFile and DSFReadMem return one of these values.
 *
 */

enum {
	dsf_ErrOK = 0,						/* Success																					*/
	dsf_ErrCouldNotOpenFile,			/* The requested file could not be opened.													*/
	dsf_ErrOutOfMemory,					/* There was not enough memory to read the file.											*/
	dsf_ErrCouldNotReadFile,			/* The file could not be read (an I/O error).												*/
	dsf_ErrNoAtoms,						/* This file appears to not be atomic.														*/
	dsf_ErrBadCookie,					/* The DSF cookie at the file didn't match - probably not a DSF file!						*/
	dsf_ErrBadVersion,					/* The DSF version is not compatible with this lib.											*/
	dsf_ErrMissingAtom,					/* Required DSF atoms are missing.															*/
	dsf_ErrBadProperties,				/* The properties atom is corrupted.														*/
	dsf_ErrMisformattedCommandAtom,		/* The commands atom is corrupted.															*/
	dsf_ErrMisformattedScalingAtom,		/* A scaling atom is corrupted.																*/
	dsf_ErrBadCommand,					/* An unknown command index was encountered.  (Usually do to a corrupt command sequence.)   */
	dsf_ErrUserCancel,					/* The NextPass_f callback returned false to cancel reading the next pass.					*/
	dsf_ErrPoolOutOfRange,				/* A bad DSF point pool was selected.  (Usually a semantically corrupt file.)				*/
	dsf_ErrBadChecksum,					/* MD5 signature is bad - indicates poorly made DSF?										*/
	dsf_ErrCanceled						/* Client code aborted in definitions CB */
};

/*
 * Other DSF enumerations
 *
 */

enum {
	/* DSF Triangle types. */
	dsf_Tri,				/* DSF Triangle - three vertices form one triangle.							*/
	dsf_TriStrip,			/* DSF Triangle Strip - N+2 vertices form N triangles (like OpenGL.)		*/
	dsf_TriFan,				/* DSF Triangle Fan - N+2 vertices form N triangles (like OpenGL.)			*/

	/* Pass flags - these flags filter DSF callbacks when reading the file. */
	dsf_CmdProps   = 0x01,	/* Return properties.							*/
	dsf_CmdDefs    = 0x02,	/* Return definitions							*/
	dsf_CmdPatches = 0x04,	/* Return Patches and triangles					*/
	dsf_CmdVectors = 0x08,	/* Return vector types							*/
	dsf_CmdPolys   = 0x10,	/* Return polygons (facades, etc.)				*/
	dsf_CmdObjects = 0x20,	/* Return objects								*/
	dsf_CmdSign	   = 0x40,	/* Do MD5 signature of data						*/
	dsf_CmdRaster	=0x80,	/* Raster data									*/
	dsf_CmdAll	   = 0xFF	/* Return everything at once.					*/
};

enum {

	obj_ModeDraped = 0,	// 3 coords, lon/lat/rotation
	obj_ModeMSL = 1,	// 4 coords, lon/lat/rotation/MSL ele
	obj_ModeAGL = 2		// 4 coords, lon/lat/rotation/AGL ele

};


/*
 * Error message strings
 *
 * DSFLib contains an array of null terminated C strings with text versions of the
 * error returns that can be shown to the user for diagnostic purposes.
 *
 */
extern const char *	dsfErrorMessages[];


/*
 * DSFCallbacks_t
 *
 * This structure contains function pointers for each feeder function.
 *
 */
struct	DSFCallbacks_t {

	/* This is called when the lib completes before it goes on to the next pass. */
	bool (* NextPass_f)(int finished_pass_index, void * inRef);

	/* These functions accept our various definitions.  Return 1 to proceed, 0 to cancel */
	int (*	AcceptTerrainDef_f)(const char * inPartialPath, void * inRef);
	int (*	AcceptObjectDef_f )(const char * inPartialPath, void * inRef);
	int (*	AcceptPolygonDef_f)(const char * inPartialPath, void * inRef);
	int (*	AcceptNetworkDef_f)(const char * inPartialPath, void * inRef);
	int (*	AcceptRasterDef_f )(const char * inPartialPath, void * inRef);

	/* This function accepts properties from the file. */
	void (* AcceptProperty_f)(const char * inProp, const char * inValue, void * inRef);

	/* These functions build patches.  You receive a start
	 * patch, then a set of homogenous triangles, then an
	 * end patch.  All patch vertices must match the number
	 * of coordinates passed in inCoordDepth. */
	void (* BeginPatch_f)(
					unsigned int	inTerrainType,
					double 			inNearLOD,
					double 			inFarLOD,
					unsigned char	inFlags,
					int				inCoordDepth,
					void *			inRef);
	void (* BeginPrimitive_f)(
					int				inType,
					void *			inRef);
	void (* AddPatchVertex_f)(
					double			inCoordinates[],
					void *			inRef);
	void (* EndPrimitive_f)(
					void *			inRef);
	void (* EndPatch_f)(
					void *			inRef);

	/* This function adds an object.  All objects take
	 * two coordinates. */
	void (*	AddObjectWithMode_f)(
					unsigned int	inObjectType,
					double			inCoordinates[4],	// Lon Lat Rot [MSL]
					int				inMode,				// Draped/AGL/MSL enum
					void *			inRef);

	/* This function adds a complete chain.  All chains
	 * take 3 coordinates for non-curved nodes and 6
	 * coordinates for curved nodes. */
	void (* BeginSegment_f)(
					unsigned int	inNetworkType,
					unsigned int	inNetworkSubtype,
					double			inCoordinates[],	// lon lat el, start node ID, shape lon lat el
					bool			inCurved,
					void *			inRef);
	void (*	AddSegmentShapePoint_f)(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef);
	void (* EndSegment_f)(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef);

	/* These functions add polygons.  You'll get at least
	 * one winding per polygon.  All polygons take two
	 * coordinates. */
	void (* BeginPolygon_f)(
					unsigned int	inPolygonType,
					unsigned short	inParam,
					int				inCoordDepth,
					void *			inRef);
	void (* BeginPolygonWinding_f)(
					void *			inRef);
	void (* AddPolygonPoint_f)(
					double *		inCoordinates,
					void *			inRef);
	void (* EndPolygonWinding_f)(
					void *			inRef);
	void (* EndPolygon_f)(
					void *			inRef);

	void (* AddRasterData_f)(
					DSFRasterHeader_t *	header,
					void *				data,
					void *				inRef);

	void (* SetFilter_f)(
					int					inFilterIndex,
					void *				inRef);

};

/************************************************************
 * DFS READING UTILS
 ************************************************************
 *
 * To read a DSF file, you fill out a struct with pointers
 * to functions to accept the various DSF primitives, and then
 * call DSFRead.
 *
 * You can use DSFReadFile to have DSFLib open the file for
 * you.  You can also pass a block of memory that represents
 * the whole file, using DSFReadMem - DSFReadMem will not
 * read outside the block and will not write to it, so you
 * can use a read-only memory mapped file.
 *
 * inRef is a void * passed to each of your callbacks.
 *
 * if inPasses is not NULL, it is an array of ints with a
 * set of filter flags indicating what callbacks you want
 * for each pass over the file.  You must return true from
 * your nextPass routine to go to the next pass.  The last
 * int in the array should be 0 to indicate the end of
 * the array.
 *
 * These functions return an error code.  See DSFLib.cpp for
 * #defines to control debug diagnostic output.
 *
 */

/* Returns true if successful, false if not. */
int		DSFReadFile(const char * inPath, void * (* malloc_func)(size_t s), void (* free_func)(void * ptr), DSFCallbacks_t * inCallbacks, const int * inPasses, void * inRef);
int		DSFReadMem(const char * inStart, const char * inStop, DSFCallbacks_t * inCallbacks, const int * inPasses, void * inRef);
int		DSFCheckSignature(const char * inPath);
/************************************************************
 * DFS WRITING UTILS
 ************************************************************
 *
 * To write a DSF file, you create a file writer.  You then get
 * a callbacks struct for that writer and call them to add data
 * to the writer.  Once done adding data, you call WriteToFile,
 * which dumps the data out to disk.
 *
 * When you make a writer you must specify the geometric extent
 * of the file and the number of divisions to cut the file into
 * for a point pool.  WorldEditor currently uses 8 divisions.
 *
 */

void *	DSFCreateWriter(double inWest, double inSouth, double inNorth, double inEast, double inElevMin, double inElevMax, int divisions);
void	DSFGetWriterCallbacks(DSFCallbacks_t * ioCallbacks);
void	DSFWriteToFile(const char * inPath, void * inRef);
void	DSFDestroyWriter(void * inRef);

#endif
