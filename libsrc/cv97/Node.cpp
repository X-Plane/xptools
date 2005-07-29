/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Node.cpp
*
******************************************************************/

#include <assert.h>
#include "Node.h"
#include "TransformNode.h"
#include "SceneGraph.h"

////////////////////////////////////////////////
//	Node::Node
////////////////////////////////////////////////

void Node::initializeMember()
{
	mName				= mOrgName				= new String();
	mType				= mOrgType				= new String();
	mExposedField		= mOrgExposedField		= new Vector<Field>();
	mEventInField		= mOrgEventInField		= new Vector<Field>();
	mEventOutField		= mOrgEventOutField		= new Vector<Field>();
	mField				= mOrgField				= new Vector<Field>();
	mPrivateField		= mOrgPrivateField		= new Vector<Field>();

	mPrivateNodeVector	= new Vector<Node>();
	mInitialized		= new bool;

	mChildNodes			= new LinkedList<Node>();

	setName(NULL);
	setParentNode(NULL);
	setSceneGraph(NULL);
#ifdef SUPPORT_JSAI
	setJavaNodeObject(NULL);
#endif
	setValue(NULL);
	setInitialized(false);
	setName(NULL);
	setReferenceNode(NULL);
}

Node::Node() 
{
	setHeaderFlag(true);
	initializeMember();
}
	
Node::Node(char * nodeType, char * nodeName) 
{
	setHeaderFlag(false);
	initializeMember();

	setType(nodeType);
	setName(nodeName);
}
	
////////////////////////////////////////////////
//	Node::~Node
////////////////////////////////////////////////

void Node::deleteChildNodes(void)
{
	Node *node=getChildNodes();
	while (node) {
		Node *nextNode = node->next();
		delete node;
		node = nextNode;
	}
}

Node::~Node() 
{
	deleteChildNodes();

	SceneGraph *sg = getSceneGraph();
	if (sg) {
		if (sg->getSelectedShapeNode() == this)
			sg->setSelectedShapeNode(NULL);
		if (sg->getSelectedNode() == this)
			sg->setSelectedNode(NULL);
	}

	remove();

	if (isInstanceNode() == true)
		setOriginalMembers();

#ifdef SUPPORT_JSAI
	delete mJNode;
#endif

	delete mName;
	delete mType;
	delete mExposedField;
	delete mEventInField;
	delete mEventOutField;
	delete mField;
	delete mPrivateField;
	delete mPrivateNodeVector;
	delete mChildNodes;
	delete mInitialized;
}

////////////////////////////////////////////////
//	Name
////////////////////////////////////////////////

void Node::setName(char * name) 
{
	String	nodeName(name);
	char *nameString = nodeName.getValue(); 
	for (int n=0; n<nodeName.length(); n++) {
		if (nameString[n] <= 0x20)
			nameString[n] = '_';
	}
	mName->setValue(nameString);
}

char *Node::getName() 
{
	return mName->getValue();
}

bool Node::hasName() 
{
	char *name = getName();
	if (name == NULL)
		return false;
	if (strlen(name) <= 0)
		return false;
	return true;
}

////////////////////////////////////////////////
//	Type
////////////////////////////////////////////////

void Node::setType(char * name) 
{
	mType->setValue(name);
}

char *Node::getType() 
{
	return mType->getValue();
}

////////////////////////////////////////////////
//	Node::addChildNode
////////////////////////////////////////////////

void Node::addChildNode(Node *node, bool initialize) {
	moveChildNode(node);
	if (initialize)
		node->initialize();
}

void Node::addChildNodeAtFirst(Node *node, bool initialize) {
	moveChildNodeAtFirst(node);
	if (initialize)
		node->initialize();
}

////////////////////////////////////////////////
//	Node::moveChildNode
////////////////////////////////////////////////

void Node::moveChildNode(Node *node) {
	mChildNodes->addNode(node); 
	node->setParentNode(this);
	node->setSceneGraph(getSceneGraph());
}

void Node::moveChildNodeAtFirst(Node *node) {
	mChildNodes->addNodeAtFirst(node); 
	node->setParentNode(this);
	node->setSceneGraph(getSceneGraph());
}

////////////////////////////////////////////////
//	Node::remove
////////////////////////////////////////////////

