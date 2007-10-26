/* 
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */
#include "WED_MapView.h"

#include "DEMTables.h"
#include "XPLMMenus.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPWidgetUtils.h"
#include "XPUIGraphics.h"
#include "AssertUtils.h"
#include "WED_MapZoomer.h"
#include "WED_MapTool.h"
#include "ObjTables.h"
#include "WED_Progress.h"
#include "WED_SelectionTool.h"
#include "WED_CropTool.h"
#include "WED_BezierTestTool.h"
#include "WED_ImageTool.h"
#include "WED_TerraTool.h"
#include "WED_TopoTester.h"
#include "WED_MeshTester.h"
#include "WED_Selection.h"
#include "WED_Globals.h"
//#include "WED_TriTestTool.h"
#include "WED_Msgs.h"

#include "WED_DEMGraphics.h"

#include "ParamDefs.h"
#include "MapAlgs.h"
#include "AptIO.h"
#include "WED_DrawMap.h"

#include "BitmapUtils.h"
#include "TexUtils.h"

#if IBM
#include "XWinGL.h"
#endif

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

#define GL_EXT_texture_env_combine 1
#define GL_GLEXT_FUNCTION_POINTERS 1

#if APL
	#include <OpenGL/glext.h>
#else
	#include <glext.h>
#endif

#define DRAW_MESH_BORDERS 1

#define DEBUG_PRINT_LAYERS 1

#define DEBUG_PRINT_NORMALS 0

#define DEBUG_PRINT_CORNERS 0

#define DEBUG_PRINT_TRI_PARAMS 0


WED_MapView *		gMapView = NULL;

const int 	kInfoStripHeight = 18;

const char *	kSelButtonLabels[] = { "V", "E", "F", "P", 0 };
const char *	kToolButtonLabels[] = { "Select", "Crop", "Bezier", "Image", "Terra", "TTEST", "MTEST", 0 };

struct	DEMViewInfo_t {
	int				dem;
	const char *	cmdName;
	int				view_mode;
	bool			interpolate; 
	bool			is_enum;
	const char *	format_string;
};
	
static DEMViewInfo_t	kDEMs[] = {
{		NO_VALUE,				"None"							,	0,							false,	false,	" "				},
{		dem_Elevation,			"Elevation"						,	dem_Elevation,				false,	false,	"MSL=%fm "		},
{		dem_OrigLandUse,		"Land Use (Old)"				,	dem_Enum,					false,	true,	"Old LU=%s "	},
{		dem_LandUse,			"Land Use"						,	dem_Enum,					false,	true,	"LU=%s "		},
{		dem_Climate,			"Climate"						,	dem_Enum,					false,	true,	"Climate=%s "	},
{		dem_Biomass,			"Biomass"						,	dem_Biomass,				true,	false,	"Biomass=%f "	},
{		dem_Rainfall,			"Rainfall"						,	dem_Rainfall,				true,	false,	"Rain=%fmm "	},
{		dem_Temperature,		"Temperature"					,	dem_Temperature,			true,	false,	"Temp=%fC "		},
{		dem_TemperatureSeaLevel,"Sea Level Temperature"			,	dem_TemperatureSeaLevel,	true,	false,	"SL Temp=%fC "	},
{		dem_TemperatureRange,	"Temperature Range"				,	dem_TemperatureRange,		true,	false,	"TempR=%fC "	},
{		dem_UrbanDensity,		"Urban Density"					,	dem_UrbanDensity,			true,	false,	"Density=%f "	},
{		dem_UrbanPropertyValue,	"Property Values"				,	dem_UrbanPropertyValue,		true,	false,	"$$=%f "		},
{		dem_UrbanRadial,		"Urban Radial"					,	dem_UrbanRadial,			true,	false,	"Urban Rad=%f "	},
{		dem_UrbanTransport,		"Urban Transport"				,	dem_UrbanTransport,			true,	false,	"Urban Trns=%f "},
{		dem_UrbanSquare,		"Urban Square"					,	dem_UrbanSquare,			true,	false,	"Square=%f "	},
{		dem_Slope,				"Slope"							,	dem_Slope,					true,	false,	"A=%f "			},
{		dem_SlopeHeading,		"Slope Heading"					,	dem_SlopeHeading,			true,	false,	"H=%f "			},
{		dem_RelativeElevation,	"Relative Elevation"			,	dem_RelativeElevation,		true,	false,	"Rel MSL=%f "	},
{		dem_ElevationRange,		"Elevation Range"				,	dem_ElevationRange,			true,	false,	"MSL Rnge=%fm "	},
{		dem_HydroDirection,		"Hydro Flow Direction"			,	dem_Enum,					false,	true,	"%s "			},
{		dem_HydroQuantity,		"Hydro Flow Quantity"			,	dem_HydroQuantity,			true,	false,	"%fm "			},
{		dem_HydroElevation,		"Hydro Elevation"				,	dem_Elevation,				false,	false,	"%fm "			},
{		dem_Wizard,				"Spreadsheet Wizard"			,	dem_Strata,					false,	false,	"%fm "			},

};

const int DEMChoiceCount = sizeof(kDEMs) / sizeof(DEMViewInfo_t);



enum {
	viewCmd_DEMChoice = 0,
	viewCmd_ShowShading,
	viewCmd_ShowTensor,
	viewCmd_DEMDataChoice,
	viewCmd_Break,
	viewCmd_RecalcDEM,	
	viewCmd_PrevDEM,
	viewCmd_NextDEM,
	viewCmd_Break2,
	viewCmd_VecMap,
	viewCmd_Airports,
//	viewCmd_MeshPoints,
//	viewCmd_MeshLines,
//	viewCmd_MeshTrisLo,
	viewCmd_MeshTrisHi,
	viewCmd_MeshTerrains,
//	viewCmd_MeshBorders,
	viewCmd_Break3,
	viewCmd_ZoomSel,
	viewCmd_Count
};

const char *	kCmdNames [] = {
	"Raster Layer",
	"Show Shading on Raster Layer",
	"Show Tensors",
	"Show Raster Data",
	"-",
	"Recalculate Raster Data Preview",
	"Previous Raster",
	"Next Raster",
	"-",
	"Vector Map",
	"Airports",
//	"Mesh Points",
//	"Mesh Lines",
//	"Mesh (Lores)",
	"Mesh (Hires)",
	"Mesh Terrains (Hires)",
//	"Mesh Borders (Hires)",
	"-",
	"Move To Selection",
	0
};

static	const char	kCmdKeys [] = {
	0,	 0,
	0,	 0,
	0,	 0,
	0,	 0,
	0,	 0,	// Divider
	'R', xplm_ControlFlag,
	XPLM_KEY_UP, xplm_ControlFlag + xplm_OptionAltFlag,
	XPLM_KEY_DOWN, xplm_ControlFlag + xplm_OptionAltFlag,
	0,	 0,	// Divider2
	'1', xplm_ControlFlag + xplm_OptionAltFlag,
	'2', xplm_ControlFlag + xplm_OptionAltFlag,
	'3', xplm_ControlFlag + xplm_OptionAltFlag,
	'4', xplm_ControlFlag + xplm_OptionAltFlag,
//	'5', xplm_ControlFlag + xplm_OptionAltFlag,
//	'6', xplm_ControlFlag + xplm_OptionAltFlag,
//	'7', xplm_ControlFlag + xplm_OptionAltFlag,
	0,	0, // Divider 3
	0,	0,
	0,   0	// END
};

static	XPLMMenuID	sViewMenu = NULL;
static	XPLMMenuID	sDEMMenu = NULL;
static	XPLMMenuID	sDEMDataMenu = NULL;
static	int			sDEMType = 0;
static	int			sShowMeshPoints = 1;
static	int			sShowMeshLines  = 1;

int			sShowMap = 1;
int			sShowMeshTrisHi = 1;
int			sShowMeshAlphas = 1;
int			sShowAirports =1;
int			sShowShading = 1;
int			sShowTensors = 1;
float		sShadingAzi = 315;
float		sShadingDecl = 45;

static int			sShowDEMData[DEMChoiceCount-1] = { 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0 };


//static	int			sShowMeshBorders = 1;
//static	int			sShowMeshTrisLo = 1;

void	WED_MapView_HandleMenuCommand(void *, void *);
void	WED_MapView_HandleDEMMenuCommand(void *, void *);
void	WED_MapView_HandleDEMDataMenuCommand(void *, void *);
void	WED_MapView_UpdateCommandStatus(void);

inline const char * QuickToFile(const string& s)
{
	string::size_type p = s.find_last_of("\\/:");
	if (p != s.npos)		return s.c_str() + p;
	else				return s.c_str();
}

void	SetupNormalShading(void)
{
	double	nrm_z    = sin(sShadingDecl * DEG_TO_RAD);
	double	scale_xy = cos(sShadingDecl * DEG_TO_RAD);
	
	double	nrm_x = scale_xy * sin(sShadingAzi * DEG_TO_RAD);
	double	nrm_y = scale_xy * cos(sShadingAzi * DEG_TO_RAD);

	GLfloat	color[4] = { nrm_x * 0.5 + 0.5, nrm_y * 0.5 + 0.5, nrm_z * 0.5 + 0.5, 1.0 };
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, 	GL_DOT3_RGB_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, 	GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, 	GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, 	GL_CONSTANT_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, 	GL_SRC_COLOR);
}

inline	int GetBucketForFace(Bbox2 * buckets, int bucket_count, CDT::Face_handle f)
{
	Point2 p1(f->vertex(0)->point().x(), f->vertex(0)->point().y());
	Point2 p2(f->vertex(1)->point().x(), f->vertex(1)->point().y());
	Point2 p3(f->vertex(2)->point().x(), f->vertex(2)->point().y());
	for (int n = 0; n < bucket_count; ++n)
	{
		if (buckets[n].contains(p1) &&
			buckets[n].contains(p2) &&
			buckets[n].contains(p3))
		return n;
	}
	return bucket_count;
}

inline	int GetBucketForEdge(Bbox2 * buckets, int bucket_count, CDT::Finite_edges_iterator eit)
{
	CDT::Vertex_handle	a = eit->first->vertex(eit->first->ccw(eit->second));
	CDT::Vertex_handle	b = eit->first->vertex(eit->first->cw(eit->second));

	Point2 p1(a->point().x(),a->point().y());
	Point2 p2(b->point().x(),b->point().y());
	for (int n = 0; n < bucket_count; ++n)
	{
		if (buckets[n].contains(p1) &&
			buckets[n].contains(p2))
		return n;
	}
	return bucket_count;
}

