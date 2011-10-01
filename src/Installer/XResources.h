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
#ifndef XRESOURCES_H
#define XRESOURCES_H

#error this file is deprecated.

/*******************************************************
 * RESOURCE READING FROM THE APP ITSELF
 *******************************************************/

/*
 * Count the number of resources in our file.
 */

int		XRES_CountResources();

/*
 * Get the next resource.  Data must be freed with free().
 * Returns 1 for success, 0 for failure.  (If it fails,
 * memory is not allocated.
 *
 */
int		XRES_GetResourceData(char ** outPtr, int * outSize);


/*******************************************************
 * RESOURCE WRITING TO A FILE
 *******************************************************/

/*
 * Open a file to rewrite resources (nukes existing ones) -
 * returns a a void* ref to the open file, or NULL for a failure.
 *
 */
void *	XRES_BeginSettingResources(const char * inFilePath);

/*
 * Adds a resource of a fixed size to the file.  Returns 1 for
 * success, 0 for failure.
 *
 */
int		XRES_AddResource(void * inFile, char * inPtr, int inSize);

/*
 * Finish the file up and close it.  1 for success, 0 for failure.
 *
 */
int		XRES_EndSettingResources(void * inFile);

#endif
