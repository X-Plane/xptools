/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FontStyleNode.cpp
*
******************************************************************/

#include <string.h>
#include "FontStyleNode.h"

FontStyleNode::FontStyleNode() 
{
	setHeaderFlag(false);
	setType(fontStyleNodeString);

	///////////////////////////
	// Field 
	///////////////////////////

	// family field
	familyField = new SFString("SERIF");
	addField(familyFieldString, familyField);

	// style field
	styleField = new SFString("PLAIN");
	addField(styleFieldString, styleField);

	// language field
	languageField = new SFString("");
	addField(languageFieldString, languageField);

	// justify field
	justifyField = new MFString();
	addField(justifyFieldString, justifyField);

	// size field
	sizeField = new SFFloat(1.0f);
	addField(sizeFieldString, sizeField);

	// spacing field
	spacingField = new SFFloat(1.0f);
	addField(spacingFieldString, spacingField);

	// horizontal field
	horizontalField = new SFBool(true);
	addField(horizontalFieldString, horizontalField);

	// leftToRight field
	leftToRightField = new SFBool(true);
	addField(leftToRightFieldString, leftToRightField);

	// topToBottom field
	topToBottomField = new SFBool(true);
	addField(topToBottomFieldString, topToBottomField);
}

FontStyleNode::~FontStyleNode() 
{
}

////////////////////////////////////////////////
//	Size
////////////////////////////////////////////////

SFFloat *FontStyleNode::getSizeField()
{
	if (isInstanceNode() == false)
		return sizeField;
	return (SFFloat *)getField(sizeFieldString);
}

void FontStyleNode::setSize(float value) 
{
	getSizeField()->setValue(value);
}

float FontStyleNode::getSize() 
{
	return getSizeField()->getValue();
}

////////////////////////////////////////////////
//	Family
////////////////////////////////////////////////

SFString *FontStyleNode::getFamilyField()
{
	if (isInstanceNode() == false)
		return familyField;
	return (SFString *)getField(familyFieldString);
}
	
void FontStyleNode::setFamily(char *value) 
{
	getFamilyField()->setValue(value);
}

char *FontStyleNode::getFamily() 
{
	return getFamilyField()->getValue();
}

////////////////////////////////////////////////
//	Style
////////////////////////////////////////////////

SFString *FontStyleNode::getStyleField()
{
	if (isInstanceNode() == false)
		return styleField;
	return (SFString *)getField(styleFieldString);
}
	
void FontStyleNode::setStyle(char *value) 
{
	getStyleField()->setValue(value);
}

char *FontStyleNode::getStyle() 
{
	return getStyleField()->getValue();
}

////////////////////////////////////////////////
//	Language
////////////////////////////////////////////////

SFString *FontStyleNode::getLanguageField()
{
	if (isInstanceNode() == false)
		return languageField;
	return (SFString *)getField(languageFieldString);
}
	
void FontStyleNode::setLanguage(char *value) 
{
	getLanguageField()->setValue(value);
}

char *FontStyleNode::getLanguage() 
{
	return getLanguageField()->getValue();
}

////////////////////////////////////////////////
//	Horizontal
////////////////////////////////////////////////

SFBool *FontStyleNode::getHorizontalField()
{
	if (isInstanceNode() == false)
		return horizontalField;
	return (SFBool *)getField(horizontalFieldString);
}
	
void FontStyleNode::setHorizontal(bool value) 
{
	getHorizontalField()->setValue(value);
}

void FontStyleNode::setHorizontal(int value) 
{
	setHorizontal(value ? true : false);
}

bool FontStyleNode::getHorizontal() 
{
	return getHorizontalField()->getValue();
}

////////////////////////////////////////////////
//	LeftToRight
////////////////////////////////////////////////

SFBool *FontStyleNode::getLeftToRightField()
{
	if (isInstanceNode() == false)
		return leftToRightField;
	return (SFBool *)getField(leftToRightFieldString);
}
	
void FontStyleNode::setLeftToRight(bool value) 
{
	getLeftToRightField()->setValue(value);
}

void FontStyleNode::setLeftToRight(int value) 
{
	setLeftToRight(value ? true : false);
}

bool FontStyleNode::getLeftToRight() 
{
	return getLeftToRightField()->getValue();
}

////////////////////////////////////////////////
//	TopToBottom
////////////////////////////////////////////////

SFBool *FontStyleNode::getTopToBottomField()
{
	if (isInstanceNode() == false)
		return topToBottomField;
	return (SFBool *)getField(topToBottomFieldString);
}
	
