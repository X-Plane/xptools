/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextNode.cpp
*
******************************************************************/

#include <assert.h>
#include <string.h>
#include "TextNode.h"

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

TextNode::TextNode()
{
	setHeaderFlag(false);
	setType(textNodeString);

	///////////////////////////
	// ExposedField
	///////////////////////////

	// maxExtent exposed field
	maxExtentField = new SFFloat(1.0f);
	addExposedField(maxExtentFieldString, maxExtentField);

	// length exposed field
	lengthField = new MFFloat();
	addExposedField(lengthFieldString, lengthField);

	// string exposed field
	stringField = new MFString();
	addExposedField(stringFieldString, stringField);
}

TextNode::~TextNode()
{
}

////////////////////////////////////////////////
//	MaxExtent
////////////////////////////////////////////////

SFFloat *TextNode::getMaxExtentField()
{
	if (isInstanceNode() == false)
		return maxExtentField;
	return (SFFloat *)getExposedField(maxExtentFieldString);
}

void TextNode::setMaxExtent(float value)
{
	getMaxExtentField()->setValue(value);
}

float TextNode::getMaxExtent()
{
	return getMaxExtentField()->getValue();
}

////////////////////////////////////////////////
// String
////////////////////////////////////////////////

MFString *TextNode::getStringField()
{
	if (isInstanceNode() == false)
		return stringField;
	return (MFString *)getExposedField(stringFieldString);
}

void TextNode::addString(char *value)
{
	getStringField()->addValue(value);
}

int TextNode::getNStrings()
{
	return getStringField()->getSize();
}

char *TextNode::getString(int index)
{
	return getStringField()->get1Value(index);
}

void TextNode::setString(int index, char* value)
{
	getStringField()->set1Value(index, value);
}

////////////////////////////////////////////////
// length
////////////////////////////////////////////////

MFFloat *TextNode::getLengthField()
{
	if (isInstanceNode() == false)
		return lengthField;
	return (MFFloat *)getExposedField(lengthFieldString);
}

void TextNode::addLength(float value)
{
	getLengthField()->addValue(value);
}

int TextNode::getNLengths()
{
	return getLengthField()->getSize();
}