void Node::removeRoutes() 
{
	SceneGraph *sg = getSceneGraph();
	if (sg) {
		Route *route=sg->getRoutes();
		while (route) {
			Route *nextRoute = route->next();
			if (route->getEventInNode() == this || route->getEventOutNode() == this)
				delete route;
			route = nextRoute;
		}
	}
}

void Node::removeSFNodes() 
{
	SceneGraph *sg = getSceneGraph();
	if (sg) {
		for (ScriptNode *script = sg->findScriptNode(); script; script=script->nextTraversal()) {
			for (int n=0; n<script->getNFields(); n++) {
				Field *field = script->getField(n);
				if (field->getType() == fieldTypeSFNode) {
					SFNode *sfnode = (SFNode *)field;
					if (sfnode->getValue() == this)
						sfnode->setValue((Node *)NULL);
				}
			}
		}
	}
}

void Node::removeInstanceNodes() 
{
	SceneGraph *sg = getSceneGraph();
	if (sg && isInstanceNode() == false) {
		Node *node = sg->getNodes();
		while (node) {
			Node *nextNode = node->nextTraversal();
			if (node->isInstanceNode() == true) {
				Node *refNode = node->getReferenceNode();
				while (refNode->isInstanceNode() == true)
					refNode = refNode->getReferenceNode();
				if (refNode == this) {
					node->deleteChildNodes();
					nextNode = node->nextTraversal();
					delete node;
				}
			}
			node = nextNode;
		}
	
	}
}

void Node::remove() 
{
	LinkedListNode<Node>::remove();

	if (isInstanceNode() == false) {
		removeRoutes();
		removeSFNodes();
		removeInstanceNodes();

		if (isBindableNode()) {
			SceneGraph *sceneGraph = getSceneGraph();
			if (sceneGraph)
				sceneGraph->setBindableNode((BindableNode *)this, false);			
		}
	}

	setParentNode(NULL);
	setSceneGraph(NULL);
}

////////////////////////////////////////////////
//	Node::createField
////////////////////////////////////////////////

Field *Node::createField(int type)
{
	Field	*field = NULL;

	switch (type) {
	case fieldTypeSFBool:
		field = new SFBool();
		break;
	case fieldTypeSFFloat:
		field = new SFFloat();
		break;
	case fieldTypeSFInt32:
		field = new SFInt32();
		break;
	case fieldTypeSFVec2f:
		field = new SFVec2f();
		break;
	case fieldTypeSFVec3f:
		field = new SFVec3f();
		break;
	case fieldTypeSFString:
		field = new SFString();
		break;
	case fieldTypeSFColor:
		field = new SFColor();
		break;
	case fieldTypeSFTime:
		field = new SFTime();
		break;
	case fieldTypeSFRotation:
		field = new SFRotation();
		break;
	}

	assert(field != NULL);

	return field;
}

////////////////////////////////////////////////
//	EventIn
////////////////////////////////////////////////

Field *Node::getEventIn(char * fieldString) 
{

	String fieldName(fieldString);
		
	int nEventIn = getNEventIn();
	for (int n=0; n<nEventIn; n++) {
		Field *field = getEventIn(n);
		if (fieldName.compareTo(field->getName()) == 0)
			return field;
		if (fieldName.startsWith(eventInStripString) == 0) {
			if (fieldName.endsWith(field->getName()) == 0)
				return field;
		}
	}

	return NULL;
}

int Node::getNEventIn() 
{
	return mEventInField->size();
}

void Node::addEventIn(Field *field) 
{
	assert(field->getName() && strlen(field->getName()));
	assert(!getEventIn(field->getName()));
	mEventInField->addElement(field);
}

void Node::addEventIn(char * name, Field *field) 
{
	assert(name && strlen(name));
	assert(!getEventIn(name));
	field->setName(name);
	mEventInField->addElement(field);
}

void Node::addEventIn(char * name, int fieldType) 
{
	addEventIn(name, createField(fieldType));
}

Field *Node::getEventIn(int index) 
{
	return (Field *)mEventInField->elementAt(index);
}

int Node::getEventInNumber(Field *field) 
{
	int nEventIn = getNEventIn();
	for (int n=0; n<nEventIn; n++) {
		if (getEventIn(n) == field)
			return n;
	}
	return -1;
}

////////////////////////////////////////////////
//	EventOut
////////////////////////////////////////////////

Field *Node::getEventOut(char *fieldString) 
{

	String fieldName(fieldString);

	int nEventOut = getNEventOut();
	for (int n=0; n<nEventOut; n++) {
		Field *field = getEventOut(n);
		if (fieldName.compareTo(field->getName()) == 0)
			return field;
		if (fieldName.endsWith(eventOutStripString) == 0) {
			if (fieldName.startsWith(field->getName())  == 0)
				return field;
		}
	}
	return NULL;
}

int Node::getNEventOut() 
{
	return mEventOutField->size();
}

void Node::addEventOut(Field *field) 
{
	assert(field->getName() && strlen(field->getName()));
	assert(!getEventOut(field->getName()));
	mEventOutField->addElement(field);
}

void Node::addEventOut(char *name, Field *field) 
{
	assert(name && strlen(name));
	assert(!getEventOut(name));
	field->setName(name);
	mEventOutField->addElement(field);
}

void Node::addEventOut(char * name, int fieldType) 
{
	addEventOut(name, createField(fieldType));
}

Field *Node::getEventOut(int index) 
{
	return (Field *)mEventOutField->elementAt(index);
}

int Node::getEventOutNumber(Field *field) 
{
	int nEventOut = getNEventOut();
	for (int n=0; n<nEventOut; n++) {
		if (getEventOut(n) == field)
			return n;
	}
	return -1;
}

////////////////////////////////////////////////
//	ExposedField
////////////////////////////////////////////////

Field *Node::getExposedField(char * fieldString) 
{
	
	String fieldName(fieldString);

	int nExposedField = getNExposedFields();
	for (int n=0; n<nExposedField; n++) {
		Field *field = getExposedField(n);
		char *filedName = field->getName();
		if (fieldName.compareTo(filedName) == 0)
			return field;
		if (fieldName.startsWith(eventInStripString) == 0) {
			if (fieldName.endsWith(filedName) == 0)
				return field;
		}
		if (fieldName.endsWith(eventOutStripString) == 0) {
			if (fieldName.startsWith(filedName) == 0)
				return field;
		}
	}
	return NULL;
}

int Node::getNExposedFields() 
{
	return mExposedField->size();
}

void Node::addExposedField(Field *field) 
{
	assert(field->getName() && strlen(field->getName()));
	assert(!getExposedField(field->getName()));
	mExposedField->addElement(field);
}

void Node::addExposedField(char * name, Field *field) 
{
	assert(name && strlen(name));
	assert(!getExposedField(name));
	field->setName(name);
	mExposedField->addElement(field);
}

void Node::addExposedField(char * name, int fieldType) 
{
	addExposedField(name, createField(fieldType));
}

Field *Node::getExposedField(int index) 
{
	return (Field *)mExposedField->elementAt(index);
}

int Node::getExposedFieldNumber(Field *field) 
{
	int nExposedField = getNExposedFields();
	for (int n=0; n<nExposedField; n++) {
		if (getExposedField(n) == field)
			return n;
	}
	return -1;
}

////////////////////////////////////////////////
//	Field
////////////////////////////////////////////////

Field *Node::getField(char *fieldString) 
{
	String fieldName(fieldString);

	int nField = getNFields();
	for (int n=0; n<nField; n++) {
		Field *field = getField(n);
		if (fieldName.compareTo(field->getName()) == 0)
			return field;
	}
	return NULL;
}

int Node::getNFields() 
{
	return mField->size();
}

void Node::addField(Field *field) 
{
	assert(field->getName() && strlen(field->getName()));
	assert(!getField(field->getName()));
	mField->addElement(field);
}

void Node::addField(char * name, Field *field) 
{
	assert(name && strlen(name));
	assert(!getField(name));
	field->setName(name);
	mField->addElement(field);
}

void Node::addField(char * name, int fieldType) 
{
	addField(name, createField(fieldType));
}

Field *Node::getField(int index) 
{
	return (Field *)mField->elementAt(index);
}

int Node::getFieldNumber(Field *field) 
{
	int nField = getNFields();
	for (int n=0; n<nField; n++) {
		if (getField(n) == field)
			return n;
	}
	return -1;
}

////////////////////////////////////////////////
//	PrivateField
////////////////////////////////////////////////

Field *Node::getPrivateField(char *fieldString) 
{
		
	String fieldName(fieldString);

	int nPrivateField = getNPrivateFields();
	for (int n=0; n<nPrivateField; n++) {
		Field *field = getPrivateField(n);
		if (fieldName.compareTo(field->getName()) == 0)
			return field;
	}
	return NULL;
}

