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

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
		#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
	#endif
	#ifndef GL_COMPRESSED_LUMINANCE_LATC1_EXT
		#define GL_COMPRESSED_LUMINANCE_LATC1_EXT 0x8C70
	#endif
#else
	#include "glew.h"
	#include <GL/glu.h>
#endif


struct  gl_info_t {
	int		gl_major_version;
	bool	has_tex_compression;
	bool	has_non_pots;
	bool	has_bgra;
	int		max_tex_size;
	bool	has_rgtc;
	bool	has_bptc;
};

static gl_info_t gl_info = { 0 };

#define INIT_GL_INFO		if(gl_info.gl_major_version == 0) init_gl_info(&gl_info);

static void init_gl_info(gl_info_t * i)
{
	CHECK_GL_ERR
	const char * ver_str = (const char *)glGetString(GL_VERSION);      CHECK_GL_ERR
	const char * ext_str = (const char *)glGetString(GL_EXTENSIONS);   CHECK_GL_ERR

	sscanf(ver_str,"%d", &i->gl_major_version);
#if APL
/* Apple only gives by default a 2.1 compatible openGL contexts.
   BUT if openGL 3.2 or higher compatible contexts are requested by adding
   "NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core," just as the textbooks
   say - all openGl 1/2 direct drawing is disbled. i.e. most openGl functions used
   by WED are unavailable. So we can only test for openGL 2 core functionality available 
   here and pray glew gets us the rest ...
*/
	if(i->gl_major_version < 2)
	{
		LOG_MSG("OpenGL 2.0 or higher required. Found version '%s'\n", ver_str);
		AssertPrintf("OpenGL 2.0 or higher required. Found version '%s'\n", ver_str);
	}
#else
	if(i->gl_major_version < 3)
	{
		LOG_MSG("OpenGL 3.0 or higher required. Found version '%s'\n", ver_str);
		AssertPrintf("OpenGL 3.0 or higher required. Found version '%s'\n", ver_str);
	}
#endif
	i->has_tex_compression = strstr(ext_str,"GL_ARB_texture_compression") != NULL;
	i->has_non_pots = strstr(ext_str,"GL_ARB_texture_non_power_of_two") != NULL;
	i->has_bgra = strstr(ext_str,"GL_EXT_bgra") != NULL;
	i->has_rgtc = strstr(ext_str,"GL_ARB_texture_compression_rgtc") != NULL;
	i->has_bptc = strstr(ext_str,"GL_ARB_texture_compression_bptc") != NULL;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&i->max_tex_size);
	// if(i->max_tex_size > 2*8192)	i->max_tex_size = 2*8192;
	if(i->has_tex_compression)	glHint(GL_TEXTURE_COMPRESSION_HINT,GL_NICEST); CHECK_GL_ERR
	LOG_MSG("OpenGL renderer  : %s\n", glGetString(GL_RENDERER));              CHECK_GL_ERR
	LOG_MSG("OpenGL Version   : %s\n", ver_str);
	LOG_MSG("Max texture size : %5d  DXT : %d   POT : %d\n", i->max_tex_size, i->has_tex_compression, i->has_non_pots);
	LOG_MSG("                         RGTC : %d  BPTC : %d \n", i->has_rgtc, i->has_bptc);
	LOG_MSG("                          FBO : %d   VBO : %d  MSext : %d\n", strstr(ext_str, "GL_ARB_framebuffer_object") != nullptr,
											strstr(ext_str, "GL_ARB_vertex_buffer_object") != nullptr,
	                                        strstr(ext_str, "GL_EXT_framebuffer_multisample") != nullptr);
	LOG_FLUSH();
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
	if (LoadBitmapFromAnyFile(inFileName, &im) == 0)
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
	INIT_GL_INFO

	// Process alpha.  Then remove padding.  Finally, figure out the next biggest power of 2.  If we aren't
	// a power of 2, we may need to resize.  That will be done with rescaling if the user wants.  Also if the bitmap
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

	glBindTexture(GL_TEXTURE_2D, inTexNum);  CHECK_GL_ERR
	
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
		gluBuild2DMipmaps(GL_TEXTURE_2D, iformat, useIt->width, useIt->height, glformat, GL_UNSIGNED_BYTE, useIt->data); CHECK_GL_ERR
	else
		glTexImage2D(GL_TEXTURE_2D, 0, iformat, useIt->width ,useIt->height, 0,	glformat, GL_UNSIGNED_BYTE, useIt->data); CHECK_GL_ERR

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	return true;
}

#if LOAD_DDS_DIRECT

struct DXT1Block {
    uint16_t colors[2];
    uint8_t  rows[4];
};

struct DXT3Block {
    DXT1Block	alphas;
    DXT1Block 	colors;
};

struct DXT5AlphaBlock {
    uint8_t 		alpha[2];
    uint8_t			idx[6];
};

