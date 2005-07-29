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
#ifndef DSFDEFS_H
#define DSFDEFS_H

/* Use chunky file utils because they define some aspects of chunky files! */
#include "XChunkyFileUtils.h"

#if APL
#pragma options align=mac68k
#endif
#if IBM
#pragma pack(push, 2)
#endif

/***********************************************************************
 * DSF FUNDAMENTAL FILE COMPONENTS
 ***********************************************************************/

#define DSF_COOKIE			"XPLNEDSF"		/* Uniquely identifies DSF files.      */
#define DSF_MASTER_VERSION	1				/* Master version of this format is 1. */

/* The DSF header is a cookie and master version, 12 bytes at the beginning
 * of the file. */
struct	DSFHeader_t {
	char	cookie[8];
	int		version;
};	

/* An atom starts with an ID, and then a length including the 8 byte header. */
struct	AtomHeader_t {
	int		id;
	int		size;
};

/* The DSF file ends with a 128-bit foot MD5, signature summarizing the rest
 * of the file. */
struct	DSFFooter_t {
	char	signature[16];
};	

/* DSF uses the standard atoms as defined by our chunky file utils. */
typedef	XAtomHeader_t	DSFAtomHeader_t;

/***********************************************************************
 * DSF ATOM NAMES
 ***********************************************************************/
enum {

	dsf_MetaDataAtom				= 'HEAD',	//	Atom of atoms
		dsf_PropertyAtom			= 'PROP',	//	String table
	dsf_DefinitionsAtom				= 'DEFN',	//	Atom of atoms
		dsf_TerrainTypesAtom		= 'TERT',	//	String table
		dsf_ObjectsAtom				= 'OBJT',	//	String table
		dsf_PolygonAtom				= 'POLY',	//	String table
		dsf_NetworkAtom				= 'NETW',	//	String table
	dsf_GeoDataAtom					= 'GEOD',	//	Atom of atoms
		def_PointPoolAtom			= 'POOL',	//	Planar Numeric 16-bit
		def_PointScaleAtom			= 'SCAL',	//	32-bit scaling values for point pools
		def_PointPool32Atom			= 'PO32',	//	Planar Numeric 16-bit
		def_PointScale32Atom		= 'SC32',	//	32-bit scaling values for point pools
	dsf_CommandsAtom				= 'CMDS'	//	(command structure)

};	

/***********************************************************************
 * DSF COMMAND ENUMERATIONS
 ***********************************************************************/

/* Attributes for patches. */
enum {
	dsf_Flag_Physical			= 1 << 0,	/* Use this patch in testing for ground.  			  */
	dsf_Flag_Overlay			= 1 << 1	/* Overlays another patch, take Z-buffer precautions. */
};
	
/* This is a list of all of the known DSF commands. */
enum {
	dsf_Cmd_Reserved					= 0,

	dsf_Cmd_PoolSelect					= 1,
	dsf_Cmd_JunctionOffsetSelect		= 2,
	dsf_Cmd_SetDefinition8				= 3,
	dsf_Cmd_SetDefinition16				= 4,
	dsf_Cmd_SetDefinition32				= 5,
	dsf_Cmd_SetRoadSubtype8				= 6,
	
	dsf_Cmd_Object						= 7,
	dsf_Cmd_ObjectRange					= 8,
	
	dsf_Cmd_NetworkChain				= 9,
	dsf_Cmd_NetworkChainRange			= 10,
	dsf_Cmd_NetworkChain32				= 11,
	
	dsf_Cmd_Polygon						= 12,
	dsf_Cmd_PolygonRange				= 13,
	dsf_Cmd_NestedPolygon				= 14,
	dsf_Cmd_NestedPolygonRange			= 15,

	dsf_Cmd_TerrainPatch				= 16,
	dsf_Cmd_TerrainPatchFlags			= 17,
	dsf_Cmd_TerrainPatchFlagsLOD		= 18,

	/* Commands 19-22 were removed during DSF spec development. */	
	
	dsf_Cmd_Triangle					= 23,
	dsf_Cmd_TriangleCrossPool			= 24,
	dsf_Cmd_TriangleRange				= 25,
	dsf_Cmd_TriangleStrip				= 26,
	dsf_Cmd_TriangleStripCrossPool		= 27,
	dsf_Cmd_TriangleStripRange			= 28,
	dsf_Cmd_TriangleFan					= 29,
	dsf_Cmd_TriangleFanCrossPool		= 30,
	dsf_Cmd_TriangleFanRange			= 31,
	
	dsf_Cmd_Comment8					= 32,
	dsf_Cmd_Comment16					= 33,
	dsf_Cmd_Comment32					= 34
		
};

#if APL
#pragma options align=reset
#endif
#if IBM
#pragma pack(pop)
#endif


#endif
