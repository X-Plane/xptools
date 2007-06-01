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
#include "hl_types.h"
#include "MemFileUtils.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#if APL
#if defined(__MWERKS__)
#include <CFURL.h>
#include <CFString.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#endif
#endif
/*
	TODO - level two analysis
	TODO - single file open handles zip, gz
	TODO - caching of open files in mem for all types of archives in case they are opened repeatedly	
*/

/*
	UNIT TEST - DIR ITERATION

	FILE * fi = fopen("junk", "a");
	
	fprintf(fi, "Dumping Macintosh HD:code:\n");
	if (!IterateDirectory("Macintosh HD:code:", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD:code:", 0));

	fprintf(fi, "Dumping Macintosh HD:code\n");
	if (!IterateDirectory("Macintosh HD:code", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD:code", 0));

	fprintf(fi, "Dumping Macintosh HD:code:voiceserv\n");
	if (!IterateDirectory("Macintosh HD:code:voiceserv", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD:code:voiceserv", 0));

	fprintf(fi, "Dumping Macintosh HD:\n");
	if (!IterateDirectory("Macintosh HD:", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD:", 0));

	fprintf(fi, "Dumping Macintosh HD\n");
	if (!IterateDirectory("Macintosh HD", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD", 0));

	fprintf(fi, "Dumping Macintosh HD:joy_error.txt\n");
	if (!IterateDirectory("Macintosh HD:joy_error.txt", hacktoFile, fi))
		fprintf(fi,"Failed dumping dir.\n");
	fprintf(fi," ft=%d\n", GetFileType("Macintosh HD:joy_error.txt", 0));

	fclose(fi);

	UNIT TEST - TARBALLS
	
	static	void * UTTest_Open(const char * fname, void * ref)
	{
		printf("Starting file %s\n", fname);
		char * fi = (char *) malloc(strlen(fname)+1);
		strcpy(fi, fname);
		return fi;
	}
	static	int		UTTest_Print(const char * data, int len, void * ref, void * fi)
	{
		printf("%d -'%s'\n", len, data);
		return len;
	}
	static void		UTTest_Close(void * ref, void * fi)
	{
		printf("Closing %s\n", fi);
		free(fi);
	}
				MF_Untar(argv[++n], true, UTTest_Open, UTTest_Print, UTTest_Close, NULL);

*/
	
#if LIN || (APL && __MACH__)
		#include <dirent.h>
		#include <sys/stat.h>
		#define DIR_CHAR '/'
#elif APL
		#include <Files.h>
		#define DIR_CHAR ':'
#elif IBM
	#define DIR_CHAR '\\'
	#include <sys/stat.h>
#else
	#error PLATFORM NOT DEFINED NEED DIR CHAR
#endif


#include <zlib.h>

#if LIN || (APL && __MACH__)
		#include <sys/types.h>
		#include <sys/mman.h>
		#include <unistd.h>
		#include <fcntl.h>
		#define bsd_open	 	open
		#define bsd_close 		close
		#define bsd_lseek 		lseek
		#define bsd_mmap	 	mmap
		#define bsd_munmap		munmap
		#define bsd_fstat 		fstat
#elif APL
		#include "hl_types.h"		
#elif IBM
	#include <windows.h>
#else
	#error PLATFORM NOT DEFINED
#endif	

#include <unzip.h>

#define REGTYPE	 '0'		/* regular file */
#define AREGTYPE '\0'		/* regular file */
#define LNKTYPE  '1'		/* link */
#define SYMTYPE  '2'		/* reserved */
#define CHRTYPE  '3'		/* character special */
#define BLKTYPE  '4'		/* block special */
#define DIRTYPE  '5'		/* directory */
#define FIFOTYPE '6'		/* FIFO special */
#define CONTTYPE '7'		/* reserved */

#define BLOCKSIZE 512

struct tar_header
{				/* byte offset */
  char name[100];		/*   0 */
  char mode[8];			/* 100 */
  char uid[8];			/* 108 */
  char gid[8];			/* 116 */
  char size[12];		/* 124 */
  char mtime[12];		/* 136 */
  char chksum[8];		/* 148 */
  char typeflag;		/* 156 */
  char linkname[100];		/* 157 */
  char magic[6];		/* 257 */
  char version[2];		/* 263 */
  char uname[32];		/* 265 */
  char gname[32];		/* 297 */
  char devmajor[8];		/* 329 */
  char devminor[8];		/* 337 */
  char prefix[155];		/* 345 */
				/* 500 */
};

union tar_buffer {
  char               buffer[BLOCKSIZE];
  struct tar_header  header;
};








struct	MFFileSet {

	MF_FileType			mType;			// Type of directory (directory, zip files, or gz tar ball.
	bool				mHasListing;	// Is the dir listing already loaded?
	string				mPath;			// Our path
	unzFile				mZipFile;		// The open zip archive for zip files
	vector<string>		mNames;			// Per file - filename
	vector<int>			mSizes;			// Per file - size of this file in bytes
	vector<char *>		mFileData;		// Per file - bytes of this file if pre-loaded (NULLs allowed to keep array ok.)
};	

struct	MFMemFile {
	bool			mFree;	// True if we need to free memory when done.
	bool			mClose;	// True if we need to close our file descriptor.
	bool			mUnmap;	// True if we need to unmap memory.

