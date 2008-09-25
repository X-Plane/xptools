/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SceneGraph.cpp
*
******************************************************************/

#include "SceneGraph.h"

////////////////////////////////////////////////////////////
//	SceneGraph::SceneGraph
////////////////////////////////////////////////////////////

SceneGraph::SceneGraph()
{
//	setHeaderFlag(false);
	setOption(SCENEGRAPH_OPTION_NONE);
	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(-1.0f, -1.0f, -1.0f);
	setSelectedShapeNode(NULL);
	setSelectedNode(NULL);

	mBackgroundNodeVector		= new Vector<BindableNode>;
	mFogNodeVector				= new Vector<BindableNode>;
	mNavigationInfoNodeVector	= new Vector<BindableNode>;
	mViewpointNodeVector		= new Vector<BindableNode>;

	mDefaultBackgroundNode		= new BackgroundNode();
	mDefaultFogNode				= new FogNode();
	mDefaultNavigationInfoNode	= new NavigationInfoNode();
	mDefaultViewpointNode		= new ViewpointNode();

#ifdef SUPPORT_URL
	mUrl = new UrlFile();
#endif
}

////////////////////////////////////////////////
//	find*(char *name)
////////////////////////////////////////////////

AnchorNode *SceneGraph::findAnchorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (AnchorNode *node = findAnchorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

AppearanceNode *SceneGraph::findAppearanceNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (AppearanceNode *node = findAppearanceNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

AudioClipNode *SceneGraph::findAudioClipNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (AudioClipNode *node = findAudioClipNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

BackgroundNode *SceneGraph::findBackgroundNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (BackgroundNode *node = findBackgroundNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

BillboardNode *SceneGraph::findBillboardNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (BillboardNode *node = findBillboardNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

BoxNode *SceneGraph::findBoxNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (BoxNode *node = findBoxNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

CollisionNode *SceneGraph::findCollisionNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (CollisionNode *node = findCollisionNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ColorNode *SceneGraph::findColorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ColorNode *node = findColorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ColorInterpolatorNode *SceneGraph::findColorInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ColorInterpolatorNode *node = findColorInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ConeNode *SceneGraph::findConeNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ConeNode *node = findConeNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

CoordinateNode *SceneGraph::findCoordinateNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (CoordinateNode *node = findCoordinateNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

CoordinateInterpolatorNode *SceneGraph::findCoordinateInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (CoordinateInterpolatorNode *node = findCoordinateInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

CylinderNode *SceneGraph::findCylinderNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (CylinderNode *node = findCylinderNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

CylinderSensorNode *SceneGraph::findCylinderSensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (CylinderSensorNode *node = findCylinderSensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

DirectionalLightNode *SceneGraph::findDirectionalLightNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (DirectionalLightNode *node = findDirectionalLightNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ElevationGridNode *SceneGraph::findElevationGridNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ElevationGridNode *node = findElevationGridNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ExtrusionNode *SceneGraph::findExtrusionNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ExtrusionNode *node = findExtrusionNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

FogNode *SceneGraph::findFogNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (FogNode *node = findFogNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

FontStyleNode *SceneGraph::findFontStyleNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (FontStyleNode *node = findFontStyleNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

GroupNode *SceneGraph::findGroupNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (GroupNode *node = findGroupNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ImageTextureNode *SceneGraph::findImageTextureNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ImageTextureNode *node = findImageTextureNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

IndexedFaceSetNode *SceneGraph::findIndexedFaceSetNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (IndexedFaceSetNode *node = findIndexedFaceSetNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

IndexedLineSetNode *SceneGraph::findIndexedLineSetNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (IndexedLineSetNode *node = findIndexedLineSetNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

InlineNode *SceneGraph::findInlineNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (InlineNode *node = findInlineNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

LodNode *SceneGraph::findLodNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (LodNode *node = findLodNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

MaterialNode *SceneGraph::findMaterialNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (MaterialNode *node = findMaterialNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

MovieTextureNode *SceneGraph::findMovieTextureNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (MovieTextureNode *node = findMovieTextureNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

NavigationInfoNode *SceneGraph::findNavigationInfoNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (NavigationInfoNode *node = findNavigationInfoNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

NormalNode *SceneGraph::findNormalNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (NormalNode *node = findNormalNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

NormalInterpolatorNode *SceneGraph::findNormalInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (NormalInterpolatorNode *node = findNormalInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

OrientationInterpolatorNode *SceneGraph::findOrientationInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (OrientationInterpolatorNode *node = findOrientationInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

PixelTextureNode *SceneGraph::findPixelTextureNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (PixelTextureNode *node = findPixelTextureNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

PlaneSensorNode *SceneGraph::findPlaneSensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (PlaneSensorNode *node = findPlaneSensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

PointLightNode *SceneGraph::findPointLightNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (PointLightNode *node = findPointLightNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

PointSetNode *SceneGraph::findPointSetNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (PointSetNode *node = findPointSetNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

PositionInterpolatorNode *SceneGraph::findPositionInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (PositionInterpolatorNode *node = findPositionInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ProximitySensorNode *SceneGraph::findProximitySensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ProximitySensorNode *node = findProximitySensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ScalarInterpolatorNode *SceneGraph::findScalarInterpolatorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ScalarInterpolatorNode *node = findScalarInterpolatorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ScriptNode *SceneGraph::findScriptNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ScriptNode *node = findScriptNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ShapeNode *SceneGraph::findShapeNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ShapeNode *node = findShapeNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

SoundNode *SceneGraph::findSoundNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (SoundNode *node = findSoundNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

SphereNode *SceneGraph::findSphereNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (SphereNode *node = findSphereNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

SphereSensorNode *SceneGraph::findSphereSensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (SphereSensorNode *node = findSphereSensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

SpotLightNode *SceneGraph::findSpotLightNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (SpotLightNode *node = findSpotLightNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

SwitchNode *SceneGraph::findSwitchNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (SwitchNode *node = findSwitchNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TextNode *SceneGraph::findTextNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TextNode *node = findTextNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TextureCoordinateNode *SceneGraph::findTextureCoordinateNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TextureCoordinateNode *node = findTextureCoordinateNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TextureTransformNode *SceneGraph::findTextureTransformNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TextureTransformNode *node = findTextureTransformNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TimeSensorNode *SceneGraph::findTimeSensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TimeSensorNode *node = findTimeSensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TouchSensorNode *SceneGraph::findTouchSensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TouchSensorNode *node = findTouchSensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

TransformNode *SceneGraph::findTransformNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (TransformNode *node = findTransformNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

ViewpointNode *SceneGraph::findViewpointNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (ViewpointNode *node = findViewpointNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

VisibilitySensorNode *SceneGraph::findVisibilitySensorNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (VisibilitySensorNode *node = findVisibilitySensorNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

WorldInfoNode *SceneGraph::findWorldInfoNode(char *name) {
	if (!name || strlen(name) <= 0)
		return NULL;
	for (WorldInfoNode *node = findWorldInfoNode(); node; node = node->nextTraversal()) {
		char *nodeName = node->getName();
		if (nodeName && strlen(nodeName)) {
			if (!strcmp(name, nodeName))
				return node;
		}
	}
	return NULL;
}

////////////////////////////////////////////////
//	Node Number
////////////////////////////////////////////////

unsigned int SceneGraph::getNodeNumber(Node *node) {
	unsigned int nNode = 1;
	for (Node *n = getNodes(); n; n = n->nextTraversal()) {
		if (n == node)
			return nNode;
		nNode++;
	}
	return 0;
}

////////////////////////////////////////////////////////////
//	SceneGraph::SceneGraph
////////////////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

void SceneGraph::setJavaEnv(char *javaClassPath, jint (JNICALL *printfn)(FILE *fp, const char *format, va_list args))
{
	CreateJavaVM(javaClassPath, printfn);
}

#endif

////////////////////////////////////////////////////////////
//	SceneGraph::~SceneGraph
////////////////////////////////////////////////////////////

SceneGraph::~SceneGraph()
{
	Node *node=getNodes();
	while (node) {
		delete node;
		node = getNodes();
	}
	Route *route=getRoutes();
	while (route) {
		Route *nextRoute=route->next();
		delete route;
		route = nextRoute;
	}

	delete mBackgroundNodeVector;
	delete mFogNodeVector;
	delete mNavigationInfoNodeVector;
	delete mViewpointNodeVector;

	delete mDefaultBackgroundNode;
	delete mDefaultFogNode;
	delete mDefaultNavigationInfoNode;
	delete mDefaultViewpointNode;

#ifdef SUPPORT_URL
	delete mUrl;
#endif

#ifdef SUPPORT_JSAI
	DeleteJavaVM();
#endif
}

////////////////////////////////////////////////
//	child node list
////////////////////////////////////////////////

int SceneGraph::getNAllNodes()
{
	int nNode = 0;
	for (Node *node = Parser::getNodes(); node; node = node->nextTraversal())
		nNode++;
	return nNode;
}

int SceneGraph::getNNodes()
{
	int nNode = 0;
	for (Node *node = Parser::getNodes(); node; node = node->next())
		nNode++;
	return nNode;
}

Node *SceneGraph::getNodes(char *typeName)
{
	Node *node = Parser::getNodes();
	if (node == NULL)
		return NULL;
	String nodeString(node->getType());
	if (nodeString.compareTo(typeName) == 0)
		return node;
	else
		return node->next(typeName);
}

Node *SceneGraph::getNodes()
{
	return Parser::getNodes();
}

////////////////////////////////////////////////
//	find node
////////////////////////////////////////////////

Node *SceneGraph::findNode(char *name)
{
	return Parser::findNodeByName(name);
}

bool SceneGraph::hasNode(Node *targetNode)
{
	for (Node *node = Parser::getNodes(); node; node = node->nextTraversal()) {
		if (node == targetNode)
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////
//	SceneGraph::clear
////////////////////////////////////////////////////////////

void SceneGraph::clear()
{
	clearNodeList();
	clearRouteList();
}

////////////////////////////////////////////////////////////
//	SceneGraph::load
////////////////////////////////////////////////////////////

void SceneGraph::load(char *filename, bool bInitialize, void (*callbackFn)(int nLine, void *info), void *callbackFnInfo)
{
	clear();

	Parser::load(filename, callbackFn, callbackFnInfo);

	if (bInitialize)
		initialize();

	setBackgroundNode(findBackgroundNode(), true);
	setFogNode(findFogNode(), true);
	setNavigationInfoNode(findNavigationInfoNode(), true);
	setViewpointNode(findViewpointNode(), true);
}

void SceneGraph::add(char *filename, bool bInitialize, void (*callbackFn)(int nLine, void *info), void *callbackFnInfo)
{
	Parser::load(filename, callbackFn, callbackFnInfo);

	if (bInitialize)
		initialize();

	setBackgroundNode(findBackgroundNode(), true);
	setFogNode(findFogNode(), true);
	setNavigationInfoNode(findNavigationInfoNode(), true);
	setViewpointNode(findViewpointNode(), true);
}

////////////////////////////////////////////////////////////
//	SceneGraph::save
////////////////////////////////////////////////////////////

bool SceneGraph::save(char *filename, void (*callbackFn)(int nNode, void *info), void *callbackFnInfo)
{

	ofstream outputFile(filename);

	if (!outputFile)
		return false;

	uninitialize();

	outputFile << "#VRML V2.0 utf8" << endl;

	int nNode = 0;
	for (Node *node = Parser::getNodes(); node; node = node->next()) {
		node->output(outputFile, 0);
		nNode++;
		if (callbackFn)
			callbackFn(nNode, callbackFnInfo);
	}
	for (Route *route = Parser::getRoutes(); route; route = route->next()) {
		route->output(outputFile);
	}

	initialize();

	return true;
}

////////////////////////////////////////////////////////////
//	SceneGraph::initialize
////////////////////////////////////////////////////////////

void SceneGraph::initialize(void (*callbackFn)(int nNode, void *info), void *callbackFnInfo)
{
	Node *node;

	int nNode = 0;
	for (node = Parser::getNodes(); node; node = node->nextTraversal()) {
		node->setSceneGraph(this);
		if (node->isInstanceNode() == false)
			node->initialize();
		nNode++;
		if (callbackFn)
			callbackFn(nNode, callbackFnInfo);
	}

	// Convert from InstanceNode into DEFNode
	node = Parser::getNodes();
	while(node != NULL) {
		Node *nextNode = node->nextTraversal();
		if (node->isInstanceNode() == true && node->isDEFNode() == false) {
			Node *referenceNode	= node->getReferenceNode();
			Node *parentNode	= node->getParentNode();
			Node *defNode;

			defNode = referenceNode->createDEFNode();
			if (parentNode != NULL)
				parentNode->addChildNode(defNode, false);
			else
				addNode(defNode, false);

			node->remove();
			delete node;

			nextNode = defNode->nextTraversal();
		}
		node = nextNode;
	}

	// Convert from DEFNode into InstanceNode
	node = Parser::getNodes();
	while(node != NULL) {
		Node *nextNode = node->nextTraversal();

		if (node->isDEFNode() == true) {
			Node *defNode = findNode(node->getName());
			assert(defNode);
			if (defNode) {
				Node *instanceNode = defNode->createInstanceNode();
				Node *parentNode = node->getParentNode();
				if (parentNode != NULL)
					parentNode->moveChildNode(instanceNode);
				else
					moveNode(instanceNode);
				node->remove();
				delete node;
			}
		}

		node = nextNode;
	}

	recomputeBoundingBox();

	for (Route *route = Parser::getRoutes(); route; route = route->next())
		route->initialize();
}

////////////////////////////////////////////////
//	update
////////////////////////////////////////////////

void SceneGraph::update()
{
	for (Node *node = Parser::getNodes(); node; node = node->nextTraversal()) {
		node->update();
	}
}

void SceneGraph::updateRoute(Node *eventOutNode, Field *eventOutField)
{
	for (Route *route = Parser::getRoutes(); route; route = route->next()) {
		if (route->getEventOutNode() == eventOutNode && route->getEventOutField() == eventOutField) {
			route->update();
			route->getEventInNode()->update();
			updateRoute(route->getEventInNode(), route->getEventInField());
		}
	}
}

///////////////////////////////////////////////
//	Output node infomations
///////////////////////////////////////////////

void SceneGraph::print()
{
	uninitialize();

	for (Node *node = Parser::getNodes(); node; node = node->next()) {
		node->print();
	}
	for (Route *route = Parser::getRoutes(); route; route = route->next()) {
		route->output(cout);
	}

	initialize();
}

///////////////////////////////////////////////
//	Delete/Remove Node
///////////////////////////////////////////////

void SceneGraph::removeNode(Node *node)
{
	deleteRoutes(node);
	node->remove();
}

void SceneGraph::deleteNode(Node *node)
{
	deleteRoutes(node);
	delete node;
}

////////////////////////////////////////////////////////////
//	SceneGraph::uninitialize
////////////////////////////////////////////////////////////

void SceneGraph::uninitialize(void (*callbackFn)(int nNode, void *info), void *callbackFnInfo)
{
	int nNode = 0;
	for (Node *node = Parser::getNodes(); node; node = node->nextTraversal()) {
		node->uninitialize();
		nNode++;
		if (callbackFn)
			callbackFn(nNode, callbackFnInfo);
	}
}

////////////////////////////////////////////////
//	BoundingBoxSize
////////////////////////////////////////////////

void SceneGraph::setBoundingBoxSize(float value[])
{
	boundingBoxSize[0] = value[0];
	boundingBoxSize[1] = value[1];
	boundingBoxSize[2] = value[2];
}

void SceneGraph::setBoundingBoxSize(float x, float y, float z)
{
	boundingBoxSize[0] = x;
	boundingBoxSize[1] = y;
	boundingBoxSize[2] = z;
}

void SceneGraph::getBoundingBoxSize(float value[])
{
	value[0] = boundingBoxSize[0];
	value[1] = boundingBoxSize[1];
	value[2] = boundingBoxSize[2];
}

////////////////////////////////////////////////
//	BoundingBoxCenter
////////////////////////////////////////////////

void SceneGraph::setBoundingBoxCenter(float value[])
{
	boundingBoxCenter[0] = value[0];
	boundingBoxCenter[1] = value[1];
	boundingBoxCenter[2] = value[2];
}

void SceneGraph::setBoundingBoxCenter(float x, float y, float z)
{
	boundingBoxCenter[0] = x;
	boundingBoxCenter[1] = y;
	boundingBoxCenter[2] = z;
}

void SceneGraph::getBoundingBoxCenter(float value[])
{
	value[0] = boundingBoxCenter[0];
	value[1] = boundingBoxCenter[1];
	value[2] = boundingBoxCenter[2];
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void SceneGraph::setBoundingBox(BoundingBox *bbox)
{
	float center[3];
	float size[3];
	bbox->getCenter(center);
	bbox->getSize(size);
	setBoundingBoxCenter(center);
	setBoundingBoxSize(size);
}

void SceneGraph::recomputeBoundingBox()
{
	Node	*node;
	float	center[3];
	float	size[3];

	BoundingBox bbox;

	for (node=getNodes(); node; node=node->nextTraversal()) {
		if (node->isGroupingNode()) {
			GroupingNode *gnode = (GroupingNode *)node;
			gnode->getBoundingBoxCenter(center);
			gnode->getBoundingBoxSize(size);
			bbox.addBoundingBox(center, size);
		}
		else if (node->isGeometryNode()) {
			GeometryNode *gnode = (GeometryNode *)node;
			gnode->getBoundingBoxCenter(center);
			gnode->getBoundingBoxSize(size);
			bbox.addBoundingBox(center, size);
		}
	}

	setBoundingBox(&bbox);
}

///////////////////////////////////////////////
//	Bindable Nodes
///////////////////////////////////////////////

void SceneGraph::setBindableNode(Vector<BindableNode> *nodeVector, BindableNode *node, bool bind)
{
	if (!node)
		return;

	BindableNode *topNode = nodeVector->lastElement();

	if (bind) {
		if (topNode != node) {
			if (topNode) {
				topNode->setIsBound(false);
				topNode->sendEvent(topNode->getIsBoundField());
			}

			nodeVector->removeElement(node);
			nodeVector->addElement(node, false);

			node->setIsBound(true);
			node->sendEvent(node->getIsBoundField());
		}
	}
	else {
		if (topNode == node) {
			node->setIsBound(false);
			node->sendEvent(node->getIsBoundField());

			nodeVector->removeElement(node);

			BindableNode *newTopNode = nodeVector->lastElement();
			if (newTopNode) {
				newTopNode->setIsBound(true);
				newTopNode->sendEvent(newTopNode->getIsBoundField());
			}
		}
		else {
			nodeVector->removeElement(node);
		}
	}
}

void SceneGraph::setBindableNode(BindableNode *node, bool bind)
{
	if (node->isBackgroundNode())		setBackgroundNode((BackgroundNode *)node, bind);
	if (node->isFogNode())					setFogNode((FogNode *)node, bind);
	if (node->isNavigationInfoNode())	setNavigationInfoNode((NavigationInfoNode *)node, bind);
	if (node->isViewpointNode())		setViewpointNode((ViewpointNode *)node, bind);
}


///////////////////////////////////////////////
//	Zoom All Viewpoint
///////////////////////////////////////////////

void SceneGraph::zoomAllViewpoint()
{
	float	bboxCenter[3];
	float	bboxSize[3];

	getBoundingBoxCenter(bboxCenter);
	getBoundingBoxSize(bboxSize);

	ViewpointNode *view = getViewpointNode();
	if (view == NULL)
		view = getDefaultViewpointNode();

	float fov = view->getFieldOfView();
	float zoffset = bboxSize[0] / (float)tan(fov);
	view->setPosition(bboxCenter[0], bboxCenter[1], bboxCenter[2] + zoffset*5.0f);
	view->setOrientation(0.0f, 0.0f, 1.0f, 0.0f);
}
