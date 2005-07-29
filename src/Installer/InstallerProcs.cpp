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
#include "InstallerProcs.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include "ErrMsg.h"
#include "PlatformUtils.h"
#include <errno.h>

using namespace std;

bool	MF_IterateDirectory(const char * dirPath, bool (* cbFunc)(const char * fileName, bool isDir, void * ref), void * ref)
{
#if APL
#if __MACH__
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

#else

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

#endif
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
			if (cbFunc(FindFileData.cFileName, (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), ref))
				return true;
		}

		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			if ( !( (strcmp(FindFileData.cFileName, ".") == 0) || (strcmp(FindFileData.cFileName, "..") == 0) ) )
			{
				strcpy(FilePath, SearchPath);
				strcat(FilePath, FindFileData.cFileName);
				if (cbFunc(FindFileData.cFileName, (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), ref))
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




using namespace std;

InstallerChunk::InstallerChunk() : mem(NULL), len(0) { }
InstallerChunk::~InstallerChunk() { if (mem) free(mem); }
	void InstallerChunk::Assign(char * imem, int ilen)
	{
	 if (mem) free(mem);
	 mem = imem; len = ilen;
	}

void	InstallerChunk::Decode(
		char& exists_old,
		char& exists_new,
		MD5_Sig& outOldSig, MD5_Sig& 
		outNewSig, long& outZipSize, long& outRealSize, char ** outData)
{
	char * p = mem;
	p += strlen(p) + 1;
	exists_old = *p;
	++p;
	exists_new = *p;
	++p;
	memcpy(outOldSig.digest, p, 16);
	p += 16;
	memcpy(outNewSig.digest, p, 16);
	p += 16;
	memcpy(&outZipSize, p, 4);
	p += 4;
	memcpy(&outRealSize, p, 4);
	p += 4;
	*outData = p;	
}

// FORMAT OF AN INSTALLER CHUNK:
// 1. NULL-terminated file name.
// 2. 1 char each old exists, new exists?
// 3. MD5-hash of old file
// 4. MD-5 hash of new file
// 5. 4-byte size of file zipped image
// 6. 4-byte size of actual file
// 7. The file's image, zipped.

// Compare files, whether both on disk, or both in mem.
bool	FilesAreSame(const char * inPath1, const char * inPath2)
{
	int as, bs;
	as = GetFileBlockSizeIfExists(inPath1);
	bs = GetFileBlockSizeIfExists(inPath2);
	if (as != bs) return false;
	if (as == -1 && bs == -1) return true;	// Both missing!
	MD5_Sig	a, b;
	bool a_exists = GetFileMD5(inPath1, a);
	bool b_exists = GetFileMD5(inPath2, b);
	if (a_exists != b_exists) return false;
	if (!a_exists) return true;
	return a == b;
}

int	FileStatus(const char * inPath1, InstallerChunk& inChunk)
{	
	MD5_Sig	old_sig, new_sig, our_sig;
	char	old_exists, new_exists;
	char our_exists = GetFileMD5(inPath1, our_sig);
	char * d;
	long zsize,fsize;
	inChunk.Decode(old_exists, new_exists, old_sig, new_sig, zsize, fsize, &d);
	
	// FILE CHANGED IN X-PLANE
	if (new_exists && old_exists)
	{
		if (our_exists)
		{
			return (new_sig == our_sig) ? file_MatchesAlready :
				((old_sig == our_sig) ? file_MatchesOK : file_MatchesUnexpected);
		} else 
			return file_MatchesMissing;
	} 
	// FILE WAS CREATED IN X-PLANE
	else if (new_exists && !old_exists)
	{
		if (our_exists)
		{
			return (new_sig == our_sig) ? file_CreatedAlready : file_CreateUnexpected;
		} else 
			return file_CreateOK;
	}
	// FILE WAS REMOVED IN X-PLANE
	else if (!new_exists && old_exists)
	{
		if (our_exists)
			return (old_sig == our_sig) ? file_DeleteOK : file_DeleteUnexpected;
		else
			return file_DeleteAlready;
	} else 
		return file_CreatedAlready;	// SHOULD NEVER GET HERE!
}

bool	GetFileMD5(const char * inPath, MD5_Sig& outSig)
{
	char *	p;
	int sz;
	memset(outSig.digest, 0, 16);
	if (GetFileBlockSizeIfExists(inPath) == -1) return false;
	if (!FileToBlock(inPath, &p, &sz)) return false;
	MD5_Block(p, sz, outSig);
	free(p);
	return true;
}

int	BuildChunk(const char * inPathOld, const char * inPathNew, InstallerChunk& outChunk, const char * basePath)
{
	char	* 	uncomp = NULL;
	int			uncompS = 0;
	char *		comp = NULL;
	int			compS = 0;
	MD5_Sig	old_sig, new_sig;
	
	char exists_old = GetFileMD5(inPathOld, old_sig);
	char exists_new = GetFileMD5(inPathNew, new_sig);
	if (exists_new)
	{
		if (!FileToBlock(inPathNew, &uncomp, &uncompS))
			return 0;
		compS = uncompS + 512 + uncompS / 1000;
		comp = (char *) malloc(compS);
		if (comp == NULL)
		{
			ReportError("Not enough memory to compress", ENOMEM, inPathNew);
			free(uncomp);
			return 0;
		}
		if (!ZipBlock(uncomp, uncompS, comp, &compS))
		{
			ReportError("Error compressing file", -1, inPathNew);
			free(comp);
			free(uncomp);
			return 0;
		}
		free(uncomp);
	}
	outChunk.Assign(NULL, 0);
	const char * partial = inPathNew + strlen(basePath);
	int fnamelen = strlen(partial);
	int	total_size = fnamelen + 1 + 1 + 1 + 16 + 16 + 4 + 4 + compS;
	outChunk.mem = (char *) malloc(total_size);
	if (outChunk.mem == NULL)
	{
		if (comp) free(comp);
		ReportError("Out of memory trying to build image for file", ENOMEM, inPathNew);
		return 0;
	}
	outChunk.len = total_size;
	char * p = outChunk.mem;
	memcpy(p, partial, fnamelen+1);
	p += (fnamelen+1);
	*p = exists_old;
	++p;
	*p = exists_new;
	++p;
	memcpy(p, old_sig.digest, 16);
	p += 16;
	memcpy(p, new_sig.digest, 16);
	p += 16;
	memcpy(p, &compS, 4);
	p += 4;
	memcpy(p, &uncompS, 4);
	p += 4;
	if (compS) memcpy(p, comp, compS);
	if (comp) free(comp);
	return 1;
}

void	InstallChunk(InstallerChunk& inChunk, const char * basePath)
{
	long	zsize, fsize;
	MD5_Sig	old_sig, new_sig;
	char * d;
	char	old_e, new_e;
	inChunk.Decode(old_e, new_e, old_sig, new_sig, zsize, fsize, &d);
	string	full_path(basePath);
	full_path += string(inChunk.mem);
	
	if (new_e)
	{	
		char * block = (char *) malloc(fsize);
		UnzipBlock(d, zsize, block, fsize);
		BlockToFile(full_path.c_str(), block);
		free(block);	
	} else if (old_e) {
		NukeFile(full_path.c_str());
	}
}