	int				mFile;	// File descriptor	
	const char *	mBegin;	// Span of memory the file is loaded into.
	const char *	mEnd;
#if IBM
	HANDLE 			mWinFile;
	HANDLE 			mWinFileMapping;
#endif	
};	

/*

	Three kinds of file sets:
	- File set made from a directory...lazily create directory.  Test files with stat or open.
	  Create a regular mem file for each entry using the regular file.
	- File set made from a zip archive.  Immediately decompress the headers.  Decompress a file as
	  need for an individual file, then nuke it when done.
	- File set made from a gz tar ball...immediately unzip, process whole tar ball keeping various
	  file chunks around.  Just return the buffer on open, don't nuke it later.
	  
	A few kinds of files:
	 - Regular file (fopen/fclose with its own malloc).
	 - Regular GZ'd file.  Same as above, but use GZ/IO
	 - Single zipped file, same as above, but use zip lib, affirm one file!
	 - File made from a mem buffer that can be nuked.  Made by opening a zip archive.
	 - File made from a buffer that's not ours, used to read tar balls.

*/

bool	StringVectorCB(const char * fname, bool dir, void * ref)
{
	vector<string> * v = (vector<string> *) ref;
	v->push_back(fname);
	return false;
}

static	void	FileSet_LoadDirectory(MFFileSet * fs)
{
	if (fs->mHasListing) return;
	if (fs->mType == mf_Directory)
	{
		fs->mNames.clear();
		MF_IterateDirectory(fs->mPath.c_str(), StringVectorCB, &fs->mNames);
	}
}

void * FileSet_TarballOpen(const char * fname, int size, void * ref)
{
	MFFileSet * ns = (MFFileSet *) ref;
	ns->mNames.push_back(fname);
	ns->mSizes.push_back(size);
	char * buf = (char *) malloc(size);
	ns->mFileData.push_back(buf);
	return buf;
}

int FileSet_TarballRead(const char * data, int len, void * ref, void * fi)
{
	memcpy(fi, data, len);
	return len;
}

void FileSet_TarballClose(void * ref, void * fi)
{
}

MFFileSet *		FileSet_Open(const char * inPath)
{
	MFFileSet * ns = NULL;
	MF_FileType	pathType = MF_GetFileType(inPath, 0);
	if (pathType == mf_BadFile) return NULL;
	
	if (pathType == mf_Directory)
	{
		ns = new MFFileSet;
		ns->mZipFile = NULL;
		ns->mType = mf_Directory;
		ns->mHasListing = false;
		ns->mPath = inPath;
		return ns;
	} else {
		FILE * fi = fopen(inPath, "rb");
		unsigned char	sig[2];
		if (fread(sig, 1, 2, fi) != 2)
		{
			fclose(fi);
			return NULL;
		}
		fclose(fi);
		
		if (sig[0] == 'P' && sig[1] == 'K')
		{
			// We have a zip archive.
			unzFile zipFile = unzOpen(inPath);
			if (zipFile == NULL) return NULL;
			
			ns = new MFFileSet;
			
			unzGoToFirstFile(zipFile);
			do { 
				char			fname[512];
				unz_file_info	info;
						
				unzGetCurrentFileInfo(zipFile, &info, fname, sizeof(fname), NULL, 0, NULL, 0);
				ns->mNames.push_back(fname);
				ns->mSizes.push_back(info.uncompressed_size);
				
			} while (unzGoToNextFile(zipFile) == UNZ_OK);
			
			ns->mZipFile = zipFile;
			ns->mType = (ns->mNames.size() > 1) ? mf_ZipFiles : mf_ZipFile;
			ns->mHasListing = true;
			ns->mPath = inPath;
			
			return ns;
		}
		if (sig[0] == 0x1f && sig[1] == 0x8b)
		{
			// We have a GZ archive
			ns = new MFFileSet;
			ns->mZipFile = NULL;
			ns->mType = mf_GZTarBall;
			ns->mHasListing = true;
			ns->mPath = inPath;
			MF_Untar(inPath, true, 
				FileSet_TarballOpen, FileSet_TarballRead, FileSet_TarballClose, ns);
			if (ns->mNames.empty())
			{
				delete ns;
				return NULL;
			}
			return ns;			
		}
	}
	
	return NULL;	
}

void			FileSet_Close(MFFileSet * fs)
{
	for (vector<char *>::iterator i = fs->mFileData.begin(); i != fs->mFileData.end(); ++i)
	{
		if (*i)
			free(*i);
	}
	if (fs->mZipFile != NULL)
		unzClose(fs->mZipFile);
	delete fs;
}

int				FileSet_Count(MFFileSet * fs)
{
	FileSet_LoadDirectory(fs);
	return fs->mNames.size();
	
}

const char *	FileSet_GetNth(MFFileSet * fs, int n)
{
	FileSet_LoadDirectory(fs);
	return fs->mNames[n].c_str();
}

