/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileTarga.h
*
******************************************************************/

#ifndef _CV97_FILETARGA_H_
#define _CV97_FILETARGA_H_

#include "FileImage.h"

typedef struct {
	unsigned char		IDLength;
	unsigned char		CoMapType;
	unsigned char		ImgType;
	unsigned short int	Index;	
	unsigned short int	Length;	
	unsigned char		CoSize;	
	unsigned short int	XOrg;	
	unsigned short int	YOrg;	
	unsigned short int	Width;	
	unsigned short int	Height;	
	unsigned char		PixelSize;
	unsigned char		AttBits;
} TargaHeadeInfor;

class FileTarga : public FileImage {
	unsigned char		idLength;
	unsigned char		coMapType;
	unsigned char		imgType;
	unsigned short int	index;	
	unsigned short int	length;	
	unsigned char		coSize;	
	unsigned short int	xOrg;	
	unsigned short int	yOrg;	
	unsigned short int	width;	
	unsigned short int	height;	
	unsigned char		pixelSize;
	unsigned char		attBits;
	RGBColor24			*imageBuffer;
public:
	FileTarga(char *filename);
	FileTarga(int cx, int cy, RGBColor24 *color);
	~FileTarga();

	void		initialize();
	bool		load(char *filename);
	bool		save(char *filename);

	int			getFileType() { return FILETYPE_TARGA; }

	int			getWidth()	{ return width; }
	int			getHeight()	{ return height; }
	RGBColor24	*getImage()	{ return imageBuffer; }
};

#endif
