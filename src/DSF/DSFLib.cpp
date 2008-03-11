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
 
/*
	TODO - efficient short circuiting of various reads based on passes param to make DSFLib fast for 
	special cases like reading properties only!
 */
 
 
 // BENTODO - sort out interdependence with hl_types.h
 
//#if !defined(APL) && !defined(IBM) && !defined(LIN)
//#include "hl_types.h"
//#endif

#include "DSFLib.h"
#include "XChunkyFileUtils.h"
#include <stdio.h>
#if LIN
#include "md5global.h"
#endif
#include "md5.h"
#include "DSFDefs.h"
#include "DSFPointPool.h"

char *	dsfErrorMessages[] = {
	"dsf_ErrOK",
	"dsf_ErrCouldNotOpenFile",
	"dsf_ErrOutOfMemory",
	"dsf_ErrCouldNotReadFile",
	"dsf_ErrNoAtoms",
	"dsf_ErrBadCookie",
	"dsf_ErrBadVersion",
	"dsf_ErrMissingAtom",
	"dsf_ErrBadProperties",
	"dsf_ErrMisformattedCommandAtom",
	"dsf_ErrMisformattedScalingAtom",
	"dsf_ErrBadCommand",
	"dsf_ErrUserCancel",
	"dsf_ErrPoolOutOfRange"
};

// Define this to 1 to have the reader print atoms sizes as it reads for diagnostics
#define PRINT_ATOM_SIZES 0
// Define this to 1 to have the reader print detailed error messages before it returns an error code.
#define DEBUG_MESSAGES 1

// These debug macros are used to swap the headers around.
#if BIG
	#if APL
		#if defined(__MACH__)
			#include <libkern/OSByteOrder.h>
			#define SWAP32(x) (OSSwapConstInt32(x))
			#define SWAP16(x) (OSSwapConstInt16(x))
		#else
			#include <Endian.h>
			#define SWAP32(x) (Endian32_Swap(x))
			#define SWAP16(x) (Endian16_Swap(x))
		#endif
	#else
		#error we do not have big endian support on non-Mac platforms
	#endif
#elif LIL
	#define SWAP32(x) (x)
	#define SWAP16(x) (x)
#else
	#error BIG or LIL are not defined - what endian are we?
#endif	

inline	double	DECODE_SCALED(
						unsigned int index, unsigned int pool, unsigned int plane, 
						const vector<vector<unsigned short> >& points, const vector<int>& depths,
						const vector<vector<float> >& scale, const vector<vector<float> >& delta)
{
	double sc = scale[pool][plane];
	double dl = delta[pool][plane];
	double	lon_raw = points[pool][index * depths[pool] + plane];
//	printf("     %lf  ->", lon_raw);
	if (sc)
		lon_raw = lon_raw * sc / 65535.0 + dl;
//	printf("     %lf\n", lon_raw);
	return lon_raw;
}

inline	double	DECODE_SCALED32(
						unsigned int index, unsigned int pool, unsigned int plane, 
						const vector<vector<unsigned long> >& points, const vector<int>& depths,
						const vector<vector<float> >& scale, const vector<vector<float> >& delta)
{
	double sc = scale[pool][plane];
	double dl = delta[pool][plane];
	double	lon_raw = points[pool][index * depths[pool] + plane];
//	printf("     %lf  ->", lon_raw);
	if (sc)
		lon_raw = lon_raw * sc / 4294967295.0 + dl;
//	printf("     %lf\n", lon_raw);
	return lon_raw;
}


int		DSFReadFile(const char * inPath, DSFCallbacks_t * inCallbacks, const int * inPasses, void * inRef)
{
	FILE *			fi = NULL;
	char *			mem = NULL;
	unsigned int	file_size = 0;
	int				result = dsf_ErrOK;
	
	fi = fopen(inPath, "rb");
	if (!fi) { result = dsf_ErrCouldNotOpenFile; goto bail; }

	fseek(fi, 0L, SEEK_END);
	file_size = ftell(fi);
	fseek(fi, 0L, SEEK_SET);
	
	mem = (char *) malloc(file_size);
	if (!mem) { result = dsf_ErrOutOfMemory; goto bail; }

	if (fread(mem, 1, file_size, fi) != file_size)
		{ result = dsf_ErrCouldNotReadFile; goto bail; }
	
	result = DSFReadMem(mem, mem + file_size, inCallbacks, inPasses, inRef);
	
bail:
	if (fi) fclose(fi);
	if (mem) free(mem);	
	return result;
}

