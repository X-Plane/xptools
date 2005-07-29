/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileImage.h
*
******************************************************************/

#ifndef _CV97_FILEIMAGE_H_
#define _CV97_FILEIMAGE_H_

#ifdef SUPPORT_OLDCPP
#include "OldCpp.h"
#endif

#if !R && !G && !B
#define R	0
#define G	1
#define B	2
#endif

typedef unsigned char RGBColor24[3];
typedef unsigned char RGBAColor32[4];

enum {
FILETYPE_NONE,
FILETYPE_GIF,
FILETYPE_JPEG,
FILETYPE_TARGA,
FILETYPE_PNG
};

class FileImage {

public:

			FileImage();
	virtual ~FileImage();

	bool isOk();
	
	virtual int			getFileType() = 0;

	virtual int			getWidth() = 0;
	virtual int			getHeight() = 0;
	virtual RGBColor24	*getImage() = 0;

	virtual bool hasTransparencyColor() {
		return false;
	}

	virtual void getTransparencyColor(RGBColor24 color) {
	};

	RGBColor24	*getImage(int newx, int newy);
	RGBAColor32	*getRGBAImage();
	RGBAColor32	*getRGBAImage(int newx, int newy);
};

#endif
