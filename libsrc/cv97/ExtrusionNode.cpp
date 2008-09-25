/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ExtrusionNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "ExtrusionNode.h"
#include "MathUtil.h"

void AddDefaultParameters(ExtrusionNode *ex);

ExtrusionNode::ExtrusionNode()
{

	setHeaderFlag(false);
	setType(extrusionNodeString);

	///////////////////////////
	// Field
	///////////////////////////

	// beginCap field
	beginCapField = new SFBool(true);
	addField(beginCapFieldString, beginCapField);

	// endCap field
	endCapField = new SFBool(true);
	addField(endCapFieldString, endCapField);

	// ccw field
	ccwField = new SFBool(true);
	ccwField->setName(ccwFieldString);
	addField(ccwField);

	// convex field
	convexField = new SFBool(true);
	convexField->setName(convexFieldString);
	addField(convexField);

	// creaseAngle field
	creaseAngleField = new SFFloat(0.0f);
	creaseAngleField->setName(creaseAngleFieldString);
	addField(creaseAngleField);

	// solid field
	solidField = new SFBool(true);
	solidField->setName(solidFieldString);
	addField(solidField);

	// orientation field
	orientationField = new MFRotation();
	orientationField->setName(orientationFieldString);
	addField(orientationField);

	// scale field
	scaleField = new MFVec2f();
	scaleField->setName(scaleFieldString);
	addField(scaleField);

	// crossSection field
	crossSectionField = new MFVec2f();
	addField(crossSectionFieldString, crossSectionField);

	// spine field
	spineField = new MFVec3f();
	addField(spineFieldString, spineField);

	///////////////////////////
	// EventIn
	///////////////////////////

	// orientation EventIn
	MFRotation *setOrientationField = new MFRotation();
	setOrientationField->setName(orientationFieldString);
	addEventIn(setOrientationField);

	// scale EventIn
	MFVec2f *setScaleField = new MFVec2f();
	setScaleField->setName(scaleFieldString);
	addEventIn(setScaleField);

	// crossSection EventIn
	MFVec2f *setCrossSectionField = new MFVec2f();
	addEventIn(crossSectionFieldString, setCrossSectionField);

	// spine EventIn
	MFVec3f *setSpineField = new MFVec3f();
	addEventIn(spineFieldString, setSpineField);
}

ExtrusionNode::~ExtrusionNode()
{
}

////////////////////////////////////////////////
//	BeginCap
////////////////////////////////////////////////

SFBool *ExtrusionNode::getBeginCapField()
{
	if (isInstanceNode() == false)
		return beginCapField;
	return (SFBool *)getField(beginCapFieldString);
}

void ExtrusionNode::setBeginCap(bool value)
{
	getBeginCapField()->setValue(value);
}

void ExtrusionNode::setBeginCap(int value)
{
	setBeginCap(value ? true : false);
}

bool ExtrusionNode::getBeginCap()
{
	return getBeginCapField()->getValue();
}

////////////////////////////////////////////////
//	EndCap
////////////////////////////////////////////////

SFBool *ExtrusionNode::getEndCapField()
{
	if (isInstanceNode() == false)
		return endCapField;
	return (SFBool *)getField(endCapFieldString);
}

void ExtrusionNode::setEndCap(bool value)
{
	getEndCapField()->setValue(value);
}

void ExtrusionNode::setEndCap(int value)
{
	setEndCap(value ? true : false);
}

bool ExtrusionNode::getEndCap()
{
	return getEndCapField()->getValue();
}

////////////////////////////////////////////////
//	CCW
////////////////////////////////////////////////

SFBool *ExtrusionNode::getCCWField()
{
	if (isInstanceNode() == false)
		return ccwField;
	return (SFBool *)getField(ccwFieldString);
}

void ExtrusionNode::setCCW(bool value)
{
	getCCWField()->setValue(value);
}

void ExtrusionNode::setCCW(int value)
{
	setCCW(value ? true : false);
}

bool ExtrusionNode::getCCW()
{
	return getCCWField()->getValue();
}

////////////////////////////////////////////////
//	Convex
////////////////////////////////////////////////

SFBool *ExtrusionNode::getConvexField()
{
	if (isInstanceNode() == false)
		return convexField;
	return (SFBool *)getField(convexFieldString);
}

void ExtrusionNode::setConvex(bool value)
{
	getConvexField()->setValue(value);
}

void ExtrusionNode::setConvex(int value)
{
	setConvex(value ? true : false);
}

bool ExtrusionNode::getConvex()
{
	return getConvexField()->getValue();
}

////////////////////////////////////////////////
//	CreaseAngle
////////////////////////////////////////////////

