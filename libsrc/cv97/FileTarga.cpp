/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FileTarga.h
*
******************************************************************/
#include	<stdio.h>
#include	<stdlib.h>
#include	"FileTarga.h"

void FileTarga::initialize()
{
	idLength	= 0;
	coMapType	= 0;
	imgType		= 2;
	index		= 0;
	length		= 0;
	coSize		= 0;
	xOrg		= 0;
	yOrg		= 0;
	width		= 0;
	height		= 0;
	pixelSize	= 0;
	attBits		= 0;
	imageBuffer = NULL;
}

FileTarga::FileTarga(char *filename)
{
	load(filename);
}

FileTarga::FileTarga(int cx, int cy, RGBColor24 *color)
{
	initialize();

	width		= cx;
	height		= cy;
	pixelSize	= 24;
	imageBuffer = color;
}

FileTarga::~FileTarga()
{
	if (imageBuffer)
		free(imageBuffer);
}

bool FileTarga::load(char *filename)
{
	initialize();

	FILE			*fp;

	if (!(fp = fopen(filename, "rb")))
		return false;

	fread(&idLength, sizeof(char), sizeof(char), fp);
	fread(&coMapType, sizeof(char), sizeof(char), fp);
	fread(&imgType, sizeof(char), sizeof(char), fp);
	fread(&index, sizeof(char), sizeof(short), fp);
	fread(&length, sizeof(char), sizeof(short), fp);
	fread(&coSize, sizeof(char), sizeof(char), fp);
	fread(&xOrg, sizeof(char), sizeof(short), fp);
	fread(&yOrg, sizeof(char), sizeof(short), fp);
	fread(&width, sizeof(char), sizeof(short), fp);
	fread(&height, sizeof(char), sizeof(short), fp);
	fread(&pixelSize, sizeof(char), sizeof(char), fp);
	fread(&attBits, sizeof(char), sizeof(char), fp);

	if (pixelSize != 24)
		return false;

	if (0 < idLength) {
		fseek(fp, idLength, SEEK_CUR);
		idLength = 0;
	}

	imageBuffer = (RGBColor24 *)malloc(sizeof(RGBColor24)*(height*width));
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++)
			fread(&imageBuffer[x+y*width], sizeof(char), sizeof(RGBColor24), fp);
	}

	fclose(fp);

	return true;
}

bool FileTarga::save(char *filename)
{
	FILE			*fp;

	if (!imageBuffer)
		return false;

	if (!(fp = fopen(filename, "wb")))
		return false;

	fwrite(&idLength, sizeof(char), sizeof(char), fp);
	fwrite(&coMapType, sizeof(char), sizeof(char), fp);
	fwrite(&imgType, sizeof(char), sizeof(char), fp);
	fwrite(&index, sizeof(char), sizeof(short), fp);
	fwrite(&length, sizeof(char), sizeof(short), fp);
	fwrite(&coSize, sizeof(char), sizeof(char), fp);
	fwrite(&xOrg, sizeof(char), sizeof(short), fp);
	fwrite(&yOrg, sizeof(char), sizeof(short), fp);
	fwrite(&width, sizeof(char), sizeof(short), fp);
	fwrite(&height, sizeof(char), sizeof(short), fp);
	fwrite(&pixelSize, sizeof(char), sizeof(char), fp);
	fwrite(&attBits, sizeof(char), sizeof(char), fp);

	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			fwrite(&imageBuffer[x+y*width][0], sizeof(char), sizeof(char), fp);
			fwrite(&imageBuffer[x+y*width][1], sizeof(char), sizeof(char), fp);
			fwrite(&imageBuffer[x+y*width][2], sizeof(char), sizeof(char), fp);
		}
	}

	fclose(fp);

	return true;
}