int Node::getNPrivateFields() 
{
	return mPrivateField->size();
}

void Node::addPrivateField(Field *field) 
{
	assert(field->getName() && strlen(field->getName()));
	assert(!getPrivateField(field->getName()));
	mPrivateField->addElement(field);
}

void Node::addPrivateField(char * name, Field *field) 
{
	assert(name && strlen(name));
	assert(!getPrivateField(name));
	field->setName(name);
	mPrivateField->addElement(field);
}

Field *Node::getPrivateField(int index) 
{
	return (Field *)mPrivateField->elementAt(index);
}

int Node::getPrivateFieldNumber(Field *field) 
{
	int nPrivateField = getNPrivateFields();
	for (int n=0; n<nPrivateField; n++) {
		if (getPrivateField(n) == field)
			return n;
	}
	return -1;
}

////////////////////////////////////////////////
//	PrivateField
////////////////////////////////////////////////

int Node::getNPrivateNodeElements() 
{
	return mPrivateNodeVector->size();
}

void Node::addPrivateNodeElement(Node *node) 
{
	mPrivateNodeVector->addElement(node, false);
}

Node *Node::getPrivateNodeElementAt(int n) 
{
	return mPrivateNodeVector->elementAt(n);
}

void Node::removeAllNodeElement() 
{
	mPrivateNodeVector->removeAllElements();
}

////////////////////////////////////////////////
//	Parent node
////////////////////////////////////////////////

void Node::setParentNode(Node *parentNode) 
{
	mParentNode = parentNode;
}

Node *Node::getParentNode() 
{
	return mParentNode;
}

bool Node::isParentNode(Node *node) 
{
	return (getParentNode() == node) ? true : false;
}

bool Node::isAncestorNode(Node *node) 
{
	for (Node *parentNode = getParentNode(); parentNode; parentNode = parentNode->getParentNode()) {
		if (node == parentNode)
				return true;
	}
	return false;
}

////////////////////////////////////////////////
//	Traversal node list
////////////////////////////////////////////////

Node *Node::nextTraversal() 
{
	Node *nextNode = getChildNodes();
	if (nextNode != NULL)
		return nextNode;
	nextNode = next();
	if (nextNode == NULL) {
		Node *parentNode = getParentNode();
		while (parentNode != NULL) { 
			Node *parentNextNode = parentNode->next();
			if (parentNextNode != NULL)
				return parentNextNode;
			parentNode = parentNode->getParentNode();
		}
	}
	return nextNode;
}

Node *Node::nextTraversalByType(char *typeString) 
{
	if (typeString == NULL)
		return NULL;
		
	String type(typeString);

	for (Node *node = nextTraversal(); node != NULL; node = node->nextTraversal()) {
		if (node->getType() != NULL) {
			if (type.compareTo(node->getType()) == 0)
				return node;
		}
	}
	return NULL;
}

Node *Node::nextTraversalByName(char *nameString) 
{
	if (nameString == NULL)
		return NULL;

	String name(nameString);

	for (Node *node = nextTraversal(); node != NULL; node = node->nextTraversal()) {
		if (node->getName() != NULL) {
			if (name.compareTo(node->getName()) == 0)
				return node;
		}
	}
	return NULL;
}

////////////////////////////////////////////////
//	next node list
////////////////////////////////////////////////

Node *Node::next() 
{
	return LinkedListNode<Node>::next(); 
}

Node *Node::next(char *typeString) 
{
	String type(typeString);
	for (Node *node = next(); node != NULL; node = node->next()) {
		if (type.compareTo(node->getType()) == 0)
			return node;
	}
	return NULL;
}

////////////////////////////////////////////////
//	child node list
////////////////////////////////////////////////

Node *Node::getChildNodes() 
{
	return mChildNodes->getNodes();
}

Node *Node::getChildNode(char *typeString) 
{

	String type(typeString);
		
	for (Node *node = getChildNodes(); node != NULL; node = node->next()) {
		if (type.compareTo(node->getType()) == 0)
			return node;
	}
	return NULL;
}

Node *Node::getChildNode(int n) 
{
	return mChildNodes->getNode(n);
}

int Node::getNChildNodes() 
{
	return mChildNodes->getNNodes();
}

////////////////////////////////////////////////
//	Add / Remove children (for Groupingnode)
////////////////////////////////////////////////