SFFloat *ExtrusionNode::getCreaseAngleField()
{
	if (isInstanceNode() == false)
		return creaseAngleField;
	return (SFFloat *)getField(creaseAngleFieldString);
}

void ExtrusionNode::setCreaseAngle(float value)
{
	getCreaseAngleField()->setValue(value);
}

float ExtrusionNode::getCreaseAngle()
{
	return getCreaseAngleField()->getValue();
}

////////////////////////////////////////////////
//	Solid
////////////////////////////////////////////////

SFBool *ExtrusionNode::getSolidField()
{
	if (isInstanceNode() == false)
		return solidField;
	return (SFBool *)getField(solidFieldString);
}

void ExtrusionNode::setSolid(bool value)
{
	getSolidField()->setValue(value);
}

void ExtrusionNode::setSolid(int value)
{
	setSolid(value ? true : false);
}

bool ExtrusionNode::getSolid()
{
	return getSolidField()->getValue();
}

////////////////////////////////////////////////
// orientation
////////////////////////////////////////////////

MFRotation *ExtrusionNode::getOrientationField()
{
	if (isInstanceNode() == false)
		return orientationField;
	return (MFRotation *)getField(orientationFieldString);
}

void ExtrusionNode::addOrientation(float value[])
{
	getOrientationField()->addValue(value);
}

void ExtrusionNode::addOrientation(float x, float y, float z, float angle)
{
	getOrientationField()->addValue(x, y, z, angle);
}

int ExtrusionNode::getNOrientations()
{
	return getOrientationField()->getSize();
}

void ExtrusionNode::getOrientation(int index, float value[])
{
	getOrientationField()->get1Value(index, value);
}

////////////////////////////////////////////////
// scale
////////////////////////////////////////////////

MFVec2f *ExtrusionNode::getScaleField()
{
	if (isInstanceNode() == false)
		return scaleField;
	return (MFVec2f *)getField(scaleFieldString);
}

void ExtrusionNode::addScale(float value[])
{
	getScaleField()->addValue(value);
}

void ExtrusionNode::addScale(float x, float z)
{
	getScaleField()->addValue(x, z);
}

int ExtrusionNode::getNScales()
{
	return getScaleField()->getSize();
}

void ExtrusionNode::getScale(int index, float value[])
{
	getScaleField()->get1Value(index, value);
}

////////////////////////////////////////////////
// crossSection
////////////////////////////////////////////////

MFVec2f *ExtrusionNode::getCrossSectionField()
{
	if (isInstanceNode() == false)
		return crossSectionField;
	return (MFVec2f *)getField(crossSectionFieldString);
}

void ExtrusionNode::addCrossSection(float value[])
{
	getCrossSectionField()->addValue(value);
}

void ExtrusionNode::addCrossSection(float x, float z)
{
	getCrossSectionField()->addValue(x, z);
}

int ExtrusionNode::getNCrossSections()
{
	return getCrossSectionField()->getSize();
}

void ExtrusionNode::getCrossSection(int index, float value[])
{
	getCrossSectionField()->get1Value(index, value);
}

////////////////////////////////////////////////
// spine
////////////////////////////////////////////////

MFVec3f *ExtrusionNode::getSpineField()
{
	if (isInstanceNode() == false)
		return spineField;
	return (MFVec3f *)getField(spineFieldString);
}

void ExtrusionNode::addSpine(float value[])
{
	getSpineField()->addValue(value);
}

void ExtrusionNode::addSpine(float x, float y, float z)
{
	getSpineField()->addValue(x, y, z);
}

int ExtrusionNode::getNSpines()
{
	return getSpineField()->getSize();
}