struct DXT5Block {
    DXT5AlphaBlock	alphas;
    DXT1Block		colors;
};

// ***** y-flipping of BC1/2/3/4/5 format selector arrays ******

inline void swap_12bit_idx(uint8_t * a)
{
	uint64_t src = *(uint64_t *) a;
	uint64_t dst = src & 0xFFFF'0000'0000'0000ull;

	dst |= (src & 0xFFFull) << 36 | (src & 0xFFF000ull) << 12 | (src & 0xFFF000000ull) >> 12 | (src & 0xFFF000000000ull) >>36;

	*(uint64_t *) a = dst;
}

// ****** swapping of BC7 format color endpoints ******
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format-mode-reference#mode-3

static void swap_bc7m1_sets(char * a)
{
	uint32_t src, dst;

	for(int i = 0; i < 3; i++)
	{
		src = *(uint32_t*) (a + 1 + 3 * i);      // array starts at 8 bit offset = 1 byte
		dst = src & 0xFF00'0000u;
		
		dst |= (src & 0xFFFu   ) << 12;
		dst |= (src & 0xFFF000u) >> 12;
		if(i == 2)                               // one p-bit per set, shared for both endpoints
		{
			dst &= ~0x0300'0000u;
			dst |= (src & 0x0100'0000u) << 1 | (src & 0x0200'0000u) >> 1;
		}
		*(uint32_t*) (a + 1 + 3 * i) = dst;
	}
}

static void swap_bc7m1_endpts(char * a, bool first, bool second)
{
	uint32_t src, dst;

	for(int i = 0; i < 3; i++)
	{
		src = *(uint32_t*) (a + 1 + 3 * i);       // array starts at 8 bit offset = 1 byte
		dst = src & 0xFF00'0000u;

		if(first)  dst |= (src & 0x03F) << 6 | (src & 0xFC0) >> 6;
		else       dst |=  src & 0xFFF;
		
		if(second) dst |= (src & 0x03F000u) << 6 | (src & 0xFC0000u) >> 6;
		else       dst |=  src & 0xFFF000u;
		
		*(uint32_t*) (a + 1 + 3 * i) = dst;
	}
}

static void swap_bc7m3_sets(char * a)
{
	uint64_t src, dst;
	
	src = *(uint64_t*) (a + 1);                                // array starts at 10 bit offset - 1 byte = 2 bits
	dst  =  src & 0xFC00'0000'0000'0003ull;
	dst |= (src & (0xFFFCull       | 0xFFFCull << 28)) << 14;  // red & green
	dst |= (src & (0xFFFCull << 14 | 0xFFFCull << 42)) >> 14;
	*(uint64_t*) (a + 1) = dst;

	src = *(uint64_t*) (a + 8);                                // offset 10 + 8 * 7 = 66 bis - 8 byte = 2 bits
	dst  =  src & 0xFFFF'FFFC'0000'0003ull;
	dst |= (src & 0xFFFCull      ) << 14;                      // blue
	dst |= (src & 0xFFFCull << 14) >> 14;
	dst |= (src & 0x000Cull << 14) << 2 | (src & 0x0030ull << 14) >> 2;   // p-bits
	*(uint64_t*) (a + 8) = dst;
}

static void swap_bc7m3_endpts(char * a, bool first, bool second)
{
	uint64_t src, dst;
	
	src = *(uint64_t*) (a + 1);
	dst  =  src & 0xFC00'0000'0000'0003ull;
	if(first) { dst |= (src & (0x01FCull        | 0x01FCull << 28)) << 7;
	            dst |= (src & (0xFE00ull        | 0xFE00ull << 28)) >> 7; }
	else        dst |=  src & (0xFFFCull        | 0xFFFCull << 28);
	
	if(second) { dst |= (src & (0x01FCull << 14 | 0x01FCull << 42)) << 7;
	             dst |= (src & (0xFE00ull << 14 | 0xFE00ull << 42)) >> 7; }
	else         dst |=  src & (0xFFFCull << 14 | 0xFFFCull << 42);
	*(uint64_t*) (a + 1) = dst;

	src = *(uint64_t*) (a + 8);
	dst  =  src & 0xFFFF'FFFF'C000'0003ull;
	if(first) { dst |= (src & 0x01FCull) << 7;
	            dst |= (src & 0xFE00ull) >> 7; }
	else        dst |=  src & 0xFFFCull;                         // todo: pbits
	
	if(second) { dst |= (src & 0x01FCull << 14) << 7;
	             dst |= (src & 0xFE00ull << 14) >> 7; }
	else         dst |=  src & 0xFFFCull << 14;                  // todo: pbits
	*(uint64_t*) (a + 8) = dst;
}

