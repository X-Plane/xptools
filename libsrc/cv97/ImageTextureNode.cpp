/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ImageTextureNode.cpp
*
******************************************************************/
#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "SceneGraph.h"
#include "ImageTextureNode.h"
#include "FileGIF89a.h"
#include "FileJPEG.h"
#include "FileTarga.h"
#include "FilePNG.h"

////////////////////////////////////////////////
//	GetFileType
////////////////////////////////////////////////

static int GetFileType(char *filename) 
{
	FILE *fp = fopen(filename, "rb");
	if (!fp)	
		return FILETYPE_NONE;

	unsigned char signature[4];

	if (fread(signature, 4, 1, fp) != 1) {
		fclose(fp);
		return FILETYPE_NONE;
	}

	fclose(fp);

	int fileType = FILETYPE_NONE;

	if (!strncmp("GIF", (char *)signature, 3))
		fileType = FILETYPE_GIF;

	if (signature[0] == 0xff && signature[1] == 0xd8)
		fileType = FILETYPE_JPEG;

	if (!strncmp("PNG", (char *)(signature+1), 3))
		fileType = FILETYPE_PNG;

	return fileType;
}

////////////////////////////////////////////////
//	ImageTextureNode::ImageTextureNode
////////////////////////////////////////////////

ImageTextureNode::ImageTextureNode() 
{
	setHeaderFlag(false);
	setType(imageTextureNodeString);

	///////////////////////////
	// Exposed Field 
	///////////////////////////
 
	// url field
	urlField = new MFString();
	addExposedField(urlFieldString, urlField);

	///////////////////////////
	// Field 
	///////////////////////////

	mImageBuffer	= NULL;
	mFileImage		= NULL;
}

////////////////////////////////////////////////
//	ImageTextureNode::~ImageTextureNode
////////////////////////////////////////////////

ImageTextureNode::~ImageTextureNode() 
{
	if (mImageBuffer)
		delete []mImageBuffer;
	if (mFileImage)
		delete mFileImage;
}

////////////////////////////////////////////////
// Url
////////////////////////////////////////////////

MFString *ImageTextureNode::getUrlField()
{
	if (isInstanceNode() == false)
		return urlField;
	return (MFString *)getExposedField(urlFieldString);
}

void ImageTextureNode::addUrl(char * value) 
{
	getUrlField()->addValue(value);
}

int ImageTextureNode::getNUrls()
{
	return getUrlField()->getSize();
}

char *ImageTextureNode::getUrl(int index) 
{
	return getUrlField()->get1Value(index);
}

void ImageTextureNode::setUrl(int index, char *urlString) 
{
	getUrlField()->set1Value(index, urlString);
}

////////////////////////////////////////////////
//	ImageTextureNode::createImage
////////////////////////////////////////////////

bool ImageTextureNode::createImage()
{
	if (mFileImage) {
		delete mFileImage;
		mFileImage = NULL;
		mWidth	= 0;
		mHeight	= 0;
	}

	if (getNUrls() <= 0)
		return false;

	char *filename = getUrl(0);
	if (filename == NULL)
		return false;

#ifdef SUPPORT_URL
	SceneGraph	*sg			= getSceneGraph();
	bool		downloaded	= false;
#endif

	FILE *fp = fopen(filename, "rt");
	if (fp == NULL){
#ifdef SUPPORT_URL
		if (sg->getUrlStream(filename)) {
			downloaded = true;
			char *filename = sg->getUrlOutputFilename();
		}
		else
			return false;
#else
		return false;
#endif
	}
	else
		fclose(fp);

	mFileImage = NULL;

	switch (GetFileType(filename)) {
	case FILETYPE_GIF:
		mFileImage = new FileGIF89a(filename);
		break;
#ifdef SUPPORT_JPEG
	case FILETYPE_JPEG:
		mFileImage = new FileJPEG(filename);
		break;
#endif
#ifdef SUPPORT_PNG
	case FILETYPE_PNG:
		mFileImage = new FilePNG(filename);
		break;
#endif
	}

#ifdef SUPPORT_URL
	if (downloaded)
		sg->deleteUrlOutputFilename();
#endif

	if (!mFileImage)
		return false;

#ifdef SUPPORT_OPENGL
	mWidth	= GetOpenGLTextureSize(mFileImage->getWidth());
	mHeight	= GetOpenGLTextureSize(mFileImage->getHeight());
#else
	mWidth	= mFileImage->getWidth();
	mHeight	= mFileImage->getHeight();
#endif

	if (mImageBuffer != NULL)
		delete []mImageBuffer;

	mImageBuffer = mFileImage->getRGBAImage(mWidth, mHeight);
	
	if (mImageBuffer == NULL) {
		mWidth	= 0;
		mHeight	= 0;
	}

#ifdef SUPPORT_OPENGL
	setHasTransparencyColor(mFileImage->hasTransparencyColor());
#endif

	return true;
}

////////////////////////////////////////////////
//	ImageTextureNode::createImage
////////////////////////////////////////////////

void ImageTextureNode::initialize() 
{
	if (0 < getNUrls()) 
		updateTexture();
	else
		setCurrentTextureName(NULL);
}

////////////////////////////////////////////////
//	ImageTextureNode::uninitialize
////////////////////////////////////////////////

void ImageTextureNode::uninitialize() 
{
}

////////////////////////////////////////////////
//	ImageTextureNode::updateTexture
////////////////////////////////////////////////

void ImageTextureNode::updateTexture() 
{
#ifdef SUPPORT_OPENGL
	GLuint texName = (GLuint)getTextureName();
	if (0 < texName) 
		glDeleteTextures(1, &texName);

	if (createImage() == false) {
		setTextureName(0);
		setCurrentTextureName(NULL);
		return;
	}

	if (getWidth() == 0 || getHeight() == 0) {
		setTextureName(0);
		setCurrentTextureName(NULL);
		return;
	}

	glGenTextures(1, &texName);
	if (0 < texName) {
		glBindTexture(GL_TEXTURE_2D, texName);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, getWidth(), getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage());
		
		setTextureName(texName);
		
		if (0 < getNUrls()) 
			setCurrentTextureName(getUrl(0));
		else
			setCurrentTextureName(NULL);
	}
	else {
		setTextureName(0);
		setCurrentTextureName(NULL);
	}
#endif
}

////////////////////////////////////////////////
//	ImageTextureNode::update
////////////////////////////////////////////////

void ImageTextureNode::update() 
{
	if (0 < getNUrls()) {
		char *urlFilename = getUrl(0);
		char *currTexFilename = getCurrentTextureName();
		
		if (urlFilename != NULL && currTexFilename != NULL) {
			if (strcmp(urlFilename, currTexFilename) != 0)
				updateTexture();
		}
		if (urlFilename == NULL && currTexFilename != NULL) 
			updateTexture();
		if (urlFilename != NULL && currTexFilename == NULL) 
			updateTexture();
	}
}

////////////////////////////////////////////////
//	infomation
////////////////////////////////////////////////

void ImageTextureNode::outputContext(ostream &printStream, char *indentString) 
{
	SFBool *repeatS = getRepeatSField();
	SFBool *repeatT = getRepeatTField();

	printStream << indentString << "\t" << "repeatS " << repeatS  << endl;
	printStream << indentString << "\t" << "repeatT " << repeatT  << endl;

	if (0 < getNUrls()) {
		MFString *url = getUrlField();
		printStream << indentString << "\t" << "url [" << endl;
		url->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]"  << endl;
	}
}
