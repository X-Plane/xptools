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
#include "InstallerScript.h"
#include <stdio.h>
#include "PlatformUtils.h"
#include <string.h>
#include "XFileTwiddle.h"
#include <errno.h>
#include <unzip.h>
#include "InstallerRun.h"
#include "ErrMsg.h"
#include <stdlib.h>
/************************************************************************************
 * INSTALLER SCRIPTING LANGUAGE
 ************************************************************************************

 When we add a text file called 'installer.txt' to the same dir as the app, the updater
 becomes an installer - mostly this means a change in UI.  But the script contains
 actions to be executed.  Actions supported:

 CONDITION Question?
 NOCONDITION

 UPDATE
 UPDATE
 run the updater part of the installer.

 COPY <source path relative to installer> <dest path relative to install>
 COPY repository/big_file.txt	Custom scenery folder/My package/a_file.txt
 Copies one file from the volume of the installer to the destionation dir.

 UNZIP <source path relative to installer> <dest path relative to install>
 COPY repository/big_file.txt	Custom scenery folder/
 Unzips one file from the volume of the installer to the destionation dir.


-NOTE: the command line tool 'zip' makes clean x-platform zip archives with useful
unix paths as follows:
	zip -r archive.zip dir_name
e.g.
	zip -r archive.zip Resources/sounds




 ************************************************************************************/
static char 	msgBuf[1024];

static FILE * scriptLog = NULL;

 void	ScriptError(
				const char * 	inMessage,
				int 			errNum,
				const char *	descrip,
				const char * 	inFile,
				const char *	inModule,
				int				inLine,
				void * 			inRef)
{
	static char	buf[1024];
	sprintf(buf,"Error: %s (OS Err = %d %s, file = %s)", inMessage, errNum, descrip, inFile);
	if (scriptLog) fprintf(scriptLog, "%s (%s %d.)\n", buf, inFile, inModule);
	DoUserAlert(buf);
}




// Fill buf with line.  Strip \n and \r.  REturns
// empty string for empty lines.   Returns NULL for io err or eof.
// max should be size of buf not including room for null.
// buf will always be null terminated.
char *	get_line(FILE * fi, char * buf, int max)
{
	int n = 0;
	while (1)
	{
		if (n >= max || feof(fi))  break;
		int c = fgetc(fi);
		if (c == EOF) break;
		if (c == '\n' || c == '\r')
		{
			buf[n] = 0;
			return buf;
		}
		buf[n] = c;
		++n;
	}
	buf[n] = 0;
	return (n > 0) ? buf : NULL;
}


char *	next_token(char ** buf)
{
	char * p = *buf;

	// First scan past any spaces.
	while (*p == ' ' || *p == '\t')	++p;

	// If we hit the end, there was nothing to get but whitespace, bail.
	if (*p == 0)
	{
		*buf = p;
		return NULL;
	}

	// We are now at the beginning of the token.  Remember it and
	// scan forward.
	char * r = p;
	while (*p != 0 && *p != ' ' && *p != '\t') ++p;

	// If we hit NULL, the token is already a natural termination -
	// update and return.
	if (*p == 0)
	{
		*buf = p;
		return r;
	}

	// More whitespace after the token - keep scanning.
	*p = 0;
	++p;
	while (*p == ' ' || *p == '\t')	++p;
	*buf = p;
	return r;
}

void	strip_to_delim(char * buf, char delim)
{
	char * p = buf + strlen(buf);
	while (p > buf && *(p-1) != delim)
		--p;
	if (*(p-1) == delim)
		*p = 0;
}

void	normalize_dir_chars(char * buf)
{
	char * p = buf;
	while (*p)
	{
		if (*p == '/')
			*p = DIR_CHAR;
		++p;
	}
}


