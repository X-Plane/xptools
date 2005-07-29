/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Node.h
*
******************************************************************/

#ifndef _CV97_NODE_H_
#define _CV97_NODE_H_

#include <iostream.h>
#include <fstream.h>
#include <assert.h>
#include "Vector.h"
#include "Field.h"
#include "String.h"
#include "LinkedList.h"
#include "SFMatrix.h"
#include "JNode.h"

#include "VRMLString.h"

class	SceneGraph;
	
class	AnchorNode;
class	AppearanceNode;
class	AudioClipNode;
class	BackgroundNode;
class	BillboardNode;
class	BoxNode;
class	CollisionNode;
class	ColorNode;
class	ColorInterpolatorNode;
class	ConeNode;
class	CoordinateNode;
class	CoordinateInterpolatorNode;
class	CylinderNode;
class	CylinderSensorNode;
class	DirectionalLightNode;
class	ElevationGridNode;
class	ExtrusionNode;
class	FogNode;
class	FontStyleNode;
class	GroupNode;
class	ImageTextureNode;
class	IndexedFaceSetNode;
class	IndexedLineSetNode;
class	InlineNode;
class	LodNode;
class	MaterialNode;
class	MovieTextureNode;
class	NavigationInfoNode;
class	NormalNode;
class	NormalInterpolatorNode;
class	OrientationInterpolatorNode;
class	PixelTextureNode;
class	PlaneSensorNode;
class	PointLightNode;
class	PointSetNode;
class	PositionInterpolatorNode;
class	ProximitySensorNode;
class	ScalarInterpolatorNode;
class	ScriptNode;
class	ShapeNode;
class	SoundNode;
class	SphereNode;
class	SphereSensorNode;
class	SpotLightNode;
class	SwitchNode;
class	TextNode;
class	TextureNode;
class	TextureCoordinateNode;
class	TextureTransformNode;
class	TimeSensorNode;
class	TouchSensorNode;
class	TransformNode;
class	ViewpointNode;
class	VisibilitySensorNode;
class	WorldInfoNode;

class	GroupingNode;

class	DEFNode;

class Node : public LinkedListNode<Node> {

public:
	String				*mName;
	String				*mType;
	Vector<Field>		*mExposedField;
	Vector<Field>		*mEventInField;
	Vector<Field>		*mEventOutField;
	Vector<Field>		*mField;
	Vector<Field>		*mPrivateField;
	Vector<Node>		*mPrivateNodeVector;
	bool				*mInitialized;

	String				*mOrgName;
	String				*mOrgType;
	Vector<Field>		*mOrgExposedField;
	Vector<Field>		*mOrgEventInField;
	Vector<Field>		*mOrgEventOutField;
	Vector<Field>		*mOrgField;
	Vector<Field>		*mOrgPrivateField;

private:
	Node				*mParentNode;
	LinkedList<Node>	*mChildNodes;
	SceneGraph			*mSceneGraph;

#ifdef SUPPORT_JSAI
	JNode				*mJNode;
#endif

	void				*mValue;
	Node				*mReferenceNode;

public:

	Node();
	
	Node(char * nodeType, char * nodeName);
	
	virtual ~Node();

	void initializeMember();

	void remove();

	void setName(char * name);
	char *getName();
	bool hasName();

	void setType(char * name);
	char *getType();

	////////////////////////////////////////////////
	//	Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

	void createJavaNodeObject() {
		mJNode = new JNode(this);
	}

	void setJavaNodeObject(JNode *jnode) {
		mJNode = jnode;
	}

	JNode *getJavaNodeObject() {
		return mJNode;
	}

#endif

	////////////////////////////////////////////////
	//	Field
	////////////////////////////////////////////////

	Field *createField(int type);

	////////////////////////////////////////////////
	//	EventIn
	////////////////////////////////////////////////

	Field *getEventIn(char * fieldString);
	int getNEventIn();
	void addEventIn(Field *field);
	void addEventIn(char * name, Field *field);
	void addEventIn(char * name, int fieldType);
	Field *getEventIn(int index);
	int getEventInNumber(Field *field);

	////////////////////////////////////////////////
	//	EventOut
	////////////////////////////////////////////////