MFMemFile *		FileSet_OpenNth(MFFileSet * fs, int n)
{
	FileSet_LoadDirectory(fs);
	
	// If we have the data cached just use it.  This could
	// be the case for a tar ball or for some other file 
	// format where we're doing file-open caching.
	if (fs->mFileData.size() > n && fs->mFileData[n])
	{
		MFMemFile * mf = new MFMemFile;
		mf->mFree = false;
		mf->mClose = false;
		mf->mUnmap = false;
		mf->mFile = 0;
		mf->mBegin = fs->mFileData[n];
		mf->mEnd = mf->mBegin + fs->mSizes[n];
		return mf;
	}
	
	if (fs->mType == mf_Directory)
	{
		string	fpath = fs->mPath + DIR_CHAR + fs->mNames[n];
		return MemFile_Open(fpath.c_str());
	} else if (fs->mType == mf_ZipFile || fs->mType == mf_ZipFiles) {

		if (unzLocateFile(fs->mZipFile, fs->mNames[n].c_str(), 1) != UNZ_OK)			
			return NULL;

		char * buf = (char *) malloc(fs->mSizes[n]);
		if (buf == NULL) return NULL;

		if (unzOpenCurrentFile(fs->mZipFile) == UNZ_OK) 
		{
			if (unzReadCurrentFile(fs->mZipFile, buf, fs->mSizes[n]) == fs->mSizes[n])
			{
				MFMemFile * mf = new MFMemFile;
				mf->mFree = true;
				mf->mClose = false;
				mf->mUnmap = false;
				mf->mFile = 0;
				mf->mBegin = buf;
				mf->mEnd = buf + fs->mSizes[n];
				unzCloseCurrentFile(fs->mZipFile);
				return mf;
			}
			unzCloseCurrentFile(fs->mZipFile);
		}
		free(buf);
		return NULL;
	}
	return NULL;
}

MFMemFile *		FileSet_OpenSpecific(MFFileSet * fs, const char * fname)
{
	if (fs->mType == mf_Directory)
	{
		string fpath = fs->mPath + DIR_CHAR + fname;
		return MemFile_Open(fpath.c_str());
	} else if (fs->mType == mf_ZipFiles || fs->mType == mf_ZipFile)
	{
		for (int n = 0; n < fs->mNames.size(); ++n)
		{
			if (fs->mNames[n] == fname)
				return FileSet_OpenNth(fs, n);
		}
		return NULL;
	}
	return NULL;
}

const char *	MemFile_GetBegin(MFMemFile * inFile)
{
	return inFile->mBegin;
}

const char *	MemFile_GetEnd(MFMemFile * inFile)
{
	return inFile->mEnd;
}

