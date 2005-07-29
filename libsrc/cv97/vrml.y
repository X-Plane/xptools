/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	vrml.y
*
******************************************************************/

%union {
int		ival;
float	fval;
char	*sval;
}

%token <ival> NUMBER
%token <fval> FLOAT
%token <sval> STRING NAME

%token ANCHOR APPEARANCE AUDIOCLIP BACKGROUND BILLBOARD BOX COLLISION COLOR
%token COLOR_INTERP COORDINATE COORDINATE_INTERP CYLINDER_SENSOR NULL_STRING
%token CONE CUBE CYLINDER DIRECTIONALLIGHT FONTSTYLE ERROR EXTRUSION
%token ELEVATION_GRID FOG INLINE MOVIE_TEXTURE NAVIGATION_INFO PIXEL_TEXTURE
%token GROUP INDEXEDFACESET INDEXEDLINESET S_INFO LOD MATERIAL NORMAL
%token POSITION_INTERP PROXIMITY_SENSOR SCALAR_INTERP SCRIPT SHAPE SOUND SPOTLIGHT
%token SPHERE_SENSOR TEXT TEXTURE_COORDINATE TEXTURE_TRANSFORM TIME_SENSOR SWITCH
%token TOUCH_SENSOR VIEWPOINT VISIBILITY_SENSOR WORLD_INFO NORMAL_INTERP ORIENTATION_INTERP
%token POINTLIGHT POINTSET SPHERE PLANE_SENSOR TRANSFORM

%token S_CHILDREN S_PARAMETER S_URL S_MATERIAL S_TEXTURETRANSFORM S_TEXTURE S_LOOP
%token S_STARTTIME S_STOPTIME S_GROUNDANGLE S_GROUNDCOLOR S_SPEED S_AVATAR_SIZE
%token S_BACKURL S_BOTTOMURL S_FRONTURL S_LEFTURL S_RIGHTURL S_TOPURL S_SKYANGLE S_SKYCOLOR 
%token S_AXIS_OF_ROTATION S_COLLIDE S_COLLIDETIME S_PROXY S_SIDE S_AUTO_OFFSET S_DISK_ANGLE
%token S_ENABLED S_MAX_ANGLE S_MIN_ANGLE S_OFFSET S_BBOXSIZE S_BBOXCENTER S_VISIBILITY_LIMIT
%token S_AMBIENT_INTENSITY S_NORMAL S_TEXCOORD S_CCW S_COLOR_PER_VERTEX S_CREASE_ANGLE
%token S_NORMAL_PER_VERTEX S_XDIMENSION S_XSPACING S_ZDIMENSION S_ZSPACING S_BEGIN_CAP
%token S_CROSS_SECTION S_END_CAP S_SPINE S_FOG_TYPE S_VISIBILITY_RANGE S_HORIZONTAL S_JUSTIFY 
%token S_LANGUAGE S_LEFT2RIGHT S_TOP2BOTTOM IMAGE_TEXTURE S_SOLID S_KEY S_KEYVALUE 
%token S_REPEAT_S S_REPEAT_T S_CONVEX S_BOTTOM S_PICTH S_COORD S_COLOR_INDEX S_COORD_INDEX S_NORMAL_INDEX
%token S_MAX_POSITION S_MIN_POSITION S_ATTENUATION S_APPEARANCE S_GEOMETRY S_DIRECT_OUTPUT
%token S_MUST_EVALUATE S_MAX_BACK S_MIN_BACK S_MAX_FRONT S_MIN_FRONT S_PRIORITY S_SOURCE S_SPATIALIZE
%token S_BERM_WIDTH S_CHOICE S_WHICHCHOICE S_FONTSTYLE S_LENGTH S_MAX_EXTENT S_ROTATION S_SCALE
%token S_CYCLE_INTERVAL S_FIELD_OF_VIEW S_JUMP S_TITLE S_TEXCOORD_INDEX S_HEADLIGHT
%token S_TOP S_BOTTOMRADIUS S_HEIGHT S_POINT S_STRING S_SPACING S_SCALE S_HEADLIGHT S_TYPE
%token S_RADIUS S_ON S_INTENSITY S_COLOR S_DIRECTION S_SIZE S_FAMILY S_STYLE S_RANGE
%token S_CENTER S_TRANSLATION S_LEVEL S_DIFFUSECOLOR S_SPECULARCOLOR S_EMISSIVECOLOR S_SHININESS
%token S_TRANSPARENCY S_VECTOR S_POSITION S_ORIENTATION S_LOCATION S_ROTATION 
%token S_CUTOFFANGLE S_WHICHCHILD S_IMAGE S_SCALEORIENTATION S_DESCRIPTION  
 
%token SFBOOL SFFLOAT SFINT32 SFTIME SFROTATION SFNODE SFCOLOR SFIMAGE SFSTRING SFVEC2F SFVEC3F
%token MFBOOL MFFLOAT MFINT32 MFTIME MFROTATION MFNODE MFCOLOR MFIMAGE MFSTRING MFVEC2F MFVEC3F
%token FIELD EVENTIN EVENTOUT USE

%token S_VALUE_CHANGED

%type <fval> SFFloat SFTime
%type <ival> SFBool SFInt32
%type <sval> SFString 

%start Vrml

%{

#include <stdio.h>
#include <stdlib.h>

#ifndef __GNUC__
#define alloca	malloc
#endif

#include "SceneGraph.h"
#include "VRMLNodeType.h"
#include "VRMLSetInfo.h"

float			gColor[3];
float			gVec2f[2];
float			gVec3f[3];
float			gRotation[4];
int				gWidth;
int				gHeight;
int				gComponents;

#define	YYMAXDEPTH	(1024 * 8 * 1000)

int yyerror(char *s);
int yyparse(void);
int yylex(void);

%} 

%%

Vrml
	: VrmlNodes
	| error		{YYABORT;}
	| ERROR		{YYABORT;}
	;

VrmlNodes
	: SFNode VrmlNodes
	|
	;

GroupingNode
	: Anchor
	| Billboard
	| Collision
	| Group
	| Inline
	| Lod
	| Switch
	| Transform
	;

InterpolatorNode
	: ColorInterp
	| CoordinateInterp
	| NormalInterp
	| OrientationInterp
	| PositionInterp
	| ScalarInterp
	;

SensorNode
	: CylinderSensor
	| PlaneSensor
	| SphereSensor
	| ProximitySensor
	| TimeSensor
	| TouchSensor
	| VisibilitySensor
	;

GeometryNode
	: Box
	| Cone
	| Cylinder
	| ElevationGrid
	| Extrusion
	| IdxFaceset
	| IdxLineset
	| Pointset
	| Sphere
	| Text
	;

LightNode
	: DirLight
	| SpotLight
	| PointLight
	;

CommonNode
	: AudioClip
	| LightNode
	| Script
	| Shape
	| Sound
	| WorldInfo
	;

BindableNode
	: Background
	| Fog
	| NavigationInfo
	| Viewpoint
	;

SFNode
	: CommonNode
	| BindableNode
	| FontStyle
	| InterpolatorNode
	| SensorNode
	| GroupingNode
	| USE
	;

SFInt32
	: NUMBER
		{
			AddSFInt32($1);
		}
	;

SFBool
	: NUMBER
	;

SFString
	: STRING
		{
			AddSFString($1);
		}
	;

SFFloat
	: FLOAT
		{
			AddSFFloat($1);
		}
	| NUMBER
		{
			$$ = (float)$1;
			AddSFFloat((float)$1);
		}
	;

SFTime
	: FLOAT
	| NUMBER {$$ = (float)$1;}
	;

SFColor
	: SFFloat SFFloat SFFloat 
	    {
			gColor[0] = $1;
			gColor[1] = $2;
			gColor[2] = $3;
			AddSFColor(gColor);
	    }
	;

SFRotation
	: SFFloat SFFloat SFFloat SFFloat 
	    {
			gRotation[0] = $1;
			gRotation[1] = $2;
			gRotation[2] = $3;
			gRotation[3] = $4;
			AddSFRotation(gRotation);
		}
	;

SFImageList
	: SFInt32 SFImageList {}
	|
	;


SFImageHeader
	: NUMBER NUMBER NUMBER
	    {
			gWidth = $1;
			gHeight = $2;
			gComponents = $3;
	    }
	;

SFImage
	: '[' SFImageHeader SFImageList ']'
	;

SFVec2f
	: SFFloat SFFloat 
	    {
			gVec2f[0] = $1;
			gVec2f[1] = $2;
			AddSFVec2f(gVec2f);
		}
	;

SFVec3f
	: SFFloat SFFloat SFFloat
		{
			gVec3f[0] = $1;
			gVec3f[1] = $2;
			gVec3f[2] = $3;
			AddSFVec3f(gVec3f);
		}
	;

SFColorList
	: SFColor
	| SFColor ',' SFColorList
	| SFColor SFColorList
	| ','
	|
	;

MFColor
	: SFColor 
	| '[' SFColorList ']'
	;

SFInt32List
	: SFInt32 {}
	| SFInt32 ',' SFInt32List {}
	| SFInt32 SFInt32List {}
	| ','
	|
	;

MFInt32
	: SFInt32 {}
	| '[' SFInt32List ']' {}
	; 


SFFloatList
	: SFFloat {}
	| SFFloat ',' SFFloatList {}
	| SFFloat SFFloatList {}
	| ','
	|
	;

MFFloat
	: SFFloat {}
	| '[' SFFloatList ']' {}
	; 

SFStringList
	: SFString {}
	| SFString ',' SFStringList {}
	| SFString SFStringList {}
	| ','
	|
	;

MFString
	: SFString {}
	| '[' SFStringList ']' {}
	; 

SFVec2fList
	: SFVec2f
	| SFVec2f ',' SFVec2fList
	| SFVec2f SFVec2fList
	| ','
	|
	;

MFVec2f
	: SFVec2f
	| '[' SFVec2fList ']'
	; 

SFVec3fList
	: SFVec3f
	| SFVec3f ',' SFVec3fList
	| SFVec3f SFVec3fList
	| ','
	|
	;

MFVec3f
	: SFVec3f
	| '[' SFVec3fList ']'
	; 