	Field *getEventOut(char *fieldString);
	int getNEventOut();
	void addEventOut(Field *field);
	void addEventOut(char *name, Field *field);
	void addEventOut(char * name, int fieldType);
	Field *getEventOut(int index);
	int getEventOutNumber(Field *field);

	////////////////////////////////////////////////
	//	ExposedField
	////////////////////////////////////////////////

	Field *getExposedField(char * fieldString);
	int getNExposedFields();
	void addExposedField(Field *field);
	void addExposedField(char * name, Field *field);
	void addExposedField(char * name, int fieldType);
	Field *getExposedField(int index);
	int getExposedFieldNumber(Field *field);

	////////////////////////////////////////////////
	//	Field
	////////////////////////////////////////////////

	Field *getField(char *fieldString);
	int getNFields();
	void addField(Field *field);
	void addField(char * name, Field *field);
	void addField(char * name, int fieldType);
	Field *getField(int index);
	int getFieldNumber(Field *field);

	////////////////////////////////////////////////
	//	PrivateField
	////////////////////////////////////////////////

	Field *getPrivateField(char *fieldString);
	int getNPrivateFields();
	void addPrivateField(Field *field);
	void addPrivateField(char * name, Field *field);
	Field *getPrivateField(int index);
	int getPrivateFieldNumber(Field *field);

	////////////////////////////////////////////////
	//	PrivateField
	////////////////////////////////////////////////

	int getNPrivateNodeElements();
	void addPrivateNodeElement(Node *node);
	Node *getPrivateNodeElementAt(int n);
	void removeAllNodeElement();

	////////////////////////////////////////////////
	//	Parent node
	////////////////////////////////////////////////

	void setParentNode(Node *parentNode);
	Node *getParentNode();
	bool isParentNode(Node *node);
	bool isAncestorNode(Node *node);

	////////////////////////////////////////////////
	//	Traversal node list
	////////////////////////////////////////////////

	Node *nextTraversal();
	Node *nextTraversalByType(char *typeString);
	Node *nextTraversalByName(char *nameString);

	////////////////////////////////////////////////
	//	next node list
	////////////////////////////////////////////////

	Node *next();
	Node *next(char *typeString);

	////////////////////////////////////////////////
	//	child node list
	////////////////////////////////////////////////

	Node *getChildNodes();
	Node *getChildNode(char *typeString);
	Node *getChildNode(int n);
	
	int getNChildNodes();
	
	virtual bool isChildNodeType(Node *node) = 0;

	void addChildNode(Node *node, bool initialize = true);
	void addChildNodeAtFirst(Node *node, bool initialize = true);

	void moveChildNode(Node *node);
	void moveChildNodeAtFirst(Node *node);

	void deleteChildNodes(void);

	void removeRoutes();
	void removeSFNodes();
	void removeInstanceNodes();

	////////////////////////////////////////////////
	//	Add / Remove children (for Groupingnode)
	////////////////////////////////////////////////

	bool isChildNode(Node *parentNode, Node *node);
	bool isChildNode(Node *node);

	////////////////////////////////////////////////
	//	get child node list
	////////////////////////////////////////////////

	GroupingNode *getGroupingNodes();
	Node *getGeometryNode();
	TextureNode *getTextureNode();

	AnchorNode *getAnchorNodes() {
		return (AnchorNode *)getChildNode(anchorNodeString);
	}

	AppearanceNode *getAppearanceNodes() {
		return (AppearanceNode *)getChildNode(appearanceNodeString);
	}

	AudioClipNode *getAudioClipNodes() {
		return (AudioClipNode *)getChildNode(audioClipNodeString);
	}

	BackgroundNode *getBackgroundNodes() {
		return (BackgroundNode *)getChildNode(backgroundNodeString);
	}

	BillboardNode *getBillboardNodes() {
		return (BillboardNode *)getChildNode(billboardNodeString);
	}

	BoxNode *getBoxeNodes() {
		return (BoxNode *)getChildNode(boxNodeString);
	}

	CollisionNode *getCollisionNodes() {
		return (CollisionNode *)getChildNode(collisionNodeString);
	}