bool Node::isChildNode(Node *parentNode, Node *node) 
{
	for (Node *cnode = parentNode->getChildNodes(); cnode != NULL; cnode = cnode->next()) {
		if (cnode == node)
			return true;
		if (isChildNode(cnode, node) == true)
			return true;
	}
	return false;
}

bool Node::isChildNode(Node *node) 
{
	for (Node *cnode = getChildNodes(); cnode != NULL; cnode = cnode->next()) {
		if (isChildNode(cnode, node) == true)
			return true;
	}
	return false;
}

////////////////////////////////////////////////
//	get child node list
////////////////////////////////////////////////

GroupingNode *Node::getGroupingNodes() 
{
	for (Node *node = getChildNodes(); node != NULL; node = node->next()) {
		if (node->isGroupingNode())
			return (GroupingNode *)node;
	}
	return NULL;
}

Node *Node::getGeometryNode() 
{
	for (Node *node = getChildNodes(); node != NULL; node = node->next()) {
		if (node->isGeometryNode())
			return node;
	}
	return NULL;
}

TextureNode *Node::getTextureNode() 
{
	for (Node *node = getChildNodes(); node != NULL; node = node->next()) {
		if (node->isTextureNode())
			return (TextureNode *)node;
	}
	return NULL;
}

////////////////////////////////////////////////
//	Node::getTransformMatrix(SFMatrix *matrix)
////////////////////////////////////////////////

void Node::getTransformMatrix(SFMatrix *mxOut)
{
	mxOut->init();

	for (Node *node=this; node; node=node->getParentNode()) {
		if (node->isTransformNode() || node->isBillboardNode()) {
			SFMatrix	mxNode;
			if (node->isTransformNode())
				((TransformNode *)node)->getSFMatrix(&mxNode);
			else
				((BillboardNode *)node)->getSFMatrix(&mxNode);
			mxNode.add(mxOut);
			mxOut->setValue(&mxNode);
		}
	}
}

////////////////////////////////////////////////
// is*
////////////////////////////////////////////////

bool Node::isNode(char * nodeType) 
{
	if (!nodeType)
		return false;
	char *nodeString = getType();
	if (!nodeString)
		return false;
	if (strcmp(nodeString, nodeType) == 0)
		return true;
	else
		return false;
}

bool Node::isRootNode() 
{
	return isNode(rootNodeString);
}

bool Node::isDEFNode() 
{
	return isNode(defNodeString);
}

bool Node::isInlineChildNode() 
{
	Node *parentNode = getParentNode();
	while (parentNode != NULL) {
		if (parentNode->isInlineNode() == true)
			return true;
		parentNode = parentNode->getParentNode();
	}
	return false;
}

////////////////////////////////////////////////
//	SceneGraph
////////////////////////////////////////////////

void Node::setSceneGraph(SceneGraph *sceneGraph)	
{
	mSceneGraph = sceneGraph;
	for (Node *node = getChildNodes(); node; node = node->next()) {
			node->setSceneGraph(sceneGraph);
	}
}

SceneGraph *Node::getSceneGraph() 
{
	return mSceneGraph;
}

////////////////////////////////////////////////
//	Node::getTransformMatrix(float value[4][4])
////////////////////////////////////////////////

void Node::getTransformMatrix(float value[4][4])
{
	SFMatrix	mx;
	getTransformMatrix(&mx);
	mx.getValue(value);
}

////////////////////////////////////////////////
//	Node::getTranslationMatrix(SFMatrix *matrix)
////////////////////////////////////////////////

void Node::getTranslationMatrix(SFMatrix *mxOut)
{
	mxOut->init();

	for (Node *node=this; node; node=node->getParentNode()) {
		if (node->isTransformNode() || node->isBillboardNode()) {
			SFMatrix	mxNode;
			if (node->isTransformNode()) {
				float	translation[3];
				TransformNode *transNode = (TransformNode *)node;
				transNode->getTranslation(translation);
				mxNode.setTranslation(translation);
			}
			mxNode.add(mxOut);
			mxOut->setValue(&mxNode);
		}
	}
}

////////////////////////////////////////////////
//	Node::getTranslationMatrix(float value[4][4])
////////////////////////////////////////////////

void Node::getTranslationMatrix(float value[4][4])
{
	SFMatrix	mx;
	getTranslationMatrix(&mx);
	mx.getValue(value);
}

