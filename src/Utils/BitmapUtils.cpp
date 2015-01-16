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

#include "BitmapUtils.h"
#include "EndianUtils.h"
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "Interpolation.h"
#include <squish.h>
#include <zlib.h>
#if USE_GEOJPEG2K
#include <jasper/jasper.h>
#endif
#include "AssertUtils.h"

/*
	WARNING: Struct alignment must be "68K" (e.g. 2-byte alignment) for these
	structures to be happy!!!
*/

/*
	These headers match the Microsoft bitmap file and image headers more or less
	and are read right out of the file.
*/

#if APL
#pragma pack(2)
#endif
#if IBM || LIN
#pragma pack(push, 2)
#endif

struct	BMPHeader {
	char			signature1;
	char			signature2;
	long			fileSize;
	long			reserved;
	long			dataOffset;
};

struct	BMPImageDesc {
	long			structSize;
	long			imageWidth;
	long			imageHeight;
	short			planes;
	short			bitCount;
	long			compressionType;
	long			imageSize;
	long			xPixelsPerM;	//130B0000?  B013 = 45075?
	long			yPixelsPerM;
	long			colorsUsed;
	long			colorsImportant;
};

#if APL
#pragma options align=reset
#endif
#if IBM || LIN
#pragma pack(pop)
#endif



// DD surface flags
#define DDSD_CAPS               0x00000001l     // default
#define DDSD_HEIGHT             0x00000002l
#define DDSD_WIDTH              0x00000004l
#define DDSD_PITCH              0x00000008l		// rowbytes to mac nerds
#define DDSD_PIXELFORMAT        0x00001000l
#define DDSD_MIPMAPCOUNT        0x00020000l
#define DDSD_LINEARSIZE         0x00080000l

// DD Pixel format flags
#define DDPF_ALPHAPIXELS        0x00000001l		// has alpha in addition to RGB
#define DDPF_FOURCC             0x00000004l		// Is 4cc compressed
#define DDPF_RGB				0x00000040l		// Is RGB (may have alpha)

// DD surface caps
#define DDSCAPS_TEXTURE			0x00001000l
#define DDSCAPS_MIPMAP          0x00400000l
#define DDSCAPS_COMPLEX         0x00000008l

#if APL || LIN
	#define DWORD unsigned int
#endif

struct TEX_dds_caps2 {
    DWORD       dwCaps;         // capabilities of surface wanted
    DWORD       dwCaps2;
    DWORD       dwCaps3;
    DWORD       dwCaps4;
};

struct TEX_dds_pixelformat {
    DWORD       dwSize;                 // size of structure (must be 32)
    DWORD       dwFlags;                // pixel format flags
    char        dwFourCC[4];               // (FOURCC code)		D X T 3 in memory string.
	DWORD		dwRGBBitCount;          // how many bits per pixel
	DWORD		dwRBitMask;             // mask for red bit
	DWORD		dwGBitMask;             // mask for green bits
	DWORD		dwBBitMask;             // mask for blue bits
	DWORD		dwRGBAlphaBitMask;      // mask for alpha channel
};

struct TEX_dds_desc {
	char				dwMagic[4];				// D D S <space> sequential string in memory.  This is not REALLY in the struct, but good enough for me.

    DWORD               dwSize;                 // size of the DDSURFACEDESC structure		(Must be 124)
    DWORD               dwFlags;                // determines what fields are valid			(DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH, DDSD_HEIGHT.)
    DWORD               dwHeight;               // height of surface to be created
    DWORD               dwWidth;                // width of input surface
	DWORD				dwLinearSize;           // Formless late-allocated optimized surface size
    DWORD               dwDepth;				// Vol texes-depth.
	DWORD				dwMipMapCount;          // number of mip-map levels requestde
	DWORD               dwReserved1[11];        //
	TEX_dds_pixelformat	ddpfPixelFormat;        // pixel format description of the surface
    TEX_dds_caps2       ddsCaps;                // direct draw surface capabilities			DDSCAPS_TEXTURE, DDSCAPS_MIPMAP, DDSCAPS_COMPLEX		TEXTURE, LINEARSIZE, COMPLEX, MIPMAP, FOURCC)
    DWORD               dwReserved2;			//
};







/*
	NOTES ON ENDIAN CHAOS!!!!!!!!!!!!!!!!!!!

	Simply viewing the way pixels are stored in memory from low to high mem:
	BMP contains RGBRGBRGB triplets.
		(This is a direct byte-order read out of memory, so it can't get f-cked.
	PNG lib returns RGBARGBARGBA quads, but only because we set it in BGR mode.  (Don't ask me.)
		(As far as I can tell, pnglib is just goofy.)

	TiffLib on Mach gives it to us in ARGB format...and on windows in BGRA format.
		(Tifflib is endian-aware and always uses ARGB.  But at least it's consistent!)

	OpenGL strangely wants BGRA in memory when we say RGBA, which makes no sense to me.


	TODO...
		It would be nice to generic-ize these APIs so that we could:
		- refer to file formats by enum.
		- read and write any format (BMP, PNG, TIFF, JPG)
		- consider more formats (targa?)
		- handle indexed color
		- handle 16-bit color
 */

#if IBM
	#include <windows.h>
	#define XMD_H
	#define HAVE_BOOLEAN
	#if MINGW_BUILD
	#define __INTEL__
	#endif
#endif

// Note: the std jpeg lib does not have any #ifdef C++ name
// mangling protection.  Since wek now we're going to be CPP,
// add it ourself.  Gross, but perhaps better than hacking up
// libjpeg??

#if USE_JPEG
extern "C" {
	#include <jpeglib.h>
	#include <jerror.h>
}
#endif

#include <png.h>
#if USE_TIF
#include <tiffio.h>
#endif


// BMP is always BGR, and alwys lower-left origin.  Since this is what we want in memory
// (E.g. DIB conventions) we can just load.
int		CreateBitmapFromFile(const char * inFilePath, struct ImageInfo * outImageInfo)
{
		struct	BMPHeader		header;
		struct	BMPImageDesc	imageDesc;
		long					pad;
		int 					err = 0;
		FILE *					fi = NULL;

	outImageInfo->data = NULL;

	fi = fopen(inFilePath, "rb");
	if (fi == NULL)
		goto bail;

	/*  First we read in the headers, endian flip them, sanity check them, and decide how big our
		image is. */

	if (fread(&header, sizeof(header), 1, fi) != 1)
		goto bail;
	if (fread(&imageDesc, sizeof(imageDesc), 1, fi) != 1)
		goto bail;

	EndianFlipLong(&header.fileSize);
	EndianFlipLong(&header.dataOffset);

	EndianFlipLong(&imageDesc.imageWidth);
	EndianFlipLong(&imageDesc.imageHeight);
	EndianFlipShort(&imageDesc.bitCount);

	if ((header.signature1 != 'B') ||
		(header.signature2 != 'M') ||
		(imageDesc.bitCount != 24) ||
		(imageDesc.imageWidth <= 0) ||
		(imageDesc.imageHeight <= 0))
		goto bail;

	if ((header.fileSize - header.dataOffset) < (imageDesc.imageWidth * imageDesc.imageHeight * 3))
		goto bail;

	pad = (imageDesc.imageWidth * 3 + 3) & ~3;
	pad -= imageDesc.imageWidth * 3;

	outImageInfo->width = imageDesc.imageWidth;
	outImageInfo->height = imageDesc.imageHeight;
	outImageInfo->pad = pad;

	/* Now we can allocate an image buffer. */

	outImageInfo->channels = 3;
	outImageInfo->data = (unsigned char *) malloc(imageDesc.imageWidth * imageDesc.imageHeight * outImageInfo->channels + imageDesc.imageHeight * pad);
	if (outImageInfo->data == NULL)
		goto bail;

	/*  We can pretty much just read the bytes in; we know that we're 24 bit so there is no
		color table, and 24 bit BMP files cannot be compressed. */

	if (fread(outImageInfo->data, imageDesc.imageWidth * imageDesc.imageHeight * outImageInfo->channels + imageDesc.imageHeight * pad, 1, fi) != 1)
		goto bail;

	fclose(fi);
	return 0;

bail:
	err = errno;
	if (fi != NULL)
		fclose(fi);
	if (outImageInfo->data != NULL)
		free(outImageInfo->data);
	if (err == 0)
		err = -1;
	return err;
}

