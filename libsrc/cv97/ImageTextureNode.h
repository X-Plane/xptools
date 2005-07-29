/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ImageTextureNode.h
*
******************************************************************/

#ifndef _CV97_IMAGETEXTURE_H_
#define _CV97_IMAGETEXTURE_H_

#include "VRMLField.h"
#include "TextureNode.h"
#include "FileImage.h"

class ImageTextureNode : public TextureNode {

	MFString *urlField;

	int			mWidth;
	int			mHeight;
	FileImage	*mFileImage;
	RGBAColor32	*mImageBuffer;
	String		mCurrentTextureName;

public:

	ImageTextureNode();
	~ImageTextureNode();

	////////////////////////////////////////////////
	// Url
	////////////////////////////////////////////////

	MFString *getUrlField();

	void addUrl(char * value);
	int getNUrls();
	char *getUrl(int index);
	void setUrl(int index, char *urlString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ImageTextureNode *next() 
	{
		return (ImageTextureNode *)Node::next(getType());
	}

	ImageTextureNode *nextTraversal() 
	{
		return (ImageTextureNode *)Node::nextTraversalByType(getType());
	}

	////////////////////////////////////////////////
	//	Image
	////////////////////////////////////////////////

	bool createImage();
	
	int getWidth()
	{
		return mWidth;
	}
	
	int getHeight()
	{
		return mHeight;
	}
	
	RGBAColor32 *getImage()
	{
		return mImageBuffer;
	}
	
	FileImage *getFileImage()
	{
		return mFileImage;
	}

	////////////////////////////////////////////////
	//	virtual functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node)
	{
		return false;
	}

	void initialize();

	void uninitialize();

	void setCurrentTextureName(char *name) 
	{
		mCurrentTextureName.setValue(name);
	}

	char *getCurrentTextureName() 
	{
		return mCurrentTextureName.getValue();
	}

	void updateTexture();

	void update();

	////////////////////////////////////////////////
	//	infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