////////////////////////////////////////////////
//	Node::Route
////////////////////////////////////////////////

void Node::sendEvent(Field *eventOutField) {
	getSceneGraph()->updateRoute(this, eventOutField);
}

void Node::sendEvent(char *eventOutFieldString) {
	getSceneGraph()->updateRoute(this, getEventOut(eventOutFieldString));
}

////////////////////////////////////////////////
//	Node::output
////////////////////////////////////////////////

char *Node::getIndentLevelString(int nIndentLevel) 
{
	char *indentString = new char[nIndentLevel+1];
	for (int n=0; n<nIndentLevel; n++)
		indentString[n] = '\t';
	indentString[nIndentLevel] = '\0';
	return indentString;
}

void Node::outputHead(ostream& printStream, char *indentString) 
{
	if (getName() != NULL && strlen(getName()))
		printStream << indentString << "DEF " << getName() << " " << getType() << " {" << endl;
	else
		printStream << indentString << getType() << " {" << endl;
}

void Node::outputTail(ostream& printStream, char * indentString) 
{
	printStream << indentString << "}" << endl;
}

void Node::outputContext(ostream& printStream, char *indentString1, char *indentString2) 
{
	char *indentString = new char[strlen(indentString1)+strlen(indentString2)+1];
	strcpy(indentString, indentString1);
	strcat(indentString, indentString2);
	outputContext(printStream, indentString);
	delete indentString;
}

void Node::output(ostream& printStream, int indentLevet) 
{
	char *indentString = getIndentLevelString(indentLevet);

	if (isInstanceNode() == false) {
		outputHead(printStream, indentString);
		outputContext(printStream, indentString);
	
		if (!isElevationGridNode() && !isShapeNode() && !isSoundNode() && !isPointSetNode() && !isIndexedFaceSetNode() && 
			!isIndexedLineSetNode() && !isTextNode() && !isAppearanceNode()) {
			if (getChildNodes() != NULL) {
				if (isLodNode()) 
					printStream << indentString << "\tlevel [" << endl;
				else if (isSwitchNode()) 
					printStream << indentString << "\tchoice [" << endl;
				else
					printStream << indentString <<"\tchildren [" << endl;
			
				for (Node *node = getChildNodes(); node; node = node->next()) {
					if (node->isInstanceNode() == false) 
						node->output(printStream, indentLevet+2);
					else
						node->output(printStream, indentLevet+2);
				}
			
				printStream << indentString << "\t]" << endl;
			}
		}
		outputTail(printStream, indentString);
	}
	else 
		printStream << indentString << "USE " << getName() << endl;

	delete indentString;
}

////////////////////////////////////////////////
//	InstanceNode
////////////////////////////////////////////////

void Node::setReferenceNodeMembers(Node *node) 
{
	if (!node)
		return;

	mName				= node->mName;
	//mType				= node->mType;
	mExposedField		= node->mExposedField;
	mEventInField		= node->mEventInField;
	mEventOutField		= node->mEventOutField;
	mField				= node->mField;
	mPrivateField		= node->mPrivateField;
}

void Node::setOriginalMembers() 
{
	mName				= mOrgName;
	//mType				= mOrgType;
	mExposedField		= mOrgExposedField;
	mEventInField		= mOrgEventInField;
	mEventOutField		= mOrgEventOutField;
	mField				= mOrgField;
	mPrivateField		= mOrgPrivateField;
}
	