SFRotationList
	: SFRotation
	| SFRotation ',' SFRotationList
	| SFRotation SFRotationList
	| ','
	|
	;

MFRotation
	: SFRotation
	| '[' SFRotationList ']'
	; 

NodeBegin
	: '{'
	;

NodeEnd
	: '}'
	| '}' ','
	;

/******************************************************************
*
*	Anchor
*
******************************************************************/

AnchorElements	
	: AnchorElement AnchorElements
	|
	;

AnchorElementParameterBegin 
	: S_PARAMETER
		{
			PushNode(VRML_NODETYPE_ANCHOR_PARAMETER, GetCurrentNodeObject());
		}
	;

AnchorElementURLBegin 
	: S_URL	
		{
			PushNode(VRML_NODETYPE_ANCHOR_URL, GetCurrentNodeObject());
		}
	;

bboxCenter
	: S_BBOXCENTER	SFVec3f
		{
			((GroupingNode *)GetCurrentNodeObject())->setBoundingBoxCenter(gVec3f);
		}
	;

bboxSize
	: S_BBOXSIZE	SFVec3f
		{
			((GroupingNode *)GetCurrentNodeObject())->setBoundingBoxSize(gVec3f);
		}
	;

AnchorElement 
	: children
	| S_DESCRIPTION	SFString
		{
			((AnchorNode *)GetCurrentNodeObject())->setDescription($2);
		}

	| AnchorElementParameterBegin MFString 
		{
			PopNode();
		}
	| AnchorElementURLBegin MFString
		{
			PopNode();
		}
	| bboxCenter
	| bboxSize
	;

AnchorBegin
	: ANCHOR 
		{   
			AnchorNode	*anchor = new AnchorNode();
			anchor->setName(GetDEFName());
			AddNode(anchor);
			PushNode(VRML_NODETYPE_ANCHOR, anchor);
		}	
	;

