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
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "BitmapUtils.h"
#include "EnumSystem.h"
#include "XObjReadWrite.h"
#include "XObjDefs.h"
#include <errno.h>

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


int	CreateTerrainPackage(const char * inPackage, bool make_stub_pngs)
{
	int 	n;
	string	lib_path, dir_path;
	string 	package(inPackage);
	FILE *	lib;
	FILE *	ter;
	FILE *	pol;
	int image_ctr = 0, border_ctr = 0;
	int		e;
	e = FILE_make_dir_exist(package.c_str());
	if(e != 0)
	{
		fprintf(stderr,"Could not make directory %s: %d.\n",package.c_str(), e); 
		return e;
	}

	lib_path = package + "library.txt";
	lib = fopen(lib_path.c_str(), "w");
	if(lib == NULL)
	{
		fprintf(stderr,"Could not make file %s.\n", lib_path.c_str());
		return errno;
	}

	fprintf(lib,"%c" CRLF "800" CRLF "LIBRARY" CRLF CRLF, APL ? 'A' : 'I');

	set<string>	imageFiles;
	set<string> borderFiles;

	for (NaturalTerrainInfoMap::iterator n = gNaturalTerrainInfo.begin(); n != gNaturalTerrainInfo.end(); ++n)
	{
		string	pol_name(FetchTokenString(n->first));
		string::size_type d = pol_name.find('/');
		DebugAssert(d != pol_name.npos);
		pol_name.erase(0,d);
		pol_name.insert(0,"pol");

		fprintf(lib, "EXPORT   lib/g8/%s.ter       %s.ter" CRLF,
			FetchTokenString(n->first),FetchTokenString(n->first));

		lib_path = package + FetchTokenString(n->first) + ".ter";
		local_path(lib_path);
		dir_path = lib_path;
		only_dir(dir_path);
		e = FILE_make_dir_exist(dir_path.c_str());		
		if(e != 0)
		{
			fprintf(stderr,"Could not make directory %s: %d.\n",dir_path.c_str(), e); 
			return e;
		}
		ter = fopen(lib_path.c_str(), "w");
		if(ter == NULL)
		{
			fprintf(stderr,"Could not make file %s.\n", lib_path.c_str());
			return errno;
		}
		fprintf(ter, "%c" CRLF "800" CRLF "TERRAIN" CRLF CRLF, APL ? 'A' : 'I');
		fprintf(ter, "BASE_TEX %s" CRLF, n->second.base_tex.c_str());
		if (!n->second.lit_tex.empty())
			fprintf(ter, "LIT_TEX %s" CRLF, n->second.lit_tex.c_str());
		fprintf(ter, "BORDER_TEX %s" CRLF, n->second.border_tex.c_str());
		fprintf(ter, "PROJECTED %d %d" CRLF, (int) n->second.base_res.x(), (int) n->second.base_res.y());

		dir_path = string(FetchTokenString(n->first)) + ".ter";
		only_dir(dir_path);
		canonical_path(dir_path);

		switch(n->second.shader) {
		case shader_vary:	
			if(!n->second.compo_tex.empty())
				fprintf(ter, "COMPOSITE_TEX %s" CRLF, n->second.compo_tex.c_str());
			fprintf(ter, "AUTO_VARY" CRLF);	
			break;
		case shader_tile:	
			if(!n->second.compo_tex.empty())
				fprintf(ter, "COMPOSITE_TEX %s" CRLF, n->second.compo_tex.c_str());
			fprintf(ter, "AUTO_TILE %d %d" CRLF, n->second.tiles_x, n->second.tiles_y);	
			break;
		case shader_slope:	
		case shader_slope2:	
			fprintf(ter, n->second.shader == shader_slope2 ? ("AUTO_SLOPE_HEADING" CRLF) : ("AUTO_SLOPE" CRLF));
			fprintf(ter,"AUTO_SLOPE_HILL %d %d %f %f %s" CRLF, (int) n->second.cliff_info.hill_res.x(), (int) n->second.cliff_info.hill_res.y(), n->second.cliff_info.hill_angle1, n->second.cliff_info.hill_angle2, n->second.cliff_info.hill_tex.c_str());
			fprintf(ter,"AUTO_SLOPE_CLIFF %d %d %f %f %s" CRLF, (int) n->second.cliff_info.cliff_res.x(), (int) n->second.cliff_info.cliff_res.y(), n->second.cliff_info.cliff_angle1, n->second.cliff_info.cliff_angle2, n->second.cliff_info.cliff_tex.c_str());
			imageFiles.insert(dir_path+n->second.cliff_info.hill_tex);
			imageFiles.insert(dir_path+n->second.cliff_info.cliff_tex);
			break;
		case shader_heading:
			fprintf(ter, "AUTO_HEADING" CRLF);
			break;
		case  shader_normal:
			if(!n->second.compo_tex.empty())
				printf("WARNING: terrain %s has unneeded compo tex.\n", FetchTokenString(n->first));
			break;
		default:
			printf("WARNING: terrain %s has unknown shader type.\n",FetchTokenString(n->first));
			break;
		}

		fprintf(ter, "NO_ALPHA" CRLF);

		fprintf(ter, "COMPOSITE_BORDERS" CRLF);

		imageFiles.insert(dir_path+n->second.base_tex);
		if (!n->second.compo_tex.empty())	imageFiles.insert(dir_path+n->second.compo_tex);
		if (!n->second.lit_tex.empty())	imageFiles.insert(dir_path+n->second.lit_tex);
		borderFiles.insert(dir_path+n->second.border_tex);

		fclose(ter);
	}

	for (NaturalTerrainInfoMap::iterator p = gNaturalTerrainInfo.begin(); p != gNaturalTerrainInfo.end(); ++p)
	{
		fprintf(lib, "EXPORT   lib/g8/%s.pol       %s.pol" CRLF, FetchTokenString(p->first), FetchTokenString(p->first));

		lib_path = package + FetchTokenString(p->first) + ".pol";
		local_path(lib_path);
		dir_path = lib_path;
		only_dir(dir_path);
		e = FILE_make_dir_exist(dir_path.c_str());
		if(e != 0)
		{
			fprintf(stderr,"Could not make directory %s: %d.\n",dir_path.c_str(), e); 
			return e;
		}

		pol = fopen(lib_path.c_str(), "w");
		if(pol == NULL)
		{
			fprintf(stderr,"Could not make file %s.\n", lib_path.c_str());
			return errno;
		}
		fprintf(pol, "%c" CRLF "850" CRLF "DRAPED_POLYGON" CRLF CRLF, APL ? 'A' : 'I');
		fprintf(pol, "TEXTURE %s" CRLF, p->second.base_tex.c_str());
		if (!p->second.lit_tex.empty())
			fprintf(pol, "TEXTURE_LIT %s" CRLF, p->second.lit_tex.c_str());

		fprintf(pol, "SCALE %d %d" CRLF, (int) p->second.base_res.x(), (int) p->second.base_res.y());
		fprintf(pol, "NO_ALPHA" CRLF);
		fprintf(pol, "SURFACE dirt" CRLF);
		fprintf(pol, "LAYER_GROUP airports -1" CRLF);
		fclose(pol);
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
			e = FILE_make_dir_exist(end_dir.c_str());
			if(e != 0)
			{
				fprintf(stderr,"Could not make directory %s: %d.\n",dir_path.c_str(), e); 
				return e;
			}

			string path_as_png(path);	path_as_png.erase(path_as_png.length()-3,3);	path_as_png.insert(path_as_png.length(),"png");
			string path_as_dds(path);	path_as_dds.erase(path_as_dds.length()-3,3);	path_as_dds.insert(path_as_dds.length(),"dds");

			if (!FILE_exists(path_as_png.c_str()))
			if (!FILE_exists(path_as_dds.c_str()))
			{
				++image_ctr;
				WriteBitmapToPNG(&image_data, path_as_png.c_str(), NULL, 0);		
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
			e = FILE_make_dir_exist(end_dir.c_str());
			if(e != 0)
			{
				fprintf(stderr,"Could not make directory %s: %d.\n",end_dir.c_str(), e); 
				return e;
			}

			if(!FILE_exists(path.c_str()))
			{
				++border_ctr;
				WriteBitmapToPNG(&border, path.c_str(), NULL, 0);
			}
		}


		DestroyBitmap(&image_data);
		DestroyBitmap(&border);

//		char buf[1024];
		printf("Made %d images and %d borders that were missing.\n", image_ctr, border_ctr);
//		DoUserAlert(buf);
	}
	return 0;
}

void	CreatePackageForDSF(const char * inPackage, int lon, int lat, char * outDSFDestination)
{
	sprintf(outDSFDestination, "%sEarth nav data" DIR_STR "%+03d%+04d" DIR_STR,
					inPackage, latlon_bucket(lat), latlon_bucket(lon));

	FILE_make_dir_exist(outDSFDestination);
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
