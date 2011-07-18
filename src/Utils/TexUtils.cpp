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
#include "TexUtils.h"
#include "BitmapUtils.h"
#include "MemFileUtils.h"
#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif
#include "squish.h"

#if IBM
// Ben says - this sucks!
#include "XWinGL.h"
#endif

/*****************************************************************************************
 * UTILS
 *****************************************************************************************/

// janos says: i just stumbled over this function while hunting a memory leak. ben you know
// that i am a fan of microoptimizations, i couldn't resist :-)
/*
static int	NextPowerOf2(int v)
{
	GLint	maxDim = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDim);
	int	pt = 4;
	while (pt < v && pt < maxDim)
		pt *= 2;
	return pt;
}
*/
inline int NextPowerOf2(int a)
{
	GLint	maxDim = 1024;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDim);
	if(maxDim > 4096) maxDim = 4096;
	int rval = 2;
	while(rval < a && rval < maxDim)
		rval <<= 1;
	return rval;
}


void UnpadImage(ImageInfo * im)
{
	for (int y = 0; y < im->height; ++y)
	{
		unsigned char * src = im->data + (y * (im->width * im->channels + im->pad));
		unsigned char * dst = im->data + (y * (im->width * im->channels          ));
		if (src != dst)
			memmove(dst,src,im->width * im->channels);
	}
	im->pad = 0;
}

/*****************************************************************************************
 * LoadTextureFromFile
 *****************************************************************************************/
bool LoadTextureFromFile(
						const char * 	inFileName,
						int 			inTexNum,
						int 			flags,
						int * 			outWidth,
						int * 			outHeight,
						float *  		outS,
						float * 		outT)
{
	bool	ok = false;
	struct ImageInfo	im = { 0 };

/*	MFMemFile * mf = MemFile_Open(inFileName);
	if(mf)
	{
		if(LoadTextureFromDDS((const unsigned char *) MemFile_GetBegin(mf),(const unsigned char *) MemFile_GetEnd(mf),inTexNum,flags,outWidth,outHeight))
		{
			if(outS) *outS = 1.0;
			if(outT) *outT = 1.0;
			MemFile_Close(mf);
			return true;
		}
		MemFile_Close(mf);
	}
*/

	int result =  CreateBitmapFromPNG(inFileName, &im, false, GAMMA_SRGB);
	if (result) result = CreateBitmapFromDDS(inFileName, &im);
	if (result) result = CreateBitmapFromFile(inFileName, &im);
	#if USE_TIF
	if (result) result = CreateBitmapFromTIF(inFileName, &im);
	#endif
	#if USE_JPEG
	if (result) result = CreateBitmapFromJPEG(inFileName, &im);
	#endif
	if (result == 0)
	{
		if (im.pad != 0)
			UnpadImage(&im);

		int res = LoadTextureFromImage(im, inTexNum, flags, outWidth, outHeight, outS, outT);
		DestroyBitmap(&im);
		return res;
	}
	return false;
}

/*****************************************************************************************
 * LoadTextureFromImage
 *****************************************************************************************/
bool LoadTextureFromImage(ImageInfo& im, int inTexNum, int inFlags, int * outWidth, int * outHeight, float * outS, float * outT)
{
	long				count = 0;
	unsigned char * 	p;
	bool				ok = false;

	/* PREP */


	// Process alpha.  Then remove padding.  Finally, figure out the next biggest power of 2.  If we aren't
	// a power of 2, we need to resize.  That will be done with rescaling if the user wants.  Also if the bitmap
	// is bigger than the max power of 2 supported by the HW, force rescaling.
	if (inFlags & tex_MagentaAlpha)	ConvertBitmapToAlpha(&im, true);
	if (im.pad != 0)
 		UnpadImage(&im);

	int		res_x = NextPowerOf2(im.width);
	int		res_y = NextPowerOf2(im.height);
	bool	resize = (res_x != im.width || res_y != im.height);
	bool	rescale = resize && (inFlags & tex_Rescale);
	if (resize && (im.width > res_x || im.height > res_y))	// Always rescale if the image is too big for the max tex!!
		rescale = true;

	ImageInfo *	useIt = &im;
	ImageInfo	rescaleBits;
	if (resize)
	{
		if (CreateNewBitmap(res_x, res_y, im.channels, &rescaleBits) != 0)
		{
			return false;
		}
		useIt = &rescaleBits;

		if (rescale) {
			CopyBitmapSection(&im, &rescaleBits, 0, 0, im.width, im.height, 0, 0, rescaleBits.width, rescaleBits.height);
			if (outS) *outS = 1.0;
			if (outT) *outT = 1.0;
		} else {
			CopyBitmapSectionDirect(im, rescaleBits, 0, 0, 0, 0, im.width, im.height);
			if (im.width < rescaleBits.width)
				CopyBitmapSectionDirect(im, rescaleBits, im.width-1, 0, im.width, 0, 1, im.height);
			if (im.height < rescaleBits.height)
				CopyBitmapSectionDirect(im, rescaleBits, 0, im.height-1, 0, im.height, im.width, 1);
			if (im.height < rescaleBits.height && im.width < rescaleBits.width)
				CopyBitmapSectionDirect(im, rescaleBits, im.width-1, im.height-1, im.width, im.height, 1, 1);

			if (outS) *outS = (float) im.width / (float) rescaleBits.width;
			if (outT) *outT = (float) im.height / (float) rescaleBits.height;
		}
	} else {
		if (outS) *outS = 1.0;
		if (outT) *outT = 1.0;
	}


	if (outWidth) *outWidth = useIt->width;
	if (outHeight) *outHeight = useIt->height;


	glBindTexture(GL_TEXTURE_2D, inTexNum);

	p = useIt->data;
	count = useIt->width * useIt->height;
	if(useIt->channels > 2)
	while (count--)
	{
		swap(p[0], p[2]);						// Ben says: since we get BGR or BGRA, swap red and blue channesl to make RGB or RGBA.  Some day we could
		p += useIt->channels;					// use our brains and use GL_BGR_EXT and GL_BGRA_EXT; literally all GL cards from Radeon/GeForce on support this.
	}

	int							glformat = GL_RGB;
	if (useIt->channels == 4)	glformat = GL_RGBA;
	if (useIt->channels == 1)	glformat = GL_ALPHA;

	if (inFlags & tex_Mipmap)
		gluBuild2DMipmaps(GL_TEXTURE_2D, glformat, useIt->width, useIt->height, glformat, GL_UNSIGNED_BYTE, useIt->data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, glformat,
			useIt->width ,useIt->height, 0,
			glformat,
			GL_UNSIGNED_BYTE,
			useIt->data);

	ok = true;

	if (resize)
		DestroyBitmap(&rescaleBits);

	if (ok)
	{
		// BAS note: for some reason on my WinXP system with GF-FX, if
		// I do not set these explicitly to linear, I get no drawing at all.
		// Who knows what default state the card is in. :-(
		if(inFlags & tex_Nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else if (inFlags & tex_Linear) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
		}

		static const char * ver_str = (const char *) glGetString(GL_VERSION);
		static const char * ext_str = (const char *) glGetString(GL_EXTENSIONS);

		static bool tex_clamp_avail =
			strstr(ext_str,"GL_SGI_texture_edge_clamp"		) ||
			strstr(ext_str,"GL_SGIS_texture_edge_clamp"		) ||
			strstr(ext_str,"GL_ARB_texture_edge_clamp"		) ||
			strstr(ext_str,"GL_EXT_texture_edge_clamp"		) ||
			strncmp(ver_str,"1.2", 3) ||
			strncmp(ver_str,"1.3", 3) ||
			strncmp(ver_str,"1.4", 3) ||
			strncmp(ver_str,"1.5", 3);


			 if(inFlags & tex_Wrap){glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT		 );
								    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT		 );}
// Janos says: why not on windows? without it the terraserver overlay looks ...well, not clamped :-)
//#if !IBM
		else if(tex_clamp_avail)   {glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
								    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}
