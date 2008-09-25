/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ScriptNode.h
*
******************************************************************/

#ifndef _CV97_SCRIPT_H_
#define _CV97_SCRIPT_H_

#include "VRMLField.h"
#include "Node.h"
#include "JavaVM.h"
#include "JScript.h"

#ifdef SUPPORT_JSAI
class ScriptNode : public Node, public CJavaVM {
#else
class ScriptNode : public Node {
#endif

	SFBool *directOutputField;
	SFBool *mustEvaluateField;
	MFString *urlField;

#ifdef SUPPORT_JSAI
	JScript			*mpJScriptNode;
#endif

public:

	ScriptNode();
	~ScriptNode();

	////////////////////////////////////////////////
	// Initialization
	////////////////////////////////////////////////

	void initialize();
	void uninitialize();

	////////////////////////////////////////////////
	// DirectOutput
	////////////////////////////////////////////////

	SFBool *getDirectOutputField();

	void setDirectOutput(bool  value);
	void setDirectOutput(int value);
	bool  getDirectOutput();

	////////////////////////////////////////////////
	// MustEvaluate
	////////////////////////////////////////////////

	SFBool *getMustEvaluateField();

	void setMustEvaluate(bool  value);
	void setMustEvaluate(int value);
	bool  getMustEvaluate();

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

	ScriptNode *next();
	ScriptNode *nextTraversal();

	////////////////////////////////////////////////
	//	virtual function
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);

	////////////////////////////////////////////////
	//	update
	////////////////////////////////////////////////

	void update();

	////////////////////////////////////////////////
	//	output
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	// JSAI
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

	int hasScript() {
		return getJavaNode() ? 1 : 0;
	}

	JScript	*getJavaNode()	{return mpJScriptNode;}

#endif

	////////////////////////////////////////////////
	// Update Java Fields
	////////////////////////////////////////////////

	void	update(Field *eventInField);
	void	updateFields();
};

#endif