static void swap_bc7m4_endpts(char * a, bool swap_col, bool swap_a)
{
	uint32_t src, dst;
	
	bool mode = *(uint8_t *) a & 0x08;
	int rot = *(uint8_t *) a >> 5 & 3;   // todo: do these affect which component to swap ?
	
	if(mode ? swap_a : swap_col) 
	{
		src = *(uint32_t*) (a+1);                // array starts at 8 bit offset = 1 byte
		dst = src & 0xC000'0000u;
		dst |= (src & (0x01Fu | 0x01Fu << 10 | 0x01Fu << 20)) << 5;
		dst |= (src & (0x3E0u | 0x3E0u << 10 | 0x3E0u << 20)) >> 5;
		*(uint32_t*) (a+1) = dst;
	}
	
	if(mode ? swap_col : swap_a) 
	{
		src = *(uint32_t*) (a+4);                // array starts at 8 + 6 * 5 bit offset = 4 byte + 6 bits
		dst = src & 0xFFFC'003Fu;
		dst |= (src & 0x00FC0u) << 6 | (src & 0x3F000u) >> 6;
		*(uint32_t*) (a+4) = dst;
	}
}

static void swap_bc7m5_endpts(char * a)
{
	uint64_t src = *(uint64_t*) (a+1);      // array starts at 8 bit offset = 1 byte
	uint64_t dst = src & 0xFFFF'FC00'0000'0000ull;
	for(int i = 0; i < 3; i++)
	{
		dst |= (src & (0x7Full << (    14 * i))) << 7;
		dst |= (src & (0x7Full << (7 + 14 * i))) >> 7;
	}
	*(uint64_t*) (a+1) = dst;
}

static void swap_bc7m5_a_endpts(char * a)
{
	uint64_t src, dst;
	
	src = *(uint64_t*) (a+1);               // array starts at 8 + 6 * 7 bit offset = 1 byte + 42 bits
	dst = src & 0xFC00'03FF'FFFF'FFFFull;
	dst |= (src & (0xFFull << (    14 * 3))) << 8;
	dst |= (src & (0xFFull << (8 + 14 * 3))) >> 8;
	*(uint64_t*) (a+1) = dst;
}

static void swap_bc7m6_endpts(char * a)
{
	uint64_t src, dst;
	
	src = *(uint64_t*) (a);                 // array starts at 7 bit offset
	dst = src & 0x800'0000'0000'0007Full;
	for(int i = 0; i < 4; i++)
	{
		dst |= (src & (0x7Full << ( 7 + 14 * i))) << 7;
		dst |= (src & (0x7Full << (14 + 14 * i))) >> 7;
	}
	*(uint64_t*) (a) = dst;
}

// ***** y-flipping of BC7 format selector arrays ******

static void swap_63bit_idx(char * a)
{
	uint64_t src = *(uint64_t *) (a+8);
	uint64_t dst, tmp;

	bool lsb = src & 1;                  // that bit isn't part of our index table at all, save it for restoration later

	tmp = src & 0x0E;                    // that implicit zero - its omitted and the last 3 LSB of the table are shifted up. Undo that by
	src &= ~0x0Full;                     // inserting the implicit 0 where it belongs, as this idx will after the swap be a "normal" index
	src |= tmp >> 1;

	dst = (src & 0xFFFFull) << 48 | (src & 0xFFFF'0000ull) << 16 | (src & 0xFFFF'0000'0000ull) >> 16 | (src & 0xFFFF'0000'0000'0000ull) >> 48;

	bool need_swap = dst & 0x08;         // Now another idx got into the first position and that idx may have a non-zreo MSB. To fix that,
										 // reverse/invert all IDX and swap endpoints to get the effectively same thing.
	if(need_swap) dst = ~dst;

	tmp = dst & 0x07;                    // shift things up to propperly omit the implicit zero again
	dst &= ~0x0Full;
	dst |= tmp << 1;

 	if(lsb) dst |= 1;                    // restore the LSB to whatever it was before - now that we have shifted to omit the implicit MSB again

	*(uint64_t *) (a+8) = dst;
	if(need_swap) swap_bc7m6_endpts(a);
}

static void swap_31b31b_idx(char * a)
{
	uint64_t src = *(uint64_t *) (a+8);
	uint32_t src_c = (uint32_t) (src >> 1);
	uint32_t src_a = (uint32_t) (src >> 32);
	uint32_t dst_c, dst_a, tmp;
	
	tmp = src_c & 0x03;                // that implicit zero - its omitted and the LSB of the first idx in the table is shifted up. Undo that by
	src_c &= ~0x03u;                   // inserting the implicit 0 where it belongs, as this idx will after the swap be a "normal" index
	src_c |= tmp >> 1;

	tmp = src_a & 0x03;
	src_a &= ~0x03u;
	src_a |= tmp >> 1;

	dst_c = (src_c & 0xFFu) << 24 | (src_c & 0xFF00u) << 8 | (src_c & 0xFF0000u) >> 8 | (src_c & 0xFF000000u) >> 24;
	dst_a = (src_a & 0xFFu) << 24 | (src_a & 0xFF00u) << 8 | (src_a & 0xFF0000u) >> 8 | (src_a & 0xFF000000u) >> 24;

	bool swap_col = dst_c & 0x02;     // Now another idx got into the first position and that idx may have a non-zreo MSB. To fix that,
	if(swap_col) dst_c = ~dst_c;	   // reverse/invert all IDX and swap endpoints to get the effectively same thing.
	bool swap_a = dst_a & 0x02;
	if(swap_a) dst_a = ~dst_a;

	tmp = dst_c & 0x01;                // shift things up to propperly omit the implicit zero again
	dst_c &= ~0x03u;
	dst_c |= tmp << 1;

	tmp = dst_a & 0x01;
	dst_a &= ~0x03u;
	dst_a |= tmp << 1;

	uint64_t dst = ((uint64_t) dst_a << 32) | ((uint64_t) dst_c << 1);

 	dst |= src & 3;                   // restore the two LSB to whatever it was before - now that we have shifted to omit the implicit MSB again

	*(uint64_t *) (a+8) = dst;
	if(swap_col) swap_bc7m5_endpts(a);
	if(swap_a)   swap_bc7m5_a_endpts(a);
}

