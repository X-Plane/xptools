/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FilePNG.h
*
******************************************************************/

#ifndef _CV97_FILEPNG_H_
#define _CV97_FILEPNG_H_

#include "FileImage.h"

#ifdef SUPPORT_PNG

class FilePNG : public FileImage {
	int			mWidth;
	int			mHeight;
	RGBColor24	*mImgBuffer;
	bool		mHasTransparencyColor;
	RGBColor24	mTransparencyColor;
public:

	FilePNG(char *filename);
	~FilePNG();

	bool load(char *filename);

	int getFileType() {
		return FILETYPE_PNG;
	}

	int getWidth() {
		return mWidth;
	}

	int getHeight() {
		return mHeight;
	}

	RGBColor24 *getImage() {
		return mImgBuffer;
	}

	bool hasTransparencyColor() {
		return mHasTransparencyColor;
	}

	void setTransparencyColor(RGBColor24 color);

	void getTransparencyColor(RGBColor24 color);

};

#endif

#endif