	ColorNode *getColorNodes() {
		return (ColorNode *)getChildNode(colorNodeString);
	}

	ColorInterpolatorNode *getColorInterpolatorNodes() {
		return (ColorInterpolatorNode *)getChildNode(colorInterpolatorNodeString);
	}

	ConeNode *getConeNodes() {
		return (ConeNode *)getChildNode(coneNodeString);
	}

	CoordinateNode *getCoordinateNodes() {
		return (CoordinateNode *)getChildNode(coordinateNodeString);
	}

	CoordinateInterpolatorNode *getCoordinateInterpolatorNodes() {
		return (CoordinateInterpolatorNode *)getChildNode(coordinateInterpolatorNodeString);
	}

	CylinderNode *getCylinderNodes() {
		return (CylinderNode *)getChildNode(cylinderNodeString);
	}

	CylinderSensorNode *getCylinderSensorNodes() {
		return (CylinderSensorNode *)getChildNode(cylinderSensorNodeString);
	}

	DirectionalLightNode *getDirectionalLightNodes() {
		return (DirectionalLightNode *)getChildNode(directionalLightNodeString);
	}

	ElevationGridNode *getElevationGridNodes() {
		return (ElevationGridNode *)getChildNode(elevationGridNodeString);
	}

	ExtrusionNode *getExtrusionNodes() {
		return (ExtrusionNode *)getChildNode(extrusionNodeString);
	}

	FogNode *getFogNodes() {
		return (FogNode *)getChildNode(fogNodeString);
	}

	FontStyleNode *getFontStyleNodes() {
		return (FontStyleNode *)getChildNode(fontStyleNodeString);
	}

	GroupNode *getGroupNodes() {
		return (GroupNode *)getChildNode(groupNodeString);
	}

	ImageTextureNode *getImageTextureNodes() {
		return (ImageTextureNode *)getChildNode(imageTextureNodeString);
	}

	IndexedFaceSetNode *getIndexedFaceSetNodes() {
		return (IndexedFaceSetNode *)getChildNode(indexedFaceSetNodeString);
	}

	IndexedLineSetNode *getIndexedLineSetNodes() {
		return (IndexedLineSetNode *)getChildNode(indexedLineSetNodeString);
	}

	InlineNode *getInlineNodes() {
		return (InlineNode *)getChildNode(inlineNodeString);
	}

	LodNode *getLodNodes() {
		return (LodNode *)getChildNode(lodNodeString);
	}

	MaterialNode *getMaterialNodes() {
		return (MaterialNode *)getChildNode(materialNodeString);
	}

	MovieTextureNode *getMovieTextureNodes() {
		return (MovieTextureNode *)getChildNode(movieTextureNodeString);
	}

	NavigationInfoNode *getNavigationInfoNodes() {
		return (NavigationInfoNode *)getChildNode(navigationInfoNodeString);
	}

	NormalNode *getNormalNodes() {
		return (NormalNode *)getChildNode(normalNodeString);
	}

	NormalInterpolatorNode *getNormalInterpolatorNodes() {
		return (NormalInterpolatorNode *)getChildNode(normalInterpolatorNodeString);
	}

	OrientationInterpolatorNode *getOrientationInterpolatorNodes() {
		return (OrientationInterpolatorNode *)getChildNode(orientationInterpolatorNodeString);
	}

	PixelTextureNode *getPixelTextureNodes() {
		return (PixelTextureNode *)getChildNode(pixelTextureNodeString);
	}

	PlaneSensorNode *getPlaneSensorNodes() {
		return (PlaneSensorNode *)getChildNode(planeSensorNodeString);
	}

	PointLightNode *getPointLightNodes() {
		return (PointLightNode *)getChildNode(pointLightNodeString);
	}

	PointSetNode *getPointSetNodes() {
		return (PointSetNode *)getChildNode(pointSetNodeString);
	}

	PositionInterpolatorNode *getPositionInterpolatorNodes() {
		return (PositionInterpolatorNode *)getChildNode(positionInterpolatorNodeString);
	}

	ProximitySensorNode *getProximitySensorNodes() {
		return (ProximitySensorNode *)getChildNode(proximitySensorNodeString);
	}