Node *Node::createInstanceNode() 
{
	Node *instanceNode = NULL;
		
	if (isAnchorNode())
		instanceNode = new AnchorNode();
	else if (isAppearanceNode()) 
		instanceNode = new AppearanceNode();
	else if (isAudioClipNode())
		instanceNode = new AudioClipNode();
	else if (isBackgroundNode())
		instanceNode = new BackgroundNode();
	else if (isBillboardNode())
		instanceNode = new BillboardNode();
	else if (isBoxNode())
		instanceNode = new BoxNode();
	else if (isCollisionNode())
		instanceNode = new CollisionNode();
	else if (isColorNode())
		instanceNode = new ColorNode();
	else if (isColorInterpolatorNode())
		instanceNode = new ColorInterpolatorNode();
	else if (isConeNode())
		instanceNode = new ConeNode();
	else if (isCoordinateNode())
		instanceNode = new CoordinateNode();
	else if (isCoordinateInterpolatorNode())
		instanceNode = new CoordinateInterpolatorNode();
	else if (isCylinderNode())
		instanceNode = new CylinderNode();
	else if (isCylinderSensorNode())
		instanceNode = new CylinderSensorNode();
	else if (isDirectionalLightNode())
		instanceNode = new DirectionalLightNode();
	else if (isElevationGridNode())
		instanceNode = new ElevationGridNode();
	else if (isExtrusionNode())
		instanceNode = new ExtrusionNode();
	else if (isFogNode())
		instanceNode = new FogNode();
	else if (isFontStyleNode())
		instanceNode = new FontStyleNode();
	else if (isGroupNode())
		instanceNode = new GroupNode();
	else if (isImageTextureNode())
		instanceNode = new ImageTextureNode();
	else if (isIndexedFaceSetNode())
		instanceNode = new IndexedFaceSetNode();
	else if (isIndexedLineSetNode()) 
		instanceNode = new IndexedLineSetNode();
	else if (isInlineNode()) 
		instanceNode = new InlineNode();
	else if (isLodNode())
		instanceNode = new LodNode();
	else if (isMaterialNode())
		instanceNode = new MaterialNode();
	else if (isMovieTextureNode())
		instanceNode = new MovieTextureNode();
	else if (isNavigationInfoNode())
		instanceNode = new NavigationInfoNode();
	else if (isNormalNode())
		instanceNode = new NormalNode();
	else if (isNormalInterpolatorNode())
		instanceNode = new NormalInterpolatorNode();
	else if (isOrientationInterpolatorNode())
		instanceNode = new OrientationInterpolatorNode();
	else if (isPixelTextureNode())
		instanceNode = new PixelTextureNode();
	else if (isPlaneSensorNode())
		instanceNode = new PlaneSensorNode();
	else if (isPointLightNode())
		instanceNode = new PointLightNode();
	else if (isPointSetNode())
		instanceNode = new PointSetNode();
	else if (isPositionInterpolatorNode())
		instanceNode = new PositionInterpolatorNode();
	else if (isProximitySensorNode())
		instanceNode = new ProximitySensorNode();
	else if (isScalarInterpolatorNode())
		instanceNode = new ScalarInterpolatorNode();
	else if (isScriptNode())
		instanceNode = new ScriptNode();
	else if (isShapeNode())
		instanceNode = new ShapeNode();
	else if (isSoundNode())
		instanceNode = new SoundNode();
	else if (isSphereNode())
		instanceNode = new SphereNode();
	else if (isSphereSensorNode())
		instanceNode = new SphereSensorNode();
	else if (isSpotLightNode())
		instanceNode = new SpotLightNode();
	else if (isSwitchNode())
		instanceNode = new SwitchNode();
	else if (isTextNode())
		instanceNode = new TextNode();
	else if (isTextureCoordinateNode())
		instanceNode = new TextureCoordinateNode();
	else if (isTextureTransformNode())
		instanceNode = new TextureTransformNode();
	else if (isTimeSensorNode())
		instanceNode = new TimeSensorNode();
	else if (isTouchSensorNode())
		instanceNode = new TouchSensorNode();
	else if (isTransformNode())
		instanceNode = new TransformNode();
	else if (isViewpointNode())
		instanceNode = new ViewpointNode();
	else if (isVisibilitySensorNode())
		instanceNode = new VisibilitySensorNode();
	else if (isWorldInfoNode())
		instanceNode = new WorldInfoNode();

	assert(instanceNode);

	if (instanceNode) {
		Node *refNode = this;
		while (refNode->isInstanceNode() == true) 
			refNode = refNode->getReferenceNode();
		instanceNode->setAsInstanceNode(refNode);
		for (Node *cnode=getChildNodes(); cnode; cnode = cnode->next()) {
			Node *childInstanceNode = cnode->createInstanceNode();
			instanceNode->addChildNode(childInstanceNode);
		}
	}		
		
	return instanceNode;
}

////////////////////////////////////////////////
//	DEF node
////////////////////////////////////////////////

DEFNode *Node::createDEFNode() 
{
	DEFNode *defNode = new DEFNode();

	Node *refNode = this;
	while (refNode->isInstanceNode() == true) 
		refNode = refNode->getReferenceNode();
	defNode->setAsInstanceNode(refNode);

	return defNode;
}
