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
#include "QuiltUtils.h"
#include "FileUtils.h"
#include "MathUtils.h"

#if PHONE
	#define WANT_PVR 1
	#define WANT_ATI IBM
#else
	#define WANT_PVR 0
	#define WANT_ATI 0
#endif



#if PHONE
	#if WANT_ATI
		#include "ATI_Compress.h"
	#endif
	#include "MathUtils.h"
#endif

/*

	WHAT IS ALL OF THIS PHONE STUFF???

	DDSTool is mainly used by the X-Plane community to convert PNGs to DDS/DXT files so that the texture compression they get is high quality/produced offline.

	But...LR also uses DDSTool and XGrinder to prepare content for the iphone apps.  The iphone uses "pvr" files, an image container for use with PowerVR's
	PVRTC compressed format.  In other words, the iphone has its own file formats and its own compression.  When PHONE is defined to 1, the tools compile
	with iphone options enabled...that's what most of this junk is.  Normally we ship the tools with phone features off because they are (1) completely useless
	to desktop users and (2) likely to create confusion.

*/

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

#if PHONE
typedef struct ATC_Header_Texture_TAG
{
		unsigned int signature;
		unsigned int width;
		unsigned int height;
		unsigned int flags;
		unsigned int dataOffset;  // From start of header/file
		unsigned int reserved1;
		unsigned int reserved2;
		unsigned int reserved3;
} ATC_Texture_Header;
#endif

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

#if PHONE
enum {
		ATC_RGB   			= 0x01,
		ATC_RGBA  			= 0x02,
		ATC_TILED 			= 0x04,
		ATC_INTERP_RGBA		= 0x12,
		ATC_RAW_RGBA_4444	= 0x20,		// From here down, these are formats that WE made up
		ATC_RAW_RGBA_8888,				// They are NOT part of the ATITC spec.
		ATC_RAW_RGB_565,
		ATC_RAW_RGB_888,
		ATC_RAW_I_8
};
#endif

static int size_for_image(int x, int y, int bpp, int min_bytes)
{
	return max(x * y * bpp / 8, min_bytes);
}
static int size_for_mipmap(int x, int y, int bpp, int min_bytes)
{
	int total = 0;
	while (1)
	{
		total += size_for_image(x,y,bpp, min_bytes);
		if (x == 1 && y == 1)
			break;
		if (x > 1) x >>= 1;
		if (y > 1) y >>= 1;
	}
	return total;
}

