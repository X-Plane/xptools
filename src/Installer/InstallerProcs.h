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
#ifndef INSTALLERPROCS_H
#define INSTALLERPROCS_H

#include "XFileTwiddle.h"


// An installer chunk - basically a block of memory plus memory management
// protection and a smart header parser.
struct InstallerChunk {
	char *		mem;
	int			len;
	InstallerChunk();
	~InstallerChunk();	
	void Assign(char * mem, int len);
	
	void	Decode(char& exists_old, char& exists_new, MD5_Sig& outOldSig, MD5_Sig& outNewSig, long& outZipSize, long& outRealSize, char ** outData);
private:
	InstallerChunk& operator=(const InstallerChunk& rhs);
	InstallerChunk(const InstallerChunk&);
	
};

enum {
	file_CreateOK,				// A file needs to be created and it is safe to do so.
	file_CreateUnexpected,		// A file needs to be created - there is something in the way!
	file_CreatedAlready,		// A file that we thought we'd have to create is already there.

	file_MatchesOK,				// A needs to be updated.
	file_MatchesUnexpected,		// A file needs to be updated but the old file has been modified.
	file_MatchesMissing,		// A file needs to be updated - the old file has been deleted.
	file_MatchesAlready,		// A file that needs to be updated already has been.

	file_DeleteOK,				// A file needs to be deleted.
	file_DeleteUnexpected,		// A file needs to be deleted but has been modified.
	file_DeleteAlready			// A file needs to be deleted but already has been.
};

// This checks the files on disk for identical contents.
bool	FilesAreSame(const char * inPath1, const char * inPath2);
// This checks an installer chunk against disk for its status.
int		FileStatus(const char * inPath1, InstallerChunk& inChunk);

// A utility routine to sign an MD5.  Returns true if the file
// exists.
bool	GetFileMD5(const char * inPath, MD5_Sig& outSig);

// Build a chunk, return true if it works.
int		BuildChunk(const char * inPathOld, const char * inPathNew, InstallerChunk& outChunk, const char * basePath);

// Install a chunk on disk.
void	InstallChunk(InstallerChunk& inChunk, const char * basePath);

bool	MF_IterateDirectory(const char * dirPath, bool (* cbFunc)(const char * fileName, bool isDir, void * ref), void * ref);

#endif
