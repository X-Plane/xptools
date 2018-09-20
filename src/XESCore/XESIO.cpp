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
#include "XESIO.h"
#include "AptIO.h"
#include "MeshIO.h"
#include "XChunkyFileUtils.h"
#include "SimpleIO.h"
#include "EnumSystem.h"
#include "GISTool_Globals.h"
#include "FileUtils.h"
#include "MeshAlgs.h" // for verify_triangulation_bounds()
#include "MapAlgs.h" // for verify_map_bounds()

const	int	kMapID = 'MAP1';
const	int	kDemDirID = 'DEMd';
const	int	kMeshID = 'MSH1';

const	int	kTokensID = 'TOKN';

const	int	kAptID = 'aptD';

bool	WriteXESFile(
				const char *	inFileName,
				const Pmwx&		inMap,
					  CDT&		inMesh,
				DEMGeoMap&		inDEM,
				const AptVector& inApts,
				ProgressFunc	inFunc)
{
	// Create directories "recursively" up to this path
	const string containing_dir = FILE_get_dir_name(inFileName);
	FILE_make_dir_exist(containing_dir.c_str());
	
	FILE * fi = fopen(inFileName, "wb");
	if (!fi) return false;

	WriteEnumsAtomToFile(fi, gTokens, kTokensID);
	WriteMap(fi, inMap, inFunc, kMapID);
	WriteMesh(fi, inMesh, kMeshID, inFunc);

	{
		StAtomWriter	demDir(fi, kDemDirID);
		FileWriter		writer(fi);
		writer.WriteInt(inDEM.size());
		for (DEMGeoMap::iterator dem = inDEM.begin(); dem != inDEM.end(); ++dem)
			writer.WriteInt(dem->first);
	}

	if (!inApts.empty())
	{
		StAtomWriter	aptDir(fi, kAptID);
		WriteAptFileOpen(fi, inApts, LATEST_APT_VERSION);
	}

	for (DEMGeoMap::iterator dem = inDEM.begin(); dem != inDEM.end(); ++dem)
	{
		StAtomWriter	demAtom(fi, dem->first);
		FileWriter		writer(fi);
		WriteDEM(dem->second, &writer);
	}

	fclose(fi);
	return true;
}

void	ReadXESFile(
				MFMemFile *		inFile,
				Pmwx *			inMap,
				CDT *			inMesh,
				DEMGeoMap *		inDEM,
				AptVector *		inApts,
				ProgressFunc	inFunc)
{
	if (inApts) inApts->clear();

	XAtomContainer	container;
	container.begin = (char *) MemFile_GetBegin(inFile);
	container.end = (char *) MemFile_GetEnd(inFile);

	TokenMap			fileTokens;
	TokenConversionMap 	conversionMap;

	ReadEnumsAtomFromFile(container, fileTokens, kTokensID);
	BuildTokenConversionMap(gTokens, fileTokens, conversionMap);

	XAtom	mapAtom, demAtom, demDirAtom, aptAtom;
	XSpan	mapAtomData, demAtomData, demDirAtomData, aptAtomData;

	vector<int>	dems;

	if (inMap)
	ReadMap(container, *inMap, inFunc, kMapID, conversionMap);

	if (inMesh)
	ReadMesh(container, *inMesh, kMeshID, conversionMap, inFunc);

	if (container.GetNthAtomOfID(kDemDirID, 0, demDirAtom))
	{
		demDirAtom.GetContents(demDirAtomData);
		MemFileReader	reader(demDirAtomData.begin, demDirAtomData.end);
		int count, demID;
		reader.ReadInt(count);
		while (count--)
		{
			reader.ReadInt(demID);
			dems.push_back(demID);
		}
	}

	if (inApts)
	if (container.GetNthAtomOfID(kAptID, 0, aptAtom))
	{
		aptAtom.GetContents(aptAtomData);
		ReadAptFileMem(aptAtomData.begin, aptAtomData.end, *inApts);
	}

	if (inDEM)
	for (int i = 0; i < dems.size(); ++i)
	{
		if (container.GetNthAtomOfID(dems[i], 0, demAtom))
		{
			demAtom.GetContents(demAtomData);
			MemFileReader	reader(demAtomData.begin, demAtomData.end);
			DEMGeo	aDem;
			ReadDEM(aDem, &reader);
			int demID = conversionMap[dems[i]];
			if (demID == dem_LandUse || demID == dem_Climate)	// || demID == dem_NudeColor)
				RemapEnumDEM(aDem, conversionMap);
			//inDEM->insert(DEMGeoMap::value_type(demID, aDem));

			// Massage the XES data to support old-ass Mobile config files
			// Legacy Mobile code reads in all its climate data to the dem_Climate enum,
			// but the modern codebase expects it in dem_ClimStyle, so... make the switch!
			if(gMobile && demID == dem_Climate)
				demID = dem_ClimStyle;

			(*inDEM)[demID] = aDem;
		}
	}
	verify_triangulation_bounds(gDem[dem_Elevation], gTriangulationHi);
	verify_map_bounds();
}
