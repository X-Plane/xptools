/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SceneGraph.h
*
******************************************************************/

#ifndef _CV97_SCENEGRAPH_H_
#define _CV97_SCENEGRAPH_H_

#include <iostream.h>
#include <fstream.h>
#include "String.h"
#include "VRMLField.h"
#include "VRMLNodes.h"
#include "Parser.h"
#include "JavaVM.h"
#include "UrlFile.h"
#include "BoundingBox.h"
#include "RouteList.h"
#include "MathUtil.h"

enum {
SCENEGRAPH_OPTION_NONE			= 0x00,
SCENEGRAPH_NORMAL_GENERATION	= 0x01,
SCENEGRAPH_TEXTURE_GENERATION	= 0x02,
};

#ifdef SUPPORT_JSAI
class SceneGraph : public Parser, public JavaVM {
#else
class SceneGraph : public Parser {
#endif

	int						mOption;
	float					boundingBoxCenter[3];
	float					boundingBoxSize[3];

	Vector<BindableNode>	*mBackgroundNodeVector;
	Vector<BindableNode>	*mFogNodeVector;
	Vector<BindableNode>	*mNavigationInfoNodeVector;
	Vector<BindableNode>	*mViewpointNodeVector;

	ShapeNode				*mSelectedShapeNode;
	Node					*mSelectedNode;

	BackgroundNode			*mDefaultBackgroundNode;
	FogNode					*mDefaultFogNode;
	NavigationInfoNode		*mDefaultNavigationInfoNode;
	ViewpointNode			*mDefaultViewpointNode;

#ifdef SUPPORT_URL
	UrlFile					*mUrl;
#endif

public:

	SceneGraph();

#ifdef SUPPORT_JSAI
	void setJavaEnv(char *javaClassPath = NULL, jint (JNICALL *printfn)(FILE *fp, const char *format, va_list args) = NULL);
#endif

	~SceneGraph();

	////////////////////////////////////////////////
	//	Option
	////////////////////////////////////////////////

	void setOption(int option) {
		mOption = option;
	}

	int getOption() {
		return mOption;
	}

	////////////////////////////////////////////////
	//	child node list
	////////////////////////////////////////////////

	int getNAllNodes();
	int getNNodes();
	Node *getNodes(char *typeName);
	Node *getNodes();

	////////////////////////////////////////////////
	//	find node
	////////////////////////////////////////////////

	Node *findNode(char *name);
	bool hasNode(Node *targetNode);

	////////////////////////////////////////////////
	//	child node list
	////////////////////////////////////////////////

	GroupingNode *getGroupingNodes() {
		for (Node *node = Parser::getNodes(); node; node = node->next()) {
			if (node->isGroupingNode())
				return (GroupingNode *)node;
		}
		return NULL;
	}

	AnchorNode *getAnchorNodes() {
		return (AnchorNode *)getNodes(anchorNodeString);
	}

	AppearanceNode *getAppearanceNodes() {
		return (AppearanceNode *)getNodes(appearanceNodeString);
	}

	AudioClipNode *getAudioClipNodes() {
		return (AudioClipNode *)getNodes(audioClipNodeString);
	}

	BackgroundNode *getBackgroundNodes() {
		return (BackgroundNode *)getNodes(backgroundNodeString);
	}

	BillboardNode *getBillboardNodes() {
		return (BillboardNode *)getNodes(billboardNodeString);
	}

	BoxNode *getBoxeNodes() {
		return (BoxNode *)getNodes(boxNodeString);
	}

	CollisionNode *getCollisionNodes() {
		return (CollisionNode *)getNodes(collisionNodeString);
	}

	ColorNode *getColorNodes() {
		return (ColorNode *)getNodes(colorNodeString);
	}

	ColorInterpolatorNode *getColorInterpolatorNodes() {
		return (ColorInterpolatorNode *)getNodes(colorInterpolatorNodeString);
	}

	ConeNode *getConeNodes() {
		return (ConeNode *)getNodes(coneNodeString);
	}

	CoordinateNode *getCoordinateNodes() {
		return (CoordinateNode *)getNodes(coordinateNodeString);
	}

	CoordinateInterpolatorNode *getCoordinateInterpolatorNodes() {
		return (CoordinateInterpolatorNode *)getNodes(coordinateInterpolatorNodeString);
	}

	CylinderNode *getCylinderNodes() {
		return (CylinderNode *)getNodes(cylinderNodeString);
	}

	CylinderSensorNode *getCylinderSensorNodes() {
		return (CylinderSensorNode *)getNodes(cylinderSensorNodeString);
	}

	DirectionalLightNode *getDirectionalLightNodes() {
		return (DirectionalLightNode *)getNodes(directionalLightNodeString);
	}

	ElevationGridNode *getElevationGridNodes() {
		return (ElevationGridNode *)getNodes(elevationGridNodeString);
	}

	ExtrusionNode *getExtrusionNodes() {
		return (ExtrusionNode *)getNodes(extrusionNodeString);
	}

	FogNode *getFogNodes() {
		return (FogNode *)getNodes(fogNodeString);
	}

	FontStyleNode *getFontStyleNodes() {
		return (FontStyleNode *)getNodes(fontStyleNodeString);
	}

	GroupNode *getGroupNodes() {
		return (GroupNode *)getNodes(groupNodeString);
	}

	ImageTextureNode *getImageTextureNodes() {
		return (ImageTextureNode *)getNodes(imageTextureNodeString);
	}

	IndexedFaceSetNode *getIndexedFaceSetNodes() {
		return (IndexedFaceSetNode *)getNodes(indexedFaceSetNodeString);
	}

	IndexedLineSetNode *getIndexedLineSetNodes() {
		return (IndexedLineSetNode *)getNodes(indexedLineSetNodeString);
	}

	InlineNode *getInlineNodes() {
		return (InlineNode *)getNodes(inlineNodeString);
	}

	LodNode *getLodNodes() {
		return (LodNode *)getNodes(lodNodeString);
	}

	MaterialNode *getMaterialNodes() {
		return (MaterialNode *)getNodes(materialNodeString);
	}

	MovieTextureNode *getMovieTextureNodes() {
		return (MovieTextureNode *)getNodes(movieTextureNodeString);
	}

	NavigationInfoNode *getNavigationInfoNodes() {
		return (NavigationInfoNode *)getNodes(navigationInfoNodeString);
	}

	NormalNode *getNormalNodes() {
		return (NormalNode *)getNodes(normalNodeString);
	}

	NormalInterpolatorNode *getNormalInterpolatorNodes() {
		return (NormalInterpolatorNode *)getNodes(normalInterpolatorNodeString);
	}

	OrientationInterpolatorNode *getOrientationInterpolatorNodes() {
		return (OrientationInterpolatorNode *)getNodes(orientationInterpolatorNodeString);
	}

	PixelTextureNode *getPixelTextureNodes() {
		return (PixelTextureNode *)getNodes(pixelTextureNodeString);
	}

	PlaneSensorNode *getPlaneSensorNodes() {
		return (PlaneSensorNode *)getNodes(planeSensorNodeString);
	}

	PointLightNode *getPointLightNodes() {
		return (PointLightNode *)getNodes(pointLightNodeString);
	}

	PointSetNode *getPointSetNodes() {
		return (PointSetNode *)getNodes(pointSetNodeString);
	}

	PositionInterpolatorNode *getPositionInterpolatorNodes() {
		return (PositionInterpolatorNode *)getNodes(positionInterpolatorNodeString);
	}

	ProximitySensorNode *getProximitySensorNodes() {
		return (ProximitySensorNode *)getNodes(proximitySensorNodeString);
	}

	ScalarInterpolatorNode *getScalarInterpolatorNodes() {
		return (ScalarInterpolatorNode *)getNodes(scalarInterpolatorNodeString);
	}

	ScriptNode *getScriptNodes() {
		return (ScriptNode *)getNodes(scriptNodeString);
	}

	ShapeNode *getShapeNodes() {
		return (ShapeNode *)getNodes(shapeNodeString);
	}

	SoundNode *getSoundNodes() {
		return (SoundNode *)getNodes(soundNodeString);
	}

	SphereNode *getSphereNodes() {
		return (SphereNode *)getNodes(sphereNodeString);
	}

	SphereSensorNode *getSphereSensorNodes() {
		return (SphereSensorNode *)getNodes(sphereSensorNodeString);
	}

	SpotLightNode *getSpotLightNodes() {
		return (SpotLightNode *)getNodes(spotLightNodeString);
	}

	SwitchNode *getSwitchNodes() {
		return (SwitchNode *)getNodes(switchNodeString);
	}

	TextNode *getTextNodes() {
		return (TextNode *)getNodes(textNodeString);
	}

