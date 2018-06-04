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

#include "GUI_Resources.h"
#if IBM
#include "GUI_Unicode.h"
#endif
#include "AssertUtils.h"
#include "TexUtils.h"
#include "BitmapUtils.h"
#include "MemFileUtils.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#if APL
	#include <CoreFoundation/CoreFoundation.h>
#endif

#if LIN
#include <dlfcn.h>
#include <ctype.h>

static void* gModuleHandle = 0;

/*
** slightly modified version from binutils (binary.c)
** to ensure the same name mangling as objcopy does
*/
static char* mangle_name (const char* filename, const char* suffix)
{
	size_t size;
	char *buf;
	char *p;

	size = (strlen(filename) + strlen(suffix) + sizeof("_binary__"));

	buf = new char[size];
	if (buf == 0) return 0;
	snprintf(buf, size, "_binary_%s_%s", filename, suffix);

	for (p = buf; *p; p++)
	if (!isalnum(*p)) *p = '_';
	return buf;
}

static bool resource_get_memory(const string& res_name, char** begin, char** end)
{
	const char* sym_start = 0;
	const char* sym_end = 0;

	if (!gModuleHandle) return false;
	if (!begin) return false;
	if (!end) return false;

	*begin = 0;
	*end = 0;

	sym_start = mangle_name(res_name.c_str(), "start");
	if (!sym_start) goto cleanup;
	sym_end = mangle_name(res_name.c_str(), "end");
	if (!sym_end) goto cleanup;

	*begin = (char*)dlsym(gModuleHandle, sym_start);
	*end = (char*)dlsym(gModuleHandle, sym_end);
cleanup:
	if (sym_start) delete[] sym_start;
	if (sym_end) delete[] sym_end;
	if (*begin && *end)	return true;
	return false;
}

#endif

#if APL
static int 	GUI_GetResourcePath(const char * in_resource, string& out_path)
{
	#if APL
		int found = 0;

		CFStringRef res_str = CFStringCreateWithCString(kCFAllocatorDefault,in_resource,kCFStringEncodingMacRoman);
		if (res_str)
		{
			CFURLRef		res_url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), res_str, NULL, NULL);
			if(res_url)
			{
				CFStringRef	path_str = CFURLCopyFileSystemPath(res_url, kCFURLPOSIXPathStyle);
				if(path_str)
				{
					CFRange range = CFRangeMake(0, CFStringGetLength(path_str));
					vector<UInt8> buf(CFStringGetBytes(path_str,range,kCFStringEncodingMacRoman,0,false,NULL,0, NULL));
					CFStringGetBytes(path_str,range,kCFStringEncodingMacRoman,0,false,&*buf.begin(),buf.size(), NULL);
					out_path = string(buf.begin(), buf.end());
					found = 1;
					CFRelease(path_str);
				}
				CFRelease(res_url);
			}
			CFRelease(res_str);
		}
		return found;

	#else
		#error not impl
	#endif
}

GUI_Resource	GUI_LoadResource(const char * in_resource)
{
	string path;
	if (!GUI_GetResourcePath(in_resource, path)) return NULL;
	return MemFile_Open(path.c_str());
}

void			GUI_UnloadResource(GUI_Resource res)
{
	MemFile_Close((MFMemFile *) res);
}

const char *	GUI_GetResourceBegin(GUI_Resource res)
{
	return 	MemFile_GetBegin((MFMemFile *) res);
}

const char *	GUI_GetResourceEnd(GUI_Resource res)
{
	return 	MemFile_GetEnd((MFMemFile *) res);
}

bool			GUI_GetTempResourcePath(const char * in_resource, string& out_path)
{
	return GUI_GetResourcePath(in_resource, out_path);
}


#elif IBM

struct res_struct {
	const char * start_p;
	const char * end_p;
};
typedef map<string, res_struct>	res_map;
static res_map sResMap;

GUI_Resource	GUI_LoadResource(const char * in_resource)
{
	string path(in_resource);
	res_map::iterator i = sResMap.find(path);
	if (i != sResMap.end()) return &(i->second);

	HRSRC	res_info = FindResourceW(NULL, convert_str_to_utf16(in_resource).c_str(),L"GUI_RES");
	if (res_info==NULL) return NULL;
	HGLOBAL res = LoadResource(NULL, res_info);
	if (res == NULL) return NULL;
	DWORD sz = SizeofResource(NULL,res_info);
	if (sz == 0) return NULL;
	LPVOID ptr = LockResource(res);
	if (ptr == NULL) return NULL;

	res_struct info = { (const char *) ptr, (const char *) ptr + sz };
	pair<res_map::iterator,bool> ins = sResMap.insert(res_map::value_type(path,info));
	return &ins.first->second;
}

void			GUI_UnloadResource(GUI_Resource res)
{
}

