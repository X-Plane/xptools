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


#include "DSFLib.h"
#include "XChunkyFileUtils.h"
#include <stdio.h>
#include "md5.h"
#include "DSFDefs.h"
#include "DSFPointPool.h"

#if USE_7Z
	#include "7z.h"
	#include "7zAlloc.h"
	#include "7zCrc.h"
	#include "7zFile.h"
	#define kInputBufSize ((size_t)1 << 18)   // 256kB read buffer
#endif

const char *	dsfErrorMessages[] = {
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
	"dsf_ErrPoolOutOfRange",
	"dsf_ErrBadChecksum",
	"dsf_ErrCanceled"
};

// Define this to 1 to have the reader print atoms sizes as it reads for diagnostics
#define PRINT_ATOM_SIZES 0
// Define this to 1 to have the reader print detailed error messages before it returns an error code.
#define DEBUG_MESSAGES 1

// These debug macros are used to swap the headers around.
#if BIG
	#if APL
		#include <libkern/OSByteOrder.h>
		#define SWAP32(x) (OSSwapConstInt32(x))
		#define SWAP16(x) (OSSwapConstInt16(x))
	#else
		#error we do not have big endian support on non-Mac platforms
	#endif
#elif LIL
	#define SWAP32(x) (x)
	#define SWAP16(x) (x)
#else
	#error BIG or LIL are not defined - what endian are we?
#endif

const double recip_65535 = 1.0 / 65535.0;
const double recip_4294967295 = 1.0 / 4294967295.0;


/*double	R_DECODE_SCALED(
						unsigned int index, unsigned int pool, unsigned int plane,
						const vector<vector<unsigned short> >& points, const vector<int>& depths,
						const vector<vector<float> >& scale, const vector<vector<float> >& delta)
{
	double sc = scale[pool][plane];
	double dl = delta[pool][plane];
	double	lon_raw = points[pool][index * depths[pool] + plane];
	if (sc)
		lon_raw = lon_raw * sc * recip_65535 + dl;
	return lon_raw;
}

double	R_DECODE_SCALED32(
						unsigned int index, unsigned int pool, unsigned int plane,
						const vector<vector<unsigned int> >& points, const vector<int>& depths,
						const vector<vector<float> >& scale, const vector<vector<float> >& delta)
{
	double sc = scale[pool][plane];
	double dl = delta[pool][plane];
	double	lon_raw = points[pool][index * depths[pool] + plane];
	if (sc)
		lon_raw = lon_raw * sc * recip_4294967295 + dl;
	return lon_raw;
}*/

#define	DECODE_SCALED(__index, __pool, __points, __depths) 	((&*__points[__pool].begin())+__index * __depths[__pool])

#define	DECODE_SCALED_CURRENT(__index) 									(currentPoolPtr+__index * currentDepth)

#define	DECODE_SCALED32_CURRENT(__index)					 			(currentPoolPtr32 +__index * currentDepth32)


int		DSFReadFile(
			const char *		inPath,  
			void * (*			malloc_func)(size_t s), 
			void (*				free_func)(void * ptr), 
			DSFCallbacks_t *	inCallbacks, 
			const int *			inPasses, 
			void *				inRef)
{
	char *		mem = nullptr;
	size_t		uncomp_size = 0;
	int			result = dsf_ErrOK;
	FILE * 		fi = nullptr;
		
#if USE_7Z
	size_t 		mem_offset = 0;
	size_t		mem_size = 0;
	UInt32		blockIndex = 0;
	bool 		dsf_compressed = true;
		
	CrcGenerateTable();

	CSzArEx 	db;
	SzArEx_Init(&db);

	ISzAlloc allocImp = { SzAlloc, SzFree };           // this is not useing malloc_func and free_func ...
	ISzAlloc allocTempImp = { SzAllocTemp, SzFreeTemp };

	CFileInStream archiveStream;
	CLookToRead2 lookStream;
	
	if (InFile_Open(&archiveStream.file, inPath))
		result = dsf_ErrCouldNotOpenFile;
	else
	{
		FileInStream_CreateVTable(&archiveStream);
		LookToRead2_CreateVTable(&lookStream, False);
		lookStream.buf = (Byte *)ISzAlloc_Alloc(&allocImp, kInputBufSize);
		lookStream.bufSize = kInputBufSize;
		lookStream.realStream = &archiveStream.vt;
		LookToRead2_Init(&lookStream);

		if (SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp))
			dsf_compressed = false;
		else
		{
			// no need to skip over directory-only entries. New api keeps directories vs files separate. So fileIndex = 0 is always the first real file
			if (SzArEx_Extract(&db, &lookStream.vt, 0 , &blockIndex, (Byte **) &mem, &mem_size, &mem_offset, &uncomp_size, &allocImp, &allocTempImp) == 0)
			{
				result = DSFReadMem(mem + mem_offset, mem + mem_offset + uncomp_size, inCallbacks, inPasses, inRef);
			}
			SzArEx_Free(&db, &allocImp);
		}
		ISzAlloc_Free(&allocImp, lookStream.buf);
		File_Close(&archiveStream.file);
	}
	if(dsf_compressed) return result;
