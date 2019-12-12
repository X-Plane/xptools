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
#include "AssertUtils.h"
#include "BitmapUtils.h"
#include "MemFileUtils.h"
#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif
#include "squish.h"

#if IBM
// Ben says - this sucks!
#include "XWinGL.h"
#endif

struct  gl_info_t {
	int		gl_major_version;
	bool	has_tex_compression;
	bool	has_non_pots;
	bool	has_bgra;
	int		max_tex_size;
};

static gl_info_t gl_info = { 0 };

#define INIT_GL_INFO		if(gl_info.gl_major_version == 0) init_gl_info(&gl_info);

static void init_gl_info(gl_info_t * i)
{
	const char * ver_str = (const char *) glGetString(GL_VERSION);
	const char * ext_str = (const char *) glGetString(GL_EXTENSIONS);
	
	sscanf(ver_str,"%d", &i->gl_major_version);
	if(i->gl_major_version < 3)                                                        // Need the framebuffer object for the DDS loader, flipping the image
		AssertPrintf("OpenGL 3.0 or higher required. GL_VERSION = '%s'\n", ver_str);
	
	i->has_tex_compression = strstr(ext_str,"GL_ARB_texture_compression") != NULL;
	i->has_non_pots = strstr(ext_str,"GL_ARB_texture_non_power_of_two") != NULL;
	i->has_bgra = strstr(ext_str,"GL_EXT_bgra") != NULL;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&i->max_tex_size);
	if(i->max_tex_size > 8192)	i->max_tex_size = 8192;
	if(i->has_tex_compression)	glHint(GL_TEXTURE_COMPRESSION_HINT,GL_NICEST);
}

/*****************************************************************************************
 * UTILS
 *****************************************************************************************/


inline int NextPowerOf2(int a)
{
	GLint	maxDim = gl_info.max_tex_size;
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
	struct ImageInfo	im = { 0 };

	MFMemFile * mf = MemFile_Open(inFileName);
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
	else
	{
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
	}
	return false;
}

/*****************************************************************************************
 * LoadTextureFromImage
 *****************************************************************************************/
bool LoadTextureFromImage(ImageInfo& im, int inTexNum, int inFlags, int * outWidth, int * outHeight, float * outS, float * outT)
{
	INIT_GL_INFO

	// Process alpha.  Then remove padding.  Finally, figure out the next biggest power of 2.  If we aren't
	// a power of 2, we need to resize.  That will be done with rescaling if the user wants.  Also if the bitmap
	// is bigger than the max power of 2 supported by the HW, force rescaling.
	if (inFlags & tex_MagentaAlpha)	ConvertBitmapToAlpha(&im, true);
	if (im.pad != 0)
 		UnpadImage(&im);

	int non_pots = ((inFlags & tex_Always_Pad) == 0) && gl_info.has_non_pots;

	int		res_x = non_pots ? min((int) im.width, gl_info.max_tex_size) : NextPowerOf2(im.width);
	int		res_y = non_pots ? min((int) im.height,gl_info.max_tex_size) : NextPowerOf2(im.height);
	bool	resize = (res_x != im.width || res_y != im.height);
	bool	rescale = resize && (inFlags & tex_Rescale);
	if (im.width > res_x || im.height > res_y)	// Always rescale if the image is too big for the max tex!!
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
	
	int	iformat, glformat;
	if (useIt->channels == 1)
	{
		iformat = glformat = GL_ALPHA;
	}
	else if(gl_info.has_bgra)
	{
		                            iformat = GL_RGB;  glformat = GL_BGR;
		if (useIt->channels == 4) { iformat = GL_RGBA; glformat = GL_BGRA; }
	}
	else
	{
		long cnt = useIt->width * useIt->height;
		unsigned char * p = useIt->data;
		while (cnt--)
		{
			swap(p[0], p[2]);						// Ben says: since we get BGR or BGRA, swap red and blue channesl to make RGB or RGBA.  Some day we could
			p += useIt->channels;					// use our brains and use GL_BGR_EXT and GL_BGRA_EXT; literally all GL cards from Radeon/GeForce on support this.
			}
													// Michael says: that day was the last day of 2018. Eight years, 4 months and EXA bytes swapped by CPUs later.
													// But, its just a drop into the ocean: ALL images except BMP are channel order swapped when loading in BitmapUtils,
													// the real architectural misfortune was choosing the ImageInfo data format to be BGR rather than RGB.
													
								  iformat = glformat = GL_RGB;
		if (useIt->channels == 4) iformat = glformat = GL_RGBA;
	}

	if(gl_info.has_tex_compression && (inFlags & tex_Compress_Ok))
	{
		switch (iformat) {
		case GL_RGB:	iformat = GL_COMPRESSED_RGB;	break;
		case GL_RGBA:	iformat = GL_COMPRESSED_RGBA;	break;
		}
	}

	if (inFlags & tex_Mipmap)
		gluBuild2DMipmaps(GL_TEXTURE_2D, iformat, useIt->width, useIt->height, glformat, GL_UNSIGNED_BYTE, useIt->data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, iformat, useIt->width ,useIt->height, 0,	glformat, GL_UNSIGNED_BYTE, useIt->data);

	if (resize)
		DestroyBitmap(&rescaleBits);

	// BAS note: for some reason on my WinXP system with GF-FX, if
	// I do not set these explicitly to linear, I get no drawing at all.
	// Who knows what default state the card is in. :-(
//	if(inFlags & tex_Nearest)
//	{
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	} else 
	if (inFlags & tex_Linear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	} else {
		// If we have nearest-neighboring and we are down-sampling WITHOUT a mip-map we STILL use linear in an attempt to keep this thing from looking TOTALY blitzed, I guess?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	}
	if(inFlags & tex_Wrap) {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT );
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT );
	}
	else { 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	}
	return true;
}