#if PHONE
static int WriteToRaw(const ImageInfo& info, const char * outf, int s_raw_16_bit, bool isPvr, int mip_count)
{
	PVR_Texture_Header	h 			= { 0 };
	ATC_Texture_Header	atcHdr 		= { 0 };
	unsigned int 		totalSize 	= 0;
	unsigned int		hdrSize 	= 0;
	void*				hdr			= NULL;
	int					bpp = 8;
	ImageInfo			img(info);				/// copy so we can advance the mip stack.

	switch(info.channels) {
	case 1:		bpp = 8;						break;
	case 3:		bpp = s_raw_16_bit ? 16 : 24;	break;
	case 4:		bpp = s_raw_16_bit ? 16 : 32;	break;
	}
	totalSize = (mip_count > 1) ? size_for_mipmap(info.width, info.height, bpp, 1) : size_for_image(info.width,info.height,bpp,1);

	if(isPvr)
	{
		h.dwHeaderSize = sizeof(h);
		h.dwHeight = info.height;
		h.dwWidth = info.width;
		h.dwMipMapCount = mip_count;
		h.dwTextureDataSize = totalSize;
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

		hdrSize = h.dwHeaderSize;
		hdr = &h;
	}
	// We must be ATC then if we're not PVR
	else
	{
		atcHdr.signature = 0xCCC40002;
		atcHdr.width = info.width;
		atcHdr.height = info.height;
		atcHdr.dataOffset = sizeof(atcHdr);

		if(s_raw_16_bit)
		switch(info.channels) {
		case 1: atcHdr.flags =	ATC_RAW_I_8;		break;
		case 3: atcHdr.flags =	ATC_RAW_RGB_565;	break;
		case 4: atcHdr.flags =	ATC_RAW_RGBA_4444;	break;
		}
		else
		switch(info.channels) {
		case 1: atcHdr.flags =	ATC_RAW_I_8;		break;
		case 3: atcHdr.flags =	ATC_RAW_RGB_888;	break;
		case 4: atcHdr.flags =	ATC_RAW_RGBA_8888;	break;
		}

		hdrSize = sizeof(atcHdr);
		hdr = &atcHdr;
	}

	int sbp = info.channels;
	int dbp = bpp / 8;

	unsigned char * storage = (unsigned char *) malloc(totalSize);
	unsigned char * base_ptr = storage;

	while(mip_count--)
	{
		for(int y = 0; y < img.height; ++y)
		for(int x = 0; x < img.width; ++x)
		{
			unsigned char * srcb = (unsigned char*)img.data + img.width * sbp * y + x * sbp;
			unsigned char * dstb = base_ptr + img.width * dbp * y + dbp * x;
			if(img.channels == 1)
			{
				*dstb = *srcb;
			}
			else if (img.channels == 3)
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
			else if (img.channels == 4)
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

		base_ptr += size_for_image(img.width,img.height,bpp,1);
		AdvanceMipmapStack(&img);
	}

	FILE * fi = fopen(outf,"wb");
	if(fi)
	{
		fwrite(hdr,1,hdrSize,fi);
		fwrite(storage,1,totalSize,fi);
		fclose(fi);
	}
	free(storage);
	return 0;
}
#endif

int pow2_up(int n)
{
	int o = 1;
	while(o < n) o *= 2;
	return o;
}

int pow2_down(int n)
{
	int o = 65536;
	while(o > n) o /= 2;
	return o;
}

// Resizes the image to meet our constraints.
// up - resize bigger to hit power of 2
// down - resize smaller to hit power of 2
// if neither: do nothing
// square: require image to be square
// returns true if the image ALREADY meets these criteria, false if it must
// be resized.  if up & down are both false, a "false" return leaves the image in its
// original (unusable) form.
static bool HandleScale(ImageInfo& info, bool up, bool down, bool half, bool square)
{
	int nx = down ? pow2_down(info.width) : pow2_up(info.width);
	int ny = down ? pow2_down(info.height) : pow2_up(info.height);
	if(half) nx /= 2.0;
	if(half) ny /= 2.0;

	if(square && up	 ) nx = ny = max(nx,ny);
	if(square && down) nx = ny = min(nx, ny);

	if(nx == info.width && ny == info.height)
		return true;



	if(!up && !down && !half)
		return false;

	ImageInfo	n;
	CreateNewBitmap(nx,ny,info.channels, &n);
	CopyBitmapSection(&info, &n, 0, 0, info.width, info.height, 0, 0, nx, ny);
	swap(n, info);
	DestroyBitmap(&n);

	return false;
}


inline float to_srgb(float p)
{
	if(p <= 0.0031308f)
		return 12.92f * p;
	return 1.055f * pow(p,0.41666f) - 0.055f;
}

inline float from_srgb(float p)
{
	if(p <= 0.04045f)
		return p / 12.92f;
	else
		return powf(p * (1.0/1.055f) + (0.055f/1.055f),2.4f);
}

unsigned char srgb_filter(unsigned char src[], int count, int channel, int level)
{
	if(channel == 3)	// alpha is not corrected
	{
		int total = 0;
		for(int i = 0; i < count; ++i)
			total += (int) src[i];
		return min(255,total / count);
	}
	
	float total = 0.f;
	for(int i = 0; i < count; ++i)
	{
		float p = src[i];
		p /= 255.0f;
		p = from_srgb(p);
		total += p;
	}
	
	total /= ((float) count);
	
	total = to_srgb(total);
	
	total *= 255.0f;
	
	if(total <= 0.0f) return 0;
	if (total >= 255.0f) return 255;

	return round(total);   // msvc2010 has no roundf
}

unsigned char night_filter(unsigned char src[], int count, int channel, int level)
{
	int total = 0;
	for(int i = 0; i < count; ++i)
		total += (int) src[i];
	total *= 2;
	return min(255,total / count);
}

unsigned char fade_filter(unsigned char src[], int count, int channel, int level)
{
	int total = 0;
	for(int i = 0; i < count; ++i)
		total += (int) src[i];
	total /= count;
	if (channel == 3)
	{
		float alpha = interp(3,1.0,6,0.0,level);
		total = (float) total * alpha;
	}

	return total;
}

unsigned char fade_2_black_filter(unsigned char src[], int count, int channel, int level)
{
	int total = 0;
	for(int i = 0; i < count; ++i)
		total += (int) src[i];
	total /= count;
	{
		float alpha = interp(3,1.0,6,0.0,level);
		total = (float) total * alpha;
	}

	return total;
}


int main(int argc, char * argv[])
{
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
		printf("CMD .png .dds \"%s\" DDS_MODE HAS_MIPS GAMMA_MODE PVR_SCALE \"INFILE\" \"OUTFILE\"\n",argv[0]);
		printf("OPTIONS DDSTool\n");
		printf("RADIO DDS_MODE 1 --png2dxt Auto-pick compression\n");
		printf("RADIO DDS_MODE 0 --png2dxt1 Use DXT1 Compression (1-bit alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt3 Use DXT3 Compression (high-freq alpha)\n");
		printf("RADIO DDS_MODE 0 --png2dxt5 Use DXT5 Compression (smooth alpha)\n");
		printf("RADIO DDS_MODE 0 --png2rgb No Compression\n");
		printf("DIV\n");
		printf("RADIO HAS_MIPS 1 --std_mips Generate Mip-Maps\n");
		printf("RADIO HAS_MIPS 0 --pre_mips Image Is a Mip-Map Tree\n");
		printf("RADIO HAS_MIPS 0 --night_mips Generate Night-Style Mip-Map\n");
		printf("RADIO HAS_MIPS 0 --fade_mips Generate Fading Mip-Map\n");
		printf("RADIO HAS_MIPS 0 --ctl_mips Generate Fading CTL Mip-Map\n");
		printf("DIV\n");
		printf("RADIO GAMMA_MODE 1 --gamma_22 Use X-Plane 10 Gamma\n");
		printf("RADIO GAMMA_MODE 0 --gamma_18 Use X-Plane 9 Gamma\n");

#if WANT_ATI
		printf("CMD .png .atc \"%s\" ATC_MODE MIPS PVR_SCALE \"INFILE\" \"OUTFILE\"\n",argv[0]);
		printf("DIV\n");
		printf("RADIO ATC_MODE 1 --png2atc4 4-bit ATC compression\n");
		printf("RADIO ATC_MODE 0 --png2atc_raw16 ATC uses 16-bit color\n");
		printf("RADIO ATC_MODE 0 --png2atc_raw24 ATC uses 24-bit color\n");
#endif
#if PHONE
		printf("CMD .png .txt \"%s\" --info ONEFILE \"INFILE\" \"OUTFILE\"\n", argv[0]);
		printf("DIV\n");
		printf("RADIO PVR_MODE 1 --png2pvrtc2 2-bit PVR compression\n");
		printf("RADIO PVR_MODE 0 --png2pvrtc4 4-bit PVR compression\n");
		printf("RADIO PVR_MODE 0 --png2pvr_raw16 PVR uses 16-bit color\n");
		printf("RADIO PVR_MODE 0 --png2pvr_raw24 PVR uses 24-bit color\n");
#endif
		printf("DIV\n");
		printf("RADIO PVR_SCALE 1 --scale_none Do not resize images\n");
		printf("RADIO PVR_SCALE 0 --scale_up Scale up to nearest power of 2\n");
		printf("RADIO PVR_SCALE 0 --scale_down Scale down to nearest power of 2\n");
		printf("RADIO PVR_SCALE 0 --scale_half Scale down by a factor of two\n");
#if PHONE
		printf("DIV\n");
		printf("CHECK PREVIEW 0 --make_preview Output preview of compressed PVR image\n");
		printf("CHECK MIPS 1 --make_mips Create mipmap for PVR image\n");
		printf("CHECK ONEFILE 1 --one_file All text info goes into one file\n");
		printf("CMD .png .pvr \"%s\" PVR_MODE PVR_SCALE PREVIEW MIPS \"INFILE\" \"OUTFILE\"\n",argv[0]);
#endif
		return 0;
	}

	if (argc < 4) {
		printf("Usage: %s <convert mode> <options> <input_file> <output_file>|-\n",argv[0]);
		printf("Usage: %s --quilt <input_file> <width> <height> <patch size> <overlap> <trials> <output_files>n",argv[0]);
		printf("       %s --version\n",argv[0]);
		exit(1);
	}

	if(strcmp(argv[1],"--info")==0)
	{
		ImageInfo	info;

		int n = 2;
		bool one_file = false;

		if(strcmp(argv[n],"--one_file")==0) { ++n; one_file = true; }
		if(CreateBitmapFromPNG(argv[n], &info, true, GAMMA_SRGB))
		{
			printf("Unable to open png file %s\n", argv[n]);
			return 1;
		}

		string target = argv[n+1];
		if(one_file)
		{
			string::size_type p = target.find_last_of("\\/:");
			if(p != target.npos)
				target.erase(p+1);
			target += "info.txt";
		}

		FILE * fi = fopen(target.c_str(), one_file ? "a" : "w");
		if(fi)
		{
			const char * name = argv[n];
			const char * p = name;
			while(*p)
			{
				if(*p == '\\' || *p == ':' || *p == '/')
					name = p+1;
				++p;
			}
			fprintf(fi,"\"%s\", %ld, %ld, %hd\n", name, info.width, info.height, info.channels);
			fclose(fi);
		}

	}

	else if(strcmp(argv[1],"--png2pvrtc2")==0 ||
	   strcmp(argv[1],"--png2pvrtc4")==0)
	{
		char cmd_buf[2048];
		const char *							pvr_mode = "--bits-per-pixel-2";
		if(strcmp(argv[1],"--png2pvrtc4")==0)	pvr_mode = "--bits-per-pixel-4";

		// PowerVR does not provide open src for PVRTC.  So...we have to convert our png using Apple's texture tool.

		bool want_preview = false;
		bool want_mips = false;
		bool scale_up = strcmp(argv[2], "--scale_up") == 0;
		bool scale_down = strcmp(argv[2], "--scale_down") == 0;
		bool scale_half = strcmp(argv[2], "--scale_half") == 0;

		int n = 3;

		if(strcmp(argv[n],"--make_preview")==0) { want_preview = true; ++n; }
		if(strcmp(argv[n],"--make_mips")==0) { want_mips = true; ++n; }

		ImageInfo	info;
		if(CreateBitmapFromPNG(argv[n], &info, true, 2.2f))
		{
			printf("Unable to open png file %s\n", argv[n]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, true))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The image is not a square power of 2.  It is: %ld by %ld\n", info.width, info.height);
				return 1;
			}
			else if(want_preview)
			{
				string preview_path = string(argv[n]) + ".scl.png";
				WriteBitmapToPNG(&info, preview_path.c_str(), NULL, 0, 2.2f);
			}
		}

		FlipImageY(info);
		string temp_path = argv[n];
		temp_path += "_temp";
		if(WriteBitmapToPNG(&info, temp_path.c_str(), NULL, 0, 2.2f))
		{
			printf("Unable to write temp file %s\n", temp_path.c_str());
			return 1;
		}
		DestroyBitmap(&info);

		string preview = string(argv[n+1]) + ".png";

		string flags;
		if(want_mips) flags += "-m ";
		if(want_preview){
			flags += "-p \"";
			flags += preview;
			flags += "\" ";
		}

		sprintf(cmd_buf,"\"%stexturetool\" -e PVRTC %s %s -c PVR -o \"%s\" \"%s\"", my_dir, pvr_mode, flags.c_str(), argv[n+1], temp_path.c_str());
		printf("Cmd: %s\n", cmd_buf);
		system(cmd_buf);

		FILE_delete_file(temp_path.c_str(), 0);
		if(want_preview)
		{
			if(!CreateBitmapFromPNG(preview.c_str(), &info, true, 2.2f))
			{
				FlipImageY(info);
				WriteBitmapToPNG(&info, preview.c_str(),0,false, 2.2f);
				DestroyBitmap(&info);
			}
		}

		return 1;
	}
