/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CylinderNode.cpp
*
******************************************************************/

#include "CylinderNode.h"

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

CylinderNode::CylinderNode()
{
	setHeaderFlag(false);
	setType(cylinderNodeString);

	// radius field
	radiusField = new SFFloat(1.0f);
	addExposedField(radiusFieldString, radiusField);

	// height field
	heightField = new SFFloat(2.0f);
	addExposedField(heightFieldString, heightField);

	// top field
	topField = new SFBool(true);
	addExposedField(topFieldString, topField);

	// side field
	sideField = new SFBool(true);
	addExposedField(sideFieldString, sideField);

	// bottom field
	bottomField = new SFBool(true);
	addExposedField(bottomFieldString, bottomField);
}

CylinderNode::~CylinderNode()
{
}

////////////////////////////////////////////////
//	radius
////////////////////////////////////////////////

SFFloat *CylinderNode::getRadiusField()
{
	if (isInstanceNode() == false)
		return radiusField;
	return (SFFloat *)getExposedField(radiusFieldString);
}

void CylinderNode::setRadius(float value)
{
	getRadiusField()->setValue(value);
}

float CylinderNode::getRadius()
{
	return getRadiusField()->getValue();
}

////////////////////////////////////////////////
//	height
////////////////////////////////////////////////

SFFloat *CylinderNode::getHeightField()
{
	if (isInstanceNode() == false)
		return heightField;
	return (SFFloat *)getExposedField(heightFieldString);
}

void CylinderNode::setHeight(float value)
{
	getHeightField()->setValue(value);
}

float CylinderNode::getHeight()
{
	return getHeightField()->getValue();
}

////////////////////////////////////////////////
//	top
////////////////////////////////////////////////

SFBool *CylinderNode::getTopField()
{
	if (isInstanceNode() == false)
		return topField;
	return (SFBool *)getExposedField(topFieldString);
}

void CylinderNode::setTop(bool value)
{
	getTopField()->setValue(value);
}

void CylinderNode::setTop(int value)
{
	setTop(value ? true : false);
}

bool CylinderNode::getTop()
{
	return getTopField()->getValue();
}

////////////////////////////////////////////////
//	side
////////////////////////////////////////////////

SFBool *CylinderNode::getSideField()
{
	if (isInstanceNode() == false)
		return sideField;
	return (SFBool *)getExposedField(sideFieldString);
}

void CylinderNode::setSide(bool value)
{
	getSideField()->setValue(value);
}

void CylinderNode::setSide(int value)
{
	setSide(value ? true : false);
}

bool CylinderNode::getSide()
{
	return getSideField()->getValue();
}

////////////////////////////////////////////////
//	bottom
////////////////////////////////////////////////

SFBool *CylinderNode::getBottomField()
{
	if (isInstanceNode() == false)
		return bottomField;
	return (SFBool *)getExposedField(bottomFieldString);
}

void CylinderNode::setBottom(bool  value)
{
	getBottomField()->setValue(value);
}

void CylinderNode::setBottom(int value)
{
	setBottom(value ? true : false);
}

bool  CylinderNode::getBottom()
{
	return getBottomField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

CylinderNode *CylinderNode::next()
{
	return (CylinderNode *)Node::next(getType());
}

CylinderNode *CylinderNode::nextTraversal()
{
	return (CylinderNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool CylinderNode::isChildNodeType(Node *node)
{
	return false;
}

void CylinderNode::initialize()
{
	recomputeBoundingBox();
#ifdef SUPPORT_OPENGL
	recomputeDisplayList();
#endif
}

void CylinderNode::uninitialize()
{
}

void CylinderNode::update()
{
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void CylinderNode::recomputeBoundingBox()
{
	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(getRadius(), getHeight()/2.0f, getRadius());
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void CylinderNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *top = getTopField();
	SFBool *side = getSideField();
	SFBool *bottom = getBottomField();

	printStream << indentString << "\t" << "radius " << getRadius() << endl;
	printStream << indentString << "\t" << "height " << getHeight() << endl;
	printStream << indentString << "\t" << "side " << side << endl;
	printStream << indentString << "\t" << "top " << top << endl;
	printStream << indentString << "\t" << "bottom " << bottom << endl;
}

////////////////////////////////////////////////
//	CylinderNode::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

void CylinderNode::recomputeDisplayList() {
	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
		GLUquadricObj *quadObj;

		glFrontFace(GL_CCW);

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
		    gluQuadricDrawStyle(quadObj, GLU_FILL);
		    gluQuadricNormals(quadObj, GLU_SMOOTH);
		    gluQuadricTexture(quadObj, GL_TRUE);
		    gluCylinder(quadObj, getRadius(), getRadius(), getHeight(), 12, 2);
			gluDeleteQuadric(quadObj);
		}

		if (getTop()) {
		    glPushMatrix ();
		    glRotatef (180.0, 1.0, 0.0, 0.0);
		    quadObj = gluNewQuadric ();
		    gluQuadricTexture(quadObj, GL_TRUE);
			gluDisk(quadObj, 0.0, getRadius(), 12, 2);
			gluDeleteQuadric(quadObj);
		    glPopMatrix ();
		}

		if (getBottom()) {
		    glTranslatef (0.0, 0.0, getHeight());
		    quadObj = gluNewQuadric ();
		    gluQuadricTexture(quadObj, GL_TRUE);
			gluDisk(quadObj, 0.0, getRadius(), 12, 2);
			gluDeleteQuadric(quadObj);
		}

	    glPopMatrix ();
	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif
