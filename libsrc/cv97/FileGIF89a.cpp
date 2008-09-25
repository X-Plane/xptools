/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GIF89a.cpp
*
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <assert.h>
#include <memory>
#include "FileGIF89a.h"

FileGIF89a::FileGIF89a(char *fname)
{
	globalColorTable = NULL;
	nImage = 0;
	image = NULL;
	setLzwBuffer(NULL);
	setLzwTableIndex(0);
	load(fname);
}

FileGIF89a::~FileGIF89a()
{
	if (globalColorTable)
		delete	[]globalColorTable;

	for (int n=0; n<nImage; n++) {
		if (image[n].buffer)
			delete []image[n].buffer;
		if (image[n].localColorTable)
			delete []image[n].localColorTable;
	}
}

static bool GetGIF89aHeaderInfo(FILE *fp, GIF89aHeaderInfo *headerInfo)
{
	if (fread(&headerInfo->signature,		sizeof(unsigned char), 3, fp) != 3)		return false;
	if (fread(&headerInfo->version,			sizeof(unsigned char), 3, fp) != 3)		return false;
	if (fread(&headerInfo->width,			sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&headerInfo->height,			sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&headerInfo->packedField,		sizeof(unsigned char), 1, fp) != 1)		return false;
	if (fread(&headerInfo->bgColorIndex,	sizeof(unsigned char), 1, fp) != 1)		return false;
	if (fread(&headerInfo->aspectRaito,		sizeof(unsigned char), 1, fp) != 1)		return false;

	return true;
}

static bool GetGIF89aImageInfo(FILE *fp, GIF89aImageInfo *info)
{
	if (fread(&info->imageLeftPosition,		sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&info->imageTopPosition,		sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&info->imageWidth,			sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&info->imageHeight,			sizeof(unsigned short), 1, fp) != 1)	return false;
	if (fread(&info->packedField,			sizeof(unsigned char), 1, fp) != 1)		return false;

	return true;
}

bool FileGIF89a::load(char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	if (!GetGIF89aHeaderInfo(fp, &headerInfo)) {
		fclose(fp);
		return false;
	}

	if (getGlobalColorTableFlag()) {
		unsigned int globalTableSize = 1 << (getSizeOfGlobalTable() + 1);
		globalColorTable = new RGBColor24[globalTableSize];
		if (fread(globalColorTable, globalTableSize*3, 1, fp) != 1) {
			fclose(fp);
			return false;
		}
	}

	int id = fgetc(fp);
	while (id != 0x3b && id != EOF) {

		bool			transparencyFlag = false;
		unsigned int	transparencyColorIndex = 0;

		if (id == 0x21) {
			int commentlabel = fgetc(fp);
			switch (commentlabel) {
			case 0xf9:
				{
					transparencyFlag = true;
					int nCommentData = fgetc(fp);
					unsigned char *commentData = new unsigned char[nCommentData];
					if (fread(commentData, nCommentData, 1, fp) != 1) {
						fclose(fp);
						return false;
					}
					transparencyFlag = commentData[0] & 0x01;
					transparencyColorIndex = commentData[3];
					delete []commentData;
				}
				break;
			case 0x01:
			case 0xff:
				{
					int nCommentData = fgetc(fp);
					for (int n=0; n<nCommentData; n++)
						fgetc(fp);
				}
			}

			do {
				id = fgetc(fp);
			} while (id != 0x00);

			id = fgetc(fp);
		}

		if (id != 0x2c) {
			id = fgetc(fp);
			continue;
		}

		image = (GIF89aImage *)realloc(image, sizeof(GIF89aImage)*(nImage+1));
		image[nImage].localColorTable = NULL;
		image[nImage].buffer = NULL;
		image[nImage].bufferSize = 0;
		image[nImage].transparencyFlag = transparencyFlag;
		image[nImage].transparencyColorIndex = transparencyColorIndex;

		if (!GetGIF89aImageInfo(fp, &image[nImage].info)) {
			fclose(fp);
			return false;
		}

		if (getImageLocalColorTableFlag(nImage)) {
			unsigned int localTableSize = 1 << (getImageSizeOfLocalTable(nImage) + 1);
			image[nImage].localColorTable = new RGBColor24[localTableSize];
			if (fread(image[nImage].localColorTable, localTableSize*3, 1, fp) != 1) {
				fclose(fp);
				return false;
			}
		}

		unsigned char codeSize;
		if (fread(&codeSize, sizeof(unsigned char), 1, fp) != 1) {
			fclose(fp);
			return false;
		}

		unsigned char	*dataByte = NULL;
		unsigned char	blockSize;
		unsigned int	dataSize;

		if (fread(&blockSize, sizeof(unsigned char), 1, fp) != 1) {
			fclose(fp);
			return false;
		}
		dataSize = 0;
		while (blockSize) {
			dataByte = (unsigned char *)realloc(dataByte, dataSize + blockSize);
			if (fread(&dataByte[dataSize], blockSize, 1, fp) != 1) {
				fclose(fp);
				return false;
			}
			dataSize += blockSize;
			if (fread(&blockSize, sizeof(unsigned char), 1, fp) != 1) {
				fclose(fp);
				return false;
			}
		}

		if (dataSize == 0) {
			fclose(fp);
			return false;
		}

		initializeLzwTable(codeSize, dataByte, dataSize);

		initializeImageBuffer(nImage);

		unsigned int codeBitSize = codeSize + 1;
		unsigned int nextCode, code;

		do {
			code = getNextCode(codeBitSize);
		} while (code == getClearCode());
		outputData(nImage, code);

		while ((nextCode = getNextCode(codeBitSize)) != getEndCode()) {
			if (nextCode == getClearCode()) {
				reinitializeLzwTable();
				codeBitSize = codeSize + 1;
				code = getNextCode(codeBitSize);
				if (code == getEndCode())
					break;
				outputData(nImage, code);
			}
			else if (nextCode < getLzwTableIndex()) {
				outputData(nImage, nextCode);
				if (getLzwTableIndex() < 4095)
					codeBitSize = addLzwTable(code, nextCode, codeBitSize);
				code = nextCode;
			}
			else {
				outputData(nImage, code);
				outputFirstData(nImage, code);
				if (getLzwTableIndex() < 4095)
					codeBitSize = addLzwTable(code, code, codeBitSize);
				code = nextCode;
			}
		}

		terminateLzwTable();

		free(dataByte);

		if (getImageInterlaceFlag(nImage))
			convertInterlacedImage(nImage);

		nImage++;
	}

	fclose(fp);

	return true;
}