const char *	GUI_GetResourceBegin(GUI_Resource res)
{
	return ((res_struct*)res)->start_p;
}

const char *	GUI_GetResourceEnd(GUI_Resource res)
{
		return ((res_struct*)res)->end_p;
}

bool			GUI_GetTempResourcePath(const char* in_resource, string& out_path)
{
	GUI_Resource res = GUI_LoadResource(in_resource);
	if (res == NULL) return false;
	const char * sp = GUI_GetResourceBegin(res);
	const char * ep = GUI_GetResourceEnd(res);

	WCHAR	temp_path[MAX_PATH] = { 0 };
	WCHAR	temp_file[MAX_PATH] = { 0 };
     // Get the temp path.
    int result = GetTempPathW(MAX_PATH, temp_path);
	if (result > sizeof(temp_path) || result == 0)
	{
		GUI_UnloadResource(res); return false;
	}

	result =  GetTempFileNameW(temp_path, convert_str_to_utf16(in_resource).c_str(), 0, temp_file);
	if (result == 0) { GUI_UnloadResource(res); return false; }

	wcscat(temp_file, convert_str_to_utf16(in_resource).c_str());

	FILE * fi = fopen(convert_utf16_to_str(temp_file).c_str(), "wb");
	if (fi == NULL) { GUI_UnloadResource(res); return false; }
	fwrite(sp, ep - sp, 1, fi);

	fclose(fi);
	GUI_UnloadResource(res);
	out_path = convert_utf16_to_str(temp_file);
	return true;
}


#elif LIN
struct res_struct {
	const char * start_p;
	const char * end_p;
};
typedef map<string, res_struct>	res_map;
static res_map sResMap;