int		WriteBitmapToFile(const struct ImageInfo * inImage, const char * inFilePath)
{
		FILE *					fi = NULL;
		struct	BMPHeader		header;
		struct	BMPImageDesc	imageDesc;
		int						err = 0;
	
		//If the file has the wrong kind of channels or will not work with the padding required
		//Fail imidiantly.
		Assert(((inImage->width * inImage->channels + inImage->pad) % 4) == 0);
		Assert(inImage->channels == 3);

	/* First set up the appropriate header structures to match our bitmap. */

	header.signature1 = 'B';
	header.signature2 = 'M';
	header.fileSize = sizeof(struct BMPHeader) + sizeof(struct BMPImageDesc) + ((inImage->width * 3 + inImage->pad) * inImage->height);
	header.reserved = 0;
	header.dataOffset = sizeof(struct BMPHeader) + sizeof(struct BMPImageDesc);
	EndianFlipLong(&header.fileSize);
	EndianFlipLong(&header.reserved);
	EndianFlipLong(&header.dataOffset);

	imageDesc.structSize = sizeof(imageDesc);
	imageDesc.imageWidth = inImage->width;
	imageDesc.imageHeight = inImage->height;
	imageDesc.planes = 1;
	imageDesc.bitCount = 24;
	imageDesc.compressionType = 0;
	imageDesc.imageSize = (inImage->width * inImage->height * 3);
	imageDesc.xPixelsPerM = 0;
	imageDesc.yPixelsPerM = 0;
	imageDesc.colorsUsed = 0;
	imageDesc.colorsImportant = 0;

	EndianFlipLong(&imageDesc.structSize);
	EndianFlipLong(&imageDesc.imageWidth);
	EndianFlipLong(&imageDesc.imageHeight);
	EndianFlipShort(&imageDesc.planes);
	EndianFlipShort(&imageDesc.bitCount);
	EndianFlipLong(&imageDesc.compressionType);
	EndianFlipLong(&imageDesc.imageSize);
	EndianFlipLong(&imageDesc.xPixelsPerM);
	EndianFlipLong(&imageDesc.yPixelsPerM);
	EndianFlipLong(&imageDesc.colorsUsed);
	EndianFlipLong(&imageDesc.colorsImportant);

	fi = fopen(inFilePath, "wb");
	if (fi == NULL)
		goto bail;

	/* We can just write out two headers and the data and be done. */

	if (fwrite(&header, sizeof(header), 1, fi) != 1)
		goto bail;
	if (fwrite(&imageDesc, sizeof(imageDesc), 1, fi) != 1)
		goto bail;
	if (fwrite(inImage->data, (inImage->width * 3 + inImage->pad) * inImage->height, 1, fi) != 1)
		goto bail;

	fclose(fi);
	return 0;

bail:
	err = errno;
	if (fi != NULL)
		fclose(fi);
	if (err == 0)
		err = -1;
	return err;
}

int		CreateNewBitmap(long inWidth, long inHeight, short inChannels, struct ImageInfo * outImageInfo)
{
	outImageInfo->width = inWidth;
	outImageInfo->height = inHeight;
	/* This nasty voodoo calculates the padding necessary to make each scanline a multiple of four bytes. */
	outImageInfo->pad = ((inWidth * inChannels + 3) & ~3) - (inWidth * inChannels);
	outImageInfo->channels = inChannels;
	outImageInfo->data = (unsigned char *) malloc(inHeight * ((inWidth * inChannels) + outImageInfo->pad));
	if (outImageInfo->data == NULL)
		return ENOMEM;
	return 0;
}
int GetSupportedType(const char * path)
{
	string extension(path);
	//Takes care of cases like .PNG and .JPG
	extension = extension.substr(extension.find_last_of('.')+1);

	if(extension.length() == 3)
	{
		extension[0] = tolower(extension[0]);
		extension[1] = tolower(extension[1]);
		extension[2] = tolower(extension[2]);
	}
	else
	{
		return -1;
	}

	//compare the string and if it is perfectly the same return that code
	if(extension == "bmp") return WED_BMP;
	if(extension == "dds") return WED_DDS;
	#if USE_GEOJPEG2K
	if(extension == "jp2") return WED_JP2K;
	#endif
	//jpeg or jpg is supported
	if((extension == "jpeg")||(extension == "jpg")) return WED_JPEG;
	if(extension == "png") return WED_PNG;
	if(extension == "tif") return WED_TIF;
	
	//Otherwise return the error
	return -1;
}

int MakeSupportedType(const char * path, ImageInfo * inImage)
{
	int error = -1;//Guilty until proven innocent
	switch(GetSupportedType(path))
	{
	case WED_BMP:
		error = CreateBitmapFromFile(path,inImage);
		break;
	case WED_DDS:
		error = CreateBitmapFromDDS(path,inImage);
		break;
	#if USE_GEOJPEG2K
	case WED_JP2K:
		error = CreateBitmapFromJP2K(path,inImage);
		break;
	#endif
	#if USE_JPEG
	case WED_JPEG:
		error = CreateBitmapFromJPEG(path,inImage);
		break;
	#endif	
	case WED_PNG:
		error = CreateBitmapFromPNG(path,inImage,false, GAMMA_SRGB);
		break;
	#if USE_TIF	
	case WED_TIF:
		error = CreateBitmapFromTIF(path,inImage);
		break;
	#endif
	default:
		return error;//No good images or a broken file path
	}
	return error;
}

void	FillBitmap(const struct ImageInfo * inImageInfo, char c)
{
	memset(inImageInfo->data, c, inImageInfo->width * inImageInfo->height * inImageInfo->channels);
}

void	DestroyBitmap(const struct ImageInfo * inImageInfo)
{
	free(inImageInfo->data);
}


void	CopyBitmapSection(
			const struct ImageInfo *	inSrc,
			const struct ImageInfo	*	inDst,
			long				inSrcLeft,
			long				inSrcTop,
			long				inSrcRight,
			long				inSrcBottom,
			long				inDstLeft,
			long				inDstTop,
			long				inDstRight,
			long				inDstBottom)
{
	/*  This routine copies a subsection of one bitmap onto a subsection of another, using bicubic interpolation
		for scaling. */
	double	srcLeft = inSrcLeft, srcRight = inSrcRight, srcTop = inSrcTop, srcBottom = inSrcBottom;
	double	dstLeft = inDstLeft, dstRight = inDstRight, dstTop = inDstTop, dstBottom = inDstBottom;

	/*	Here's why we subtract one from all of these...
		(Ignore bicubic interpolation for a moment please...)
		Every destination pixel is comprised of two pixels horizontally and two vertically.  We use these widths and heights to do the rescaling
		of the image.  The goal is to have the rightmost pixel in the destination actually correspond to the rightmost pixel of the source.  In
		otherwords, we want to get (inDstRight - 1) from (inSrcRight - 1).  Now since we use [inDstLeft - inDstRight) as our set of pixels, this
		is the last pixel we ask for.  But we have to subtract one from the width to get the rescaling right, otherwise we map inDstRight to
		inSrcRight, which for very large upscales make inDstRight - 1 derive from inSrcRight - (something less than one) which is a pixel partially
		off the right side of the bitmap, which is bad.
		Bicubic interpolation is not used when we're this close to the edge of the border, so it is not a factor.
	*/

	double	dstWidth = dstRight - dstLeft - 1.0;
	double	dstHeight = dstBottom - dstTop - 1.0;
	double	srcWidth = srcRight - srcLeft - 1.0;
	double	srcHeight = srcBottom - srcTop - 1.0;

	double	dx, dy;

	long	srcRowBytes = inSrc->width * inSrc->channels + inSrc->pad;
	long	srcRowBytes2 = srcRowBytes * 2;
	long	dstRowBytes = inDst->width * inSrc->channels + inDst->pad;
	unsigned char *	srcBaseAddr = inSrc->data;
	unsigned char *	dstBaseAddr = inDst->data;

	int		channels;

	for (dy = dstTop; dy < dstBottom; dy += 1.0)
	{
		for (dx = dstLeft; dx < dstRight; dx += 1.0)
		{
			/*  For each pixel in the destination, find a pixel in the source.  Note that it may have a fractional part
				if we are scaling. */

			double	sx = ((dx - dstLeft) / dstWidth * srcWidth) + srcLeft;
			double	sy = ((dy - dstTop) / dstHeight * srcHeight) + srcTop;

			unsigned char *	dstPixel = dstBaseAddr + ((long) dx * inDst->channels) + ((long) dy * dstRowBytes);
			unsigned char *	srcPixel = srcBaseAddr + ((long) sx * inSrc->channels) + ((long) sy * srcRowBytes);

			/* 	If we would need pixels from off the edge of the image for bicubic interpolation,
			 	just use bilinear. */

			if ((sx < 1) ||
				(sy < 1) ||
				(sx >= (inSrc->width - 2)) ||
				(sy >= (inSrc->height - 2)))
			{
				channels = inSrc->channels;
				while (channels--)
				{
					// While this interpolation works right, when it reads the last row, its mix ratio is 100% the top/left
					// BUT it reads the bot/right anyway.  This causes access violations on Windows.
					// (Apparently it's a "real" operating system...)

					// so we specifically check this case.

					double	mixH = sx - floor(sx);
					double	mixV = sy - floor(sy);

					unsigned char tl = *srcPixel;
					unsigned char tr = (mixH> 0.0) ? *(srcPixel+inSrc->channels) : 0;
					unsigned char bl = (mixV> 0.0) ? *(srcPixel+srcRowBytes) : 0;
					unsigned char br = ((mixH> 0.0) && (mixV > 0.0)) ?
						*(srcPixel+srcRowBytes + inSrc->channels) : 0;

					/*  Take the pixel (rounded down to integer coords), the one to the right, below, and below to the right.
						The fractional part of the pixel is our weighting for interpolation. */
					unsigned char pixel = (unsigned char) BilinearInterpolate2d(
								tl, tr, bl, br, mixH, mixV);
					*dstPixel = pixel;
					++srcPixel;
					++dstPixel;
				}

			} else {
				channels = inSrc->channels;
				while (channels--)
				{
					/* Same as above, except we now take 16 pixels surrounding the location we want. */
					*dstPixel = (unsigned char) BicubicInterpolate2d(
									*(srcPixel-inSrc->channels-srcRowBytes),*(srcPixel-srcRowBytes),*(srcPixel+inSrc->channels-srcRowBytes),*(srcPixel+inSrc->channels*2-srcRowBytes),
									*(srcPixel-inSrc->channels),*srcPixel,*(srcPixel+inSrc->channels),*(srcPixel+inSrc->channels*2),
									*(srcPixel-inSrc->channels+srcRowBytes),*(srcPixel+srcRowBytes),*(srcPixel+inSrc->channels+srcRowBytes),*(srcPixel+inSrc->channels*2+srcRowBytes),
									*(srcPixel-inSrc->channels+srcRowBytes2),*(srcPixel+srcRowBytes2),*(srcPixel+inSrc->channels+srcRowBytes2),*(srcPixel+inSrc->channels*2+srcRowBytes2),
									sx - floor(sx), sy - floor(sy));
					++srcPixel;
					++dstPixel;
				}
			}

		}
	}
}

