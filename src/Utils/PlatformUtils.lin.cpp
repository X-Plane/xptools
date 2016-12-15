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
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include "PlatformUtils.h"
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <cstring>
#include <string>
#include <linux/limits.h>

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
	switch(inType)
	{
		case getFile_Open:
		{
			QString fileName = QFileDialog::getOpenFileName(0,QString::fromUtf8(inPrompt));
			if (!fileName.length())
				return 0;
			else {
				::strncpy(outFileName, fileName.toUtf8().constData(), inBufSize);
				return 1;
			}
		}
		case getFile_Save:
		{
			QString fileName = QFileDialog::getSaveFileName(0,QString::fromUtf8(inPrompt));
			if (!fileName.length())
				return 0;
			else {
				::strncpy(outFileName, fileName.toUtf8().constData(), inBufSize);
				return 1;
			}
		}
		case getFile_PickFolder:
		{
			QString dir = QFileDialog::getExistingDirectory(0, QString::fromUtf8(inPrompt),
			                                                "", QFileDialog::ShowDirsOnly);
			if (!dir.length())
				return 0;
			else {
				if(dir.endsWith ('/'))
						dir.truncate(dir.size()-1);
				::strncpy(outFileName, dir.toUtf8().constData(), inBufSize);
				return 1;
			}
		}
		default:
			return 0;
	}
}

char *	GetMultiFilePathFromUser(
					const char * 		inPrompt,
					const char *		inAction,
					int					inID)
{

	QStringList fileNames = QFileDialog::getOpenFileNames(0,QString::fromUtf8(inPrompt));

	vector<string> outFiles;
	if (fileNames.empty()) return NULL;
	for(int i=0; i < fileNames.size(); ++i)
	{
		if(!fileNames.at(i).isEmpty())
			outFiles.push_back(fileNames[i].toUtf8().constData());
	}

	if(outFiles.size() < 1) return NULL;

	int buf_size = 1;
	for(int i = 0; i < outFiles.size(); ++i)
		buf_size += (outFiles[i].size() + 1);

	char * ret = (char *) malloc(buf_size);
	char * p = ret;

	for(int i = 0; i < outFiles.size(); ++i)
	{
		strcpy(p, outFiles[i].c_str());
		p += (outFiles[i].size() + 1);
	}
	*p = 0;

	return ret;
}

void	DoUserAlert(const char * inMsg)
{
	QMessageBox::warning(0, "", QString::fromUtf8(inMsg));
}

void	ShowProgressMessage(const char * inMsg, float * inProgress)
{
	if(inProgress)	fprintf(stderr,"%s: %f\n",inMsg,100.0f * *inProgress);
	else			fprintf(stderr,"%s\n",inMsg);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	return (QMessageBox::question(0,"", QString::fromUtf8(inMsg), proceedBtn, cancelBtn) == 0 ) ;
}

int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	int res = QMessageBox::question(0, QString::fromUtf8(inMessage1), QString::fromUtf8(inMessage2),
	QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
	QMessageBox::Cancel);
	switch (res)
	{
		case QMessageBox::Save:
			return close_Save;
		case QMessageBox::Discard:
			return close_Discard;
		case QMessageBox::Cancel:
		default:
			return close_Cancel;
	}
}
