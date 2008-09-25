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
#include <string>
using namespace std;
#include "BuildInstaller.h"
#include "InstallerProcs.h"
#include "XResources.h"
#include "PlatformUtils.h"
#include <stdio.h>
#include "ErrMsg.h"

static FILE * logFile = NULL;
static bool	force_bail = false;

struct	InstallerStat {
	string	master_base;
	string	cur_old;
	string	cur_new;
	void *	installer;
};

char	buf[4096];

static int	gFileCount;

void	InstallerError(
				const char * 	inMessage,
				int 			errNum,
				const char *	descrip,
				const char * 	inFile,
				const char *	inModule,
				int				inLine,
				void * 			)
{
	sprintf(buf,"Error: %s (OS Err = %d %s, file = %s)", inMessage, errNum, descrip, inFile);
	if (logFile) fprintf(logFile, "%s (%s %d.)\n", buf, inModule, inLine);
	DoUserAlert(buf);
}

bool IterateDirsNew(const char * fileName, bool isDir, void * ref)
{
	float inprog = -1;
	if (force_bail) return true;
	InstallerStat * stat = (InstallerStat*) ref;
	sprintf(buf, "Processing directory %s", stat->cur_new.c_str());
	if (isDir)
	{
		InstallerStat sub;
		sub.master_base = stat->master_base;
		sub.cur_old = stat->cur_old + fileName + DIR_STR;
		sub.cur_new = stat->cur_new + fileName + DIR_STR;
		sub.installer = stat->installer;
		ShowProgressMessage(buf, &inprog);
		MF_IterateDirectory(sub.cur_new.c_str(), IterateDirsNew, &sub);
	} else {
		if (strlen(fileName) > 31)
		{
			char	buf[256];
			sprintf(buf, "WARNING: file %s has a name length of more than 31 characters.", fileName);
			DoUserAlert(buf);
		}
		if (fileName[0] != '.') {
			string	f1 = stat->cur_old + fileName;
			string	f2 = stat->cur_new + fileName;
			if (!FilesAreSame(f1.c_str(), f2.c_str()))
			{
				if (logFile) fprintf(logFile, "Adding %s to the installer.\n", f2.c_str());
				InstallerChunk	chunk;
				if (!BuildChunk(f1.c_str(), f2.c_str(), chunk, stat->master_base.c_str()))
					force_bail = true;
				if (!XRES_AddResource(stat->installer, chunk.mem, chunk.len))
					force_bail = true;
				else
					gFileCount++;
			}
		}
	}
	return force_bail;
}

bool IterateDirsOld(const char * fileName, bool isDir, void * ref)
{
	float inprog = -1;
	if (force_bail) return true;
	InstallerStat * stat = (InstallerStat*) ref;
	if (isDir)
	{
		InstallerStat sub;
		sub.master_base = stat->master_base;
		sub.cur_old = stat->cur_old + fileName + DIR_STR;
		sub.cur_new = stat->cur_new + fileName + DIR_STR;
		sub.installer = stat->installer;
		sprintf(buf, "Processing old directory %s", sub.cur_old.c_str());
		ShowProgressMessage(buf, &inprog);
		MF_IterateDirectory(sub.cur_old.c_str(), IterateDirsOld, &sub);
	} else {
		if (strlen(fileName) > 31)
		{
			char	buf[256];
			sprintf(buf, "WARNING: file %s has a name length of more than 31 characters.", fileName);
			DoUserAlert(buf);
		}
		if (fileName[0] != '.') {
			string	f1 = stat->cur_old + fileName;
			string	f2 = stat->cur_new + fileName;
			int l1 = GetFileBlockSizeIfExists(f1.c_str());
			int l2 = GetFileBlockSizeIfExists(f2.c_str());
			if (l1 != -1 && l2 == -1)
			{
				if (logFile) fprintf(logFile, "Adding removal of %s to the installer.\n", f1.c_str());
				InstallerChunk	chunk;
				if (!BuildChunk(f1.c_str(), f2.c_str(), chunk, stat->master_base.c_str()))
					force_bail = true;
				if (!XRES_AddResource(stat->installer, chunk.mem, chunk.len))
					force_bail = true;
				else
					++gFileCount;
			}
		}
	}
	return force_bail;
}

int	BuildInstaller(const char * inOldBase,
					const char * inNewBase,
					const char * inInstallerExe)
{
	ShowProgressMessage("Building Installer...", NULL);
	gFileCount = 0;
	InstallErrFunc(InstallerError, NULL);

	logFile = fopen("installer_build_log.txt", "a");
	if (logFile) fprintf(logFile, "Building installer:\n");
	if (logFile) fprintf(logFile, "Old base: %s\n", inOldBase);
	if (logFile) fprintf(logFile, "New base: %s\n", inNewBase);
	if (logFile) fprintf(logFile, "InstallerExe: %s\n", inInstallerExe);
	InstallerStat stat;
	stat.master_base = inNewBase;
	stat.cur_old = inOldBase;
	stat.cur_new = inNewBase;
	stat.installer = XRES_BeginSettingResources(inInstallerExe);

	force_bail = false;
	MF_IterateDirectory(inNewBase, IterateDirsNew, &stat);
	if (!force_bail)
	MF_IterateDirectory(inOldBase, IterateDirsOld, &stat);

	XRES_EndSettingResources(stat.installer);
	if (logFile) fclose(logFile);

	InstallErrFunc(NULL, NULL);
	if (force_bail)
		DoUserAlert("Warning: the installer aborted early.");
	return gFileCount;
}