	TextureCoordinateNode *getTextureCoordinateNodes() {
		return (TextureCoordinateNode *)getNodes(textureCoordinateNodeString);
	}

	TextureTransformNode *getTextureTransformNodes() {
		return (TextureTransformNode *)getNodes(textureTransformNodeString);
	}

	TimeSensorNode *getTimeSensorNodes() {
		return (TimeSensorNode *)getNodes(timeSensorNodeString);
	}

	TouchSensorNode *getTouchSensorNodes() {
		return (TouchSensorNode *)getNodes(touchSensorNodeString);
	}

	TransformNode *getTransformNodes() {
		return (TransformNode *)getNodes(transformNodeString);
	}

	ViewpointNode *getViewpointNodes() {
		return (ViewpointNode *)getNodes(viewpointNodeString);
	}

	VisibilitySensorNode *getVisibilitySensorNodes() {
		return (VisibilitySensorNode *)getNodes(visibilitySensorNodeString);
	}

	WorldInfoNode *getWorldInfoNodes() {
		return (WorldInfoNode *)getNodes(worldInfoNodeString);
	}

	////////////////////////////////////////////////
	//	child node list
	////////////////////////////////////////////////

	GroupingNode *findGroupingNode() {
		for (Node *node = (getRootNode())->nextTraversal() ; node; node = node->nextTraversal()) {
			if (node->isGroupingNode())
				return (GroupingNode *)node;
		}
		return NULL;
	}

	AnchorNode *findAnchorNode() {
		return (AnchorNode *)findNodeByType(anchorNodeString);
	}

	AppearanceNode *findAppearanceNode() {
		return (AppearanceNode *)findNodeByType(appearanceNodeString);
	}

	AudioClipNode *findAudioClipNode() {
		return (AudioClipNode *)findNodeByType(audioClipNodeString);
	}

	BackgroundNode *findBackgroundNode() {
		return (BackgroundNode *)findNodeByType(backgroundNodeString);
	}

	BillboardNode *findBillboardNode() {
		return (BillboardNode *)findNodeByType(billboardNodeString);
	}

	BoxNode *findBoxNode() {
		return (BoxNode *)findNodeByType(boxNodeString);
	}

	CollisionNode *findCollisionNode() {
		return (CollisionNode *)findNodeByType(collisionNodeString);
	}

	ColorNode *findColorNode() {
		return (ColorNode *)findNodeByType(colorNodeString);
	}

	ColorInterpolatorNode *findColorInterpolatorNode() {
		return (ColorInterpolatorNode *)findNodeByType(colorInterpolatorNodeString);
	}

	ConeNode *findConeNode() {
		return (ConeNode *)findNodeByType(coneNodeString);
	}

	CoordinateNode *findCoordinateNode() {
		return (CoordinateNode *)findNodeByType(coordinateNodeString);
	}

	CoordinateInterpolatorNode *findCoordinateInterpolatorNode() {
		return (CoordinateInterpolatorNode *)findNodeByType(coordinateInterpolatorNodeString);
	}

	CylinderNode *findCylinderNode() {
		return (CylinderNode *)findNodeByType(cylinderNodeString);
	}

	CylinderSensorNode *findCylinderSensorNode() {
		return (CylinderSensorNode *)findNodeByType(cylinderSensorNodeString);
	}

	DirectionalLightNode *findDirectionalLightNode() {
		return (DirectionalLightNode *)findNodeByType(directionalLightNodeString);
	}

	ElevationGridNode *findElevationGridNode() {
		return (ElevationGridNode *)findNodeByType(elevationGridNodeString);
	}

	ExtrusionNode *findExtrusionNode() {
		return (ExtrusionNode *)findNodeByType(extrusionNodeString);
	}

	FogNode *findFogNode() {
		return (FogNode *)findNodeByType(fogNodeString);
	}

	FontStyleNode *findFontStyleNode() {
		return (FontStyleNode *)findNodeByType(fontStyleNodeString);
	}

	GroupNode *findGroupNode() {
		return (GroupNode *)findNodeByType(groupNodeString);
	}

	ImageTextureNode *findImageTextureNode() {
		return (ImageTextureNode *)findNodeByType(imageTextureNodeString);
	}

	IndexedFaceSetNode *findIndexedFaceSetNode() {
		return (IndexedFaceSetNode *)findNodeByType(indexedFaceSetNodeString);
	}