GUI_Resource	GUI_LoadResource(const char* in_resource)
{
	if (sResMap.empty()) gModuleHandle = dlopen(0, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
	if (!gModuleHandle)
	{
		printf("error opening module\n");
		return 0;
	}
	string path(in_resource);
	res_map::iterator i = sResMap.find(path);
	if (i != sResMap.end()) return &(i->second);

	char* b = 0;
	char* e = 0;

	if (!resource_get_memory(path, &b, &e)) return 0;

	res_struct info = { (const char *) b, (const char *) e };

	pair<res_map::iterator,bool> ins = sResMap.insert(res_map::value_type(path,info));
	return &ins.first->second;
}

void			GUI_UnloadResource(GUI_Resource res)
{
	// TODO: remove specific resource from sResMap here
	if (sResMap.empty())
	{
		dlclose(gModuleHandle);
		gModuleHandle = 0;
	}
}

const char *	GUI_GetResourceBegin(GUI_Resource res)
{
	return ((res_struct*)res)->start_p;
}

const char *	GUI_GetResourceEnd(GUI_Resource res)
{
	return ((res_struct*)res)->end_p;
}

typedef map<string, string>	tmp_res_paths;
static tmp_res_paths sTmpResPaths;

bool			GUI_GetTempResourcePath(const char * in_resource, string& out_path)
{
	tmp_res_paths::iterator it = sTmpResPaths.find(in_resource);
	if (it != sTmpResPaths.end())
	{
		out_path = it->second;
		return true;
	}

	GUI_Resource res = GUI_LoadResource(in_resource);
	if (!res) return false;
	const char * sp = GUI_GetResourceBegin(res);
	const char * ep = GUI_GetResourceEnd(res);
	char tmpfilename[255] = "/tmp/.wed_XXXXXX";
	int fd = mkstemp(tmpfilename);
	if (fd == -1)
	{
		printf("error opening temporary resource file %s\n", tmpfilename);
		GUI_UnloadResource(res); return false;
	}

	int r = write(fd, sp, ep - sp);
	close(fd);

	GUI_UnloadResource(res);
	sTmpResPaths[in_resource]=tmpfilename;
	out_path = tmpfilename;
    return true;
}

#endif

//---------------------------------------------------------------------------------------------------------------------------------------------------
// TEXTURES
//---------------------------------------------------------------------------------------------------------------------------------------------------

int GUI_GetImageResource(
			const char *		in_resource,
			ImageInfo *			io_image)
{
	GUI_Resource res = GUI_LoadResource(in_resource);
	if (res == NULL) return -1;
	int ret;
	if (strstr(in_resource,".jpg")) {
    #if USE_JPEG
		ret = CreateBitmapFromJPEGData((void *) GUI_GetResourceBegin(res), GUI_GetResourceEnd(res) - GUI_GetResourceBegin(res), io_image);
    #else
        ret = 1;
    #endif
	} else
		ret = CreateBitmapFromPNGData(GUI_GetResourceBegin(res), GUI_GetResourceEnd(res) - GUI_GetResourceBegin(res), io_image, 1, GAMMA_SRGB);
	GUI_UnloadResource(res);
	//Because createbitmapfromX now return the channels instead of -1,0,or 1 this is so we do not need to update everything else that calls this.
	//The GUI does not care about the channels, only if it is real or not
	if(ret >= 0) ret = 0;
	return ret;
}


struct	TexInfo {
	GLuint				tex_id;
	GUI_TexPosition_t	metrics;
};

typedef hash_map<string, TexInfo>	TexResourceTable;
static TexResourceTable		sTexes;

int	GUI_GetTextureResource(
			const char *		in_resource,
			int					flags,
			GUI_TexPosition_t *	out_metrics)
{
	string r(in_resource);

	TexResourceTable::iterator i = sTexes.find(r);
	if (i != sTexes.end())
	{
		if (out_metrics)	memcpy(out_metrics, &i->second.metrics,sizeof(GUI_TexPosition_t));
		return i->second.tex_id;
	}

	TexInfo	info;
	glGenTextures(1,&info.tex_id);
	string full_path;

	ImageInfo	image;

	if (GUI_GetImageResource(in_resource,&image) != 0)
		AssertPrintf("Error: could not find internal bitmap %s\n", in_resource);

	if (!LoadTextureFromImage(image, info.tex_id, flags,
			&info.metrics.tex_width, &info.metrics.tex_height,
			&info.metrics.s_rescale, &info.metrics.t_rescale))
		AssertPrintf("Error: could not load internal bitmap %s\n", full_path.c_str());

	info.metrics.real_width  = info.metrics.tex_width  * info.metrics.s_rescale;
	info.metrics.real_height = info.metrics.tex_height * info.metrics.t_rescale;

	sTexes[r] = info;

	if (out_metrics)	memcpy(out_metrics, &info.metrics,sizeof(GUI_TexPosition_t));

	DestroyBitmap(&image);

	return info.tex_id;

}

int	GUI_GetImageResourceWidth(const char * in_resource)
{
	string r(in_resource);
	TexResourceTable::iterator i = sTexes.find(r);
	if (i != sTexes.end())
		return i->second.metrics.real_width;

	ImageInfo	im;
	int			ret;

	if (GUI_GetImageResource(in_resource, &im) == 0)
	{
		ret = im.width;
		DestroyBitmap(&im);
		return ret;
	}
	return 0;
}

int	GUI_GetImageResourceHeight(const char * in_resource)
{
	string r(in_resource);
	TexResourceTable::iterator i = sTexes.find(r);
	if (i != sTexes.end())
		return i->second.metrics.real_height;

	ImageInfo	im;
	int			ret;

	if (GUI_GetImageResource(in_resource, &im) == 0)
	{
		ret = im.height;
		DestroyBitmap(&im);
		return ret;
	}
	return 0;

}

int	GUI_GetImageResourceSize(const char * in_resource, int bounds[2])
{
	bounds[0] = bounds[1] = 0;
	string r(in_resource);
	TexResourceTable::iterator i = sTexes.find(r);
	if (i != sTexes.end())
	{
		bounds[0] = i->second.metrics.real_width;
		bounds[1] = i->second.metrics.real_height;
		return 1;
	}

	ImageInfo	im;

	if (GUI_GetImageResource(in_resource, &im) == 0)
	{
		bounds[0] = im.width;
		bounds[1] = im.height;
		DestroyBitmap(&im);
		return 1;
	}
	return 0;

}


float	GUI_Rescale_S(float s, GUI_TexPosition_t * metrics);
float	GUI_Rescale_T(float t, GUI_TexPosition_t * metrics);

// WordWrap a string by replacing spaces in it for \n.
// If the string already has some \n in it, be clever as to not insert more than needed.
// Also be clever about words longer than the allowed width, i.e. don't truncate those.

std::string WordWrap( std::string str, size_t width )
{
	size_t maxPos = width;                         // latest position we can afford to have a CR in
	std::string::size_type lastCR, spacePos = 0;   // position of last detected CR and space

	while( maxPos < str.length() )
    {
    #if 1
		lastCR = str.rfind('\n', maxPos);       // look if suitable CR already there
		if (lastCR != std::string::npos && lastCR != spacePos)
		{
			maxPos = lastCR + width + 1;
			spacePos = lastCR;
		}
		else
	#endif
		{
			spacePos = str.rfind(' ', maxPos);     // look for a space just before the line gets too long
			if( spacePos == std::string::npos ||
			   (lastCR != string::npos && spacePos <= lastCR))   // no joy, i.e. a very long word
			{
				spacePos = str.find(' ', maxPos);  // so we look for a good place after that word,
			}                                      // as we have no better choice here.
			if( spacePos != std::string::npos )
			{
				str[spacePos] = '\n';
				maxPos = spacePos + width + 1;
			}
		}
	}
	return str;
}
