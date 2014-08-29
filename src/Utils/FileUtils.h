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

#ifndef FILEUTILS_H
#define FILEUTILS_H

class	FILE_case_correct_path {
public: 
	FILE_case_correct_path(const char * in_path);
	~FILE_case_correct_path();
	
	operator const char * (void) const;
private:
	char * path;
	FILE_case_correct_path(const FILE_case_correct_path& rhs);
	FILE_case_correct_path();
	FILE_case_correct_path& operator=(const FILE_case_correct_path& rhs);
};

enum date_cmpr_result_t
{
	dcr_firstIsNew = -1,
	dcr_secondIsNew = 1,
	dcr_same = 0,
	dcr_error = -2
};

bool FILE_exists(const char * path);									//Returns true if the file exists, returns false if it doesn't

// WARNING: these do not take trailing / for directories!
int FILE_delete_file(const char * nuke_path, int is_dir);

// Path should end in a /
int FILE_delete_dir_recursive(const string& path);

int FILE_rename_file(const char * old_name, const char * new_name);

// Create in_dir in its parent directory
int FILE_make_dir(const char * in_dir);


// Recursively create all dirs needed for in_dir to exist - handles trailing / ok.
int FILE_make_dir_exist(const char * in_dir);

int FILE_delete_dir_recursive(const string& path);

// Get a directory listing.  Returns number of files found, or -1 on error.
int FILE_get_directory(
		const string&		path,
		vector<string> *	out_files,	// Both arrays are optional.
		vector<string> *	out_dirs);

int FILE_compress_dir(const string& src_path, const string& dst_path, const string& prefix);


/* Pass in a file path for the first and second file
* Return 1: The second file is more updated that the first
* Return 0: Both files have the same timestamp
* Return -1: The first file is more current than the second or the second does not exist
* Return -2: There's been an error
*/
date_cmpr_result_t FILE_date_cmpr(const char * first, const char * second);
int FILE_case_correct(char * buf);

#endif