#endif
	fi = fopen(inPath, "rb");
	if (!fi) { result = dsf_ErrCouldNotOpenFile; goto bail; }

	fseek(fi, 0L, SEEK_END);
	uncomp_size = ftell(fi);
	fseek(fi, 0L, SEEK_SET);

	mem = (char *) malloc_func(uncomp_size);
	if (!mem) { result = dsf_ErrOutOfMemory; goto bail; }

	if (fread(mem, 1, uncomp_size, fi) != uncomp_size)
		{ result = dsf_ErrCouldNotReadFile; goto bail; }

	result = DSFReadMem(mem + mem_offset, mem + uncomp_size, inCallbacks, inPasses, inRef);

bail:
	if (fi) fclose(fi);
	if (mem) free_func(mem);	
	return result;
}

int		DSFCheckSignature(const char * inPath)
{
	FILE *			fi = NULL;
	char *			mem = NULL;
	char * d, * s; 
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

	s = mem;
	d = mem + file_size - 16;

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
	/* MD5 checksum...*/
	if(inPasses && (inPasses[0] & dsf_CmdSign))
	{
		if((inStop - inStart) < 16)
			return dsf_ErrNoAtoms;
			
		MD5_CTX ctx;
		MD5Init(&ctx);
		
		const char *	s = inStart;
		const char *	d = inStop - 16;
		
		while(s < d)
		{
			int l = d - s;
			if (l > 1024) l = 1024;
			MD5Update(&ctx, (unsigned char *) s, l);
			s += l;
		}
		MD5Final(&ctx);
		if(memcmp(ctx.digest, d, 16) != 0) return dsf_ErrBadChecksum;
	}

	/* Do basic file analysis and check all headers and other basic requirements. */
	const DSFHeader_t * header = (const DSFHeader_t *) inStart;
//	const DSFFooter_t * footer = (const DSFFooter_t *) (inStop - sizeof(DSFFooter_t));
#if BENTODO
someday check footer when in sloooow mode
#endif
	XAtomContainer		dsf_container;
	dsf_container.begin = (char *) (inStart + sizeof(DSFHeader_t));
	dsf_container.end = (char *) (inStop - sizeof(DSFFooter_t));
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

		XAtom			headAtom, 		defnAtom, 		geodAtom,		demsAtom	;
		XAtomContainer	headContainer, 	defnContainer,	geodContainer,	demsContainer;
		XAtomPackedData					cmdsAtom, scalAtom;
		XAtomStringTable				propAtom, tertAtom, objtAtom, polyAtom, netwAtom, demnAtom;
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
	
	bool has_demn = defnContainer.GetNthAtomOfID(dsf_RasterNameAtom, 0, demnAtom);
	

#if PRINT_ATOM_SIZES
	printf("Geo data is	%d bytes.\n", geodAtom.GetContentLength());
	printf("Geo cmd  is	%d bytes.\n", cmdsAtom.GetContentLength());
#endif

	/* Read raw geodata. */

	int n;	//,i,p;
//	vector<vector<unsigned short> >	planarDataRaw;	// Per plane array of shorts
	vector<vector<double> 		>	planarData;		// Per plane array of doubles
	vector<int>						planeDepths;	// Per plane plane count
	vector<int>						planeSizes;		// Per plane length of plane
	vector<vector<double> >			planeScales;	// Per plane scaling factor
	vector<vector<double> >			planeOffsets;	// Per plane offset

//	vector<vector<unsigned int> >	planarData32Raw;// Per plane array of shorts
	vector<vector<double> >			planarData32;	// Per plane array of shorts
	vector<int>						planeDepths32;	// Per plane plane count
	vector<int>						planeSizes32;	// Per plane length of plane
	vector<vector<double> >			planeScales32;	// Per plane scaling factor
	vector<vector<double> >			planeOffsets32;	// Per plane offset


	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointScaleAtom, n++, scalAtom))
	{
		planeScales.push_back(vector<double>());
		planeOffsets.push_back(vector<double>());
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
		planeScales32.push_back(vector<double>());
		planeOffsets32.push_back(vector<double>());
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




	
	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointPoolAtom, n, poolAtom))
	{
		int aSize = poolAtom.GetArraySize();
		int pCount = poolAtom.GetPlaneCount();
		planeDepths.push_back(pCount);
		planeSizes.push_back(aSize);
//		planarDataRaw.push_back(vector<unsigned short>());
//		planarDataRaw.back().resize(aSize * pCount);
		planarData.push_back(vector<double>());
		planarData.back().resize(aSize * pCount);
//		poolAtom.DecompressShort(pCount, aSize, 1, (short *) &*planarDataRaw.back().begin());
		poolAtom.DecompressShortToDoubleInterleaved(pCount, aSize, &*planarData.back().begin(),
					&*planeScales[n].begin(),
					recip_65535,
					&*planeOffsets[n].begin());
		++n;
	}

	n = 0;
	while (geodContainer.GetNthAtomOfID(def_PointPool32Atom, n, poolAtom))
	{
		int aSize = poolAtom.GetArraySize();
		int pCount = poolAtom.GetPlaneCount();
		planeDepths32.push_back(pCount);
		planeSizes32.push_back(aSize);
//		planarData32Raw.push_back(vector<unsigned int>());
//		planarData32Raw.back().resize(aSize * pCount);
		planarData32.push_back(vector<double>());
		planarData32.back().resize(aSize * pCount);
//		poolAtom.DecompressInt(pCount, aSize, 1, (int *) &*planarData32Raw.back().begin());

		poolAtom.DecompressIntToDoubleInterleaved(pCount, aSize, &*planarData32.back().begin(),
					&*planeScales32[n].begin(),
					recip_4294967295,
					&*planeOffsets32[n].begin());


		++n;
	}	
	
	

