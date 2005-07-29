%{
/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	vrml.l
*
******************************************************************/
 
#include <stdio.h>
#include <string.h>
#include "SceneGraph.h"
#include "vrml.tab.h"

static int	nCurrentLine = 1;
static char	*buffer;
static char	*lineBuffer;
static char	*name;
static void (*callbackFn)(int nLine, void *info);
static void *callbackFnInfo;

#define input	yyinput
#define output	yyoutput

void CurrentLineIncrement();
int UnputString(char *pBegin);

%}

%start	SCRIPTNODE
%option interactive
%option always-interactive

%%

-?[0-9]+								{ yylval.ival = atoi(yytext); return NUMBER;}
-?[0-9]*\.?[0-9]*([eE][+-]?[0-9]+)?		{ yylval.fval = (float)atof(yytext); return FLOAT;}
0x[0-9A-Fa-f]+							{ sscanf(yytext, "0x%x", &yylval.ival); return NUMBER; }

","					{ return ','; }
"}"					{ BEGIN INITIAL;	return '}'; }
"{"					{ return '{'; }
"["					{ return '['; }
"]"					{ return ']'; }

[ \t]+				;
#[^\r\n]*[\r\n]		{ CurrentLineIncrement();}
#[^\r\n]*			{ }
[\n\r][^\r\n]*		{ strcpy(lineBuffer, yytext+1); CurrentLineIncrement(); yyless(1);}
\"[^\"]*\"			{ yytext[yyleng-1] = '\0'; yylval.sval = yytext+1;  return STRING; }

TRUE				{ yylval.ival = 1; return NUMBER; }
FALSE				{ yylval.ival = 0; return NUMBER; }
NULL				{ return NULL_STRING; }

<INITIAL>Anchor				{ return ANCHOR; }
<INITIAL>Appearance			{ return APPEARANCE; }
<INITIAL>AudioClip			{ return AUDIOCLIP; }
<INITIAL>Background			{ return BACKGROUND; }
<INITIAL>Billboard			{ return BILLBOARD; }
<INITIAL>Box				{ return BOX; }
<INITIAL>Collision			{ return COLLISION; }
<INITIAL>Color				{ return COLOR; }
<INITIAL>ColorInterpolator	{ return COLOR_INTERP; }
<INITIAL>Cone				{ return CONE; }
<INITIAL>Coordinate			{ return COORDINATE; }
<INITIAL>CoordinateInterpolator	{ return COORDINATE_INTERP; }
<INITIAL>Cylinder			{ return CYLINDER; }
<INITIAL>CylinderSensor		{ return CYLINDER_SENSOR; }
<INITIAL>DirectionalLight	{ return DIRECTIONALLIGHT; }
<INITIAL>ElevationGrid		{ return ELEVATION_GRID; }
<INITIAL>Extrusion			{ return EXTRUSION; }
<INITIAL>Fog				{ return FOG; }
<INITIAL>FontStyle			{ return FONTSTYLE; }
<INITIAL>Group				{ return GROUP; }
<INITIAL>ImageTexture		{ return IMAGE_TEXTURE; }
<INITIAL>IndexedFaceSet		{ return INDEXEDFACESET; }
<INITIAL>IndexedLineSet		{ return INDEXEDLINESET; }
<INITIAL>Inline				{ return INLINE; }
<INITIAL>LOD				{ return LOD; }
<INITIAL>Material			{ return MATERIAL; }
<INITIAL>MovieTexture		{ return MOVIE_TEXTURE; }
<INITIAL>NavigationInfo		{ return NAVIGATION_INFO; }
<INITIAL>Normal				{ return NORMAL; }
<INITIAL>NormalInterpolator	{ return NORMAL_INTERP; }
<INITIAL>OFF				{ yylval.ival = 1; return NUMBER; }
<INITIAL>ON					{ yylval.ival = 0; return NUMBER; }
<INITIAL>OrientationInterpolator	{ return ORIENTATION_INTERP; }
<INITIAL>PixelTexture		{ return PIXEL_TEXTURE; }
<INITIAL>PlaneSensor		{ return PLANE_SENSOR; }
<INITIAL>PointLight			{ return POINTLIGHT; }
<INITIAL>PointSet			{ return POINTSET; }
<INITIAL>PositionInterpolator	{ return POSITION_INTERP; }
<INITIAL>ProximitySensor	{ return PROXIMITY_SENSOR; }
<INITIAL>ScalarInterpolator	{ return SCALAR_INTERP; }
<INITIAL>Script				{ BEGIN SCRIPTNODE;	return SCRIPT; }
<INITIAL>Shape				{ return SHAPE; }
<INITIAL>Sound				{ return SOUND; }
<INITIAL>Sphere				{ return SPHERE; }
<INITIAL>SphereSensor		{ return SPHERE_SENSOR; }
<INITIAL>SpotLight			{ return SPOTLIGHT; }
<INITIAL>Switch				{ return SWITCH; }
<INITIAL>Text				{ return TEXT; }
<INITIAL>TextureCoordinate	{ return TEXTURE_COORDINATE; }
<INITIAL>TextureTransform	{ return TEXTURE_TRANSFORM; }
<INITIAL>TimeSensor			{ return TIME_SENSOR; }
<INITIAL>TouchSensor		{ return TOUCH_SENSOR; }
<INITIAL>Transform			{ return TRANSFORM; }
<INITIAL>Viewpoint			{ return VIEWPOINT; }
<INITIAL>VisibilitySensor	{ return VISIBILITY_SENSOR; }
<INITIAL>WorldInfo			{ return WORLD_INFO; }

