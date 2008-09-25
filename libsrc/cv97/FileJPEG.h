/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileJPEG.h
*
******************************************************************/

#ifndef _CV97_FILEJPEG_H_
#define _CV97_FILEJPEG_H_

#include "FileImage.h"

#ifdef SUPPORT_JPEG

class FileJPEG : public FileImage {
	int			width;
	int			height;
	RGBColor24	*imgBuffer;

public:

	FileJPEG(char *filename);
	~FileJPEG();

	bool load(char *filename);

	int getFileType() {
		return FILETYPE_JPEG;
	}

	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	RGBColor24 *getImage() {
		return imgBuffer;
	}
};

#endif

#endif
