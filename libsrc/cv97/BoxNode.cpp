/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BoxNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "BoxNode.h"

BoxNode::BoxNode() 
{
	setHeaderFlag(false);
	setType(boxNodeString);

	// size exposed field
	sizeField = new SFVec3f(2.0f, 2.0f, 2.0f);
	sizeField->setName(sizeFieldString);
	addExposedField(sizeField);
}

BoxNode::~BoxNode() 
{
}

////////////////////////////////////////////////
//	size
////////////////////////////////////////////////

SFVec3f *BoxNode::getSizeField() 
{
	if (isInstanceNode() == false)
		return sizeField;
	return (SFVec3f *)getExposedField(sizeFieldString);
}

void BoxNode::setSize(float value[]) 
{
	getSizeField()->setValue(value);
}

void BoxNode::setSize(float x, float y, float z) 
{
	getSizeField()->setValue(x, y, z);
}

void BoxNode::getSize(float value[]) 
{
	getSizeField()->getValue(value);
}

float BoxNode::getX() 
{
	return getSizeField()->getX();
}

float BoxNode::getY() 
{
	return getSizeField()->getY();
}

float BoxNode::getZ() 
{
	return getSizeField()->getZ();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

BoxNode *BoxNode::next() 
{
	return (BoxNode *)Node::next(getType());
}

BoxNode *BoxNode::nextTraversal() 
{
	return (BoxNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool BoxNode::isChildNodeType(Node *node)
{
	return false;
}

void BoxNode::initialize() 
{
	recomputeBoundingBox();
#ifdef SUPPORT_OPENGL
	recomputeDisplayList();
#endif
}

void BoxNode::uninitialize() 
{
}

void BoxNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void BoxNode::outputContext(ostream &printStream, char *indentString) 
{
	SFVec3f *size = getSizeField();
	printStream << indentString << "\t" << "size " << size << endl;
}

////////////////////////////////////////////////
//	BoxNode::recomputeBoundingBox
////////////////////////////////////////////////

void BoxNode::recomputeBoundingBox() 
{
	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(getX()/2.0f, getY()/2.0f, getZ()/2.0f);
}

////////////////////////////////////////////////
//	DrawBox
//
//	   4+--------+5
//	   /|       /|
//	  / |      / |
//	0+--------+1 |
//	 |  |     |  |
//	 | 7+-----|--+6
//	 | /      | /
//	 |/       |/
//	3+--------+2
//
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

static void DrawBox(float x0, float x1, float y0, float y1,	float z0, float z1)
{
    static float n[6][3] = {
			{0.0, 0.0, 1.0}, {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0},
			{0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}};

    static int faces[6][4] = {
			{ 3, 2, 1, 0 }, { 7, 6, 2, 3 }, { 4, 5, 6, 7 },
			{ 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 3, 0, 4, 7 }};

    static float t[4][2] = {
			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f } };

    float	v[8][3];

	v[0][0] = v[3][0] = v[4][0] = v[7][0] = x0;
	v[1][0] = v[2][0] = v[5][0] = v[6][0] = x1;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = y0;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = y1;
	v[4][2] = v[5][2] = v[6][2] = v[7][2] = z0;
	v[0][2] = v[1][2] = v[2][2] = v[3][2] = z1;

	glFrontFace(GL_CCW);

    for (int i = 0; i < 6; i++) {
		glBegin(GL_POLYGON);
		glNormal3fv(n[i]);
		glTexCoord2fv(t[0]);
		glVertex3fv(v[faces[i][0]]);
		glTexCoord2fv(t[1]);
		glVertex3fv(v[faces[i][1]]);
		glTexCoord2fv(t[2]);
		glVertex3fv(v[faces[i][2]]);
		glTexCoord2fv(t[3]);
		glVertex3fv(v[faces[i][3]]);
		glEnd();
    }
}

////////////////////////////////////////////////
//	BoxNode::recomputeDisplayList
////////////////////////////////////////////////

void BoxNode::recomputeDisplayList() {
	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
	    DrawBox(-getX()/2.0f, getX()/2.0f, -getY()/2.0f, getY()/2.0f, -getZ()/2.0f, getZ()/2.0f);
	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif
