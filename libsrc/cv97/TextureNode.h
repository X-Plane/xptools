/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextureNode.h
*
******************************************************************/

#ifndef _CV97_TEXTURENODE_H_
#define _CV97_TEXTURENODE_H_

#include "VRMLField.h"
#include "Node.h"
#include "FileImage.h"

#ifdef SUPPORT_OPENGL
#define	textureNamePrivateFieldString			"oglTextureName"
#define	hasTransparencyColorPrivateFieldString	"hasTransparencyColor"
#endif

class TextureNode : public Node {

	SFBool *repeatSField;
	SFBool *repeatTField;

	SFInt32 *texNameField;
	SFBool *hasTransColorField;

public:

	TextureNode();
	~TextureNode();

	////////////////////////////////////////////////
	//	RepeatS
	////////////////////////////////////////////////

	SFBool *getRepeatSField();

	void setRepeatS(bool value);
	void setRepeatS(int value);
	bool getRepeatS();

	////////////////////////////////////////////////
	//	RepeatT
	////////////////////////////////////////////////

	SFBool *getRepeatTField();

	void setRepeatT(bool value);
	void setRepeatT(int value);
	bool getRepeatT();

	////////////////////////////////////////////////
	//	Virtual Methods
	////////////////////////////////////////////////

	virtual int getWidth() {
		return 0;
	}

	virtual int getHeight() {
		return 0;
	}

	virtual RGBAColor32	*getImage() {
		return NULL;
	}

#ifdef SUPPORT_OPENGL

	////////////////////////////////////////////////
	//	TextureName
	////////////////////////////////////////////////

	SFInt32 *getTextureNameField();

	void setTextureName(unsigned int n);
	unsigned int getTextureName();

	////////////////////////////////////////////////
	//	Transparency
	////////////////////////////////////////////////

	SFBool *getHasTransparencyColorField();

	void setHasTransparencyColor(bool value);
	bool hasTransparencyColor();

#endif

};

#ifdef SUPPORT_OPENGL
int GetOpenGLTextureSize(int size);
#endif

#endif