void ExtrusionNode::getSpine(int index, float value[])
{
	getSpineField()->get1Value(index, value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ExtrusionNode *ExtrusionNode::next()
{
	return (ExtrusionNode *)Node::next(getType());
}

ExtrusionNode *ExtrusionNode::nextTraversal()
{
	return (ExtrusionNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool ExtrusionNode::isChildNodeType(Node *node)
{
	return false;
}

void ExtrusionNode::initialize()
{
	if (!isInitialized()) {
		AddDefaultParameters(this);
#ifdef SUPPORT_OPENGL
		recomputeDisplayList();
#endif
		recomputeBoundingBox();
		setInitialized(true);
	}
}

void ExtrusionNode::uninitialize()
{
}

void ExtrusionNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void ExtrusionNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *beginCap = getBeginCapField();
	SFBool *endCap = getEndCapField();
	SFBool *ccw = getCCWField();
	SFBool *convex = getConvexField();
	SFBool *solid = getSolidField();

	printStream << indentString << "\t" << "beginCap " << beginCap << endl;
	printStream << indentString << "\t" << "endCap " << endCap << endl;
	printStream << indentString << "\t" << "solid " << solid << endl;
	printStream << indentString << "\t" << "ccw " << ccw << endl;
	printStream << indentString << "\t" << "convex " << convex << endl;
	printStream << indentString << "\t" << "creaseAngle " << getCreaseAngle() << endl;

	if (0 < getNCrossSections()) {
		MFVec2f *crossSection = getCrossSectionField();
		printStream << indentString << "\t" << "crossSection [" << endl;
		crossSection->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNOrientations()) {
		MFRotation *orientation = getOrientationField();
		printStream << indentString << "\t" << "orientation [" << endl;
		orientation->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNScales()) {
		MFVec2f *scale = getScaleField();
		printStream << indentString << "\t" << "scale [" << endl;
		scale->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNSpines()) {
		MFVec3f *spine = getSpineField();
		printStream << indentString << "\t" << "spine [" << endl;
		spine->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	GroupingNode::recomputeBoundingBox
////////////////////////////////////////////////

void AddDefaultParameters(ExtrusionNode *ex)
{
	if (ex->getNCrossSections() == 0) {
		ex->addCrossSection(1.0f, 1.0);
		ex->addCrossSection(1.0f, -1.0);
		ex->addCrossSection(-1.0f, -1.0);
		ex->addCrossSection(-1.0f, 1.0);
		ex->addCrossSection(1.0f, 1.0);
	}
	if (ex->getNSpines() == 0) {
		ex->addSpine(0.0f, 0.0f, 0.0f);
		ex->addSpine(0.0f, 1.0f, 0.0f);
	}
}

////////////////////////////////////////////////
//	GroupingNode::recomputeBoundingBox
////////////////////////////////////////////////

void ExtrusionNode::recomputeBoundingBox()
{
}

////////////////////////////////////////////////
//	GroupingNode::recomputeBoundingBox
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

static void initializePoint(ExtrusionNode *ex, SFVec3f *point)
{
	int nCrossSections = ex->getNCrossSections();
	for (int n=0; n<nCrossSections; n++) {
		float cs[2];
		ex->getCrossSection(n, cs);
		point[n].setValue(cs[0], 0.0f, cs[1]);
	}
}

static void transformPoint(SFVec3f *point, float scale[2], float scp[3][3], float orientation[4], float spine[3])
{
	point->scale(scale[0], 1.0f, scale[1]);

	float value[3];
	point->getValue(value);

	if (0.0f < VectorGetLength(scp[0]) && 0.0f < VectorGetLength(scp[1]) && 0.0f < VectorGetLength(scp[2])) {
		float x = value[0]*scp[0][0]+value[1]*scp[1][0]+value[2]*scp[2][0];
		float y = value[0]*scp[0][1]+value[1]*scp[1][1]+value[2]*scp[2][1];
		float z = value[0]*scp[0][2]+value[1]*scp[1][2]+value[2]*scp[2][2];
		value[0] = x;
		value[1] = y;
		value[2] = z;
	}

	point->setValue(value);

	point->translate(spine);
	point->rotate(orientation);
}

static void DrawExtrusion(ExtrusionNode *ex)
{
	bool ccw = ex->getCCW();
	if (ccw == true)
		glFrontFace(GL_CCW);
	else
		glFrontFace(GL_CW);

	bool solid = ex->getSolid();
//	if (solid == false)
		glDisable(GL_CULL_FACE);
//	else
//		glEnable(GL_CULL_FACE);

	int nCrossSections = ex->getNCrossSections();

	SFVec3f *point[2];
	point[0] = new SFVec3f[nCrossSections];
	point[1] = new SFVec3f[nCrossSections];

	int		nOrientations	= ex->getNOrientations();
	int		nScales			= ex->getNScales();
	int		nSpines			= ex->getNSpines();

	float	spineStart[3];
	float	spineEnd[3];
	bool	bClosed;

	ex->getSpine(0,			spineStart);
	ex->getSpine(nSpines-1, spineEnd);
	bClosed = VectorEquals(spineStart, spineEnd);

	float	scale[2];
	float	orientation[4];
	float	spine[3];
	float	scp[3][3];

	for (int n=0; n<(nSpines-1); n++) {
		initializePoint(ex, point[0]);
		initializePoint(ex, point[1]);

		for (int i=0; i<2; i++) {

			if (nScales == 1)
				ex->getScale(0, scale);
			else  if ((n+i) < nScales)
				ex->getScale(n+i, scale);
			else {
				scale[0] = 1.0f;
				scale[1] = 1.0f;
			}

			if (nOrientations == 1)
				ex->getOrientation(0, orientation);
			else if ((n+i) < nOrientations)
				ex->getOrientation(n+i, orientation);
			else {
				orientation[0] = 0.0f;
				orientation[1] = 0.0f;
				orientation[2] = 1.0f;
				orientation[3] = 0.0f;
			}

			ex->getSpine(n+i, spine);

			// SCP Y
			float spine0[3], spine1[3], spine2[3];
			if (nSpines <= 2) {
				ex->getSpine(1, spine1);
				ex->getSpine(0, spine2);
			}
			else if (bClosed && (n+i == 0 || n+i == (nSpines-1))) {
				ex->getSpine(1,			spine1);
				ex->getSpine(nSpines-2,	spine2);
			}
			else if (n+i == 0) {
				ex->getSpine(1, spine1);
				ex->getSpine(0, spine2);
			}
			else if (n+i == (nSpines-1)) {
				ex->getSpine(nSpines-1, spine1);
				ex->getSpine(nSpines-2, spine2);
			}
			else {
				ex->getSpine(n+i+1, spine1);
				ex->getSpine(n+i-1, spine2);
			}
			VectorGetDirection(spine1, spine2, scp[1]);
			VectorNormalize(scp[1]);

			// SCP Z
			float v1[3], v2[3];
			if (nSpines <= 2) {
				ex->getSpine(0, spine0);
				ex->getSpine(1, spine1);
				ex->getSpine(1, spine2);
			}
			else if (bClosed && (n+i == 0 || n+i == (nSpines-1))) {
				ex->getSpine(0,			spine0);
				ex->getSpine(1,			spine1);
				ex->getSpine(nSpines-2,	spine2);
			}
			else if (n+i == 0) {
				ex->getSpine(1,	spine0);
				ex->getSpine(2,	spine1);
				ex->getSpine(0,	spine2);
			}
			else if (n+i == (nSpines-1)) {
				ex->getSpine(nSpines-2, spine1);
				ex->getSpine(nSpines-1, spine1);
				ex->getSpine(nSpines-3, spine2);
			}
			else {
				ex->getSpine(n+i,	spine0);
				ex->getSpine(n+i+1,	spine1);
				ex->getSpine(n+i-1,	spine2);
			}
			VectorGetDirection(spine1, spine0, v1);
			VectorGetDirection(spine2, spine0, v2);
			VectorGetCross(v1, v2, scp[2]);

			// SCP X
			VectorGetCross(scp[1], scp[2], scp[0]);

			for (int j=0; j<nCrossSections; j++)
				transformPoint(&point[i][j], scale, scp, orientation, spine);
		}

		for (int k=0; k<nCrossSections-1; k++) {

			float	vpoint[3][3];
			float	normal[3];

			point[1][k].getValue(vpoint[0]);
			point[0][k].getValue(vpoint[1]);
			point[1][(k+1)%nCrossSections].getValue(vpoint[2]);
			GetNormalFromVertices(vpoint, normal);
			glNormal3fv(normal);

			SFVec3f	*vertex;

			glBegin(GL_POLYGON);
			vertex = &point[1][(k+1)%nCrossSections];	glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			vertex = &point[1][k];						glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			vertex = &point[0][k];						glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			glEnd();

			glBegin(GL_POLYGON);
			vertex = &point[0][(k+1)%nCrossSections];	glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			vertex = &point[1][(k+1)%nCrossSections];	glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			vertex = &point[0][k];						glVertex3f(vertex->getX(), vertex->getY(), vertex->getZ());
			glEnd();
		}

		if (n==0 && ex->getBeginCap() == true) {
			glBegin(GL_POLYGON);
			for (int k=0; k<nCrossSections; k++)
				glVertex3f(point[0][k].getX(), point[0][k].getY(), point[0][k].getZ());
			glEnd();
		}

		if (n==(nSpines-1)-1 && ex->getEndCap() == true) {
			glBegin(GL_POLYGON);
			for (int k=0; k<nCrossSections; k++)
				glVertex3f(point[1][k].getX(), point[1][k].getY(), point[1][k].getZ());
			glEnd();
		}
	}

	if (ccw == false)
		glFrontFace(GL_CCW);

//	if (solid == false)
		glEnable(GL_CULL_FACE);

	delete []point[0];
	delete []point[1];
}

void ExtrusionNode::recomputeDisplayList()
{
	unsigned int nCurrentDisplayList = getDisplayList();
	if (0 < nCurrentDisplayList)
		glDeleteLists(nCurrentDisplayList, 1);

	unsigned int nNewDisplayList = glGenLists(1);
	glNewList(nNewDisplayList, GL_COMPILE);
		DrawExtrusion(this);
	glEndList();

	setDisplayList(nNewDisplayList);
}

#endif

