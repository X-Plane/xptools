/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PointSetNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "PointSetNode.h"

PointSetNode::PointSetNode() 
{
	setHeaderFlag(false);
	setType(pointSetNodeString);
}

PointSetNode::~PointSetNode() {
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

PointSetNode *PointSetNode::next() 
{
	return (PointSetNode *)Node::next(getType());
}

PointSetNode *PointSetNode::nextTraversal() 
{
	return (PointSetNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool PointSetNode::isChildNodeType(Node *node)
{
	if (node->isCoordinateNode() || node->isColorNode())
		return true;
	else
		return false;
}

void PointSetNode::initialize() 
{
	if (!isInitialized()) {
#ifdef SUPPORT_OPENGL
		recomputeDisplayList();
#endif
		recomputeBoundingBox();
		setInitialized(1);
	}
}

void PointSetNode::uninitialize() 
{
}

void PointSetNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void PointSetNode::outputContext(ostream &printStream, char *indentString) 
{
	ColorNode *color = getColorNodes();
	if (color != NULL) {
		if (color->isInstanceNode() == false) {
			if (color->getName() != NULL && strlen(color->getName()))
				printStream << indentString << "\t" << "color " << "DEF " << color->getName() << " Color {" << endl;
			else
				printStream << indentString << "\t" << "color Color {" << endl;
			color->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "color USE " << color->getName() << endl;
	}

	CoordinateNode *coord = getCoordinateNodes();
	if (coord != NULL) {
		if (coord->isInstanceNode() == false) {
			if (coord->getName() != NULL && strlen(coord->getName()))
				printStream << indentString << "\t" << "coord " << "DEF " << coord->getName() << " Coordinate {" << endl;
			else
				printStream << indentString << "\t" << "coord Coordinate {" << endl;
			coord->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "coord USE " << coord->getName() << endl;
	}
}

////////////////////////////////////////////////////////////
//	PointSetNode::recomputeBoundingBox
////////////////////////////////////////////////////////////

void PointSetNode::recomputeBoundingBox() 
{
	CoordinateNode *coordinate = getCoordinateNodes();
	if (!coordinate) {
		setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
		setBoundingBoxSize(-1.0f, -1.0f, -1.0f);
		return;
	}

	BoundingBox bbox;
	float		point[3];

	int nCoordinatePoints = coordinate->getNPoints();
	for (int n=0; n<nCoordinatePoints; n++) {
		coordinate->getPoint(n, point);
		bbox.addPoint(point);
	}

	setBoundingBox(&bbox);
}

////////////////////////////////////////////////
//	PointSetNode::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

static void DrawPointSet(PointSetNode *pointSet)
{
	CoordinateNode *coordinate = pointSet->getCoordinateNodes();
	if (!coordinate)
		return;

	NormalNode	*normal = pointSet->getNormalNodes();
	ColorNode	*color = pointSet->getColorNodes();

	float	vpoint[3];
	float	pcolor[3];

	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_POINTS);

	int nCoordinatePoint = coordinate->getNPoints();
	for (int n=0; n<nCoordinatePoint; n++) {

		if (color) {
			color->getColor(n, pcolor);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, pcolor);
//			glColor3fv(pcolor);
		}

		coordinate->getPoint(n, vpoint);
		glVertex3fv(vpoint);
	}

	glEnd();
}

void PointSetNode::recomputeDisplayList() 
{
	CoordinateNode *coordinate = getCoordinateNodes();
	if (!coordinate)
		return;

	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
		DrawPointSet(this);
	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif
