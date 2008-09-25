
/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ConeNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ConeNode.h"

ConeNode::ConeNode()
{
	setHeaderFlag(false);
	setType(coneNodeString);

	// bottomRadius field
	bottomRadiusField = new SFFloat(1.0f);
	addExposedField(bottomRadiusFieldString, bottomRadiusField);

	// height field
	heightField = new SFFloat(2.0f);
	addExposedField(heightFieldString, heightField);

	// side field
	sideField = new SFBool(true);
	addExposedField(sideFieldString, sideField);

	// bottom field
	bottomField = new SFBool(true);
	addExposedField(bottomFieldString, bottomField);
}

ConeNode::~ConeNode()
{
}

////////////////////////////////////////////////
//	bottomRadius
////////////////////////////////////////////////

SFFloat *ConeNode::getBottomRadiusField()
{
	if (isInstanceNode() == false)
		return bottomRadiusField;
	return (SFFloat *)getExposedField(bottomRadiusFieldString);
}

void ConeNode::setBottomRadius(float value)
{
	getBottomRadiusField()->setValue(value);
}

float ConeNode::getBottomRadius()
{
	return getBottomRadiusField()->getValue();
}

////////////////////////////////////////////////
//	height
////////////////////////////////////////////////

SFFloat *ConeNode::getHeightField()
{
	if (isInstanceNode() == false)
		return heightField;
	return (SFFloat *)getExposedField(heightFieldString);
}

void ConeNode::setHeight(float value)
{
	getHeightField()->setValue(value);
}

float ConeNode::getHeight()
{
	return getHeightField()->getValue();
}

////////////////////////////////////////////////
//	side
////////////////////////////////////////////////

SFBool *ConeNode::getSideField()
{
	if (isInstanceNode() == false)
		return sideField;
	return (SFBool *)getExposedField(sideFieldString);
}

void ConeNode::setSide(bool value)
{
	getSideField()->setValue(value);
}

void ConeNode::setSide(int value)
{
	setSide(value ? true : false);
}

bool ConeNode::getSide()
{
	return getSideField()->getValue();
}

////////////////////////////////////////////////
//	bottom
////////////////////////////////////////////////

SFBool *ConeNode::getBottomField()
{
	if (isInstanceNode() == false)
		return bottomField;
	return (SFBool *)getExposedField(bottomFieldString);
}

void ConeNode::setBottom(bool value)
{
	getBottomField()->setValue(value);
}

void ConeNode::setBottom(int value)
{
	setBottom(value ? true : false);
}

bool ConeNode::getBottom()
{
	return getBottomField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ConeNode *ConeNode::next()
{
	return (ConeNode *)Node::next(getType());
}

ConeNode *ConeNode::nextTraversal()
{
	return (ConeNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool ConeNode::isChildNodeType(Node *node)
{
	return false;
}

void ConeNode::initialize()
{
	recomputeBoundingBox();
#ifdef SUPPORT_OPENGL
	recomputeDisplayList();
#endif
}

void ConeNode::uninitialize()
{
}

void ConeNode::update()
{
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void ConeNode::recomputeBoundingBox()
{
	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(getBottomRadius(), getHeight()/2.0f, getBottomRadius());
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void ConeNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *side = getSideField();
	SFBool *bottom = getBottomField();

	printStream << indentString << "\t" << "bottomRadius " << getBottomRadius() << endl;
	printStream << indentString << "\t" << "height " << getHeight() << endl;
	printStream << indentString << "\t" << "side " << side << endl;
	printStream << indentString << "\t" << "bottom " << bottom << endl;
}

////////////////////////////////////////////////
//	ConeNode::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

void ConeNode::recomputeDisplayList() {
	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);

		glFrontFace(GL_CCW);

		GLUquadricObj *quadObj;

	    glPushMatrix ();

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	    glRotatef (180.0, 0.0, 1.0, 0.0);

		glMatrixMode(GL_MODELVIEW);

	    glRotatef (180.0, 0.0, 1.0, 0.0);
	    glRotatef (90.0, 1.0, 0.0, 0.0);
	    glTranslatef (0.0, 0.0, -getHeight()/2.0f);

		if (getSide()) {
		    quadObj = gluNewQuadric ();
		    gluQuadricDrawStyle (quadObj, GLU_FILL);
			gluQuadricNormals (quadObj, GLU_SMOOTH);
		    gluQuadricTexture(quadObj, GL_TRUE);
		    gluCylinder (quadObj, 0.0, getBottomRadius(), getHeight(), 15, 10);
			gluDeleteQuadric(quadObj);
		}

		if (getBottom()) {
		    glTranslatef (0.0, 0.0, getHeight());
		    quadObj = gluNewQuadric ();
		    gluQuadricTexture(quadObj, GL_TRUE);
			gluDisk(quadObj, 0.0, getBottomRadius(), 15, 10);
			gluDeleteQuadric(quadObj);
		    glTranslatef (0.0, 0.0, -1.0);
		}

	    glPopMatrix ();
	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif

