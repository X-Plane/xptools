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

#include "version.h"
#include "BitmapUtils.h"

#if LIN
#include "initializer.h"
#endif

enum {
	raw_16 = 0,
	raw_24 = 1,
	pvr_2 = 2,
	pvr_4 = 3
};
static int exp_mode = pvr_2;

typedef struct PVR_Header_Texture_TAG
{
        unsigned int dwHeaderSize;                      /*!< size of the structure */
        unsigned int dwHeight;                          /*!< height of surface to be created */
        unsigned int dwWidth;                           /*!< width of input surface */
        unsigned int dwMipMapCount;                     /*!< number of mip-map levels requested */
        unsigned int dwpfFlags;                         /*!< pixel format flags */
        unsigned int dwTextureDataSize;                                 /*!< Total size in bytes */
        unsigned int dwBitCount;                        /*!< number of bits per pixel  */
        unsigned int dwRBitMask;                        /*!< mask for red bit */
        unsigned int dwGBitMask;                        /*!< mask for green bits */
        unsigned int dwBBitMask;                        /*!< mask for blue bits */
        unsigned int dwAlphaBitMask;                                    /*!< mask for alpha channel */
        unsigned int dwPVR;                             /*!< magic number identifying pvr file */
        unsigned int dwNumSurfs;                        /*!< the number of surfaces present in the pvr */
} PVR_Texture_Header;

enum {
		OGL_RGBA_4444= 0x10,
        OGL_RGBA_5551,
        OGL_RGBA_8888,
        OGL_RGB_565,
        OGL_RGB_555,
        OGL_RGB_888,
        OGL_I_8,
        OGL_AI_88,
        OGL_PVRTC2,
        OGL_PVRTC4,
        OGL_PVRTC2_2,
        OGL_PVRTC2_4,
};

static int WriteToRaw(const ImageInfo& info, const char * outf, int s_raw_16_bit)
{
	PVR_Texture_Header	h = { 0 };
	h.dwHeaderSize = sizeof(h);
	h.dwHeight = info.height;
	h.dwWidth = info.width;
	h.dwMipMapCount = 1;
	if(s_raw_16_bit)		h.dwTextureDataSize = (info.channels > 1) ? (info.width * info.height * 2) : (info.width * info.height);
	else					h.dwTextureDataSize = info.channels * info.width * info.height;
	h.dwPVR = 0x21525650;
	if(s_raw_16_bit)
	switch(info.channels) {
	case 1: h.dwpfFlags =	OGL_I_8;		break;
	case 3: h.dwpfFlags =	OGL_RGB_565;	break;
	case 4: h.dwpfFlags =	OGL_RGBA_4444;	break;
	}
	else
	switch(info.channels) {
	case 1: h.dwpfFlags =	OGL_I_8;		break;
	case 3: h.dwpfFlags =	OGL_RGB_888;	break;
	case 4: h.dwpfFlags =	OGL_RGBA_8888;	break;
	}
	int sbp = info.channels;
	int dbp = sbp >= 3 ? 2 : 1;
	if(!s_raw_16_bit)
		dbp = sbp;
	unsigned char * storage = (unsigned char *) malloc(h.dwTextureDataSize);

	for(int y = 0; y < info.height; ++y)
	for(int x = 0; x < info.width; ++x)
	{
		unsigned char * srcb = (unsigned char*)info.data + info.width * sbp * y + x * sbp;
		unsigned char * dstb = storage + info.width * dbp * y + dbp * x;
		if(info.channels == 1)
		{
			*dstb = *srcb;
		}
		else if (info.channels == 3)
		{
			if(s_raw_16_bit)
			*((unsigned short *) dstb) =
			((srcb[2] & 0xF8) << 8) |
			((srcb[1] & 0xFC) << 3) |
			((srcb[0] & 0xF8) >> 3);
			else
				dstb[0]=srcb[2],
				dstb[1]=srcb[1],
				dstb[2]=srcb[0];
		}
		else if (info.channels == 4)
		{
			if(s_raw_16_bit)
			*((unsigned short *) dstb) =
			((srcb[2] & 0xF0) << 8) |
			((srcb[1] & 0xF0) << 4) |
			((srcb[0] & 0xF0) << 0) |
			((srcb[3] & 0xF0) >> 4);
			else
				dstb[0]=srcb[2],
				dstb[1]=srcb[1],
				dstb[2]=srcb[0],
				dstb[3]=srcb[3];
		}
	}

	FILE * fi = fopen(outf,"wb");
	if(fi)
	{
		fwrite(&h,1,sizeof(h),fi);
		fwrite(storage,1,h.dwTextureDataSize,fi);
		fclose(fi);
	}
	free(storage);
	return 0;
}


int main(int argc, char * argv[])
{
#if LIN
//	Initializer initializer(&argc, &argv, 0);
#endif

	char	my_dir[2048];
	strcpy(my_dir,argv[0]);
	char * last_slash = my_dir;
	char * p = my_dir;
	while(*p)
	{
		if(*p=='/') last_slash = p;
		++p;
	}
	last_slash[1] = 0;

	// DDSTool --png2dds <infile> <outfile>

	if (argc == 2 && strcmp(argv[1],"--version")==0)
	{
		print_product_version("DDSTool", DDSTOOL_VER, DDSTOOL_EXTRAVER);
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
		printf("DIV\n");
		printf("RADIO PVR_MODE 1 --png2pvrtc2 2-bit PVR compression\n");
		printf("RADIO PVR_MODE 0 --png2pvrtc4 4-bit PVR compression\n");
		printf("RADIO PVR_MODE 0 --png2pvr_raw16 PVR uses 16-bit color\n");
		printf("RADIO PVR_MODE 0 --png2pvr_raw24 PVR uses 24-bit color\n");
		printf("CMD .png .pvr \"%s\" PVR_MODE \"INFILE\" \"OUTFILE\"\n",argv[0]);
		return 0;
	}

	if (argc < 4) {
		printf("Usage: %s --png2dds <input_file> <output_file>|-\n",argv[0]);
		printf("       %s --version\n",argv[0]);
		exit(1);
	}

	if(strcmp(argv[1],"--png2pvrtc2")==0 ||
	   strcmp(argv[1],"--png2pvrtc4")==0)
	{
		char cmd_buf[2048];
		const char *							pvr_mode = "--bits-per-pixel-2";
		if(strcmp(argv[1],"--png2pvrtc4")==0)	pvr_mode = "--bits-per-pixel-4";

		//sprintf(cmd_buf,"\"%stexturetool\" -e PVRTC %s -m -c PVR -o \"%s\" -p \"%s.png\" \"%s\"", my_dir, pvr_mode, argv[3], argv[3], argv[2]);
		//printf("Cmd: %s\n", cmd_buf);
		//system(cmd_buf);
		printf("png2pvrtc/png2pvrtc4 are not implemented, poke Ben <bsupnik@xsquawkbox.net> to fix this.\n");
		return 1;
	}
	else if(strcmp(argv[1],"--png2pvr_raw16")==0 ||
			strcmp(argv[1],"--png2pvr_raw24")==0)
	{
		ImageInfo	info;
		if (CreateBitmapFromPNG(argv[2], &info, true)!=0)
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
		if (WriteToRaw(info, outf, strcmp(argv[1],"--png2pvr_raw16")==0)!=0)
		{
			printf("Unable to write raw PVR file %s\n", argv[3]);
			return 1;
		}
		return 0;
	}
	else if(strcmp(argv[1],"--png2dxt")==0 ||
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

