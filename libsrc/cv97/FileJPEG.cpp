/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileJPEG.cpp
*
******************************************************************/

#ifdef SUPPORT_JPEG

#include <ctype.h>
extern "C" {
#include "cdjpeg.h"
}
#include "FileJPEG.h"

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

METHODDEF(void) ErrorExit(j_common_ptr cinfo)
{
  (*cinfo->err->output_message) (cinfo);
}

FileJPEG::FileJPEG(char *filename)
{	
	imgBuffer = NULL;
	width = height = 0;
	
	load(filename);
}

bool FileJPEG::load(char *filename)
{	
	imgBuffer = NULL;
	width = height = 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	djpeg_dest_ptr dest_mgr = NULL;

	/* Initialize the JPEG decompression object with default error handling. */
	jpeg_std_error(&jerr);
	jerr.error_exit = ErrorExit;
	cinfo.err = &jerr;
	jpeg_create_decompress(&cinfo);

	/* Add some application-specific error messages (from cderror.h) */
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;

	FILE *fp = fopen(filename, READ_BINARY);
	if (!fp) 
		return false;

	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, fp);

	/* Read file header, set default decompression parameters */
	jpeg_read_header(&cinfo, TRUE);

//	if (cinfo.err->msg_code != 0)
//		return false;

	width = cinfo.image_width;
	height = cinfo.image_height;
	imgBuffer = new RGBColor24[width*height];

	/* Start decompressor */
	jpeg_start_decompress(&cinfo);

	/* Process data */
	unsigned char	**buffer = new unsigned char *[1]; 
	int				scanline = 0;

	while (cinfo.output_scanline < cinfo.output_height) {
		buffer[0] = (unsigned char *)imgBuffer[width*scanline];
		jpeg_read_scanlines(&cinfo, buffer, 1);
		scanline++;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	/* Close files, if we opened them */
	if (fp)
		fclose(fp);

	return true;
}

FileJPEG::~FileJPEG()
{
	if (imgBuffer)
		delete []imgBuffer;
}

#endif
