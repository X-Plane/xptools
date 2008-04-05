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

void	EndianFlipShort(short * ioShort)
{
	// Not necessary on WINTEL machines.
}

void	EndianFlipLong(long * ioLong)
{
	// Not necessary on WINTEL machines.
}

const char * GetApplicationPath(char * pathBuf, int sz)
{
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
	return 0;
}

void	DoUserAlert(const char * inMsg)
{
	fprintf(stderr,"%s\n",inMsg);
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
	return close_Discard;
}