WED_MapView::WED_MapView(
                       int                  inLeft,    
                       int                  inTop,    
                       int                  inRight,    
                       int                  inBottom,    
                       int                  inVisible,
                       WED_Pane *			inSuper) :
	WED_Pane(inLeft, inTop , inRight, inBottom, inVisible, "Map", inSuper),
	mNeedRecalcMapFull(true),
	mNeedRecalcMapMeta(true),
	mNeedRecalcDEM(true),
	mNeedRecalcRelief(true),
	mNeedRecalcMeshHi(true),
	mNeedRecalcMeshHiAlpha(true)
{
	DebugAssert(gMapView == NULL);
	gMapView = this;
	for (int n = 0; n < (MESH_BUCKET_SIZE * MESH_BUCKET_SIZE); ++n)
		mDLBuckets[n] = Bbox2(0, 0, 0, 0);
	mDLBuckets[MESH_BUCKET_SIZE * MESH_BUCKET_SIZE] = Bbox2(-180, -90, 180, 90);
	mDLMeshLine = 0;
	mDLMeshFill = 0;

	int n;
	sViewMenu = XPLMCreateMenu("View", NULL, 0, WED_MapView_HandleMenuCommand, reinterpret_cast<void *>(this));
	n = 0;
	while (kCmdNames[n])
	{
		XPLMAppendMenuItem(sViewMenu, kCmdNames[n], (void *) n, 1);
		if (kCmdKeys[n*2])
			XPLMSetMenuItemKey(sViewMenu,n,kCmdKeys[n*2],kCmdKeys[n*2+1]);
		++n;
	}
	sDEMMenu = XPLMCreateMenu("DEMs", sViewMenu, viewCmd_DEMChoice, WED_MapView_HandleDEMMenuCommand, reinterpret_cast<void*>(this));
	sDEMDataMenu = XPLMCreateMenu("DEM Data", sViewMenu, viewCmd_DEMDataChoice, WED_MapView_HandleDEMDataMenuCommand, reinterpret_cast<void*>(this));
	for (n = 0; n < DEMChoiceCount; ++n)
	{	
		XPLMAppendMenuItem(sDEMMenu, kDEMs[n].cmdName, (void *) n, 1);
		if (n != 0)
			XPLMAppendMenuItem(sDEMDataMenu, kDEMs[n].cmdName, (void *) n, 1);
		if (n <= 9)
			XPLMSetMenuItemKey(sDEMMenu, n, '0' + n, xplm_ControlFlag);
		if (n >= 10 && n <= 19)
			XPLMSetMenuItemKey(sDEMMenu, n, '0' + n - 10, xplm_ControlFlag + xplm_ShiftFlag);			
	}
	WED_MapView_UpdateCommandStatus();

	XPLMGenerateTextureNumbers(&mTexID, 1);
	XPLMGenerateTextureNumbers(&mReliefID, 1);
	XPLMGenerateTextureNumbers(&mFlowID, 1);
	mHasTex = false;
	mHasRelief = false;
	mHasFlow = false;

	mCurTool = 0;
	mZoomer = new WED_MapZoomer;
	
	mZoomer->SetPixelBounds(inLeft, inBottom + kInfoStripHeight, inRight, inTop - kInfoStripHeight);

	mToolBtnsOffset   = inLeft + 10;	
	mToolStatusOffset = inLeft + 10;
	n = 0;
	while (kSelButtonLabels[n])
	{
		int labelWidth = 15 + 7 * strlen(kSelButtonLabels[n]);
		mSelModeBtns.push_back(XPCreateWidget(mToolBtnsOffset, inTop, 
						mToolBtnsOffset + labelWidth, inTop - kInfoStripHeight, 1, kSelButtonLabels[n], 0, GetWidget(), xpWidgetClass_Button));
		XPSetWidgetProperty(mSelModeBtns.back(), xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
		XPSetWidgetProperty(mSelModeBtns.back(), xpProperty_ButtonState, n == gSelectionMode);
		mToolBtnsOffset += (labelWidth + 10);
		++n;	
	}
	n = 0;
	mToolBtnsOffset += 10;
	while (kToolButtonLabels[n])
	{
		int labelWidth = 15 + 7 * strlen(kToolButtonLabels[n]);
		mToolBarBtns.push_back(XPCreateWidget(mToolBtnsOffset, inTop, 
						mToolBtnsOffset + labelWidth, inTop - kInfoStripHeight, 1, kToolButtonLabels[n], 0, GetWidget(), xpWidgetClass_Button));
		XPSetWidgetProperty(mToolBarBtns.back(), xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
		XPSetWidgetProperty(mToolBarBtns.back(), xpProperty_ButtonState, n == mCurTool);

		mToolBtnsOffset += (labelWidth + 10);
		++n;	
	}	
	mToolBtnsOffset += 10;

	mTools.push_back(new WED_SelectionTool(mZoomer));	
	mTools.push_back(new WED_CropTool(mZoomer));	
	mTools.push_back(new WED_BezierTestTool(mZoomer));		
	mTools.push_back(new WED_ImageTool(mZoomer));	
	mTools.push_back(new WED_TerraTool(mZoomer));	
	mTools.push_back(new WED_TopoTester(mZoomer));	
	mTools.push_back(new WED_MeshTester(mZoomer));	
	
	SetupForTool();
	
//	XPCreateTab(50, 150, 300, 100, 1, "Tab A;Tab B;Tab C;Tab D", GetWidget());
}
                       
WED_MapView::~WED_MapView()
{
	if (mDLMeshLine != 0)	glDeleteLists(mDLMeshLine, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);			
	if (mDLMeshFill != 0)	glDeleteLists(mDLMeshFill, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);			

	int n;
	delete mZoomer;
	for (n = 0; n < mToolFuncBtns.size(); ++n)
		XPDestroyWidget(mToolFuncBtns[n], true);
	for (n = 0; n < mToolProperties.size(); ++n)
		XPDestroyWidget(mToolProperties[n], true);
	for (n = 0; n < mSelModeBtns.size(); ++n)
		XPDestroyWidget(mSelModeBtns[n], true);
	for (n = 0; n < mToolBarBtns.size(); ++n)
		XPDestroyWidget(mToolBarBtns[n], true);
}

/***************************************************************************************************************************************
 * MAP DRAWING
 ***************************************************************************************************************************************/

void	WED_MapView::DrawSelf(void)
{
	/***************************************************************************************************************************************
	 * BASE LAYER, UNDERLAY AND DECALS
	 ***************************************************************************************************************************************/

	int	l, t, r, b;
	XPGetWidgetGeometry(GetWidget(), &l, &t, &r, &b);
	WED_MapTool * cur = CurTool();
	char * status = NULL;;
	if (cur) status = cur->GetStatusText();
	char * mon = MonitorCaption();

	XPDrawWindow(l, b, r, t,xpWindow_Screen);

	double	pl, pb, pr, pt;
	double	ll, lb, lr, lt;
	mZoomer->GetPixelBounds(pl, pb, pr, pt);
	mZoomer->GetMapVisibleBounds(ll, lb, lr, lt);
	
	XPLMSetGraphicsState(0, 0, 0,  0, 0,  0, 0);
	glColor3f(0.0, 0.0, 0.0);
	
	glBegin(GL_QUADS);
	glVertex2f(pl, pb);
	glVertex2f(pl, pt);
	glVertex2f(pr, pt);
	glVertex2f(pr, pb);			
	glEnd();
	
	glScissor (pl, pb, pr - pl, pt - pb);
	glEnable(GL_SCISSOR_TEST);
	
	for (int n = 0; n < mTools.size(); ++n)
		mTools[n]->DrawFeedbackUnderlay(mTools[n] == cur);

	/***************************************************************************************************************************************
	 * PRECALCULATE DISPLAY LISTS, ETC IF INVALID
	 ***************************************************************************************************************************************/


	// Recalc the map before we nuke the coordinate system, or the progress func will not be visible.
	if (sShowMap)
	{
		if (mNeedRecalcMapFull)
		{
			WED_ProgressFunc(0, 1, "Building graphics for vector map...", 0.0);
			gMap.Index();
			WED_ProgressFunc(0, 1, "Building graphics for vector map...", 0.5);
			PrecalcOGL(gMap,WED_ProgressFunc);
			WED_ProgressFunc(0, 1, "Building graphics for vector map...", 1.0);
		} 
		else if (mNeedRecalcMapMeta)
		{
			WED_ProgressFunc(0, 1, "Updating graphics for vector map...", 0.0);
			RecalcOGLColors(gMap,WED_ProgressFunc);
			WED_ProgressFunc(0, 1, "Updating graphics for vector map...", 1.0);
		}
		
		mNeedRecalcMapMeta = mNeedRecalcMapFull = false;
	}
	

	if (sShowMeshTrisHi)
	{
		if (mNeedRecalcMeshHi)
		{
			WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", 0.0);
		
			if (mDLMeshLine != 0)	glDeleteLists(mDLMeshLine, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);			
			mDLMeshLine = glGenLists(MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);
			
			for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
			{
				WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", (float) n / (float) (MESH_BUCKET_SIZE*MESH_BUCKET_SIZE));			
				glNewList(mDLMeshLine + n, GL_COMPILE);
				glBegin(GL_LINES);
				mEdges[n] = 0;

				for (CDT::Finite_edges_iterator eit = gTriangulationHi.finite_edges_begin(); eit != gTriangulationHi.finite_edges_end(); ++eit)
				if (GetBucketForEdge(mDLBuckets, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE, eit) == n)
				{
					++mEdges[n];
					GLfloat	color[4] = { 1.0, 1.0, 1.0, 1.0 };
					if (!gTriangulationHi.is_constrained(*eit))
					{
						if (sDEMType)
							color[0] = 0.0, color[1] = 0.0, color[2] = 0.5, color[3] = 0.8;
						else
							color[0] = 0.4, color[1] = 0.4, color[2] = 0.8, color[3] = 0.8;
					}
					
					CDT::Vertex_handle	a = eit->first->vertex(eit->first->ccw(eit->second));
					CDT::Vertex_handle	b = eit->first->vertex(eit->first->cw(eit->second));
					
					CDT::Point	p1 = a->point();
					CDT::Point p2 = b->point();
					
					glColor4fv(color);
					glVertex2f(p1.x(), p1.y());
					glColor4fv(color);
					glVertex2f(p2.x(), p2.y());
				}
				
				glEnd();
				glEndList();				
			}
			mNeedRecalcMeshHi = false;
			WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", 1.0);
			
		}
	}
	
	if (sShowMeshAlphas)
	{	
		glDisable(GL_CULL_FACE);
		
		if (mNeedRecalcMeshHiAlpha)
		{
			WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", 0.0);
		
			if (mDLMeshFill != 0)	glDeleteLists(mDLMeshFill, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);			
			mDLMeshFill = glGenLists(MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);
			
			for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
			{
				WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", (float) n / (float) (MESH_BUCKET_SIZE*MESH_BUCKET_SIZE));
			
				glNewList(mDLMeshFill + n, GL_COMPILE);
				glBegin(GL_TRIANGLES);
				mTris[n] = 0;
		
				for (CDT::Finite_faces_iterator fit = gTriangulationHi.finite_faces_begin(); fit != gTriangulationHi.finite_faces_end(); ++fit)
				if (GetBucketForFace(mDLBuckets, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE, fit) == n)
				{
					++mTris[n];
					if (fit->info().terrain != NO_VALUE)
					{				
						CDT::Vertex_handle	a = fit->vertex(2);
						CDT::Vertex_handle	b = fit->vertex(1);
						CDT::Vertex_handle	c = fit->vertex(0);
						
						CDT::Point p1 = a->point();
						CDT::Point p2 = b->point();
						CDT::Point p3 = c->point();

						float	col[4];

						if (fit->info().terrain != terrain_Water)
						{
							GetNaturalTerrainColor(fit->info().terrain, col);
							col[3] = 0.5;
						} else {

							RGBColor_t&	rgbc = gEnumColors[fit->info().terrain];
							col[0] = rgbc.rgb[0];
							col[1] = rgbc.rgb[1];
							col[2] = rgbc.rgb[2];
							col[3] = 0.5;
							if (fit->info().normal[2] < 0.97) {
								col[0] = 1.0; col[1] = 0.0; col[2] = 0.0;
							}
						}
						
						glColor4fv(col);					glVertex2f(p1.x(), p1.y());
						glColor4fv(col);					glVertex2f(p2.x(), p2.y());
						glColor4fv(col);					glVertex2f(p3.x(), p3.y());

#if DRAW_MESH_BORDERS
						for (set<int>::iterator i = fit->info().terrain_border.begin(); i != fit->info().terrain_border.end(); ++i)
						{
							GetNaturalTerrainColor(*i, col);

							col[3] = 0.5 * fit->vertex(2)->info().border_blend[*i];
							glColor4fv(col);	
							glVertex2f(p1.x(), p1.y());

							col[3] = 0.5 * fit->vertex(1)->info().border_blend[*i];
							glColor4fv(col);	
							glVertex2f(p2.x(), p2.y());

							col[3] = 0.5 * fit->vertex(0)->info().border_blend[*i];
							glColor4fv(col);	
							glVertex2f(p3.x(), p3.y());
						}
#endif						
					}
				}
				
				glEnd();
				glEndList();				
			}
			mNeedRecalcMeshHiAlpha = false;
			WED_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", 1.0);			
		}
	}

	/***************************************************************************************************************************************
	 * MATRIX SETUP - AT THIS POINT WE ARE IN LAT LON COORDS
	 ***************************************************************************************************************************************/
	

	double	lw = lr - ll;
	double	lh = lt - lb;
	double	pw = pr - pl;
	double	ph = pt - pb;
	Bbox2	logical_vis(ll, lb, lr, lt);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(pl, pb, 0.0);
	glScaled(pw / lw, ph / lh, 1.0);
	glTranslated(-ll, -lb, 0.0);	

	/***************************************************************************************************************************************
	 * RASTER LAYER
	 ***************************************************************************************************************************************/
	
	if (mNeedRecalcDEM)
	{
		if (RecalcDEM(mNeedRecalcRelief))
			mNeedRecalcRelief = false;
		mNeedRecalcDEM = false;
	}
		
	if (mHasTex)
	{
		bool	 do_relief = mHasRelief && sShowShading;
		XPLMSetGraphicsState(0, do_relief ? 2 : 1, 0,   0, 0,  0, 0);
		XPLMBindTexture2d(mTexID, do_relief ? 1 : 0);
		if (do_relief) 
		{
			XPLMBindTexture2d(mReliefID, 0);
			SetupNormalShading();			
		}
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		if (do_relief)
		{
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0  , 0.0  );glTexCoord2f(0.0     , 0.0     );		glVertex2f((mDEMBounds[0]), (mDEMBounds[1]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0  , mTexT);glTexCoord2f(0.0     , mReliefT);		glVertex2f((mDEMBounds[0]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexS, mTexT);glTexCoord2f(mReliefS, mReliefT);		glVertex2f((mDEMBounds[2]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexS, 0.0  );glTexCoord2f(mReliefS, 0.0     );		glVertex2f((mDEMBounds[2]), (mDEMBounds[1]));
		} else {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  , 0.0  );glVertex2f((mDEMBounds[0]), (mDEMBounds[1]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  , mTexT);glVertex2f((mDEMBounds[0]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mTexS, mTexT);glVertex2f((mDEMBounds[2]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mTexS, 0.0  );glVertex2f((mDEMBounds[2]), (mDEMBounds[1]));
		}
		glEnd();
		
		if (mHasRelief) 
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		
	}

	if(mHasFlow && sShowTensors)
	{
		XPLMSetGraphicsState(0, 11, 0,   0, 0,  0, 0);
		XPLMBindTexture2d(mFlowID, 0);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  ,  0.0   );glVertex2f((mFlowBounds[0]), (mFlowBounds[1]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  ,  mFlowT);glVertex2f((mFlowBounds[0]), (mFlowBounds[3]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mFlowS, mFlowT);glVertex2f((mFlowBounds[2]), (mFlowBounds[3]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mFlowS, 0.0   );glVertex2f((mFlowBounds[2]), (mFlowBounds[1]));
		glEnd();
	}
	
	/***************************************************************************************************************************************
	 * VECTOR MAP
	 ***************************************************************************************************************************************/	
	
	if (sShowMap)
	{		
		DrawMapBucketed(gMap, 
			ll, lb, lr, lt,
//			pl, pb, pr, pt,
			gVertexSelection,
			gEdgeSelection, 
			gFaceSelection,
			gPointFeatureSelection);
	}
	
	/***************************************************************************************************************************************
	 * DEBUG: PTS AND LINES
	 ***************************************************************************************************************************************/	

	XPLMSetGraphicsState(0, 0, 0,    0, 1,    0, 0);

	if (sShowMeshPoints)
	{
		glPointSize(3);
		glBegin(GL_POINTS);
		for (vector<pair<Point2, Point3> >::iterator i = gMeshPoints.begin(); i != gMeshPoints.end(); ++i)
		{
			glColor3f(i->second.x, i->second.y, i->second.z);
			glVertex2f((i->first.x),
					   (i->first.y));		
		}
		glEnd();
		glPointSize(1);
	}
	
	if (sShowMeshLines)
	{
		glLineWidth(2);
		glBegin(GL_LINES);
		for (vector<pair<Point2, Point3> >::iterator i = gMeshLines.begin(); i != gMeshLines.end(); ++i)
		{
			glColor4f(i->second.x, i->second.y, i->second.z, 0.5);
			glVertex2f((i->first.x),
					   (i->first.y));		
		}
		glEnd();
		glLineWidth(1);
	}
	
	/***************************************************************************************************************************************
	 * MESH AS LINES AND TERRAIN (DISPLAY LISTS - EASY)
	 ***************************************************************************************************************************************/	

	if (sShowMeshTrisHi)
	{		
		for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
		if (mDLBuckets[n].overlap(logical_vis))
		{
			glCallList(mDLMeshLine + n);
		}
	}		
	
	if (sShowMeshAlphas)
	{	
		glDisable(GL_CULL_FACE);				
		for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
		if (mDLBuckets[n].overlap(logical_vis))
		{
			glCallList(mDLMeshFill + n);
		}
		
	}

	/***************************************************************************************************************************************
	 * AIRPORTS
	 ***************************************************************************************************************************************/

	if (sShowAirports)
	{
		double e, s, n, w;
		mZoomer->GetMapVisibleBounds(w, s, e, n);
		Bbox2	vis_area(w, s, e, n);
		XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_QUADS);
		set<int>	apts;
		FindAirports(vis_area, gAptIndex, apts);
		for (set<int>::iterator e = apts.begin(); e != apts.end(); ++e)
		{
			int n = *e;			
			if (vis_area.overlap(gApts[n].bounds))
			{
				glColor3f(1.0, 0.0, 1.0);
//				DebugAssert(gApts[n].bounds.xmin() <= gApts[n].bounds.xmax());
//				DebugAssert(gApts[n].bounds.ymin() <= gApts[n].bounds.ymax());
//				glVertex2f((gApts[n].bounds.xmin()),
//							(gApts[n].bounds.ymin()));
//				glVertex2f((gApts[n].bounds.xmin()),
//							(gApts[n].bounds.ymax()));
//				glVertex2f((gApts[n].bounds.xmax()),
//							(gApts[n].bounds.ymax()));
//				glVertex2f((gApts[n].bounds.xmax()),
//							(gApts[n].bounds.ymin()));
			
				for (int m = 0; m < gApts[n].pavements.size(); ++m)
				{
					for (int i = 0, j = 0; i < gApts[n].pavements[m].quad_coords.size(); i += 2, j += 3)
					{
						glColor3f(gApts[n].pavements[m].quad_colors[j],gApts[n].pavements[m].quad_colors[j+1],gApts[n].pavements[m].quad_colors[j+2]);
						glVertex2f(
							(gApts[n].pavements[m].quad_coords[i]),
							(gApts[n].pavements[m].quad_coords[i+1]));
					}
				}
//				glEnd();
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				float col[3] = { 1.0, 1.0, 1.0 };
//				XPLMDrawString(col, (gApts[n].bounds.xmax()),(gApts[n].bounds.ymin()),
//														gApts[n].icao.c_str(), NULL, xplmFont_Basic);
				
//				XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);
//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);				
				glBegin(GL_QUADS);
			}
		}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	/***************************************************************************************************************************************
	 * BEACHES (DISABLED)
	 ***************************************************************************************************************************************/

#if 0
	if (sShowMeshBorders)
	{
		glLineWidth(2);
		glColor3f(1.0, 0.0, 1.0);
		glBegin(GL_LINES);
		for (CDT::Finite_faces_iterator fit = gTriangulationHi.finite_faces_begin();
			fit != gTriangulationHi.finite_faces_end(); ++fit)
		{
			if (fit->info().terrain_general != -1)
			{				
				CDT::Vertex_handle	a = fit->vertex(2);
				CDT::Vertex_handle	b = fit->vertex(1);
				CDT::Vertex_handle	c = fit->vertex(0);
				
				CDT::Point p1 = a->point();
				CDT::Point p2 = b->point();
				CDT::Point p3 = c->point();
				
				if (fit->info().beaches[0] != -1)
				{
					glVertex2f(	(p1.x()),
								(p1.y()));				
					glVertex2f(	(p2.x()),
								(p2.y()));
				}
				if (fit->info().beaches[1] != -1)
				{
					glVertex2f(	(p1.x()),
								(p1.y()));				
					glVertex2f(	(p3.x()),
								(p3.y()));
				}
				if (fit->info().beaches[2] != -1)
				{
					glVertex2f(	(p2.x()),
								(p2.y()));				
					glVertex2f(	(p3.x()),
								(p3.y()));
				}				
			}
		}
		glEnd();
		glLineWidth(1);
	}
#endif

	/***************************************************************************************************************************************
	 * LOW RES MESH (DISABLED)
	 ***************************************************************************************************************************************/

#if 0
	if (sShowMeshTrisLo)
	{	
		for (CDT::Finite_edges_iterator eit = gTriangulationLo.finite_edges_begin();
			eit != gTriangulationLo.finite_edges_end(); ++eit)
		{
			if (gTriangulationLo.is_constrained(*eit))
			{
				glColor3f(1.0, 1.0, 1.0);
//				glPointSize(2);
			} else {
				glColor4f(0.3, 8.0, 0.3, 0.6);
//				glPointSize(1);
			}
			
			glBegin(GL_LINES);
			
			CDT::Vertex_handle	a = eit->first->vertex(eit->first->ccw(eit->second));
			CDT::Vertex_handle	b = eit->first->vertex(eit->first->cw(eit->second));
			
			CDT::Point	p1 = a->point();
			CDT::Point p2 = b->point();
			
			glVertex2f(	(p1.x()),
						(p1.y()));
			glVertex2f(	(p2.x()),
						(p2.y()));
			
			glEnd();
			
		}
//		glPointSize(1);

#if !DEV
put in  color enums?
#endif
		glBegin(GL_TRIANGLES);
		for (CDT::Finite_faces_iterator fit = gTriangulationLo.finite_faces_begin();
			fit != gTriangulationLo.finite_faces_end(); ++fit)
		{
			if (fit->info().terrain_general != NO_DATA)
			{
				switch(fit->info().terrain_general) {
				case terrain_VirtualOrtho00:	glColor4f(1.0, 0.0, 0.0, 0.3);		break;
				case terrain_VirtualOrtho01:	glColor4f(0.0, 1.0, 0.0, 0.3);		break;
				case terrain_VirtualOrtho10:	glColor4f(1.0, 1.0, 1.0, 0.3);		break;
				case terrain_VirtualOrtho11:	glColor4f(0.0, 0.0, 1.0, 0.3);		break;
				case NO_VALUE:					glColor4f(1.0, 0.0, 1.0, 0.3);		break;
				default:						glColor4f(0.0, 1.0, 1.0, 0.3);		break;
				}
				
				CDT::Vertex_handle	a = fit->vertex(2);
				CDT::Vertex_handle	b = fit->vertex(1);
				CDT::Vertex_handle	c = fit->vertex(0);
				
				CDT::Point p1 = a->point();
				CDT::Point p2 = b->point();
				CDT::Point p3 = c->point();
				
				glVertex2f(	(p1.x()),
							(p1.y()));
				glVertex2f(	(p2.x()),
							(p2.y()));
				glVertex2f(	(p3.x()),
							(p3.y()));
				
			}
		}
		glEnd();
	}
#endif

	/***************************************************************************************************************************************
	 * MISC CLEANUP AND CAPTIONS
	 ***************************************************************************************************************************************/

	glPopMatrix();

	for (int n = 0; n < mTools.size(); ++n)
		mTools[n]->DrawFeedbackOverlay(mTools[n] == cur);

	glDisable(GL_SCISSOR_TEST);	

		int w, h;
		static GLfloat	white[3] = { 1.0, 1.0, 1.0 };
	
	XPLMGetFontDimensions(xplmFont_Basic, &w, &h, NULL);
	
	if (mon)
	{
		XPLMDrawTranslucentDarkBox(l + 3, b + 20 + h, l + 5 + strlen(mon) * w, b + 20 - 1);
		XPLMDrawString(white, l + 7, b + 20 + 1, mon, NULL, xplmFont_Basic);
	}
	if (status)
	{
		XPLMDrawString(white, mToolStatusOffset, b + 7, status, NULL, xplmFont_Basic);
	}
	
	{
		int	x, y;
		XPLMGetMouseLocation(&x, &y);
		double	lat, lon;
		lat = mZoomer->YPixelToLat(y);
		lon = mZoomer->XPixelToLon(x);	
		
		
		int	k = t - 30;
		
		for (int n = 0; n < DEMChoiceCount; ++n)
		{
			char buf[1024];
			if (n == 0)
			{
				sprintf(buf, "Viewing: %s", kDEMs[sDEMType].cmdName);
				XPLMDrawTranslucentDarkBox(l+3, k + h, l + 5 + strlen(buf) * w, k -1);
				XPLMDrawString(white, l + 5, k, buf, NULL, xplmFont_Basic);
				k -= (h+1);
			}
			else if (sShowDEMData[n-1] || n == sDEMType)
			{	
				if (gDem.count(	kDEMs[n].dem ))
				{	
					float hh = gDem[kDEMs[n].dem].xy_nearest(lon, lat, x, y);
					
					// HACK city - for certain DEMs, do the trig on the fly.
					
					if (kDEMs[n].dem == dem_Slope)
						hh = RAD_TO_DEG * acos(1.0 - hh);
					else if (kDEMs[n].dem == dem_SlopeHeading)
						hh = RAD_TO_DEG * acos(hh);
					
					if (kDEMs[n].is_enum)
						sprintf(buf,kDEMs[n].format_string,FetchTokenString(hh));
					else
						sprintf(buf,kDEMs[n].format_string,hh);
					XPLMDrawTranslucentDarkBox(l+3, k + h, l + 5 + strlen(buf) * w, k -1);
					XPLMDrawString(white, l + 5, k, buf, NULL, xplmFont_Basic);
					k -= (h+1);				
				}
			}
		}
	}	
	
	const char * nat = QuickToFile(gNaturalTerrainFile);
	const char * obj = QuickToFile(gObjPlacementFile);
	
	XPLMDrawTranslucentDarkBox(r-strlen(nat) * w - 20, t - 30 + h, r - 15, t - 30 - 1);
	XPLMDrawString(white, r - strlen(nat) * w - 20, t - 30, nat, NULL, xplmFont_Basic);

	XPLMDrawTranslucentDarkBox(r-gLanduseTransFile.size() * w - 20, t - 50 + h, r - 15, t - 50 - 1);
	XPLMDrawString(white, r - gLanduseTransFile.size() * w - 20, t - 50, gLanduseTransFile.c_str(), NULL, xplmFont_Basic);

	XPLMDrawTranslucentDarkBox(r-gNaturalTerrainFile.size() * w - 20, t - 30 + h, r - 15, t - 30 - 1);
	XPLMDrawString(white, r - gNaturalTerrainFile.size() * w - 20, t - 30, gNaturalTerrainFile.c_str(), NULL, xplmFont_Basic);

	XPLMDrawTranslucentDarkBox(r-gReplacementClimate.size() * w - 20, t - 70 + h, r - 15, t - 70 - 1);
	XPLMDrawString(white, r - gReplacementClimate.size() * w - 20, t - 70, gReplacementClimate.c_str(), NULL, xplmFont_Basic);

	XPLMDrawTranslucentDarkBox(r-gReplacementRoads.size() * w - 20, t - 90 + h, r - 15, t - 90 - 1);
	XPLMDrawString(white, r - gReplacementRoads.size() * w - 20, t - 90, gReplacementRoads.c_str(), NULL, xplmFont_Basic);

	XPLMDrawTranslucentDarkBox(r-strlen(obj) * w - 20, t - 120 + h, r - 15, t - 120 - 1);
	XPLMDrawString(white, r - strlen(obj) * w - 20, t - 120, obj, NULL, xplmFont_Basic);

	
	char	buf[50];
	int	x, y;
	int	lat, lon;
	XPLMGetMouseLocation(&x, &y);
	lat = mZoomer->YPixelToLat(y) * 1000000.0;
	lon = mZoomer->XPixelToLon(x) * 1000000.0;
	char	ns = 'N';
	char	ew = 'E';
	if (lat < 0) { lat = -lat; ns = 'S'; }
	if (lon < 0) { lon = -lon; ew = 'W'; }
	sprintf(buf,"Lat: %02d.%06d%c Lon: %03d.%06d%c", 
		lat / 1000000, lat % 1000000, ns,
		lon / 1000000, lon % 1000000, ew);
	XPLMDrawTranslucentDarkBox(r-300, b + 20 + h, r - 300 + strlen(buf) * w, b + 20 - 1);	
	XPLMDrawString(white, r - 300, b + 20, buf, NULL, xplmFont_Basic);
}

/***************************************************************************************************************************************
 * MAP INTERACTION
 ***************************************************************************************************************************************/

int		WED_MapView::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	WED_MapTool * cur = CurTool();
	if (cur && cur->HandleClick(status, x, y, button)) { UpdateForTool(); return 1; }
	
	static	int oldX = x, oldY = y;
	
	switch(status) {
	case xplm_MouseDown:
		oldX = x; 
		oldY = y;
		break;
	case xplm_MouseDrag:
		mZoomer->PanPixels(oldX, oldY, x, y);
		oldX = x; 
		oldY = y;
		break;	
	case xplm_MouseUp:
		mZoomer->PanPixels(oldX, oldY, x, y);
		break;
	}
	
	return 1;
}

int		WED_MapView::HandleKey(char key, XPLMKeyFlags flags, char vkey)
{
	return 0;
}	

int		WED_MapView::HandleMouseWheel(int x, int y, int direction)
{
	double	zoom = 1.0;
	while (direction > 0)
	{
		zoom *= 1.1;
		--direction;
	}
	while (direction < 0)
	{
		zoom /= 1.1;
		++direction;
	}
	
	if (zoom != 1.0)
		mZoomer->ZoomAround(zoom, x, y);
	return 1;
}

WED_MapTool * WED_MapView::CurTool(void)
{
	if (mCurTool < 0 || mCurTool >= mTools.size()) return NULL;
	return mTools[mCurTool];
}

void	WED_MapView::HandleNotification(int catagory, int message, void * param)
{
	int n;
	switch(catagory) {
	case wed_Cat_File:
		switch(message) {
		case wed_Msg_FileLoaded:
			{
				mNeedRecalcMapFull = true;
				mNeedRecalcMapMeta = true;
				mNeedRecalcDEM = true;
				mNeedRecalcRelief = true;
				mNeedRecalcMeshHi = true;
				mNeedRecalcMeshHiAlpha = true;
				Bbox2		full;
				if (gDem.empty())
				{
					Point2 sw, ne;
					CalcBoundingBox(gMap, full.p1, full.p2);
					mZoomer->SetMapLogicalBounds(
								full.p1.x,
								full.p1.y,
								full.p2.x,
								full.p2.y);
					mZoomer->SetAspectRatio(1.0 / cos((full.p1.y + full.p2.y) * 0.5 * PI / 180.0));
				} else {
					int e = gDem.begin()->first;
					mZoomer->SetMapLogicalBounds(
								gDem[e].mWest,
								gDem[e].mSouth,
								gDem[e].mEast,
								gDem[e].mNorth);
					mZoomer->SetAspectRatio(1.0 / cos((gDem[e].mSouth + gDem[e].mNorth) * 0.5 * PI / 180.0));
					full = Bbox2(gDem[e].mWest, gDem[e].mSouth, gDem[e].mEast, gDem[e].mNorth);
				}		
				mZoomer->ZoomShowAll();		
				
				for (int y = 0; y < MESH_BUCKET_SIZE; ++y)
				for (int x = 0; x < MESH_BUCKET_SIZE; ++x)
				{
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p1.x = full.p1.x + (full.p2.x - full.p1.x) * (float) (x  ) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p1.y = full.p1.y + (full.p2.y - full.p1.y) * (float) (y  ) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p2.x = full.p1.x + (full.p2.x - full.p1.x) * (float) (x+1) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p2.y = full.p1.y + (full.p2.y - full.p1.y) * (float) (y+1) / (float) MESH_BUCKET_SIZE;
				}
			}	
			break;
		case wed_Msg_RasterChange:
			mNeedRecalcDEM = true;
			mNeedRecalcRelief = true;
			break;
		case wed_Msg_VectorChange:
			mNeedRecalcMapFull = true;
			break;
		case wed_Msg_VectorMetaChange:
			mNeedRecalcMapMeta = true;
			break;
		case wed_Msg_TriangleHiChange:
			mNeedRecalcMeshHi = true;
			mNeedRecalcMeshHiAlpha = true;
			break;
		case wed_Msg_AirportsLoaded:
			break;
		}
		break;
	case wed_Cat_Selection:
		switch(message) {
		case wed_Msg_SelectionModeChanged:
			{
				for (n = 0; n < mSelModeBtns.size(); ++n)
					XPSetWidgetProperty(mSelModeBtns[n], xpProperty_ButtonState, n == gSelectionMode);
			}
			break;
		}
		break;
	}
	WED_MapView_UpdateCommandStatus();
}

void	WED_MapView::SetupForTool(void)
{
	int n;
	
	for (n = 0; n < mToolBarBtns.size(); ++n)
		XPSetWidgetProperty(mToolBarBtns[n], xpProperty_ButtonState, mCurTool == n);
	
	// Blow away old btns
	for (n = 0; n < mToolFuncBtns.size(); ++n)
		XPDestroyWidget(mToolFuncBtns[n], true);
	for (n = 0; n < mToolProperties.size(); ++n)
		XPDestroyWidget(mToolProperties[n], true);

	mToolProperties.clear();
	mToolFuncBtns.clear();
	
	int l, b, r, t;
	XPGetWidgetGeometry(GetWidget(), &l, &t, &r, &b);
	mToolStatusOffset = l + 10;

	WED_MapTool * cur = CurTool();
	if (cur)
	{
		int	btnLeft = mToolBtnsOffset;
		for (n = 0; n < cur->GetNumButtons(); ++n)
		{
			string	label;
			cur->GetNthButtonName(n, label);
			int labelWidth = 15 + 7 * label.length();
			mToolFuncBtns.push_back(XPCreateWidget(btnLeft, t, btnLeft + labelWidth, t - kInfoStripHeight,
				1, label.c_str(), 0, GetWidget(), xpWidgetClass_Button));
			btnLeft += (labelWidth + 10);
		}
		
		for (n = 0; n < cur->GetNumProperties(); ++n)
		{
			string	prop;
			double	val;
			char	valStr[50];
			cur->GetNthPropertyName(n, prop);
			val = cur->GetNthPropertyValue(n);
			sprintf(valStr, "%lf", val);
			int	labelWidth = 15 + 7 * prop.length();
			mToolProperties.push_back(
				XPCreateWidget(mToolStatusOffset, b + kInfoStripHeight, mToolStatusOffset + labelWidth, b, 
					1, prop.c_str(), 0, GetWidget(), xpWidgetClass_Caption));
			XPSetWidgetProperty(mToolProperties.back(), xpProperty_CaptionLit, 1);
			mToolStatusOffset += (10 + 	labelWidth);

			labelWidth = 80;
			mToolProperties.push_back(
				XPCreateWidget(mToolStatusOffset, b + kInfoStripHeight, mToolStatusOffset + labelWidth, b,
					1, valStr, 0, GetWidget(), xpWidgetClass_TextField));				
			mToolStatusOffset += (10 + 	labelWidth);
		}
		
		mToolStatusOffset += 10;
	}
}

void	WED_MapView::UpdateForTool(void)
{
	WED_MapTool * cur = CurTool();
	if (!cur) return;
	for (int n = 0; n < cur->GetNumProperties(); ++n)
	{
		double	d = cur->GetNthPropertyValue(n);
		char	buf[50];
		sprintf(buf,"%lf", d);
		XPSetWidgetDescriptor(mToolProperties[n * 2 + 1], buf);
	}
}

int		WED_MapView::MessageFunc(
                                   XPWidgetMessage      inMessage,    
                                   long                 inParam1,    
                                   long                 inParam2)
{
	XPWidgetID	target;
	int n;
	WED_MapTool * cur = CurTool();
	switch(inMessage) {
	case xpMsg_KeyPress:
		if (cur)
		{
			if (KEY_CHAR(inParam1) == XPLM_KEY_RETURN)
			{
				target = XPGetWidgetWithFocus();
				for (n = 0; n < cur->GetNumProperties(); ++n)
				{
					if (target == mToolProperties[n*2+1])
					{
						char	buf[60];
						XPGetWidgetDescriptor(target, buf, sizeof(buf));
						cur->SetNthPropertyValue(n, atof(buf));
						UpdateForTool();
						XPLoseKeyboardFocus(target);
						return 1;
					}
				}
			}
			if (KEY_CHAR(inParam1) == XPLM_KEY_ESCAPE)
			{
				XPLoseKeyboardFocus(XPGetWidgetWithFocus());
				UpdateForTool();
				return 1;
			}
		}
		return WED_Pane::MessageFunc(inMessage, inParam1, inParam2);
	case xpMsg_PushButtonPressed:
	case xpMsg_ButtonStateChanged:
		target = (XPWidgetID) inParam1;
		if (cur)
		for (n = 0; n < mToolFuncBtns.size(); ++n)
		{
			if (target == mToolFuncBtns[n])
			{
				cur->NthButtonPressed(n);
				UpdateForTool();
				return 1;
			}
		}
		for (n = 0; n < mSelModeBtns.size(); ++n)
		{
			if (target == mSelModeBtns[n])
			{
				WED_SetSelectionMode(n);
				return 1;
			}
		}
		for (n = 0; n < mToolBarBtns.size(); ++n)
		{
			if (target == mToolBarBtns[n])
			{
				if (mCurTool != n)
				{
					mCurTool = n;
					SetupForTool();
				}				
				return 1;
			}
		}
		
		return 0;
	default:
		return WED_Pane::MessageFunc(inMessage, inParam1, inParam2);
	}
}                                   


bool	WED_MapView::RecalcDEM(bool do_relief)
{
	if (sDEMType == 0)
	{
		mHasTex = false;
		return false;
	}
	
	ImageInfo	image;
	double	south, north, east, west;
	
	mZoomer->GetMapVisibleBounds(west, south, east, north);
	
	DEMGeo	sub;
	int param = kDEMs[sDEMType].dem;
	int mode = 	kDEMs[sDEMType].view_mode;
	bool nearest = !kDEMs[sDEMType].interpolate;

	if (gDem.find(param) == gDem.end())
	{
		mHasTex = false;
		return false;
	}

	const DEMGeo&	master = gDem[param];
	
	if (DEMToBitmap(master, image, mode) == 0)
	{
		mDEMBounds[3] = master.mNorth;
		mDEMBounds[1] = master.mSouth;
		mDEMBounds[0] = master.mWest;
		mDEMBounds[2] = master.mEast;
		
		if (LoadTextureFromImage(image, mTexID, tex_Mipmap + (nearest ? 0 : tex_Linear), NULL, NULL, &mTexS, &mTexT))
		{
			mHasTex = true;
		}
		DestroyBitmap(&image);
	}	
	
	if (do_relief)
	{
		if (gDem.count(dem_Elevation) == 0)
			mHasRelief = false;
		else {
			const DEMGeo&	master = gDem[dem_Elevation];
			
			if (DEMToBitmap(master, image, dem_Normals) == 0)
			{
				mDEMBounds[3] = master.mNorth;
				mDEMBounds[1] = master.mSouth;
				mDEMBounds[0] = master.mWest;
				mDEMBounds[2] = master.mEast;
				
				if (LoadTextureFromImage(image, mReliefID, tex_Mipmap + (tex_Linear), NULL, NULL, &mReliefS, &mReliefT))
				{
					mHasRelief = true;
				}
				DestroyBitmap(&image);		
			}
		}
	}
	return true;
}

void	WED_MapView_HandleDEMMenuCommand(void * r, void * i)
{
	sDEMType = (int) i;
	WED_MapView * me = (WED_MapView *) r;
	me->mNeedRecalcDEM = true;
	WED_MapView_UpdateCommandStatus();	
}

void	WED_MapView_HandleDEMDataMenuCommand(void * r, void * i)
{
	int item = (int) i - 1;
	sShowDEMData[item] = 1 - sShowDEMData[item];

	WED_MapView_UpdateCommandStatus();	
}

void	WED_MapView_HandleMenuCommand(void * r, void * i)
{
	int cmd = (int) i;
	WED_MapView * me = (WED_MapView *) r;
	switch(cmd) {
	case viewCmd_PrevDEM:
		sDEMType--;
		if (sDEMType < 0) sDEMType = DEMChoiceCount-1;
		me->mNeedRecalcDEM = true;
		break;		
	case viewCmd_NextDEM:
		sDEMType++;
		if (sDEMType >= DEMChoiceCount) sDEMType = 0;
		me->mNeedRecalcDEM = true;
		break;		
	case viewCmd_RecalcDEM:
		me->mNeedRecalcDEM = true;
		me->mNeedRecalcRelief = true;
		break;
	case viewCmd_VecMap:		sShowMap = !sShowMap;				break;
	case viewCmd_Airports:		sShowAirports = !sShowAirports;		break;
	case viewCmd_ShowShading:	sShowShading = !sShowShading;		break;
	case viewCmd_ShowTensor:	sShowTensors = !sShowTensors;		break;
//	case viewCmd_MeshPoints:	sShowMeshPoints = !sShowMeshPoints;	break;
//	case viewCmd_MeshLines:		sShowMeshLines = !sShowMeshLines;	break;
//	case viewCmd_MeshTrisLo:	sShowMeshTrisLo = !sShowMeshTrisLo;	break;
	case viewCmd_MeshTrisHi:	sShowMeshTrisHi = !sShowMeshTrisHi;	break;
	case viewCmd_MeshTerrains:	sShowMeshAlphas = !sShowMeshAlphas;	break;
//	case viewCmd_MeshBorders:	sShowMeshBorders = !sShowMeshBorders;	break;
	case viewCmd_ZoomSel:
		{
			Bbox2	bounds;
			bool	has = false;
			for (set<GISFace *>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
			if (!(*f)->is_unbounded())
			{
				if (has)	bounds += (*f)->outer_ccb()->target()->point();
				else		bounds  = (*f)->outer_ccb()->target()->point();
				Pmwx::Ccb_halfedge_circulator iter, stop;
				iter = stop = (*f)->outer_ccb();
				do {
					bounds += iter->target()->point();
					++iter;
				} while (iter != stop);
				has = true;
			}
			for (set<GISHalfedge *>::iterator e = gEdgeSelection.begin(); e != gEdgeSelection.end(); ++e)
			{
				if (has)	bounds += (*e)->target()->point();
				else		bounds  = (*e)->target()->point();
				bounds += (*e)->source()->point();
				has = true;
			}
			for (set<GISVertex *>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
			{
				if (has)	bounds += (*v)->point();
				else		bounds  = (*v)->point();
				has = true;
			}			
			if (has)
			{
				me->mZoomer->ScrollReveal(bounds.p1.x, bounds.p1.y, bounds.p2.x, bounds.p2.y);
			}
		}			
		break;
	}
	WED_MapView_UpdateCommandStatus();
}

void	WED_MapView_UpdateCommandStatus(void)
{
	XPLMCheckMenuItem(sViewMenu, viewCmd_VecMap	     ,sShowMap		? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sViewMenu, viewCmd_Airports	 ,sShowAirports ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sViewMenu, viewCmd_ShowShading ,sShowShading  ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sViewMenu, viewCmd_ShowTensor  ,sShowTensors  ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshPoints  ,sShowMeshPoints ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshLines   ,sShowMeshLines  ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshTrisLo  ,sShowMeshTrisLo ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshTrisHi  ,sShowMeshTrisHi ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshTerrains,sShowMeshAlphas ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sViewMenu, viewCmd_MeshBorders ,sShowMeshBorders?xplm_Menu_Checked : xplm_Menu_Unchecked);


//	XPLMEnableMenuItem(sViewMenu, viewCmd_ZoomSel, !gFaceSelection.empty() || !gEdgeSelection.empty() || !gVertexSelection.empty());
	
	for (int n = 0; n < DEMChoiceCount; ++n) {
		XPLMCheckMenuItem(sDEMMenu, n, (sDEMType == n) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
		XPLMEnableMenuItem(sDEMMenu, n, (n == 0) ? 1 : (gDem.find(kDEMs[n].dem) != gDem.end()));
		
		if (n != 0)
		{
			XPLMCheckMenuItem(sDEMDataMenu, n-1, sShowDEMData[n-1] ? xplm_Menu_Checked : xplm_Menu_Unchecked);
			XPLMEnableMenuItem(sDEMDataMenu, n-1, (gDem.find(kDEMs[n].dem) != gDem.end()));
		}
	}
}


char * WED_MapView::MonitorCaption(void)
{
	static float then = XPLMGetElapsedTime();
	static char buf[1024];
	int n = 0;
	float now = XPLMGetElapsedTime();
	float elapsed = now - then;
	then = now;
	float fps = (elapsed == 0.0) ? 60.0 : 1.0 / elapsed;
	n += sprintf(buf+n, "Framerate: %03d ", (int) fps);

	n += sprintf(buf+n,"Hires: %d ", gTriangulationHi.number_of_faces()/*, gTriangulationLo.number_of_faces()*/);

	
	CDT::Face_handle	recent;
		int	x, y;
		XPLMGetMouseLocation(&x, &y);
		double	lat, lon;
		lat = mZoomer->YPixelToLat(y);
		lon = mZoomer->XPixelToLon(x);	
	
	static int hint_id = CDT::gen_cache_key();
	int i;
	CDT::Locate_type lt;
	recent = gTriangulationHi.locate_cache(CDT::Point(lon,lat), lt, i, hint_id);
	if (lt == CDT::FACE)
	{
		int ts = recent->info().terrain;
		n += sprintf(buf+n, "Tri:%s ", FetchTokenString(ts));

		int slope = acos(recent->info().normal[2]) * RAD_TO_DEG;
		float slope_head_f = recent->info().normal[1];

		float flat_len = sqrt(recent->info().normal[1] * recent->info().normal[1] + recent->info().normal[0] * recent->info().normal[0]);
		if (flat_len != 0.0)
			slope_head_f /= flat_len;
		int slope_head = -asin(slope_head_f) * RAD_TO_DEG + 90.0;		

#if DEBUG_PRINT_NORMALS		
		n += sprintf(buf+n, "S=%d H=%d ", slope, slope_head);
#endif	
#if DEBUG_PRINT_TRI_PARAMS
		n += sprintf(buf+n,"(sd=%.0f,st=%.0f,t=%.1f,tr=%.1f,r=%.0f,h=%.2f) ",
					acos(1.0-recent->info().debug_slope_dem) * RAD_TO_DEG,
					acos(1.0-recent->info().debug_slope_tri) * RAD_TO_DEG,
					recent->info().debug_temp,
					recent->info().debug_temp_range,
					recent->info().debug_rain,
//					-asin(recent->info().debug_heading) * RAD_TO_DEG + 90.0);
					recent->info().debug_heading);
#endif					

#if DEBUG_PRINT_CORNERS
		n += sprintf(buf+n,"%.0f,%.0f,%.0f ",
						recent->vertex(0)->info().height,
						recent->vertex(1)->info().height,
						recent->vertex(2)->info().height);
#endif

#if HACK_CHECK_FLATCALCS
		{
			if (recent->info()->terrain == terrain_Water)
			for (int vi = 0; vi < 3; ++vi)
			{
				DEMGeo& wetPts(gDem[dem_Elevation];
				int xw, yw;
				float e = wetPts.xy_nearest(ffi->vertex(vi)->point().x(),ffi->vertex(vi)->point().y(), xw, yw);
				e = wetPts.get_lowest_heuristic(xw, yw, 5);
			}
			
		}
#endif

#if DEBUG_PRINT_LAYERS
//		PRINT BLENDING LAYERS
		for (set<int>::iterator border = recent->info().terrain_border.begin(); border != recent->info().terrain_border.end(); ++ border)
		{
			n += sprintf(buf+n, "%s ", FetchTokenString(*border));
//			n += sprintf(buf+n, "%s (%f,%f,%f) ", FetchTokenString(*border),
//						recent->vertex(0)->info().border_blend[*border],
//						recent->vertex(1)->info().border_blend[*border],
//						recent->vertex(2)->info().border_blend[*border]);
		}
#endif		
	}		
		
	return buf;
}


void	WED_MapView::SetFlowImage(
							ImageInfo&				image,
							double					bounds[4])
{
	for(int n = 0; n < 4; ++n)
	mFlowBounds[n] = bounds[n];
	if (LoadTextureFromImage(image, mFlowID, tex_Mipmap + tex_Linear, NULL, NULL, &mFlowS, &mFlowT))
		mHasFlow = true;
}