/*

	for (n = 0; n < planarData.size(); ++n)
	for (p = 0; p < planeDepths[n]; ++p)
	{
		int skip = planeDepths[n];		
		double * dst = &*planarData[n].begin() + p;
		unsigned short * src = &*planarDataRaw[n].begin() + p;
		double sc = planeScales[n][p];
		double dl = planeOffsets[n][p];
		i = planeSizes[n];
		if (sc)
		{
			while (i--)
			{
				*dst = (((double) *src) * sc * recip_65535) + dl;
				dst += skip;
				src += skip;
			}
		} else {
			while (i--)
			{
				*dst = *src;
				dst += skip;
				src += skip;
			}
		}
	}
*/			
/*
	for (n = 0; n < planarData32.size(); ++n)
	for (p = 0; p < planeDepths32[n]; ++p)
	{
		int skip = planeDepths32[n];		
		double * dst = &*planarData32[n].begin() + p;
		unsigned int * src = &*planarData32Raw[n].begin() + p;
		double sc = planeScales32[n][p];
		double dl = planeOffsets32[n][p];
		i = planeSizes32[n];
		if (sc)
		{
			while (i--)
			{
				*dst = (((double) *src) * sc * recip_4294967295) + dl;
				dst += skip;
				src += skip;
			}
		} else {
			while (i--)
			{
				*dst = *src;
				dst += skip;
				src += skip;
			}
		}
	}
	*/
