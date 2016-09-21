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
#include <time.h>
#if IBM
#include "GUI_Unicode.h"
#endif

#include <errno.h>
#if LIN || APL
#include <dirent.h>
#include <sys/stat.h>
#endif
#include "zip.h"

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

string FILE_get_file_extension(const string& path)
{
	string name;

	//If the path contains no path seperators at all, we have just the filename, extension, or an empty string
	if(path.find('\\') == string::npos && path.find('/') == string::npos)
	{
		name = path;
	}
	else
	{
		name = FILE_get_file_name(path);
	}
	
	size_t dot_start = name.find_last_of('.');
	if(dot_start == string::npos)
	{
		return "";
	}
	else
	{
		return name.substr(dot_start);
	}
}

int FILE_get_file_meta_data(const string& path, struct stat& meta_data)
{
	return stat(path.c_str(), &meta_data);
}

string FILE_get_file_name(const string& path)
{
	//If we can find a / we're using UNIX
	size_t pos = path.find_first_of('/');

	char separator = '\0';

	if(pos != string::npos)
	{
		separator = '/';
	}
	else
	{
		separator = '\\';
	}
	size_t last_sep = path.find_last_of(separator);

	if(last_sep == string::npos)
	{
		return "";
	}
	else
	{
		return path.substr(last_sep + 1);
	}
}

string FILE_get_file_name_wo_extensions(const string& path)
{
	string name = FILE_get_file_name(path);
	
	size_t dot_pos = name.find_first_of('.');
	if(dot_pos == path.npos || dot_pos == 0)
	{
		return name;
	}
	
	return name.substr(0, dot_pos);
}

int FILE_delete_file(const char * nuke_path, bool is_dir)
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

int FILE_read_file_to_string(FILE* file, string& content)
{
	content = "";
	if(file != NULL)
	{
#if LIN || APL
		errno = 0;
#else
		SetLastError(0);
#endif
		fseek(file, 0, SEEK_END);
		content.resize(ftell(file));
		rewind(file);
		fread(&content[0], sizeof(char), content.size(), file);
	}
#if LIN ||APL
	return errno;
#else
	return GetLastError();
#endif
}

int FILE_read_file_to_string(const string& path, string& content)
{
	int res = -1;

	FILE* file = fopen(path.c_str(),"r");
	
	if(file != NULL)
	{
		res = FILE_read_file_to_string(file, content);
	}

	fclose(file);
	return res;
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

int FILE_get_directory(const string& path, vector<string> * out_files, vector<string> * out_dirs)
{
#if IBM

	string				searchPath(path);
	WIN32_FIND_DATA		findData;
	HANDLE				hFind;
	int					total = 0;

	searchPath += string("\\*.*");

	hFind = FindFirstFile(searchPath.c_str(),&findData);
	if (hFind == INVALID_HANDLE_VALUE) return -1;

	do {

		if(strcmp(findData.cFileName,".") == 0 ||
			strcmp(findData.cFileName,"..") == 0)
		{
			continue;
		}

		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(out_dirs)
				out_dirs->push_back(findData.cFileName);
		}
		else
		{
			if(out_files)
				out_files->push_back(findData.cFileName);
		}
		
		++total;

	} while(FindNextFile(hFind,&findData) != 0);


	FindClose(hFind);
	return total;

#elif LIN || APL

	int total=0;
	DIR* dir = opendir ( path.c_str() );
	if ( !dir ) return -1;
	struct dirent* ent;

	while ( ( ent = readdir ( dir ) ) )
	{
		struct stat ss;
		if ( ( strcmp ( ent->d_name, "." ) ==0 ) ||
		        ( strcmp ( ent->d_name, ".." ) ==0 ) )
			continue;

		string	fullPath ( path );
		fullPath += DIR_CHAR;
		fullPath += ent->d_name;

		if ( stat ( fullPath.c_str(), &ss ) < 0 )
			continue;
		total++;
		
		if(S_ISDIR ( ss.st_mode ))
		{
			if(out_dirs)
				out_dirs->push_back(ent->d_name);
		}
		else
		{
			if(out_files)
				out_files->push_back(ent->d_name);
		}
	}
	closedir ( dir );

	return total;

#else
#error not implemented
#endif
}

