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

#include "BitmapUtils.h"
int main(int argc, const char * argv[])
{
	// DDSTool --png2dds <infile> <outfile>

	if (argc < 4) { 
		printf("Usage: %s --png2dds <input_file> <output_file>|-\n",argv[0]); exit(1); 
	}
	
	if(strcmp(argv[1],"--png2dds")==0)
	{
		ImageInfo	info;
		if (CreateBitmapFromPNG(argv[2], &info, false)!=0)
		{
			printf("Unable to open png file %s\n", argv[2]);
			return 1;
		}
		
		char buf[1024];
		const char * outf = argv[3];
		if(strcmp(outf,"-")==0)
		{
			strcpy(buf,argv[2]);
			buf[strlen(buf)-4]=0;
			strcat(buf,".dds");
			outf=buf;
		}
		
		if (WriteBitmapToDDS(info, outf)!=0)
		{
			printf("Unable to write DDS file %s\n", argv[3]);
			return 1;
		}
		return 0;
		
	} else {
		printf("Unknown conversion flag %s\n", argv[1]);
		return 1;
	}
	return 0;
}