#if 1

#define SWAP32(x) (x)

bool	LoadTextureFromDDS(
				const unsigned char *	mem_start,
				const unsigned char *	mem_end,
				int						in_tex_num,
				int						inFlags,
				int *					outWidth,
				int *					outHeight)
{
	INIT_GL_INFO
	
	if((mem_end - mem_start) < sizeof(TEX_dds_desc)) return false;

	const TEX_dds_desc * desc = (const TEX_dds_desc *) mem_start;
	
	if (strncmp(desc->dwMagic, "DDS ", 4) != 0) return false;
	if((SWAP32(desc->dwSize)) != (sizeof(*desc) - sizeof(desc->dwMagic))) return false;
	if(strncmp(desc->ddpfPixelFormat.dwFourCC, "DXT",3) != 0 ) return false;

	GLenum glformat = 0;
	int	flags = 0;
	switch(desc->ddpfPixelFormat.dwFourCC[3]) 
	{
		case '1':	flags = squish::kDxt1; glformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;	break;
		case '3':	flags = squish::kDxt3; glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;	break;
		case '5':	flags = squish::kDxt5; glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;	break;
		default: return false;
	}
	int mips = 0;
	if((SWAP32(desc->dwFlags)) & DDSD_MIPMAPCOUNT)
		mips = SWAP32(desc->dwMipMapCount);
	int x = SWAP32(desc->dwWidth);
	int y = SWAP32(desc->dwHeight);

	if (outWidth) *outWidth = x;
	if (outHeight) *outHeight = y;

	const unsigned char * data = mem_start + sizeof(TEX_dds_desc);
#if 0
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
#else
	glBindTexture(GL_TEXTURE_2D, in_tex_num);
#endif
	for (int level = 0; level <= mips; ++level) 
	{
		int data_len = squish::GetStorageRequirements(x,y,flags);
		if((data + data_len) > mem_end) return false;        // not enough data for mipmaps = broken dds !

		glCompressedTexImage2D( GL_TEXTURE_2D, level, glformat, x, y, 0, data_len, data);
		

		if (x < 4 && y < 4) break;  // don't care about loading all mipmaps, just enough
		x >>= 1;
		y >>= 1;
		data += data_len;
	}
#if 0
	// here is the mess: DXT images are upside down, as XP/WED had choosen the origin of textures to the left top, while
	// OpenGl has them in the right bottom. So its either flipping all UV coorcinates all thoughout WED and all tools that
	// that also use TexUtils (ObjView, RenderFarmUI) or flip the texture on the GPU before use. Which means uncompress
	// and recompress, but now on the GPU rather than with lib_squish.
	
	x = SWAP32(desc->dwWidth);
	y = SWAP32(desc->dwHeight);
	
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	
	glBindTexture(GL_TEXTURE_2D, in_tex_num);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, in_tex_num), 0);
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0,0,x, y); 

	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2i(0, 0);
		glTexCoord2f(0, 1); glVertex2i(0, y);
		glTexCoord2f(1, 1); glVertex2i(x, y);
		glTexCoord2f(1, 0); glVertex2i(x, 0);
	glEnd();

//	glDeleteFramebuffers(GL_FRAMEBUFFER, FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, in_tex_num);
#endif

	if (inFlags & tex_Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	} 
	else 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	}

	if(1) // inFlags & tex_Wrap)          // why is that needed ???
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT );
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	}
	return true;
}
#endif