<INITIAL>ambientIntensity	{ return S_AMBIENT_INTENSITY; }
<INITIAL>appearance			{ return S_APPEARANCE; }
<INITIAL>attenuation		{ return S_ATTENUATION; }
<INITIAL>autoOffset			{ return S_AUTO_OFFSET; }
<INITIAL>avatarSize			{ return S_AVATAR_SIZE; }
<INITIAL>axisOfRotation		{ return S_AXIS_OF_ROTATION;}
<INITIAL>backUrl			{ return S_BACKURL; }
<INITIAL>bboxCenter			{ return S_BBOXCENTER; }
<INITIAL>bboxSize			{ return S_BBOXSIZE; }
<INITIAL>beamWidth			{ return S_BERM_WIDTH; }
<INITIAL>beginCap			{ return S_BEGIN_CAP; }
<INITIAL>bottom				{ return S_BOTTOM; }
<INITIAL>bottomRadius		{ return S_BOTTOMRADIUS; }
<INITIAL>bottomUrl			{ return S_BOTTOMURL; }
<INITIAL>ccw				{ return S_CCW; }
<INITIAL>center				{ return S_CENTER; }
<INITIAL>children			{ return S_CHILDREN; }
<INITIAL>choice				{ return S_CHOICE; }
<INITIAL>collide			{ return S_COLLIDE; }
<INITIAL>collideTime		{ return S_COLLIDETIME;}
<INITIAL>color				{ return S_COLOR; }
<INITIAL>colorIndex			{ return S_COLOR_INDEX; }
<INITIAL>colorPerVertex		{ return S_COLOR_PER_VERTEX; }
<INITIAL>convex				{ return S_CONVEX; }
<INITIAL>coord				{ return S_COORD; }
<INITIAL>coordIndex			{ return S_COORD_INDEX; }
<INITIAL>creaseAngle		{ return S_CREASE_ANGLE;}
<INITIAL>crossSection		{ return S_CROSS_SECTION; }
<INITIAL>cutOffAngle		{ return S_CUTOFFANGLE; }
<INITIAL>cycleInterval		{ return S_CYCLE_INTERVAL; }
<INITIAL>description		{ return S_DESCRIPTION; }
<INITIAL>diffuseColor		{ return S_DIFFUSECOLOR; }
<INITIAL>direction			{ return S_DIRECTION; }
<INITIAL>diskAngle			{ return S_DISK_ANGLE; }
<INITIAL>emissiveColor		{ return S_EMISSIVECOLOR; }
<INITIAL>enabled			{ return S_ENABLED; }
<INITIAL>endCap				{ return S_END_CAP; }
<INITIAL>family				{ return S_FAMILY; }
<INITIAL>fieldOfView		{ return S_FIELD_OF_VIEW; }
<INITIAL>fogType			{ return S_FOG_TYPE; }
<INITIAL>fontStyle			{ return S_FONTSTYLE; }
<INITIAL>frontUrl			{ return S_FRONTURL; }
<INITIAL>geometry			{ return S_GEOMETRY; }
<INITIAL>groundAngle		{ return S_GROUNDANGLE; }
<INITIAL>groundColor		{ return S_GROUNDCOLOR; }
<INITIAL>headlight			{ return S_HEADLIGHT; }
<INITIAL>height				{ return S_HEIGHT; }
<INITIAL>horizontal			{ return S_HORIZONTAL; }
<INITIAL>image				{ return S_IMAGE; }
<INITIAL>info				{ return S_INFO; }
<INITIAL>intensity			{ return S_INTENSITY; }
<INITIAL>jump				{ return S_JUMP; }
<INITIAL>justify			{ return S_JUSTIFY; }
<INITIAL>key				{ return S_KEY; }
<INITIAL>keyValue			{ return S_KEYVALUE; }
<INITIAL>language			{ return S_LANGUAGE; }
<INITIAL>leftToRight		{ return S_LEFT2RIGHT; }
<INITIAL>leftUrl			{ return S_LEFTURL; }
<INITIAL>length				{ return S_LENGTH; }
<INITIAL>level				{ return S_LEVEL; }
<INITIAL>location			{ return S_LOCATION; }
<INITIAL>loop				{ return S_LOOP; }
<INITIAL>material			{ return S_MATERIAL; }
<INITIAL>maxAngle			{ return S_MAX_ANGLE; }
<INITIAL>maxBack			{ return S_MAX_BACK; }
<INITIAL>maxExtent			{ return S_MAX_EXTENT; }
<INITIAL>maxFront			{ return S_MAX_FRONT; }
<INITIAL>maxPosition		{ return S_MAX_POSITION; }
<INITIAL>minAngle			{ return S_MIN_ANGLE; }
<INITIAL>minBack			{ return S_MIN_BACK; }
<INITIAL>minFront			{ return S_MIN_FRONT; }
<INITIAL>minPosition		{ return S_MIN_POSITION; }
<INITIAL>normal				{ return S_NORMAL; }
<INITIAL>normalIndex		{ return S_NORMAL_INDEX; }
<INITIAL>normalPerVertex	{ return S_NORMAL_PER_VERTEX; }
<INITIAL>offset				{ return S_OFFSET;}
<INITIAL>on					{ return S_ON; }
<INITIAL>orientation		{ return S_ORIENTATION; }
<INITIAL>parameter			{ return S_PARAMETER; }
<INITIAL>pitch				{ return S_PICTH; }
<INITIAL>point				{ return S_POINT; }
<INITIAL>position			{ return S_POSITION; }
<INITIAL>priority			{ return S_PRIORITY; }
<INITIAL>proxy				{ return S_PROXY; }
<INITIAL>radius				{ return S_RADIUS; }
<INITIAL>range				{ return S_RANGE; }
<INITIAL>repeatS			{ return S_REPEAT_S; }
<INITIAL>repeatT			{ return S_REPEAT_T; }
<INITIAL>rightUrl			{ return S_RIGHTURL; }
<INITIAL>rotation			{ return S_ROTATION; }
<INITIAL>scale				{ return S_SCALE; }
<INITIAL>scaleOrientation	{ return S_SCALEORIENTATION; }
<INITIAL>shininess			{ return S_SHININESS; }
<INITIAL>side				{ return S_SIDE; }
<INITIAL>size				{ return S_SIZE; }
<INITIAL>skyAngle			{ return S_SKYANGLE; }
<INITIAL>skyColor			{ return S_SKYCOLOR; }
<INITIAL>solid				{ return S_SOLID;}
<INITIAL>source				{ return S_SOURCE; }
<INITIAL>spacing			{ return S_SPACING; }
<INITIAL>spatialize			{ return S_SPATIALIZE; }
<INITIAL>specularColor		{ return S_SPECULARCOLOR; }
<INITIAL>speed				{ return S_SPEED; }
<INITIAL>spine				{ return S_SPINE; }
<INITIAL>startTime			{ return S_STARTTIME; }
<INITIAL>stopTime			{ return S_STOPTIME; }
<INITIAL>string				{ return S_STRING; }
<INITIAL>style				{ return S_STYLE; }
<INITIAL>texCoord			{ return S_TEXCOORD; }
<INITIAL>texCoordIndex		{ return S_TEXCOORD_INDEX;}
<INITIAL>texture			{ return S_TEXTURE; }
<INITIAL>textureTransform	{ return S_TEXTURETRANSFORM; }
<INITIAL>title				{ return S_TITLE; }
<INITIAL>top				{ return S_TOP; }
<INITIAL>topToBottom		{ return S_TOP2BOTTOM; }
<INITIAL>topUrl				{ return S_TOPURL; }
<INITIAL>translation		{ return S_TRANSLATION; }
<INITIAL>transparency		{ return S_TRANSPARENCY; }
<INITIAL>type				{ return S_TYPE; }
<INITIAL>url				{ return S_URL; }
<INITIAL>vector				{ return S_VECTOR; }
<INITIAL>visibilityLimit	{ return S_VISIBILITY_LIMIT; }
<INITIAL>visibilityRange	{ return S_VISIBILITY_RANGE; }
<INITIAL>whichChild			{ return S_WHICHCHILD; }
<INITIAL>whichChoice		{ return S_WHICHCHOICE; }
<INITIAL>xDimension			{ return S_XDIMENSION; }
<INITIAL>xSpacing			{ return S_XSPACING; }
<INITIAL>zDimension			{ return S_ZDIMENSION; }
<INITIAL>zSpacing			{ return S_ZSPACING; }