	IndexedLineSetNode *findIndexedLineSetNode() {
		return (IndexedLineSetNode *)findNodeByType(indexedLineSetNodeString);
	}

	InlineNode *findInlineNode() {
		return (InlineNode *)findNodeByType(inlineNodeString);
	}

	LodNode *findLodNode() {
		return (LodNode *)findNodeByType(lodNodeString);
	}

	MaterialNode *findMaterialNode() {
		return (MaterialNode *)findNodeByType(materialNodeString);
	}

	MovieTextureNode *findMovieTextureNode() {
		return (MovieTextureNode *)findNodeByType(movieTextureNodeString);
	}

	NavigationInfoNode *findNavigationInfoNode() {
		return (NavigationInfoNode *)findNodeByType(navigationInfoNodeString);
	}

	NormalNode *findNormalNode() {
		return (NormalNode *)findNodeByType(normalNodeString);
	}

	NormalInterpolatorNode *findNormalInterpolatorNode() {
		return (NormalInterpolatorNode *)findNodeByType(normalInterpolatorNodeString);
	}

	OrientationInterpolatorNode *findOrientationInterpolatorNode() {
		return (OrientationInterpolatorNode *)findNodeByType(orientationInterpolatorNodeString);
	}

	PixelTextureNode *findPixelTextureNode() {
		return (PixelTextureNode *)findNodeByType(pixelTextureNodeString);
	}

	PlaneSensorNode *findPlaneSensorNode() {
		return (PlaneSensorNode *)findNodeByType(planeSensorNodeString);
	}

	PointLightNode *findPointLightNode() {
		return (PointLightNode *)findNodeByType(pointLightNodeString);
	}

	PointSetNode *findPointSetNode() {
		return (PointSetNode *)findNodeByType(pointSetNodeString);
	}

	PositionInterpolatorNode *findPositionInterpolatorNode() {
		return (PositionInterpolatorNode *)findNodeByType(positionInterpolatorNodeString);
	}

	ProximitySensorNode *findProximitySensorNode() {
		return (ProximitySensorNode *)findNodeByType(proximitySensorNodeString);
	}

	ScalarInterpolatorNode *findScalarInterpolatorNode() {
		return (ScalarInterpolatorNode *)findNodeByType(scalarInterpolatorNodeString);
	}

	ScriptNode *findScriptNode() {
		return (ScriptNode *)findNodeByType(scriptNodeString);
	}

	ShapeNode *findShapeNode() {
		return (ShapeNode *)findNodeByType(shapeNodeString);
	}

	SoundNode *findSoundNode() {
		return (SoundNode *)findNodeByType(soundNodeString);
	}

	SphereNode *findSphereNode() {
		return (SphereNode *)findNodeByType(sphereNodeString);
	}

	SphereSensorNode *findSphereSensorNode() {
		return (SphereSensorNode *)findNodeByType(sphereSensorNodeString);
	}

	SpotLightNode *findSpotLightNode() {
		return (SpotLightNode *)findNodeByType(spotLightNodeString);
	}

	SwitchNode *findSwitchNode() {
		return (SwitchNode *)findNodeByType(switchNodeString);
	}

	TextNode *findTextNode() {
		return (TextNode *)findNodeByType(textNodeString);
	}

	TextureCoordinateNode *findTextureCoordinateNode() {
		return (TextureCoordinateNode *)findNodeByType(textureCoordinateNodeString);
	}

	TextureTransformNode *findTextureTransformNode() {
		return (TextureTransformNode *)findNodeByType(textureTransformNodeString);
	}

	TimeSensorNode *findTimeSensorNode() {
		return (TimeSensorNode *)findNodeByType(timeSensorNodeString);
	}

	TouchSensorNode *findTouchSensorNode() {
		return (TouchSensorNode *)findNodeByType(touchSensorNodeString);
	}

	TransformNode *findTransformNode() {
		return (TransformNode *)findNodeByType(transformNodeString);
	}

	ViewpointNode *findViewpointNode() {
		return (ViewpointNode *)findNodeByType(viewpointNodeString);
	}

	VisibilitySensorNode *findVisibilitySensorNode() {
		return (VisibilitySensorNode *)findNodeByType(visibilitySensorNodeString);
	}

	WorldInfoNode *findWorldInfoNode() {
		return (WorldInfoNode *)findNodeByType(worldInfoNodeString);
	}