#if PHONE
	else if(strcmp(argv[1],"--png2pvr_raw16")==0 ||
			strcmp(argv[1],"--png2pvr_raw24")==0)
	{
		bool want_preview = false;
		bool want_mips = false;
		bool scale_up = strcmp(argv[2], "--scale_up") == 0;
		bool scale_down = strcmp(argv[2], "--scale_down") == 0;
		bool scale_half = strcmp(argv[2], "--scale_half") == 0;

		int n = 3;

		if(strcmp(argv[n],"--make_preview")==0) { want_preview = true; ++n; }
		if(strcmp(argv[n],"--make_mips")==0) { want_mips = true; ++n; }

		ImageInfo	info;
		if (CreateBitmapFromPNG(argv[n], &info, true,2.2f)!=0)
		{
			printf("Unable to open png file %s\n", argv[n]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, false))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The imager is not a power of 2.  It is: %d by %d\n", info.width, info.height);
				return 1;
			}
			else if(want_preview)
			{
				string preview_path = string(argv[n]) + ".scl.png";
				WriteBitmapToPNG(&info, preview_path.c_str(), NULL, 0, 2.2f);
			}
		}

		char buf[1024];
		const char * outf = argv[n+1];
		if(strcmp(outf,"-")==0)
		{
			strcpy(buf,argv[n]);
			buf[strlen(buf)-4]=0;
			strcat(buf,".raw");
			outf=buf;
		}
		int mc = 1;
		if(want_mips)
		mc = MakeMipmapStack(&info);

		if (WriteToRaw(info, outf, strcmp(argv[1],"--png2pvr_raw16")==0, true, mc)!=0)
		{
			printf("Unable to write raw PVR file %s\n", argv[n+1]);
			return 1;
		}
		return 0;
	}