//	planarData32Raw.clear();
//	planarDataRaw.clear();


	
		
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
				if(!inCallbacks->AcceptTerrainDef_f(str, ref))
					return dsf_ErrCanceled;

			for (str = objtAtom.GetFirstString(); str != NULL; str = objtAtom.GetNextString(str))
				if(!inCallbacks->AcceptObjectDef_f(str, ref))
					return dsf_ErrCanceled;				

			for (str = polyAtom.GetFirstString(); str != NULL; str = polyAtom.GetNextString(str))
				if(!inCallbacks->AcceptPolygonDef_f(str, ref))
					return dsf_ErrCanceled;				

			for (str = netwAtom.GetFirstString(); str != NULL; str = netwAtom.GetNextString(str))
				if(!inCallbacks->AcceptNetworkDef_f(str, ref))
					return dsf_ErrCanceled;				

			if(has_demn)
			for (str = demnAtom.GetFirstString(); str != NULL; str = demnAtom.GetNextString(str))
				if(!inCallbacks->AcceptRasterDef_f(str, ref))
				return dsf_ErrCanceled;
			
		}

		if(flags & dsf_CmdRaster)
		{
			if(dsf_container.GetNthAtomOfID(dsf_RasterContainerAtom, 0, demsAtom))
			{
				demsAtom.GetContents(demsContainer);
				XAtom				raster_data;
				XSpan				the_data;
				XAtomPackedData		raster_header;
				
				int r = 0;
				while (demsContainer.GetNthAtomOfID(dsf_RasterInfoAtom, r, raster_header) &&
					   demsContainer.GetNthAtomOfID(dsf_RasterDataAtom, r, raster_data))
				{
					raster_data.GetContents(the_data);
					DSFRasterHeader_t	h;
					
					raster_header.Reset();
					h.version			= raster_header.ReadUInt8  ();
					h.bytes_per_pixel	= raster_header.ReadUInt8  ();
					h.flags				= raster_header.ReadUInt16 ();
					h.width				= raster_header.ReadUInt32 ();
					h.height			= raster_header.ReadUInt32 ();
					h.scale				= raster_header.ReadFloat32();
					h.offset			= raster_header.ReadFloat32();
					
					inCallbacks->AddRasterData_f(&h, the_data.begin, ref);
					
					++r;
				}
				
			}
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
		double *			currentPoolPtr = NULL;
		double *			currentPoolPtr32 = NULL;
		int					currentDepth = -1;
		int					currentDepth32 = -1;


	cmdsAtom.Reset();
	while (!cmdsAtom.Done())
	{
		unsigned int	commentLen;
		unsigned int	index, index1, index2;
		unsigned int	count, counter;

//		double			objCoord3[3];

		double*			segCoord;
		bool			hasCurve;

		unsigned short	polyParam;

//		vector<double>	triCoord;
		int				triCoordDim;
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
				printf("DSF ERROR: Pool out of range at pool select.  Desired = %d.  Normal pools = %zd.  32-bit pools = %zd.\n", 
						currentPool, planarData.size(), planarData32.size());
#endif
				return dsf_ErrPoolOutOfRange;
			}
			
			if (currentPool < planarData.size())	{ currentPoolPtr   = &*planarData  [currentPool].begin(); currentDepth   = planeDepths  [currentPool]; } else currentPoolPtr = NULL;
			if (currentPool < planarData32.size())	{ currentPoolPtr32 = &*planarData32[currentPool].begin(); currentDepth32 = planeDepths32[currentPool]; } else currentPoolPtr32 = NULL;
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
//				objCoord3[0] = DECODE_SCALED_CURRENT(index)[0];
//				objCoord3[1] = DECODE_SCALED_CURRENT(index)[1];
//				objCoord3[2] = DECODE_SCALED_CURRENT(index)[2];
				inCallbacks->AddObject_f(currentDefinition, DECODE_SCALED_CURRENT(index), planeDepths[currentPool], ref);
					}
			break;
		case dsf_Cmd_ObjectRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
				if (flags & dsf_CmdObjects)
			for (index = index1; index < index2; ++index)
			{
//				objCoord3[0] = DECODE_SCALED_CURRENT(index)[0];
//				objCoord3[1] = DECODE_SCALED_CURRENT(index)[1];
//				objCoord3[2] = DECODE_SCALED_CURRENT(index)[2];
				inCallbacks->AddObject_f(currentDefinition, DECODE_SCALED_CURRENT(index), planeDepths[currentPool], ref);
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
					segCoord = DECODE_SCALED32_CURRENT(index);
					if (segCoord[3]) {
					if (counter > 0)
							inCallbacks->EndSegment_f(segCoord, hasCurve, ref);
					if (counter < (count-1))
							inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segCoord, hasCurve, ref);
					} else {
#if DEBUG_MESSAGES
						if(counter == 0 || counter == count-1)
							printf("DSF ERROR: Road contains a shape point for one of it's ends.\n");
#endif			

						inCallbacks->AddSegmentShapePoint_f(segCoord, hasCurve, ref);
			}
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
				segCoord = DECODE_SCALED32_CURRENT(index);
				if (segCoord[3])
				{
					if (index != index1)
						inCallbacks->EndSegment_f(segCoord, hasCurve, ref);
					if (index != (index2-1))
						inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segCoord, hasCurve, ref);					
				} else
					inCallbacks->AddSegmentShapePoint_f(segCoord, hasCurve, ref);
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
					segCoord = DECODE_SCALED32_CURRENT(index);
					if (segCoord[3]) {
					if (counter > 0)
							inCallbacks->EndSegment_f(segCoord, hasCurve, ref);
					if (counter < (count-1))
							inCallbacks->BeginSegment_f(currentDefinition, roadSubtype, segCoord, hasCurve, ref);
				} else
						inCallbacks->AddSegmentShapePoint_f(segCoord, hasCurve, ref);
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
				triCoordDim = planeDepths[currentPool];
			}
			while(count--)
			{
				index = cmdsAtom.ReadUInt16();
				if (dsf_CmdPolys)
				{
					inCallbacks->AddPolygonPoint_f(DECODE_SCALED_CURRENT(index), ref);
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
				triCoordDim = planeDepths[currentPool];
				for (index = index1; index < index2; ++index)
				{
					inCallbacks->AddPolygonPoint_f(DECODE_SCALED_CURRENT(index), ref);
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
			triCoordDim = planeDepths[currentPool];
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
						inCallbacks->AddPolygonPoint_f(DECODE_SCALED_CURRENT(index), ref);
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
			triCoordDim = planeDepths[currentPool];
			while(count--)
			{
				if (flags & dsf_CmdPolys)
					inCallbacks->BeginPolygonWinding_f(ref);
				index2 = cmdsAtom.ReadUInt16();
				if (flags & dsf_CmdPolys)
				{
					for (index = index1; index < index2; ++index)
					{
						inCallbacks->AddPolygonPoint_f(DECODE_SCALED_CURRENT(index), ref);
					}
					inCallbacks->EndPolygonWinding_f(ref);
				}
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
			triCoordDim = planeDepths[currentPool];
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleCrossPool:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_Tri, ref);
			triCoordDim = planeDepths[currentPool];
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange cross-pool.  Desired = %d.  Normal pools = %zd.\n", pool, planarData.size());
#endif
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED(index, pool, planarData, planeDepths), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;

		case dsf_Cmd_TriangleRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
			triCoordDim = planeDepths[currentPool];
				if (flags & dsf_CmdPatches)
				{
			inCallbacks->BeginPrimitive_f(dsf_Tri, ref);
			for (index = index1; index < index2; ++index)
			{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;
		case dsf_Cmd_TriangleStrip					:
			triCoordDim = planeDepths[currentPool];
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleStripCrossPool:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);
			triCoordDim = planeDepths[currentPool];
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange strip cross-pool.  Desired = %d.  Normal pools = %zd.\n", pool, planarData.size());
#endif
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED(index, pool, planarData, planeDepths), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;

		case dsf_Cmd_TriangleStripRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
			triCoordDim = planeDepths[currentPool];
				if (flags & dsf_CmdPatches)
				{
			inCallbacks->BeginPrimitive_f(dsf_TriStrip, ref);
			for (index = index1; index < index2; ++index)
			{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;
		case dsf_Cmd_TriangleFan					:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);

			triCoordDim = planeDepths[currentPool];
			count = cmdsAtom.ReadUInt8();
			for (counter = 0; counter < count; ++counter)
			{
				index = cmdsAtom.ReadUInt16();
					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			break;
		case dsf_Cmd_TriangleFanCrossPool:
				if (flags & dsf_CmdPatches)
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);
			triCoordDim = planeDepths[currentPool];
			count = cmdsAtom.ReadUInt8();
			
			for (counter = 0; counter < count; ++counter)
			{
				pool = cmdsAtom.ReadUInt16();
				if (pool >= planarData.size())
				{
#if DEBUG_MESSAGES
					printf("DSF ERROR: Pool out of range at triange fan cross-pool.  Desired = %d.  Normal pools = %zd.\n", pool, planarData.size());
#endif
					return dsf_ErrPoolOutOfRange;
				}
				index = cmdsAtom.ReadUInt16();

					if (flags & dsf_CmdPatches)
					{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED(index, pool, planarData, planeDepths), ref);
			}
				}
				if (flags & dsf_CmdPatches)
			inCallbacks->EndPrimitive_f(ref);
			
			break;

		case dsf_Cmd_TriangleFanRange				:
			index1 = cmdsAtom.ReadUInt16();
			index2 = cmdsAtom.ReadUInt16();
			triCoordDim = planeDepths[currentPool];
				if (flags & dsf_CmdPatches)
				{
			inCallbacks->BeginPrimitive_f(dsf_TriFan, ref);
			for (index = index1; index < index2; ++index)
			{
					inCallbacks->AddPatchVertex_f(DECODE_SCALED_CURRENT(index), ref);
			}
			inCallbacks->EndPrimitive_f(ref);
				}
			break;



		/**************************************************************************************************************
		 * COMMENT COMMANDS
		 **************************************************************************************************************/
		case dsf_Cmd_Comment8					:
			commentLen = cmdsAtom.ReadUInt8();
			if(commentLen > 1)
			{
				uint16_t ctype = cmdsAtom.ReadUInt16();
				commentLen -= sizeof(ctype);
				if(ctype == dsf_Comment_Filter && commentLen == sizeof(int32_t))
				{
					int32_t filter_idx = cmdsAtom.ReadSInt32();
					commentLen -= sizeof(filter_idx);
					inCallbacks->SetFilter_f(filter_idx, ref);
				}
			}
			cmdsAtom.Advance(commentLen);
			break;
		case dsf_Cmd_Comment16					:
			commentLen = cmdsAtom.ReadUInt16();
			if(commentLen > 1)
			{
				uint16_t ctype = cmdsAtom.ReadUInt16();
				commentLen -= sizeof(ctype);
				if(ctype == dsf_Comment_Filter && commentLen == sizeof(int32_t))
				{
					int32_t filter_idx = cmdsAtom.ReadSInt32();
					commentLen -= sizeof(filter_idx);
					inCallbacks->SetFilter_f(filter_idx, ref);
				}
			}
			cmdsAtom.Advance(commentLen);
			break;
		case dsf_Cmd_Comment32					:
			commentLen = cmdsAtom.ReadUInt32();
			if(commentLen > 1)
			{
				uint16_t ctype = cmdsAtom.ReadUInt16();
				commentLen -= sizeof(ctype);
				if(ctype == dsf_Comment_Filter && commentLen == sizeof(int32_t))
				{
					int32_t filter_idx = cmdsAtom.ReadSInt32();
					commentLen -= sizeof(filter_idx);
					inCallbacks->SetFilter_f(filter_idx, ref);
				}
			}
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
