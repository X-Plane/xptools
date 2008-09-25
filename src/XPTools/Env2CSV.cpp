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
#include <vector>
#include <string>
#include "EnvWrite.h"
#include "XGrinderApp.h"
#include "EnvParser.h"
#include "PlatformUtils.h"
#include "XUtils.h"
#include "Persistence.h"
#include "EnvPrint.h"
#include "DSFPrint.h"
#include "EnvScan.h"
#include "Env2DSF.h"

void	XGrindFile(const char * inFileName);

void	XGrindFiles(const vector<string>& files)
{
	for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		XGrindFile(i->c_str());
	}
}

void	XGrindInit(string& t)
{
	t = "Env2CSV";
	XGrinder_ShowMessage("Drag a file into this window to convert it.");
}

int	XGrinderMenuPick(xmenu menu, int item)
{
	return 0;
}


void	XGrindFile(const char * inFileName)
{
		int		err;
		string	path(inFileName);
		string	fname(inFileName);
		string	dir;
		string::size_type sep = path.rfind(DIR_CHAR);

	if (sep != path.npos)
	{
		fname = path.substr(sep+1);
		dir = path.substr(0, sep+1);
	}

	if (HasExtNoCase(fname, ".csv"))
	{
		XGrinder_ShowMessage("Converting %s...",fname.c_str());
		if (!ScanEnv(inFileName, fname.c_str()))
		{
			XGrinder_ShowMessage("Could not open the file %s.", inFileName);
		} else {
			string	newPath = path.substr(0, path.length() - 4) + "_new.env";
			string	newName = fname.substr(0, fname.length() - 4) + "_new.env";

			if (EnvWrite(newPath.c_str()))
			{
				XGrinder_ShowMessage("Error writing %s.",newName.c_str());
			} else {
				XGrinder_ShowMessage("Wrote%s.",newName.c_str());
			}
		}
	}

	if (HasExtNoCase(fname, ".dsf"))
	{
		string	newName = string(inFileName) + ".txt";
//		PrintDSF(inFileName, newName.c_str());
	}

	if (HasExtNoCase(fname, ".env"))
	{
		XGrinder_ShowMessage("Converting %s...",fname.c_str());

		ClearEnvData();

		if ((err = ReadEnvFile(inFileName)) == 0)
		{
#if 0
			string	newPath = path.substr(0, path.length() - 4) + ".dsf";
			string	newName = fname.substr(0, fname.length() - 4) + ".dsf";

			XGrinder_ShowMessage("Writing %s...",newName.c_str());

			Env2DSF(newPath.c_str());
#endif

#if 1
			string	newPath = path.substr(0, path.length() - 4) + ".csv";
			string	newName = fname.substr(0, fname.length() - 4) + ".csv";

			XGrinder_ShowMessage("Writing %s...",newName.c_str());

			if (PrintENVData(newPath.c_str()))
			{
				XGrinder_ShowMessage("Wrote %s.",newName.c_str());
			} else {
				XGrinder_ShowMessage("Could not write csv for %s.",newName.c_str());
			}
#endif
		} else {
			XGrinder_ShowMessage("Error %d reading %s.",err, fname.c_str());
		}
	}
}