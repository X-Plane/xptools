/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SwitchNode.h
*
******************************************************************/

#include "SwitchNode.h"

void UpdateSwitch(SwitchNode *snode);
void InitializeSwitch(SwitchNode *snode);
void UninitializeSwitch(SwitchNode *snode);

SwitchNode::SwitchNode() 
{
	setHeaderFlag(false);
	setType(switchNodeString);

	// whichChoice field
	whichChoiceField = new SFInt32(-1);
	addField(whichChoiceFieldString, whichChoiceField);
}

SwitchNode::~SwitchNode() 
{
}

////////////////////////////////////////////////
//	whichChoice
////////////////////////////////////////////////

SFInt32 *SwitchNode::getWhichChoiceField()
{
	if (isInstanceNode() == false)
		return whichChoiceField;
	return (SFInt32 *)getField(whichChoiceFieldString);
}

void SwitchNode::setWhichChoice(int value) 
{
	getWhichChoiceField()->setValue(value);
}

int SwitchNode::getWhichChoice() 
{
	return getWhichChoiceField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

SwitchNode *SwitchNode::next() 
{
	return (SwitchNode *)Node::next(getType());
}

SwitchNode *SwitchNode::nextTraversal() 
{
	return (SwitchNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool SwitchNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void SwitchNode::outputContext(ostream &printStream, char *indentString) 
{
	printStream << indentString << "\t" << "whichChoice " << getWhichChoice() << endl;
}

////////////////////////////////////////////////
//	SwitchNode::update
////////////////////////////////////////////////

void UpdateSwitch(SwitchNode *snode)
{
	int nNodes = snode->getNPrivateNodeElements();
	for (int n=0; n<nNodes; n++) {
		Node *node = snode->getPrivateNodeElementAt(n);
		node->remove();
	}
	Node *node = snode->getPrivateNodeElementAt(snode->getWhichChoice());
	if (node)
		snode->addChildNode(node);
}

void SwitchNode::update() 
{
	UpdateSwitch(this);
}
	
////////////////////////////////////////////////
//	SwitchNode::initialize
////////////////////////////////////////////////

void InitializeSwitch(SwitchNode *snode)
{
	snode->uninitialize();

	Node *node = snode->getChildNodes();
	while (node) {
		Node *nextNode = node->next();
//		node->remove();
		snode->addPrivateNodeElement(node);
		node = nextNode;
	}
/*
	Node *selectedNode = snode->getPrivateNodeElementAt(snode->getWhichChoice());
	if (selectedNode)
		snode->addChildNode(selectedNode);
*/
}

void SwitchNode::initialize() 
{
	if (isInitialized() == false) {
		InitializeSwitch(this);
		setInitialized(true);
	}
}

////////////////////////////////////////////////
//	SwitchNode::uninitialize
////////////////////////////////////////////////

void UninitializeSwitch(SwitchNode *snode)
{
	int nNodes = snode->getNPrivateNodeElements();
	for (int n=0; n<nNodes; n++) {
		Node *node = snode->getPrivateNodeElementAt(n);
		node->remove();
		snode->addChildNode(node);
	}
	snode->removeAllNodeElement();
}

void SwitchNode::uninitialize() 
{
	if (isInitialized() == true) {
		UninitializeSwitch(this);
		setInitialized(false);
	}
}

