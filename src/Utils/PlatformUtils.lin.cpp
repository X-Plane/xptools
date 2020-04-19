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

#include "PlatformUtils.h"
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <cstring>
#include <string>
#include <linux/limits.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <Fl_Native_File_Chooser.H>

string GetApplicationPath()
{
	char pathBuf[PATH_MAX] = { 0 };
	memset(pathBuf, 0, PATH_MAX);
	if (readlink("/proc/self/exe", pathBuf, PATH_MAX) == -1)
		return 0;
	return string(pathBuf);
}

//common practice to get the cache folder:
//1. read env var 'XDG_CACHE_HOME'
//2. ~/.cache if exists
//3. fallback to default temp folder.
string GetCacheFolder()
{
	string path;
	const char * cpath  = getenv("XDG_CACHE_HOME");
	if(!cpath)
	{
		const char * hdir = getenv("HOME");
		passwd *pw = getpwuid(getuid());
		if (pw)
		{
			const char * pdir = pw->pw_dir;
			if( strcmp(pdir,hdir) != 0)
				return "";
		}
		path = hdir;
		path += "/.cache";
	}
	else
	{
		path = cpath;
	}

	struct stat ss;
	if (stat(path.c_str(), &ss) == 0)
	{
		return path;
	}

	return "";
}

string GetTempFilesFolder()
{
	const char * tpath  = getenv("TMPDIR");
	if(!tpath)
		tpath = "/tmp";
	char temp_path[PATH_MAX] = { 0 };
	int n = snprintf(temp_path,PATH_MAX,"%s/xptools-%d",tpath,getuid());
	if( n < 0 || n >= PATH_MAX)
		return "";

	struct stat ss;
	if (stat(temp_path,&ss) < 0)
		if(mkdir(temp_path,0700) !=0)
			return "";

	return string(temp_path);
}

int		GetFilePathFromUser(
					int					inType,
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					char * 				outFileName,
					int					inBufSize)
{
    int ret = 0;

    Fl_Native_File_Chooser * mFileDialog = new Fl_Native_File_Chooser();

    mFileDialog->title(inPrompt);

	switch(inType)
	{
		case getFile_Open:       mFileDialog->type(Fl_Native_File_Chooser::BROWSE_FILE);      break;
		case getFile_Save:       mFileDialog->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE); break;
		case getFile_PickFolder: mFileDialog->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY); break;

        default: { if(mFileDialog) delete mFileDialog;  return ret;}
    }


    if(mFileDialog->show() == 0)
    {
        ::strncpy(outFileName,mFileDialog->filename(),inBufSize);
       ret = 1;
    }


    if(mFileDialog) delete mFileDialog;
    return ret ;
}

char *	GetMultiFilePathFromUser(
					const char * 		inPrompt,
					const char *		inAction,
					int					inID)
{
    char * ret = NULL;
    Fl_Native_File_Chooser * mFileDialog = new Fl_Native_File_Chooser();

    mFileDialog->title(inPrompt);
    mFileDialog->type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);

    if(mFileDialog->show() == 0)
    {
        int file_cnt(mFileDialog->count());
        if(file_cnt == 0 ){ if(mFileDialog) delete mFileDialog; return ret;}

        vector<string> outFiles;
        for (int i=0; i < file_cnt; ++i )
        {
            if(strlen(mFileDialog->filename(i)) > 0)
                outFiles.push_back(mFileDialog->filename(i));
        }

        int buf_size = 1;
        for(int i = 0; i < outFiles.size(); ++i)
        {
            buf_size += (outFiles[i].size() + 1);
        }

        ret = (char *) malloc(buf_size);
        char * p = ret;

        for(int i = 0; i < outFiles.size(); ++i)
        {
            strcpy(p, outFiles[i].c_str());
            p += (outFiles[i].size() + 1);
        }
        *p = 0;
    }

    if(mFileDialog) delete mFileDialog;
    return ret ;
}

void DoUserAlert(const char * inMsg)
{
	fl_message_hotspot(false);
	fl_alert(inMsg);
}

void ShowProgressMessage(const char * inMsg, float * inProgress)
{
	if(inProgress)	fprintf(stderr,"%s: %f\n",inMsg,100.0f * *inProgress);
	else			fprintf(stderr,"%s\n",inMsg);
}

int ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	fl_message_hotspot(false);

	int result = 0;
	switch (fl_choice(inMsg,proceedBtn,cancelBtn,0))
	{
	  case 0: result = 1; break; // proceedBtn
	  case 1: result = 0; break; // cancelBtn (default)
	}

	if(Fl::event_key(FL_Escape)) result = 0;

	return result;
}


int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	fl_message_hotspot(false);

	int result = close_Cancel;
	switch (fl_choice(inMessage2,"Discard","Cancel","Save"))
	{
	  case 0: result = close_Discard; break;
	  case 1: result = close_Cancel;  break; //(default)
	  case 2: result = close_Save;    break;
	}

	if(Fl::event_key(FL_Escape)) result = close_Cancel;

	return result;
}

