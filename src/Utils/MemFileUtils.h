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
#ifndef MEMFILEUTILS_H
#define MEMFILEUTILS_H

/*

	MemFileUtils - THEORY OF OPERATION
	
	MemFileUtils is a set of file reading commands designed around memory-mapped or pseudo-memory-mapped files.
	
	This lib optimizes for:
	 - Flexibility to read compressed archives.
	 - Speed via bulk reads or memory mapping.
	 - Ease of implementation.
	
	It does not optimize for:
	 - Handling of very large files.
	 - Memory and address space consumption.
	
	Files are managed as ranges of addresses; the entire file is memory mapped or read into memory all at once.
	MemFileUtils are good for GIS systems where data is spread over multiple files and parsing may be CPU-intensive.
	
	These utilities are not good for huge files such as extremely large tar balls or large image files.

 */

/******************************************************************************
 * TYPES
 ******************************************************************************/

/* MFFileSet represents a flat collection of files.  This can be a directory,
 * zip archive, or gzipped tarball. */
struct	MFFileSet;

/* MFMemFile represents a single file, loaded into memory for reading. */
struct	MFMemFile;

/* MFTextScanner scans single lines of text files. */
struct	MFTextScanner;


/******************************************************************************
 * FILE SET ROUTINES
 ******************************************************************************/
MFFileSet *		FileSet_Open(const char * inPath);
void			FileSet_Close(MFFileSet * inFileSet);
int				FileSet_Count(MFFileSet * inFileSet);
const char *	FileSet_GetNth(MFFileSet * inFileSet, int n);
MFMemFile *		FileSet_OpenNth(MFFileSet * inFileSet, int n);
MFMemFile *		FileSet_OpenSpecific(MFFileSet * inFileSet, const char * fileName);

/******************************************************************************
 * FILE ROUTINES
 ******************************************************************************/
const char *	MemFile_GetBegin	(MFMemFile * inFile);
const char *	MemFile_GetEnd		(MFMemFile * inFile);
MFMemFile * 	MemFile_Open		(const char * inPath);
void			MemFile_Close		(MFMemFile * inFile);

/******************************************************************************
 * TEXT SCANNING ROUTINES
 ******************************************************************************/
const char *	TextScanner_GetBegin	(MFTextScanner * inScanner);
const char *	TextScanner_GetEnd		(MFTextScanner * inScanner);
MFTextScanner *	TextScanner_Open		(MFMemFile * inFile);
MFTextScanner *	TextScanner_OpenMem		(const char * inBegin, const char * inEnd);
void			TextScanner_Close		(MFTextScanner * inScanner);
bool			TextScanner_IsDone		(MFTextScanner * inScanner);
void			TextScanner_Next		(MFTextScanner * inScanner);

char			TextScanner_ExtractChar(MFTextScanner * inScanner, int inBegin);
void			TextScanner_ExtractString(MFTextScanner * inScanner, int inBegin, int inEnd, string& outString, bool inTrim);
long			TextScanner_ExtractLong(MFTextScanner * inScanner, int inBegin, int inEnd);
unsigned long	TextScanner_ExtractUnsignedLong(MFTextScanner * inScanner, int inBegin, int inEnd);

// Return true to continue
typedef	bool (* TextScanner_TokenizeFunc_f)(const char * inBegin, const char * inEnd, void * inRef);
void			TextScanner_TokenizeLine(MFTextScanner * inScanner, const char * inDelim, const char * inTerm, int inMax, TextScanner_TokenizeFunc_f inFunc, void * inRef);

// The string characters are: i = int, s = short, t = char buf, T = STL string, f = float, d = double, space = skip.  
// Return the number of processed arguments, limited by eitiher format or number of tokens in the line.
int				TextScanner_FormatScan(MFTextScanner * inScanner, const char * fmt, ...);


/******************************************************************************
 * UTILITIES
 ******************************************************************************/

/* MF_FileType tells what a path points to. */
enum {
	mf_BadFile,
	mf_Directory,
	mf_DataFile,
	mf_ZipFile,
	mf_ZipFiles,
	mf_GZFile,
	mf_GZTarBall
};
typedef int	MF_FileType;

/* Macros to tell if the file makes sense as a single or plural entity. */
inline bool	MF_IsCollection(MF_FileType f) { return f == mf_Directory || f == mf_ZipFile || f == mf_ZipFiles || f == mf_GZTarBall; }
inline bool	MF_IsSingle(MF_FileType f) 	   { return f == mf_DataFile || f == mf_ZipFile || f == mf_GZFile; }

/* 
 * MF_IterateDirectory
 *
 * MF_IterateDirectory goes through a real physical directory, providing a 
 * callback for each file or sub directory.  Return false from your callback 
 * to halt scanning.  Returns true if the whole directory is scanned without 
 * error.
 *
 */
bool		
MF_IterateDirectory(
		const char * 	dirPath, 
		bool (* 		cbFunc)(const char * fileName, bool isDir, void * ref), 
		void * 			ref);

/* 
 * MF_GetFileType
 *
 * Examine a path to determine what it points to. Three levels of analysis: 
 * Level 0 = just see if we have a file, dir or bad file.
 * Level 1 = identify compressed archives by their headers.
 * Level 2 = do enough decompression to distinguish number of zip files and tar balls.
 *
 */
MF_FileType	
MF_GetFileType(
		const char * 	path, 
		int 			analysis_level);

/*
 * MF_Untar
 *
 * Opens a tar ball and returns the contents.  Pass true for unzip to ungzip the tarball first.
 * You provide three callbacks: the first receives a new file name and size and returns a ref
 * to be used for I/O.  The second is called to add bytes to the file.  The third closes the
 * file.  Return NULL if your open routine fails; return other than the length to be written
 * if your write routine fails.  Either error will halt MF_Untar.  MF_Untar returns true if the
 * whole archive was decompressed successfully. 
 *
 */
bool	
MF_Untar(
		const char * 	path, 
		bool 			unzip,
		void * 	(* 		TarOpenFile_f)	(const char * filename, int filesize, void * tarRef),
		int		(* 		TarWriteFile_f)	(const char * data, int len, void * tarRef, void * fileRef),
		void	(* 		TarCloseFile_f)	(void * tarRef, void * fileRef),
		void *			tarRef);

#endif