void FileGIF89a::convertInterlacedImage(int nImage)
{
 	if (!image[nImage].buffer)
		return;

	int width	= getImageWidth(nImage);
	int height	= getImageHeight(nImage);

	RGBColor24 *newImage		= new RGBColor24[sizeof(RGBColor24)*width*height];

	int	n, nLine;

	nLine = 0;
	for (n=0; n<height; n+=8, nLine++)
		memcpy(newImage[n*width], image[nImage].buffer[nLine*width], sizeof(RGBColor24)*width);
	for (n=4; n<height; n+=8, nLine++)
		memcpy(newImage[n*width], image[nImage].buffer[nLine*width], sizeof(RGBColor24)*width);
	for (n=2; n<height; n+=4, nLine++)
		memcpy(newImage[n*width], image[nImage].buffer[nLine*width], sizeof(RGBColor24)*width);
	for (n=1; n<height; n+=2, nLine++)
		memcpy(newImage[n*width], image[nImage].buffer[nLine*width], sizeof(RGBColor24)*width);

	delete []image[nImage].buffer;

	image[nImage].buffer = newImage;
}

///////////////////////////////////////////////////////////////////
//	LZW Decode
///////////////////////////////////////////////////////////////////

void FileGIF89a::initializeLzwTable(unsigned int codeSize, unsigned char *dataByte, unsigned int dataSize)
{
	setLzwCodeSize(codeSize);

	reinitializeLzwTable();

	setLzwBuffer(dataByte);
	setLzwBufferSize(dataSize);

	lzwOffsetBit = 0;
}

void FileGIF89a::reinitializeLzwTable()
{
	unsigned int n;

	terminateLzwTable();

	unsigned int codeSize = getLzwCodeSize();

	for (n=0; n<GIF89A_LZW_TABLE_SIZE; n++) {
		lzwTable[n].n = 0;
		lzwTable[n].data = NULL;
	}

	unsigned int tableSize = 1 << codeSize;
	for (n=0; n<tableSize; n++)
		setLzwTable(n, n);

	setClearCode(tableSize);
	setEndCode(tableSize+1);
	setLzwTableIndex(tableSize+2);
}

void FileGIF89a::initializeImageBuffer(int n)
{
 	if (image[n].buffer)
		delete []image[n].buffer;
	image[n].buffer = new RGBColor24[sizeof(RGBColor24)*getImageWidth(nImage)*getImageHeight(nImage)];
	image[n].bufferSize = 0;
}

void FileGIF89a::getColor(int n, unsigned int index, RGBColor24 color)
{
	if (image[n].localColorTable) {
		color[R] = image[n].localColorTable[index][R];
		color[G] = image[n].localColorTable[index][G];
		color[B] = image[n].localColorTable[index][B];
	}
	else if (globalColorTable) {
		color[R] = globalColorTable[index][R];
		color[G] = globalColorTable[index][G];
		color[B] = globalColorTable[index][B];
	}
}

unsigned char SwapBit(unsigned char c)
{
	unsigned char ret = 0;
	for (int n=0; n<8; n++) {
		if (c & (1 << (7-n)))
			ret |= 1 << n;
	}
	return ret;
}

