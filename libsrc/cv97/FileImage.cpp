/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileImage.cpp
*
******************************************************************/

#include <stdio.h>
#include <string.h>
#include <memory>
#include "FileImage.h"

FileImage::FileImage()
{
}

FileImage::~FileImage()
{
}

bool FileImage::isOk()
{
	if (0 < getWidth() && 0 < getHeight() && getImage())
		return true;
	else
		return false;
}

RGBColor24 *FileImage::getImage(int newx, int newy)
{
	float xscale = (float)getWidth() / (float)newx;
	float yscale = (float)getHeight() / (float)newy;

	RGBColor24 *color = getImage();
	if (color == NULL)
		return NULL;

	RGBColor24 *newColor = new RGBColor24[newx*newy];

	int width = getWidth();

	for (int y=0; y<newy; y++) {
		for (int x=0; x<newx; x++) {
			int xIndex = (int)((float)x*xscale);
			int yIndex = (int)((float)y*yscale);
			memcpy(newColor[x + y*newx], color[xIndex + yIndex*width], sizeof(RGBColor24));
		}
	}

	return newColor;
}

RGBAColor32 *FileImage::getRGBAImage()
{
	int width	= getWidth();
	int height	= getHeight();

	RGBColor24 *color = getImage(width, height);
	if (color == NULL)
		return NULL;

	RGBAColor32 *newColor = new RGBAColor32[width*height];

	RGBColor24 bgColor;
	if (hasTransparencyColor())
		getTransparencyColor(bgColor);

	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			int index = x + y*width;
			memcpy(newColor[index], color[index], sizeof(RGBColor24));
			if (hasTransparencyColor() && bgColor[0] == color[index][0] && bgColor[1] == color[index][1] && bgColor[2] == color[index][2])
				newColor[index][3] = 0x00;
			else
				newColor[index][3] = 0xff;
		}
	}

	return newColor;
}

RGBAColor32 *FileImage::getRGBAImage(int newx, int newy)
{
	RGBColor24	*color = getImage(newx, newy);
	if (color == NULL)
		return NULL;

	RGBAColor32	*newColor = new RGBAColor32[newx*newy];

	RGBColor24	bgColor;
	if (hasTransparencyColor())
		getTransparencyColor(bgColor);

	for (int y=0; y<newy; y++) {
		for (int x=0; x<newx; x++) {
			int index = x + y*newx;
			memcpy(newColor[index], color[index], sizeof(RGBColor24));
			if (hasTransparencyColor() && bgColor[0] == color[index][0] && bgColor[1] == color[index][1] && bgColor[2] == color[index][2])
				newColor[index][3] = 0x00;
			else
				newColor[index][3] = 0xff;
		}
	}

	delete []color;

	return newColor;
}