Anchor
	: AnchorBegin NodeBegin AnchorElements NodeEnd
		{
			AnchorNode *anchor = (AnchorNode *)GetCurrentNodeObject();
			anchor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Appearance
*
******************************************************************/

AppearanceNodes
	: AppearanceNode AppearanceNodes
	|
	;

AppearanceNode
	: S_MATERIAL NULL_STRING
	| S_MATERIAL Material
	| S_MATERIAL USE
	| S_TEXTURE NULL_STRING
	| S_TEXTURE ImageTexture
	| S_TEXTURE MovieTexture
	| S_TEXTURE PixelTexture
	| S_TEXTURE USE
	| S_TEXTURETRANSFORM NULL_STRING
	| S_TEXTURETRANSFORM TexTransform
	| S_TEXTURETRANSFORM USE
	;
	
AppearanceBegin
	: APPEARANCE  
		{
			AppearanceNode	*appearance = new AppearanceNode();
			appearance->setName(GetDEFName());
			AddNode(appearance);
			PushNode(VRML_NODETYPE_APPEARANCE, appearance);
		}
		;

Appearance
	:  AppearanceBegin NodeBegin AppearanceNodes NodeEnd
		{
			AppearanceNode	*appearance = (AppearanceNode *)GetCurrentNodeObject();
			appearance->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Audio Clip
*
******************************************************************/

AudioClipElements
	: AudioClipElement AudioClipElements
	|
	;

AudioClipURL
	: S_URL
		{
			PushNode(VRML_NODETYPE_AUDIOCLIP_URL, GetCurrentNodeObject());
		}
	;

AudioClipElement
	: S_DESCRIPTION			SFString
		{
			((AudioClipNode *)GetCurrentNodeObject())->setDescription($2);
		}
	| S_LOOP					SFBool
		{
			((AudioClipNode *)GetCurrentNodeObject())->setLoop($2);
		}
	| S_PICTH					SFFloat
		{
			((AudioClipNode *)GetCurrentNodeObject())->setPitch($2);
		}
	| S_STARTTIME				SFTime
		{
			((AudioClipNode *)GetCurrentNodeObject())->setStartTime($2);
		}
	| S_STOPTIME				SFTime
		{
			((AudioClipNode *)GetCurrentNodeObject())->setStopTime($2);
		}
	| AudioClipURL	MFString
		{
			PopNode();
		}
	;

AudioClipBegin
	: AUDIOCLIP 
		{
			AudioClipNode	*audioClip = new AudioClipNode();
			audioClip->setName(GetDEFName());
			AddNode(audioClip);
			PushNode(VRML_NODETYPE_AUDIOCLIP, audioClip);
		}

AudioClip
	: AudioClipBegin NodeBegin AudioClipElements NodeEnd
		{
			AudioClipNode *audioClip = (AudioClipNode *)GetCurrentNodeObject();
			audioClip->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Background
*
******************************************************************/

BackGroundElements
	: BackGroundElement BackGroundElements
	|
	;

BackGroundBackURL
	: S_BACKURL
		{
			PushNode(VRML_NODETYPE_BACKGROUND_BACKURL, GetCurrentNodeObject());
		}
	;

BackGroundBottomURL
	: S_BOTTOMURL
		{
			PushNode(VRML_NODETYPE_BACKGROUND_BOTTOMURL, GetCurrentNodeObject());
		}
	;

BackGroundFrontURL
	: S_FRONTURL
		{
			PushNode(VRML_NODETYPE_BACKGROUND_FRONTURL, GetCurrentNodeObject());
		}
	;

BackGroundLeftURL
	: S_LEFTURL	
		{
			PushNode(VRML_NODETYPE_BACKGROUND_LEFTURL, GetCurrentNodeObject());
		}
	;

BackGroundRightURL
	: S_RIGHTURL
		{
			PushNode(VRML_NODETYPE_BACKGROUND_RIGHTURL, GetCurrentNodeObject());
		}
	;

BackGroundTopURL
	: S_TOPURL
		{
			PushNode(VRML_NODETYPE_BACKGROUND_TOPURL, GetCurrentNodeObject());
		}
	;

BackGroundGroundAngle
	: S_GROUNDANGLE
		{
			PushNode(VRML_NODETYPE_BACKGROUND_GROUNDANGLE, GetCurrentNodeObject());
		}
	;

BackGroundGroundColor
	: S_GROUNDCOLOR
		{
			PushNode(VRML_NODETYPE_BACKGROUND_GROUNDCOLOR, GetCurrentNodeObject());
		}
	;

BackGroundSkyAngle
	: S_SKYANGLE
		{
			PushNode(VRML_NODETYPE_BACKGROUND_SKYANGLE, GetCurrentNodeObject());
		}
	;

BackGroundSkyColor
	: S_SKYCOLOR
		{
			PushNode(VRML_NODETYPE_BACKGROUND_SKYCOLOR, GetCurrentNodeObject());
		}
	;

BackGroundElement
	: BackGroundGroundAngle	MFFloat
		{
			PopNode();
		}
	| BackGroundGroundColor	MFColor
		{
			PopNode();
		}
	| BackGroundBackURL	MFString
		{
			PopNode();
		}
	| BackGroundBottomURL	MFString
		{
			PopNode();
		}
	| BackGroundFrontURL	MFString
		{
			PopNode();
		}
	| BackGroundLeftURL	MFString
		{
			PopNode();
		}
	| BackGroundRightURL	MFString
		{
			PopNode();
		}
	| BackGroundTopURL		MFString
		{
			PopNode();
		}
	| BackGroundSkyAngle	MFFloat
		{
			PopNode();
		}
	| BackGroundSkyColor	MFColor
		{
			PopNode();
		}
	;

BackgroundBegin
	: BACKGROUND 
		{
			BackgroundNode *bg = new BackgroundNode();
			bg->setName(GetDEFName());
			AddNode(bg);
			PushNode(VRML_NODETYPE_BACKGROUND, bg);
		}
	;

Background
	: BackgroundBegin NodeBegin BackGroundElements NodeEnd
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Billboard
*
******************************************************************/

BillboardElements
	: BillboardElement BillboardElements
	|
	;

BillboardElement
	: children
	| S_AXIS_OF_ROTATION	SFVec3f
		{
			((BillboardNode *)GetCurrentNodeObject())->setAxisOfRotation(gVec3f);
		}
	| bboxCenter
	| bboxSize
	;

BillboardBegin
	: BILLBOARD 
		{   
			BillboardNode *billboard = new BillboardNode();
			billboard->setName(GetDEFName());
			AddNode(billboard);
			PushNode(VRML_NODETYPE_BILLBOARD, billboard);
		}	
	;

Billboard
	: BillboardBegin NodeBegin BillboardElements NodeEnd
		{
			BillboardNode *billboard = (BillboardNode *)GetCurrentNodeObject();
			billboard->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Box
*
******************************************************************/

BoxElements
	: BoxElement BoxElements
	|
	;

BoxElement
	: S_SIZE SFVec3f
		{
			((BoxNode *)GetCurrentNodeObject())->setSize(gVec3f);
		}
	;

BoxBegin
	: BOX 
		{
			BoxNode *box = new BoxNode();
			box->setName(GetDEFName());
			AddNode(box);
			PushNode(VRML_NODETYPE_BOX, box);
		}
	;

Box					
	: BoxBegin NodeBegin BoxElements NodeEnd
		{
			BoxNode *box = (BoxNode *)GetCurrentNodeObject();
			box->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Children
*
******************************************************************/

childrenElements
	: SFNode childrenElements
	|
	;

children
	: S_CHILDREN '[' childrenElements ']'
	| S_CHILDREN SFNode
	;

/******************************************************************
*
*	Collision
*
******************************************************************/

CollisionElements
	: CollisionElement CollisionElements
	|
	;

CollisionElementProxyBegin
	: S_PROXY
		{
			PushNode(VRML_NODETYPE_COLLISION_PROXY, GetCurrentNodeObject());
		}
	;

CollisionElement
	: children
	| S_COLLIDE						SFBool
		{
			((CollisionNode *)GetCurrentNodeObject())->setCollide($2);
		}
	| bboxCenter
	| bboxSize
	| S_PROXY USE
	| S_PROXY NULL_STRING
	| CollisionElementProxyBegin	SFNode
		{
			PopNode();							
		}
	;

CollisionBegin
	: COLLISION 
		{   
			CollisionNode *collision = new CollisionNode();
			collision->setName(GetDEFName());
			AddNode(collision);
			PushNode(VRML_NODETYPE_BOX, collision);
		}	
	;

Collision
	: CollisionBegin NodeBegin CollisionElements NodeEnd
		{
			CollisionNode *collision = (CollisionNode *)GetCurrentNodeObject();
			collision->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Color
*
******************************************************************/

ColorElements
	: ColorElement ColorElements
	|
	;

ColorElement
	: S_COLOR MFColor 				
	;

ColorBegin
	: COLOR  
		{
			ColorNode *color = new ColorNode();
			color->setName(GetDEFName());
			AddNode(color);
			PushNode(VRML_NODETYPE_COLOR, color);
		}
	;

Color
	: ColorBegin NodeBegin ColorElements NodeEnd
		{
			ColorNode *color = (ColorNode *)GetCurrentNodeObject();
			color->initialize();
			PopNode();
		}
	;
		                                                                                                                                                                                                                                                                                          
/******************************************************************
*
*	ColorInterpolator
*
******************************************************************/

ColorInterpElements
	: ColorInterpElement ColorInterpElements
	|
	;

InterpolateKey
	: S_KEY
		{
			PushNode(VRML_NODETYPE_INTERPOLATOR_KEY, GetCurrentNodeObject());
		}
	;

InterporlateKeyValue
	: S_KEYVALUE
		{
			PushNode(VRML_NODETYPE_INTERPOLATOR_KEYVALUE, GetCurrentNodeObject());
		}
	;

ColorInterpElement
	: InterpolateKey		MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue	MFColor
		{
			PopNode();
		}
	;

ColorInterpBegin
	: COLOR_INTERP  
		{
			ColorInterpolatorNode *colInterp = new ColorInterpolatorNode();
			colInterp->setName(GetDEFName());
			AddNode(colInterp);
			PushNode(VRML_NODETYPE_COLORINTERPOLATOR, colInterp);
		}
	;

ColorInterp
	: ColorInterpBegin NodeBegin ColorInterpElements NodeEnd
		{
			ColorInterpolatorNode *colInterp = (ColorInterpolatorNode *)GetCurrentNodeObject();
			colInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Cone
*
******************************************************************/

ConeElements
	: ConeElement ConeElements
	|
	;

ConeElement
	: S_SIDE			SFBool
		{
			((ConeNode *)GetCurrentNodeObject())->setSide($2);
		}
	| S_BOTTOM		SFBool
		{
			((ConeNode *)GetCurrentNodeObject())->setBottom($2);
		}
	| S_BOTTOMRADIUS	SFFloat
		{
			((ConeNode *)GetCurrentNodeObject())->setBottomRadius($2);
		}
	| S_HEIGHT		SFFloat
		{
			((ConeNode *)GetCurrentNodeObject())->setHeight($2);
		}
	;

ConeBegin
	: CONE 
		{
			ConeNode *cone = new ConeNode();
			cone->setName(GetDEFName());
			AddNode(cone);
			PushNode(VRML_NODETYPE_CONE, cone);
		}
	;

Cone
	: ConeBegin NodeBegin ConeElements NodeEnd
		{
			ConeNode *cone = (ConeNode *)GetCurrentNodeObject();
			cone->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Coordinate
*
******************************************************************/

CoordinateElements
	:  S_POINT	MFVec3f
	|
	;

CoordinateBegin
	: COORDINATE 
		{
			CoordinateNode *coord = new CoordinateNode();
			coord->setName(GetDEFName());
			AddNode(coord);
			PushNode(VRML_NODETYPE_COORDINATE, coord);
		}
	;

Coordinate
	: CoordinateBegin NodeBegin CoordinateElements NodeEnd
		{
			CoordinateNode *coord = (CoordinateNode *)GetCurrentNodeObject();
			coord->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	ColorInterpolator
*
******************************************************************/

CoordinateInterpElements
	: CoordinateInterpElement CoordinateInterpElements
	|
	;

CoordinateInterpElement
	: InterpolateKey		MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue	MFVec3f
		{
			PopNode();
		}
	;

CoordinateInterpBegin
	: COORDINATE_INTERP  
		{
			CoordinateInterpolatorNode *coordInterp = new CoordinateInterpolatorNode();
			coordInterp->setName(GetDEFName());
			AddNode(coordInterp);
			PushNode(VRML_NODETYPE_COORDINATEINTERPOLATOR, coordInterp);
		}
	;

CoordinateInterp
	: CoordinateInterpBegin NodeBegin CoordinateInterpElements NodeEnd
		{
			CoordinateInterpolatorNode *coordInterp = (CoordinateInterpolatorNode *)GetCurrentNodeObject();
			coordInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Cylinder
*
******************************************************************/

CylinderElements		
	: CylinderElement CylinderElements
	|
	;

CylinderElement
	: S_SIDE		SFBool
		{
			((CylinderNode *)GetCurrentNodeObject())->setSide($2);
		}
	| S_BOTTOM		SFBool
		{
			((CylinderNode *)GetCurrentNodeObject())->setBottom($2);
		}
	| S_TOP		SFBool
		{
			((CylinderNode *)GetCurrentNodeObject())->setTop($2);
		}
	| S_RADIUS		SFFloat
		{
			((CylinderNode *)GetCurrentNodeObject())->setRadius($2);
		}
	| S_HEIGHT		SFFloat
		{
			((CylinderNode *)GetCurrentNodeObject())->setHeight($2);
		}
	;

CylinderBegin
	: CYLINDER  
		{
			CylinderNode *cylinder = new CylinderNode();
			cylinder->setName(GetDEFName());
			AddNode(cylinder);
			PushNode(VRML_NODETYPE_CYLINDER, cylinder);
		}
	;

Cylinder
	: CylinderBegin NodeBegin CylinderElements NodeEnd
		{
			CylinderNode *cylinder = (CylinderNode *)GetCurrentNodeObject();
			cylinder->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   CylinderSensor
*
******************************************************************/

CylinderSensorElements
	: CylinderSensorElement CylinderSensorElements
	|
	;

CylinderSensorElement
	: S_AUTO_OFFSET			SFBool
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setAutoOffset($2);
		}
	| S_DISK_ANGLE			SFFloat
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setDiskAngle($2);
		}
	| S_ENABLED				SFBool
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	| S_MAX_ANGLE				SFFloat
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setMaxAngle($2);
		}
	| S_MIN_ANGLE				SFFloat
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setMinAngle($2);
		}
	| S_OFFSET				SFFloat
		{
			((CylinderSensorNode *)GetCurrentNodeObject())->setOffset($2);
		}
	;


CylinderSensorBegin
	: CYLINDER_SENSOR 
		{
			CylinderSensorNode *cysensor = new CylinderSensorNode();
			cysensor->setName(GetDEFName());
			AddNode(cysensor);
			PushNode(VRML_NODETYPE_CYLINDERSENSOR, cysensor);
		}
	;

CylinderSensor
	: CylinderSensorBegin NodeBegin CylinderSensorElements NodeEnd
		{
			CylinderSensorNode *cysensor = (CylinderSensorNode *)GetCurrentNodeObject();
			cysensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Directional Light
*
******************************************************************/

DirLightElements		
	: DirLightElement DirLightElements
	|
	;

DirLightElement
	: S_ON				SFBool
		{
			((DirectionalLightNode *)GetCurrentNodeObject())->setOn($2);
		}
	| S_INTENSITY			SFFloat
		{
			((DirectionalLightNode *)GetCurrentNodeObject())->setIntensity($2);
		}
	| S_COLOR		SFColor
		{
			((DirectionalLightNode *)GetCurrentNodeObject())->setColor(gColor);
		}
	| S_DIRECTION			SFVec3f
		{
			((DirectionalLightNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		}
	| S_AMBIENT_INTENSITY	SFFloat
		{
			((DirectionalLightNode *)GetCurrentNodeObject())->setAmbientIntensity($2);
		}
	;

DirLightBegin			
	: DIRECTIONALLIGHT 
		{
			DirectionalLightNode *dirLight = new DirectionalLightNode();
			dirLight->setName(GetDEFName());
			AddNode(dirLight);
			PushNode(VRML_NODETYPE_DIRECTIONALLIGHT, dirLight);
		}
	;

DirLight
	: DirLightBegin NodeBegin DirLightElements NodeEnd
		{
			DirectionalLightNode *dirLight = (DirectionalLightNode *)GetCurrentNodeObject();
			dirLight->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   ElevationGrid
*
******************************************************************/

ElevationGridElements
	: ElevationGridElement ElevationGridElements
	|
	;

ElevationGridHeight
	: S_HEIGHT
		{
			PushNode(VRML_NODETYPE_ELEVATIONGRID_HEIGHT, GetCurrentNodeObject());
		}
	;


ElevationGridElement
	: S_COLOR					NULL_STRING
	| S_COLOR					Color
	| S_COLOR					USE
	| S_NORMAL					NULL_STRING
	| S_NORMAL					Normal
	| S_NORMAL					USE
	| S_TEXCOORD				NULL_STRING
	| S_TEXCOORD				TexCoordinate
	| S_TEXCOORD				USE
	| ElevationGridHeight		MFFloat
		{
			PopNode();
		}
	| S_CCW 				SFBool
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setCCW($2);
		}
	| S_CREASE_ANGLE		SFFloat
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setCreaseAngle($2);
		}
	| S_SOLID				SFBool
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setSolid($2);
		}
	| S_COLOR_PER_VERTEX	SFBool
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setColorPerVertex($2);
		}
	| S_NORMAL_PER_VERTEX	SFBool
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setNormalPerVertex($2);
		}
	| S_XDIMENSION		SFInt32
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setXDimension($2);
		}
	| S_XSPACING			SFFloat
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setXSpacing($2);
		}
	| S_ZDIMENSION		SFInt32
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setZDimension($2);
		}
	| S_ZSPACING			SFFloat
		{
			((ElevationGridNode *)GetCurrentNodeObject())->setZSpacing($2);
		}
	;

ElevationGridBegin
	: ELEVATION_GRID 
		{
			ElevationGridNode *elev = new ElevationGridNode();
			elev->setName(GetDEFName());
			AddNode(elev);
			PushNode(VRML_NODETYPE_ELEVATIONGRID, elev);
		}
	;

ElevationGrid
	: ElevationGridBegin NodeBegin ElevationGridElements NodeEnd
		{
			ElevationGridNode *elev = (ElevationGridNode *)GetCurrentNodeObject();
			elev->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Extrusion
*
******************************************************************/

ExtrusionElements
	: ExtrusionElement ExtrusionElements
	|
	;

ExtrusionCrossSection
	: S_CROSS_SECTION
		{
			PushNode(VRML_NODETYPE_EXTRUSION_CROSSSECTION, GetCurrentNodeObject());
		}
	;

ExtrusionOrientation
	: S_ORIENTATION
		{
			PushNode(VRML_NODETYPE_EXTRUSION_ORIENTATION, GetCurrentNodeObject());
		}
	;

ExtrusionScale
	: S_SCALE
		{
			PushNode(VRML_NODETYPE_EXTRUSION_SCALE, GetCurrentNodeObject());
		}
	;

ExtrusionSpine
	: S_SPINE
		{
			PushNode(VRML_NODETYPE_EXTRUSION_SPINE, GetCurrentNodeObject());
		}
	;

ExtrusionElement
	: S_BEGIN_CAP			SFBool
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setBeginCap($2);
		}
	| S_CCW					SFBool 
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setCCW($2);
		}
	| S_CONVEX				SFBool
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setConvex($2);
		}
	| S_CREASE_ANGLE		SFFloat
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setCreaseAngle($2);
		}
	| S_SOLID				SFBool
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setSolid($2);
		}
	| ExtrusionCrossSection MFVec2f
		{
			PopNode();
		}
	| S_END_CAP			SFBool
		{
			((ExtrusionNode *)GetCurrentNodeObject())->setEndCap($2);
		}
	| ExtrusionOrientation	MFRotation
		{
			PopNode();
		}
	| ExtrusionScale MFVec2f
		{
			PopNode();
		}
	| ExtrusionSpine MFVec3f
		{
			PopNode();
		}
	;