inline double	Interp2(double frac, double sml, double big)
{
	return sml + frac * (big - sml);
}

void	CopyBitmapSectionWarped(
			const struct ImageInfo *	inSrc,
			const struct ImageInfo *	inDst,
			long				inTopLeftX,
			long				inTopLeftY,
			long				inTopRightX,
			long				inTopRightY,
			long				inBotRightX,
			long				inBotRightY,
			long				inBotLeftX,
			long				inBotLeftY,
			long				inDstLeft,
			long				inDstTop,
			long				inDstRight,
			long				inDstBottom)
{
	/*  This routine copies a subsection of one bitmap onto a subsection of another, using bicubic interpolation
		for scaling. */

	double	dstLeft = inDstLeft, dstRight = inDstRight, dstTop = inDstTop, dstBottom = inDstBottom;
	double	topLeftX = inTopLeftX, topLeftY = inTopLeftY, topRightX = inTopRightX, topRightY = inTopRightY;
	double	botLeftX = inBotLeftX, botLeftY = inBotLeftY, botRightX = inBotRightX, botRightY = inBotRightY;

	/*	Here's why we subtract one from all of these...
		(Ignore bicubic interpolation for a moment please...)
		Every destination pixel is comprised of two pixels horizontally and two vertically.  We use these widths and heights to do the rescaling
		of the image.  The goal is to have the rightmost pixel in the destination actually correspond to the rightmost pixel of the source.  In
		otherwords, we want to get (inDstRight - 1) from (inSrcRight - 1).  Now since we use [inDstLeft - inDstRight) as our set of pixels, this
		is the last pixel we ask for.  But we have to subtract one from the width to get the rescaling right, otherwise we map inDstRight to
		inSrcRight, which for very large upscales make inDstRight - 1 derive from inSrcRight - (something less than one) which is a pixel partially
		off the right side of the bitmap, which is bad.
		Bicubic interpolation is not used when we're this close to the edge of the border, so it is not a factor.
	*/

	double	dstWidth = dstRight - dstLeft - 1.0;
	double	dstHeight = dstBottom - dstTop - 1.0;

	double	dx, dy;

	long	srcRowBytes = inSrc->width * inSrc->channels + inSrc->pad;
	long	srcRowBytes2 = srcRowBytes * 2;
	long	dstRowBytes = inDst->width * inSrc->channels + inDst->pad;
	unsigned char *	srcBaseAddr = inSrc->data;
	unsigned char *	dstBaseAddr = inDst->data;

	int		channels;

	for (dy = dstTop; dy < dstBottom; dy += 1.0)
	{
		for (dx = dstLeft; dx < dstRight; dx += 1.0)
		{
			/*  For each pixel in the destination, find a pixel in the source.  Note that it may have a fractional part
				if we are scaling. */

			double frac_x = ((dx - dstLeft) / dstWidth);
			double frac_y = ((dy - dstTop) / dstHeight);

			double	sx = Interp2(frac_y, Interp2(frac_x, topLeftX, topRightX), Interp2(frac_x, botLeftX, botRightX));
			double	sy = Interp2(frac_x, Interp2(frac_y, topLeftY, botLeftY), Interp2(frac_y, topRightY, botRightY));

			unsigned char *	dstPixel = dstBaseAddr + ((long) dx * inDst->channels) + ((long) dy * dstRowBytes);
			unsigned char *	srcPixel = srcBaseAddr + ((long) sx * inSrc->channels) + ((long) sy * srcRowBytes);

			/* 	If we would need pixels from off the edge of the image for bicubic interpolation,
			 	just use bilinear. */

			if ((sx < 1) ||
				(sy < 1) ||
				(sx >= (inSrc->width - 2)) ||
				(sy >= (inSrc->height - 2)))
			{
				channels = inSrc->channels;
				while (channels--)
				{
					// While this interpolation works right, when it reads the last row, its mix ratio is 100% the top/left
					// BUT it reads the bot/right anyway.  This causes access violations on Windows.
					// (Apparently it's a "real" operating system...)

					// so we specifically check this case.

					double	mixH = sx - floor(sx);
					double	mixV = sy - floor(sy);

					unsigned char tl = *srcPixel;
					unsigned char tr = (mixH> 0.0) ? *(srcPixel+inSrc->channels) : 0;
					unsigned char bl = (mixV> 0.0) ? *(srcPixel+srcRowBytes) : 0;
					unsigned char br = ((mixH> 0.0) && (mixV > 0.0)) ?
						*(srcPixel+srcRowBytes + inSrc->channels) : 0;

					/*  Take the pixel (rounded down to integer coords), the one to the right, below, and below to the right.
						The fractional part of the pixel is our weighting for interpolation. */
					unsigned char pixel = (unsigned char) BilinearInterpolate2d(
								tl, tr, bl, br, mixH, mixV);
					*dstPixel = pixel;
					++srcPixel;
					++dstPixel;
				}

			} else {
				channels = inSrc->channels;
				while (channels--)
				{
					/* Same as above, except we now take 16 pixels surrounding the location we want. */
					*dstPixel = (unsigned char) BicubicInterpolate2d(
									*(srcPixel-inSrc->channels-srcRowBytes),*(srcPixel-srcRowBytes),*(srcPixel+inSrc->channels-srcRowBytes),*(srcPixel+inSrc->channels*2-srcRowBytes),
									*(srcPixel-inSrc->channels),*srcPixel,*(srcPixel+inSrc->channels),*(srcPixel+inSrc->channels*2),
									*(srcPixel-inSrc->channels+srcRowBytes),*(srcPixel+srcRowBytes),*(srcPixel+inSrc->channels+srcRowBytes),*(srcPixel+inSrc->channels*2+srcRowBytes),
									*(srcPixel-inSrc->channels+srcRowBytes2),*(srcPixel+srcRowBytes2),*(srcPixel+inSrc->channels+srcRowBytes2),*(srcPixel+inSrc->channels*2+srcRowBytes2),
									sx - floor(sx), sy - floor(sy));
					++srcPixel;
					++dstPixel;
				}
			}

		}
	}
}

void	CopyBitmapSectionDirect(
			const struct ImageInfo&		inSrc,
			const struct ImageInfo&		inDst,
			long						inSrcLeft,
			long						inSrcTop,
			long						inDstLeft,
			long						inDstTop,
			long						inWidth,
			long						inHeight)
{
	if (inSrc.channels != inDst.channels) return;

	long	src_rb = inSrc.width * inSrc.channels + inSrc.pad;
	long	dst_rb = inDst.width * inDst.channels + inDst.pad;
	long	src_nr = src_rb - inWidth* inSrc.channels;
	long	dst_nr = dst_rb - inWidth* inDst.channels;
	unsigned char *	src_p = inSrc.data + inSrcTop * src_rb + inSrcLeft * inSrc.channels;
	unsigned char *	dst_p = inDst.data + inDstTop * dst_rb + inDstLeft * inDst.channels;

	while (inHeight--)
	{
		long ctr = inWidth * inSrc.channels;
		while (ctr--)
		{
			*dst_p++ = *src_p++;
		}
		src_p += src_nr;
		dst_p += dst_nr;
	}

}


void	RotateBitmapCCW(
			struct ImageInfo *	ioBitmap)
{
	/* We have to allocate a new bitmap to transfer our old data to.  The new bitmap might not have the same
	 * storage size as the old bitmap because of padding! */

	long	newWidth = ioBitmap->height;
	long	newHeight = ioBitmap->width;
	long	newPad = ((newWidth * ioBitmap->channels + 3) & ~3) - (newWidth * ioBitmap->channels);
	unsigned char * newData = (unsigned char *) malloc(((newWidth * ioBitmap->channels) + newPad) * newHeight);
	if (newData == NULL)
		return;

	for (long y = 0; y < ioBitmap->height; ++y)
	for (long x = 0; x < ioBitmap->width; ++x)
	{
		long	nx = ioBitmap->height - y - 1;
		long	ny = x;

		unsigned char *	srcP = ioBitmap->data + (x * ioBitmap->channels) + (y * (ioBitmap->channels * ioBitmap->width + ioBitmap->pad));
		unsigned char *	dstP = newData + (nx * ioBitmap->channels) + (ny * (ioBitmap->channels * newWidth + newPad));
		long	chCount = ioBitmap->channels;
		while (chCount--)
		{
			*dstP++ = *srcP++;
		}
	}

	free(ioBitmap->data);
	ioBitmap->data = newData;
	ioBitmap->width = newWidth;
	ioBitmap->height = newHeight;
	ioBitmap->pad = newPad;

}

int	ConvertBitmapToAlpha(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha)
{
		unsigned char * 	oldData, * newData, * srcPixel, * dstPixel;
		long 	count;
		long	x,y;

	if (ioImage->channels == 4)
		return 0;

	/* We have to allocate a new bitmap that is larger than the old to store the alpha channel. */

	newData = (unsigned char *) malloc(ioImage->width * ioImage->height * 4);
	if (newData == NULL)
		return ENOMEM;
	oldData = ioImage->data;

	srcPixel = oldData;
	dstPixel = newData;
	count = ioImage->width * ioImage->height;
	for (y = 0; y < ioImage->height; ++y)
	for (x = 0; x < ioImage->width; ++x)
	{
		/* For each pixel, if it is pure magenta, it becomes pure black transparent.  Otherwise it is
		 * opaque and retains its color.  NOTE: one of the problems with the magenta=alpha strategy is
		 * that we don't know what color was 'under' the transparency, so if we stretch or skew this bitmap
		 * we can't really do a good job of interpolating. */
		if (doMagentaAlpha &&
			(srcPixel[0] == 0xFF) &&
			(srcPixel[1] == 0x00) &&
			(srcPixel[2] == 0xFF))
		{
			dstPixel[0] = 0;
			dstPixel[1] = 0;
			dstPixel[2] = 0;
			dstPixel[3] = 0;
		} else {
			dstPixel[0] = srcPixel[0];
			dstPixel[1] = srcPixel[1];
			dstPixel[2] = srcPixel[2];
			dstPixel[3] = 0xFF;
		}

		srcPixel += 3;
		dstPixel += 4;

		if (x == (ioImage->width - 1))
			srcPixel += ioImage->pad;
	}

	ioImage->data = newData;
	ioImage->pad = 0;
	free(oldData);
	ioImage->channels = 4;
	return 0;
}


int	ConvertAlphaToBitmap(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha)
{
	return ConvertAlphaToBitmap(ioImage, doMagentaAlpha, true);
}

int	ConvertAlphaToBitmap(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha,
			bool					doPadding)
{
		unsigned char * 	oldData, * newData, * srcPixel, * dstPixel;
		long 	count;
		long 	x,y;
		int		pad;

	if (ioImage->channels == 3)
		return 0;

	if(doPadding)
		pad = ((ioImage->width * 3 + 3) & ~3) - (ioImage->width * 3);
	else
		pad = 0;

	newData = (unsigned char *) malloc((ioImage->width * 3 + pad) * ioImage->height);
	if (newData == NULL)
		return ENOMEM;
	oldData = ioImage->data;

	ioImage->pad = pad;

	srcPixel = oldData;
	dstPixel = newData;
	count = ioImage->width * ioImage->height;

	for (y = 0; y < ioImage->height; ++y)
	for (x = 0; x < ioImage->width; ++x)
	{
		/* For each pixel, only full opaque is taken.  Here's why: if the pixel is part alpha, then it is a blend of an
		 * alpha pixel and a non-alpha pixel.  But...we don't have good color data for the alpha pixel; from the above
		 * routine we set the color to black.  So the color data for this pixel is mixed with black.  When viewed the
		 * edges of a stretched bitmap will appear to turn dark before they fade out.
		 *
		 * But this point is moot anyway; we really only have one bit of alpha, on or off.  So we could pick any cutoff
		 * value and we'll still get a really sharp jagged line at the edge of the transparency.
		 *
		 * You might ask yourself, why does X-Plane do it this way?  The answer is that as of this writing, most graphics
		 * cards do not have the alpha-blending fill rate to blend the entire aircraft panel; this would be a huge hit on
		 * frame rate.  So Austin is using the alpha test mechanism for transparency, which is much faster but only one
		 * bit deep.
		 *
		 */
		if (doMagentaAlpha && srcPixel[3] != 0xFF)
		{
			dstPixel[0] = 0xFF;
			dstPixel[1] = 0x00;
			dstPixel[2] = 0xFF;
		} else {
			dstPixel[0] = srcPixel[0];
			dstPixel[1] = srcPixel[1];
			dstPixel[2] = srcPixel[2];
		}

		srcPixel += 4;
		dstPixel += 3;

		if (x == (ioImage->width - 1))
			dstPixel += ioImage->pad;
	}

	ioImage->data = newData;
	free(oldData);
	ioImage->channels = 3;
	return 0;
}

#pragma mark -

#if USE_JPEG

/*
 * JPEG in-memory source manager
 *
 * Given a JFIF file that is entirley in memory, this object uses that buffer to provide data
 * to the JPEG lib.
 *
 */

typedef struct {
	struct jpeg_source_mgr pub;		// Fields inherited from all jpeg source mgs.

	JOCTET * 	buffer;				// Buffer start and size
	int			len;				//
} mem_source_mgr;
typedef mem_source_mgr *  mem_src_ptr;

typedef struct {
	struct jpeg_error_mgr	pub;
	jmp_buf					buf;
} setjmp_err_mgr;
typedef setjmp_err_mgr *	setjmp_err_ptr;


METHODDEF(void) mem_init_source (j_decompress_ptr cinfo)
{
}

METHODDEF(boolean) mem_fill_input_buffer (j_decompress_ptr cinfo)
{
	// If we are asked to fill the buffer, we can't; we give JPEG
	// all of the memory up front.  So throw a fatal err.
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
    return TRUE;
}

METHODDEF(void) mem_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	mem_src_ptr src = (mem_src_ptr) cinfo->src;
	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF(void) mem_term_source (j_decompress_ptr cinfo)
{
}

/*
 * JPEG exception-based error handler.  This throws an exception
 * rather than calling exit() on the process.
 *
 */
METHODDEF(void) throw_error_exit (j_common_ptr cinfo)
{
	// On a fatal error, we deallocate the struct first,
	// then throw.  This is a good idea because the cinfo
	// struct may go out of scope during the throw; this
	// relieves client code from having to worry about
	// order of destruction.
	jpeg_destroy(cinfo);
//	throw EXIT_FAILURE;
	longjmp(((setjmp_err_ptr) cinfo->err)->buf,1);
}

METHODDEF(void)
eat_output_message (j_common_ptr cinfo)
{
	// If the user needed to see something, this is where
	// we'd find out.  We currently don't have a good way
	// of showing the users messages.
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message) (cinfo, buffer);
}

GLOBAL(struct jpeg_error_mgr *)
jpeg_throw_error (setjmp_err_mgr * err)
{
	// This routine sets up our various error handlers.
	// We use their decision making logic, and change
	// two of our own handlers.
	jpeg_std_error(&err->pub);
	err->pub.error_exit = throw_error_exit;
	err->pub.output_message = eat_output_message;

	return &err->pub;
}




// JPEG is 0,0 = upper left and lib gives us RGB.  So we need to red-blue swap and vertically flip.
int		CreateBitmapFromJPEG(const char * inFilePath, struct ImageInfo * outImageInfo)
{
	// We bail immediately if the file is no good.  This prevents us from
	// having to keep track of file openings; if we have a problem, but the file must be
	// closed.
	outImageInfo->data = NULL;
	FILE * fi = fopen(inFilePath, "rb");
	if (!fi) return errno;

	try {

			struct jpeg_decompress_struct cinfo;
			setjmp_err_mgr		  jerr;

		if(setjmp(jerr.buf))
		{
			fclose(fi);
			return 1;
		}

		cinfo.err = jpeg_throw_error(&jerr);
		jpeg_create_decompress(&cinfo);

		jpeg_stdio_src(&cinfo, fi);

		jpeg_read_header(&cinfo, TRUE);

		jpeg_start_decompress(&cinfo);

		outImageInfo->width = cinfo.output_width;
		outImageInfo->height = cinfo.output_height;
		outImageInfo->pad = 0;
		outImageInfo->channels = 3;
		outImageInfo->data = (unsigned char *) malloc(outImageInfo->width * outImageInfo->height * outImageInfo->channels);

		int linesize = outImageInfo->width * outImageInfo->channels;
		int linecount = outImageInfo->height;
		unsigned char * p = outImageInfo->data + (linecount - 1) * linesize;
		while (linecount--)
		{
			if (jpeg_read_scanlines (&cinfo, &p, 1) == 0)
				break;

			for (int n = cinfo.output_width - 1; n >= 0; --n)
			{
				if (cinfo.output_components == 1)
					p[n*3+2] = p[n*3+1] = p[n*3] = p[n];
				else
					swap(p[n*3+2],p[n*3]);
			}
			p -= linesize;
		}

		jpeg_finish_decompress(&cinfo);

		jpeg_destroy_decompress(&cinfo);
		fclose(fi);
		return 0;
	} catch (...) {
		// If we ever get an exception, it's because we got a fatal JPEG error.  Our
		// error handler deallocates the jpeg struct, so all we have to do is close the
		// file and bail.
		fclose(fi);
		return 1;
	}
}