MFMemFile * 	MemFile_Open(const char * inPath)
{
	FILE *		fi = NULL;
	char *		mem = NULL;
	int			file_size = 0;
	MFMemFile *	obj = NULL;
	unzFile		unz = NULL;
#if APL || LIN
	struct stat	ss;			// Put this here to avoid crossing
	int			fd = 0;		// definition when you do a goto!
	void *		addr = NULL;		// Not that you should be doing that
	int			len = 0;	// anyway.
#endif
	
	obj = new MFMemFile;
	if (!obj) goto bail;
	
	unz = unzOpen(inPath);
	if (unz)
	{
		unz_global_info	info;
		if (unzGetGlobalInfo(unz, &info) == UNZ_OK)
		if (info.number_entry == 1)
		{
			unz_file_info	finfo;
			if (unzGoToFirstFile(unz) == UNZ_OK)
			if (unzGetCurrentFileInfo(unz, &finfo, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
			{
				file_size = finfo.uncompressed_size;
				mem = (char *) malloc(file_size);
				if (!mem) goto bail;
				
				unzOpenCurrentFile(unz);
				unzReadCurrentFile(unz, mem, file_size);
				unzCloseCurrentFile(unz);
				unzClose(unz);
				unz = NULL;
				obj->mBegin = mem;
				obj->mEnd = mem + file_size;
				obj->mFree = true;
				obj->mClose = false;
				obj->mUnmap = false;
				obj->mFile = 0;				
				return obj;
			}
		}
		unzClose(unz);
	}
	
#if APL	|| LIN

	#if LIN || __MACH__
		fd = bsd_open(inPath, O_RDONLY, 0);
	#else
	Str255	pStrPath;
	FSSpec	spec;
	FSRef	ref;
	char	osx_path[1024];
	int pStrLen = strlen(inPath);
	if (pStrLen > 255) pStrLen = 255;
	pStrPath[0] = pStrLen;
	memcpy(pStrPath+1,inPath, pStrLen);
	if(FSMakeFSSpec(0, 0, pStrPath, &spec) != noErr) goto cleanmmap;
	FSpMakeFSRef(&spec, &ref);
	FSRefMakePath(&ref, (unsigned char *) osx_path, sizeof(osx_path)-1);
	fd = bsd_open(osx_path, O_RDONLY, 0);
#endif

	if (fd == 0 || fd == -1) goto cleanmmap;
	
	if (bsd_fstat(fd, &ss) < 0) goto cleanmmap;
	len = ss.st_size;

	addr = bsd_mmap(NULL, len, PROT_READ, MAP_FILE, fd, 0);
	if (addr == 0) goto cleanmmap;
	if (addr == (void *) -1) goto cleanmmap;
	
	obj->mBegin = (char *) addr;
	obj->mEnd = obj->mBegin + len;
	obj->mFree = false;
	obj->mClose = true;
	obj->mUnmap = true;
	obj->mFile = fd;	
	return obj;
	
cleanmmap:
	if (addr != 0 && addr != (void *) -1) bsd_munmap(addr, len);	
	if (fd) bsd_close(fd);
	fd = 0;
	addr = NULL;
#endif

#if __INTEL__ && !LIN
	HANDLE			winFile = NULL;
	HANDLE			winFileMapping = NULL;
	char *			winAddr = NULL;

	winFile = CreateFile(inPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!winFile) goto cleanupwin;

	winFileMapping = CreateFileMapping(winFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!winFileMapping) goto cleanupwin;

	winAddr = (char *) MapViewOfFile(winFileMapping, FILE_MAP_READ, 0, 0, 0);
	obj->mBegin = winAddr;
	obj->mEnd = obj->mBegin + GetFileSize(winFile, NULL);
	obj->mFree = false;
	obj->mClose = false;
	obj->mUnmap = true;
	obj->mFile = NULL;
	obj->mWinFile = winFile;
	obj->mWinFileMapping = winFileMapping;
	return obj;
cleanupwin:
	if (winAddr) 		UnmapViewOfFile(winAddr);
	if (winFileMapping) CloseHandle(winFileMapping);
	if (winFile) 		CloseHandle(winFile);
#endif

	
	fi = fopen(inPath, "rb");
	if (!fi) goto bail;

	fseek(fi, 0L, SEEK_END);
	file_size = ftell(fi);
	fseek(fi, 0L, SEEK_SET);
	
	mem = (char *) malloc(file_size);
	if (!mem) goto bail;
	
	if (fread(mem, 1, file_size, fi) != file_size)
		goto bail;

	fclose(fi);
	obj->mBegin = mem;
	obj->mEnd = mem + file_size;
	obj->mFree = true;
	obj->mClose = false;
	obj->mUnmap = false;
	obj->mFile = 0;

	return obj;	
bail:
	if (unz) unzClose(unz);
	if (obj) delete obj;
	if (mem) free(mem);
	if (fi) fclose(fi);	
	return NULL;
}

void		MemFile_Close(MFMemFile * inFile)
{
	if (inFile->mFree)
		free((void *) inFile->mBegin);
#if APL || LIN
	if (inFile->mUnmap)
		bsd_munmap((void *) inFile->mBegin, inFile->mEnd - inFile->mBegin);
	if (inFile->mClose)
		bsd_close(inFile->mFile);
#endif		
#if IBM
	if (inFile->mUnmap)
	{	
		UnmapViewOfFile(inFile->mBegin);
		CloseHandle(inFile->mWinFileMapping);
		CloseHandle(inFile->mWinFile);
	}
#endif
	delete inFile;
}

struct	MFTextScanner {
	const char * mRunBegin;
	const char * mRunEnd;
	const char * mFileEnd;
};

const char *	TextScanner_GetBegin	(MFTextScanner * inScanner)
{
	return inScanner->mRunBegin;
}

const char *	TextScanner_GetEnd		(MFTextScanner * inScanner)
{
	return inScanner->mRunEnd;
}

MFTextScanner *	TextScanner_Open		(MFMemFile * inFile)
{
	MFTextScanner * scanner = new MFTextScanner;
	scanner->mRunBegin = inFile->mBegin;
	scanner->mRunEnd = inFile->mBegin;
	scanner->mFileEnd = inFile->mEnd;
	
	while (scanner->mRunEnd < scanner->mFileEnd && 
				*(scanner->mRunEnd) != '\n' &&
				*(scanner->mRunEnd) != '\r')
		scanner->mRunEnd++;
	
	return scanner;
}

MFTextScanner *	TextScanner_OpenMem		(const char * inBegin, const char * inEnd)
{
	MFTextScanner * scanner = new MFTextScanner;
	scanner->mRunBegin = inBegin;
	scanner->mRunEnd = inBegin;
	scanner->mFileEnd = inEnd;
	
	while (scanner->mRunEnd < scanner->mFileEnd && 
				*(scanner->mRunEnd) != '\n' &&
				*(scanner->mRunEnd) != '\r')
		scanner->mRunEnd++;
	
	return scanner;
}

void			TextScanner_Close		(MFTextScanner * inScanner)
{
	delete inScanner;
}

bool			TextScanner_IsDone		(MFTextScanner * inScanner)
{
	return inScanner->mRunBegin >= inScanner->mFileEnd;
}

void			TextScanner_Next		(MFTextScanner * s)
{
	if (s->mRunBegin == s->mFileEnd)	return;

	s->mRunBegin = s->mRunEnd;
	if (*(s->mRunBegin) == '\r')
	{
		s->mRunBegin++;
		if (s->mRunBegin == s->mFileEnd)	return;

		if (*(s->mRunBegin) == '\n')
			s->mRunBegin++;			
	} else
		s->mRunBegin++;
	s->mRunEnd = s->mRunBegin;
	
	while (s->mRunEnd < s->mFileEnd && 
				*(s->mRunEnd) != '\n' &&
				*(s->mRunEnd) != '\r')
		s->mRunEnd++;
}

char			TextScanner_ExtractChar(MFTextScanner * inScanner, int inBegin)
{
	const char * line = TextScanner_GetBegin(inScanner);
	return *(line + inBegin);
}

void			TextScanner_ExtractString(MFTextScanner * inScanner, int inBegin, int inEnd, string& outString, bool inTrim)
{
	const char * line = TextScanner_GetBegin(inScanner);
	const char * sp = line + inBegin;
	const char * ep = line + inEnd;
	while (sp < ep && isspace(*sp))
		++sp;
	while (sp < ep && isspace(*(ep-1)))
		--ep;
			
	outString = string(sp, ep);
}

long			TextScanner_ExtractLong(MFTextScanner * inScanner, int inBegin, int inEnd)
{
	const char * line = TextScanner_GetBegin(inScanner);
	const char * s = line + inBegin;
	const char * e = line + inEnd;
	
	long	retVal = 0;
	bool	neg = false;
	
	while (s < e && isspace(*s)) ++s;
	if (*s == '-') { neg = true; ++s; }
	if (*s == '+') ++s;
	
	while (s < e && (*s) >= '0' && (*s) <= '9')
	{
		retVal *= 10;
		retVal += ((*s) - '0');
		++s;
	}
	return (neg) ? (-retVal) : retVal;
}


unsigned long	TextScanner_ExtractUnsignedLong(MFTextScanner * inScanner, int inBegin, int inEnd)
{
	const char * line = TextScanner_GetBegin(inScanner);
	const char * s = line + inBegin;
	const char * e = line + inEnd;
	
	long	retVal = 0;
	
	while (s < e && isspace(*s)) ++s;
	if (*s == '+') ++s;
	
	while (s < e && (*s) >= '0' && (*s) <= '9')
	{
		retVal *= 10;
		retVal += ((*s) - '0');
		++s;
	}
	return retVal;
}


void	TextScanner_TokenizeLine(MFTextScanner * inScanner, const char * inDelim, const char * inTerm, int inMax, TextScanner_TokenizeFunc_f inFunc, void * inRef)
{
	int n;
	int tokens_so_far = 0;
	static	char	lastDelim[256] = { 0 };
	static	char	lastTerm[256] = { 0 };
	static	char	delimLookup[256] = { 0 };
	static	char	termLookup[256] = { 0 };
	if (strcmp(lastDelim, inDelim))
	{
		memset(delimLookup, 0, sizeof(delimLookup));
		n = 0;
		while (inDelim[n])
			delimLookup[(unsigned char) inDelim[n++]] = 1;
	}
	if (strcmp(lastTerm, inTerm))
	{
		memset(termLookup, 0, sizeof(termLookup));
		n = 0;
		while (inTerm[n])
			termLookup[(unsigned char) inTerm[n++]] = 1;
		termLookup[0] = 1;	// Null is always a terminator, not that we should ever hit this!
	}

	const unsigned char * begin = (const unsigned char *) inScanner->mRunBegin;
	const unsigned char * end = (const unsigned char *) inScanner->mRunEnd;
	while (begin < end)
	{
		// First skip any white space leading this token.
		while (begin < end && delimLookup[*begin])
			++begin;

		// If the token starts with a # or we hit the newline, we're done, bail.
		if (termLookup[*begin]) return;
		
		// Mark the token start.
		const unsigned char * tokenStart = begin;
		
		// If the token will go to the end becuase of a max limit, nuke
		// the delim table so we just go
		if (tokens_so_far == inMax)
			memset(delimLookup, 0, sizeof(delimLookup));
		
		// Scan to the next white space or # symbol.  This is the whole token.
		while (begin < end && !delimLookup[*begin] && !termLookup[*begin])
			++begin;
		if (!inFunc((const char *) tokenStart, (const char *) begin, inRef))
			break;
		++tokens_so_far;
	}
}

static 	bool TokenizeToVector(const char * inBegin, const char * inEnd, void * inRef)
{
	pair<vector<string>,int> * v = (pair<vector<string>,int> *) inRef;
	
	v->first.push_back(string(inBegin, inEnd));
	return v->first.size() < v->second;
}

int				TextScanner_FormatScan(MFTextScanner * inScanner, const char * fmt, ...)
{
	pair<vector<string>, int>	tokens;
	const  char * stop_pt = strstr(fmt, "|");
	int inMax = (stop_pt ? (stop_pt - fmt - 1) : -1);
	tokens.second = strlen(fmt);
	TextScanner_TokenizeLine(inScanner, " \t", "\r\n", inMax, TokenizeToVector,&tokens);
	va_list	arg;
	va_start(arg, fmt);
	int n = 0;
	double * dptr;
	float * fptr;
	int * iptr;
	short * sptr;
	char * tptr;
	string *	Tptr;

	while (*fmt && n < tokens.first.size())
	{
		switch(*fmt) {
		case 'f':	fptr = va_arg(arg, float*);			*fptr = atof(tokens.first[n].c_str());			break;
		case 'd':	dptr = va_arg(arg, double*);		*dptr = atof(tokens.first[n].c_str());			break;
		case 'i':	iptr = va_arg(arg, int*);			*iptr = atoi(tokens.first[n].c_str());			break;
		case 's':	sptr = va_arg(arg, short*);			*sptr = atoi(tokens.first[n].c_str());			break;
		case 't':	tptr = va_arg(arg, char*);			strcpy(tptr,tokens.first[n].c_str());				break;
		case 'T':	Tptr = va_arg(arg, string*);		*Tptr = tokens.first[n];							break;
		}
		++fmt;
		++n;
	}
	va_end(arg);
	return n;
}


bool	MF_IterateDirectory(const char * dirPath, bool (* cbFunc)(const char * fileName, bool isDir, void * ref), void * ref)
{
#if (APL && __MACH__) || LIN
     DIR* dir;
     struct dirent* ent;
     dir = opendir(dirPath);
     if (!dir) return false;

     while ((ent = readdir(dir)))
     {
         struct stat ss;

         string	fullPath(dirPath);
         fullPath += DIR_CHAR;
         fullPath += ent->d_name;

         if (stat(fullPath.c_str(), &ss) < 0)
            continue;

		if (cbFunc(ent->d_name, S_ISDIR(ss.st_mode), ref))
			break;

	}     
     closedir(dir);
     return true;

#elif APL

		CInfoPBRec 			pb = { 0 };
		string				path(dirPath), comp;
		FSSpec				dirLoc = { 0 };
		string::size_type 	sep;
		Str255				fnameP;
		int					index;
	
	sep = path.find(DIR_CHAR);
	if (sep == path.npos)
	{
		path += DIR_CHAR;
		comp = path;
	} else
		comp = path.substr(0, sep+1);
	memcpy(fnameP+1,comp.c_str(), comp.size());
	fnameP[0] = comp.size();
	if (::FSMakeFSSpec(0, 0, fnameP, &dirLoc) != noErr) return false;
	if (dirLoc.parID == fsRtParID)
		dirLoc.parID = fsRtDirID;
	
	if (sep != path.npos)
		path.erase(0,sep+1);
	else
		path.clear();
		
	while(!path.empty())
	{
		sep = path.find(DIR_CHAR);
		if (sep == path.npos) 
			comp = path;
		else
			comp = path.substr(0, sep);
		memcpy(fnameP+1,comp.c_str(), comp.size());
		fnameP[0] = comp.size();
		
		pb.dirInfo.ioNamePtr = fnameP;
		pb.dirInfo.ioVRefNum = dirLoc.vRefNum;
		pb.dirInfo.ioDrDirID = dirLoc.parID;
		pb.dirInfo.ioFDirIndex = 0;
		
		if (PBGetCatInfoSync(&pb) != noErr)
			return false;
		
		if ((pb.dirInfo.ioFlAttrib & ioDirMask) == 0) 
			return false;
		dirLoc.parID = pb.dirInfo.ioDrDirID;

		if (sep == path.npos)
			path.clear();
		else
			path.erase(0, sep+1);		
	}
	
	index = 1;
	
	while (1)
	{
		pb.hFileInfo.ioNamePtr = fnameP;
		pb.hFileInfo.ioVRefNum = dirLoc.vRefNum;
		pb.hFileInfo.ioDirID = dirLoc.parID;
		pb.hFileInfo.ioFDirIndex = index;
		if (PBGetCatInfoSync(&pb) != noErr)
			return true;
		
		char	buf[256];
		memcpy(buf,fnameP+1,fnameP[0]);
		buf[fnameP[0]] = 0;
		if (cbFunc(buf, pb.hFileInfo.ioFlAttrib & ioDirMask, ref))
			return true;
	
		++index;
	}

#elif IBM

	char path[MAX_PATH], SearchPath[MAX_PATH], FilePath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;		
	strcpy(SearchPath, dirPath);
	strcpy(path, dirPath);
	strcat(path, "*.*");

	hFind = FindFirstFile(path, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	else 
	{
		if ( !( (strcmp(FindFileData.cFileName, ".") == 0) || (strcmp(FindFileData.cFileName, "..") == 0) ) )
		{
			strcpy(FilePath, SearchPath);
			strcat(FilePath, FindFileData.cFileName);
			if (cbFunc(FilePath, FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY, ref))
				return true;
		}

		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			if ( !( (strcmp(FindFileData.cFileName, ".") == 0) || (strcmp(FindFileData.cFileName, "..") == 0) ) )
			{
				strcpy(FilePath, SearchPath);
				strcat(FilePath, FindFileData.cFileName);
				if (cbFunc(FilePath, FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY, ref))
					return true;
			}
		}
	}
	return true;
	FindClose(hFind);
#else
	#error PLATFORM NOT KNOWN
#endif	
}


MF_FileType	MF_GetFileType(const char * path, int analysis_level)
{
	long long	file_size = -1;

#if (APL && __MACH__) || LIN

	struct stat ss;
	if (stat(path, &ss) < 0)
		return mf_BadFile;
	if (S_ISDIR(ss.st_mode) != 0) return mf_Directory;
	file_size = ss.st_size;

#elif APL

		CInfoPBRec 			pb = { 0 };
		string				stlpath(path), comp;
		FSSpec				dirLoc = { 0 };
		string::size_type 	sep;
		Str255				fnameP;
	
	sep = stlpath.find(DIR_CHAR);
	if (sep == stlpath.npos)
	{
		stlpath += DIR_CHAR;
		comp = stlpath;
	} else
		comp = stlpath.substr(0, sep+1);
	memcpy(fnameP+1,comp.c_str(), comp.size());
	fnameP[0] = comp.size();
	if (::FSMakeFSSpec(0, 0, fnameP, &dirLoc) != noErr) return mf_BadFile;
	if (dirLoc.parID == fsRtParID)
		dirLoc.parID = fsRtDirID;
	
	if (sep != stlpath.npos)
		stlpath.erase(0,sep+1);
	else  
		stlpath.clear();

	if (stlpath.empty()) 
		return mf_Directory;
	
	while(!stlpath.empty())
	{
		sep = stlpath.find(DIR_CHAR);
		if (sep == stlpath.npos) 
			comp = stlpath;
		else
			comp = stlpath.substr(0, sep);
		memcpy(fnameP+1,comp.c_str(), comp.size());
		fnameP[0] = comp.size();
		
		pb.dirInfo.ioNamePtr = fnameP;
		pb.dirInfo.ioVRefNum = dirLoc.vRefNum;
		pb.dirInfo.ioDrDirID = dirLoc.parID;
		pb.dirInfo.ioFDirIndex = 0;
		
		if (PBGetCatInfoSync(&pb) != noErr)
			return mf_BadFile;
		
		dirLoc.parID = pb.dirInfo.ioDrDirID;

		if (sep == stlpath.npos)
			stlpath.clear();
		else
			stlpath.erase(0, sep+1);		
	}

	if (pb.dirInfo.ioFlAttrib & ioDirMask)
		return mf_Directory;			
	
	file_size = pb.hFileInfo.ioFlLgLen;

#elif IBM
	struct stat ss;
	if (stat(path, &ss) < 0)
		return mf_BadFile;
//	if (ss.st_mode & S_IFDIR) return mf_Directory;
	if (S_ISDIR(ss.st_mode) != 0) 
	file_size = ss.st_size;

#else
	#error PLATFORM NOT KNOWN
#endif

	// At this point we know we have a file.
	if (analysis_level == 1)
	{
		// What kind of datafile is it?  Well, if its size is < 2 bytes and known, it must not 
		// be some kind of zip.
		
		if (file_size > 0 && file_size < 2) return mf_DataFile;
		
		// Read the first two bytes as a signature...that'll ID a ZIP vs. a 
		FILE * fi = fopen(path, "rb");
		if (!fi)	return mf_BadFile;
		unsigned char	buf[2] = { 0 };
		fread(buf, 2, 1, fi);
		fclose(fi);
		if (buf[0] == 'P' && buf[1] == 'K') return mf_ZipFile;
		if (buf[0] == 0x1f && buf[1] == 0x8b) return mf_GZFile;
	}

	return mf_DataFile;
}

static int getoct(char *p,int width)
{
  int result = 0;
  char c;
  
  while (width --)
    {
      c = *p++;
      if (c == ' ')
	continue;
      if (c == 0)
	break;
      result = result * 8 + (c - '0');
    }
  return result;
}

int
MF_GetDirectoryBulk(
		const char *		path,
		bool (* 			cbFunc)(const char * fileName, bool isDir, unsigned long long modTime, void * refcon),
		void *				refcon)
{
#if APL
		FSSpec			spec;
		FSRef			ref;
		OSErr			err;
		Str255			pStr;
		FSIterator		iter;
		int				total = 0;
		ItemCount		fetched;
		FSCatalogInfo	infos[256];
		HFSUniStr255	names[256];

		CFURLRef	cfurl;
		CFStringRef	cfstr;

	cfstr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingMacRoman);
	if (cfstr == NULL) return 0;
	
	#if defined(__MWERKS__)
		cfurl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfstr, kCFURLHFSPathStyle, TRUE);
	#else
 		cfurl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfstr, kCFURLPOSIXPathStyle, TRUE);
	#endif
	CFRelease(cfstr);
	if (cfurl == NULL) return 0;

	if (!CFURLGetFSRef(cfurl, &ref))
	{
		CFRelease(cfurl);
		return 0;
	}
	CFRelease(cfurl);
		
	err = FSOpenIterator(&ref, kFSIterateFlat, &iter);
	if (err != noErr) return 0;

	while (1)
	{
		err = FSGetCatalogInfoBulk(iter, 256, &fetched, NULL, kFSCatInfoNodeFlags | kFSCatInfoContentMod, infos, NULL, NULL, names);
		if (err != noErr && err != errFSNoMoreItems)
			break;
			
		for (int n = 0; n < fetched; ++n)
		{
			int m = names[n].length;
			char * d = (char *) names[n].unicode;
			UniChar * s = names[n].unicode;
			while(m--)
				*d++ = *s++;
			((char *) names[n].unicode)[names[n].length] = 0;
			if (!cbFunc((const char *) names[n].unicode, infos[n].nodeFlags & kFSNodeIsDirectoryMask, *((unsigned long long *) &infos[n].contentModDate), refcon))
			{
				FSCloseIterator(iter);
				return total;
			}
			++total;
		}
		
		if (err == errFSNoMoreItems)
			break;
	}
	
	FSCloseIterator(iter);
	return total;
			