	////////////////////////////////////////////////
	//	find*(char *name)
	////////////////////////////////////////////////

	AnchorNode *findAnchorNode(char *name) ;
	AppearanceNode *findAppearanceNode(char *name) ;
	AudioClipNode *findAudioClipNode(char *name) ;
	BackgroundNode *findBackgroundNode(char *name) ;
	BillboardNode *findBillboardNode(char *name) ;
	BoxNode *findBoxNode(char *name) ;
	CollisionNode *findCollisionNode(char *name) ;
	ColorNode *findColorNode(char *name) ;
	ColorInterpolatorNode *findColorInterpolatorNode(char *name) ;
	ConeNode *findConeNode(char *name) ;
	CoordinateNode *findCoordinateNode(char *name) ;
	CoordinateInterpolatorNode *findCoordinateInterpolatorNode(char *name) ;
	CylinderNode *findCylinderNode(char *name) ;
	CylinderSensorNode *findCylinderSensorNode(char *name) ;
	DirectionalLightNode *findDirectionalLightNode(char *name) ;
	ElevationGridNode *findElevationGridNode(char *name) ;
	ExtrusionNode *findExtrusionNode(char *name) ;
	FogNode *findFogNode(char *name) ;
	FontStyleNode *findFontStyleNode(char *name) ;
	GroupNode *findGroupNode(char *name) ;
	ImageTextureNode *findImageTextureNode(char *name) ;
	IndexedFaceSetNode *findIndexedFaceSetNode(char *name) ;
	IndexedLineSetNode *findIndexedLineSetNode(char *name) ;
	InlineNode *findInlineNode(char *name) ;
	LodNode *findLodNode(char *name) ;
	MaterialNode *findMaterialNode(char *name) ;
	MovieTextureNode *findMovieTextureNode(char *name) ;
	NavigationInfoNode *findNavigationInfoNode(char *name) ;
	NormalNode *findNormalNode(char *name) ;
	NormalInterpolatorNode *findNormalInterpolatorNode(char *name) ;
	OrientationInterpolatorNode *findOrientationInterpolatorNode(char *name) ;
	PixelTextureNode *findPixelTextureNode(char *name) ;
	PlaneSensorNode *findPlaneSensorNode(char *name) ;
	PointLightNode *findPointLightNode(char *name) ;
	PointSetNode *findPointSetNode(char *name) ;
	PositionInterpolatorNode *findPositionInterpolatorNode(char *name) ;
	ProximitySensorNode *findProximitySensorNode(char *name) ;
	ScalarInterpolatorNode *findScalarInterpolatorNode(char *name) ;
	ScriptNode *findScriptNode(char *name) ;
	ShapeNode *findShapeNode(char *name) ;
	SoundNode *findSoundNode(char *name) ;
	SphereNode *findSphereNode(char *name) ;
	SphereSensorNode *findSphereSensorNode(char *name) ;
	SpotLightNode *findSpotLightNode(char *name) ;
	SwitchNode *findSwitchNode(char *name) ;
	TextNode *findTextNode(char *name) ;
	TextureCoordinateNode *findTextureCoordinateNode(char *name) ;
	TextureTransformNode *findTextureTransformNode(char *name) ;
	TimeSensorNode *findTimeSensorNode(char *name) ;
	TouchSensorNode *findTouchSensorNode(char *name) ;
	TransformNode *findTransformNode(char *name) ;
	ViewpointNode *findViewpointNode(char *name) ;
	VisibilitySensorNode *findVisibilitySensorNode(char *name) ;
	WorldInfoNode *findWorldInfoNode(char *name) ;

	////////////////////////////////////////////////
	//	Node Number
	////////////////////////////////////////////////

	unsigned int getNodeNumber(Node *node) ;

	////////////////////////////////////////////////
	//	initialize
	////////////////////////////////////////////////

	void initialize(void (*callbackFn)(int nNode, void *info) = NULL, void *callbackFnInfo = NULL);

	void uninitialize(void (*callbackFn)(int nNode, void *info) = NULL, void *callbackFnInfo = NULL);

	////////////////////////////////////////////////
	//	update
	////////////////////////////////////////////////

	void update();
	void updateRoute(Node *eventOutNode, Field *eventOutField);

	///////////////////////////////////////////////
	//	Output node infomations
	///////////////////////////////////////////////

	void print();

	///////////////////////////////////////////////
	//	Delete/Remove Node
	///////////////////////////////////////////////