int		CreateBitmapFromJPEGData(void * inBytes, int inLength, struct ImageInfo * outImageInfo)
{
	try {
			struct jpeg_decompress_struct cinfo;
			setjmp_err_mgr		  jerr;

		if (setjmp(jerr.buf))
		{
			return 1;
		}
		cinfo.err = jpeg_throw_error(&jerr);
		jpeg_create_decompress(&cinfo);

		mem_source_mgr	src;

		cinfo.src = (struct jpeg_source_mgr *) &src;

		src.pub.init_source = mem_init_source;
		src.pub.fill_input_buffer = mem_fill_input_buffer;
		src.pub.skip_input_data = mem_skip_input_data;
		src.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
		src.pub.term_source = mem_term_source;
		src.buffer = (JOCTET *) inBytes;
		src.len = inLength;
		src.pub.bytes_in_buffer = inLength; /* forces fill_input_buffer on first read */
		src.pub.next_input_byte = (JOCTET *) inBytes; /* until buffer loaded */

		jpeg_read_header(&cinfo, TRUE);

		jpeg_start_decompress(&cinfo);

		outImageInfo->width = cinfo.output_width;
		outImageInfo->height = cinfo.output_height;
		outImageInfo->pad = 0;
		outImageInfo->channels = 3;
		outImageInfo->data = (unsigned char *) malloc(outImageInfo->width * outImageInfo->height * outImageInfo->channels);

		int linesize = outImageInfo->width * outImageInfo->channels;
		int linecount = outImageInfo->height;
		unsigned char * p = outImageInfo->data + (linecount - 1) * linesize;
		while (linecount--)
		{
			if (jpeg_read_scanlines (&cinfo, &p, 1) == 0)
				break;

			for (int n = cinfo.output_width - 1; n >= 0; --n)
			{
				if (cinfo.output_components == 1)
					p[n*3+2] = p[n*3+1] = p[n*3] = p[n];
				else
					swap(p[n*3+2],p[n*3]);
			}
			p -= linesize;
		}

		jpeg_finish_decompress(&cinfo);

		jpeg_destroy_decompress(&cinfo);
		return 0;
	} catch (...) {
		// If we get an exceptoin, cinfo is already cleaned up; just bail.
		return 1;
	}
}

#endif /* USE_JPEG */


void my_error  (png_structp,png_const_charp err){}
void my_warning(png_structp,png_const_charp err){}

const char *			png_start_pos 	= NULL;
const char *			png_end_pos 	= NULL;
const char *			png_current_pos	= NULL;

void png_buffered_read_func(png_structp png_ptr, png_bytep data, png_size_t length)
{
   if((png_current_pos+length)>png_end_pos)
		png_error(png_ptr,"PNG Read Error, overran end of buffer!");
   memcpy(data,png_current_pos,length);
   png_current_pos+=length;
}

// PNG is 0,0 = upper left so we vertically flip.  Lib gives us image in any component order we want.
int		CreateBitmapFromPNG(const char * fname, struct ImageInfo * outImageInfo, bool leaveIndexed, float target_gamma)
{
	FILE * file = fopen(fname, "rb");
	if (!file)
		return -1;

	fseek(file, 0, SEEK_END);
	int fileLength = ftell(file);
	fseek(file, 0, SEEK_SET);
	char * buffer = new char[fileLength];
	if (!buffer)
	{
		fclose(file);
		return -1;
	}
	if (fread(buffer, 1, fileLength, file) != fileLength)
	{
		fclose(file);
		delete [] buffer;
		return -1;
	}
	fclose(file);
	int result = CreateBitmapFromPNGData(buffer, fileLength, outImageInfo, leaveIndexed, target_gamma);
	delete [] buffer;
	return result;
}

int		CreateBitmapFromPNGData(const void * inStart, int inLength, struct ImageInfo * outImageInfo, bool leaveIndexed, float target_gamma)
{
	png_uint_32	width, height;
	int bit_depth,color_type,interlace_type,compression_type,P_filter_type;
	double lcl_gamma, screen_gamma;

	png_structp		pngPtr = NULL;
	png_infop		infoPtr = NULL;
	outImageInfo->data = NULL;
	char** 			rows = NULL;

	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,(png_voidp)NULL,my_error,my_warning);
	if(!pngPtr) goto bail;

	infoPtr=png_create_info_struct(pngPtr);
	if(!infoPtr) goto bail;

	png_start_pos = (const char *) inStart;
	png_current_pos = (const char *) inStart;
	png_end_pos = (const char *) inStart + inLength;

	if (png_sig_cmp((unsigned char *) png_current_pos,0,8)) goto bail;

	png_set_interlace_handling(pngPtr);

	if(setjmp(png_jmpbuf(pngPtr)))
	{
		goto bail;
	}

	png_init_io      (pngPtr,NULL						);
	png_set_read_fn  (pngPtr,NULL,png_buffered_read_func);
	png_set_sig_bytes(pngPtr,8							);	png_current_pos+=8;
	png_read_info	 (pngPtr,infoPtr					);

	png_get_IHDR(pngPtr,infoPtr,&width,&height,
			&bit_depth,&color_type,&interlace_type,
			&compression_type,&P_filter_type);

	outImageInfo->width = width;
	outImageInfo->height = height;


	if(target_gamma)
	{
		if(  png_get_gAMA (pngPtr,infoPtr     ,&lcl_gamma))		// Perhaps the file has its gamma recorded, for example by photoshop. Just tell png to callibrate for our hw platform.
			 png_set_gamma(pngPtr,target_gamma, lcl_gamma);
		else png_set_gamma(pngPtr,target_gamma, 1.0/1.8  );		// If the file doesn't have gamma, assume it was drawn on a Mac - true for really old stuff.
	}

	if(color_type==PNG_COLOR_TYPE_PALETTE && bit_depth<= 8)if (!leaveIndexed)	png_set_expand	  (pngPtr);
	if(color_type==PNG_COLOR_TYPE_GRAY    && bit_depth<  8)						png_set_expand	  (pngPtr);
	if(png_get_valid(pngPtr,infoPtr,PNG_INFO_tRNS)        )						png_set_expand	  (pngPtr);
	if(										 bit_depth==16)						png_set_strip_16  (pngPtr);
	if(										 bit_depth<  8)						png_set_packing	  (pngPtr);
	if(            color_type==PNG_COLOR_TYPE_GRAY		  )if (!leaveIndexed)	png_set_gray_to_rgb (pngPtr);
	if(            color_type==PNG_COLOR_TYPE_GRAY_ALPHA  )if (!leaveIndexed)	png_set_gray_to_rgb (pngPtr);
	switch(color_type) {
	case PNG_COLOR_TYPE_GRAY:		outImageInfo->channels = leaveIndexed ? 1 : 3;		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:	outImageInfo->channels = leaveIndexed ? 2 : 4;		break;
	case PNG_COLOR_TYPE_PALETTE:	outImageInfo->channels = leaveIndexed ? 1 : 3;		break;
	case PNG_COLOR_TYPE_RGB:		outImageInfo->channels = 					3;		break;
	case PNG_COLOR_TYPE_RGBA:		outImageInfo->channels = 					4;		break;
	default: goto bail;
	}

	// Some pngs have PNG_INFO_tRNS as a transparent index color...since we set "expansion" on this,
	// we need to update our channel count; lib png is going to write rgba data.
	if(!leaveIndexed && png_get_valid(pngPtr,infoPtr,PNG_INFO_tRNS) && outImageInfo->channels == 3)
		outImageInfo->channels = 4;

	// Since we use "BGR" conventions ask PNG to just swap red-blue for us.
	png_set_bgr(pngPtr);
	png_read_update_info(pngPtr,infoPtr);

	outImageInfo->pad = 0;
	outImageInfo->data = (unsigned char *) malloc(outImageInfo->width * outImageInfo->height * outImageInfo->channels);
	if (!outImageInfo->data) goto bail;

	rows=(char**)malloc(height*sizeof(char*));
	if (!rows) goto bail;

	// Set our rows to reverse order to flip the image.
	for(int i=0;i<height;i++)
	{
		rows[i]=(char*)outImageInfo->data     +((outImageInfo->height-1-i)*(outImageInfo->width)*(outImageInfo->channels));
	}

	png_read_image(pngPtr,(png_byte**)rows);										// Now we just tell pnglib to read in the data.  When done our row ptrs will be filled in.
	free(rows);
	rows = NULL;

	png_destroy_read_struct(&pngPtr,(png_infopp)&infoPtr,(png_infopp)NULL);

	return 0;
bail:

	if (pngPtr && infoPtr)		png_destroy_read_struct(&pngPtr,(png_infopp)&infoPtr,(png_infopp)NULL);
	else if (pngPtr)			png_destroy_read_struct(&pngPtr,(png_infopp)NULL,(png_infopp)NULL);
	if (outImageInfo->data)		free(outImageInfo->data);
	outImageInfo->data = 0;
	if (rows) 					free(rows);

	return -1;

}