static void swap_31b47b_idx(char * a)
{
	uint64_t tmp   = *(uint64_t *) (a+6); 
	uint32_t src_c = (uint32_t) (tmp >> 1);
	uint64_t src_a = *(uint64_t *) (a+10);
	uint32_t dst_c;
	uint64_t dst_a = src_a & 0xFFFF'0000'0000'0000ull;
	
	tmp = src_c & 0x03;                // that implicit zero - its omitted and the LSB of the first idx in the table is shifted up. Undo that by
	src_c &= ~0x03u;                   // inserting the implicit 0 where it belongs, as this idx will after the swap be a "normal" index
	src_c |= tmp >> 1;

	tmp = src_a & 0x07;
	src_a &= ~0x07ull;
	src_a |= tmp >> 1;

	dst_c  = (src_c & 0xFFu) << 24 | (src_c & 0xFF00u) << 8 | (src_c & 0xFF0000u) >> 8 | (src_c & 0xFF000000u) >> 24;
	dst_a |= (src_a & 0xFFFull) << 36 | (src_a & 0xFFF000ull) << 12 | (src_a & 0xFFF00'0000ull) >> 12 | (src_a & 0xFFF0'0000'0000ull) >> 36;

	bool swap_col = dst_c & 0x02;     // Now another idx got into the first position and that idx may have a non-zreo MSB. To fix that,
	if(swap_col) dst_c = ~dst_c;	   // reverse/invert all IDX and swap endpoints to get the effectively same thing.
	bool swap_a = dst_a & 0x04;
	if(swap_a) dst_a ^= 0xFFFF'FFFF'FFFFull;
	
	tmp = dst_c & 0x01;                // shift things up to propperly omit the implicit zero again
	dst_c &= ~0x03u;
	dst_c |= tmp << 1;

	tmp = dst_a & 0x03;
	dst_a &= ~0x07ull;
	dst_a |= tmp << 1;
	
	if( (bool) (dst_c & 0x8000'0000u) != (bool) (dst_a & 1)) dst_a ^= 1;     // copy the MSB of dst_c the LSB of dst_a
	*(uint64_t *) (a+10) = dst_a;
	
	dst_c <<= 1;                        // no worries - the MSB isn't needed anymore, see above
	tmp = *(uint32_t *) (a+6) & 3;     // restore the two LSB to whatever it was before - now that we have shifted to omit the implicit MSB again
	dst_c |= tmp;
	*(uint32_t *) (a+6) = dst_c;

	swap_bc7m4_endpts(a, swap_col, swap_a);
}

#define B(p,o,n,m,l,k,j,i,h,g,f,e,d,c,b,a) 0b ## a ## a ## a ## b ## b ## b ## c ## c ## c ## d ## d ## d \
                                              ## e ## e ## e ## f ## f ## f ## g ## g ## g ## h ## h ## h \
                                              ## i ## i ## i ## j ## j ## j ## k ## k ## k ## l ## l ## l \
                                              ## m ## m ## m ## n ## n ## n ## o ## o ## o ## p ## p ## p

// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
// Table.P2 (two-subset partitioning pattern)
static uint64_t bc7_part3bit[64] = {
  B(0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1),  B(0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1),  B(0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1),  B(0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1),
  B(0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1),  B(0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1),
  B(0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1),
  B(0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),  B(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1),
  B(0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1),  B(0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0),  B(0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0),  B(0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0),
  B(0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0),  B(0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0),  B(0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0),  B(0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1),
  B(0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0),  B(0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0),  B(0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0),  B(0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0),
  B(0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0),  B(0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0),  B(0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0),  B(0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0),
  B(0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1),  B(0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1),  B(0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0),  B(0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0),
  B(0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0),  B(0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0),  B(0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1),  B(0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1),
  B(0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0),  B(0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0),  B(0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0),  B(0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0),
  B(0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0),  B(0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1),  B(0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1),  B(0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0),
  B(0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0),  B(0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0),  B(0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0),  B(0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0),
  B(0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1),  B(0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0),  B(0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0),
  B(0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1),  B(0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1),  B(0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1),  B(0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1),
  B(0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1),  B(0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0),  B(0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0),  B(0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1) };

#undef B
#define B(p,o,n,m,l,k,j,i,h,g,f,e,d,c,b,a) 0b ## a ## a ## b ## b ## c ## c ## d ## d \
                                              ## e ## e ## f ## f ## g ## g ## h ## h \
                                              ## i ## i ## j ## j ## k ## k ## l ## l \
                                              ## m ## m ## n ## n ## o ## o ## p ## p

// same table again, but this time expanded to just double every bit - for 2-bit selector inversions
static uint32_t bc7_part2bit[64] = {
  B(0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1),  B(0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1),  B(0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1),  B(0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1),
  B(0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1),  B(0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1),
  B(0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1),
  B(0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),  B(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),  B(0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1),
  B(0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1),  B(0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0),  B(0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0),  B(0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0),
  B(0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0),  B(0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0),  B(0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0),  B(0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1),
  B(0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0),  B(0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0),  B(0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0),  B(0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0),
  B(0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0),  B(0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0),  B(0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0),  B(0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0),
  B(0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1),  B(0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1),  B(0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0),  B(0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0),
  B(0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0),  B(0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0),  B(0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1),  B(0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1),
  B(0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0),  B(0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0),  B(0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0),  B(0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0),
  B(0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0),  B(0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1),  B(0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1),  B(0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0),
  B(0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0),  B(0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0),  B(0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0),  B(0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0),
  B(0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1),  B(0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1),  B(0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0),  B(0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0),
  B(0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1),  B(0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1),  B(0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1),  B(0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1),
  B(0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1),  B(0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0),  B(0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0),  B(0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1) };

#undef B

// Table.A2 (Anchor index values for the second subset of two-subset partitioning)
static char bc7_anchor_idx[64] = {
	15,15,15,15,15,15,15,15,      
    15,15,15,15,15,15,15,15,
    15, 2, 8, 2, 2, 8, 8,15,
     2, 8, 2, 2, 8, 8, 2, 2,
    15,15, 6, 8, 2, 8,15,15,
     2, 8, 2, 2, 2,15,15, 6,
     6, 2, 6, 8,15,15, 2, 2,
    15,15,15,15,15, 2, 2,15  };
    
// From stareing long enough at https://rockets2000.wordpress.com/2015/05/19/bc7-partitions-subsets/ one can figure 
// for each subset partition a matching one that is an exact vertically flipped equivalent.
// Except for the last 2 partitions 62+63, those don't have one. So there will be a (slight) inaccuracy for these.
static uint8_t bc7_part_equiv[64] = {
//   0       1       2       3       4       5       6       7 
     0,      1,      2,     23,     24,     25+64,  21+64,  19,
//   8       9      10      11      12      13      14      15
    20,     22+64,  16+64,  17,     18+64,  13+64,  15+64,  14+64,
//  16      17      18      19      20      21      22      23
	10+64,  11,     12+64,   7,      8,      6+64,   9+64,   3,
//  24      25      26      27      28      29      30      31
	 4,      5+64,  26,     31+64,  30+64,  29,     28+64,  27+64,
//  32      33      34      35      36      37      38      39
	32,     33+64,  34+64,  35+64,  36+64,  37+64,  38+64,  39,
//  40      41      42      43      44      45      46      47
	41+64,  40+64,  43+64,  42+64,  44,     45,     46+64,  47,
//  48      49      50      51      52      53      54      55
	51,     50,     49,     48,     55,     54+64,  53+64,  52,
//  56      57      58      59      60      61      62      63
	57+64,  56+64,  59,     58,     61,     60,     62,     63    };

static void swap_bc7m1_idx(char * a, int part)
{
	uint64_t src = *(uint64_t *) (a+10);
	uint64_t dst, tmp;
	
	int64_t lsb = src & 0xFFFF'0000'0000'0003; // those bits aren't part of our index table at all, save it for restoration later
	
	tmp = src & 0x0F;           // The first idx has an implicit zero - its omitted and the LSBs of that idx in the table are shifted up.
	src &= ~0x0Full;            // Undo that by inserting the implicit 0 where it belongs, so all idx can be swapped in a regular fashion
	src |= tmp >> 1;
	                            // But now we TWO endpoints sets, resulting in another idx with an implicit/supressed zero. But it
								// depends on the partitioning scheme in what position that idx is. So look it up ... BC7 sucks, huh ?
	uint64_t shift_pattern = ~0ull >> (61 - 3 * bc7_anchor_idx[part]);

	tmp = src & shift_pattern;
	src &= ~shift_pattern;
	src |= tmp >> 1;

	bool swapset = bc7_part_equiv[part] & 64;
	int  newpart = bc7_part_equiv[part] & 63;
	if(newpart != part)
	{
		part = newpart;
		*(uint8_t *) a = part << 2 | 2;
		shift_pattern = ~0ull >> (61 - 3 * bc7_anchor_idx[part]);
	}
	dst = (src & 0xFFFull) << 36 | (src & 0xFFF000ull) << 12 | (src & 0xFFF000000ull) >> 12 | (src & 0xFFF000000000ull) >> 36;

	// determine need to swap endpoints + invert all relevant indices because the relevant index has MSB = 1
	bool swap_seg1 = dst & 0x04;
	bool swap_seg2 = dst & 0x04ull << (3 * bc7_anchor_idx[part]);

	uint64_t invert_pattern = (swap_seg2 ? bc7_part3bit[part] : 0) | (swap_seg1 ? (~bc7_part3bit[part] & 0xFFFF'FFFF'FFFFull) : 0);
	dst = dst ^ invert_pattern;
	
	tmp = dst & shift_pattern >> 1;	// shift things up to propperly omit the implicit zeros again
	dst &= ~shift_pattern;
	dst |= tmp << 1;
	            
	tmp = dst & 0x07;
	dst &= ~0x0Full;
	dst |= tmp << 1;
	
 	dst |= lsb;                  // restore the LSB to whatever it was before - now that we have shifted to omit the implicit MSBs again
	*(uint64_t *) (a+10) = dst;
	
	if(swapset) swap_bc7m1_sets(a);
	swap_bc7m1_endpts(a, swap_seg1, swap_seg2);
}

static void swap_bc7m3_idx(char * a, int part) // for comments see above swap_bc7m1_idx()
{
	uint32_t src = *(uint32_t *) (a+12);
	uint32_t dst, tmp;
	
	int lsb = src & 3;
	
	tmp = src & 0x07;
	src &= ~0x07u;
	src |= tmp >> 1;

	uint32_t shift_pattern = ~0u >> (30 - 2 * bc7_anchor_idx[part]);
	tmp = src & shift_pattern;
	src &= ~shift_pattern;
	src |= tmp >> 1;
	
	bool swapset = bc7_part_equiv[part] & 64;
	int  newpart = bc7_part_equiv[part] & 63;
	if(newpart != part)
	{
		part = newpart;
		tmp = *(uint16_t *) a & 0xFC0Fu;
		*(uint16_t *) a = tmp | part << 4;
		shift_pattern = ~0u >> (30 - 2 * bc7_anchor_idx[part]);
	}
	dst = (src & 0xFFu) << 24 | (src & 0xFF00u) << 8 | (src & 0xFF0000u) >> 8 | (src & 0xFF000000u) >> 24;

	bool swap_seg1 = dst & 0x02;
	bool swap_seg2 = dst & 0x02u << (2 * bc7_anchor_idx[part]);

	uint32_t invert_pattern = (swap_seg2 ? bc7_part2bit[part] : 0) | (swap_seg1 ? ~bc7_part2bit[part] : 0);
	dst = dst ^ invert_pattern;
	
	tmp = dst & shift_pattern >> 1;
	dst &= ~shift_pattern;
	dst |= tmp << 1;
	            
	tmp = dst & 0x03;
	dst &= ~0x07u;
	dst |= tmp << 1;
	
 	dst |= lsb;
	*(uint32_t *) (a+12) = dst;
	
	if(swapset) swap_bc7m3_sets(a);
	swap_bc7m3_endpts(a, swap_seg1, swap_seg2);
}

static void swap_bc7_idx(char * a)
{
	int mode;
	for(mode = 0; mode < 8; mode++)
		if(*(unsigned char*)a & (1 << mode)) break;
	switch(mode)
	{
//		case 0: triple segmented partitions
		case 1: swap_bc7m1_idx(a, *(unsigned char*)a >> 2); break;
//		case 2: triple segmented partitions
		case 3: swap_bc7m3_idx(a, *(unsigned short*)a >> 4 & 0x3F); break;
		case 4: swap_31b47b_idx(a); break;
		case 5: swap_31b31b_idx(a); break;
		case 6: swap_63bit_idx(a); break;
//      case 7: similar to mode 3, same 2-bit indices, different 5555+P color endpoints w/equally ugly alignment as mode 3
		default: *a = 0; // make blocks with modes that can't be flipped (yet) transparent
	}
}

static void swap_blocks(char *a, char *b, GLint type)
{
	if (type == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		DXT5Block * x = (DXT5Block *) a;
		DXT5Block * y = (DXT5Block *) b;
		swap(*x,*y);
		swap_12bit_idx(x->alphas.idx);
		swap_12bit_idx(y->alphas.idx);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
		swap(y->colors.rows[0], y->colors.rows[3]);
		swap(y->colors.rows[1], y->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	{
		DXT3Block * x = (DXT3Block *) a;
		DXT3Block * y = (DXT3Block *) b;
		swap(*x,*y);
		swap(x->alphas.rows[0], x->alphas.rows[3]);
		swap(x->alphas.rows[1], x->alphas.rows[2]);
		swap(y->alphas.rows[0], y->alphas.rows[3]);
		swap(y->alphas.rows[1], y->alphas.rows[2]);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
		swap(y->colors.rows[0], y->colors.rows[3]);
		swap(y->colors.rows[1], y->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		DXT1Block * x = (DXT1Block *) a;
		DXT1Block * y = (DXT1Block *) b;
		swap(*x,*y);
		swap(x->rows[0], x->rows[3]);
		swap(x->rows[1], x->rows[2]);
		swap(y->rows[0], y->rows[3]);
		swap(y->rows[1], y->rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_BPTC_UNORM_EXT) // BC7
	{
		DXT3Block * x = (DXT3Block *) a;
		DXT3Block * y = (DXT3Block *) b;
		swap(*x,*y);
		swap_bc7_idx(a);
		swap_bc7_idx(b);
	}
	else // BC4 or BC5
	{
		DXT5AlphaBlock * x = (DXT5AlphaBlock  *)a;
		DXT5AlphaBlock * y = (DXT5AlphaBlock  *)b;
		swap(*x, *y);
		swap_12bit_idx(x->idx);
		swap_12bit_idx(y->idx);
		if (type == GL_COMPRESSED_RG_RGTC2)
		{
			++x; ++y;
			swap(*x, *y);
			swap_12bit_idx(x->idx);
			swap_12bit_idx(y->idx);
		}
	}
}

static void swap_blocks(char *a, GLint type)
{
	if (type == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		DXT5Block * x = (DXT5Block *)a;
		swap_12bit_idx(x->alphas.idx);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	{
		DXT3Block * x = (DXT3Block *)a;
		swap(x->alphas.rows[0], x->alphas.rows[3]);
		swap(x->alphas.rows[1], x->alphas.rows[2]);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		DXT1Block * x = (DXT1Block *)a;
		swap(x->rows[0], x->rows[3]);
		swap(x->rows[1], x->rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_BPTC_UNORM_EXT) // BC7
	{
		swap_bc7_idx(a);
	}
	else // BC4 or BC5
	{
		DXT5AlphaBlock * x = (DXT5AlphaBlock  *)a;
		swap_12bit_idx(x->idx);
		if (type == GL_COMPRESSED_SIGNED_RG_RGTC2)
		{
			++x;
			swap_12bit_idx(x->idx);
		}
	}
}

// in-place lossless vertical flip of a single mipmap level for BC 1/2/3/4/5/7 compressed textures

static void BCx_y_flip(GLenum glformat, int dds_blocksize, char * data, int width_pix, int height_pix)
{
	int data_len = max(1 , (width_pix * height_pix) / 16) *  dds_blocksize;
	int blocks_per_line = max(1, width_pix / 4);
	int line_len = blocks_per_line * dds_blocksize;
	int swap_count = height_pix / 4 / 2;
	char * dds_line1 = data;
	char * dds_line2 = data + data_len - line_len;

	if(swap_count == 0)
		for (int i = 0; i < blocks_per_line; i++)
		{
			swap_blocks(dds_line1, glformat);
			dds_line1 += dds_blocksize;
		}
	while (swap_count--)
	{
		for (int i = 0; i < blocks_per_line; i++)
		{
			swap_blocks(dds_line1, dds_line2, glformat);
			dds_line1 += dds_blocksize;
			dds_line2 += dds_blocksize;
		}
		dds_line2 -= 2 * line_len;
	}
}

#if UNIT_TEST
int main()
{
	// for images of each CC compression type
	
	// load image A
	// flip compressed image A -> B
	// decode flipped image B
	// decode un-flipped image A
	// flip uncompressed image A
	// compare A-B
}
#endif

bool	LoadTextureFromDDS(
				char *			mem_start,
				char *			mem_end,
				int				in_tex_num,
				int				inFlags,
				int *			outWidth,
				int *			outHeight)
{
	INIT_GL_INFO

	if((mem_end - mem_start) < sizeof(TEX_dds_desc)) return false;
	const TEX_dds_desc * desc = (const TEX_dds_desc *) mem_start;

	if (strncmp(desc->dwMagic, "DDS ", 4) != 0) return false;
	if((SWAP32(desc->dwSize)) != (sizeof(*desc) - sizeof(desc->dwMagic))) return false;
	char* data = mem_start + sizeof(TEX_dds_desc);

	GLenum glformat;
	int dds_blocksize;

	if (gl_info.has_tex_compression && strncmp(desc->ddpfPixelFormat.dwFourCC, "DXT", 3) == 0)
		switch (desc->ddpfPixelFormat.dwFourCC[3])
		{
		case '1':	dds_blocksize =  8; glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;	break;
		case '3':	dds_blocksize = 16; glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;	break;
		case '5':	dds_blocksize = 16; glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;	break;
		default:	return false;
		}
	else if (gl_info.has_rgtc && strncmp(desc->ddpfPixelFormat.dwFourCC, "ATI", 3) == 0)    // This format is NOT understood by X-Plane for now !!!
		switch (desc->ddpfPixelFormat.dwFourCC[3])
		{
		case '1':	dds_blocksize =  8; glformat = GL_COMPRESSED_LUMINANCE_LATC1_EXT;  break; // BC4 decoded to rgb greyscale, e.g. crunch -DXT5A or BC4 in Gimp
		case '2':	dds_blocksize = 16; glformat = GL_COMPRESSED_RG_RGTC2;             break; // BC5 two uncorrelated channels (normals !), crunch -DXN
		default:	return false;
		}
	else if (gl_info.has_rgtc && strncmp(desc->ddpfPixelFormat.dwFourCC, "BC", 2) == 0)     // This format is NOT understood by X-Plane for now !!!
		switch (desc->ddpfPixelFormat.dwFourCC[2])
		{
		case '4':	dds_blocksize =  8; glformat = GL_COMPRESSED_LUMINANCE_LATC1_EXT;  break; // BC4, don't care if signed or unsigned
		case '5':	dds_blocksize = 16; glformat = GL_COMPRESSED_RG_RGTC2;             break; // BC5, don't care if signed or unsigned
		default:	return false;
		}
	else  if (gl_info.has_bptc && strncmp(desc->ddpfPixelFormat.dwFourCC, "DX10", 4) == 0)  // This format is NOT understood by X-Plane for now !!!
		{
			const TEX_dds_dx10 * dx10hdr = (const TEX_dds_dx10 *) (mem_start + sizeof(*desc));
			data += sizeof(TEX_dds_dx10);
			
			dds_blocksize = 16;
			switch(dx10hdr->dxgiFormat)
			{
//				case DXGI_FORMAT_BC1_UNORM:
//				case DXGI_FORMAT_BC1_UNORM_SRGB: glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;  dds_blocksize = 8; break;
//				case DXGI_FORMAT_BC2_UNORM:
//				case DXGI_FORMAT_BC2_UNORM_SRGB: glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;  break;
//				case DXGI_FORMAT_BC3_UNORM:
//				case DXGI_FORMAT_BC3_UNORM_SRGB: glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  break;
//				case DXGI_FORMAT_BC4_UNORM:      glformat = GL_COMPRESSED_LUMINANCE_LATC1_EXT; dds_blocksize = 8; break;  // gloss/metalness maps
//				case  DXGI_FORMAT_BC5_SNORM:     glformat = GL_COMPRESSED_RG_RGTC2;            break;                     // normals
//				case FORMAT_BC6H_UF16:           glformat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT; break; 
//				case ORMAT_BC6H_SF16:            glformat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT;   break; 
				case DXGI_FORMAT_BC7_UNORM:
				case DXGI_FORMAT_BC7_UNORM_SRGB: glformat = GL_COMPRESSED_RGBA_BPTC_UNORM_EXT; break;
				default: return false;													                // Deliberately don't read other DX10 DDS formats
			}
		}
	else
		return false;

	int mips = 1;
	if(inFlags & tex_Mipmap && (SWAP32(desc->dwFlags)) & DDSD_MIPMAPCOUNT)
		mips = SWAP32(desc->dwMipMapCount);

	int x = SWAP32(desc->dwWidth);
	int y = SWAP32(desc->dwHeight);
	if (outWidth) *outWidth = x;
	if (outHeight) *outHeight = y;

	if ((mips && y != NextPowerOf2(y)) || y % 8 != 0) return false;  // flipping code can only handle certain heights

	glBindTexture(GL_TEXTURE_2D, in_tex_num);

	for (int level = 0; level < mips; ++level)
	{
		// lossless flip image in Y-direction to match orientation of all other textures in XPtools/X-plane
		// to match old MSFT DIB convention (0,0) == left bottom, but DDS / DXT starts at left top.
		BCx_y_flip(glformat, dds_blocksize, data, x, y);

		int data_len = max(1 , (x * y) / 16) *  dds_blocksize;
		glCompressedTexImage2D( GL_TEXTURE_2D, level, glformat, x, y, 0, data_len, data); CHECK_GL_ERR

		x = max(1, x >> 1);
		y = max(1, y >> 1);
		data += data_len;
	}

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

	if (inFlags & tex_Wrap)
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
