/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextNode.h
*
******************************************************************/

#ifndef _CV97_TEXT_H_
#define _CV97_TEXT_H_

#include "GeometryNode.h"
#include "FontStyleNode.h"

#if defined(SUPPORT_OPENGL) && defined(WIN32)
class OGLFontOutline : public LinkedListNode<OGLFontOutline> {
private:
	int				mFamily;
	int				mStyle;
	unsigned int	mListBaseID;
public:
	OGLFontOutline(int family, int style, unsigned int id);
	void setFamily(int family);
	int getFamily();
	void setStyle(int style);
	int getStyle();
	void setListBaseID(unsigned int id);
	int getListBaseID();
	OGLFontOutline *next();
};
#endif

class TextNode : public GeometryNode {

	SFFloat *maxExtentField;
	MFFloat *lengthField;
	MFString *stringField;
	
public:

	TextNode();
	~TextNode();

	////////////////////////////////////////////////
	//	MaxExtent
	////////////////////////////////////////////////

	SFFloat *getMaxExtentField();
	
	void setMaxExtent(float value);
	float getMaxExtent();

	////////////////////////////////////////////////
	// String
	////////////////////////////////////////////////

	MFString *getStringField();

	void addString(char *value);
	int getNStrings();
	char *getString(int index);
	void setString(int index, char* value);

	////////////////////////////////////////////////
	// length
	////////////////////////////////////////////////

	MFFloat *getLengthField();

	void addLength(float value);
	int getNLengths();
	float getLength(int index);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	TextNode *next();
	TextNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	void recomputeBoundingBox();

	////////////////////////////////////////////////
	//	recomputeDisplayList
	////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL
	void recomputeDisplayList();
#endif

	////////////////////////////////////////////////
	//	FontStyle
	////////////////////////////////////////////////

	int getFontStyleFamilyNumber();
	int getFontStyleStyleNumber();

	////////////////////////////////////////////////
	//	SUPPORT_OPENGL
	////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL
	void draw();
#endif

#if defined(SUPPORT_OPENGL) && defined(WIN32)
	static LinkedList<OGLFontOutline>	mOGLFontOutlines;
	OGLFontOutline *getOGLFontOutlines();
	OGLFontOutline *getOGLFontOutline(int family, int style);
	void addOGLFontOutline(OGLFontOutline *node);
	unsigned int createUseFontOutline(int family, int style);
	void addOGLFontOutline(int family, int style, unsigned int id);
	int getNOGLFontOutlines();
#endif

	////////////////////////////////////////////////
	//	Stringmation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

