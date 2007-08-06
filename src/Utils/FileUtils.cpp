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

#include "FileUtils.h"
#include "PlatformUtils.h"

#include <errno.h>
#include <sys/stat.h>

int FILE_exists(const char * path)
{
	struct stat ss;
	if (stat(path,&ss) < 0) return 0;
	return (S_ISDIR(ss.st_mode))? 1 : 0;
}

int FILE_delete_file(const char * nuke_path, int is_dir)
{
	// NOTE: if the path is to a dir, it will end in a dir-char.
	// We must clip off this char and also call the right routine on Windows.
#if IBM
	if (is_dir)	{
		if (!RemoveDirectory(nuke_path))	return GetLastError();
	} else {
		if (!DeleteFile(nuke_path))			return GetLastError();
	}
#endif

#if LIN || APL
	if (is_dir)	{
		if (rmdir(nuke_path) != 0)	return errno;
	}	else 	{
		if (unlink(nuke_path) != 0)	return errno;
	}
#endif
	return 0;
}

int FILE_rename_file(const char * old_name, const char * new_name)
{
#if IBM
	if(!MoveFile(old_name,new_name)) return GetLastError();
#endif
#if LIN || APL
	if(rename(old_name,new_name)<0)	return errno;
#endif
	return 0;
}

int FILE_make_dir(const char * in_dir)
{
	#if IBM
		if (!CreateDirectory(in_dir,NULL))	return GetLastError();
	#endif
	#if LIN || APL
		if (mkdir(in_dir,0755) != 0)		return errno;
	#endif
	return 0;
}


int FILE_make_dir_exist(const char * in_dir)
{
	int result = 0;
	if (!FILE_exists(in_dir))
	{
		const char * dc = in_dir + strlen(in_dir) - 1;
		while(dc > in_dir && *dc != DIR_CHAR) --dc;
		
		string parent(in_dir, dc);
							result = FILE_make_dir_exist(parent.c_str());
		if (result == 0)	result = FILE_make_dir(in_dir);
	}
	return result;
}