ExtrusionBegin
	: EXTRUSION  
		{
			ExtrusionNode *ex = new ExtrusionNode();
			ex->setName(GetDEFName());
			AddNode(ex);
			PushNode(VRML_NODETYPE_EXTRUSION, ex);
		}
	;

Extrusion
	: ExtrusionBegin NodeBegin ExtrusionElements NodeEnd
		{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Fog
*
******************************************************************/

FogElements
	: FogElement FogElements
	|
	;

FogElement
	: S_COLOR		SFColor
		{
			((FogNode *)GetCurrentNodeObject())->setColor(gColor);
		}
	| S_FOG_TYPE			SFString
		{
			((FogNode *)GetCurrentNodeObject())->setFogType($2);
		}
	| S_VISIBILITY_RANGE			SFFloat
		{
			((FogNode *)GetCurrentNodeObject())->setVisibilityRange($2);
		}
	;

FogBegin
	: FOG  
		{
			FogNode *fog= new FogNode();
			fog->setName(GetDEFName());
			AddNode(fog);
			PushNode(VRML_NODETYPE_FOG, fog);
		}
	;

Fog
	: FogBegin NodeBegin FogElements NodeEnd
		{
			FogNode *fog= (FogNode *)GetCurrentNodeObject();
			fog->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Font Style 
*
******************************************************************/

FontStyleElements
	: FontStyleElement FontStyleElements
	|
	;

FontStyleJustify
	: S_JUSTIFY 
		{
			PushNode(VRML_NODETYPE_FONTSTYLE_JUSTIFY, GetCurrentNodeObject());
		}
	;

FontStyleElement
	: S_FAMILY		SFString
		{
			((FontStyleNode *)GetCurrentNodeObject())->setFamily($2);
		}
	| S_HORIZONTAL	SFBool
		{
			((FontStyleNode *)GetCurrentNodeObject())->setHorizontal($2);
		}
	| FontStyleJustify		MFString
		{
			PopNode();
		}
	| S_LANGUAGE	SFString
		{
			((FontStyleNode *)GetCurrentNodeObject())->setLanguage($2);
		}
	| S_LEFT2RIGHT	SFBool
		{
			((FontStyleNode *)GetCurrentNodeObject())->setLeftToRight($2);
		}
	| S_SIZE		SFFloat
		{
			((FontStyleNode *)GetCurrentNodeObject())->setSize($2);
		}
	| S_SPACING		SFFloat
		{
			((FontStyleNode *)GetCurrentNodeObject())->setSpacing($2);
		}
	| S_STYLE			SFString
		{
			((FontStyleNode *)GetCurrentNodeObject())->setStyle($2);
		}
	| S_TOP2BOTTOM	SFBool
		{
			((FontStyleNode *)GetCurrentNodeObject())->setTopToBottom($2);
		}
	;

FontStyleBegin	
	: FONTSTYLE NodeBegin
		{
			FontStyleNode *fs = new FontStyleNode();
			fs->setName(GetDEFName());
			AddNode(fs);
			PushNode(VRML_NODETYPE_FONTSTYLE, fs);
		}
	;

FontStyle		
	: FontStyleBegin FontStyleElements NodeEnd
		{
			FontStyleNode *fs = (FontStyleNode *)GetCurrentNodeObject();
			fs->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Group
*
******************************************************************/

GroupElements
	: GroupElement GroupElements
	|
	;

GroupElement
	: children
	| bboxCenter
	| bboxSize
	;

GroupBegin
	: GROUP 
		{   
			GroupNode *group = new GroupNode();
			group->setName(GetDEFName());
			AddNode(group);
			PushNode(VRML_NODETYPE_GROUP, group);
		}	
	;

Group
	: GroupBegin NodeBegin GroupElements NodeEnd
		{
			GroupNode *group = (GroupNode *)GetCurrentNodeObject();
			group->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   ImageTexture
*
******************************************************************/

ImgTexElements
	: ImgTexElement ImgTexElements
	|
	;

ImgTexURL
	: S_URL
		{
			PushNode(VRML_NODETYPE_IMAGETEXTURE_URL, GetCurrentNodeObject());
		}
	;

ImgTexElement
	: ImgTexURL	MFString
		{
			PopNode();
		}
	| S_REPEAT_S			SFBool
		{
			((ImageTextureNode *)GetCurrentNodeObject())->setRepeatS($2);
		}
	| S_REPEAT_T			SFBool
		{
			((ImageTextureNode *)GetCurrentNodeObject())->setRepeatT($2);
		}
	;

ImageTextureBegin
	: IMAGE_TEXTURE 
		{
			ImageTextureNode *imgTexture = new ImageTextureNode();
			imgTexture->setName(GetDEFName());
			AddNode(imgTexture);
			PushNode(VRML_NODETYPE_IMAGETEXTURE, imgTexture);
		}
	;

ImageTexture
	: ImageTextureBegin NodeBegin ImgTexElements NodeEnd
		{
			ImageTextureNode *imgTexture = (ImageTextureNode *)GetCurrentNodeObject();
			imgTexture->initialize();
			PopNode();
		} 
	;

/******************************************************************
*
*   Indexed Face set
*
******************************************************************/

IdxFacesetElements
	: IdxFacesetElement IdxFacesetElements
	|
	;

ColorIndex	
	: S_COLOR_INDEX
		{
			PushNode(VRML_NODETYPE_COLOR_INDEX, GetCurrentNodeObject());
		}
	;

CoordIndex	
	: S_COORD_INDEX
		{
			PushNode(VRML_NODETYPE_COORDINATE_INDEX, GetCurrentNodeObject());
		}
	;

NormalIndex
	: S_NORMAL_INDEX
		{
			PushNode(VRML_NODETYPE_NORMAL_INDEX, GetCurrentNodeObject());
		}
	;

TextureIndex
	: S_TEXCOORD_INDEX
	    {
			PushNode(VRML_NODETYPE_TEXTURECOODINATE_INDEX, GetCurrentNodeObject());
		}
	;

IdxFacesetElement
	: S_COLOR			NULL_STRING
	| S_COLOR			Color
	| S_COLOR			USE
	| S_COORD			NULL_STRING
	| S_COORD			Coordinate
	| S_COORD			USE
	| S_NORMAL			NULL_STRING
	| S_NORMAL			Normal
	| S_NORMAL			USE
	| S_TEXCOORD		NULL_STRING
	| S_TEXCOORD		TexCoordinate
	| S_TEXCOORD		USE
	| S_CCW				SFBool
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setCCW($2);
		}
	| S_CONVEX			SFBool
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setConvex($2);
		}
	| S_SOLID			SFBool
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setSolid($2);
		}
	| S_CREASE_ANGLE	SFFloat
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setCreaseAngle($2);
		}
	| ColorIndex	MFInt32
		{
			PopNode();
		}
	| S_COLOR_PER_VERTEX	SFBool
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setColorPerVertex($2);
		}
	| CoordIndex	MFInt32
		{
			PopNode();
		}
	| NormalIndex		MFInt32
		{
			PopNode();
		}
	| TextureIndex		MFInt32
		{
			PopNode();
		}
	| S_NORMAL_PER_VERTEX	SFBool
		{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setNormalPerVertex($2);
		}
	;

IdxFacesetBegin
	: INDEXEDFACESET  
		{
			IndexedFaceSetNode	*idxFaceset = new IndexedFaceSetNode();
			idxFaceset->setName(GetDEFName());
			AddNode(idxFaceset);
			PushNode(VRML_NODETYPE_INDEXEDFACESET, idxFaceset);
		}
	;

