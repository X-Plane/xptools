/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	IndexedLineSetNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "IndexedLineSetNode.h"

IndexedLineSetNode::IndexedLineSetNode()
{
	setHeaderFlag(false);
	setType(indexedLineSetNodeString);

	///////////////////////////
	// Field
	///////////////////////////

	// colorPerVertex  field
	colorPerVertexField = new SFBool(true);
	colorPerVertexField->setName(colorPerVertexFieldString);
	addField(colorPerVertexField);

	// coordIndex  field
	coordIndexField = new MFInt32();
	coordIndexField->setName(coordIndexFieldString);
	addField(coordIndexField);

	// colorIndex  field
	colorIndexField = new MFInt32();
	colorIndexField->setName(colorIndexFieldString);
	addField(colorIndexField);

	///////////////////////////
	// EventIn
	///////////////////////////

	// coordIndex  EventIn
	MFInt32 *setCoordIndex = new MFInt32();
	setCoordIndex->setName(coordIndexFieldString);
	addEventIn(setCoordIndex);

	// colorIndex  EventIn
	MFInt32 *setColorIndex = new MFInt32();
	setColorIndex->setName(colorIndexFieldString);
	addEventIn(setColorIndex);
}

IndexedLineSetNode::~IndexedLineSetNode()
{
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

IndexedLineSetNode *IndexedLineSetNode::next()
{
	return (IndexedLineSetNode *)Node::next(getType());
}

IndexedLineSetNode *IndexedLineSetNode::nextTraversal()
{
	return (IndexedLineSetNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	ColorPerVertex
////////////////////////////////////////////////

SFBool *IndexedLineSetNode::getColorPerVertexField()
{
	if (isInstanceNode() == false)
		return colorPerVertexField;
	return (SFBool *)getField(colorPerVertexFieldString);
}

void IndexedLineSetNode::setColorPerVertex(bool value)
{
	getColorPerVertexField()->setValue(value);
}

void IndexedLineSetNode::setColorPerVertex(int value)
{
	setColorPerVertex(value ? true : false);
}

bool IndexedLineSetNode::getColorPerVertex()
{
	return getColorPerVertexField()->getValue();
}

////////////////////////////////////////////////
// CoordIndex
////////////////////////////////////////////////

MFInt32 *IndexedLineSetNode::getCoordIndexField()
{
	if (isInstanceNode() == false)
		return coordIndexField;
	return (MFInt32 *)getField(coordIndexFieldString);
}

void IndexedLineSetNode::addCoordIndex(int value)
{
	getCoordIndexField()->addValue(value);
}

int IndexedLineSetNode::getNCoordIndexes()
{
	return getCoordIndexField()->getSize();
}

int IndexedLineSetNode::getCoordIndex(int index)
{
	return getCoordIndexField()->get1Value(index);
}

void IndexedLineSetNode::clearCoordIndex()
{
	getCoordIndexField()->clear();
}

////////////////////////////////////////////////
// ColorIndex
////////////////////////////////////////////////

MFInt32 *IndexedLineSetNode::getColorIndexField()
{
	if (isInstanceNode() == false)
		return colorIndexField;
	return (MFInt32 *)getField(colorIndexFieldString);
}

void IndexedLineSetNode::addColorIndex(int value)
{
	getColorIndexField()->addValue(value);
}

int IndexedLineSetNode::getNColorIndexes()
{
	return getColorIndexField()->getSize();
}

int IndexedLineSetNode::getColorIndex(int index)
{
	return getColorIndexField()->get1Value(index);
}

void IndexedLineSetNode::clearColorIndex()
{
	getColorIndexField()->clear();
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool IndexedLineSetNode::isChildNodeType(Node *node)
{
	if (node->isColorNode() || node->isCoordinateNode())
		return true;
	else
		return false;
}

void IndexedLineSetNode::initialize()
{
	if (!isInitialized()) {
#ifdef SUPPORT_OPENGL
		recomputeDisplayList();
#endif
		recomputeBoundingBox();
		setInitialized(true);
	}
}

void IndexedLineSetNode::uninitialize()
{
}

void IndexedLineSetNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void IndexedLineSetNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *colorPerVertex = getColorPerVertexField();

	printStream << indentString << "\t" << "colorPerVertex " << colorPerVertex << endl;

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

	if (0 < getNCoordIndexes()) {
		MFInt32 *coordIndex = getCoordIndexField();
		printStream << indentString << "\t" << "coordIndex [" << endl;
		coordIndex->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNColorIndexes()) {
		MFInt32 *colorIndex = getColorIndexField();
		printStream << indentString << "\t" << "colorIndex [" << endl;
		colorIndex->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////////////////
//	IndexedLineSetNode::recomputeBoundingBox
////////////////////////////////////////////////////////////

void IndexedLineSetNode::recomputeBoundingBox()
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
//	IndexedLineSetNode::recomputeDisplayList
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

static void DrawIdxLineSet(IndexedLineSetNode *idxLineSet)
{
	CoordinateNode *coordinate = idxLineSet->getCoordinateNodes();
	if (!coordinate)
		return;

	NormalNode	*normal = idxLineSet->getNormalNodes();
	ColorNode	*color = idxLineSet->getColorNodes();
	int		bColorPerVertex =idxLineSet->getColorPerVertex();

	bool	bLineBegin = true;
	bool	bLineClose = true;
	int		nLine = 0;

	float	vpoint[3];
	float	pcolor[3];

	glColor3f(1.0f, 1.0f, 1.0f);

	int nCoordIndexes = idxLineSet->getNCoordIndexes();
	for (int nCoordIndex=0; nCoordIndex<nCoordIndexes; nCoordIndex++) {

		int coordIndex = idxLineSet->getCoordIndex(nCoordIndex);

		if (bLineBegin) {
			glBegin(GL_LINE_STRIP);
			bLineBegin = false;
			bLineClose = false;

			if (color && !bColorPerVertex) {
				color->getColor(nLine, pcolor);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, pcolor);
//				glColor3fv(pcolor);
			}

			nLine++;
		}

		if (coordIndex != -1) {
			coordinate->getPoint(coordIndex, vpoint);
			glVertex3fv(vpoint);

			if (color && bColorPerVertex) {
				color->getColor(coordIndex, pcolor);
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, pcolor);
//				glColor3fv(pcolor);
			}
		}
		else {
			glEnd();
			bLineBegin = true;
			bLineClose = true;
		}
	}

	if (bLineClose == false)
		glEnd();
}

void IndexedLineSetNode::recomputeDisplayList()
{
	CoordinateNode *coordinate = getCoordinateNodes();
	if (!coordinate)
		return;

	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
		DrawIdxLineSet(this);
	glEndList();

	setDisplayList(nNewDisplayList);
};

#endif