#elif IBM

	char				searchPath[MAX_PATH];
	WIN32_FIND_DATA		findData;
	HANDLE				hFind;
	int					total = 0;
	unsigned long long	when;

	strcpy(searchPath,path);
	strcat(searchPath,"\\*.*");

	hFind = FindFirstFile(searchPath,&findData);
	if (hFind == INVALID_HANDLE_VALUE) return 0;

	++total;
	when = ((unsigned long long) findData.ftLastWriteTime.dwHighDateTime << 32) | ((unsigned long long) findData.ftLastWriteTime.dwLowDateTime);

	if (cbFunc(findData.cFileName, findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY, when, refcon))
	{
		while(FindNextFile(hFind,&findData) != 0)
		{
			++total;
			when= ((unsigned long long) findData.ftLastWriteTime.dwHighDateTime << 32) | ((unsigned long long) findData.ftLastWriteTime.dwLowDateTime);
			if (!cbFunc(findData.cFileName, findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY, when, refcon)) break;
		}
	}

	FindClose(hFind);
	return total;

#else
#error not implemented
#endif
}


bool	MF_Untar(		const char * 	path,
						bool			unzip,
						void * 	(* TarOpenFile_f)	(const char * filename, int filesize, void * tarRef),
						int		(* TarWriteFile_f)	(const char * data, int len, void * tarRef, void * fileRef),
						void	(* TarCloseFile_f)	(void * tarRef, void * fileRef),
						void *			tarRef)
{
	FILE *		fi = NULL;
	gzFile		gfi = NULL;
	int			remaining = 0;
	bool		expect_header = true;
	bool		success = false;
	tar_buffer	block_buf;
	void *		openOut = NULL;
	int			len;
	
	if (unzip) {
		gfi = gzopen(path, "rb"); if (!gfi) goto bail;
	} else {
		fi = fopen(path, "rb"); if (!fi) goto bail;
	}

	while(!(unzip ? gzeof(gfi) : feof(fi)))
	{
		if (unzip)
			len = gzread(gfi, &block_buf, BLOCKSIZE);
		else
			len = fread(&block_buf, 1, BLOCKSIZE, fi);
		if (len != BLOCKSIZE)
			goto bail;
			
		if (expect_header)
		{
			switch(block_buf.header.typeflag) {
			case REGTYPE:
			case AREGTYPE:
				remaining = getoct(block_buf.header.size, 12);
				if (block_buf.header.name[0] == 0)
				{
					if (remaining > 0) goto bail;
					continue;
				}
				openOut = TarOpenFile_f(block_buf.header.name, remaining, tarRef);
				if (!openOut) goto bail;
				expect_header = false;
				break;
			}
		} else {
			if (len > remaining) len = remaining;
			if (TarWriteFile_f(block_buf.buffer, len, tarRef, openOut) != len)
				goto bail;
			remaining -= len;
			if (remaining == 0)
			{
				TarCloseFile_f(tarRef, openOut);
				openOut = NULL;
				expect_header = 1;
			}
		}
	}
	if (expect_header)
		success = true;

bail:
	if (fi) fclose(fi);
	if (gfi) gzclose(gfi);
	return success;		
}

