#include "GUI_Resources.h"

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

