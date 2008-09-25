/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ScriptNode.cpp
*
******************************************************************/

#include "ScriptNode.h"
#include "Event.h"

////////////////////////////////////////////////
// ScriptNode::ScriptNode
////////////////////////////////////////////////

ScriptNode::ScriptNode()
{
	setHeaderFlag(false);
	setType(scriptNodeString);

	// directOutput exposed field
	directOutputField = new SFBool(false);
	Node::addField(directOutputFieldString, directOutputField);

	// directOutput exposed field
	mustEvaluateField = new SFBool(false);
	Node::addField(mustEvaluateFieldString, mustEvaluateField);

	// url exposed field
	urlField = new MFString();
	addExposedField(urlFieldString, urlField);

	// Clear Java object
#ifdef SUPPORT_JSAI
	mpJScriptNode = NULL;
#endif
}


////////////////////////////////////////////////
// ScriptNode::~ScriptNode
////////////////////////////////////////////////

ScriptNode::~ScriptNode()
{
#ifdef SUPPORT_JSAI
	if (mpJScriptNode)
		delete mpJScriptNode;
#endif
}

////////////////////////////////////////////////
// DirectOutput
////////////////////////////////////////////////

SFBool *ScriptNode::getDirectOutputField()
{
	if (isInstanceNode() == false)
		return directOutputField;
	return (SFBool *)getField(directOutputFieldString);
}

void ScriptNode::setDirectOutput(bool  value)
{
	getDirectOutputField()->setValue(value);
}

void ScriptNode::setDirectOutput(int value)
{
	setDirectOutput(value ? true : false);
}

bool ScriptNode::getDirectOutput()
{
	return getDirectOutputField()->getValue();
}

////////////////////////////////////////////////
// MustEvaluate
////////////////////////////////////////////////

SFBool *ScriptNode::getMustEvaluateField()
{
	if (isInstanceNode() == false)
		return mustEvaluateField;
	return (SFBool *)getField(mustEvaluateFieldString);
}

void ScriptNode::setMustEvaluate(bool  value)
{
	getMustEvaluateField()->setValue(value);
}

void ScriptNode::setMustEvaluate(int value)
{
	setMustEvaluate(value ? true : false);
}

bool ScriptNode::getMustEvaluate()
{
	return getMustEvaluateField()->getValue();
}

////////////////////////////////////////////////
// Url
////////////////////////////////////////////////

MFString *ScriptNode::getUrlField()
{
	if (isInstanceNode() == false)
		return urlField;
	return (MFString *)getExposedField(urlFieldString);
}

void ScriptNode::addUrl(char * value)
{
	getUrlField()->addValue(value);
}

int ScriptNode::getNUrls()
{
	return getUrlField()->getSize();
}

char *ScriptNode::getUrl(int index)
{
	return getUrlField()->get1Value(index);
}

void ScriptNode::setUrl(int index, char *urlString)
{
	getUrlField()->set1Value(index, urlString);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ScriptNode *ScriptNode::next()
{
	return (ScriptNode *)Node::next(getType());
}

ScriptNode *ScriptNode::nextTraversal()
{
	return (ScriptNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	virtual function
////////////////////////////////////////////////

bool ScriptNode::isChildNodeType(Node *node)
{
	return false;
}

void ScriptNode::update()
{
}

////////////////////////////////////////////////
//	output
////////////////////////////////////////////////

void ScriptNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *directOutput = getDirectOutputField();
	SFBool *mustEvaluate = getMustEvaluateField();

	printStream << indentString << "\t" << "directOutput " << directOutput << endl;
	printStream << indentString << "\t" << "mustEvaluate " << mustEvaluate << endl;

	if (0 < getNUrls()) {
		MFString *url = getUrlField();
		printStream << indentString << "\t" << "url [" << endl;
		url->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	int	n;

	for (n=0; n<getNEventIn(); n++) {
		Field *field = getEventIn(n);
		printStream << indentString << "\t" << "eventIn " << field->getTypeName() << " " << ((field->getName() && strlen(field->getName())) ? field->getName() : "NONE") << endl;
	}

	for (n=0; n<getNFields(); n++) {
		Field *field = getField(n);
		String fieldName(field->getName());
		if (fieldName.compareTo(directOutputFieldString) != 0 && fieldName.compareTo(mustEvaluateFieldString) != 0) {
			if (field->getType() == fieldTypeSFNode) {
				Node	*node = ((SFNode *)field)->getValue();
				char	*nodeName = NULL;
				if (node)
					nodeName = node->getName();
				if (nodeName && strlen(nodeName))
					printStream << indentString << "\t" << "field " << "SFNode" << " " << ((field->getName() && strlen(field->getName())) ? field->getName() : "NONE") << " USE " << nodeName << endl;
				else
					printStream << indentString << "\t" << "field " << "SFNode" << " " << ((field->getName() && strlen(field->getName())) ? field->getName() : "NONE") << " NULL" << endl;
			}
			else
				printStream << indentString << "\t" << "field " << field->getTypeName() << " " << ((field->getName() && strlen(field->getName())) ? field->getName() : "NONE") << " " << field << endl;
		}
	}

	for (n=0; n<getNEventOut(); n++) {
		Field *field = getEventOut(n);
		printStream << indentString << "\t" << "eventOut " << field->getTypeName() << " " << ((field->getName() && strlen(field->getName())) ? field->getName() : "NONE") << endl;
	}
}

////////////////////////////////////////////////
// ScriptNode::initialize
////////////////////////////////////////////////

void ScriptNode::initialize()
{
#ifdef SUPPORT_JSAI
	if (!isInitialized()) {

		if (mpJScriptNode) {
			delete mpJScriptNode;
			mpJScriptNode = NULL;
		}

		JScript *sjnode = new JScript(this);

		assert(sjnode);

		if (sjnode->isOK()) {
			mpJScriptNode = sjnode;
		}
		else
			delete sjnode;

		setInitialized(true);
	}

	if (mpJScriptNode) {
		mpJScriptNode->setValue(this);
		mpJScriptNode->initialize();
		mpJScriptNode->getValue(this);
	}

#endif
}

////////////////////////////////////////////////
// ScriptNode::initialize
////////////////////////////////////////////////

void ScriptNode::uninitialize()
{
	setInitialized(false);

#ifdef SUPPORT_JSAI

	if (hasScript()) {
		JScript *jscript = getJavaNode();
		jscript->setValue(this);
		jscript->shutdown();
		jscript->getValue(this);
	}

#endif
}

////////////////////////////////////////////////
// ScriptNode::update
////////////////////////////////////////////////

void ScriptNode::update(Field *eventInField) {

#ifdef SUPPORT_JSAI

	if (hasScript()) {

		JScript *jscript = getJavaNode();

		jscript->setValue(this);

		Event event(eventInField);
		jscript->processEvent(&event);

		jscript->getValue(this);

		int nEventOut = getNEventOut();
		for (int n=0; n<nEventOut; n++) {
			Field *field = getEventOut(n);
			sendEvent(field);
		}
	}

#endif

}

////////////////////////////////////////////////
// ScriptNode::updateFields
////////////////////////////////////////////////

void ScriptNode::updateFields() {

#ifdef SUPPORT_JSAI
	if (hasScript()) {
		JScript *jscript = getJavaNode();
		jscript->setValue(this);
		jscript->processEvent(NULL);
		jscript->getValue(this);
	}
#endif

}









