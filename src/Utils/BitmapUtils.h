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

/*
	WARNING: Struct alignment must be "68K" (e.g. 2-byte alignment) for these
	structures to be happy!!!
*/

/* 
	These headers match the Microsoft bitmap file and image headers more or less
	and are read right out of the file.
*/

#if APL
#pragma options align=mac68k
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
	This is our in memory way of storing an image.  Data is a pointer
	to an array of bytes large enough to hold the image.  We always
	use 24-bit RGB or 32-bit ARGB.  The lower left corner of the BMP
	file is in the first byte of data.
	
	Pad is how many bytes we skip at the end of each scanline.  Each
	scanline must start on a 4-byte boundary!
*/

struct	ImageInfo {
	unsigned char *	data;
	long			width;
	long			height;
	long			pad;
	short			channels;
};	

/* Given a file path and an uninitialized imageInfo structure, this routine fills
 * in the imageInfo structure by loading the bitmap. */
int		CreateBitmapFromFile(const char * inFilePath, struct ImageInfo * outImageInfo);

#if USE_JPEG

/* Same as above, but uses IJG JPEG code. */
int		CreateBitmapFromJPEG(const char * inFilePath, struct ImageInfo * outImageInfo);

/* Same as above, but create the image from an in-memory image of a JFIF file,
 * allows you to read the image yourself, or mem map it. */
int		CreateBitmapFromJPEGData(void * inBytes, int inLength, struct ImageInfo * outImageInfo);

#endif

/* Yada yada yada, libPng. */
int		CreateBitmapFromPNG(const char * inFilePath, struct ImageInfo * outImageInfo, bool leaveIndexed);
int		CreateBitmapFromPNGData(const void * inBytes, int inLength, struct ImageInfo * outImageInfo, bool leaveIndexed);

#if USE_TIF

/* Yada yada yada, libtiff. */
int		CreateBitmapFromTIF(const char * inFilePath, struct ImageInfo * outImageInfo);

#endif

/* Given an imageInfo structure, this routine writes it to disk as a .bmp file. 
 * Note that only 3-channel bitmaps may be written as .bmp files!! */
int		WriteBitmapToFile(const struct ImageInfo * inImage, const char * inFilePath);

/* Given an imageInfo structure, this routine writes it to disk as a .png file.  */
int		WriteBitmapToPNG(const struct ImageInfo * inImage, const char * inFilePath, char * inPalette, int inPaletteLen);


/* This routine creates a new bitmap and fills in an uninitialized imageInfo structure.
 * The contents of the bitmap are undetermined and must be 'cleared' by you. */
int		CreateNewBitmap(long inWidth, long inHeight, short inChannels, struct ImageInfo * outImageInfo);

/* Given a bitmap, this routine fills the whole bitmap in with a gray level of c, where
 * c = 0 means black and c = 255 means white. */
void	FillBitmap(const struct ImageInfo * inImageInfo, char c);

/* This routine deallocates a bitmap that was created with CreateBitmapFromFile or
 * CreateNewBitmap. */ 
void	DestroyBitmap(const struct ImageInfo * inImageInfo);

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

/* Create a 4-channel image from a DDS file. */
int		CreateBitmapFromDDS(const char * inFilePath, struct ImageInfo * outImageInfo);
			
/* This routine writes a 3 or 4 channel bitmap as a mip-mapped DXT1 or DXT3 image. */
int	WriteBitmapToDDS(struct ImageInfo& ioImage, int dxt, const char * file_name);

int	WriteUncompressedToDDS(struct ImageInfo& ioImage, const char * file_name);
			
#endif
