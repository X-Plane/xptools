#include "GUI_Resources.h"
#include "AssertUtils.h"
#include "TexUtils.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#if APL
	#include <CoreFoundation/CoreFoundation.h>
#endif

int 	GUI_GetResourcePath(const char * in_resource, string& out_path)
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

//---------------------------------------------------------------------------------------------------------------------------------------------------
// TEXTURES
//---------------------------------------------------------------------------------------------------------------------------------------------------

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
	
	if (!GUI_GetResourcePath(in_resource,full_path))
		AssertPrintf("Error: could not find internal bitmap %s\n", in_resource);
	
	if (!LoadTextureFromFile(full_path.c_str(), info.tex_id, flags,
			&info.metrics.tex_width, &info.metrics.tex_height,
			&info.metrics.s_rescale, &info.metrics.t_rescale))
		AssertPrintf("Error: could not load internal bitmap %s\n", full_path.c_str());
	
	info.metrics.real_width  = info.metrics.tex_width  * info.metrics.s_rescale;
	info.metrics.real_height = info.metrics.tex_height * info.metrics.t_rescale;

	sTexes[r] = info;

	if (out_metrics)	memcpy(out_metrics, &info.metrics,sizeof(GUI_TexPosition_t));
	
	return info.tex_id;

}	


float	GUI_Rescale_S(float s, GUI_TexPosition_t * metrics);
float	GUI_Rescale_T(float t, GUI_TexPosition_t * metrics);


