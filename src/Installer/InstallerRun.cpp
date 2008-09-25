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
#include "InstallerRun.h"
#include "XResources.h"
#include "InstallerProcs.h"
#include "PlatformUtils.h"
#include <string>
#include "ErrMsg.h"
using std::string;
#include <stdio.h>

static FILE * logFile = NULL;

static char	buf[4096];

void	InstallerError(
				const char * 	inMessage,
				int 			errNum,
				const char *	descrip,
				const char * 	inFile,
				const char *	module,
				int				line,
				void * 			)
{
	sprintf(buf,"Error: %s (OS Err = %d %s, file = %s)", inMessage, errNum, descrip, inFile);
	if (logFile) fprintf(logFile, "%s (%s %d.)\n", buf, module, line);
	DoUserAlert(buf);
}



void	RunInstaller(const char * inBasePath)
{
	ShowProgressMessage("Updating...", NULL);

	ErrFunc_f old = InstallErrFunc(InstallerError, NULL);
	logFile = fopen("installer_log.txt", "a");
	if (logFile) fprintf(logFile, "Installing to path %s.\n", inBasePath);
	InstallerChunk	chunk;
	int id = 0;
	char * d;
	int sz;
	int	max = XRES_CountResources();
	for (id = 0; id < max; ++id)
	{
		if (!XRES_GetResourceData(&d, &sz))
			break;
		chunk.Assign(d, sz);

		string	full_path(inBasePath);
		full_path += chunk.mem;
		int stat = FileStatus(full_path.c_str(), chunk);
		const char * msg = "Unknown result";
		bool		runit = false;
		bool		askit = false;
		switch(stat) {
		case file_CreateOK:				runit = true;	msg = "Creating file %s.\n";							break;
		case file_CreateUnexpected:		askit = true;	msg = "Unexpected file in place of new file %s.\n";		break;
		case file_CreatedAlready:						msg = "File %s already created.\n";						break;

		case file_MatchesOK:			runit = true;	msg = "Will update file %s.\n";							break;
		case file_MatchesUnexpected:	askit = true;	msg = "Modified file %s needs updating.\n";				break;
		case file_MatchesMissing:		runit = true;	msg = "File %s was deleted, will replace with new.\n";	break;
		case file_MatchesAlready:						msg = "File %s already updated.\n";						break;

		case file_DeleteOK:				runit = true;	msg = "Will delete file %s.\n";							break;
		case file_DeleteUnexpected:		askit = true;	msg = "Modified file %s needs deleting.\n";				break;
		case file_DeleteAlready:						msg = "File %s already deleted.\n";						break;
		}

		if (logFile)	fprintf(logFile, msg, full_path.c_str());
		if (askit)
		{
			char	buf[2048];
			if (stat == file_CreateUnexpected || stat == file_MatchesUnexpected)
			{
				sprintf(buf,"The file %s needs to be updated but has already been modified.  Do you want to overwrite your modified file?", full_path.c_str());
				if (ConfirmMessage(buf, "Overwrite", "Skip"))
					runit = true;
			}
			if (stat == file_DeleteUnexpected)
			{
				sprintf(buf,"The file %s is no longer used by X-Plane.  Do you want to delete it?", full_path.c_str());
				if (!ConfirmMessage(buf, "Leave It", "Delete"))
					runit = true;
			}
			if (logFile && runit)	fprintf(logFile, "User chose installer.\n");
			if (logFile && !runit)	fprintf(logFile, "User chose local file.\n");
		}
		if (runit)
		{
			sprintf(buf,"%s", chunk.mem);
			float v = ((float) id / (float) max);
			ShowProgressMessage(buf, &v);
			InstallChunk(chunk, inBasePath);
		}
	}
	if (logFile) fclose(logFile);
	InstallErrFunc(old, NULL);
}