#endif
	else if(strcmp(argv[1],"--png2dxt")==0 ||
	   strcmp(argv[1],"--png2dxt1")==0 ||
	   strcmp(argv[1],"--png2dxt3")==0 ||
	   strcmp(argv[1],"--png2dxt5")==0)
	{
		int arg_base = 2;
		int has_mips = 0;

		if(strcmp(argv[arg_base], "--std_mips") == 0)
		{
			has_mips = 0;
			++arg_base;
		}
		else if(strcmp(argv[arg_base], "--pre_mips") == 0)
		{
			has_mips = 1;
			++arg_base;
		}
		else if(strcmp(argv[arg_base], "--night_mips") == 0)
		{
			has_mips = 2;
			++arg_base;
		}
		else if(strcmp(argv[arg_base], "--fade_mips") == 0)
		{
			has_mips = 3;
			++arg_base;
		}
		else if(strcmp(argv[arg_base], "--ctl_mips") == 0)
		{
			has_mips = 4;
			++arg_base;
		}

		float gamma = (strcmp(argv[arg_base], "--gamma_22") == 0) ? 2.2f : 1.8f;
		arg_base +=1;


		bool scale_up = strcmp(argv[arg_base], "--scale_up") == 0;
		bool scale_down = strcmp(argv[arg_base], "--scale_down") == 0;
		bool scale_half = strcmp(argv[arg_base], "--scale_half") == 0;
		arg_base +=1;

		ImageInfo	info;
		if (CreateBitmapFromPNG(argv[arg_base], &info, false, gamma)!=0)
		{
			printf("Unable to open png file %s\n", argv[arg_base]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, false))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The imager is not a power of 2.  It is: %ld by %ld\n", info.width, info.height);
				return 1;
			}
		}

		char buf[1024];
		const char * outf = argv[arg_base+1];
		if(strcmp(outf,"-")==0)
		{
			strcpy(buf,argv[arg_base]);
			buf[strlen(buf)-4]=0;
			strcat(buf,".dds");
			outf=buf;
		}

		if(info.channels == 1)
		{
			printf("Unable to write DDS file from alpha-only PNG %s\n", argv[arg_base+1]);
		}
		int dxt_type = argv[1][9]-'0';
		if(argv[1][9]==0)
		{
			if(info.channels == 3)  dxt_type=1;
			else					dxt_type=5;
		}

		ConvertBitmapToAlpha(&info,false);
		switch(has_mips) {
//		case 0:			MakeMipmapStack(&info);							break;
		case 0:			MakeMipmapStackWithFilter(&info,srgb_filter);	break;
		case 1:			MakeMipmapStackFromImage(&info);				break;
		case 2:			MakeMipmapStackWithFilter(&info,night_filter);	break;
		case 3:			MakeMipmapStackWithFilter(&info,fade_filter);	break;
		case 4:			MakeMipmapStackWithFilter(&info,fade_2_black_filter);	break;
		}

		if (WriteBitmapToDDS(info, dxt_type, outf, gamma == GAMMA_SRGB)!=0)
		{
			printf("Unable to write DDS file %s\n", argv[arg_base+1]);
			return 1;
		}
		return 0;
	}
	else if(strcmp(argv[1],"--png2rgb")==0)
	{
		int arg_base = 2;
		int has_mips = 0;
		if(strcmp(argv[2], "--std_mips") == 0)
		{
			has_mips = 0;
			++arg_base;
		}
		else if(strcmp(argv[2], "--pre_mips") == 0)
		{
			has_mips = 1;
			++arg_base;
		}
		else if(strcmp(argv[2], "--night_mips") == 0)
		{
			has_mips = 2;
			++arg_base;
		}
		else if(strcmp(argv[2], "--fade_mips") == 0)
		{
			has_mips = 3;
			++arg_base;
		}
		else if(strcmp(argv[2], "--ctl_mips") == 0)
		{
			has_mips = 4;
			++arg_base;
		}

		float gamma = (strcmp(argv[arg_base], "--gamma_22") == 0) ? 2.2f : 1.8f;
		arg_base +=1;


		bool scale_up = strcmp(argv[arg_base], "--scale_up") == 0;
		bool scale_down = strcmp(argv[arg_base], "--scale_down") == 0;
		bool scale_half = strcmp(argv[arg_base], "--scale_half") == 0;
		arg_base +=1;

		ImageInfo	info;
		if (CreateBitmapFromPNG(argv[arg_base], &info, true, gamma)!=0)
		{
			printf("Unable to open png file %s\n", argv[arg_base]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, false))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The imager is not a power of 2.  It is: %ld by %ld\n", info.width, info.height);
				return 1;
			}
		}

		char buf[1024];
		const char * outf = argv[arg_base+1];
		if(strcmp(outf,"-")==0)
		{
			strcpy(buf,argv[arg_base]);
			buf[strlen(buf)-4]=0;
			strcat(buf,".raw");
			outf=buf;
		}

		switch(has_mips) {
		case 0:			MakeMipmapStack(&info);							break;
		case 1:			MakeMipmapStackFromImage(&info);				break;
		case 2:			MakeMipmapStackWithFilter(&info,night_filter);	break;
		case 3:			MakeMipmapStackWithFilter(&info,fade_filter);	break;
		case 4:			MakeMipmapStackWithFilter(&info,fade_2_black_filter);	break;
		}


		if (WriteUncompressedToDDS(info, outf, gamma == GAMMA_SRGB)!=0)
		{
			printf("Unable to write DDS file %s\n", argv[arg_base+1]);
			return 1;
		}
		return 0;
		// Quilt src w h splat over trials dest
	}
	else if (strcmp(argv[1],"--quilt")==0)
	{
		ImageInfo src, dst;
		if(CreateBitmapFromPNG(argv[2], &src, false, 2.2f) != 0)
			return 1;

		int dst_w = atoi(argv[3]);
		int dst_h = atoi(argv[4]);
		int splat = atoi(argv[5]);
		int overlap = atoi(argv[6]);
		int trials = atoi(argv[7]);

		printf("Will make %d x %d tex, with %d splats (%d overlap, %d trials.)\n", dst_w,dst_h, splat,overlap,trials);

		CreateNewBitmap(dst_w,dst_h, 4, &dst);
		if(src.channels == 3) ConvertBitmapToAlpha(&src,false);

		make_texture(src, dst, splat, overlap, trials);

		WriteBitmapToPNG(&dst, argv[8], NULL, 0, 2.2f);

	}