	ScalarInterpolatorNode *getScalarInterpolatorNodes() {
		return (ScalarInterpolatorNode *)getChildNode(scalarInterpolatorNodeString);
	}

	ScriptNode *getScriptNodes() {
		return (ScriptNode *)getChildNode(scriptNodeString);
	}

	ShapeNode *getShapeNodes() {
		return (ShapeNode *)getChildNode(shapeNodeString);
	}

	SoundNode *getSoundNodes() {
		return (SoundNode *)getChildNode(soundNodeString);
	}

	SphereNode *getSphereNodes() {
		return (SphereNode *)getChildNode(sphereNodeString);
	}

	SphereSensorNode *getSphereSensorNodes() {
		return (SphereSensorNode *)getChildNode(sphereSensorNodeString);
	}

	SpotLightNode *getSpotLightNodes() {
		return (SpotLightNode *)getChildNode(spotLightNodeString);
	}

	SwitchNode *getSwitchNodes() {
		return (SwitchNode *)getChildNode(switchNodeString);
	}

	TextNode *getTextNodes() {
		return (TextNode *)getChildNode(textNodeString);
	}

	TextureCoordinateNode *getTextureCoordinateNodes() {
		return (TextureCoordinateNode *)getChildNode(textureCoordinateNodeString);
	}

	TextureTransformNode *getTextureTransformNodes() {
		return (TextureTransformNode *)getChildNode(textureTransformNodeString);
	}

	TimeSensorNode *getTimeSensorNodes() {
		return (TimeSensorNode *)getChildNode(timeSensorNodeString);
	}

	TouchSensorNode *getTouchSensorNodes() {
		return (TouchSensorNode *)getChildNode(touchSensorNodeString);
	}

	TransformNode *getTransformNodes() {
		return (TransformNode *)getChildNode(transformNodeString);
	}

	ViewpointNode *getViewpointNodes() {
		return (ViewpointNode *)getChildNode(viewpointNodeString);
	}

	VisibilitySensorNode *getVisibilitySensorNodes() {
		return (VisibilitySensorNode *)getChildNode(visibilitySensorNodeString);
	}

	WorldInfoNode *getWorldInfoNodes() {
		return (WorldInfoNode *)getChildNode(worldInfoNodeString);
	}

	////////////////////////////////////////////////
	//	is*
	////////////////////////////////////////////////

	bool isNode(char * nodeType);
	bool isRootNode();
	bool isDEFNode();
	bool isInlineChildNode();

	bool isGroupingNode() {
		if (isAnchorNode() || isBillboardNode() || isCollisionNode() || isGroupNode() || isTransformNode())
			return true;
		else
			return false;
	}

	bool isSpecialGroupNode() {
		if (isInlineNode() || isLodNode() || isSwitchNode())
			return true;
		else
			return false;
	}

	bool isCommonNode() {
		if (isLightNode() || isAudioClipNode() || isScriptNode() || isShapeNode() || isSoundNode() || isWorldInfoNode())
			return true;
		else
			return false;
	}

	bool isLightNode() {
		if (isDirectionalLightNode() || isSpotLightNode() || isPointLightNode())
			return true;
		else
			return false;
	}

	bool isGeometryNode() {
		if (isBoxNode() || isConeNode() || isCylinderNode() || isElevationGridNode() || isExtrusionNode() || isIndexedFaceSetNode() || isIndexedLineSetNode() || isPointSetNode() || isSphereNode() || isTextNode())
			return true;
		else
			return false;
	}

	bool isGeometryPropertyNode() {
		if (isColorNode() || isCoordinateNode() || isNormalNode() || isTextureCoordinateNode())
			return true;
		else
			return false;
	}

	bool isTextureNode() {
		if (isMovieTextureNode() || isPixelTextureNode() || isImageTextureNode() )
			return true;
		else
			return false;
	}

	bool isSensorNode() {
		if (isCylinderSensorNode() || isPlaneSensorNode() || isSphereSensorNode() || isProximitySensorNode() || isTimeSensorNode() || isTouchSensorNode() || isVisibilitySensorNode())
			return true;
		else
			return false;
	}