void RunScript(FILE * script, const char * path)
{
	ErrFunc_f old = InstallErrFunc(ScriptError, NULL);
	char * mem;
	int len, ok;
	char	strbuf[1024];
	strcpy(strbuf, path);
	strcat(strbuf, DIR_STR);
	MakeDirExist(strbuf);
	strcat(strbuf, "Installer Log.txt");
	scriptLog = fopen(strbuf, "w");
	if (scriptLog) fprintf(scriptLog, "Installing X-Plane to %s.\n", path);
	char	from_buf[1024], to_buf[1024], partial[1024];
	const char *	app_path = GetApplicationPath();
	if (app_path == NULL)
	{
		return;
	}
	bool	condition = true;
	while (get_line(script, strbuf, sizeof(strbuf)-1))
	{
		char * t;
		char * p = strbuf;
		t = next_token(&p);
		if (t == NULL) continue;
		if (!strcmp(t, "NOCONDITION"))
		{
			condition = true;
		}
		if (!strcmp(t, "ELSE"))
		{
			condition = !condition;
		}
		if (!strcmp(t, "MESSAGE"))
		{
			if (scriptLog) fprintf(scriptLog, "Note: %s\n", p);
			DoUserAlert(p);
		}
		if (!strcmp(t, "CONDITION"))
		{
			if (scriptLog) fprintf(scriptLog, "Checking: %s\n", p);
			condition = ConfirmMessage(p, "Yes", "No");
			if (scriptLog) fprintf(scriptLog, "Answer: %s\n", condition ? "yes" : "no");
		}
		if (!strcmp(t, "UPDATE"))
		{
			if (scriptLog) fprintf(scriptLog, "Running update.\n");
			if (condition)
				RunInstaller(path);
		}
		if (!strcmp(t, "COPY"))
		{
			t = next_token(&p);
			float indi = -1.0;
			sprintf(msgBuf, "Installing %s...", t);
			ShowProgressMessage(msgBuf, &indi);
			strcpy(from_buf, app_path);
			strip_to_delim(from_buf,DIR_CHAR);
			strcat(from_buf, t);
			strcpy(to_buf, path);
			strcat(to_buf, DIR_STR);
			strcat(to_buf, p);
			normalize_dir_chars(to_buf);
			normalize_dir_chars(from_buf);
			if (condition)
			{
				if (scriptLog) fprintf(scriptLog, "Copying %s to %s\n", from_buf, to_buf);
				ok = FileToBlock(from_buf, &mem, &len);
				if (!ok) return;
				ok = BlockToFile(to_buf, mem);
				if (!ok) return;
				free(mem);
			} else {
				if (scriptLog) fprintf(scriptLog, "Not copying %s to %s\n", from_buf, to_buf);
			}
		}
		if (!strcmp(t, "UNZIP"))
		{
			t = next_token(&p);
			sprintf(msgBuf, "Installing %s...", t);
			strcpy(from_buf, app_path);
			strip_to_delim(from_buf,DIR_CHAR);
			strcat(from_buf, t);
			if (!strcmp(p, "/"))
			{
				strcpy(partial, path);
				strcat(partial, DIR_STR);
			} else {
				strcpy(partial, path);
				strcat(partial, DIR_STR);
				strcat(partial, p);
			}
			normalize_dir_chars(partial);
			normalize_dir_chars(from_buf);

			if (condition)
			{
				if (scriptLog) fprintf(scriptLog, "Unzipping %s to %s\n", from_buf, partial);
				unzFile unz = unzOpen(from_buf);
				if (unz == NULL)
				{
					ReportError("Unable to open zip file.", EUNKNOWN, from_buf);
					return;
				}
				unz_global_info		global;
				unzGetGlobalInfo(unz, &global);
				unzGoToFirstFile(unz);
				int counter = 0;
				do {

					char				zip_path[1024];
					unz_file_info		info;
					unzGetCurrentFileInfo(unz, &info, zip_path, sizeof(zip_path),
						NULL, 0, NULL, 0);

					sprintf(msgBuf, "Installing %s...", zip_path);
					float prog = (global.number_entry > 0) ? ((float) counter / (float) global.number_entry) : -1.0;
					ShowProgressMessage(msgBuf, &prog);

					++counter;

					strcpy(to_buf, partial);
					strcat(to_buf, zip_path);
					normalize_dir_chars(to_buf);
					strip_to_delim(to_buf, DIR_CHAR);
					MakeDirExist(to_buf);

					if (info.uncompressed_size == 0)
						continue;

					char * mem = (char *) malloc(info.uncompressed_size);
					if (!mem) { ReportError("Out of memory", ENOMEM, NULL); return; }
					unzOpenCurrentFile(unz);
					int result = unzReadCurrentFile(unz,mem, info.uncompressed_size);
					if (result != info.uncompressed_size)
					{	ReportError("Could not read installer archive.", EUNKNOWN, zip_path); }
					unzCloseCurrentFile(unz);

					strcpy(to_buf, partial);
					strcat(to_buf, zip_path);
					normalize_dir_chars(to_buf);

					FILE * fi = fopen(to_buf, "wb");
					if (fi == NULL)
					{
						ReportError("Could not create file", errno, to_buf);
						return;
					}
					result = fwrite(mem, 1, info.uncompressed_size, fi);
					if (result != info.uncompressed_size)
					{ ReportError("Could not read installer archive.", errno, to_buf); }
					fclose(fi);
					free(mem);
				} while(unzGoToNextFile(unz) == UNZ_OK);
				unzClose(unz);
			} else {
				if (scriptLog) fprintf(scriptLog, "Not unzipping %s to %s\n", from_buf, partial);
			}
		}
	}
	InstallErrFunc(old, NULL);
	if (scriptLog) fprintf(scriptLog, "Installer completed successfully.\n");
	if (scriptLog) fclose(scriptLog);
	DoUserAlert("Installation was successful!");
}
