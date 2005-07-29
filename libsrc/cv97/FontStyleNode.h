/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FontStyleNode.h
*
******************************************************************/

#ifndef _CV97_FONTSTYLE_H_
#define _CV97_FONTSTYLE_H_

#include "VRMLField.h"
#include "Node.h"

enum {
FONTSTYLE_FAMILY_SERIF,
FONTSTYLE_FAMILY_SANS,
FONTSTYLE_FAMILY_TYPEWRITER,
};

enum {
FONTSTYLE_STYLE_PLAIN,
FONTSTYLE_STYLE_BOLD,
FONTSTYLE_STYLE_ITALIC,
FONTSTYLE_STYLE_BOLDITALIC,
};

enum {
FONTSTYLE_JUSTIFY_BEGIN,
FONTSTYLE_JUSTIFY_MIDDLE,
FONTSTYLE_JUSTIFY_END,
FONTSTYLE_JUSTIFY_FIRST,
};

class FontStyleNode : public Node {

	SFString *familyField;
	SFString *styleField;
	SFString *languageField;
	MFString *justifyField;
	SFFloat *sizeField;
	SFFloat *spacingField;
	SFBool *horizontalField;
	SFBool *leftToRightField;
	SFBool *topToBottomField;
	
public:

	FontStyleNode();
	~FontStyleNode();

	////////////////////////////////////////////////
	//	Size
	////////////////////////////////////////////////

	SFFloat *getSizeField();

	void setSize(float value);
	float getSize();

	////////////////////////////////////////////////
	//	Family
	////////////////////////////////////////////////
	
	SFString *getFamilyField();

	void setFamily(char *value);
	char *getFamily();
	int getFamilyNumber();

	////////////////////////////////////////////////
	//	Style
	////////////////////////////////////////////////
	
	SFString *getStyleField();

	void setStyle(char *value);
	char *getStyle();
	int getStyleNumber();

	////////////////////////////////////////////////
	//	Language
	////////////////////////////////////////////////
	
	SFString *getLanguageField();

	void setLanguage(char *value);
	char *getLanguage();

	////////////////////////////////////////////////
	//	Horizontal
	////////////////////////////////////////////////
	
	SFBool *getHorizontalField();

	void setHorizontal(bool value);
	void setHorizontal(int value);
	bool getHorizontal();

	////////////////////////////////////////////////
	//	LeftToRight
	////////////////////////////////////////////////
	
	SFBool *getLeftToRightField();

	void setLeftToRight(bool value);
	void setLeftToRight(int value);
	bool getLeftToRight();

	////////////////////////////////////////////////
	//	TopToBottom
	////////////////////////////////////////////////
	
	SFBool *getTopToBottomField();

	void setTopToBottom(bool value);
	void setTopToBottom(int value);
	bool getTopToBottom();

	////////////////////////////////////////////////
	// Justify
	////////////////////////////////////////////////

	MFString *getJustifyField();

	void addJustify(char *value);
	int getNJustifys();
	char *getJustify(int index);

	////////////////////////////////////////////////
	//	Spacing
	////////////////////////////////////////////////

	SFFloat *getSpacingField();

	void setSpacing(float value);
	float getSpacing();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	FontStyleNode *next();
	FontStyleNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Justifymation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