int		WriteBitmapToPNG(const struct ImageInfo * inImage, const char * inFilePath, char * inPalette, int inPaletteLen, float target_gamma)
{
	FILE *		file = NULL;
	png_structp	png_ptr = NULL;
	png_infop	info_ptr = NULL;
	char **		rows = NULL;

	file = fopen(inFilePath, "wb");
	if (!file) goto bail;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,(png_voidp)NULL,my_error,my_warning);
    if (!png_ptr) goto bail;

	info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) goto bail;

	if (setjmp(png_jmpbuf(png_ptr))) goto bail;

	png_init_io(png_ptr, file);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	png_set_compression_mem_level(png_ptr, 8);
	png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png_ptr, 15);
	png_set_compression_method(png_ptr, 8);
	png_set_compression_buffer_size(png_ptr, 8192);
	if(target_gamma)
		png_set_gAMA(png_ptr, info_ptr, 1.0 / target_gamma);

	png_set_bgr(png_ptr);

	png_set_PLTE(png_ptr, info_ptr, (png_colorp) inPalette, inPaletteLen);

    png_set_IHDR(png_ptr, info_ptr, inImage->width, inImage->height, 8,
    	(inImage->channels == 1) ? (inPalette ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY) :
    	((inImage->channels == 3) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA),
    	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);


	rows=(char**)malloc(inImage->height*sizeof(char*));
	if (!rows) goto bail;

	for(int i=0;i<inImage->height;i++)
	{
		rows[i]=(char*)inImage->data+((inImage->height-1-i)*((inImage->width)*(inImage->channels)+inImage->pad));
	}

	png_write_image(png_ptr,(png_byte**)rows);
	free(rows);
	rows = NULL;

	png_write_end(png_ptr, info_ptr);

    fclose(file);
	file = NULL;
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return 0;

bail:

	if (png_ptr && info_ptr)	png_destroy_write_struct(&png_ptr, &info_ptr);
	else if (png_ptr)			png_destroy_write_struct(&png_ptr, NULL);
	if (file)					fclose(file);
	if (rows)					free(rows);

	return -1;

}

#if USE_TIF

static	void	IgnoreTiffWarnings(const char *, const char*, va_list)
{
}

// TIFF is 0,0 = lower left.  But the byte order is ENDIAN dependent.
// BIG ENDIAN: we get ABGR
// LIL ENDIAN: we get RGBA
int		CreateBitmapFromTIF(const char * inFilePath, struct ImageInfo * outImageInfo)
{
	int result = -1;
	TIFFErrorHandler	errH = TIFFSetWarningHandler(IgnoreTiffWarnings);
	TIFFErrorHandler	errH2= TIFFSetErrorHandler(IgnoreTiffWarnings);
    TIFF* tif = TIFFOpen(inFilePath, "r");
    if (tif == NULL) goto bail;

	uint32 w, h;
	uint16 cc;
	size_t npixels;
	uint32* raster;

	
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &cc);

	npixels = w * h;
	raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
	if (raster != NULL) {
	    if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {

			outImageInfo->data = (unsigned char *) malloc(npixels * 4);
			outImageInfo->width = w;
			outImageInfo->height = h;
			outImageInfo->channels = 4;
			outImageInfo->pad = 0;
			int	count = outImageInfo->width * outImageInfo->height;
			unsigned char * d = outImageInfo->data;
			unsigned char * s = (unsigned char *) raster;
			while (count--)
			{
#if BIG
				d[0] = s[1];	// B
				d[1] = s[2];	// G
				d[2] = s[3];	// R
				d[3] = s[0];	// A
#elif LIL
				d[0] = s[2];	// B
				d[1] = s[1];	// G
				d[2] = s[0];	// R
				d[3] = s[3];	// A
#else
	#error PLATFORM NOT DEFINED
#endif
				s += 4;
				d += 4;
			}
			result = 0;
	    }
	    _TIFFfree(raster);
	}
	TIFFClose(tif);
	TIFFSetWarningHandler(errH);
    return result;
bail:
	TIFFSetWarningHandler(errH);
	return -1;
}

#endif

#if USE_GEOJPEG2K
int CreateBitmapFromJP2K(const char * inFilePath, struct ImageInfo * outImageInfo)
{
	//Clean out the image info
	outImageInfo->data = NULL;
	
	//Initialize JasPerGEO
	if(jas_init() != 0 )
	{
		//If it failed then return error
		return -1;
	}

	//Create the data stream
	jas_stream_t * inStream;
	
	//If the data stream cannot be created
	if((inStream = jas_stream_fopen(inFilePath,"rb"))==false)
	{
		return -1;
	}

	//Get the format ID
	int formatId;

	//If there are any errors in getting the format
	if((formatId = jas_image_getfmt(inStream)) < 0)
	{
		//It is an invalid format
		return -2;
	}
	
	
	//Create an image from the stream
	jas_image_t * image;

	//If the image cannot be decoded
	if((image = jas_image_decode(inStream, formatId, 0)) == false)
	{
		//Return an error
		return -3;
	}

	//Set the properties of the outImageInfo that we can
	outImageInfo->width = jas_image_width(image);
	outImageInfo->height = jas_image_height(image);
	outImageInfo->pad = 0;

	//Get the red green and blue of each image
	int channels[4] = {	
		jas_image_getcmptbytype(image, JAS_IMAGE_CT_RGB_B),
		jas_image_getcmptbytype(image, JAS_IMAGE_CT_RGB_G),
		jas_image_getcmptbytype(image, JAS_IMAGE_CT_RGB_R), 
		jas_image_getcmptbytype(image, JAS_IMAGE_CT_OPACITY) };

	if(image->numcmpts_ > 3 && channels[3] == -1)
		channels[3] = 3;


	//If it has 3 channels it will later be filled in the DSF export, else it is filled in with it's values
	outImageInfo->channels = channels[3] != -1 ? 4 : 3;


	//Allocate a place in memory equal to the width*height*channels, aka just right
	outImageInfo->data = (unsigned char*) malloc(outImageInfo->width*outImageInfo->height*outImageInfo->channels);

		
	//If the precision is more than 8 create shift values
	
	for(int chan_idx = 0; chan_idx < outImageInfo->channels; ++ chan_idx)
	{
		int chan_id = channels[chan_idx];
		int chan_shift = max(jas_image_cmptprec(image, chan_id) - 8, 0);

		jas_matrix_t * comp = jas_matrix_create(outImageInfo->height, outImageInfo->width);

		jas_image_readcmpt(image, chan_id, 0, 0, outImageInfo->width, outImageInfo->height, comp);
	
		//Save the original pointer
		unsigned char * p = outImageInfo->data + chan_idx;

		//For the width and height of the image
		//(This way makes sure the image is of the correct orientation
		for (int j = outImageInfo->height - 1; j >= 0; j--) 
		{
			for (int i = 0; i < outImageInfo->width; i++) 
			{
				int px = jas_matrix_get(comp, j, i) >> chan_shift;
				
				*p = px;
				p += outImageInfo->channels;				
			}
		}
		jas_matrix_destroy(comp);
		
	}
	//Clean up jas_stuff. Since we havea working image 
	jas_cleanup();

	return 0;
}
#endif

static void	in_place_scaleXY(int x, int y, unsigned char * src, unsigned char * dst, int channels)
{
	int rb = x * channels;
	unsigned char *	s1 = src;
	unsigned char *	s2 = src + rb;
	unsigned char * d1 = dst;

	x /= 2;
	y /= 2;

	int t1,t2,t3,t4;
	while(y--)
	{
		int ctr=x;
		while(ctr--)
		{
			t1=t2=t3=t4=0;
			t1 += *s1++;	if(channels>1)t2 += *s1++;		if(channels>2)t3 += *s1++;		if(channels>3)t4 += *s1++;
			t1 += *s1++;	if(channels>1)t2 += *s1++;		if(channels>2)t3 += *s1++;		if(channels>3)t4 += *s1++;
			t1 += *s2++;	if(channels>1)t2 += *s2++;		if(channels>2)t3 += *s2++;		if(channels>3)t4 += *s2++;
			t1 += *s2++;	if(channels>1)t2 += *s2++;		if(channels>2)t3 += *s2++;		if(channels>3)t4 += *s2++;
			t1 >>= 2;		if(channels>1)t2 >>= 2;			if(channels>2)t3 >>= 2;			if(channels>3)t4 >>= 2;
			*d1++ = t1;		if(channels>1)*d1++ = t2;		if(channels>2)*d1++ = t3;		if(channels>3)*d1++ = t4;

		}
		s1 += rb;
		s2 += rb;
	}
}

static void	in_place_scaleX(int x, int y, unsigned char * src, unsigned char * dst, int channels)
{
	unsigned char *	s1 = src;
	unsigned char * d1 = dst;

	x /= 2;

	int t1,t2,t3,t4;
	int ctr = x * y;
	while(ctr--)
	{
		t1=t2=t3=t4=0;
		t1 += *s1++;		if(channels>1)t2 += *s1++;		if(channels>2)t3 += *s1++;		if(channels>3)t4 += *s1++;
		t1 += *s1++;		if(channels>1)t2 += *s1++;		if(channels>2)t3 += *s1++;		if(channels>3)t4 += *s1++;
		t1 >>= 1;			if(channels>1)t2 >>= 1;			if(channels>2)t3 >>= 1;			if(channels>3)t4 >>= 1;
		*d1++ = t1;			if(channels>1)*d1++ = t2;		if(channels>2)*d1++ = t3;		if(channels>3)*d1++ = t4;
	}
}