//#endif
		else					   {glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP		 );
								    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP		 );}

	}
	return ok;
}

#if BIG
	#if APL
		#if defined(__MACH__)
			#include <libkern/OSByteOrder.h>
			#define SWAP32(x) (OSSwapConstInt32(x))
		#else
			#include <Endian.h>
			#define SWAP32(x) (Endian32_Swap(x))
		#endif
	#else
		#error we do not have big endian support on non-Mac platforms
	#endif
#elif LIL
	#define SWAP32(x) (x)
#else
	#error BIG or LIL are not defined - what endian are we?
#endif


#if 0
bool	LoadTextureFromDDS(
				const unsigned char *	mem_start,
				const unsigned char *	mem_end,
				int						in_tex_num,
				int					in_flags,
				int *					outWidth,
				int *					outHeight)
{
	if((mem_end - mem_start) < sizeof(TEX_dds_desc)) return false;

	const TEX_dds_desc * desc = (const TEX_dds_desc *) mem_start;

	if (desc->dwMagic[0] != 'D' ||
		desc->dwMagic[1] != 'D' ||
		desc->dwMagic[2] != 'S' ||
		desc->dwMagic[3] != ' ') return false;

	if((SWAP32(desc->dwSize)) != (sizeof(*desc) - sizeof(desc->dwMagic))) return false;

	if(desc->ddpfPixelFormat.dwFourCC[0] != 'D' ||
	   desc->ddpfPixelFormat.dwFourCC[1] != 'X' ||
	   desc->ddpfPixelFormat.dwFourCC[2] != 'T') return false;

	GLenum format = 0;
	int	flags = 0;
	switch(desc->ddpfPixelFormat.dwFourCC[3]) {
	case '1':		flags = squish::kDxt1;			format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;	break;
	case '3':		flags = squish::kDxt3;			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;	break;
	case '5':		flags = squish::kDxt5;			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;	break;
	default: return false;
	}

	int mips = 0;
	if((SWAP32(desc->dwFlags)) & DDSD_MIPMAPCOUNT)
		mips = SWAP32(desc->dwMipMapCount);
	int has_mips = (mips > 1);
	int x = SWAP32(desc->dwWidth);
	int y = SWAP32(desc->dwHeight);

	if (outWidth) *outWidth = x;
	if (outHeight) *outHeight = y;

	const unsigned char * data = mem_start + sizeof(TEX_dds_desc);

	glBindTexture(GL_TEXTURE_2D, in_tex_num);

	int level = 0;
	do {
		int data_len = squish::GetStorageRequirements(x,y,flags);
		if((data + data_len) > mem_end) return false;

		glCompressedTexImage2DARB(
			GL_TEXTURE_2D,
			level,
			format,
			x,
			y,
			0,
			data_len,
			data);

		if (x==1 && y == 1)	break;
		++level;
		if (x > 1) x >>= 1;
		if (y > 1) y >>= 1;
		--mips;
		if(mips<=0) break;
		data += data_len;
	} while (1);

	if(inFlags & tex_Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	} else if (inFlags & tex_Linear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (in_flags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (in_flags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	}

	static const char * ver_str = (const char *) glGetString(GL_VERSION);
	static const char * ext_str = (const char *) glGetString(GL_EXTENSIONS);

	static bool tex_clamp_avail =
		strstr(ext_str,"GL_SGI_texture_edge_clamp"		) ||
		strstr(ext_str,"GL_SGIS_texture_edge_clamp"		) ||
		strstr(ext_str,"GL_ARB_texture_edge_clamp"		) ||
		strstr(ext_str,"GL_EXT_texture_edge_clamp"		) ||
		strncmp(ver_str,"1.2", 3) ||
		strncmp(ver_str,"1.3", 3) ||
		strncmp(ver_str,"1.4", 3) ||
		strncmp(ver_str,"1.5", 3);


		 if(in_flags & tex_Wrap){glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT		 );
								glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT		 );}
#if !IBM
	else if(tex_clamp_avail)   {glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);}
#endif
	else					   {glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP		 );
								glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP		 );}

	return true;
}
#endif