#if PHONE && WANT_ATI
	else if(strcmp(argv[1],"--png2atc4")==0)
	{
		bool scale_up = strcmp(argv[2], "--scale_up") == 0;
		bool scale_down = strcmp(argv[2], "--scale_down") == 0;
		bool scale_half = strcmp(argv[2], "--scale_half") == 0;
		int n = 3;

		ImageInfo	info;
		if(CreateBitmapFromPNG(argv[n], &info, true, 2.2f))
		{
			printf("Unable to open png file %s\n", argv[n]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, false))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The imager is not a power of 2.  It is: %d by %d\n", info.width, info.height);
				return 1;
			}
		}

		// We need to save our channel count because MakeMipmapStack is going to
		// manually force everything to have an alpha channel.
		int channels = info.channels;
		// Ben says: WTF? Well, in the old days, MakeMipmapStack "upgraded" the image to RGBA.  Chris's
		// code goes through some hoops to avoid that.  The new MakeMipmapStack works in any color format.
		// So to keep Chris's code working, I do the upgrade by hand.
		// Chris, you could someday rip out the convert bitmap to alpha call here as well as the special
		// handling of RGB images.
		ConvertBitmapToAlpha(&info,false);
		MakeMipmapStack(&info);
		struct ImageInfo img(info);

		ATC_Texture_Header h;
		h.signature = 0xCCC40002;
		h.dataOffset = sizeof(h);
		h.height = info.height;
		h.width = info.width;
		h.flags = (channels == 3) ? ATC_RGB : ATC_RGBA;
		h.reserved1 = h.reserved2 = h.reserved3 = 0;

		FILE * fi = fopen(argv[++n],"wb");

		if(!fi)
		{
			printf("Unable to open destination file %s\n", argv[n]);
			return 1;
		}

		// Write the file header
		fwrite(&h,1,sizeof(h),fi);

		// Initialize our source texture
		ATI_TC_Texture srcTex;
		srcTex.dwSize = sizeof(srcTex);
		srcTex.format = (channels == 3) ? ATI_TC_FORMAT_RGB_888 : ATI_TC_FORMAT_ARGB_8888;

		// Initialize our destination texture
		ATI_TC_Texture destTex;
		destTex.dwSize = sizeof(destTex);
		destTex.format = (channels == 3) ? ATI_TC_FORMAT_ATC_RGB : ATI_TC_FORMAT_ATC_RGBA_Explicit;

		do {
			ImageInfo tempImg;
			CreateNewBitmap(img.width, img.height, img.channels, &tempImg);
			CopyBitmapSectionDirect(img, tempImg, 0, 0, 0, 0, img.width, img.height);

			if(channels == 3)
				ConvertAlphaToBitmap(&tempImg, false, false);

			srcTex.dwWidth = tempImg.width;
			srcTex.dwHeight = tempImg.height;
			// This is true because we shut padding off in ConvertAlphaToBitmap
			srcTex.dwPitch = tempImg.width * tempImg.channels;
			srcTex.dwDataSize = (tempImg.width * tempImg.height * (tempImg.channels * 8)) / 8;
			srcTex.pData = tempImg.data;

			destTex.dwWidth = srcTex.dwWidth;
			destTex.dwHeight = srcTex.dwHeight;
			destTex.dwPitch = 0;
			destTex.dwDataSize = intmax2((destTex.dwHeight * destTex.dwWidth * (tempImg.channels == 3 ? 4 : 8)) / 8, (tempImg.channels == 3) ? 8 : 16);
   			destTex.pData = (ATI_TC_BYTE*) malloc(destTex.dwDataSize);

			// Convert it!
   			ATI_TC_ConvertTexture(&srcTex, &destTex, NULL, NULL, NULL, NULL);

   			fwrite(destTex.pData,1,destTex.dwDataSize,fi);
   			free(destTex.pData);
   			DestroyBitmap(&tempImg);

			if(!AdvanceMipmapStack(&img))
				break;
		} while (1);

		fclose(fi);
		DestroyBitmap(&info);

	}
	else if(strcmp(argv[1],"--png2atc_raw16")==0 ||
			strcmp(argv[1],"--png2atc_raw24")==0)
	{
		bool want_mips  = false;
		bool scale_up   = false;
		bool scale_down = false;
		bool scale_half = false;

		int n = 2;
		     if(strcmp(argv[n], "--make_mips")  ==0) 	{ want_mips  = true;++n; }

			 if(strcmp(argv[n], "--scale_none") ==0)	{					++n; }
		else if(strcmp(argv[n], "--scale_up")   ==0)	{ scale_up   = true;++n; }
		else if(strcmp(argv[n], "--scale_down") ==0)	{ scale_down = true;++n; }
		else if(strcmp(argv[n], "--scale_half") ==0)	{ scale_half = true;++n; }

		ImageInfo	info;
		if(CreateBitmapFromPNG(argv[n], &info, true, 2.2f))
		{
			printf("Unable to open png file %s\n", argv[n]);
			return 1;
		}

		if (!HandleScale(info, scale_up, scale_down, scale_half, false))
		{
			// Image does NOT meet our power of 2 needs.
			if(!scale_up && !scale_down && !scale_half)
			{
				printf("The imager is not a power of 2.  It is: %d by %d\n", info.width, info.height);
				return 1;
			}
		}

		const char * outf = argv[++n];

		int mc = 1;
		if(want_mips)
		mc = MakeMipmapStack(&info);

		if (WriteToRaw(info, outf, strcmp(argv[1],"--png2atc_raw16")==0, false, mc)!=0)
		{
			printf("Unable to write raw ARC file %s\n", argv[n]);
			return 1;
		}

		return 0;
	}
#endif
	else {
		printf("Unknown conversion flag %s\n", argv[1]);
		return 1;
	}
	return 0;
}