	bool isInterpolatorNode() {
		if (isColorInterpolatorNode() || isCoordinateInterpolatorNode() || isNormalInterpolatorNode() || isOrientationInterpolatorNode() || isPositionInterpolatorNode() || isScalarInterpolatorNode())
			return true;
		else
			return false;
	}

	bool isBindableNode() {
		if (isBackgroundNode() || isFogNode() || isNavigationInfoNode() || isViewpointNode())
			return true;
		else
			return false;
	}


	bool isAnchorNode() {
		return isNode(anchorNodeString);
	}

	bool isAppearanceNode() {
		return isNode(appearanceNodeString);
	}

	bool isAudioClipNode() {
		return isNode(audioClipNodeString);
	}

	bool isBackgroundNode() {
		return isNode(backgroundNodeString);
	}

	bool isBillboardNode() {
		return isNode(billboardNodeString);
	}

	bool isBoxNode() {
		return isNode(boxNodeString);
	}

	bool isCollisionNode() {
		return isNode(collisionNodeString);
	}

	bool isColorNode() {
		return isNode(colorNodeString);
	}

	bool isColorInterpolatorNode() {
		return isNode(colorInterpolatorNodeString);
	}

	bool isConeNode() {
		return isNode(coneNodeString);
	}

	bool isCoordinateNode() {
		return isNode(coordinateNodeString);
	}

	bool isCoordinateInterpolatorNode() {
		return isNode(coordinateInterpolatorNodeString);
	}

	bool isCylinderNode() {
		return isNode(cylinderNodeString);
	}

	bool isCylinderSensorNode() {
		return isNode(cylinderSensorNodeString);
	}

	bool isDirectionalLightNode() {
		return isNode(directionalLightNodeString);
	}

	bool isElevationGridNode() {
		return isNode(elevationGridNodeString);
	}

	bool isExtrusionNode() {
		return isNode(extrusionNodeString);
	}

	bool isFogNode() {
		return isNode(fogNodeString);
	}

	bool isFontStyleNode() {
		return isNode(fontStyleNodeString);
	}

	bool isGroupNode() {
		return isNode(groupNodeString);
	}

	bool isImageTextureNode() {
		return isNode(imageTextureNodeString);
	}

	bool isIndexedFaceSetNode() {
		return isNode(indexedFaceSetNodeString);
	}

	bool isIndexedLineSetNode() {
		return isNode(indexedLineSetNodeString);
	}

	bool isInlineNode() {
		return isNode(inlineNodeString);
	}

	bool isLodNode() {
		return isNode(lodNodeString);
	}

	bool isMaterialNode() {
		return isNode(materialNodeString);
	}

	bool isMovieTextureNode() {
		return isNode(movieTextureNodeString);
	}

	bool isNavigationInfoNode() {
		return isNode(navigationInfoNodeString);
	}

	bool isNormalNode() {
		return isNode(normalNodeString);
	}

	bool isNormalInterpolatorNode() {
		return isNode(normalInterpolatorNodeString);
	}

	bool isOrientationInterpolatorNode() {
		return isNode(orientationInterpolatorNodeString);
	}

	bool isPixelTextureNode() {
		return isNode(pixelTextureNodeString);
	}

	bool isPlaneSensorNode() {
		return isNode(planeSensorNodeString);
	}

	bool isPointLightNode() {
		return isNode(pointLightNodeString);
	}

	bool isPointSetNode() {
		return isNode(pointSetNodeString);
	}

	bool isPositionInterpolatorNode() {
		return isNode(positionInterpolatorNodeString);
	}

	bool isProximitySensorNode() {
		return isNode(proximitySensorNodeString);
	}

	bool isScalarInterpolatorNode() {
		return isNode(scalarInterpolatorNodeString);
	}

	bool isScriptNode() {
		return isNode(scriptNodeString);
	}

	bool isShapeNode() {
		return isNode(shapeNodeString);
	}

	bool isSoundNode() {
		return isNode(soundNodeString);
	}

	bool isSphereNode() {
		return isNode(sphereNodeString);
	}

	bool isSphereSensorNode() {
		return isNode(sphereSensorNodeString);
	}

