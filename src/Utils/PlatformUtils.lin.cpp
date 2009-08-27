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
#include <cstring>
#include <string>

const char* GetApplicationPath(char* pathBuf, int sz)
{
//TODO: readlink() on /proc/self/exe
	return ".";
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
			QString fileName = QFileDialog::getOpenFileName(0,
			inPrompt, "/");
			if (!fileName.length())
				return 0;
			else {
				::strncpy(outFileName, fileName.toAscii().constData(), inBufSize);
				return 1;
			}
		}
		case getFile_Save:
		{
			QString fileName = QFileDialog::getSaveFileName(0,
			inPrompt, "/");
			if (!fileName.length())
				return 0;
			else {
				::strncpy(outFileName, fileName.toAscii().constData(), inBufSize);
				return 1;
			}
		}
		case getFile_PickFolder:
		{
			QString dir = QFileDialog::getExistingDirectory
			(0, inPrompt, "/", QFileDialog::ShowDirsOnly);
			if (!dir.length())
				return 0;
			else {
				::strncpy(outFileName, dir.toAscii().constData(), inBufSize);
				return 1;
			}
		}
		default:
			return 0;
	}
}

void	DoUserAlert(const char * inMsg)
{
	QMessageBox::warning(0, "", inMsg);
}

void	ShowProgressMessage(const char * inMsg, float * inProgress)
{
	if(inProgress)	fprintf(stderr,"%s: %f\n",inMsg,100.0f * *inProgress);
	else			fprintf(stderr,"%s\n",inMsg);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	fprintf(stderr,"%s (%s/%s)\n", inMsg, proceedBtn, cancelBtn);
	return 0;
}

int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	int res = QMessageBox::question(0, inMessage1, inMessage2,
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