inline int iseoln(const char c) { return c == '\n' || c == '\r'; }

void	MFS_init(MFScanner * scanner, MFMemFile * inFile)
{
	scanner->begin = scanner->cur = MemFile_GetBegin(inFile);
					 scanner->end = MemFile_GetEnd(inFile);
}

void	MFS_init(MFScanner * scanner, const char * begin, const char * end)
{
	scanner->begin = scanner->cur = begin;
					 scanner->end = end;
}

int		MFS_done(MFScanner * s)
{
	return (s->cur >= s->end);
}


void	MFS_string_eol(MFScanner * s, string * out_string)
{
	while(s->cur<s->end && isspace(*s->cur)						)s->cur++;	const char * c1=s->cur;
	while(s->cur<s->end && !iseoln(*s->cur)						)s->cur++;	const char * c2=s->cur;	if(out_string)*out_string=string(c1,c2);
	while(s->cur<s->end && (iseoln(*s->cur) || isspace(*s->cur)))s->cur++;
}

void	MFS_string(MFScanner * s, string * out_string)
{
	while(s->cur<s->end &&  isspace(*s->cur)						)s->cur++;	const char* c1=s->cur;
	while(s->cur<s->end && !isspace(*s->cur) && !iseoln(*s->cur)	)s->cur++;	const char* c2=s->cur;	if(out_string)*out_string=string(c1,c2);
}