	bool isSpotLightNode() {
		return isNode(spotLightNodeString);
	}

	bool isSwitchNode() {
		return isNode(switchNodeString);
	}

	bool isTextNode() {
		return isNode(textNodeString);
	}

	bool isTextureCoordinateNode() {
		return isNode(textureCoordinateNodeString);
	}

	bool isTextureTransformNode() {
		return isNode(textureTransformNodeString);
	}

	bool isTimeSensorNode() {
		return isNode(timeSensorNodeString);
	}

	bool isTouchSensorNode() {
		return isNode(touchSensorNodeString);
	}

	bool isTransformNode() {
		return isNode(transformNodeString);
	}

	bool isViewpointNode() {
		return isNode(viewpointNodeString);
	}

	bool isVisibilitySensorNode() {
		return isNode(visibilitySensorNodeString);
	}

	bool isWorldInfoNode() {
		return isNode(worldInfoNodeString);
	}

	////////////////////////////////////////////////
	//	output
	////////////////////////////////////////////////

	char *getIndentLevelString(int nIndentLevel);

	void outputHead(ostream& printStream, char *indentString);

	void outputTail(ostream& printStream, char * indentString);

	virtual void outputContext(ostream &printStream, char *indentString) = 0;

	void outputContext(ostream& printStream, char *indentString1, char *indentString2);

	void output(ostream& printStream, int indentLevet);

	void print(ostream& printStream, int indentLevel) {
		output(printStream, indentLevel);
	}

	void print(){
		output(cout, 0);
	}
/*
	void save(ofstream outputStream){
		output(outputStream, 0);
	}
*/

	int save(char * filename) {
		ofstream outputFile(filename, ios::out);
		if (outputFile) {
			output(outputFile, 0);
			return 1;
		}
		else
			return 0;
	}

	////////////////////////////////////////////////
	//	Virtual functions
	////////////////////////////////////////////////

	virtual void update()		= 0;
	virtual void initialize()	= 0;
	virtual void uninitialize()	= 0;

	////////////////////////////////////////////////
	//	Transform matrix
	////////////////////////////////////////////////

	void	getTransformMatrix(SFMatrix *matrix);
	void	getTransformMatrix(float value[4][4]);

	////////////////////////////////////////////////
	//	Translation matrix
	////////////////////////////////////////////////

	void	getTranslationMatrix(SFMatrix *matrix);
	void	getTranslationMatrix(float value[4][4]);

	////////////////////////////////////////////////
	//	SceneGraph
	////////////////////////////////////////////////

	void setSceneGraph(SceneGraph *sceneGraph);
	SceneGraph	*getSceneGraph();

	////////////////////////////////////////////////
	//	Route
	////////////////////////////////////////////////

	void		sendEvent(Field *eventOutField);
	void		sendEvent(char *eventOutFieldString);

	////////////////////////////////////////////////
	//	Value
	////////////////////////////////////////////////

	void		setValue(void *value)	{ mValue = value; }
	void		*getValue()				{ return mValue; }

	////////////////////////////////////////////////
	//	Initialized
	////////////////////////////////////////////////

	void		setInitialized(bool flag)	{ *mInitialized = flag; }
	bool		isInitialized()				{ return *mInitialized; }

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	virtual void recomputeBoundingBox() {
	}

	////////////////////////////////////////////////
	//	DisplayList
	////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

	virtual void recomputeDisplayList() {
	}

#endif

	////////////////////////////////////////////////
	//	Instance node
	////////////////////////////////////////////////

	bool isInstanceNode() {
		return (getReferenceNode() != NULL ? true : false);
	}

	void setReferenceNodeMembers(Node *node);

	void setOriginalMembers();
	
	void setReferenceNode(Node *node) {
		mReferenceNode = node;
	}
	
	Node *getReferenceNode() {
		return mReferenceNode;
	}

	void setAsInstanceNode(Node *node) {
		setReferenceNode(node);
		setReferenceNodeMembers(node);
	}
	
	Node *createInstanceNode();

	////////////////////////////////////////////////
	//	DEF node
	////////////////////////////////////////////////

	DEFNode *createDEFNode();
};

#endif
