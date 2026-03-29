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
#ifndef TEXUTILS_H
#define TEXUTILS_H

#include <vector>

struct	ImageInfo;
struct	PreparedTextureImage {
	PreparedTextureImage() :
		data(nullptr),
		data_size(0),
		width(0),
		height(0),
		channels(0),
		level_count(0),
		org_x(0),
		org_y(0),
		act_x(0),
		act_y(0),
		vis_x(0),
		vis_y(0)
	{
	}

	unsigned char *	data;
	int				data_size;
	int				width;
	int				height;
	int				channels;
	int				level_count;
	int				org_x;
	int				org_y;
	int				act_x;
	int				act_y;
	int				vis_x;
	int				vis_y;
};

struct CompressedTextureImage {
	CompressedTextureImage() :
		data(nullptr),
		data_size(0),
		internal_format(0),
		width(0),
		height(0),
		level_count(0),
		org_x(0),
		org_y(0),
		act_x(0),
		act_y(0),
		vis_x(0),
		vis_y(0)
	{
	}

	unsigned char *		data;
	int					data_size;
	int					internal_format;
	int					width;
	int					height;
	int					level_count;
	int					org_x;
	int					org_y;
	int					act_x;
	int					act_y;
	int					vis_x;
	int					vis_y;
	std::vector<int>	level_sizes;
};

struct PreparedTextureUploadPerf {
	PreparedTextureUploadPerf() :
		convert_seconds(0.0),
		upload_seconds(0.0),
		configure_seconds(0.0),
		total_seconds(0.0),
		data_size(0),
		width(0),
		height(0),
		channels(0),
		level_count(0),
		compress_ok(0)
	{
	}

	double			convert_seconds;
	double			upload_seconds;
	double			configure_seconds;
	double			total_seconds;
	int				data_size;
	int				width;
	int				height;
	int				channels;
	int				level_count;
	int				compress_ok;
};

enum {

	tex_MagentaAlpha	=	1,	// Convert RGB image to RGBA, chroma-key alpha.
	tex_Wrap			=	2,	// Use wrapped tex, otherwise clamped.
	tex_Linear			=	4,	// Linear filter (otherwise nearest)
	tex_Mipmap			=	8,	// Generate mipmaps
	tex_Rescale			=	16,	// Rescale to use whole tex
//	tex_Nearest			=	32,	// Use nearest-neighbor - appears to be legacy that this is explicit?
	tex_Compress_Ok		=	64,	// Allow driver-driven texture compression
	tex_Always_Pad		=	128	// Force pad up to pow2 even if we have non-pots card.  Needed for UI

};

bool LoadTextureFromFile(
				const char * 	inFileName,
				int 			inTexNum,
				int				inFlags,
				int * 			outWidth,
				int * 			outHeight,
				float *			outS,
				float *			outT);

bool LoadTextureFromImage(
				ImageInfo& 		inInfo,
				int 			inTexNum,
				int				inFlags,
				int * 			outWidth,
				int * 			outHeight,
				float *			outS,
				float *			outT);

void EnsureTextureUploadCapsInitializedOnDrawThread();
bool TextureUploadCapsReady();
bool PrepareTextureImageForUpload(
				ImageInfo&					inInfo,
				int							inFlags,
				PreparedTextureImage *		outPrepared);
bool LoadTextureFromPreparedImage(
				const PreparedTextureImage&	inPrepared,
				int							inTexNum,
				int							inFlags,
				PreparedTextureUploadPerf *	outPerf = nullptr);
void DestroyPreparedTextureImage(PreparedTextureImage * image);
bool CaptureCompressedTextureFromBoundTexture(
				const PreparedTextureImage&	inPrepared,
				CompressedTextureImage *	outCompressed);
bool LoadTextureFromCompressedImage(
				const CompressedTextureImage&	inCompressed,
				int								inTexNum,
				int								inFlags,
				PreparedTextureUploadPerf *		outPerf = nullptr);
void DestroyCompressedTextureImage(CompressedTextureImage * image);

bool LoadTextureFromDDS(
				char *			mem_start,
				char *			mem_end,
				int				in_tex_num,
				int				inFlags,
				int *			outWidth,
				int *			outHeight);

bool LoadTextureFromKTX2(
				char*			mem_start,
				char*			mem_end,
				int				in_tex_num,
				int				inFlags,
				int*			outWidth,
				int*			outHeight);
#endif