unsigned int FileGIF89a::getNextCode(unsigned int codeSize)
{
	unsigned int i, j, nextCode;

	nextCode = 0;
	for (i = lzwOffsetBit, j = 0; j < codeSize; ++i, ++j)
		nextCode |= ((lzwBuffer[ i / 8 ] & (1 << (i % 8))) != 0) << j;

	lzwOffsetBit += codeSize;

	return nextCode;
}

void FileGIF89a::terminateLzwTable()
{
	for (unsigned int n=0; n<getLzwTableIndex(); n++)
		delete 	lzwTable[n].data;

	setLzwTableIndex(0);
}

void FileGIF89a::setLzwTable(unsigned int index, unsigned int value)
{
	lzwTable[index].n = 1;
	lzwTable[index].data = new unsigned int[1];
	lzwTable[index].data[0] = value;
}

unsigned int FileGIF89a::addLzwTable(unsigned int sindex, unsigned int cindex, unsigned int codeBitSize)
{
	unsigned int index = getLzwTableIndex();

	unsigned int nData = lzwTable[sindex].n + (0 < lzwTable[cindex].n ? 1 : 0);

	lzwTable[index].n = nData;
	lzwTable[index].data = new unsigned int[nData];

	for (unsigned int n=0; n<lzwTable[sindex].n; n++)
		lzwTable[index].data[n] = lzwTable[sindex].data[n];

	if (0 < lzwTable[cindex].n)
		lzwTable[index].data[nData-1] = lzwTable[cindex].data[0];

	if ((index == (unsigned int)((1 << codeBitSize) - 1)) && codeBitSize != 12)
		codeBitSize++;

	setLzwTableIndex(index + 1);

	return codeBitSize;
}

void FileGIF89a::outputFirstData(int nImage, unsigned int tableIndex)
{
	assert(tableIndex < getLzwTableIndex());

	if (0<lzwTable[tableIndex].n) {
		getColor(nImage, lzwTable[tableIndex].data[0], image[nImage].buffer[image[nImage].bufferSize]);
		image[nImage].bufferSize++;
	}
}

void FileGIF89a::outputData(int nImage, unsigned int tableIndex)
{
	assert(tableIndex < getLzwTableIndex());

	for (unsigned int n=0; n<lzwTable[tableIndex].n; n++) {
		getColor(nImage, lzwTable[tableIndex].data[n], image[nImage].buffer[image[nImage].bufferSize]);
		image[nImage].bufferSize++;
	}
}

void FileGIF89a::outputData(int nImage, unsigned int sindex, unsigned int cindex)
{
	for (unsigned int n=0; n<lzwTable[sindex].n; n++) {
		getColor(nImage, lzwTable[sindex].data[n], image[nImage].buffer[image[nImage].bufferSize]);
		image[nImage].bufferSize++;
	}

	if (0<lzwTable[cindex].n) {
		getColor(nImage, lzwTable[cindex].data[0], image[nImage].buffer[image[nImage].bufferSize]);
		image[nImage].bufferSize++;
	}
}

///////////////////////////////////////////////////////////////////
//	Output
///////////////////////////////////////////////////////////////////

void FileGIF89a::printHeaderInfo()
{
	cout << "===== Header infomation ============================" << endl;
	cout << headerInfo.signature << endl;
	cout << headerInfo.version << endl;
	cout << "WIDTH : " << getGlobalWidth() << endl;
	cout << "HEIGHT : " << getGlobalHeight()  << endl;
	cout << "GLOBAL COLOR TABLE FLAG : " << getGlobalColorTableFlag()  << endl;
	cout << "COLOR RESOLUTION : " << getColorResolution()  << endl;
	cout << "SORT FLAG : " << getSortFlag()  << endl;
	cout << "SIZE OF GLOBAL TABLE : " << getSizeOfGlobalTable()  << endl;
	cout << "BG COLOR INDEX : " << getBgColorIndex()  << endl;
	cout << "ASPECT RAITO : " << getAspectRaito()  << endl;
}

void FileGIF89a::printImageInfo(int n)
{
	cout << "===== Image infomation ============================" << endl;
	cout << "IMAGE LEFT POSITION : " << getImageLeftPosition(n)  << endl;
	cout << "IMAGE TOP POSITION : " << getImageTopPosition(n)  << endl;
	cout << "IMAGE WIDTH : " << getImageWidth(n)  << endl;
	cout << "IMAGE HEIGHT : " << getImageHeight(n)  << endl;
	cout << "GLOBAL COLOR TABLE FLAG : " << getImageLocalColorTableFlag(n)  << endl;
	cout << "INTERLACE FLAG : " << getImageInterlaceFlag(n)  << endl;
	cout << "SORT FLAG : " << getImageSortFlag(n)  << endl;
	cout << "SIZE OF LOCAL TABLE : " << getImageSizeOfLocalTable(n)  << endl;
	cout << "===== Buffer infomation ============================" << endl;
	cout << "IMAZE SIZE : " << getImageBufferSize(n) << endl;
}