float TextNode::getLength(int index)
{
	return getLengthField()->get1Value(index);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

TextNode *TextNode::next()
{
	return (TextNode *)Node::next(getType());
}

TextNode *TextNode::nextTraversal()
{
	return (TextNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool TextNode::isChildNodeType(Node *node)
{
	if (node->isFontStyleNode())
		return true;
	else
		return false;
}

void TextNode::initialize()
{
	recomputeBoundingBox();
#ifdef SUPPORT_OPENGL
	recomputeDisplayList();
#endif
}

void TextNode::uninitialize()
{
}

void TextNode::update()
{
}

////////////////////////////////////////////////
//	FontStyle
////////////////////////////////////////////////

int TextNode::getFontStyleFamilyNumber()
{
	FontStyleNode *fontStyle = getFontStyleNodes();

	if (fontStyle == NULL)
		return FONTSTYLE_FAMILY_SERIF;
	return fontStyle->getFamilyNumber();
}

int TextNode::getFontStyleStyleNumber()
{
	FontStyleNode *fontStyle = getFontStyleNodes();

	if (fontStyle == NULL)
		return FONTSTYLE_STYLE_PLAIN;

	return fontStyle->getStyleNumber();
}

////////////////////////////////////////////////
//	Stringmation
////////////////////////////////////////////////

void TextNode::outputContext(ostream &printStream, char *indentString)
{
	printStream << indentString << "\t" << "maxExtent " << getMaxExtent() << endl;

	if (0 < getNStrings()) {
		MFString *string = getStringField();
		printStream << indentString << "\t" << "string [" << endl;
		string->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]"<< endl;
	}

	if (0 < getNLengths()) {
		MFFloat *length = getLengthField();
		printStream << indentString << "\t" << "length [" << endl;
		length->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]"<< endl;
	}

	FontStyleNode *fontStyle = getFontStyleNodes();
	if (fontStyle != NULL) {
		if (fontStyle->isInstanceNode() == false) {
			if (fontStyle->getName() != NULL && strlen(fontStyle->getName()))
				printStream << indentString << "\t" << "fontStyle " << "DEF " << fontStyle->getName() << " FontStyle {" << endl;
			else
				printStream << indentString << "\t" << "fontStyle FontStyle {"<< endl;
			fontStyle->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else
			printStream << indentString << "\t" << "fontStyle USE " << fontStyle->getName() << endl;
	}
}

////////////////////////////////////////////////
//	TextNode::recomputeBoundingBox
////////////////////////////////////////////////

void TextNode::recomputeBoundingBox()
{
	int nStrings = getNStrings();
	char *string = NULL;
	if (0 < nStrings) {
		string = getString(0);
		if (string != NULL) {
			if (strlen(string) <= 0)
				string = NULL;
		}
	}

	if (string != NULL) {
		float width = (float)strlen(string);
		setBoundingBoxCenter(-width/4.0f/1.0f, 0.5f, 0.0f);
		setBoundingBoxSize(width/4.0f, 0.5f, 0.5f);
	}
	else {
		setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
		setBoundingBoxSize(-1.0f, -1.0f, -1.0f);
	}
}

////////////////////////////////////////////////
//	SUPPORT_OPENGL
////////////////////////////////////////////////

#if defined(SUPPORT_OPENGL) && defined(WIN32)

OGLFontOutline *TextNode::getOGLFontOutlines()
{
	return mOGLFontOutlines.getNodes();
}

OGLFontOutline *TextNode::getOGLFontOutline(int family, int style)
{
	for (OGLFontOutline *node = getOGLFontOutlines(); node != NULL; node = node->next()) {
		if (family == node->getFamily() && style == node->getStyle())
			return node;
	}
	return NULL;
}

void TextNode::addOGLFontOutline(OGLFontOutline *node)
{
	mOGLFontOutlines.addNode(node);
}

void TextNode::addOGLFontOutline(int family, int style, unsigned int id)
{
	addOGLFontOutline(new OGLFontOutline(family, style, id));
}

int TextNode::getNOGLFontOutlines()
{
	return mOGLFontOutlines.getNNodes();
}

#endif

////////////////////////////////////////////////
//	TextNode::createUseFontOutline
////////////////////////////////////////////////

#if defined(SUPPORT_OPENGL) && defined(WIN32)

unsigned int TextNode::createUseFontOutline(int family, int style)
{
	char *fontName = NULL;
	switch (family) {
	case FONTSTYLE_FAMILY_SERIF:
		fontName ="Times New Roman";
		break;
	case FONTSTYLE_FAMILY_SANS:
		fontName ="Helvetica";
		break;
	case FONTSTYLE_FAMILY_TYPEWRITER:
		fontName ="Courier";
		break;
	}

	assert(fontName != NULL);

	unsigned int id = 0;

#if !defined(SUPPORT_GLUT)
	LOGFONT lf;

	lf.lfHeight = -MulDiv(12, 96, 72);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = (style == FONTSTYLE_STYLE_BOLD || style == FONTSTYLE_STYLE_BOLDITALIC)? 700 : 400;
	lf.lfItalic = (style == FONTSTYLE_STYLE_ITALIC || style == FONTSTYLE_STYLE_BOLDITALIC) ? TRUE : FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH;
	strcpy(lf.lfFaceName, fontName);

	HFONT font = CreateFontIndirect(&lf);
	HDC hdc = wglGetCurrentDC();

	HFONT oldFont = (HFONT)SelectObject(hdc, font);

	id = glGenLists(256);
	GLYPHMETRICSFLOAT gmf[256];
	wglUseFontOutlines(hdc, 0, 255, id, 1.0f, 0.1f, WGL_FONT_POLYGONS, gmf);

	SelectObject(hdc, oldFont);
#endif

	return id;
}

#endif

////////////////////////////////////////////////
//	TextNode::draw
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

void TextNode::draw()
{
	unsigned int nDisplayList = getDisplayList();
	if (nDisplayList == 0)
		return;

	int nStrings = getNStrings();
	char *string = NULL;
	if (0 < nStrings) {
		string = getString(0);
		if (string != NULL) {
			if (strlen(string) <= 0)
				string = NULL;
		}
	}

	if (string == NULL)
		return;

	glListBase(nDisplayList);
	glCallLists(strlen(string), GL_UNSIGNED_BYTE, (const GLvoid*)string);
}

#endif

////////////////////////////////////////////////
//	PointSet::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

void TextNode::recomputeDisplayList()
{
#ifdef WIN32
	int family	= getFontStyleFamilyNumber();
	int style	= getFontStyleStyleNumber();

	OGLFontOutline *fontOutline = getOGLFontOutline(family, style);

	unsigned int id = 0;

	if (fontOutline != NULL) {
		id = fontOutline->getListBaseID();
	}
	else {
		id = createUseFontOutline(family, style);
		addOGLFontOutline(family, style, id);
	}

	assert(id != 0);

	setDisplayList(id);
#endif
}

#endif

////////////////////////////////////////////////
//	OGLFontOutline
////////////////////////////////////////////////

#if defined(SUPPORT_OPENGL) && defined(WIN32)

LinkedList<OGLFontOutline> TextNode::mOGLFontOutlines;

OGLFontOutline::OGLFontOutline(int family, int style, unsigned int id)
{
	setFamily(family);
	setStyle(style);
	setListBaseID(id);
}

void OGLFontOutline::setFamily(int family)
{
	mFamily = family;
}

int OGLFontOutline::getFamily()
{
	return mFamily;
}

void OGLFontOutline::setStyle(int style)
{
	mStyle = style;
}

int OGLFontOutline::getStyle()
{
	return mStyle;
}

void OGLFontOutline::setListBaseID(unsigned int id)
{
	mListBaseID = id;
}

int OGLFontOutline::getListBaseID()
{
	return mListBaseID;
}

OGLFontOutline *OGLFontOutline::next()
{
	return LinkedListNode<OGLFontOutline>::next();
}

#endif