static void	in_place_scaleY(int x, int y, unsigned char * src, unsigned char * dst, int channels)
{
	int rb = x * channels;
	unsigned char *	s1 = src;
	unsigned char *	s2 = src + rb;
	unsigned char * d1 = dst;

	x /= 2;
	y /= 2;

	int t1,t2,t3,t4;
	while(y--)
	{
		int ctr=x;
		while(ctr--)
		{
			t1=t2=t3=t4=0;
			t1 += *s1++;		if(channels>1)t2 += *s1++;		if(channels>2)t3 += *s1++;		if(channels>3)t4 += *s1++;
			t1 += *s2++;		if(channels>1)t2 += *s2++;		if(channels>2)t3 += *s2++;		if(channels>3)t4 += *s2++;
			t1 >>= 1;			if(channels>1)t2 >>= 1;			if(channels>2)t3 >>= 1;			if(channels>3)t4 >>= 1;
			*d1++ = t1;			if(channels>1)*d1++ = t2;		if(channels>2)*d1++ = t3;		if(channels>3)*d1++ = t4;

		}
		s1 += rb;
		s2 += rb;
	}
}

static void copy_mip_with_filter(const ImageInfo& src, ImageInfo& dst,int level, unsigned char (* filter)(unsigned char src[], int count, int channel, int level))
{
	unsigned char temp_buf[4];	// Enough storage for RGBA 2x2
	int xr = src.width == dst.width ? 1 : 2;
	int yr = src.height == dst.height ? 1 : 2;

	int srb = src.width * src.channels + src.pad;
	int drb = dst.width * dst.channels + dst.pad;

	for(int y = 0; y < dst.height; ++y)
	for(int x = 0; x < dst.width; ++x)
	{
		for(int c = 0; c < src.channels; ++c)
		{
			int ns = 0;
			for(int dy = 0; dy < yr; ++dy)
			for(int dx = 0; dx < xr; ++dx)
			{
				temp_buf[ns++] = src.data[(y * yr + dy) * srb + (x * xr + dx) * src.channels + c];
			}
			dst.data[y * drb + x * dst.channels + c] =
				filter(temp_buf,ns,c,level);
		}
	}
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

// This routine swaps Y and BGRA on desktop, but only BGRA on phone.
static void swap_bgra_y(struct ImageInfo& i)
{
	int num_swaps = (i.height+1) / 2;	// add 1 - if we have 9 lines, do 5 swaps, line 4 swaps on itself safely.
	for(int y=0;y<num_swaps         ;++y)
	for(int x=0;x<i.width    ; ++x)
	{
		unsigned char * srcp = i.data + x * i.channels + (i.height - y - 1) * (i.channels * i.width + i.pad);
		unsigned char * dstp = i.data + x * i.channels +				   y	  * (i.channels * i.width + i.pad);

		swap(dstp[2], dstp[0]);
		if(srcp != dstp)				// check for self-swap, don't undo, also don't waste time.
		{
			swap(srcp[0], srcp[2]);		// This swaps BGRA to RGBA

// On mobile devices, we pre-encode DXT with 0,0 = lower left so the phone doesn't have to flip the DDS before feeding it into OpenGL.
// This will look upside down on all viewers.
#if !PHONE
			for(int c = 0; c < i.channels; ++c)	// This flips the image.
				swap(srcp[c], dstp[c]);
#endif
		}
	}

}

// Compressed DDS.
int	WriteBitmapToDDS(struct ImageInfo& ioImage, int dxt, const char * file_name, int use_win_gamma)
{
	Assert(ioImage.channels == 4);//Your number of channels better equal 4 or else
	FILE * fi = fopen(file_name,"wb");
	if (fi == NULL) return -1;
	vector<unsigned char>	src_v, dst_v;
	int flags = (dxt == 1 ? squish::kDxt1 : (dxt == 3 ? squish::kDxt3 : squish::kDxt5));
	dst_v.resize(squish::GetStorageRequirements(ioImage.width,ioImage.height,flags));
	unsigned char * dst_mem = &*dst_v.begin();

	int x = ioImage.width;
	int y = ioImage.height;
	int mips=1;
	while(x > 1 || y > 1)
	{
		x >>= 1;
		y >>= 1;
		++mips;
	}

	struct ImageInfo img(ioImage);

	int len = squish::GetStorageRequirements(img.width,img.height,flags);


	TEX_dds_desc header = { 0 };
	header.dwMagic[0] = 'D';
	header.dwMagic[1] = 'D';
	header.dwMagic[2] = 'S';
	header.dwMagic[3] = ' ';
	header.dwSize = SWAP32(sizeof(header)-sizeof(header.dwMagic));
	header.dwFlags = SWAP32(DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_MIPMAPCOUNT|DDSD_LINEARSIZE);
	header.dwHeight = SWAP32(ioImage.height);
	header.dwWidth = SWAP32(ioImage.width);
	header.dwLinearSize=SWAP32(len);
	header.dwDepth=0;
	header.dwMipMapCount=SWAP32(mips);
	header.ddpfPixelFormat.dwSize=SWAP32(sizeof(header.ddpfPixelFormat));
	header.ddpfPixelFormat.dwFlags=SWAP32(DDPF_FOURCC);
	header.ddpfPixelFormat.dwFourCC[0]='D';
	header.ddpfPixelFormat.dwFourCC[1]='X';
	header.ddpfPixelFormat.dwFourCC[2]='T';
	header.ddpfPixelFormat.dwFourCC[3]='0' + dxt;
	if(use_win_gamma)
		header.ddsCaps.dwCaps=SWAP32(DDSCAPS_TEXTURE|DDSCAPS_MIPMAP				   );
	else
		header.ddsCaps.dwCaps=SWAP32(DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);

	fwrite(&header,sizeof(header),1,fi);

	do {

		// Get the image into RGBA upper left origin, that's what Squish/DXT/DDS wants.
		swap_bgra_y(img);

		squish::CompressImage(img.data, img.width, img.height, dst_mem, flags|squish::kColourIterativeClusterFit);
		len = squish::GetStorageRequirements(img.width,img.height,flags);
		fwrite(dst_mem,len,1,fi);

		// Put it back before we advance...really necessary??!
		swap_bgra_y(img);

		if(!AdvanceMipmapStack(&img))
			break;

	} while (1);

	fclose(fi);
	return 0;
	// close file
}

// Uncomp: write BGR or BGRA, origin depends on phone or desktop - see below.
int	WriteUncompressedToDDS(struct ImageInfo& ioImage, const char * file_name, int use_win_gamma)
{
	FILE * fi = fopen(file_name,"wb");
	if (fi == NULL) return -1;

	int x = ioImage.width;
	int y = ioImage.height;
	int mips=1;
	while(x > 1 || y > 1)
	{
		x >>= 1;
		y >>= 1;
		++mips;
	}

	TEX_dds_desc header = { 0 };
	header.dwMagic[0] = 'D';
	header.dwMagic[1] = 'D';
	header.dwMagic[2] = 'S';
	header.dwMagic[3] = ' ';
	header.dwSize = SWAP32(sizeof(header)-sizeof(header.dwMagic));
	header.dwFlags = SWAP32(DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_MIPMAPCOUNT|DDSD_PITCH);
	header.dwHeight = SWAP32(ioImage.height);
	header.dwWidth = SWAP32(ioImage.width);
	header.dwLinearSize = SWAP32(ioImage.width * ioImage.channels);
	header.dwDepth=0;
	header.dwMipMapCount=SWAP32(mips);
	header.ddpfPixelFormat.dwSize=SWAP32(sizeof(header.ddpfPixelFormat));

	if(ioImage.channels == 1)
	{
		header.ddpfPixelFormat.dwFlags=SWAP32(DDPF_ALPHAPIXELS);
		header.ddpfPixelFormat.dwRGBBitCount=SWAP32(8);
		header.ddpfPixelFormat.dwRBitMask=SWAP32(0x0);
		header.ddpfPixelFormat.dwGBitMask=SWAP32(0x0);
		header.ddpfPixelFormat.dwBBitMask=SWAP32(0x0);
		header.ddpfPixelFormat.dwRGBAlphaBitMask=SWAP32(0xFF);
	}
	else
	{
		header.ddpfPixelFormat.dwFlags=SWAP32((ioImage.channels==3 ? DDPF_RGB : (DDPF_RGB|DDPF_ALPHAPIXELS)));
		header.ddpfPixelFormat.dwRGBBitCount=SWAP32(ioImage.channels==3 ? 24 : 32);
		header.ddpfPixelFormat.dwRBitMask=SWAP32(0x00FF0000);
		header.ddpfPixelFormat.dwGBitMask=SWAP32(0x0000FF00);
		header.ddpfPixelFormat.dwBBitMask=SWAP32(0x000000FF);		// Little endian: B is first, FF is first byte.
		header.ddpfPixelFormat.dwRGBAlphaBitMask=SWAP32(0xFF000000);
	}

	if(use_win_gamma)
		header.ddsCaps.dwCaps=SWAP32(DDSCAPS_TEXTURE|DDSCAPS_MIPMAP				   );
	else
		header.ddsCaps.dwCaps=SWAP32(DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);

	fwrite(&header,sizeof(header),1,fi);

	struct ImageInfo im(ioImage);

	do {
	// On the phone, feed image in lower left = 0,0, looks upside down on std tool chain, but saves swap for DD->GL conventions.
	#if !PHONE
		FlipImageY(im);
	#endif
		fwrite(im.data,im.width*im.height*im.channels,1,fi);
	#if !PHONE
		FlipImageY(im);
	#endif
		if (!AdvanceMipmapStack(&im))
			break;


	} while (1);
	fclose(fi);
	return 0;
	// close file
}

int		CreateBitmapFromDDS(const char * inFilePath, struct ImageInfo * outImageInfo)
{
	FILE * fi = fopen(inFilePath, "rb");
	unsigned char * raw = NULL;
	outImageInfo->data = NULL;

	if(fi==NULL) return -1;
	TEX_dds_desc header = { 0 };
	if (fread(&header, 1, sizeof(header), fi) != sizeof(header)) goto bail;

	header.dwSize = SWAP32(header.dwSize);
	header.dwFlags = SWAP32(header.dwFlags);
	header.dwHeight = SWAP32(header.dwHeight);
	header.dwWidth = SWAP32(header.dwWidth);
	header.dwLinearSize = SWAP32(header.dwLinearSize);
	header.dwMipMapCount=SWAP32(header.dwMipMapCount);
	header.ddpfPixelFormat.dwSize=SWAP32(header.ddpfPixelFormat.dwSize);
	header.ddpfPixelFormat.dwFlags=SWAP32(header.ddpfPixelFormat.dwFlags);
	header.ddpfPixelFormat.dwRGBBitCount=SWAP32(header.ddpfPixelFormat.dwRGBBitCount);
	header.ddpfPixelFormat.dwRBitMask=SWAP32(header.ddpfPixelFormat.dwRBitMask);
	header.ddpfPixelFormat.dwGBitMask=SWAP32(header.ddpfPixelFormat.dwGBitMask);
	header.ddpfPixelFormat.dwBBitMask=SWAP32(header.ddpfPixelFormat.dwBBitMask);
	header.ddpfPixelFormat.dwRGBAlphaBitMask=SWAP32(header.ddpfPixelFormat.dwRGBAlphaBitMask);

	if (header.dwMagic[0] != 'D' ||
		header.dwMagic[1] != 'D' ||
		header.dwMagic[2] != 'S' ||
		header.dwMagic[3] != ' ') goto bail;


	if(header.ddpfPixelFormat.dwFlags & DDPF_RGB)
	{
		outImageInfo->channels = (header.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) ? 4 : 3;
		outImageInfo->width = header.dwWidth;
		outImageInfo->height = header.dwHeight;
		outImageInfo->pad = (header.dwLinearSize == 0) ? 0 : (header.dwLinearSize - outImageInfo->width * outImageInfo->channels);
		int im_len = (outImageInfo->width * outImageInfo->channels + outImageInfo->pad) * outImageInfo->height;
		outImageInfo->data = (unsigned char *) malloc(im_len);
		if(outImageInfo->data==NULL) goto bail;

		if(fread(outImageInfo->data, 1, im_len, fi) != im_len) goto bail;
		FlipImageY(*outImageInfo);

		fclose(fi);
		return 0;
	}
	else if (header.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
	{
		int sflags;
		if(header.ddpfPixelFormat.dwFourCC[0] != 'D' || header.ddpfPixelFormat.dwFourCC[1] != 'X' || header.ddpfPixelFormat.dwFourCC[2] != 'T') goto bail;
		switch(header.ddpfPixelFormat.dwFourCC[3]) {
		case '1':		sflags = squish::kDxt1;	break;
		case '3':		sflags = squish::kDxt3;	break;
		case '5':		sflags = squish::kDxt5;	break;
		default: goto bail;
		}
		outImageInfo->channels = 4;
		outImageInfo->width = header.dwWidth;
		outImageInfo->height = header.dwHeight;
		outImageInfo->pad = 0;
		int im_len = (outImageInfo->width * outImageInfo->channels + outImageInfo->pad) * outImageInfo->height;
		outImageInfo->data = (unsigned char *) malloc(im_len);
		if(outImageInfo->data==NULL) goto bail;
		int im_raw = squish::GetStorageRequirements(header.dwWidth,header.dwHeight, sflags);

		raw = (unsigned char *) malloc(im_raw);
		if(raw==NULL) goto bail;
		if(fread(raw, 1, im_raw, fi) != im_raw) goto bail;
		squish::DecompressImage( outImageInfo->data, header.dwWidth, header.dwHeight, raw, sflags);
		int count = header.dwWidth * header.dwHeight;
		unsigned char * p = outImageInfo->data;
		while(count--)
		{
			swap(p[0],p[2]);
			p += 4;
		}
		FlipImageY(*outImageInfo);
		free(raw);
		fclose(fi);
		return 0;
	}


bail:
	if (fi) fclose(fi);
	if (outImageInfo->data) free(outImageInfo->data);
	outImageInfo->data = 0;
	if(raw) free(raw);
	return -1;
}

inline void swap_mem(unsigned char * p1, unsigned char * p2, int len)
{
	while(len--)
	{
		register unsigned char a = *p1;
		register unsigned char b = *p2;
		*p1 = b;
		*p2 = a;
		++p1;
		++p2;
	}
}

void	FlipImageY(struct ImageInfo&	io_image)
{
	int y_stop = io_image.height / 2;
	int row_len = io_image.width * io_image.channels + io_image.pad;
	for(int y1 = 0; y1 < y_stop; ++y1)
	{
		int y2 = io_image.height - y1 - 1;
		swap_mem(io_image.data + y1 * row_len,io_image.data + y2 * row_len, row_len);
	}
}


int MakeMipmapStack(struct ImageInfo * ioImage)
{
	int storage = 0;
	int mips = 0;
	int x = ioImage->width;
	int y = ioImage->height;
	do {
		storage += (x * y * ioImage->channels);
		++mips;
		if(x == 1 && y == 1) break;
		if (x > 1) x >>= 1;
		if (y > 1) y >>= 1;
	} while (1);

	unsigned char * base = (unsigned char *) malloc(storage);

	ImageInfo ni;
	ni.width = ioImage->width;
	ni.height = ioImage->height;
	ni.pad = 0;
	ni.channels = ioImage->channels;
	ni.data = base;

	CopyBitmapSectionDirect(*ioImage, ni, 0, 0, 0, 0, ni.width, ni.height);

	while(ni.width > 1 || ni.height > 1)
	{
		unsigned char * old_ptr = ni.data;
		ni.data += (ni.channels * ni.width * ni.height);

		if(ni.width > 1) {
			if (ni.height > 1)		in_place_scaleXY(ni.width,ni.height,old_ptr,ni.data,ni.channels);
			else					in_place_scaleX (ni.width,ni.height,old_ptr,ni.data,ni.channels);
		} else if (ni.height > 1)	in_place_scaleY (ni.width,ni.height,old_ptr,ni.data,ni.channels);

		if(ni.width > 1) ni.width >>= 1;
		if(ni.height > 1) ni.height >>= 1;
	}

	free(ioImage->data);
	ioImage->data = base;

	return mips;
}

int MakeMipmapStackFromImage(struct ImageInfo * ioImage)
{
//	if(ioImage->channels == 3)
//		ConvertBitmapToAlpha(ioImage, false);
	int storage = 0;
	int mips = 0;
	int x = ioImage->width;
	int y = ioImage->height;
	do {
		storage += (x * y *ioImage->channels);
		++mips;
		if(x == 1 && y == 1) break;
		if (x > 1) x >>= 1;
		if (y > 1) y >>= 1;
	} while (1);

	unsigned char * base = (unsigned char *) malloc(storage);

	ImageInfo ni;
	ni.width = ioImage->width / 2;
	ni.height = ioImage->height;
	ni.pad = 0;
	ni.channels = ioImage->channels;
	ni.data = base;

	int xo = 0;

	do {

		CopyBitmapSectionDirect(*ioImage, ni, xo, 0, 0, 0, ni.width, ni.height);

		xo += ni.width;

		if (!AdvanceMipmapStack(&ni))
			break;
	} while(1);

	free(ioImage->data);
	ioImage->data = base;

	ioImage->width /= 2;

	return mips;
}

int MakeMipmapStackWithFilter(struct ImageInfo * ioImage, unsigned char (* filter)(unsigned char src[], int count, int channel, int level))
{
	int storage = 0;
	int mips = 0;
	int x = ioImage->width;
	int y = ioImage->height;
	do {
		storage += (x * y * ioImage->channels);
		++mips;
		if(x == 1 && y == 1) break;
		if (x > 1) x >>= 1;
		if (y > 1) y >>= 1;
	} while (1);

	unsigned char * base = (unsigned char *) malloc(storage);

	ImageInfo ni;
	ni.width = ioImage->width;
	ni.height = ioImage->height;
	ni.pad = 0;
	ni.channels = ioImage->channels;
	ni.data = base;

	CopyBitmapSectionDirect(*ioImage, ni, 0, 0, 0, 0, ni.width, ni.height);
	int level = 0;

	while(ni.width > 1 || ni.height > 1)
	{
		ImageInfo sd(ni);
		sd.data += (ni.channels * ni.width * ni.height);
		if(sd.width > 1) sd.width >>= 1;
		if(sd.height > 1) sd.height >>= 1;

		copy_mip_with_filter(ni,sd,level,filter);
		ni=sd;
		++level;
	}

	free(ioImage->data);
	ioImage->data = base;

	return mips;
}

int AdvanceMipmapStack(struct ImageInfo * ioImage)
{
	if(ioImage->width == 1 && ioImage->height == 1) return 0;

	ioImage->data += (ioImage->channels * ioImage->width * ioImage->height);
	if(ioImage->width > 1) ioImage->width >>= 1;
	if(ioImage->height > 1) ioImage->height >>= 1;
	return 1;
}


