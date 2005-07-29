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
#ifndef XFILETWIDDLE_H
#define XFILETWIDDLE_H


/*******************************************************
 * MD5 SIGNING
 *******************************************************/

struct	MD5_Sig {
	unsigned char digest[16];	
	bool	operator==(const MD5_Sig& rhs) const;
};

void	MD5_Block(const char * inMem, int inSize, MD5_Sig& outSig);

/*******************************************************
 * X-PLATFORM FILE I/O ROUTINES
 *******************************************************/

/*
 * MakeDirExist
 *
 * Pass in a directory to have it be created.
 *
 */
int		MakeDirExist(const char * inPath);

/* 
 * NukeFile
 * 
 * Deletes the file at the specified path.  Returns 1 for
 * success, or reports an error for a failure and returns 0.
 * Path is a full path and must be to a file.
 *
 */ 
int		NukeFile(const char * inPath);

/*
 * FileToBlock
 *
 * Creates a block of memory that can be use to recreate the file.
 * Returns 1 if successful, or 0 if not.  The poitner in *outPtr is
 * set to new memory with the block, and outSize is set to the block's
 * size.  If it is successful, the memory in *outPtr must be freed
 * using free().  inPath must be a full path to af ile.
 *
 */
int		FileToBlock(const char * inPath, char ** outPtr, int * outSize);

/*
 * BlockToFile
 *
 * Given a file path to a file and a block, creates or updates the file
 * and returns 1 for success, or 0 for failure.  The file must be 
 * created if needed, and all intermediate directories must be made too.
 *
 */
int		BlockToFile(const char * inPath, char * inPtr);

/*
 * GetFileBlockSizeIfExists
 *
 * Returns the size of the block of mem that will be needed to
 * hold the file, or -1 if the file does not exist.
 *
 */
int		GetFileBlockSizeIfExists(const char * inPath);

/*******************************************************
 * COMPRESSION AND DECOMPRESSION IN MEMORY
 *******************************************************/

// For the zipped size, pass in the unzipped size + .1% + 12 bytes
// the actual zipped size is returned.
// Neither of these routines allocate memory.
// Return 1 for sucess, 0 for fail
int		ZipBlock(
			const char * 		inRaw, 
			int 				inSize,
			char * 				ioZipped, 
			int * 				ioZippedSize);

// You must know the actual size the data will inflate to in advance.
// Return 1 for sucess, 0 for fail
int		UnzipBlock(
			const char * 		inZipped, 
			int 				inZippedSize, 
			char * 				outRaw, 
			int 				inRawSize);

#endif