int		DSFCheckSignature(const char * inPath)
{
	FILE *			fi = NULL;
	char *			mem = NULL;
	unsigned int	file_size = 0;
	int				result = dsf_ErrOK;
	
	fi = fopen(inPath, "rb");
	if (!fi) { result = dsf_ErrCouldNotOpenFile; goto bail; }

	fseek(fi, 0L, SEEK_END);
	file_size = ftell(fi);
	fseek(fi, 0L, SEEK_SET);
	
	mem = (char *) malloc(file_size);
	if (!mem) { result = dsf_ErrOutOfMemory; goto bail; }

	if (fread(mem, 1, file_size, fi) != file_size)
		{ result = dsf_ErrCouldNotReadFile; goto bail; }

	MD5_CTX ctx;
	MD5Init(&ctx);
	
	char *	s = mem;
	char *	d = mem + file_size - 16;
	
	while(s < d)
	{
		int l = d - s;
		if (l > 1024) l = 1024;
		MD5Update(&ctx, (unsigned char *) s, l);
		s += l;
	}
	MD5Final(&ctx);

	if(memcmp(ctx.digest, d, 16) != 0) result = dsf_ErrBadChecksum;
		
bail:
	if (fi) fclose(fi);
	if (mem) free(mem);	
	return result;
}


int		DSFReadMem(const char * inStart, const char * inStop, DSFCallbacks_t * inCallbacks, const int * inPasses, void * ref)
{
	/* Do basic file analysis and check all headers and other basic requirements. */	
	const DSFHeader_t * header = (const DSFHeader_t *) inStart;
	const DSFFooter_t * footer = (const DSFFooter_t *) (inStop - sizeof(DSFFooter_t));
	XAtomContainer		dsf_container;
	dsf_container.begin = (char *) (inStart + sizeof(DSFHeader_t));
	dsf_container.end = (char *) (inStop + sizeof(DSFFooter_t));
	if ((inStart - inStop) < (sizeof(DSFHeader_t) + sizeof(DSFFooter_t)))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: this file appears to not be atomic.\n");
#endif		
		return dsf_ErrNoAtoms;
	}
	if (dsf_container.begin >= dsf_container.end) 
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: this file appears to not to have any atoms.\n");
#endif		
		return dsf_ErrNoAtoms;	
	}	
	if (strncmp(header->cookie, DSF_COOKIE, strlen(DSF_COOKIE)) != 0)
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We hit a bad cookie.  Expected: '%s', got: '%c%c%c%c%c%c%c%c'\n",
			DSF_COOKIE, 
			header->cookie[0],header->cookie[1],header->cookie[2],header->cookie[3],
			header->cookie[4],header->cookie[5],header->cookie[6],header->cookie[7]);