int		MFS_string_match(MFScanner * s, const char * input, int eol_ok)
{
	while(s->cur<s->end && isspace(*s->cur)			   ) s->cur++;				const char* c1=s->cur;
	while(s->cur<s->end && *s->cur==*input && *input!=0){s->cur++; input++;}

	if(*input==0 && 		  isspace(*s->cur))return 1;
	if(*input==0 && eol_ok && iseoln(*s->cur) )return 1;
	s->cur=c1;								   return 0;
}

int		MFS_int(MFScanner * s)
{
	while(s->cur<s->end && isspace(*s->cur))s->cur++;

	xint sign_mult=1;
	if(s->cur<s->end && *s->cur=='-'){sign_mult=-1;	s->cur++;}
	if(s->cur<s->end && *s->cur=='+'){sign_mult= 1;	s->cur++;}

	xint retval=0;
	while(s->cur<s->end && !isspace(*s->cur) && !iseoln(*s->cur)){
		retval=(10*retval)+(*s->cur)-'0';
		s->cur++;}

	return sign_mult * retval;
}

double	MFS_double(MFScanner * s)
{
	while(s->cur<s->end && isspace(*s->cur))s->cur++;

	double	sign_mult	=1;
	double	ret_val		=0;
	int		decimals	=0;
	int		has_decimal	=0;

	while(s->cur<s->end && !isspace(*s->cur) && !iseoln(*s->cur))
	{
			 if(*s->cur=='-')sign_mult   =-1.0;
		else if(*s->cur=='+')sign_mult   = 1.0;
		else if(*s->cur=='.')has_decimal =1;
		else{
			ret_val=(10*ret_val)+(*s->cur)-'0';
			if(has_decimal)decimals++;}

		s->cur++;
	}
	return ret_val/pow((double)10,(double)decimals)*sign_mult;
}
