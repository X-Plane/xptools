/*
 * Copyright (c) 2007, Laminar Research.
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

#define NEW_TEX_LOAD_STRATEGY 1

#include "WED_TexMgr.h"
#if !NEW_TEX_LOAD_STRATEGY
	#include "BitmapUtils.h"
#endif
#include "MemFileUtils.h"
#include "TexUtils.h"
#include "WED_PackageMgr.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_TexMgr::WED_TexMgr(const string& package) : mPackage(package)
{
}

WED_TexMgr::~WED_TexMgr()
{
	for(map<string,TexInfo *>::iterator t = mTexes.begin(); t != mTexes.end(); ++t)
	{
		GLuint id = t->second->tex_id;
		glDeleteTextures(1, &id);
		delete t->second;
	}
}

TexRef		WED_TexMgr::LookupTexture(const char * path, bool is_absolute, int flags)
{
	TexMap::iterator i = mTexes.find(path);
	if (i == mTexes.end())
	{
		return LoadTexture(path, is_absolute,flags);
	}
	return i->second;
}

int			WED_TexMgr::GetTexID(TexRef ref)
{
	return ((TexInfo *) ref)->tex_id;
}

void		WED_TexMgr::GetTexInfo(
						TexRef	ref,
						int *	vis_x,
						int *	vis_y,
						int *	act_x,
						int *	act_y,
						int *	org_x,
						int *	org_y)
{
	TexInfo * i = (TexInfo *) ref;
	if (vis_x) *vis_x = i->vis_x;
	if (vis_y) *vis_y = i->vis_y;
	if (act_x) *act_x = i->act_x;
	if (act_y) *act_y = i->act_y;
	if (org_x) *org_x = i->org_x;
	if (org_y) *org_y = i->org_y;
}

WED_TexMgr::TexInfo *	WED_TexMgr::LoadTexture(const char * path, bool is_absolute, int flags)
{
	string fpath(is_absolute ? path : gPackageMgr->ComputePath(mPackage, path));
	TexInfo * inf = NULL;

	GLuint tn;
	glGenTextures(1,&tn);
#if LOAD_DDS_DIRECT
	FILE * file = fopen(fpath.c_str(), "rb");
	if (file)
	{
		char c[4];
		if (fread(c, 1, 4, file) == 4 && strncmp(c, "DDS ",4) == 0) // cut it short, if no joy
		{
			fseek(file, 0, SEEK_END);
			int fileLength = ftell(file);
			fseek(file, 0, SEEK_SET);
			char * buffer = new char[fileLength];
			if (buffer)
			{
				if (fread(buffer, 1, fileLength, file) == fileLength)
				{
					int siz_x, siz_y;
					if (LoadTextureFromDDS(buffer, buffer + fileLength, tn, 0, &siz_x, &siz_y))
					{
//						printf("Direct loading DDS %s\n", fpath.c_str());
						inf = new TexInfo;
						inf->tex_id = tn;
						inf->org_x = inf->vis_x = inf->act_x = siz_x;
						inf->org_y = inf->vis_y = inf->act_y = siz_y;
						mTexes[path] = inf;
					}
				}
				delete [] buffer;
			}
			fclose(file);
		}
	}
	if(inf) return inf;

//	printf("Normal load %s\n%s\n", path, fpath.c_str());
#endif

#if NEW_TEX_LOAD_STRATEGY
	// auto-detection of file type, basic on file content, only
	{
		int siz_x, siz_y;
		float s,t;
		if (LoadTextureFromFile(fpath.c_str(), tn, flags, &siz_x, &siz_y, &s,&t))
#else
	// loading based on file name suffix. With this method we preserve awareness of original image size.
	// But with openGL 3.0 as new minimum requirement - all GPU's have to support non-power-2 textures,
	// and only Orthophoto export would be affected. So thats unlikely to be a loss, ever.
	ImageInfo	im;
	if(MakeSupportedType(fpath.c_str(), &im) == 0)
	{
		int siz_x, siz_y;
		float s,t;
		if (LoadTextureFromImage(im, tn, flags, &siz_x, &siz_y, &s,&t))
#endif
		{
			inf = new TexInfo;
			inf->tex_id = tn;
#if NEW_TEX_LOAD_STRATEGY
			inf->org_x = siz_x;
			inf->org_y = siz_y;
#else
			inf->org_x = im.width;
			inf->org_y = im.height;
#endif
			inf->act_x = siz_x;
			inf->act_y = siz_y;
			inf->vis_x = (float) siz_x * s;
			inf->vis_y = (float) siz_y * t;
			mTexes[path] = inf;
		}
#if !NEW_TEX_LOAD_STRATEGY
		DestroyBitmap(&im);
#endif
	}
	return inf;
}