<INITIAL>value_changed		{ return S_VALUE_CHANGED; }

<INITIAL>DEF	{
	int c, n;
	c = n = 0;
	do {
		c = input();
		buffer[n++] = c;
	} while (c != '{');
	buffer[n-1] = '\0';

	char	defName[256];
	char	nodeName[256];
	sscanf(buffer, "%s %s", defName, nodeName);

	sprintf(buffer, "%s {", nodeName);

	SetDEFName(defName);
	UnputString(buffer);
}

<INITIAL>USE	{
	int c, n;
	c = n = 0;

	do {
		c = input();
		if (c == '\n')
			nCurrentLine++;
	} while (c == '\t' || c == ' ');

	while (('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_' || c == '-' || c == '+') {
		buffer[n++] = c;
		c = input();
		if (c == '\n')
			nCurrentLine++;
	}
	buffer[n] = '\0';
	unput(c);

#ifdef UNUSE_DEF_NODE
	Node *defNode = GetParserObject()->findNodeByName(buffer);
	Node *useParentNode = GetCurrentNodeObject();
	if (defNode) {	
		if (useParentNode == NULL || (useParentNode != defNode && useParentNode->isAncestorNode(defNode) == false)) {
			Node *instanceNode = defNode->createInstanceNode();
			if (instanceNode)
				AddNode(instanceNode);
		}
	}
	return USE;
#else
	Node *node = GetParserObject()->findNodeByName(buffer);
	if (node) {
		Node *defNode = node->createDEFNode();
		AddNode(defNode);
	}
	return USE;
#endif
}

<INITIAL>PROTO	{
	int c, n;
	c = n = 0;
	do {
		c = input();
		buffer[n++] = c;
		if (c == '\n')
			nCurrentLine++;
	} while (c != '[');
	buffer[n-1] = '\0';

	char	protoName[256];
	sscanf(buffer, "%s", protoName);

	n = 0;
	do {
		c = input();
		buffer[n++] = c;
		if (c == '\n')
			nCurrentLine++;
	} while (c != ']');
	buffer[n-1] = '\0';

//	char *fieldString = strdup(buffer);

	char *fieldString = new char[strlen(buffer)+1];
	strcpy(fieldString, buffer);

	while (c != '{') {
		c = input();
		if (c == '\n')
			nCurrentLine++;
	}

	n = 0;
	do {
		c = input();
		buffer[n++] = c;
		if (c == '\n')
			nCurrentLine++;
	} while (c != '{');

	int inputFlag = 1;
	do {
		c = input();

		switch (c) {
		case '{'	: inputFlag++; break;
		case '}'	: inputFlag--; break;
		case '\n'	: nCurrentLine++; break;
		case '#'	: /* sval comment */
			{
				do {
					c = input();
				} while ((c != '\n') && (c != '\r'));
				buffer[n++] = '\n';
				nCurrentLine++;
			}
		}

		buffer[n++] = c;

	} while (inputFlag);
	buffer[n] = '\0';

	do {
		c = input();
		if (c == '\n')
			nCurrentLine++;
	} while (c != '}');

	PROTO *proto = AddPROTOInfo(protoName, buffer, fieldString);
//	proto->getString(buffer);
//	UnputString(buffer);

	delete[] fieldString;
//	free(fieldString);
}

<INITIAL>ROUTE {
	int c, n;
	c = n = 0;
	do {
		c = input();
		if (c == '\n')
			CurrentLineIncrement();
		else
			buffer[n++] = c;
	} while ((c != '\n') || (c != '\r'));
	buffer[n] = '\0';
	AddRouteInfo(buffer);
}

<INITIAL>[_a-zA-Z][_\+\-a-zA-Z0-9]* {

	PROTO *proto = IsPROTOName(yytext);

	if (proto) {
		int c, n;
		c = 0;
		do {
			c = input();
			if (c == '\n')
				nCurrentLine++;
		} while (c != '{');

		n = 0;
		do {
			c = input();
			buffer[n++] = c;
			if (c == '\n')
				nCurrentLine++;
		} while (c != '}');
		buffer[n-1] = '\0';

		proto->addFields(buffer);

		proto->getString(buffer);

		UnputString(buffer);
		proto->deleteFields();
	}
	else {
		strcpy(name, yytext);
		yylval.sval = name;	
		return	NAME;
	}
}

<SCRIPTNODE>directOutput	{ return S_DIRECT_OUTPUT; }
<SCRIPTNODE>mustEvaluate	{ return S_MUST_EVALUATE; }
<SCRIPTNODE>url				{ return S_URL; }

<SCRIPTNODE>field			{ return FIELD; }
<SCRIPTNODE>eventIn			{ return EVENTIN; }
<SCRIPTNODE>eventOut		{ return EVENTOUT; }

<SCRIPTNODE>SFBool			{ return SFBOOL; }
<SCRIPTNODE>SFFloat			{ return SFFLOAT; }
<SCRIPTNODE>SFInt32			{ return SFINT32; }
<SCRIPTNODE>SFTime			{ return SFTIME; }
<SCRIPTNODE>SFRotation		{ return SFROTATION; }
<SCRIPTNODE>SFNode			{ return SFNODE; }
<SCRIPTNODE>SFColor			{ return SFCOLOR; }
<SCRIPTNODE>SFString		{ return SFSTRING; }
<SCRIPTNODE>SFVec2f			{ return SFVEC2F; }
<SCRIPTNODE>SFVec3f			{ return SFVEC3F; }

<SCRIPTNODE>MFBool			{ return MFBOOL; }
<SCRIPTNODE>MFFloat			{ return MFFLOAT; }
<SCRIPTNODE>MFInt32			{ return MFINT32; }
<SCRIPTNODE>MFTime			{ return MFTIME; }
<SCRIPTNODE>MFRotation		{ return MFROTATION; }
<SCRIPTNODE>MFNode			{ return MFNODE; }
<SCRIPTNODE>MFColor			{ return MFCOLOR; }
<SCRIPTNODE>MFImage			{ return MFIMAGE; }
<SCRIPTNODE>MFString		{ return MFSTRING; }
<SCRIPTNODE>MFVec2f			{ return MFVEC2F; }
<SCRIPTNODE>MFVec3f			{ return MFVEC3F; }

<SCRIPTNODE>USE				{ return USE; }

<SCRIPTNODE>[_a-zA-Z][_\+\-a-zA-Z0-9]* {
//	char *name = strdup(yytext);
	char *name = new char[strlen(yytext)+1];
	strcpy(name, yytext);
	yylval.sval = name;	
	return	NAME;
}

%%

int yywrap()
{
  return 1;
} 

int yyerror(char *s) 
{ 
	Parser *parser = GetParserObject();
	parser->setErrorLineNumber(GetCurrentLineNumber());
	parser->setErrorToken(yytext);
	parser->setErrorLineString(GetErrorLineString());	
	return 0;
}

void CurrentLineIncrement()
{
	nCurrentLine++;
	if (callbackFn) 
		callbackFn(nCurrentLine, callbackFnInfo);
}

void MakeLexerBuffers(int lexBufferSize, int lineBufferSize)
{
	buffer = (char *)malloc(sizeof(char) * lexBufferSize);	
	lineBuffer = (char *)malloc(sizeof(char) * lineBufferSize);	
	lineBuffer[0] = '\0';
	yy_current_buffer = yy_create_buffer(yyin, lexBufferSize);
	name = (char *)malloc(sizeof(char) * lineBufferSize);	
}

void DeleteLexerBuffers(void)
{
	free(buffer);
	free(lineBuffer);
	free(name);
}

void SetLexCallbackFn(void (*func)(int nLine, void *info), void *fnInfo)
{
	callbackFn = func;
	callbackFnInfo = fnInfo;
}

int UnputString(char *pBegin)
{
	int nOutputLine = 0;

	if (!pBegin && !strlen(pBegin))
		return nOutputLine;

	char *pEnd = pBegin + strlen(pBegin);
	while(pEnd > pBegin) {
		unput(*--pEnd);
		if (*pEnd == '\n')
			nOutputLine++;
	}
	return nOutputLine;
}

void SetInputFile(FILE *fp)
{
  nCurrentLine = 1;
  yyrestart(fp);
}

int GetCurrentLineNumber(void)
{
  return nCurrentLine;
}

char *GetErrorLineString(void)
{
  return lineBuffer;
}