IdxFaceset
	: IdxFacesetBegin NodeBegin IdxFacesetElements NodeEnd
		{
			IndexedFaceSetNode *idxFaceset = (IndexedFaceSetNode *)GetCurrentNodeObject();
			idxFaceset->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Indexed Line set
*
******************************************************************/

IdxLinesetElements
	: IdxLinesetElement IdxLinesetElements
	|
	;

IdxLinesetElement
	: S_COLOR				NULL_STRING
	| S_COLOR				Color
	| S_COLOR				USE
	| S_COORD				NULL_STRING
	| S_COORD				Coordinate
	| S_COORD				USE
	| S_COLOR_PER_VERTEX	SFBool
		{
			((IndexedLineSetNode *)GetCurrentNodeObject())->setColorPerVertex($2);
		}
	| ColorIndex		MFInt32
		{
			PopNode();
		}
	| CoordIndex		MFInt32
		{
			PopNode();
		}
	;

IdxLinesetBegin	
	: INDEXEDLINESET NodeBegin 
		{
			IndexedLineSetNode	*idxLineset = new IndexedLineSetNode();
			idxLineset->setName(GetDEFName());
			AddNode(idxLineset);
			PushNode(VRML_NODETYPE_INDEXEDLINESET, idxLineset);
		}
	;

IdxLineset		
	: IdxLinesetBegin IdxLinesetElements NodeEnd
		{
			IndexedLineSetNode *idxLineset = (IndexedLineSetNode *)GetCurrentNodeObject();
			idxLineset->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Inline
*
******************************************************************/

InlineElements		
	: InlineElement InlineElements
	|
	;

InlineURL 
	: S_URL	
		{
			PushNode(VRML_NODETYPE_INLINE_URL, GetCurrentNodeObject());
		}
	;

InlineElement
	: InlineURL	MFString
		{
			PopNode();
		}
	| bboxCenter
	| bboxSize
	;

InlineBegin
	: INLINE
		{   
			InlineNode *inlineNode = new InlineNode();
			inlineNode->setName(GetDEFName());
			AddNode(inlineNode);
			PushNode(VRML_NODETYPE_INLINE, inlineNode);
		}	
	;

Inline
	: InlineBegin NodeBegin InlineElements NodeEnd
		{
			InlineNode *inlineNode = (InlineNode *)GetCurrentNodeObject();
			//inlineNode->initialize();
			PopNode();
		}
	;

/************************************************************
*
*   LOD
*
************************************************************/

LodElements		
	: LodElement LodElements
	|
	;

LodRange
	:  S_RANGE
		{
			PushNode(VRML_NODETYPE_LOD_RANGE, GetCurrentNodeObject());
		}
	;


LodLevel
	: S_LEVEL
		{
			PushNode(VRML_NODETYPE_LOD_LEVEL, GetCurrentNodeObject());
		}
	;

LodElement
	: LodRange	    MFFloat
		{
			PopNode();							
		}
	| S_CENTER			SFVec3f
		{
			((LodNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		}
	| LodLevel	SFNode
		{
			PopNode();							
		}
	| LodLevel		'[' VrmlNodes ']'
		{
			PopNode();							
		}
	;

LodBegin
	: LOD
		{   
			LodNode	*lod = new LodNode();
			lod->setName(GetDEFName());
			AddNode(lod);
			PushNode(VRML_NODETYPE_INLINE, lod);
		}	
	;

Lod				
	: LodBegin NodeBegin LodElements NodeEnd
		{
			LodNode	*lod = (LodNode *)GetCurrentNodeObject();
			lod->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Material
*
******************************************************************/

MaterialElements	
	: MaterialElement MaterialElements
	|
	;

MaterialElement	
	: S_AMBIENT_INTENSITY	SFFloat
		{
			((MaterialNode *)GetCurrentNodeObject())->setAmbientIntensity($2);
		}
	| S_DIFFUSECOLOR		SFColor
		{
			((MaterialNode *)GetCurrentNodeObject())->setDiffuseColor(gColor);
		}
	| S_EMISSIVECOLOR		SFColor
		{
			((MaterialNode *)GetCurrentNodeObject())->setEmissiveColor(gColor);
		}
	| S_SHININESS			SFFloat
		{
			((MaterialNode *)GetCurrentNodeObject())->setShininess($2);
		}
	| S_SPECULARCOLOR		SFColor
		{
			((MaterialNode *)GetCurrentNodeObject())->setSpecularColor(gColor);
		}
	| S_TRANSPARENCY		SFFloat
		{
			((MaterialNode *)GetCurrentNodeObject())->setTransparency($2);
		}

MaterialBegin	
	: MATERIAL 
		{
			MaterialNode *material = new MaterialNode();
			material->setName(GetDEFName());
			AddNode(material);
			PushNode(VRML_NODETYPE_MATERIAL, material);
		}
	;

Material
	: MaterialBegin NodeBegin MaterialElements NodeEnd
		{
			MaterialNode *material = (MaterialNode *)GetCurrentNodeObject();
			material->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	MovieTexture
*
******************************************************************/

MovieTextureElements
	: MovieTextureElement MovieTextureElements
	|
	;

MovieTextureURL
	: S_URL
		{
			PushNode(VRML_NODETYPE_MOVIETEXTURE_URL, GetCurrentNodeObject());
		}
	;

MovieTextureElement	
	: S_LOOP				SFBool
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setLoop($2);
		}
	| S_SPEED				SFFloat
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setSpeed($2);
		}
	| S_STARTTIME			SFTime
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setStartTime($2);
		}
	| S_STOPTIME			SFTime
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setStopTime($2);
		}
	| MovieTextureURL MFString
		{
			PopNode();
		}
	| S_REPEAT_S			SFBool
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setRepeatS($2);
		}
	| S_REPEAT_T			SFBool
		{
			((MovieTextureNode *)GetCurrentNodeObject())->setRepeatT($2);
		}
	;

MovieTextureBegin
	: MOVIE_TEXTURE  
		{
			MovieTextureNode *movieTexture = new MovieTextureNode();
			movieTexture->setName(GetDEFName());
			AddNode(movieTexture);
			PushNode(VRML_NODETYPE_MOVIETEXTURE, movieTexture);
		}
	;

MovieTexture
	: MovieTextureBegin NodeBegin MovieTextureElements NodeEnd
		{
			MovieTextureNode *movieTexture = (MovieTextureNode *)GetCurrentNodeObject();
			movieTexture->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Navigation Info
*
******************************************************************/

NavigationInfoElements
	: NavigationInfoElement NavigationInfoElements
	|
	;

NavigationInfoAvatarSize
	: S_AVATAR_SIZE
		{
			PushNode(VRML_NODETYPE_NAVIGATIONINFO_AVATARSIZE, GetCurrentNodeObject());
		}
	;

NavigationInfoType
	: S_TYPE
		{
			PushNode(VRML_NODETYPE_NAVIGATIONINFO_TYPE, GetCurrentNodeObject());
		}
	;

NavigationInfoElement
	: NavigationInfoAvatarSize	MFFloat
		{
			PopNode();
		}
	| S_HEADLIGHT						SFBool
		{
			((NavigationInfoNode *)GetCurrentNodeObject())->setHeadlight($2);
		}
	| S_SPEED							SFFloat
		{
			((NavigationInfoNode *)GetCurrentNodeObject())->setSpeed($2);
		}
	| NavigationInfoType		MFString
		{
			PopNode();
		}
	| S_VISIBILITY_LIMIT				SFFloat
		{
			((NavigationInfoNode *)GetCurrentNodeObject())->setVisibilityLimit($2);
		}
	;

NavigationInfoBegin
	: NAVIGATION_INFO
		{
			NavigationInfoNode *navInfo = new NavigationInfoNode();
			navInfo->setName(GetDEFName());
			AddNode(navInfo);
			PushNode(VRML_NODETYPE_NAVIGATIONINFO, navInfo);
		}
	;

NavigationInfo		
	: NavigationInfoBegin NodeBegin NavigationInfoElements NodeEnd
		{
			NavigationInfoNode *navInfo = (NavigationInfoNode *)GetCurrentNodeObject();
			navInfo->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Normal
*
******************************************************************/

NormalElements	
	: NormalElement NormalElements
	|
	;

NormalElement
	: S_VECTOR	MFVec3f
	;

NormalBegin
	: NORMAL  
		{
			NormalNode *normal = new NormalNode();
			normal->setName(GetDEFName());
			AddNode(normal);
			PushNode(VRML_NODETYPE_NORMAL, normal);
		}
	;

Normal
	: NormalBegin NodeBegin NormalElements NodeEnd
		{
			NormalNode *normal = (NormalNode *)GetCurrentNodeObject();
			normal->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Normal Interpolator
*
******************************************************************/

NormalInterpElements	
	: NormalInterpElement NormalInterpElements
	|
	;

NormalInterpElement
	: InterpolateKey			MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue		MFVec3f
		{
			PopNode();
		}
	| S_VALUE_CHANGED			SFVec3f
		{
		}
	;

NormalInterpBegin
	: NORMAL_INTERP
		{
			NormalInterpolatorNode *normInterp = new NormalInterpolatorNode();
			normInterp->setName(GetDEFName());
			AddNode(normInterp);
			PushNode(VRML_NODETYPE_NORMALINTERPOLATOR, normInterp);
		}
	;

NormalInterp
	: NormalInterpBegin NodeBegin	NormalInterpElements NodeEnd
		{
			NormalInterpolatorNode *normInterp = (NormalInterpolatorNode *)GetCurrentNodeObject();
			normInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Orientation Interpolator
*
******************************************************************/

OrientationInterpElements
	: OrientationInterpElement OrientationInterpElements
	|
	;

OrientationInterpElement
	: InterpolateKey			MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue		MFRotation
		{
			PopNode();
		}
	| S_VALUE_CHANGED			SFRotation
		{
		}
	;

OrientationInterpBegin
	: ORIENTATION_INTERP
		{
			OrientationInterpolatorNode *oriInterp = new OrientationInterpolatorNode();
			oriInterp->setName(GetDEFName());
			AddNode(oriInterp);
			PushNode(VRML_NODETYPE_ORIENTATIONINTERPOLATOR, oriInterp);
		}
	;

OrientationInterp
	: OrientationInterpBegin NodeBegin OrientationInterpElements NodeEnd
		{
			OrientationInterpolatorNode *oriInterp = (OrientationInterpolatorNode *)GetCurrentNodeObject();
			oriInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Pixel Texture
*
******************************************************************/

PixelTextureElements
	: PixelTextureElement PixelTextureElements
	|
	;

PixelTextureImage
	: S_IMAGE  
		{
			PushNode(VRML_NODETYPE_PIXELTEXTURE_IMAGE, GetCurrentNodeObject());
		}
	;

PixelTextureElement
	: PixelTextureImage	'[' SFImageList ']'
		{
			PopNode();
		}
	| S_REPEAT_S		SFBool
		{
			((PixelTextureNode *)GetCurrentNodeObject())->setRepeatS($2);
		}
	| S_REPEAT_T		SFBool
		{
			((PixelTextureNode *)GetCurrentNodeObject())->setRepeatT($2);
		}
	;

PixelTextureBegin
	: PIXEL_TEXTURE 
		{
			PixelTextureNode *pixTexture = new PixelTextureNode();
			pixTexture->setName(GetDEFName());
			AddNode(pixTexture);
			PushNode(VRML_NODETYPE_PIXELTEXTURE, pixTexture);
		}
	;

PixelTexture		
	: PixelTextureBegin NodeBegin PixelTextureElements NodeEnd
		{
			PixelTextureNode *pixTexture = (PixelTextureNode *)GetCurrentNodeObject();
			pixTexture->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Plane Sensor
*
******************************************************************/

PlaneSensorElements
	: PlaneSensorElement PlaneSensorElements
	|
	;

PlaneSensorElement
	: S_AUTO_OFFSET	SFBool
		{
			((PlaneSensorNode *)GetCurrentNodeObject())->setAutoOffset($2);
		}
	| S_ENABLED		SFBool
		{
			((PlaneSensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	| S_MAX_POSITION	SFVec2f
		{
			((PlaneSensorNode *)GetCurrentNodeObject())->setMaxPosition(gVec2f);
		}
	| S_MIN_POSITION	SFVec2f
		{
			((PlaneSensorNode *)GetCurrentNodeObject())->setMinPosition(gVec2f);
		}
	| S_OFFSET		SFVec3f
		{
			((PlaneSensorNode *)GetCurrentNodeObject())->setOffset(gVec3f);
		}
	;

PlaneSensorBegin
	: PLANE_SENSOR
		{
			PlaneSensorNode *psensor = new PlaneSensorNode();
			psensor->setName(GetDEFName());
			AddNode(psensor);
			PushNode(VRML_NODETYPE_PLANESENSOR, psensor);
		}
	;

PlaneSensor
	: PlaneSensorBegin NodeBegin PlaneSensorElements NodeEnd
		{
			PlaneSensorNode *psensor = (PlaneSensorNode *)GetCurrentNodeObject();
			psensor->initialize();
			PopNode();
		}
	;


/******************************************************************
*
*   Point Light
*
******************************************************************/

PointLightNodes
	: PointLightNode PointLightNodes
	|
	;

PointLightNode
	: S_AMBIENT_INTENSITY	SFFloat
		{
			((PointLightNode *)GetCurrentNodeObject())->setAmbientIntensity($2);
		}
	| S_ATTENUATION		SFVec3f
		{
			((PointLightNode *)GetCurrentNodeObject())->setAttenuation(gVec3f);
		}
	| S_COLOR		SFColor
		{
			((PointLightNode *)GetCurrentNodeObject())->setColor(gColor);
		}
	| S_INTENSITY	SFFloat
		{
			((PointLightNode *)GetCurrentNodeObject())->setIntensity($2);
		}
	| S_LOCATION	SFVec3f
		{
			((PointLightNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		}
	| S_ON		SFBool
		{
			((PointLightNode *)GetCurrentNodeObject())->setOn($2);
		}
	| S_RADIUS	SFFloat
		{
			((PointLightNode *)GetCurrentNodeObject())->setRadius($2);
		}
	;

PointLightBegin	
	: POINTLIGHT  
		{
			PointLightNode *pointLight = new PointLightNode();
			pointLight->setName(GetDEFName());
			AddNode(pointLight);
			PushNode(VRML_NODETYPE_POINTLIGHT, pointLight);
		}
	;

PointLight
	: PointLightBegin NodeBegin PointLightNodes NodeEnd
		{
			PointLightNode *pointLight = (PointLightNode *)GetCurrentNodeObject();
			pointLight->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Point set
*
******************************************************************/

PointsetElements		
	: PointsetElement PointsetElements
	|
	;

PointsetElement
	: S_COLOR	NULL_STRING
	| S_COLOR	Color
	| S_COLOR	USE
	| S_COORD	NULL_STRING
	| S_COORD	Coordinate
	| S_COORD	USE
	;


PointsetBegin
	: POINTSET
		{
			PointSetNode *pset = new PointSetNode();
			pset->setName(GetDEFName());
			AddNode(pset);
			PushNode(VRML_NODETYPE_POINTSET, pset);
		}
	;

Pointset	
	: PointsetBegin NodeBegin PointsetElements NodeEnd
		{
			PointSetNode *pset = (PointSetNode *)GetCurrentNodeObject();
			pset->initialize();
			PopNode();
		}

/******************************************************************
*
*	Position Interpolator
*
******************************************************************/

PositionInterpElements
	: PositionInterpElement PositionInterpElements
	|
	;

PositionInterpElement
	: InterpolateKey			MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue		MFVec3f
		{
			PopNode();
		}
	| S_VALUE_CHANGED			SFVec3f
		{
		}
	;

PositionInterpBegin
	: POSITION_INTERP
		{
			PositionInterpolatorNode *posInterp = new PositionInterpolatorNode();
			posInterp->setName(GetDEFName());
			AddNode(posInterp);
			PushNode(VRML_NODETYPE_POSITIONINTERPOLATOR, posInterp);
		}
	;

PositionInterp
	: PositionInterpBegin NodeBegin PositionInterpElements NodeEnd
		{
			PositionInterpolatorNode *posInterp = (PositionInterpolatorNode *)GetCurrentNodeObject();
			posInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Proximity Sensor
*
******************************************************************/

ProximitySensorElements
	: ProximitySensorElement ProximitySensorElements
	|
	;

ProximitySensorElement
	: S_CENTER		SFVec3f
		{
			((ProximitySensorNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		}
	| S_SIZE		SFVec3f
		{
			((ProximitySensorNode *)GetCurrentNodeObject())->setSize(gVec3f);
		}
	| S_ENABLED		SFBool
		{
			((ProximitySensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	;

ProximitySensorBegin
	: PROXIMITY_SENSOR
		{
			ProximitySensorNode *psensor = new ProximitySensorNode();
			psensor->setName(GetDEFName());
			AddNode(psensor);
			PushNode(VRML_NODETYPE_PROXIMITYSENSOR, psensor);
		}
	;

ProximitySensor		
	: ProximitySensorBegin NodeBegin ProximitySensorElements NodeEnd
		{
			ProximitySensorNode *psensor = (ProximitySensorNode *)GetCurrentNodeObject();
			psensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Scalar Interpolator
*
******************************************************************/

ScalarInterpElements	
	: ScalarInterpElement ScalarInterpElements
	|
	;

ScalarInterpElement
	: InterpolateKey			MFFloat
		{
			PopNode();
		}
	| InterporlateKeyValue		MFFloat
		{
			PopNode();
		}
	| S_VALUE_CHANGED			SFVec2f
		{
		}
	;

ScalarInterpBegin
	: SCALAR_INTERP
		{
			ScalarInterpolatorNode *scalarInterp = new ScalarInterpolatorNode();
			scalarInterp->setName(GetDEFName());
			AddNode(scalarInterp);
			PushNode(VRML_NODETYPE_SCALARINTERPOLATOR, scalarInterp);
		}
	;

ScalarInterp
	: ScalarInterpBegin NodeBegin ScalarInterpElements NodeEnd
		{
			ScalarInterpolatorNode *scalarInterp = (ScalarInterpolatorNode *)GetCurrentNodeObject();
			scalarInterp->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Script
*
******************************************************************/

ScriptElements
	: ScriptElement ScriptElements
	|
	;

ScriptURL
	: S_URL
		{
			PushNode(VRML_NODETYPE_SCRIPT_URL, GetCurrentNodeObject());
		}
	;

ScriptElement
	: ScriptURL	MFString
		{
			PopNode();
		}
	| S_DIRECT_OUTPUT		SFBool
		{
			((ScriptNode *)GetCurrentNodeObject())->setDirectOutput($2);
		}
	| S_MUST_EVALUATE		SFBool
		{
			((ScriptNode *)GetCurrentNodeObject())->setMustEvaluate($2);
		}

	/*********************************************************
	*	eventIn (SFNode)
	*********************************************************/
	
	| EVENTIN	SFBOOL		NAME
		{
			SFBool *value = new SFBool();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFFLOAT		NAME
		{
			SFFloat *value = new SFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFINT32		NAME
		{
			SFInt32 *value = new SFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFTIME		NAME
		{
			SFTime *value = new SFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFROTATION	NAME
		{
			SFRotation *value = new SFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
/* 
	| EVENTIN	SFNODE		NAME
		{
			Node *value = new Node();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
*/
	| EVENTIN	SFCOLOR		NAME
		{
			SFColor *value = new SFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFIMAGE		NAME
		{
			SFImage *value = new SFImage();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFSTRING	NAME
		{
			SFString *value = new SFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFVEC2F		NAME
		{
			SFVec2f *value = new SFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	SFVEC3F		NAME
		{
			SFVec3f *value = new SFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}

	/*********************************************************
	*	eventIn (MFNode)
	*********************************************************/
	
	| EVENTIN	MFFLOAT		NAME
		{
			MFFloat *value = new MFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFINT32		NAME
		{
			MFInt32 *value = new MFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFTIME		NAME
		{
			MFTime *value = new MFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFROTATION	NAME
		{
			MFRotation *value = new MFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
/* 
	| EVENTIN	MFNODE		NAME
		{
			Node *value = new Node();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
*/
	| EVENTIN	MFCOLOR		NAME
		{
			MFColor *value = new MFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFSTRING	NAME
		{
			MFString *value = new MFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFVEC2F		NAME
		{
			MFVec2f *value = new MFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}
	| EVENTIN	MFVEC3F		NAME
		{
			MFVec3f *value = new MFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn($3, value);
			delete[] $3;
		}

	/*********************************************************
	*	eventOut (SFNode)
	*********************************************************/
	
	| EVENTOUT	SFBOOL		NAME
		{
			SFBool *value = new SFBool();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFFLOAT		NAME
		{
			SFFloat *value = new SFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFINT32		NAME
		{
			SFInt32 *value = new SFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFTIME		NAME
		{
			SFTime *value = new SFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFROTATION	NAME
		{
			SFRotation *value = new SFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
/* 
	| EVENTOUT	SFNODE		NAME
		{
			Node *value = new Node();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
*/
	| EVENTOUT	SFCOLOR		NAME
		{
			SFColor *value = new SFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFIMAGE		NAME
		{
			SFImage *value = new SFImage();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFSTRING	NAME
		{
			SFString *value = new SFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFVEC2F		NAME
		{
			SFVec2f *value = new SFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	SFVEC3F		NAME
		{
			SFVec3f *value = new SFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}

	/*********************************************************
	*	eventOut (MFNode)
	*********************************************************/
	
	| EVENTOUT	MFFLOAT		NAME
		{
			MFFloat *value = new MFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFINT32		NAME
		{
			MFInt32 *value = new MFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFTIME		NAME
		{
			MFTime *value = new MFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFROTATION	NAME
		{
			MFRotation *value = new MFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
/* 
	| EVENTOUT	MFNODE		NAME
		{
			Node *value = new Node();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
*/
	| EVENTOUT	MFCOLOR		NAME
		{
			MFColor *value = new MFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFSTRING	NAME
		{
			MFString *value = new MFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFVEC2F		NAME
		{
			MFVec2f *value = new MFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}
	| EVENTOUT	MFVEC3F		NAME
		{
			MFVec3f *value = new MFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut($3, value);
			delete[] $3;
		}

	/*********************************************************
	*	field (SFNode)
	*********************************************************/
	
	| FIELD	SFBOOL		NAME	SFBool
		{
			SFBool *value = new SFBool($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFFLOAT		NAME	SFFloat
		{
			SFFloat *value = new SFFloat($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFINT32		NAME	SFInt32
		{
			SFInt32 *value = new SFInt32($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFTIME		NAME	SFTime
		{
			SFTime *value = new SFTime($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFROTATION	NAME	SFRotation
		{
			SFRotation *value = new SFRotation(gRotation);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
 
	| FIELD	SFNODE		NAME	NULL_STRING
		{
			SFNode *value = new SFNode();
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}

	| FIELD	SFNODE		NAME	USE		NAME
		{
			Node *node = GetParserObject()->findNodeByName($5);
			SFNode *value = new SFNode(node);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3; delete[] $5;
		}

	| FIELD	SFCOLOR		NAME	SFColor
		{
			SFColor *value = new SFColor(gColor);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
/*
	| FIELD	SFIMAGE		NAME	SFImage
		{
			SFImage *value = new SFImage($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
*/
	| FIELD	SFSTRING	NAME	SFString
		{
			SFString *value = new SFString($4);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFVEC2F		NAME	SFVec2f
		{
			SFVec2f *value = new SFVec2f(gVec2f);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}
	| FIELD	SFVEC3F		NAME	SFVec3f
		{
			SFVec3f *value = new SFVec3f(gVec3f);
			((ScriptNode *)GetCurrentNodeObject())->addField($3, value);
			delete[] $3;
		}

	;

ScriptBegin
	: SCRIPT
		{
			ScriptNode *script = new ScriptNode();
			script->setName(GetDEFName());
			AddNode(script);
			PushNode(VRML_NODETYPE_SCRIPT, script);
		}
	;

Script
	: ScriptBegin NodeBegin ScriptElements NodeEnd
		{
			ScriptNode *script = (ScriptNode *)GetCurrentNodeObject();
			script->initialize();
			PopNode();
		}
	;
		

/******************************************************************
*
*	Shape
*
******************************************************************/

SharpElements
	: SharpElement SharpElements
	|
	;

SharpElement
	: S_APPEARANCE		NULL_STRING
	| S_APPEARANCE		Appearance
	| S_APPEARANCE		USE
	| S_GEOMETRY		NULL_STRING
	| S_GEOMETRY		GeometryNode
	| S_GEOMETRY		USE
	;

ShapeBegin
	: SHAPE  
		{
			ShapeNode *shape = new ShapeNode();
			shape->setName(GetDEFName());
			AddNode(shape);
			PushNode(VRML_NODETYPE_SHAPE, shape);
		}
	;

Shape
	: ShapeBegin NodeBegin SharpElements NodeEnd
		{
			ShapeNode *shape = (ShapeNode *)GetCurrentNodeObject();
			shape->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Sound
*
******************************************************************/

SoundElements
	: SoundElement SoundElements
	|
	;

SoundElement
	: S_DIRECTION			SFVec3f
		{
			((SoundNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		}
	| S_INTENSITY			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setIntensity($2);
		}
	| S_LOCATION			SFVec3f
		{
			((SoundNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		}
	| S_MAX_BACK			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setMinBack($2);
		}
	| S_MAX_FRONT			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setMaxFront($2);
		}
	| S_MIN_BACK			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setMinBack($2);
		}
	| S_MIN_FRONT			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setMinFront($2);
		}
	| S_PRIORITY			SFFloat
		{
			((SoundNode *)GetCurrentNodeObject())->setPriority($2);
		}
	| S_SOURCE			NULL_STRING
	| S_SOURCE			AudioClip
	| S_SOURCE			MovieTexture
	| S_SOURCE			USE
	| S_SPATIALIZE		SFBool
		{
			((SoundNode *)GetCurrentNodeObject())->setSpatialize($2);
		}
	;

SoundBegin
	: SOUND
		{
			SoundNode *sound = new SoundNode();
			sound->setName(GetDEFName());
			AddNode(sound);
			PushNode(VRML_NODETYPE_SOUND, sound);
		}
	;

Sound
	: SoundBegin NodeBegin SoundElements NodeEnd
		{
			SoundNode *sound = (SoundNode *)GetCurrentNodeObject();
			sound->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Sphere
*
******************************************************************/

SphereElements
	: SphereElement SphereElements
	|
	;

SphereElement
	: S_RADIUS	SFFloat
		{
			((SphereNode *)GetCurrentNodeObject())->setRadius($2);
		}
	;

SphereBegin	
	: SPHERE  
		{
			SphereNode *sphere = new SphereNode();
			sphere->setName(GetDEFName());
			AddNode(sphere);
			PushNode(VRML_NODETYPE_SPHERE, sphere);
		}
	;

Sphere
	: SphereBegin NodeBegin SphereElements NodeEnd
		{
			SphereNode *sphere = (SphereNode *)GetCurrentNodeObject();
			sphere->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Spehere Sensor
*
******************************************************************/

SphereSensorElements
	: SphereSensorElement SphereSensorElements
	|
	;

SphereSensorElement
	: S_AUTO_OFFSET	SFBool
		{
			((SphereSensorNode *)GetCurrentNodeObject())->setAutoOffset($2);
		}
	| S_ENABLED		SFBool
		{
			((SphereSensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	| S_OFFSET		SFRotation
		{
			((SphereSensorNode *)GetCurrentNodeObject())->setOffset(gRotation);
		}
	;

SphereSensorBegin
	: SPHERE_SENSOR
		{
			SphereSensorNode *spsensor = new SphereSensorNode();
			spsensor->setName(GetDEFName());
			AddNode(spsensor);
			PushNode(VRML_NODETYPE_SPHERESENSOR, spsensor);
		}
	;

SphereSensor
	: SphereSensorBegin NodeBegin SphereSensorElements NodeEnd
		{
			SphereSensorNode *spsensor = (SphereSensorNode *)GetCurrentNodeObject();
			spsensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Spot Light
*
******************************************************************/

SpotLightElements	
	: SpotLightElement SpotLightElements
	|
	;

SpotLightElement	
	: S_AMBIENT_INTENSITY	SFFloat
		{
			((SpotLightNode *)GetCurrentNodeObject())->setAmbientIntensity($2);
		}
	| S_ATTENUATION		SFVec3f
		{
			((SpotLightNode *)GetCurrentNodeObject())->setAttenuation(gVec3f);
		}
	| S_BERM_WIDTH		SFFloat
		{
			((SpotLightNode *)GetCurrentNodeObject())->setBeamWidth($2);
		}
	| S_COLOR		SFColor
		{
			((SpotLightNode *)GetCurrentNodeObject())->setColor(gColor);
		}
	| S_CUTOFFANGLE		SFFloat
		{
			((SpotLightNode *)GetCurrentNodeObject())->setCutOffAngle($2);
		}
	| S_DIRECTION			SFVec3f
		{
			((SpotLightNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		}
	| S_INTENSITY			SFFloat
		{
			((SpotLightNode *)GetCurrentNodeObject())->setIntensity($2);
		}
	| S_LOCATION			SFVec3f
		{
			((SpotLightNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		}
	| S_ON				SFBool
		{
			((SpotLightNode *)GetCurrentNodeObject())->setOn($2);
		}
	| S_RADIUS			SFFloat
		{
			((SpotLightNode *)GetCurrentNodeObject())->setRadius($2);
		}
	;

SpotLightBegin
	: SPOTLIGHT 
		{
			SpotLightNode *spotLight = new SpotLightNode();
			spotLight->setName(GetDEFName());
			AddNode(spotLight);
			PushNode(VRML_NODETYPE_SPOTLIGHT, spotLight);
		}
	;

SpotLight		
	: SpotLightBegin NodeBegin SpotLightElements NodeEnd
		{
			SpotLightNode *spotLight = (SpotLightNode *)GetCurrentNodeObject();
			spotLight->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Switch
*
******************************************************************/

SwitchElements	
	: SwitchElement SwitchElements
	|
	;

SwitchChoice
	: S_CHOICE
		{
			PushNode(VRML_NODETYPE_SWITCH_CHOICE, GetCurrentNodeObject());
		}
	;

SwitchElement
	: SwitchChoice	SFNode
		{
			PopNode();							
		}
	| SwitchChoice '[' VrmlNodes ']'
		{
			PopNode();							
		}
	| S_WHICHCHOICE	SFInt32
		{
			((SwitchNode *)GetCurrentNodeObject())->setWhichChoice($2);
		}
	;


SwitchBegin
	: SWITCH
		{   
			SwitchNode *switchNode = new SwitchNode();
			switchNode->setName(GetDEFName());
			AddNode(switchNode);
			PushNode(VRML_NODETYPE_SWITCH, switchNode);
		}	
	;

Switch			
	: SwitchBegin NodeBegin SwitchElements NodeEnd
		{
			SwitchNode *switchNode = (SwitchNode *)GetCurrentNodeObject();
			switchNode->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   Text
*
******************************************************************/

TextElements
	: TextElement TextElements
	|
	;

TextString
	: S_STRING
		{
			PushNode(VRML_NODETYPE_TEXT_STRING, GetCurrentNodeObject());
		}
	;

TextLength
	: S_LENGTH
		{
			PushNode(VRML_NODETYPE_TEXT_LENGTH, GetCurrentNodeObject());
		}
	;

TextElement
	: TextString	MFString
		{
			PopNode();
		}
	| S_FONTSTYLE	NULL_STRING
	| S_FONTSTYLE	FontStyle
	| S_FONTSTYLE	USE
	| TextLength	MFFloat
		{
			PopNode();
		}
	| S_MAX_EXTENT	SFFloat
		{
			((TextNode *)GetCurrentNodeObject())->setMaxExtent($2);
		}
	;


TextBegin
	: TEXT
		{
			TextNode *text = new TextNode();
			text->setName(GetDEFName());
			AddNode(text);
			PushNode(VRML_NODETYPE_TEXT, text);
		}
	;

Text
	: TextBegin NodeBegin TextElements NodeEnd
		{
			TextNode *text = (TextNode *)GetCurrentNodeObject();
			text->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   TexCoordinate
*
******************************************************************/

TexCoordElements	
	: TexCoordElement TexCoordElements
	|
	;

TexCoordElement
	: S_POINT			MFVec2f
	;


TexCoordBegin
	: TEXTURE_COORDINATE  
		{
			TextureCoordinateNode *texCoord = new TextureCoordinateNode();
			texCoord->setName(GetDEFName());
			AddNode(texCoord);
			PushNode(VRML_NODETYPE_TEXTURECOODINATE, texCoord);
		}
	;

TexCoordinate
	: TexCoordBegin NodeBegin TexCoordElements NodeEnd
		{
			TextureCoordinateNode *texCoord = (TextureCoordinateNode *)GetCurrentNodeObject();
			texCoord->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   TextureTransform
*
******************************************************************/

TextureTransformElements
	: TextureTransformElement TextureTransformElements
	|
	;

TextureTransformElement
	: S_CENTER			SFVec2f
		{
			((TextureTransformNode *)GetCurrentNodeObject())->setCenter(gVec2f);
		}
	| S_ROTATION		SFFloat
		{
			((TextureTransformNode *)GetCurrentNodeObject())->setRotation($2);
		}
	| S_SCALE			SFVec2f
		{
			((TextureTransformNode *)GetCurrentNodeObject())->setScale(gVec2f);
		}
	| S_TRANSLATION		SFVec2f
		{
			((TextureTransformNode *)GetCurrentNodeObject())->setTranslation(gVec2f);
		}
	;


TexTransformBegin
	: TEXTURE_TRANSFORM 
		{
			TextureTransformNode *textureTransform = new TextureTransformNode();
			textureTransform->setName(GetDEFName());
			AddNode(textureTransform);
			PushNode(VRML_NODETYPE_TEXTURETRANSFORM, textureTransform);
		}
	;

TexTransform
	: TexTransformBegin NodeBegin TextureTransformElements NodeEnd
		{
			TextureTransformNode *textureTransform = (TextureTransformNode *)GetCurrentNodeObject();
			textureTransform->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   TimeSensor
*
******************************************************************/

TimeSensorElements	
	: TimeSensorElement TimeSensorElements
	|
	;

TimeSensorElement
	: S_CYCLE_INTERVAL	SFTime
		{
			((TimeSensorNode *)GetCurrentNodeObject())->setCycleInterval($2);
		}
	| S_ENABLED			SFBool
		{
			((TimeSensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	| S_LOOP			SFBool
		{
			((TimeSensorNode *)GetCurrentNodeObject())->setLoop($2);
		}
	| S_STARTTIME		SFTime
		{
			((TimeSensorNode *)GetCurrentNodeObject())->setStartTime($2);
		}
	| S_STOPTIME		SFTime
		{
			((TimeSensorNode *)GetCurrentNodeObject())->setStopTime($2);
		}
	;


TimeSensorBegin
	: TIME_SENSOR
		{
			TimeSensorNode *tsensor = new TimeSensorNode();
			tsensor->setName(GetDEFName());
			AddNode(tsensor);
			PushNode(VRML_NODETYPE_TIMESENSOR, tsensor);
		}
	;

TimeSensor
	: TimeSensorBegin NodeBegin TimeSensorElements NodeEnd
		{
			TimeSensorNode *tsensor = (TimeSensorNode *)GetCurrentNodeObject();
			tsensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*   TouchSensor
*
******************************************************************/

TouchSensorElements	
	: TouchSensorElement TouchSensorElements
	|
	;

TouchSensorElement
	: S_ENABLED			SFBool
		{
			((TouchSensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	;

TouchSensorBegin
	: TOUCH_SENSOR
		{
			TouchSensorNode *touchSensor = new TouchSensorNode();
			touchSensor->setName(GetDEFName());
			AddNode(touchSensor);
			PushNode(VRML_NODETYPE_TOUCHSENSOR, touchSensor);
		}
	;

TouchSensor
	: TouchSensorBegin NodeBegin TouchSensorElements NodeEnd
		{
			TouchSensorNode *touchSensor = (TouchSensorNode *)GetCurrentNodeObject();
			touchSensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*  Transform
*
******************************************************************/

TransformElements	
	: TransformElement TransformElements
	|
	;

TransformElement 
	: children
	| S_CENTER			SFVec3f
		{
			((TransformNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		}
	| S_ROTATION		SFRotation
		{
			((TransformNode *)GetCurrentNodeObject())->setRotation(gRotation);
		}
	| S_SCALE			SFVec3f
		{
			((TransformNode *)GetCurrentNodeObject())->setScale(gVec3f);
		}
	| S_SCALEORIENTATION	SFRotation
	    {
			((TransformNode *)GetCurrentNodeObject())->setScaleOrientation(gRotation);
		}
	| S_TRANSLATION		SFVec3f
		{
			((TransformNode *)GetCurrentNodeObject())->setTranslation(gVec3f);
		}
	| bboxCenter
	| bboxSize
	;

TransformBegin
	: TRANSFORM 
		{
			TransformNode *transform = new TransformNode();
			transform->setName(GetDEFName());
			AddNode(transform);
			PushNode(VRML_NODETYPE_TRANSFORM, transform);
		}
	;

Transform
	: TransformBegin NodeBegin TransformElements NodeEnd
		{
			TransformNode *transform = (TransformNode *)GetCurrentNodeObject();
			transform->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	Viewpoint
*
******************************************************************/

ViewpointElements		
	: ViewpointElement ViewpointElements
	|
	;

ViewpointElement
	: S_FIELD_OF_VIEW			SFFloat
		{
			((ViewpointNode *)GetCurrentNodeObject())->setFieldOfView($2);
		}
	| S_JUMP					SFBool
		{
			((ViewpointNode *)GetCurrentNodeObject())->setJump($2);
		}
	| S_ORIENTATION			SFRotation
		{
			((ViewpointNode *)GetCurrentNodeObject())->setOrientation(gRotation);
		}
	| S_POSITION				SFVec3f
		{
			((ViewpointNode *)GetCurrentNodeObject())->setPosition(gVec3f);
		}
	| S_DESCRIPTION			SFString
		{
			((ViewpointNode *)GetCurrentNodeObject())->setDescription($2);
		}
	;

ViewpointBegin
	: VIEWPOINT 
		{
			ViewpointNode *viewpoint = new ViewpointNode();
			viewpoint->setName(GetDEFName());
			AddNode(viewpoint);
			PushNode(VRML_NODETYPE_VIEWPOINT, viewpoint);
		}
	;

Viewpoint 	
	: ViewpointBegin NodeBegin ViewpointElements NodeEnd
		{
			ViewpointNode *viewpoint = (ViewpointNode *)GetCurrentNodeObject();
			viewpoint->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	VisibilitySensor
*
******************************************************************/

VisibilitySensors
	: VisibilitySensor VisibilitySensors
	|
	;

VisibilitySensor		
	: S_CENTER				SFVec3f
		{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		}
	| S_ENABLED				SFBool
		{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setEnabled($2);
		}
	| S_SIZE				SFVec3f
		{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setSize(gVec3f);
		}
	;

VisibilitySensorBegine
	: VISIBILITY_SENSOR
		{
			VisibilitySensorNode *vsensor = new VisibilitySensorNode();
			vsensor->setName(GetDEFName());
			AddNode(vsensor);
			PushNode(VRML_NODETYPE_VISIBILITYSENSOR, vsensor);
		}
	;

VisibilitySensor	
	: VisibilitySensorBegine NodeBegin VisibilitySensors NodeEnd
		{
			VisibilitySensorNode *vsensor = (VisibilitySensorNode *)GetCurrentNodeObject();
			vsensor->initialize();
			PopNode();
		}
	;

/******************************************************************
*
*	WorldInfo
*
******************************************************************/

WorldInfoElements		
	: WorldInfoElement WorldInfoElements
	|
	;

WorldInfoInfo
	: S_INFO
		{
			PushNode(VRML_NODETYPE_WORLDINFO_INFO, GetCurrentNodeObject());
		}
	;

WorldInfoElement
	: WorldInfoInfo	MFString
		{
			PopNode();
		}
	| S_TITLE					SFString
		{
			((WorldInfoNode *)GetCurrentNodeObject())->setTitle($2);
		}
	;

WorldInfoBegin
	: WORLD_INFO 
		{
			WorldInfoNode *worldInfo = new WorldInfoNode();
			worldInfo->setName(GetDEFName());
			AddNode(worldInfo);
			PushNode(VRML_NODETYPE_WORLDINFO, worldInfo);
		}
	;

WorldInfo			
	: WorldInfoBegin NodeBegin WorldInfoElements NodeEnd
		{
			WorldInfoNode *worldInfo = (WorldInfoNode *)GetCurrentNodeObject();
			worldInfo->initialize();
			PopNode();
		}
	;

%%
