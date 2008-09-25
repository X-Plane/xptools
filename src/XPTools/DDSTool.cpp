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

struct raw_header {
	int	width;
	int height;
	int channels;
};

static int WriteToRaw(const ImageInfo& info, const char * outf)
{
	raw_header h;
	h.width = info.width;
	h.height = info.height;
	h.channels = info.channels;

	int sbp = h.channels;
	int dbp = sbp >= 3 ? 2 : 1;

	unsigned char * storage = (unsigned char *) malloc(h.width * h.height * dbp);

	for(int y = 0; y < h.height; ++y)
	for(int x = 0; x < h.width; ++x)
	{
		unsigned char * srcb = (unsigned char*)info.data + h.width * sbp * y + x * sbp;
		unsigned char * dstb = storage + h.width * dbp * y + dbp * x;
		if(info.channels == 1)
		{
			*dstb = *srcb;
		}
		else if (info.channels == 3)
		{
			*((unsigned short *) dstb) =
			((srcb[2] & 0xF8) << 8) |
			((srcb[1] & 0xFC) << 3) |
			((srcb[0] & 0xF8) >> 3);
		}
		else if (info.channels == 4)
		{
			*((unsigned short *) dstb) =
			((srcb[2] & 0xF0) << 8) |
			((srcb[1] & 0xF0) << 4) |
			((srcb[0] & 0xF0) << 0) |
			((srcb[3] & 0xF0) >> 4);
		}
	}

	FILE * fi = fopen(outf,"wb");
	if(fi)
	{
		fwrite(&h,1,sizeof(h),fi);
		fwrite(storage,1,h.width * h.height * dbp,fi);
		fclose(fi);
	}
	free(storage);
	return 0;
}


int main(int argc, const char * argv[])
{
	// DDSTool --png2dds <infile> <outfile>

	if (argc == 2 && strcmp(argv[1],"--version")==0)
	{
		printf("DDSTool 1.0b1, Coyright 2008 Laminar Research.  Compiled on " __DATE__ ".\n");
		return 0;
	}
	if (argc == 2 && strcmp(argv[1],"--auto_config")==0)
	{
		printf("CMD .png .dds \"%s\" DDS_MODE \"INFILE\" \"OUTFILE\"\n",argv[0]);
		printf("OPTIONS DDSTool\n");
		printf("RADIO DDS_MODE 1 --png2dxt Auto-pick compression\n");
		printf("RADIO DDS_MODE 0 --png2dxt1 Use DXT1 Compression (1-bit alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt3 Use DXT3 Compression (high-freq alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt5 Use DXT5 Compression (smooth alpha)\n");
		printf("RADIO DDS_MODE 0 --png2rgb Use no compression (requires mipmap)\n");
		printf("CMD .png .pv2 ./texturetool -e PVRTC --bits-per-pixel-2 -m -o \"OUTFILE\" -p \"INFILE_preview.png\" \"INFILE\"\n");
		printf("CMD .png .pv4 ./texturetool -e PVRTC --bits-per-pixel-4 -m -o \"OUTFILE\" -p \"INFILE_preview.png\" \"INFILE\"\n");
		printf("CMD .png .raw \"%s\" --png2raw \"INFILE\" \"OUTFILE\"\n",argv[0]);
		return 0;
	}

	if (argc < 4) {
		printf("Usage: %s --png2dds <input_file> <output_file>|-\n",argv[0]);
		printf("       %s --version\n",argv[0]);
		exit(1);
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
	else if(strcmp(argv[1],"--png2raw")==0)
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
			strcat(buf,".raw");
			outf=buf;
		}

		if (WriteToRaw(info, outf)!=0)
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
			strcat(buf,".raw");
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