int FILE_get_directory_recursive(const string& path, vector<string>& out_files, vector<string>& out_dirs)
{
	//Save the previous last positions before we potentially add more to files and dirs
	int files_start_index = out_files.size();
	int start_index = out_dirs.size();
	
	//Gets this level of the directory's information
	int num_files = FILE_get_directory(path, &out_files, &out_dirs);
	
	//If we have accumulated new files, prepend the path onto them
	if (out_files.size() > files_start_index)
	{
		for (int i = files_start_index; i < out_files.size(); i++)
		{
			out_files.at(i) = path + DIR_STR + out_files.at(i);
		}
	}

	//For all the directories on this level, recurse into them
	for (int i = start_index; i < out_dirs.size(); ++i)
	{
		num_files += FILE_get_directory_recursive(path + DIR_STR + out_dirs.at(i), out_files, out_dirs);
	}
	
	//For all the directories on this level prepend the path onto them
	for (int i = out_dirs.size() - 1; i >= start_index ; i--)
	{
		out_dirs.at(i) = path + DIR_STR + out_dirs.at(i);
	}
	
	return num_files;
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

int FILE_delete_dir_recursive(const string& path)
{
	vector<string>	files, dirs;

	int r = FILE_get_directory(path, &files, &dirs);
	
	if(r < 0)
		return r;
		
	for(vector<string>::iterator f = files.begin(); f != files.end(); ++f)
	{
		string fp(path);
		fp += *f;
		r = FILE_delete_file(fp.c_str(), false);
		if(r != 0)
			return r;
	}
	
	for(vector<string>::iterator d = dirs.begin(); d != dirs.end(); ++d)
	{
		string dp(path);
		dp += *d;
		dp += DIR_STR;
		int r = FILE_delete_dir_recursive(dp);
		if(r != 0)
			return r;
	}
	
	r = FILE_delete_file(path.c_str(),true);
	return r;
}
	
static int compress_one_file(zipFile archive, const string& src, const string& dst)
{
	FILE * srcf = fopen(src.c_str(),"rb");
	if(!srcf)
		return errno;

	zip_fileinfo	fi = { 0 };
	//http://unix.stackexchange.com/questions/14705/the-zip-formats-external-file-attribute
	fi.external_fa = 0100777 << 16;

	time_t		t;			
	time(&t);
	struct tm * our_time = localtime(&t);

	if(our_time)
	{
		fi.tmz_date.tm_sec  = our_time->tm_sec ;
		fi.tmz_date.tm_min  = our_time->tm_min ;
		fi.tmz_date.tm_hour = our_time->tm_hour;
		fi.tmz_date.tm_mday = our_time->tm_mday;
		fi.tmz_date.tm_mon  = our_time->tm_mon ;
		fi.tmz_date.tm_year = our_time->tm_year + 1900;
	}

	int r = zipOpenNewFileInZip (archive,dst.c_str(),
		&fi,		// mod dates, etc??
		NULL,0,
		NULL,0,
		NULL,		// comment
		Z_DEFLATED,
		Z_DEFAULT_COMPRESSION);
		
	if(r != 0) 
	{
		fclose(srcf);
		return r;
	}
	
	char buf[1024];
	while(!feof(srcf))
	{
		int rd = fread(buf,1,sizeof(buf),srcf);

		if(rd)
		{
			r = zipWriteInFileInZip(archive,buf,rd);
			if(r != 0)
			{
				fclose(srcf);
				return r;
			}
		}
		else
			break;
	}
	
	fclose(srcf);
	

	r = zipCloseFileInZip(archive);
	return r;
}
				
static int compress_recursive(zipFile archive, const string& dir, const string& prefix)
{	
	vector<string> files ,dirs;
	int r = FILE_get_directory(dir, &files,&dirs);
	if(r < 0) return r;
	
	for(vector<string>::iterator f = files.begin(); f != files.end(); ++f)
	{
		string sf = dir + *f;
		string df = prefix + *f;
		r = compress_one_file(archive, sf, df);
		if (r != 0)
			return r;
	}
	
	for(vector<string>::iterator d = dirs.begin(); d != dirs.end(); ++d)
	{
		string sd = dir + *d + DIR_STR;
		string dd = prefix + *d + "/";			// FORCE unix / or Mac loses its mind on decompress.
		r = compress_recursive(archive, sd, dd);
		if (r != 0)
			return r;
	}
	return 0;
}

int FILE_compress_dir(const string& src_path, const string& dst_path, const string& prefix)
{
	zipFile archive = zipOpen(dst_path.c_str(), 0);
	if(archive == NULL)
		return -1;

	int r = compress_recursive(archive, src_path,prefix);
	
	zipClose(archive, NULL);
	
	return r;
	
}