#endif			
		return dsf_ErrBadCookie;		
	}
	if (SWAP32(header->version) != DSF_MASTER_VERSION)
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We hit a bad version.  Expected: %d, got: %d\n", DSF_MASTER_VERSION, header->version);
#endif			
		return dsf_ErrBadVersion;
	}
	
	/* Fetch all atoms. */
	
		XAtom			headAtom, 		defnAtom, 		geodAtom;
		XAtomContainer	headContainer, 	defnContainer,	geodContainer;
		XAtomPackedData					cmdsAtom, scalAtom;
		XAtomStringTable				propAtom, tertAtom, objtAtom, polyAtom, netwAtom;
		XAtomPlanerNumericTable			poolAtom;
	
	if (!dsf_container.GetNthAtomOfID(dsf_MetaDataAtom, 0, headAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the metadata atom.\n");
#endif			
		return	dsf_ErrMissingAtom;
	}
	if (!dsf_container.GetNthAtomOfID(dsf_DefinitionsAtom, 0, defnAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the definitions atom.\n");
#endif			
		return	dsf_ErrMissingAtom;
	}
	if (!dsf_container.GetNthAtomOfID(dsf_GeoDataAtom, 0, geodAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the geodata atom.\n");
#endif			
		return	dsf_ErrMissingAtom;
	}
	if (!dsf_container.GetNthAtomOfID(dsf_CommandsAtom, 0, cmdsAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the commands atom.\n");
#endif			
		return	dsf_ErrMissingAtom;
	}
		
	headAtom.GetContents(headContainer);
	defnAtom.GetContents(defnContainer);
	geodAtom.GetContents(geodContainer);
	
	if (!headContainer.GetNthAtomOfID(dsf_PropertyAtom, 0, propAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the properties atom.\n");
#endif			
		return dsf_ErrMissingAtom;
	}
	if (!defnContainer.GetNthAtomOfID(dsf_TerrainTypesAtom, 0, tertAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the terrain types atom.\n");
#endif			
		return dsf_ErrMissingAtom;		
	}
	if (!defnContainer.GetNthAtomOfID(dsf_ObjectsAtom, 0, objtAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the object defs atom.\n");
#endif			
		return dsf_ErrMissingAtom;		
	}
	if (!defnContainer.GetNthAtomOfID(dsf_PolygonAtom, 0, polyAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the polygon defs atom.\n");
#endif			
		return dsf_ErrMissingAtom;		
	}
	if (!defnContainer.GetNthAtomOfID(dsf_NetworkAtom, 0, netwAtom))
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We are missing the networks atom.\n");
#endif			
		return dsf_ErrMissingAtom;		
	}
	
#if PRINT_ATOM_SIZES
	printf("Geo data is	%d bytes.\n", geodAtom.GetContentLength());
	printf("Geo cmd  is	%d bytes.\n", cmdsAtom.GetContentLength());
#endif	
	
	/* Read raw geodata. */

	int n;
	vector<vector<unsigned short> >	planarData;		// Per plane array of shorts
	vector<int>						planeDepths;	// Per plane plane count
	vector<int>						planeSizes;		// Per plane length of plane
	vector<vector<float> >			planeScales;	// Per plane scaling factor
	vector<vector<float> >			planeOffsets;	// Per plane offset

	vector<vector<unsigned long> >	planarData32;		// Per plane array of shorts
	vector<int>						planeDepths32;	// Per plane plane count
	vector<int>						planeSizes32;		// Per plane length of plane
	vector<vector<float> >			planeScales32;	// Per plane scaling factor
	vector<vector<float> >			planeOffsets32;	// Per plane offset
	
	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointPoolAtom, n++, poolAtom))
	{
		int aSize = poolAtom.GetArraySize();
		int pCount = poolAtom.GetPlaneCount();
		planeDepths.push_back(pCount);
		planeSizes.push_back(aSize);
		planarData.push_back(vector<unsigned short>());
		planarData.back().resize(aSize * pCount);
		if(aSize)
			poolAtom.DecompressShort(pCount, aSize, 1, (short *) &*planarData.back().begin());
//		printf("[%0d] point pool depth %d, count %d\n", n, pCount, aSize);
//		for (int n = 0; n < planarData.back().size(); ++n)
//			printf("  %04X", planarData.back()[n]);
//		printf("\n");
	}

	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointPool32Atom, n++, poolAtom))
	{
		int aSize = poolAtom.GetArraySize();
		int pCount = poolAtom.GetPlaneCount();
		planeDepths32.push_back(pCount);
		planeSizes32.push_back(aSize);
		planarData32.push_back(vector<unsigned long>());
		planarData32.back().resize(aSize * pCount);
		if(aSize)
			poolAtom.DecompressInt(pCount, aSize, 1, (int *) &*planarData32.back().begin());
//		printf("[%0d] point pool 32 depth %d, count %d\n", n, pCount, aSize);
//		for (int n = 0; n < planarData.back().size(); ++n)
//			printf("  %04X", planarData.back()[n]);
//		printf("\n");
	}	
	
	
	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointScaleAtom, n++, scalAtom))
	{
		planeScales.push_back(vector<float>());
		planeOffsets.push_back(vector<float>());
		scalAtom.Reset();
		while (!scalAtom.Done())
		{
			planeScales.back().push_back(scalAtom.ReadFloat32());
			planeOffsets.back().push_back(scalAtom.ReadFloat32());
		}
		if (scalAtom.Overrun())
		{
#if DEBUG_MESSAGES
			printf("DSF ERROR: We overran our 16-bit scaling atom.\n");
#endif		
			return dsf_ErrMisformattedScalingAtom;
		}
	}

	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointScale32Atom, n++, scalAtom))
	{
		planeScales32.push_back(vector<float>());
		planeOffsets32.push_back(vector<float>());
		scalAtom.Reset();
		while (!scalAtom.Done())
		{
			planeScales32.back().push_back(scalAtom.ReadFloat32());
			planeOffsets32.back().push_back(scalAtom.ReadFloat32());
		}
		if (scalAtom.Overrun())
		{
#if DEBUG_MESSAGES
			printf("DSF ERROR: We overran our 32-bit scaling atom.\n");
#endif		
			return dsf_ErrMisformattedScalingAtom;
		}
	}
		
	const char * str;
	int	pass_number = 0;
	if (inPasses == NULL)
	{
		static int once[2] = { dsf_CmdAll, 0 };
		inPasses = once;
	}
	
	while (inPasses[pass_number])
	{
		int flags = inPasses[pass_number];
		
		if (flags & dsf_CmdProps)
		{
	/* Read Properties. */
	for (str = propAtom.GetFirstString(); str != NULL; str = propAtom.GetNextString(str))
	{
		const char * str2 = propAtom.GetNextString(str);
		if (str2 == NULL) 
		{
#if DEBUG_MESSAGES
			printf("DSF ERROR: We have an odd number of property strings.  The overhanging property is: %s\n", str);
#endif		
			return dsf_ErrBadProperties;
		}
		inCallbacks->AcceptProperty_f(str, str2, ref);
		str = str2;
	}
	}
	
		if (flags & dsf_CmdDefs)
		{
	/* Send definitions. */
	
	for (str = tertAtom.GetFirstString(); str != NULL; str = tertAtom.GetNextString(str))
		inCallbacks->AcceptTerrainDef_f(str, ref);
	for (str = objtAtom.GetFirstString(); str != NULL; str = objtAtom.GetNextString(str))
		inCallbacks->AcceptObjectDef_f(str, ref);
	for (str = polyAtom.GetFirstString(); str != NULL; str = polyAtom.GetNextString(str))
		inCallbacks->AcceptPolygonDef_f(str, ref);
	for (str = netwAtom.GetFirstString(); str != NULL; str = netwAtom.GetNextString(str))
		inCallbacks->AcceptNetworkDef_f(str, ref);
		}

	/* Now we're ready to do the commands. */	
	
		unsigned int		currentDefinition = 0xFFFFFFFF;
		unsigned int		roadSubtype = 0xFFFFFFFF;
		unsigned short		currentPool = 0xFFFF;
		unsigned int		junctionOffset = 0xFFFFFFFF;
		double				patchLODNear = -1.0;
		double				patchLODFar = -1.0;
		unsigned char		patchFlags = 0xFF;
		bool				patchOpen = false;

	cmdsAtom.Reset();
	while (!cmdsAtom.Done())
	{
		unsigned int	commentLen;
		unsigned int	index, index1, index2;
		unsigned int	count, counter, dim;
		
		double			objCoord2[2], objRotation;
			
		double			segCoord6[6];
		unsigned int	segID;
		bool			hasCurve;
		
		unsigned short	polyParam;
		
		vector<double>	triCoord;
		unsigned short	pool;
		
		unsigned char	cmdID = cmdsAtom.ReadUInt8();
		switch(cmdID) {

		
		/**************************************************************************************************************
		 * STATE COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_Reserved					:
#if DEBUG_MESSAGES
			printf("DSF ERROR: We hit the reserved command.\n");
#endif		
			return dsf_ErrBadCommand;
		case dsf_Cmd_PoolSelect					:
			currentPool = cmdsAtom.ReadUInt16();
			if (currentPool >= planarData.size() && currentPool >= planarData32.size())
			{
#if DEBUG_MESSAGES
				printf("DSF ERROR: Pool out of range at pool select.  Desired = %d.  Normal pools = %d.  32-bit pools = %d.\n", 
						currentPool, planarData.size(), planarData32.size());
#endif			
				return dsf_ErrPoolOutOfRange;
			}			
			break;
		case dsf_Cmd_JunctionOffsetSelect		:
			junctionOffset = cmdsAtom.ReadUInt32();
			break;
		case dsf_Cmd_SetDefinition8				:
			currentDefinition = cmdsAtom.ReadUInt8();
			break;
		case dsf_Cmd_SetDefinition16			:
			currentDefinition = cmdsAtom.ReadUInt16();
			break;
		case dsf_Cmd_SetDefinition32			:
			currentDefinition = cmdsAtom.ReadUInt32();
			break;	
		case dsf_Cmd_SetRoadSubtype8:
			roadSubtype = cmdsAtom.ReadUInt8();
			break;	




		/**************************************************************************************************************
		 * OBJECT COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_Object						:
			index = cmdsAtom.ReadUInt16();
				if (flags & dsf_CmdObjects)
				{
			objCoord2[0] = DECODE_SCALED(index, currentPool, 0, planarData, planeDepths, planeScales, planeOffsets);
			objCoord2[1] = DECODE_SCALED(index, currentPool, 1, planarData, planeDepths, planeScales, planeOffsets);
			objRotation =  DECODE_SCALED(index, currentPool, 2, planarData, planeDepths, planeScales, planeOffsets);
			inCallbacks->AddObject_f(currentDefinition, objCoord2, objRotation, ref);
				}
			break;
		case dsf_Cmd_ObjectRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
				if (flags & dsf_CmdObjects)
			for (index = index1; index < index2; ++index)
			{
				objCoord2[0] = DECODE_SCALED(index, currentPool, 0, planarData, planeDepths, planeScales, planeOffsets);
				objCoord2[1] = DECODE_SCALED(index, currentPool, 1, planarData, planeDepths, planeScales, planeOffsets);
				objRotation =  DECODE_SCALED(index, currentPool, 2, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddObject_f(currentDefinition, objCoord2, objRotation, ref);
			}
			break;
		
		
		
		/**************************************************************************************************************
		 * NETWORK COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_NetworkChain				:
			count = cmdsAtom.ReadUInt8();
			hasCurve = planeDepths32[currentPool] >= 7;
			for (counter = 0; counter < count; ++counter)
			{
				index = junctionOffset + cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdVectors)
					{
				segCoord6[0] = DECODE_SCALED32(index, currentPool, 0, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[1] = DECODE_SCALED32(index, currentPool, 1, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[2] = DECODE_SCALED32(index, currentPool, 2, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segID = 	   DECODE_SCALED32(index, currentPool, 3, planarData32, planeDepths32, planeScales32, planeOffsets32);
				if (hasCurve) {
					segCoord6[3] = DECODE_SCALED32(index, currentPool, 4, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[4] = DECODE_SCALED32(index, currentPool, 5, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[5] = DECODE_SCALED32(index, currentPool, 6, planarData32, planeDepths32, planeScales32, planeOffsets32);
				}
				if (segID) {
					if (counter > 0)
						inCallbacks->EndSegment_f(segID, segCoord6, hasCurve, ref);
					if (counter < (count-1))
						inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segID, segCoord6, hasCurve, ref);
				} else 
					inCallbacks->AddSegmentShapePoint_f(segCoord6, hasCurve, ref);
			}			
				}			
			break;		
		case dsf_Cmd_NetworkChainRange			:
			index1 = junctionOffset + cmdsAtom.ReadUInt16();
			index2 = junctionOffset + cmdsAtom.ReadUInt16();
			hasCurve = planeDepths32[currentPool] >= 7;			
				if (flags & dsf_CmdVectors)	
			for (index = index1; index < index2; ++index)
			{
				segCoord6[0] = DECODE_SCALED32(index, currentPool, 0, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[1] = DECODE_SCALED32(index, currentPool, 1, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[2] = DECODE_SCALED32(index, currentPool, 2, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segID = 	   DECODE_SCALED32(index, currentPool, 3, planarData32, planeDepths32, planeScales32, planeOffsets32);
				if (hasCurve) {
					segCoord6[3] = DECODE_SCALED32(index, currentPool, 4, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[4] = DECODE_SCALED32(index, currentPool, 5, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[5] = DECODE_SCALED32(index, currentPool, 6, planarData32, planeDepths32, planeScales32, planeOffsets32);
				}
				if (segID)
				{
					if (index != index1)
						inCallbacks->EndSegment_f(segID, segCoord6, hasCurve, ref);
					if (index != (index2-1))
						inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segID, segCoord6, hasCurve, ref);					
				} else
					inCallbacks->AddSegmentShapePoint_f(segCoord6, hasCurve, ref);
			}
			break;
		case dsf_Cmd_NetworkChain32		:
			count = cmdsAtom.ReadUInt8();
			hasCurve = planeDepths32[currentPool] >= 7;
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt32();
					if (flags & dsf_CmdVectors)
					{
				segCoord6[0] = DECODE_SCALED32(index, currentPool, 0, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[1] = DECODE_SCALED32(index, currentPool, 1, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segCoord6[2] = DECODE_SCALED32(index, currentPool, 2, planarData32, planeDepths32, planeScales32, planeOffsets32);
				segID = 	   DECODE_SCALED32(index, currentPool, 3, planarData32, planeDepths32, planeScales32, planeOffsets32);
				if (hasCurve) {
					segCoord6[3] = DECODE_SCALED32(index, currentPool, 4, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[4] = DECODE_SCALED32(index, currentPool, 5, planarData32, planeDepths32, planeScales32, planeOffsets32);
					segCoord6[5] = DECODE_SCALED32(index, currentPool, 6, planarData32, planeDepths32, planeScales32, planeOffsets32);
				}
				if (segID) {
					if (counter > 0)
						inCallbacks->EndSegment_f(segID, segCoord6, hasCurve, ref);
					if (counter < (count-1))
						inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segID, segCoord6, hasCurve, ref);
				} else 
					inCallbacks->AddSegmentShapePoint_f(segCoord6, hasCurve, ref);
			}			
				}			
			break;		




		/**************************************************************************************************************
		 * POLYGON COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_Polygon:
			polyParam = cmdsAtom.ReadUInt16();
			count = cmdsAtom.ReadUInt8();
			if (flags & dsf_CmdPolys)
			{
				inCallbacks->BeginPolygon_f(currentDefinition, polyParam, planeDepths[currentPool], ref);
				inCallbacks->BeginPolygonWinding_f(ref);
				triCoord.resize(planeDepths[currentPool]);
			}
			while(count--)
			{
				index = cmdsAtom.ReadUInt16();
				if (dsf_CmdPolys)
				{
					for (dim = 0; dim < triCoord.size(); ++dim)
						triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);					
					inCallbacks->AddPolygonPoint_f(&*triCoord.begin(), ref);
				}
			}
			if (flags & dsf_CmdPolys)
			{
				inCallbacks->EndPolygonWinding_f(ref);
				inCallbacks->EndPolygon_f(ref);
			}
			break;
			
		case dsf_Cmd_PolygonRange:
			polyParam = cmdsAtom.ReadUInt16();
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
			if (flags & dsf_CmdPolys)
			{
				inCallbacks->BeginPolygon_f(currentDefinition, polyParam, planeDepths[currentPool], ref);
				inCallbacks->BeginPolygonWinding_f(ref);
				triCoord.resize(planeDepths[currentPool]);
				for (index = index1; index < index2; ++index)
				{
					for (dim = 0; dim < triCoord.size(); ++dim)
						triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
					inCallbacks->AddPolygonPoint_f(&*triCoord.begin(), ref);
				}
				inCallbacks->EndPolygonWinding_f(ref);
				inCallbacks->EndPolygon_f(ref);
			}
			break;
			
		case dsf_Cmd_NestedPolygon:
			polyParam = cmdsAtom.ReadUInt16();
			count = cmdsAtom.ReadUInt8();
			if (flags & dsf_CmdPolys)					
				inCallbacks->BeginPolygon_f(currentDefinition, polyParam, planeDepths[currentPool], ref);
			triCoord.resize(planeDepths[currentPool]);
			while(count--)
			{
				if (flags & dsf_CmdPolys)
					inCallbacks->BeginPolygonWinding_f(ref);
				counter = cmdsAtom.ReadUInt8();
				while (counter--)
				{
					index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPolys)
					{
						for (dim = 0; dim < triCoord.size(); ++dim)
							triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
					inCallbacks->AddPolygonPoint_f(&*triCoord.begin(), ref);
					}
				}
				if (flags & dsf_CmdPolys)
					inCallbacks->EndPolygonWinding_f(ref);
			}
			if (flags & dsf_CmdPolys)
				inCallbacks->EndPolygon_f(ref);
			break;						
			
		case dsf_Cmd_NestedPolygonRange:
			polyParam = cmdsAtom.ReadUInt16();
			count = cmdsAtom.ReadUInt8();
			index1 = cmdsAtom.ReadUInt16();
			if (flags & dsf_CmdPolys)
				inCallbacks->BeginPolygon_f(currentDefinition, polyParam, planeDepths[currentPool], ref);
			triCoord.resize(planeDepths[currentPool]);
			while(count--)
			{
				if (flags & dsf_CmdPolys)
					inCallbacks->BeginPolygonWinding_f(ref);			
				index2 = cmdsAtom.ReadUInt16();
				if (flags & dsf_CmdPolys)					
					for (index = index1; index < index2; ++index)
					{
						for (dim = 0; dim < triCoord.size(); ++dim)
							triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
						inCallbacks->AddPolygonPoint_f(&*triCoord.begin(), ref);
					}
				if (flags & dsf_CmdPolys)					
					inCallbacks->EndPolygonWinding_f(ref);				
				index1 = index2;
			}
			if (flags & dsf_CmdPolys)				
				inCallbacks->EndPolygon_f(ref);
			break;
		
		
		
		
		/**************************************************************************************************************
		 * TERRAIN COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_TerrainPatch				:
				if (flags & dsf_CmdPatches)
				{
			if (patchOpen) inCallbacks->EndPatch_f(ref);
			inCallbacks->BeginPatch_f(currentDefinition, patchLODNear, patchLODFar, patchFlags, planeDepths[currentPool], ref);
				}
			patchOpen = true;
			break;
		case dsf_Cmd_TerrainPatchFlags			:
				if (flags & dsf_CmdPatches)
			if (patchOpen) inCallbacks->EndPatch_f(ref);
			patchFlags = cmdsAtom.ReadUInt8();
				if (flags & dsf_CmdPatches)				
			inCallbacks->BeginPatch_f(currentDefinition, patchLODNear, patchLODFar, patchFlags, planeDepths[currentPool], ref);
			patchOpen = true;
			break;
		case dsf_Cmd_TerrainPatchFlagsLOD		:
				if (flags & dsf_CmdPatches)
			if (patchOpen) inCallbacks->EndPatch_f(ref);
			patchFlags = cmdsAtom.ReadUInt8();
			patchLODNear = cmdsAtom.ReadFloat32();
			patchLODFar = cmdsAtom.ReadFloat32();
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPatch_f(currentDefinition, patchLODNear, patchLODFar, patchFlags, planeDepths[currentPool], ref);
			patchOpen = true;
			break;		

		
		case dsf_Cmd_Triangle					:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_Tri, ref);		
			triCoord.resize(planeDepths[currentPool]);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleCrossPool:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_Tri, ref);		
			triCoord.resize(planeDepths[currentPool]);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange cross-pool.  Desired = %d.  Normal pools = %d.\n", pool, planarData.size());
#endif							
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, pool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;

		case dsf_Cmd_TriangleRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();		
				triCoord.resize(planeDepths[currentPool]);
				if (flags & dsf_CmdPatches)						
				{	
			inCallbacks->BeginPrimitive_f(dsf_Tri, ref);
			for (index = index1; index < index2; ++index)
			{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;
		case dsf_Cmd_TriangleStrip					:
				triCoord.resize(planeDepths[currentPool]);
				if (flags & dsf_CmdPatches)										
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);		
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)						
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)						
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleStripCrossPool:
				if (flags & dsf_CmdPatches)						
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);		
			triCoord.resize(planeDepths[currentPool]);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange strip cross-pool.  Desired = %d.  Normal pools = %d.\n", pool, planarData.size());
#endif			
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)						
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, pool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)						
			inCallbacks->EndPrimitive_f(ref);
			break;

		case dsf_Cmd_TriangleStripRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();		
				triCoord.resize(planeDepths[currentPool]);
				if (flags & dsf_CmdPatches)						
				{				
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);
			for (index = index1; index < index2; ++index)
			{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;
		case dsf_Cmd_TriangleFan					:
				if (flags & dsf_CmdPatches)						
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);		
			triCoord.resize(planeDepths[currentPool]);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)						
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)						
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleFanCrossPool:
				if (flags & dsf_CmdPatches)						
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);		
			triCoord.resize(planeDepths[currentPool]);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange fan cross-pool.  Desired = %d.  Normal pools = %d.\n", pool, planarData.size());
#endif			
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)						
					{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, pool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
				}
				if (flags & dsf_CmdPatches)						
			inCallbacks->EndPrimitive_f(ref);
			break;

		case dsf_Cmd_TriangleFanRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();		
				triCoord.resize(planeDepths[currentPool]);
				if (flags & dsf_CmdPatches)						
				{
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);
			for (index = index1; index < index2; ++index)
			{
				for (dim = 0; dim < triCoord.size(); ++dim)
					triCoord[dim] = DECODE_SCALED(index, currentPool, dim, planarData, planeDepths, planeScales, planeOffsets);
				inCallbacks->AddPatchVertex_f(&*triCoord.begin(), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;	


			
		/**************************************************************************************************************
		 * COMMENT COMMANDS
		 **************************************************************************************************************/			
		case dsf_Cmd_Comment8					:
			commentLen = cmdsAtom.ReadUInt8();
			cmdsAtom.Advance(commentLen);
			break;
		case dsf_Cmd_Comment16					:
			commentLen = cmdsAtom.ReadUInt16();
			cmdsAtom.Advance(commentLen);
			break;
		case dsf_Cmd_Comment32					:
			commentLen = cmdsAtom.ReadUInt32();
			cmdsAtom.Advance(commentLen);
			break;
		default:
#if DEBUG_MESSAGES
		printf("DSF ERROR: We have an unknown command 0x%02X\n", cmdID);
#endif		
			return dsf_ErrBadCommand;
		}				
	}	
	if (patchOpen) inCallbacks->EndPatch_f(ref);

	if (cmdsAtom.Overrun())
	{
#if DEBUG_MESSAGES
		printf("DSF ERROR: We overran the command atom.\n");
#endif		
		return dsf_ErrMisformattedCommandAtom;
		}


		if (!inCallbacks->NextPass_f(pass_number, ref))
			return dsf_ErrUserCancel;

		++pass_number;
	}
	
	return dsf_ErrOK;	
}

#pragma mark -
