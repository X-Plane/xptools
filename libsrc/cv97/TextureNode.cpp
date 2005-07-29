/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextureNode.cpp
*
******************************************************************/
#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "SceneGraph.h"
#include "TextureNode.h"

////////////////////////////////////////////////
//	TextureNode::TextureNode
////////////////////////////////////////////////

TextureNode::TextureNode() 
{
	///////////////////////////
	// Field 
	///////////////////////////

	// repeatS field
	repeatSField = new SFBool(true);
	addField(repeatSFieldString, repeatSField);

	// repeatT field
	repeatTField = new SFBool(true);
	addField(repeatTFieldString, repeatTField);

#ifdef SUPPORT_OPENGL
	///////////////////////////
	// Private Field 
	///////////////////////////

	// texture name field
	texNameField = new SFInt32(0);
	texNameField->setName(textureNamePrivateFieldString);
	addPrivateField(texNameField);

	// texture name field
	hasTransColorField = new SFBool(false);
	hasTransColorField->setName(hasTransparencyColorPrivateFieldString);
	addPrivateField(hasTransColorField);

#endif
}

////////////////////////////////////////////////
//	TextureNode::~TextureNode
////////////////////////////////////////////////

TextureNode::~TextureNode() 
{
}

////////////////////////////////////////////////
//	RepeatS
////////////////////////////////////////////////

SFBool *TextureNode::getRepeatSField()
{
	if (isInstanceNode() == false)
		return repeatSField;
	return (SFBool *)getField(repeatSFieldString);
}
	
void TextureNode::setRepeatS(bool value) 
{
	getRepeatSField()->setValue(value);
}

void TextureNode::setRepeatS(int value) 
{
	setRepeatS(value ? true : false);
}

bool TextureNode::getRepeatS() 
{
	return getRepeatSField()->getValue();
}

////////////////////////////////////////////////
//	RepeatT
////////////////////////////////////////////////

SFBool *TextureNode::getRepeatTField()
{
	if (isInstanceNode() == false)
		return repeatTField;
	return (SFBool *)getField(repeatTFieldString);
}
	
void TextureNode::setRepeatT(bool value) 
{
	getRepeatTField()->setValue(value);
}

void TextureNode::setRepeatT(int value) 
{
	setRepeatT(value ? true : false);
}

bool TextureNode::getRepeatT() 
{
	return getRepeatTField()->getValue();
}

////////////////////////////////////////////////
//	TextureName
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

SFInt32 *TextureNode::getTextureNameField()
{
	if (isInstanceNode() == false)
		return texNameField;
	return (SFInt32 *)getPrivateField(textureNamePrivateFieldString);
}

void TextureNode::setTextureName(unsigned int n) 
{
	getTextureNameField()->setValue((int)n);
}

unsigned int TextureNode::getTextureName() 
{
	return (unsigned int)getTextureNameField()->getValue();
} 

#endif

////////////////////////////////////////////////
//	Transparency
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

SFBool *TextureNode::getHasTransparencyColorField()
{
	if (isInstanceNode() == false)
		return hasTransColorField;
	return (SFBool *)getPrivateField(hasTransparencyColorPrivateFieldString);
}

void TextureNode::setHasTransparencyColor(bool value) 
{
	getHasTransparencyColorField()->setValue(value);
}

bool TextureNode::hasTransparencyColor() 
{
	return getHasTransparencyColorField()->getValue();
} 

#endif

////////////////////////////////////////////////
//	TextureNode::getTextureSize
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL
int GetOpenGLTextureSize(int size) 
{
	int n = 1;
	while ((1 << n) <= size)
		n++;

	return (1 << (n-1));
}
#endif