	void removeNode(Node *node);
	void deleteNode(Node *node);

	///////////////////////////////////////////////
	//	Bindable Nodes
	///////////////////////////////////////////////

	void setBindableNode(Vector<BindableNode> *nodeVector, BindableNode *node, bool bind);

	void setBindableNode(BindableNode *node, bool bind);

	void setBackgroundNode(BackgroundNode *bg, bool bind) {
		setBindableNode(mBackgroundNodeVector, bg, bind);
	}

	void setFogNode(FogNode *fog, bool bind) {
		setBindableNode(mFogNodeVector, fog, bind);
	}

	void setNavigationInfoNode(NavigationInfoNode *navInfo, bool bind) {
		setBindableNode(mNavigationInfoNodeVector, navInfo, bind);
	}

	void setViewpointNode(ViewpointNode *view, bool bind) {
		setBindableNode(mViewpointNodeVector, view, bind);
	}

	BackgroundNode *getBackgroundNode() {
		return (BackgroundNode *)mBackgroundNodeVector->lastElement();
	}

	FogNode *getFogNode() {
		return (FogNode *)mFogNodeVector->lastElement();
	}

	NavigationInfoNode *getNavigationInfoNode() {
		return (NavigationInfoNode *)mNavigationInfoNodeVector->lastElement();
	}

	ViewpointNode *getViewpointNode() {
		return (ViewpointNode *)mViewpointNodeVector->lastElement();
	}

	////////////////////////////////////////////////
	//	BoundingBoxSize
	////////////////////////////////////////////////

	void setBoundingBoxSize(float value[]);
	void setBoundingBoxSize(float x, float y, float z);
	void getBoundingBoxSize(float value[]);

	////////////////////////////////////////////////
	//	BoundingBoxCenter
	////////////////////////////////////////////////

	void setBoundingBoxCenter(float value[]);
	void setBoundingBoxCenter(float x, float y, float z);
	void getBoundingBoxCenter(float value[]);

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	void setBoundingBox(BoundingBox *bbox) ;
	void recomputeBoundingBox();

	///////////////////////////////////////////////
	//	Load
	///////////////////////////////////////////////

	void clear();

	void load(char *filename, bool bInitialize = true, void (*callbackFn)(int nLine, void *info) = NULL, void *callbackFnInfo = NULL);

	void add(char *filename, bool bInitialize = true, void (*callbackFn)(int nLine, void *info) = NULL, void *callbackFnInfo = NULL);

	///////////////////////////////////////////////
	//	Save node infomations
	///////////////////////////////////////////////

	bool save(char *filename, void (*callbackFn)(int nNode, void *info) = NULL, void *callbackFnInfo = NULL);

	///////////////////////////////////////////////
	//	URL
	///////////////////////////////////////////////

#ifdef SUPPORT_URL

	void	setUrl(char *url)				{ mUrl->setUrl(url); }
	char	*getUrl()						{ return mUrl->getUrl(); }
	bool	getUrlStream(char *urlStrnig)	{ return mUrl->getStream(urlStrnig); }
	char	*getUrlOutputFilename()			{ return mUrl->getOutputFilename(); }
	bool	deleteUrlOutputFilename()		{ return mUrl->deleteOutputFilename(); }

#endif

	//////////////////////////////////////////////////
	// Selected Shape/Node
	//////////////////////////////////////////////////

	void			setSelectedShapeNode(ShapeNode *shape)	{ mSelectedShapeNode = shape; }
	ShapeNode		*getSelectedShapeNode()					{ return mSelectedShapeNode; }

	void			setSelectedNode(Node *node)				{ mSelectedNode = node; }
	Node			*getSelectedNode()						{ return mSelectedNode; }

	//////////////////////////////////////////////////
	// Default Bindable Nodes
	//////////////////////////////////////////////////

	BackgroundNode		*getDefaultBackgroundNode()		{ return mDefaultBackgroundNode; }
	FogNode				*getDefaultFogNode()			{ return mDefaultFogNode; }
	NavigationInfoNode	*getDefaultNavigationInfoNode()	{ return mDefaultNavigationInfoNode; }
	ViewpointNode		*getDefaultViewpointNode()		{ return mDefaultViewpointNode; }

	//////////////////////////////////////////////////
	// Zoom All
	//////////////////////////////////////////////////

	void			zoomAllViewpoint();
};

#endif
