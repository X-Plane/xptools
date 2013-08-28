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
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#if IBM
#include "GUI_Unicode.h"
#endif

#include <errno.h>
#if LIN || APL
#include <dirent.h>
#include <sys/stat.h>
#endif

#define LOG_CASE_DESENS 0

#if LOG_CASE_DESENS	
	#define	LOG_MSG(fmt,...) printf(fmt, __VA_ARGS__)
#else
	#define	LOG_MSG(fmt,...)
#endif

#if LIN
static int desens_partial(DIR * dir, char * io_file)
{
 struct dirent* de;
 while (de = readdir(dir))
 {
  if (!strcasecmp(io_file, de->d_name))
  {
   strcpy(io_file, de->d_name);
   return 1;
  }
 }
 return 0;
}
#endif


int FILE_case_correct(char * buf)
{
	#if LIN
	LOG_MSG("Case desens for: '%s'\n", buf);

	// Fast match?  Try that first - MOST content in x-plane is case-correct, and any file path derived from dir scanning will be.

	struct stat sta;
	if (stat(buf, &sta) == 0) 
	{
		LOG_MSG("  Fast match.  Done.\n",0);
		return 1;
	}
	
	char * p = buf;
	
	while (*p != 0)
	{
		DIR * dir;
		if (*p == '/')
		{
			dir = opendir("/");
			LOG_MSG("  Open-dir '%s': 0x%08x\n","/",dir);
			++p;
		}
		else if (p == buf)
		{
			dir = opendir(".");
			LOG_MSG("  Open-dir '%s': 0x%08x\n",".",dir);
		}
		else
		{
			*(p-1) = 0;
			dir = opendir(buf);
			LOG_MSG("  Open-dir '%s': 0x%08x\n",buf,dir);
			*(p-1) = '/';
		}
		if (dir == NULL)
		{
			return 0;
		}	
		char * q = p;// Ptr past EOF
		while (*q != 0 && *q != '/') ++q;
		
		int last_time = *q == 0;
		*q = 0;		// Fake null term
		
		int worked = desens_partial(dir, p);
		closedir(dir);
		
		if (!last_time)
		{
			*q = '/';
			p = q+1;
			if (!worked)
			{
				LOG_MSG("  Partial-desens failed.  Done at '%s'\n",buf);
				return 0;
			}
		} else {
			LOG_MSG("  Finished all parts.  Done at '%s'\n",buf);
			return 1;
		}		
	}
	return 0;	// we hit here if our file name was empty.
#else 
	return 1;
#endif
}

FILE_case_correct_path::FILE_case_correct_path(const char * in_path) : path(strdup(in_path)) { FILE_case_correct(path); }
FILE_case_correct_path::~FILE_case_correct_path() { free(path); }
	
FILE_case_correct_path::operator const char * (void) const { return path; }
	




bool FILE_exists(const char * path)
{
#if IBM
	struct _stat ss;
	string input(path);
	string_utf16 output;
	string_utf_8_to_16(input, output);
	if (_wstat((const wchar_t*)output.c_str(),&ss) < 0) return false;
#else
	struct stat ss;
	if (stat(path,&ss) < 0) return 0;
#endif
	return true;
//	return (S_ISDIR(ss.st_mode))? 1 : 0;
}

int FILE_delete_file(const char * nuke_path, int is_dir)
{
	// NOTE: if the path is to a dir, it will end in a dir-char.
	// We must clip off this char and also call the right routine on Windows.
#if IBM
	string input(nuke_path);
	string_utf16 output;
	string_utf_8_to_16(input, output);
	if (is_dir)	{
		if (!RemoveDirectoryW((const wchar_t*)output.c_str()))	return GetLastError();
	} else {
		if (!DeleteFileW((const wchar_t*)output.c_str()))			return GetLastError();
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
	string oldn(old_name);
	string newn(new_name);
	string_utf16 old16, new16;
	string_utf_8_to_16(oldn, old16);
	string_utf_8_to_16(newn, new16);
	if(!MoveFileW((const wchar_t*)old16.c_str(), (const wchar_t*)new16.c_str())) return GetLastError();
#endif
#if LIN || APL
	if(rename(old_name,new_name)<0)	return errno;
#endif
	return 0;
}

int FILE_make_dir(const char * in_dir)
{
	#if IBM
		string input(in_dir);
		string_utf16 output;
		string_utf_8_to_16(input, output);	
		if (!CreateDirectoryW((const wchar_t*)output.c_str() ,NULL))	return GetLastError();
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
		if(dc > in_dir && *dc == DIR_CHAR) --dc;			// Dir ends in trailing /?  Better pop it off.
		while(dc > in_dir && *dc != DIR_CHAR) --dc;
		if(dc > in_dir){
		string parent(in_dir, dc);
							result = FILE_make_dir_exist(parent.c_str());}
		if (result == 0)	result = FILE_make_dir(in_dir);
	}
	return result;
}

date_cmpr_result_t FILE_date_cmpr(const char * first, const char * second)
{
//Inspired by http://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
#if IBM 
	struct _stat firstFile;
	struct _stat secondFile;
	int error1;
	int error2;

	error1 = _stat(first,&firstFile);
	error2 = _stat(second,&secondFile);

	if(error1 != 0)
	{
		return dcr_error;
	}
	else
	{
		//If first is newer
		if(firstFile.st_mtime > secondFile.st_mtime || error2 !=0)
		{
			return dcr_firstIsNew;
		}
		if(firstFile.st_mtime < secondFile.st_mtime)
		{
			return dcr_secondIsNew;
		}
		if(firstFile.st_mtime == secondFile.st_mtime)
		{
			return dcr_same;
		}
		return dcr_error;
	}
#else
	struct stat firstFile;
	struct stat secondFile;
	int error1;
	int error2;

	error1 = stat(first,&firstFile);
	error2 = stat(second,&secondFile);

	if(error1 != 0)
	{
		return dcr_error;
	}
	else
	{
		//If first is newer
		if(firstFile.st_mtime > secondFile.st_mtime || error2 !=0)
		{
			return dcr_firstIsNew;
		}
		if(firstFile.st_mtime < secondFile.st_mtime)
		{
			return dcr_secondIsNew;
		}
		if(firstFile.st_mtime == secondFile.st_mtime)
		{
			return dcr_same;
		}
		return dcr_error;
	}

#endif
}
