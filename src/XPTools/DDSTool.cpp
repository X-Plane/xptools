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

	if (argc == 2 && strcmp(argv[1],"--auto_config")==0)
	{
		printf("CMD .png .dds %s DDS_MODE \"INFILE\" \"OUTFILE\"\n",argv[0]);
		printf("OPTIONS DDSTool\n");
		printf("RADIO DDS_MODE 1 --png2dxt Auto-pick compression\n");
		printf("RADIO DDS_MODE 0 --png2dxt1 Use DXT1 Compression (1-bit alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt3 Use DXT3 Compression (high-freq alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt5 Use DXT5 Compression (smooth alpha)\n");
		printf("RADIO DDS_MODE 0 --png2rgb Use no compression (requires mipmap)\n");
		return 0;
	}

	if (argc < 4) { 
		printf("Usage: %s --png2dds <input_file> <output_file>|-\n",argv[0]); exit(1); 
	}
	
	if(strcmp(argv[1],"--png2dxt")==0 ||
	   strcmp(argv[1],"--png2dxt1")==0 ||
	   strcmp(argv[1],"--png2dxt3")==0 ||
	   strcmp(argv[1],"--png2dxt5")==0)
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
		
		if(info.channels == 1)
		{
			printf("Unable to write DDS file from alpha-only PNG %s\n", argv[2]);			
		}
		int dxt_type = argv[1][9]-'0';
		if(argv[1][9]==0)
		{
			if(info.channels == 3)  dxt_type=1;
			else					dxt_type=5;
		}
		
		if (WriteBitmapToDDS(info, dxt_type,outf)!=0)
		{
			printf("Unable to write DDS file %s\n", argv[3]);
			return 1;
		}
		return 0;
	} 
	else if(strcmp(argv[1],"--png2rgb")==0)
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
		
		if (WriteUncompressedToDDS(info, outf)!=0)
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

