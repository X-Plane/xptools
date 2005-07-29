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
this is bens OS-specific stuff
*/
#include <CFBundle.h>
#include <Folders.h>
#include "hl_types.h"
extern "C" void __start(void);
#define	DISABLE_UNIX_ALLOCATORS 0
#if APL
#define NEWMODE	5
#define MALLOCFUNC xmalloc
#define MFREEFUNC xfree
#define PTRTYPE void *
#include "New.cp"
#endif
#if APL
int (* bsd_open)(const char *path, int flags, mode_t mode) = NULL;
int (* bsd_close)(int d) = NULL;
off_t (* bsd_lseek)(int fildes, off_t offset, int whence) = NULL;
void * (* bsd_mmap)(
				void  *start,  size_t length, int prot , int
				  	flags, int fd, off_t offset) = NULL;
int (* bsd_munmap)(void * start, size_t length) = NULL;
int (* bsd_fstat)(int, struct stat *) = NULL;
void * (* xmalloc)(size_t);
void (* xfree)(void *);
#endif
#if IBM
void * (* xmalloc)(size_t) = malloc;
void (* xfree)(void *) = free;
#endif
#if LIN
void * (* xmalloc)(size_t) = malloc;
void (* xfree)(void *) = free;
#endif
extern "C" void xstart(void)
{
#if APL
	xmalloc = malloc;		// GOTTA DO THIS FOR OS 9!!
	xfree = free;
	CFBundleRef	sysBundle = ::CFBundleGetBundleWithIdentifier(CFSTR("com.apple.System"));
	if (!sysBundle)
	{
		FSRef	frameworksFolder;
		if (FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, false, &frameworksFolder) == noErr)
		{
			CFURLRef frameworksFolderURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolder);
			if (frameworksFolderURL != NULL)
			{
				CFURLRef sysBundleURL = CFURLCreateCopyAppendingPathComponent(
					kCFAllocatorSystemDefault, frameworksFolderURL, CFSTR("System.framework"), false);
				if (sysBundleURL)
				{
					sysBundle = CFBundleCreate(kCFAllocatorSystemDefault, sysBundleURL);
					CFRelease(sysBundleURL);
				}
				CFRelease(frameworksFolderURL);
			}
		}
	}
	
	if (sysBundle)
	{
		xmalloc = (void * (*)(unsigned long)) ::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("malloc"));
		xfree = (void (*)(void *)) ::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("free"));		
		
		// If we fail to get either allocator we can't use either because we can't
		// use malloc from one lib and free from another.
		if (!xmalloc || !xfree || DISABLE_UNIX_ALLOCATORS)
		{
			xmalloc = malloc;
			xfree = free;
			// Can't debug this message, the static infrastructure for the deverr
			// stream does NOT EXIST YET!
		}
		
		bsd_open = (int (*)(const char *path, int flags, mode_t mode))	::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("open"));
		bsd_close = (int (*)(int d))									::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("close"));
		bsd_lseek = (off_t (*)(int fildes, off_t offset, int whence))	::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("lseek"));
		bsd_mmap = (void * (*)(
				void  *start,  size_t length, int prot , int
				  	flags, int fd, off_t offset))							::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("mmap"));
		bsd_munmap = (int (*)(void * start, size_t length))	::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("munmap"));
		bsd_fstat = (int (*)(int, struct stat *)) ::CFBundleGetFunctionPointerForName(sysBundle, CFSTR("fstat"));
		
//		CFRelease(sysBundle);
	}
	__start();	
#endif
}	
