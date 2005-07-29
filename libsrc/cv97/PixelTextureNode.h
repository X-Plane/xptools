/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PixelTextureNode.h
*
******************************************************************/

#ifndef _CV97_PIXELTEXTURE_H_
#define _CV97_PIXELTEXTURE_H_

#include "VRMLField.h"
#include "TextureNode.h"

class PixelTextureNode : public TextureNode {

	SFImage *imageField;
	
public:

	PixelTextureNode();
	~PixelTextureNode();

	////////////////////////////////////////////////
	// Image
	////////////////////////////////////////////////

	SFImage *getImageField();

	void addImage(int value);
	int getNImages();
	int getImage(int index);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	PixelTextureNode *next();
	PixelTextureNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Imagemation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