void FontStyleNode::setTopToBottom(bool value) 
{
	getTopToBottomField()->setValue(value);
}

void FontStyleNode::setTopToBottom(int value) 
{
	setTopToBottom(value ? true : false);
}

bool FontStyleNode::getTopToBottom() 
{
	return getTopToBottomField()->getValue();
}

////////////////////////////////////////////////
// Justify
////////////////////////////////////////////////

MFString *FontStyleNode::getJustifyField()
{
	if (isInstanceNode() == false)
		return justifyField;
	return (MFString *)getField(justifyFieldString);
}

void FontStyleNode::addJustify(char *value) 
{
	getJustifyField()->addValue(value);
}

int FontStyleNode::getNJustifys() 
{
	return getJustifyField()->getSize();
}

char *FontStyleNode::getJustify(int index) 
{
	return getJustifyField()->get1Value(index);
}

////////////////////////////////////////////////
//	Spacing
////////////////////////////////////////////////

SFFloat *FontStyleNode::getSpacingField()
{
	if (isInstanceNode() == false)
		return spacingField;
	return (SFFloat *)getField(spacingFieldString);
}

void FontStyleNode::setSpacing(float value) 
{
	getSpacingField()->setValue(value);
}

float FontStyleNode::getSpacing() 
{
	return getSpacingField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

FontStyleNode *FontStyleNode::next() 
{
	return (FontStyleNode *)Node::next(getType());
}

FontStyleNode *FontStyleNode::nextTraversal() 
{
	return (FontStyleNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool FontStyleNode::isChildNodeType(Node *node)
{
	return false;
}

void FontStyleNode::initialize() 
{
	Node *parentNode = getParentNode();
	if (parentNode != NULL) {
		if (parentNode->isTextNode())
			parentNode->initialize();
	}
}

void FontStyleNode::uninitialize() 
{
}

void FontStyleNode::update() 
{
}

////////////////////////////////////////////////
//	Justifymation
////////////////////////////////////////////////

void FontStyleNode::outputContext(ostream &printStream, char *indentString) 
{
	SFString *family = getFamilyField();
	SFBool *horizontal = getHorizontalField();
	SFBool *leftToRight = getLeftToRightField();
	SFBool *topToBottom = getTopToBottomField();
	SFString *style = getStyleField();
	SFString *language = getLanguageField();

	printStream << indentString << "\t" << "size " << getSize() << endl;
	printStream << indentString << "\t" << "family " << family << endl;
	printStream << indentString << "\t" << "style " << style << endl;
	printStream << indentString << "\t" << "horizontal " << horizontal << endl;
	printStream << indentString << "\t" << "leftToRight " << leftToRight << endl;
	printStream << indentString << "\t" << "topToBottom " << topToBottom << endl;
	printStream << indentString << "\t" << "language " << language << endl;
	printStream << indentString << "\t" << "spacing " << getSpacing() << endl;

	if (0 < getNJustifys()) { 
		MFString *justify = (MFString *)getField(justifyFieldString);
		printStream << indentString << "\t" << "justify [" << endl;
		justify->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	Text::getFamilyNumber
////////////////////////////////////////////////

int FontStyleNode::getFamilyNumber()
{
	char *family = getFamily();

	if (family == NULL)
		return FONTSTYLE_FAMILY_SERIF;

	if (strcmp(family, "SERIF") == 0)
		return FONTSTYLE_FAMILY_SERIF;

	if (strcmp(family, "SANS") == 0)
		return FONTSTYLE_FAMILY_SANS;

	if (strcmp(family, "TYPEWRITER") == 0)
		return FONTSTYLE_FAMILY_TYPEWRITER;

	return FONTSTYLE_FAMILY_SERIF;
}

////////////////////////////////////////////////
//	Text::getStyleNumber
////////////////////////////////////////////////

int FontStyleNode::getStyleNumber()
{
	char *style = getStyle();

	if (style == NULL)
		return FONTSTYLE_STYLE_PLAIN;

	if (strcmp(style, "PLAIN") == 0)
		return FONTSTYLE_STYLE_PLAIN;

	if (strcmp(style, "BOLD") == 0)
		return FONTSTYLE_STYLE_BOLD;

	if (strcmp(style, "ITALIC") == 0)
		return FONTSTYLE_STYLE_ITALIC;

	if (strcmp(style, "BOLD ITALIC") == 0)
		return FONTSTYLE_STYLE_BOLDITALIC;

	return FONTSTYLE_STYLE_PLAIN;
}

