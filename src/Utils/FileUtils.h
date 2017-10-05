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

#include <sys/stat.h>

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

//Tests for file/folder case correctness on LIN, returns 1/0, or always 1 on IBM or APL
int FILE_case_correct(char * buf);

/* FILE API Overview
	Method Name                 |                    Purpose                    | Trailing Seperator? | Returns (Sucess, fail)
	exists                      | Does file exist?                              | N/A                 | True/false
	get_file_extension          | Gets the chars from the last dot to the end   | N/A                 | non-empty (".txt",".jpeg". No case change), empty string
	get_file_meta_data          | Get file info like creation time and date     | No                  | 0, -1
	get_dir_name                | Get directory part of filename                | N/A                 | non-empty, empty string
	get_file_name               | Get file name w/o directory, can use / or \   | N/A                 | non-empty, empty string
	get_file_name_wo_extensions | Get file name w/o directory or any extensions | N/A                 | non-empty, empty string
	delete_file                 | rm 1 file or folder                           | No                  | 0, last_error
	delete_dir_recursive        | rm folder and subcontents                     | Yes                 | 0, last_error
	read_file_to_string         | read a (non-binary) file to a string          | N/A                 | 0, last_error
	rename_file                 | rename 1 file                                 | N/A                 | 0, last_error
	compress_dir                | zip compress folder, save zip to disk         | No                  | 0, not zero (see zlib)
	get_directory               | get dir's content's paths*                    | No                  | num files found?**, -1 or last_error
	get_directory_recursive     | get dir and sub dir's files and folders       | No                  | num files found?**, -1 or last_error 
	make_dir                    | make directory, assumes parent folders exist  | No                  | 0, last_error
	make_dir_exist              | make directory, parent folders created on fly | No                  | 0, last_error
	date_cmpr                   | compares which of two files is newer          | N/A                 | (1,0,-1), -2

	*get_directory can do 1 to 4 things at the same time. It can implicitly tell you if the directory exists, how many files are contained in it,
	and, optionally, the relative paths of file or folders non-rescursively

	get_directory's return value is possibly bugged or is not being interpreted correctly. Use out_files->size() for a better count. -2016/05/16

	get_directory_recursive's vectors of strings contain fully qualified names, unlike get_directory.
/*/

//Returns true if the file exists, returns false if it doesn't
bool FILE_exists(const char * path);

// returns file extension, NOT including the dot, always as lower case
string FILE_get_file_extension(const string& path);

int FILE_get_file_meta_data(const string& path, struct stat& meta_data);

string FILE_get_file_name(const string& path);

// returns directory name, i.e. path to file w/o filename, including the final directory separator
string FILE_get_dir_name(const string& path);

string FILE_get_file_name_wo_extensions(const string& path);

// WARNING: these do not take trailing / for directories!
// Returns 0 for success, else last_error
int FILE_delete_file(const char * nuke_path, bool is_dir);

// Path should end in a /
// Returns 0 for success, else -1 or last_error
int FILE_delete_dir_recursive(const string& path);

//Reads the contents of a non-binary file into a string, does not close the file handle for you
int FILE_read_file_to_string(FILE* file, string& content);
int FILE_read_file_to_string(const string& path, string& content);

// Returns 0 for success, else last_error
int FILE_rename_file(const char * old_name, const char * new_name);

// Create in_dir in its parent directory
// Returns 0 for success, else last_error
int FILE_make_dir(const char * in_dir);

// Recursively create all dirs needed for in_dir to exist - handles trailing / ok.
int FILE_make_dir_exist(const char * in_dir);

// Get a directory listing.  Returns number of files found, or -1 on error. Both arrays are optional.
int FILE_get_directory(const string& path, vector<string> * out_files, vector<string> * out_dirs);

// Gets a complete listing of every file and every folder under a given directory
int FILE_get_directory_recursive(const string& path, vector<string>& out_files, vector<string>& out_dirs);

int FILE_compress_dir(const string& src_path, const string& dst_path, const string& prefix);

enum date_cmpr_result_t
{
	dcr_firstIsNew = -1,
	dcr_secondIsNew = 1,
	dcr_same = 0,
	dcr_error = -2
};

/* Pass in a file path for the first and second file
* Return 1: The second file is more updated that the first
* Return 0: Both files have the same timestamp
* Return -1: The first file is more current than the second or the second does not exist
* Return -2: There's been an error
*/
date_cmpr_result_t FILE_date_cmpr(const char * first, const char * second);
#endif
