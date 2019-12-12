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
#ifndef _BitmapUtils_h_
#define _BitmapUtils_h_

/* Technically sRGB is a gamma of 2.4 plus some weird stuff on the end, but 2.2 is often a good
 * proxy if we just have to pick a number.  So the new nominal work-flow for X-Plane is: author
 * in sRGB, mark the file as 2.2, and X-Plane will then not molest the image and show it
 * on your sRGB display.  So define this here for 80 places in the code that just need to use
 * "the right gamma for modern computers." */
#define GAMMA_SRGB 2.2f

/*
	This is our in memory way of storing an image.  Data is a pointer
	to an array of bytes large enough to hold the image.  We always
	use 8-bit alpha, 24-bit BGR or 32-bit BGRA.  The lower left corner of the 
	image file is in the first byte of data.  (Those naming conventions are
	"OpenGL" conventions, so BGR means first byte is blue, regardless of
	machine endian-ness.)

	Pad is how many bytes we skip at the end of each scanline.  Each
	scanline must start on a 4-byte boundary!
*/

//Supported image types	//CreateBitmapFromX       WriteBitmapToX	Alpha?
enum SupportedTypes{
	WED_BMP,			//NewBitmap,FromFile            X			
	WED_DDS,			//X								X				X
//	WED_JP2K,			//X												X
	WED_JPEG,			//FromJPEG,FromJPEGData
	WED_PNG,			//X								X				X
	WED_TIF				//X												X
};
struct	ImageInfo {
	unsigned char *	data;
	long			width;
	long			height;
	long			pad;
	short			channels;
};

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

#if BIG
	#if APL
		#include <libkern/OSByteOrder.h>
		#define SWAP32(x) (OSSwapConstInt32(x))
	#else
		#error we do not have big endian support on non-Mac platforms
	#endif
#elif LIL
	#define SWAP32(x) (x)
#else
	#error BIG or LIL are not defined - what endian are we?
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

	TEX_dds_desc(int width, int height, int mips, int dxt);
};

/* STANDARDS FOR CreateNewBitmapFromX: Returns 0 for all good, else each has its
 * own error codes we all wish were documented and standardized*/

/* This routine creates a new bitmap and fills in an uninitialized imageInfo structure.
 * The contents of the bitmap are undetermined and must be 'cleared' by you. */
int		CreateNewBitmap(long inWidth, long inHeight, short inChannels, struct ImageInfo * outImageInfo);

/* Given a file path and an uninitialized imageInfo structure, this routine fills
 * in the imageInfo structure by loading the bitmap. */
 
 // should really be called "CreateBitmapFromBMP"
int		CreateBitmapFromFile(const char * inFilePath, struct ImageInfo * outImageInfo);

/* Yada yada yada, libPNG.  Gamma is the gamma color curve we want our pixels in.  Since gamma is recorded on the png file
 * we have to tell libpng to convert it.  Use 0.0 for no conversion, just the raw 8-bit values.  */
int		CreateBitmapFromPNG(const char * inFilePath, struct ImageInfo * outImageInfo, bool leaveIndexed, float target_gamma, bool flipY = true);
int		CreateBitmapFromPNGData(const void * inBytes, int inLength, struct ImageInfo * outImageInfo, bool leaveIndexed, float target_gamma, bool flipY = true);

/* Create a 4-channel image from a DDS file. */
int		CreateBitmapFromDDS(const char * inFilePath, struct ImageInfo * outImageInfo);

#if USE_JPEG

/* Create a JPEG image from either a file on disk or a chunk of memory.  This requires the IJG reference
 * JPEG code. */
int		CreateBitmapFromJPEG(const char * inFilePath, struct ImageInfo * outImageInfo, bool flipY = true); // image flip is costly wrt execution time. Avoid.
int		CreateBitmapFromJPEGData(void * inBytes, int inLength, struct ImageInfo * outImageInfo);

#endif

#if USE_TIF

/* Create an image from a TIF file  requires libTIFF. */
int		CreateBitmapFromTIF(const char * inFilePath, struct ImageInfo * outImageInfo, bool flipY = true);

#endif

// load any supported filetype, repardless of file suffix. Return zero upon success
int LoadBitmapFromAnyFile(const char * inFilePath, ImageInfo * outImage, bool flipY = true);

/* Given an imageInfo structure, this routine writes it to disk as a .bmp file.
 * Note that only 3-channel bitmaps may be written as .bmp files!! 
 *
 * Beware! Trying to use more than 3 channels or an image that has the wrong padding
 * Will fail! The following must be true ((width * channels + pad) % 4) == 0 and channels must be 3*/
int		WriteBitmapToFile(const struct ImageInfo * inImage, const char * inFilePath);

