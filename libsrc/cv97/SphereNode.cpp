/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SphereNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "SphereNode.h"

SphereNode::SphereNode()
{
	setHeaderFlag(false);
	setType(sphereNodeString);

	///////////////////////////
	// Exposed Field
	///////////////////////////

	// radius exposed field
	radiusField = new SFFloat(1.0f);
	addExposedField(radiusFieldString, radiusField);
}

SphereNode::~SphereNode()
{
}

////////////////////////////////////////////////
//	Radius
////////////////////////////////////////////////

SFFloat *SphereNode::getRadiusField()
{
	if (isInstanceNode() == false)
		return radiusField;
	return (SFFloat *)getExposedField(radiusFieldString);
}

void SphereNode::setRadius(float value)
{
	getRadiusField()->setValue(value);
}

float SphereNode::getRadius()
{
	return getRadiusField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

SphereNode *SphereNode::next()
{
	return (SphereNode *)Node::next(getType());
}

SphereNode *SphereNode::nextTraversal()
{
	return (SphereNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool SphereNode::isChildNodeType(Node *node)
{
	return false;
}

void SphereNode::initialize()
{
	recomputeBoundingBox();
#ifdef SUPPORT_OPENGL
		recomputeDisplayList();
#endif
}

void SphereNode::uninitialize()
{
}

void SphereNode::update()
{
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void SphereNode::recomputeBoundingBox()
{
	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(getRadius(), getRadius(), getRadius());
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void SphereNode::outputContext(ostream &printStream, char *indentString)
{
	printStream << indentString << "\t" << "radius " << getRadius() << endl;
}

////////////////////////////////////////////////
//	SphereNode::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

void SphereNode::recomputeDisplayList()
{
	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
		glFrontFace(GL_CCW);

	    glPushMatrix ();

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	    glRotatef (180.0, 0.0, 1.0, 0.0);

		glMatrixMode(GL_MODELVIEW);

	    glRotatef (90.0, 1.0, 0.0, 0.0);
	    glRotatef (180.0, 0.0, 0.0, 1.0);

	    GLUquadricObj *quadObj = gluNewQuadric ();
	    gluQuadricDrawStyle(quadObj, GLU_FILL);
	    gluQuadricNormals(quadObj, GLU_SMOOTH);
	    gluQuadricTexture(quadObj, GL_TRUE);
	    gluSphere(quadObj, getRadius(), 16, 16);
		gluDeleteQuadric(quadObj);

	    glPopMatrix ();

	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif
