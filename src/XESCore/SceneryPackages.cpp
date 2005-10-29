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

#include "SceneryPackages.h"
#include "XFileTwiddle.h"
#include "PlatformUtils.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "BitmapUtils.h"
#include "EnumSystem.h"
#include "XObjReadWrite.h"
#include "XObjDefs.h"

static void	only_dir(string& iopath)
{
	string::size_type p = iopath.find_last_of("\\/:");
	if (p == iopath.npos) iopath.clear();
	else				  iopath.erase(p+1);
}

static void only_file(string& iopath)
{
	string::size_type p = iopath.find_last_of("\\/:");
	if (p != iopath.npos) iopath.erase(0, p+1);
}

static void canonical_path(string& iopath)
{
	for (int n = 0; n < iopath.size(); ++n)
		if (iopath[n] == ':' || iopath[n] == '\\' || iopath[n] == '/')
			iopath[n] = '/';	
}

static void local_path(string& iopath)
{
#if IBM
	for (int n = 3; n < iopath.size(); ++n)
#else
	for (int n = 0; n < iopath.size(); ++n)
#endif
		if (iopath[n] == ':' || iopath[n] == '\\' || iopath[n] == '/')
			iopath[n] = DIR_CHAR;
}


void	CreateTerrainPackage(const char * inPackage, bool make_stub_pngs)
{
	int 	n;
	string	lib_path, dir_path;
	string 	package(inPackage);
	FILE *	lib;
	FILE *	ter;

	MakeDirExist(package.c_str());

	lib_path = package + "library.txt";
	lib = fopen(lib_path.c_str(), "w");
	
	fprintf(lib,"%c" CRLF "800" CRLF "LIBRARY" CRLF CRLF, APL ? 'A' : 'I');
	
	set<string>	imageFiles;
	set<string> borderFiles;
	
	set<int>	predone;
	
	for (n = 0; n < gNaturalTerrainTable.size(); ++n)
	{
		if (predone.count(gNaturalTerrainTable[n].name)) continue;
		predone.insert(gNaturalTerrainTable[n].name);
		
		fprintf(lib, "EXPORT   lib/g8/%s.ter       %s.ter" CRLF,
			FetchTokenString(gNaturalTerrainTable[n].name),FetchTokenString(gNaturalTerrainTable[n].name));
			
		lib_path = package + FetchTokenString(gNaturalTerrainTable[n].name) + ".ter";		
		local_path(lib_path);		
		dir_path = lib_path;
		only_dir(dir_path);		
		MakeDirExist(dir_path.c_str());

		ter = fopen(lib_path.c_str(), "w");
		fprintf(ter, "%c" CRLF "800" CRLF "TERRAIN" CRLF CRLF, APL ? 'A' : 'I');
		fprintf(ter, "BASE_TEX %s" CRLF, gNaturalTerrainTable[n].base_tex.c_str());
		fprintf(ter, "BORDER_TEX %s" CRLF, gNaturalTerrainTable[n].border_tex.c_str());
		fprintf(ter, "PROJECTED %d %d" CRLF, (int) gNaturalTerrainTable[n].base_res, (int) gNaturalTerrainTable[n].base_res);
//		if (gNaturalTerrainTable[n].base_alpha_invert)
//			fprintf(ter, "BASE_ALPHA_INVERT" CRLF);

//		if (!gNaturalTerrainTable[n].comp_tex.empty())
//		{
//			fprintf(ter, "COMPOSITE_TEX %s" CRLF, gNaturalTerrainTable[n].comp_tex.c_str());
//			fprintf(ter, "COMPOSITE_PROJECTED %d %d" CRLF, (int) gNaturalTerrainTable[n].comp_res, (int) gNaturalTerrainTable[n].comp_res);
//			if (gNaturalTerrainTable[n].comp_alpha_invert)
//				fprintf(ter, "COMPOSITE_ALPHA_INVERT" CRLF);
//		} else {
			fprintf(ter, "NO_ALPHA" CRLF);
//		}

		fprintf(ter, "COMPOSITE_BORDERS" CRLF);
		
		switch(gNaturalTerrainTable[n].variant) {
		case 5:			fprintf(ter, "PROJECT_ANGLE 0 1 0 180" CRLF);		break;
		case 6:			fprintf(ter, "PROJECT_ANGLE 0 1 0 270" CRLF);		break;
		case 7:			fprintf(ter, "PROJECT_ANGLE 0 1 0 0" CRLF);			break;
		case 8:			fprintf(ter, "PROJECT_ANGLE 0 1 0 90" CRLF);		break;
		default: 
			{			
				switch(gNaturalTerrainTable[n].proj_angle) {
				case proj_Down:			fprintf(ter, "PROJECT_ANGLE 0 1 0 0" CRLF);		break;
				case proj_NorthSouth:	fprintf(ter, "PROJECT_ANGLE 0 0 1 0" CRLF);		break;
				case proj_EastWest:		fprintf(ter, "PROJECT_ANGLE -1 0 0 90" CRLF);	break;
				}
				
				switch(gNaturalTerrainTable[n].variant) {
				case 1: fprintf(ter, "PROJECT_OFFSET %d %d" CRLF, (int) (gNaturalTerrainTable[n].base_res * 0.0), (int) (gNaturalTerrainTable[n].base_res * 0.0));	break;
				case 2: fprintf(ter, "PROJECT_OFFSET %d %d" CRLF, (int) (gNaturalTerrainTable[n].base_res * 0.0), (int) (gNaturalTerrainTable[n].base_res * 0.3));	break;
				case 3: fprintf(ter, "PROJECT_OFFSET %d %d" CRLF, (int) (gNaturalTerrainTable[n].base_res * 0.7), (int) (gNaturalTerrainTable[n].base_res * 0.0));	break;
				case 4:	fprintf(ter, "PROJECT_OFFSET %d %d" CRLF, (int) (gNaturalTerrainTable[n].base_res * 0.4), (int) (gNaturalTerrainTable[n].base_res * 0.6));	break;
				}
			}
		}
		
		dir_path = string(FetchTokenString(gNaturalTerrainTable[n].name)) + ".ter";		
		only_dir(dir_path);
		canonical_path(dir_path);

		imageFiles.insert(dir_path+gNaturalTerrainTable[n].base_tex);
//		imageFiles.insert(dir_path+gNaturalTerrainTable[n].comp_tex);
		borderFiles.insert(dir_path+gNaturalTerrainTable[n].border_tex);

		fclose(ter);
	}
	
	fclose(lib);

	if (make_stub_pngs)
	{
		ImageInfo	image_data;
		ImageInfo	border;
			
		CreateNewBitmap(16, 16, 3, &image_data);
		memset(image_data.data, 0x7F, 16 * 16 * 3);

		CreateNewBitmap(128, 4, 1, &border);
		unsigned char * p = border.data;
		for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 128; ++x)		
		{
			*p = ((float) x / 127.0) * 255.0;
			++p;
		}

		for (set<string>::iterator image = imageFiles.begin(); image != imageFiles.end(); ++image)
		if (!image->empty())
		{
			string path = inPackage;
			path += *image;
			for (int n = path.size() - image->size(); n < path.size(); ++n)
			{
				if (path[n] == '/' || path[n] == ':' || path[n] == '\\')
					path[n] = DIR_CHAR;			
			}
			string end_dir = path.substr(0, path.rfind(DIR_CHAR)+1);
			MakeDirExist(end_dir.c_str());
						
			FILE * exists = fopen(path.c_str(), "rb");
			if (exists) 
				fclose(exists);
			else {
				WriteBitmapToPNG(&image_data, path.c_str(), NULL, 0);
			}
		}

		for (set<string>::iterator image = borderFiles.begin(); image != borderFiles.end(); ++image)
		if (!image->empty())
		{
			string path = inPackage;
			path += *image;
			for (int n = path.size() - image->size(); n < path.size(); ++n)
			{
				if (path[n] == '/' || path[n] == ':' || path[n] == '\\')
					path[n] = DIR_CHAR;			
			}
			string end_dir = path.substr(0, path.rfind(DIR_CHAR)+1);
			MakeDirExist(end_dir.c_str());
						
			FILE * exists = fopen(path.c_str(), "rb");
			if (exists) 
				fclose(exists);
			else {
				WriteBitmapToPNG(&border, path.c_str(), NULL, 0);
			}
		}


		DestroyBitmap(&image_data);
		DestroyBitmap(&border);
	}
}