/* Given an imageInfo structure, this routine writes it to disk as a .png file.  Image is tagged with gamma, or 0.0f to leave untagged. */
int		WriteBitmapToPNG(const struct ImageInfo * inImage, const char * inFilePath, char * inPalette, int inPaletteLen, float gamma);

/* This routine writes a 4 channel bitmap as a mip-mapped DXT1, DXT3 or DXT5 image.
 * NOTE: if you compile with PHONE then DDS are written upside down (lower left origin
 * instead of upper-left).  This is an optimization for the iphone, which can then
 * pass the data DIRECTLY to OpenGL. */
int	WriteBitmapToDDS(struct ImageInfo& ioImage, int dxt, const char * file_name, int use_win_gamma);

// same, but multi-threaded compression and gamma corrected mipmap generation is done within
int	WriteBitmapToDDS_MT(struct ImageInfo& ioImage, int dxt, const char * file_name);

/* This routine writes a 3 or 4 channel bitmap as a mip-mapped DXT1 or DXT3 image. */
int	WriteUncompressedToDDS(struct ImageInfo& ioImage, const char * file_name, int use_win_gamma);


/* This routine deallocates a bitmap that was created with CreateBitmapFromFile or
 * CreateNewBitmap. */
void	DestroyBitmap(const struct ImageInfo * inImageInfo);

//Put in a file path the image name at the end and get back -1 for error or a
//supported image code (see the enum SupportedTypes
int GetSupportedType(const char * path);

//Attempts to make a supported image type using GetSupportedType and the various CreateBitmapFromX utils
//Error codes are passed back up and returned by the method

// should really be called "CreateBitmapFromFileAccordingToSuffix"
// ********** DEPRECATED in favor of LoadBitmapFromAnyFile() ************
int MakeSupportedType(const char * path, ImageInfo * inImage);

/* Given a bitmap, this routine fills the whole bitmap in with a gray level of c, where
 * c = 0 means black and c = 255 means white. */
void	FillBitmap(const struct ImageInfo * inImageInfo, char c);

/* Given two bitmaps, this routine copies a section from one bitmap to another.
 * This routine will use bicubic and bilinear interpolation to copy the bitmap
 * as cleanly as possible.  However, if the bitmap contains alpha, the copy routine
 * will create a jagged edge to keep from smearing the alpha channel. */
void	CopyBitmapSection(
			const struct ImageInfo *	inSrc,
			const struct ImageInfo *	inDst,
			long				inSrcLeft,
			long				inSrcTop,
			long				inSrcRight,
			long				inSrcBottom,
			long				inDstLeft,
			long				inDstTop,
			long				inDstRight,
			long				inDstBottom);

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
			long				inDstBottom);

void	CopyBitmapSectionDirect(
			const struct ImageInfo&		inSrc,
			const struct ImageInfo&		inDst,
			long						inSrcLeft,
			long						inSrcTop,
			long						inDstLeft,
			long						inDstTop,
			long						inWidth,
			long						inHeight);

void	FlipImageY(struct ImageInfo&	io_image);

/* This routine rotates a bitmap counterclockwise 90 degrees, exchanging its width
 * and height. */
void	RotateBitmapCCW(
			struct ImageInfo *	ioBitmap);

/* This routine converts a 3-channel bitmap to a 4-channel bitmap by converting
 * magenta pixels to alpha. */
int	ConvertBitmapToAlpha(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha);

/* This routine converts a 4-channel bitmap to a 3-channel bitmap by converting
 * alpha back to magenta. */
int	ConvertAlphaToBitmap(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha);

/* This routine converts a 4-channel bitmap to a 3-channel bitmap by converting
 * alpha back to magenta. */
int	ConvertAlphaToBitmap(
			struct ImageInfo *		ioImage,
			bool					doMagentaAlpha,
			bool					doPadding);

/* This routine makes a "mipmap stack" from the source image by box-filtering it.
 * The number of consecutive images in the stack is returned. */
int MakeMipmapStack(struct ImageInfo * ioImage);

/* Same as above, but we take a "tree" of lower-left justified images...*/
int MakeMipmapStackFromImage(struct ImageInfo * ioImage);

/* Make a mip-map stack with a custom filter. */
int MakeMipmapStackWithFilter(struct ImageInfo * ioImage, unsigned char (* filter)(unsigned char src[], int count, int channel, int level));



/* This routine "advances" the ptr and sizes in the image to go to the next
 * mip-map.  This is ONLY safe if the image is a mip-map stack.  IMPORTANT: 
 * don't "free" the ptr that has been manipulated.  Returns 1 if there is another
 * image, 0 if there is not. */
int AdvanceMipmapStack(struct ImageInfo * ioImage);
 

#endif
