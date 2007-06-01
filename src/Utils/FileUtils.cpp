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