void	CreatePackageForDSF(const char * inPackage, int lon, int lat, char * outDSFDestination)
{
	sprintf(outDSFDestination, "%sEarth nav data" DIR_STR "%+03d%+04d" DIR_STR,
					inPackage, latlon_bucket(lat), latlon_bucket(lon));

	MakeDirExist(outDSFDestination);
	sprintf(outDSFDestination, "%sEarth nav data" DIR_STR "%+03d%+04d" DIR_STR "%+03d%+04d.dsf",
					inPackage, latlon_bucket(lat), latlon_bucket(lon), lat, lon);		
}

bool	SpreadsheetForObject(const char * inObjFile, FILE * outDstLine)
{
	XObj	obj;
	if (!XObjRead(inObjFile, obj)) return false;

	string rawName = inObjFile;
	rawName.erase(0, 1+rawName.rfind(DIR_CHAR));
	rawName.erase(rawName.size()-4);
	
	float	minp[3] = { 9.9e9, 9.9e9, 9.9e9 };
	float	maxp[3] = {-9.9e9,-9.9e9,-9.9e9 };

	for (int c = 0; c < obj.cmds.size(); ++c)
	{
		for (int s = 0; s < obj.cmds[c].st.size(); ++s)
		{
			minp[0] = min(minp[0], obj.cmds[c].st[s].v[0]);
			minp[1] = min(minp[1], obj.cmds[c].st[s].v[1]);
			minp[2] = min(minp[2], obj.cmds[c].st[s].v[2]);
			maxp[0] = max(maxp[0], obj.cmds[c].st[s].v[0]);
			maxp[1] = max(maxp[1], obj.cmds[c].st[s].v[1]);
			maxp[2] = max(maxp[2], obj.cmds[c].st[s].v[2]);
		}
		for (int r = 0; r < obj.cmds[c].rgb.size(); ++r)
		{
			minp[0] = min(minp[0], obj.cmds[c].rgb[r].v[0]);
			minp[1] = min(minp[1], obj.cmds[c].rgb[r].v[1]);
			minp[2] = min(minp[2], obj.cmds[c].rgb[r].v[2]);
			maxp[0] = max(maxp[0], obj.cmds[c].rgb[r].v[0]);
			maxp[1] = max(maxp[1], obj.cmds[c].rgb[r].v[1]);
			maxp[2] = max(maxp[2], obj.cmds[c].rgb[r].v[2]);
		}
	}
	
	fprintf(outDstLine, "OBJ_PROP	NO_VALUE	NO_VALUE	NO_VALUE	NO_VALUE	NO_VALUE	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	100		0			%s				%f	%f	0		0	0		0	0		0	0		0	0" CRLF,
		rawName.c_str(), maxp[0] - minp[0], maxp[2] - minp[2]);
	
	return true;
}
