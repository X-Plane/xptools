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

#include "../XPTools/version.h"
#include <stdio.h>
#include "AssertUtils.h"

#if IBM
#include <stdlib.h>
#endif

FILE * err_fi = stdout;

void AssertShellBail(const char * condition, const char * file, int line)
{
	fprintf(err_fi,"ERROR: %s\n", condition);
	fprintf(err_fi,"(%s, %d.)\n", file, line);
	exit(1);
}

bool DSF2Text(char ** inDSF, int n, const char * inFileName);
bool Text2DSF(const char * inFileName, const char * inDSF);

int main(int argc, char * argv[])
{
	InstallDebugAssertHandler(AssertShellBail);
	InstallAssertHandler(AssertShellBail);

	if (argc < 2) goto help;

	if(!strcmp(argv[1],"--auto_config"))
	{
		printf("CMD .txt .dsf \"%s\" -text2dsf \"INFILE\" \"OUTFILE\"\n", argv[0]);
		printf("CMD .dsf .txt \"%s\" -dsf2text \"INFILE\" \"OUTFILE\"\n", argv[0]);
		return 0;
	}

	for (int n = 1; n < argc; ++n)
	{
		if (!strcmp(argv[n], "-dsf2text") ||
			!strcmp(argv[n], "--dsf2text"))
		{
			++n;
			if (n >= argc) goto help;
			
			const char * f2 = argv[argc-1];
			if (strcmp(f2,"-")==0)			// If we are directing the DSF text stream to stdout
				err_fi=stderr;				// then put err msgs to stderr.

			fprintf(err_fi,"Converting %s from DSF to text as %s\n", argv[n], f2);
			if (DSF2Text(argv+n, argc - n - 1, f2))
				fprintf(err_fi,"Converted %s to %s\n",argv[n], f2);
			else
				{ fprintf(err_fi,"ERROR: Error convertiong %s to %s\n", argv[n], f2); exit(1); }
			n = argc - 1;
		}
		else if(!strcmp(argv[n], "-text2dsf") ||
				!strcmp(argv[n], "--text2dsf"))
		{
			++n;
			if (n >= argc) goto help;
			const char * f1 = argv[n];
			++n;
			if (n >= argc) goto help;
			const char * f2 = argv[n];
			++n;

			printf("Converting %s from text to DSF as %s\n", f1, f2);
			if (Text2DSF(f1, f2))
				printf("Converted %s to %s\n",f1, f2);
			else
				{ fprintf(err_fi, "ERROR: Error convertiong %s to %s\n", f1, f2); exit(1); }
		}
		else if(!strcmp(argv[n], "--version"))
		{
			print_product_version("DSFTool", DSFTOOL_VER, DSFTOOL_EXTRAVER);
		}
		else
		{
			fprintf(err_fi, "Unknown argument \"%s\"\n", argv[n]);
			goto help;
		}
	}

	return 0;
help:
	fprintf(err_fi, "Usage: dsftool --dsf2text [dsffile] [textfile]\n");
	fprintf(err_fi, "       dsftool --text2dsf [textfile] [dsffile]\n");
	fprintf(err_fi, "       dsftool --env2overlay [envfile] [dsffile]\n");
	fprintf(err_fi, "       dsftool --version\n");
	fprintf(err_fi, "Please note: dsftool still supports single-hyphen (-dsf2text) syntax for backward compatibility.\n");
	return 1;
}
