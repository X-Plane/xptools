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
#include "RF_MapView.h"

#include "GUI_Application.h"

#include "DEMTables.h"
#include "GUI_Fonts.h"
#include "AssertUtils.h"
#include "Forests.h"
#include "RF_MapZoomer.h"
#include "RF_MapTool.h"
#include "ObjTables.h"
#include "AptAlgs.h"
#include "DEMAlgs.h"
#include "RF_Progress.h"
#include "RF_SelectionTool.h"
#include "RF_CropTool.h"
//#include "RF_BezierTestTool.h"
#include "RF_ImageTool.h"
#include "RF_SpecialCommands.h"
#include "RF_TerraTool.h"
//#include "RF_TopoTester.h"
//#include "RF_MeshTester.h"
#include "RF_Selection.h"
#include "RF_Globals.h"
//#include "RF_TriTestTool.h"
#include "RF_Msgs.h"
#include "GISTool_Globals.h"

#include "RF_DEMGraphics.h"

#include "MatrixUtils.h"
#include "MathUtils.h"
#include "ParamDefs.h"
#include "MapAlgs.h"
#include "AptIO.h"
#include "RF_DrawMap.h"

#include "BitmapUtils.h"
#include "TexUtils.h"

#if IBM
#include "XWinGL.h"
#endif

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#define GL_EXT_texture_env_combine 1
#define GL_GLEXT_FUNCTION_POINTERS 1

#if APL
	#include <OpenGL/glext.h>
#else
	#include <GL/glext.h>
#endif

#define TRI_DARKEN 1.0

// Draw mesh borders onto the colored mesh layer.
#define DRAW_MESH_BORDERS 1

// Use this to fade out the border a bit.
#define BORDER_FADE_FACTOR 1.0

// Draw mesh tris with some shading.  Generally not as useful as it would sound.
#define SHADE_TRIS 0

// Print out each border layer on top of a tri stack under mouse
#define DEBUG_PRINT_LAYERS 1

// Print the normal for the tri under the mouse
#define DEBUG_PRINT_NORMALS 1

// Print height at all 3 corners of tri under mouse
#define DEBUG_PRINT_CORNERS 0

// Print water params of tri under mouse
#define DEBUG_PRINT_WAVES 0

// Print input parameters used to pick LU rule for tri under mouse
#define DEBUG_PRINT_TRI_PARAMS 1

// Print out forest type
#define DEBUG_PRINT_FOREST_TYPE 0

RF_MapView *		gMapView = NULL;

static GLdouble		xfrm[16];

inline void xxVertex2d(GLdouble x, GLdouble y)
{
	GLdouble	src[4] = { x, y, 0.0, 1.0 };
	GLdouble	dst[4];
	multMatrixVec(dst,xfrm, src);
	glVertex4dv(dst);
}

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
{		dem_Bathymetry,			"Bathymetry"					,	dem_Elevation,				false,	false,	"MSL=%fm "		},
{		dem_ElevationOverlay,	"Elevation Overlay"				,	dem_Elevation,				false,	false,	"MSL=%fm "		},
//{		dem_OrigLandUse,		"Land Use (Old)"				,	dem_Enum,					false,	true,	"Old LU=%s "	},
{		dem_LandUse,			"Land Use"						,	dem_Enum,					false,	true,	"LU=%s "		},
{		dem_Region,				"Region"						,	dem_Enum,					false,	true,	"Region=%s "	},
{		dem_ParkType,			"Parks"							,	dem_Enum,					false,  true,	"Park=%s"		},
{		dem_ForestType,			"Forests"						,	dem_Zones,					false,  true,	"Forest=%s"		},
{		dem_Climate,			"Climate"						,	dem_Enum,					false,	true,	"Climate=%s "	},
{		dem_Biomass,			"Biomass"						,	dem_Biomass,				true,	false,	"Biomass=%f "	},
{		dem_Rainfall,			"Rainfall"						,	dem_Rainfall,				true,	false,	"Rain=%fmm "	},
{		dem_Temperature,		"Temperature"					,	dem_Temperature,			true,	false,	"Temp=%fC "		},
{		dem_TemperatureSeaLevel,"Sea Level Temperature"			,	dem_TemperatureSeaLevel,	true,	false,	"SL Temp=%fC "	},
{		dem_TemperatureRange,	"Temperature Range"				,	dem_TemperatureRange,		true,	false,	"TempR=%fC "	},
{		dem_UrbanDensity,		"Urban Density"					,	dem_UrbanDensity,			true,	false,	"Density=%f "	},
//{		dem_UrbanPropertyValue,	"Property Values"				,	dem_UrbanPropertyValue,		true,	false,	"$$=%f "		},
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

{		dem_Wizard1,			"Spreadsheet Wizard 1"			,	dem_Zones,					false,	false,	"%fm "			},
{		dem_Wizard2,			"Spreadsheet Wizard 2"			,	dem_Strata,					false,	false,	"%fm "			},
{		dem_Wizard3,			"Spreadsheet Wizard 3"			,	dem_Strata,					false,	false,	"%fm "			},
{		dem_Wizard4,			"Spreadsheet Wizard 4"			,	dem_Strata,					false,	false,	"%fm "			},
{		dem_Wizard5,			"Spreadsheet Wizard 5"			,	dem_Strata,					false,	false,	"%fm "			},
{		dem_Wizard6,			"Spreadsheet Wizard 6"			,	dem_Strata,					false,	false,	"%fm "			},

};

const int DEMChoiceCount = sizeof(kDEMs) / sizeof(DEMViewInfo_t);

GUI_MenuItem_t	kViewItems[] = {
{	"Raster Layer",							0,				0,										0,	viewCmd_DEMChoice		},
{	"Show Shading on Raster Layer",			'S',			gui_ControlFlag,						0,	viewCmd_ShowShading		},
{	"Show Grid lines on Raster Layer",		'G',			gui_ControlFlag,						0,	viewCmd_ShowGrids		},
{	"Show Tensors",							'T',			gui_ControlFlag,						0,	viewCmd_ShowTensor		},
{	"Show Raster Data",						0,				0,										0,	viewCmd_DEMDataChoice	},
{	"Show Extent",							'E',			gui_ControlFlag,						0,	viewCmd_ShowExtent		},
{	"-",									0,				0,										0,	0						},
{	"Recalculate Raster Data Preview",		'R',			gui_ControlFlag,						0,	viewCmd_RecalcDEM		},
{	"Previous Raster",						GUI_KEY_UP,		gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_PrevDEM			},
{	"Next Raster",							GUI_KEY_DOWN,	gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_NextDEM			},
{	"-",									0,				0,										0,	0						},
{	"Vector Map",							'1',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_VecMap			},
{	"Airports",								'2',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_Airports		},
{	"Mesh Points",							'3',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_MeshPoints		},
{	"Mesh Lines",							'4',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_MeshLines		},
{	"Mesh (Hires)",							'5',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_MeshTrisHi		},
{	"Mesh Terrains (Hires)",				'6',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_MeshTerrains	},
{	"-",									0,				0,										0,	0						},
{	"Zoom To Selection",					'/',			gui_ControlFlag + gui_OptionAltFlag,	0,	viewCmd_ZoomSel			},
{	"Zoom To Extent",						'/',			gui_ControlFlag + gui_ShiftFlag,		0,	viewCmd_ZoomExt			},
{	"Zoom To Project",						'/',			gui_ControlFlag,						0,	viewCmd_ZoomProj		},
{	"Zoom On Load",							'L',			gui_ControlFlag,						0,	viewCmd_ZoomLoad		},
{	0,										0,				0,										0,	0						}};

void	RF_MapView::MakeMenus(void)
{

	GUI_Menu view_menu = gApplication->CreateMenu("View", kViewItems,gApplication->GetMenuBar(), 0);

	vector<GUI_MenuItem_t>	dem_menus(DEMChoiceCount+1);
	for(int n = 0; n < DEMChoiceCount; ++n)
	{
		dem_menus[n].name = kDEMs[n].cmdName;
		dem_menus[n].key = (n < 9) ? '1' + n : 0;
		dem_menus[n].flags = gui_ControlFlag;
		dem_menus[n].checked = 0;
		dem_menus[n].cmd = viewCmd_DEMChoice_Start + n;
	}
	dem_menus.back().name = 0;

	gApplication->CreateMenu("DEM Choice", &*dem_menus.begin(),view_menu, 0);

	for(int n = 0; n < DEMChoiceCount; ++n)
		dem_menus[n].cmd = viewCmd_DEMDataChoice_Start + n;

	gApplication->CreateMenu("DEM Data Choice", &*dem_menus.begin(),view_menu, 4);

}

static	int			sDEMType = 0;
static	int			sShowMeshPoints = 1;
static	int			sShowMeshLines  = 1;
static	int			sShowExtent = 1;
static	int			sZoomLoad = 1;

int			sShowMap = 1;
int			sShowMeshTrisHi = 1;
int			sShowMeshAlphas = 1;
int			sShowAirports =1;
int			sShowShading = 1;
int			sShowGrids = 0;
int			sShowTensors = 1;
float		sShadingAzi = 315;
float		sShadingDecl = 45;

static int			sShowDEMData[DEMChoiceCount-1] = { 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0 };


void	RF_MapView_HandleMenuCommand(void *, void *);
void	RF_MapView_HandleDEMMenuCommand(void *, void *);
void	RF_MapView_HandleDEMDataMenuCommand(void *, void *);

int		RF_MapView::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	if(command >= specCmd_Screenshot && command <= specCmd_CheckEnums) return 1;
	switch(command) {
	case viewCmd_DEMChoice:
	case viewCmd_DEMDataChoice:
	case viewCmd_RecalcDEM:
	case viewCmd_PrevDEM:
	case viewCmd_NextDEM:
	case viewCmd_ZoomSel:										return 1;
	case viewCmd_ZoomExt:										return 1;
	case viewCmd_ZoomProj:										return 1;
	case viewCmd_ZoomLoad:		ioCheck = sZoomLoad;			return 1;
	case viewCmd_ShowExtent:	ioCheck = sShowExtent;			return 1;
	case viewCmd_VecMap:		ioCheck = sShowMap;				return 1;
	case viewCmd_Airports:		ioCheck = sShowAirports;		return 1;
	case viewCmd_ShowShading:	ioCheck = sShowShading;			return 1;
	case viewCmd_ShowGrids:		ioCheck = sShowGrids;			return 1;
	case viewCmd_ShowTensor:	ioCheck = sShowTensors;			return 1;
	case viewCmd_MeshPoints:	ioCheck = sShowMeshPoints;		return 1;
	case viewCmd_MeshLines:		ioCheck = sShowMeshLines;		return 1;
	case viewCmd_MeshTrisHi:	ioCheck = sShowMeshTrisHi;		return 1;
	case viewCmd_MeshTerrains:	ioCheck = sShowMeshAlphas;		return 1;
	}

	if(command >= viewCmd_DEMChoice_Start && command < viewCmd_DEMChoice_Stop)
	{
		int n = command - viewCmd_DEMChoice_Start;
		ioCheck = sDEMType == n;
		return n == 0 || (gDem.find(kDEMs[n].dem) != gDem.end());
	}

	if(command >= viewCmd_DEMDataChoice_Start && command < viewCmd_DEMDataChoice_Stop)
	{
		int n = command - viewCmd_DEMDataChoice_Start;
		ioCheck =sShowDEMData[n-1];
		return gDem.find(kDEMs[n].dem) != gDem.end();
	}
	return 0;
}



int		RF_MapView::HandleCommand(int command)
{
	if(command >= specCmd_Screenshot && command <= specCmd_CheckEnums) { HandleSpecialCommand(command); return 1; }

	if(command >= viewCmd_DEMChoice_Start && command < viewCmd_DEMChoice_Stop)
	{
		int n = command - viewCmd_DEMChoice_Start;
		sDEMType = (int) n;
		mNeedRecalcDEM = true;
		return 1;
	}

	if(command >= viewCmd_DEMDataChoice_Start && command < viewCmd_DEMDataChoice_Stop)
	{
		int n = command - viewCmd_DEMDataChoice_Start;
		sShowDEMData[n-1] = 1 - sShowDEMData[n-1];
		return 1;
	}
	int i;
	switch(command) {
	case viewCmd_PrevDEM:
		for(i = 0; i < DEMChoiceCount; ++i)
		{
			sDEMType--;
			if (sDEMType < 0) sDEMType = DEMChoiceCount-1;
			if (gDem.count(kDEMs[sDEMType].dem))
				break;
		}
		mNeedRecalcDEM = true;
		return 1;
	case viewCmd_NextDEM:
		for(i = 0; i < DEMChoiceCount; ++i)
		{
			sDEMType++;
			if (sDEMType >= DEMChoiceCount) sDEMType = 0;
			if (gDem.count(kDEMs[sDEMType].dem))
				break;
		}
		mNeedRecalcDEM = true;
		return 1;
	case viewCmd_RecalcDEM:
		mNeedRecalcDEM = true;
		mNeedRecalcRelief = true;
		return 1;
	case viewCmd_VecMap:		sShowMap = !sShowMap;				return 1;
	case viewCmd_Airports:		sShowAirports = !sShowAirports;		return 1;
	case viewCmd_ShowShading:	sShowShading = !sShowShading;		return 1;
	case viewCmd_ShowGrids:		sShowGrids = !sShowGrids;			return 1;
	case viewCmd_ShowTensor:	sShowTensors = !sShowTensors;		return 1;
	case viewCmd_MeshPoints:	sShowMeshPoints = !sShowMeshPoints;	return 1;
	case viewCmd_MeshLines:		sShowMeshLines = !sShowMeshLines;	return 1;
//	case viewCmd_MeshTrisLo:	sShowMeshTrisLo = !sShowMeshTrisLo;	return 1;
	case viewCmd_MeshTrisHi:	sShowMeshTrisHi = !sShowMeshTrisHi;	return 1;
	case viewCmd_MeshTerrains:	sShowMeshAlphas = !sShowMeshAlphas;	return 1;
//	case viewCmd_MeshBorders:	sShowMeshBorders = !sShowMeshBorders;	return 1;
	case viewCmd_ShowExtent:	sShowExtent = !sShowExtent;			return 1;
	case viewCmd_ZoomLoad:		sZoomLoad = !sZoomLoad;				return 1;
	case viewCmd_ZoomSel:
	case viewCmd_ZoomExt:
	case viewCmd_ZoomProj:
		{
			Bbox2	bounds;
			
			if(command == viewCmd_ZoomSel)
			{
				for (set<Face_handle>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
				if (!(*f)->is_unbounded())
				{
					bounds += cgal2ben((*f)->outer_ccb()->target()->point());
					Pmwx::Ccb_halfedge_circulator iter, stop;
					iter = stop = (*f)->outer_ccb();
					do {
						bounds += cgal2ben(iter->target()->point());
						++iter;
					} while (iter != stop);
				}
				for (set<Halfedge_handle>::iterator e = gEdgeSelection.begin(); e != gEdgeSelection.end(); ++e)
				{
					bounds += cgal2ben((*e)->target()->point());
					bounds += cgal2ben((*e)->source()->point());
				}
				for (set<Vertex_handle>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
				{
					bounds += cgal2ben((*v)->point());
				}
			}
			if(command == viewCmd_ZoomExt)
			{
				bounds = Bbox2(gMapWest,gMapSouth,gMapEast,gMapNorth);
			}
			if(command == viewCmd_ZoomProj)
			{
				for(DEMGeoMap::iterator i = gDem.begin(); i != gDem.end(); ++i)
					bounds += Bbox2(i->second.mWest,i->second.mSouth,i->second.mEast,i->second.mNorth);
			}
			if (!bounds.is_null())
			{
				mZoomer->ScrollReveal(bounds.p1.x(), bounds.p1.y(), bounds.p2.x(), bounds.p2.y());
			}
		}
		return 1;
	}
	return 0;
}



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
	Point2 p1(cgal2ben(f->vertex(0)->point()));
	Point2 p2(cgal2ben(f->vertex(1)->point()));
	Point2 p3(cgal2ben(f->vertex(2)->point()));
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

	Point2 p1(cgal2ben(a->point()));
	Point2 p2(cgal2ben(b->point()));
	for (int n = 0; n < bucket_count; ++n)
	{
		if (buckets[n].contains(p1) &&
			buckets[n].contains(p2))
		return n;
	}
	return bucket_count;
}

RF_MapView::RF_MapView(GUI_Commander * cmdr) : GUI_Commander(cmdr),
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

	glGenTextures(1, &mTexID);
	glGenTextures(1, &mReliefID);
	glGenTextures(1, &mFlowID);
	mHasTex = false;
	mHasRelief = false;
	mHasFlow = false;

	mCurTool = 0;
	mZoomer = new RF_MapZoomer;

	mTools.push_back(new RF_SelectionTool(mZoomer));
	mTools.push_back(new RF_CropTool(mZoomer));
//	mTools.push_back(new RF_BezierTestTool(mZoomer));
	mTools.push_back(new RF_ImageTool(mZoomer));
	mTools.push_back(new RF_TerraTool(mZoomer));
//	mTools.push_back(new RF_TopoTester(mZoomer));
//	mTools.push_back(new RF_MeshTester(mZoomer));

//	XPCreateTab(50, 150, 300, 100, 1, "Tab A;Tab B;Tab C;Tab D", GetWidget());
}

RF_MapView::~RF_MapView()
{
	if (mDLMeshLine != 0)	glDeleteLists(mDLMeshLine, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);
	if (mDLMeshFill != 0)	glDeleteLists(mDLMeshFill, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);

	delete mZoomer;
	for(int n = 0; n < mTools.size(); ++n)
		delete mTools[n];
}

void		RF_MapView::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	mZoomer->SetPixelBounds(inBounds[0], inBounds[1], inBounds[2], inBounds[3]);
}

/***************************************************************************************************************************************
 * MAP DRAWING
 ***************************************************************************************************************************************/

static void FontDrawDarkBox(
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				float							inWidth,
				const char *					inString)
{
	const char * sb = inString;
	const char * se = inString + strlen(sb);

	float des = GUI_GetLineDescent(inFontID);
	float lh  = GUI_GetLineHeight(inFontID);
	
	while(sb < se)
	{
		int chars_this_line = GUI_FitForward(inFontID, sb, se, inWidth);

		const char * sl = sb + chars_this_line;
	
		float x1 = inX - 3;
		float x2 = inX + GUI_MeasureRange(inFontID, sb, sl) + 3;
		float y1 = inY - des - 3;
		float y2 = inY - des + lh + 3;

		inState->SetState(false, 0, false, true, true, false, false);
		glColor4f(0,0,0,0.5);

		glBegin(GL_QUADS);
		glVertex2f(x1,y1);
		glVertex2f(x1,y2);
		glVertex2f(x2,y2);
		glVertex2f(x2,y1);
		glEnd();


		GUI_FontDrawScaled(inState, inFontID, color, 
			inX,
			inY - des,
			inX,
			inY - des  + lh,
			sb, sl,
			align_Left);

		inY -= lh;

		sb = sl;
	}
}

void	RF_MapView::Draw(GUI_GraphState * state)
{
	/***************************************************************************************************************************************
	 * BASE LAYER, UNDERLAY AND DECALS
	 ***************************************************************************************************************************************/

	int	l, t, r, b;
	int lbrt[4];
	GetBounds(lbrt);
	l=lbrt[0];
	b=lbrt[1];
	r=lbrt[2];
	t=lbrt[3];
	RF_MapTool * cur = CurTool();
	char * status = NULL;;
	if (cur)
	{
		int	mx, my;
		GetMouseLocNow(&mx, &my);
		status = cur->GetStatusText(mx,my);
	}
	char * mon = MonitorCaption();

	/* TODO - map background */
	state->SetState(false,0,false, false, false,   false, false);
	glColor3f(0,0,0);
	glBegin(GL_QUADS);
	glVertex2i(l,b);
	glVertex2i(l,t);
	glVertex2i(r,t);
	glVertex2i(r,b);
	glEnd();

	double	pl, pb, pr, pt;
	double	ll, lb, lr, lt;
	mZoomer->GetPixelBounds(pl, pb, pr, pt);
	mZoomer->GetMapVisibleBounds(ll, lb, lr, lt);

	state->SetState(0, 0, 0,  0, 0,  0, 0);
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
		mTools[n]->DrawFeedbackUnderlay(state,mTools[n] == cur);

	/***************************************************************************************************************************************
	 * PRECALCULATE DISPLAY LISTS, ETC IF INVALID
	 ***************************************************************************************************************************************/


	// Recalc the map before we nuke the coordinate system, or the progress func will not be visible.
	if (sShowMap)
	{
		if (mNeedRecalcMapFull)
		{
//			RF_ProgressFunc(0, 1, "Building graphics for vector map...", 0.0);
//			gMap.Index();
//			RF_ProgressFunc(0, 1, "Building graphics for vector map...", 0.5);
			PrecalcOGL(gMap,RF_ProgressFunc);
//			RF_ProgressFunc(0, 1, "Building graphics for vector map...", 1.0);
		}
		else if (mNeedRecalcMapMeta)
		{
//			RF_ProgressFunc(0, 1, "Updating graphics for vector map...", 0.0);
			RecalcOGLColors(gMap,RF_ProgressFunc);
//			RF_ProgressFunc(0, 1, "Updating graphics for vector map...", 1.0);
		}

		mNeedRecalcMapMeta = mNeedRecalcMapFull = false;
	}


	if (sShowMeshTrisHi)
	{
		if (mNeedRecalcMeshHi)
		{
			RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", 0.0);

			if (mDLMeshLine != 0)	glDeleteLists(mDLMeshLine, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);
			mDLMeshLine = glGenLists(MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);

			for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
			{
				RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", (float) n / (float) (MESH_BUCKET_SIZE*MESH_BUCKET_SIZE));
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
						if(gTriangulationHi.is_flipable(eit->first,eit->second))
							color[0] = 0.8, color[1] = 0.4, color[2] = 0.4, color[3] = 0.8;						
						else if (sDEMType)
							color[0] = 0.0, color[1] = 0.0, color[2] = 0.5, color[3] = 0.8;
						else
							color[0] = 0.4, color[1] = 0.4, color[2] = 0.8, color[3] = 0.8;
					} else
						if(eit->first->info().get_edge_feature(eit->second))
							color[0] = 1.0, color[1] = 0.4, color[2] = 0.2, color[3] = 1.0;												

					CDT::Vertex_handle	a = eit->first->vertex(eit->first->ccw(eit->second));
					CDT::Vertex_handle	b = eit->first->vertex(eit->first->cw(eit->second));

					CDT::Point	p1 = a->point();
					CDT::Point p2 = b->point();

					glColor4fv(color);
					glVertex2f(CGAL::to_double(p1.x()), CGAL::to_double(p1.y()));
					glColor4fv(color);
					glVertex2f(CGAL::to_double(p2.x()), CGAL::to_double(p2.y()));
				}

				glEnd();
				glEndList();
			}
			mNeedRecalcMeshHi = false;
			RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh line preview...", 1.0);

		}
	}

	if (sShowMeshAlphas)
	{
		glDisable(GL_CULL_FACE);

		if (mNeedRecalcMeshHiAlpha)
		{
			RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", 0.0);

			if (mDLMeshFill != 0)	glDeleteLists(mDLMeshFill, MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);
			mDLMeshFill = glGenLists(MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1);

			for (int n = 0; n <= MESH_BUCKET_SIZE * MESH_BUCKET_SIZE; ++n)
			{
				RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", (float) n / (float) (MESH_BUCKET_SIZE*MESH_BUCKET_SIZE));

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
							col[0] *= TRI_DARKEN;
							col[1] *= TRI_DARKEN;
							col[2] *= TRI_DARKEN;
							col[3] = 1.0;
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
						
						#if SHADE_TRIS
						{
							float	nrm_z    = sin(sShadingDecl * DEG_TO_RAD);
							float	scale_xy = cos(sShadingDecl * DEG_TO_RAD);

							float	nrm_x = scale_xy * sin(sShadingAzi * DEG_TO_RAD);
							float	nrm_y = scale_xy * cos(sShadingAzi * DEG_TO_RAD);

							float scale = nrm_x * fit->info().normal[0] +
										  nrm_y * fit->info().normal[1] +
										  nrm_z * fit->info().normal[2];
							scale = max(scale,0.0f);
										  		  
							col[0] *= scale;
							col[1] *= scale;
							col[2] *= scale;
						}
						#endif
						

						glColor4fv(col);					glVertex2f(CGAL::to_double(p1.x()), CGAL::to_double(p1.y()));
						glColor4fv(col);					glVertex2f(CGAL::to_double(p2.x()), CGAL::to_double(p2.y()));
						glColor4fv(col);					glVertex2f(CGAL::to_double(p3.x()), CGAL::to_double(p3.y()));

#if DRAW_MESH_BORDERS
						for (set<int>::iterator i = fit->info().terrain_border.begin(); i != fit->info().terrain_border.end(); ++i)
						{
							GetNaturalTerrainColor(*i, col);
							col[0] *= TRI_DARKEN;
							col[1] *= TRI_DARKEN;
							col[2] *= TRI_DARKEN;

							col[3] = BORDER_FADE_FACTOR * fit->vertex(2)->info().border_blend[*i];
							glColor4fv(col);
							glVertex2f(CGAL::to_double(p1.x()), CGAL::to_double(p1.y()));

							col[3] = BORDER_FADE_FACTOR * fit->vertex(1)->info().border_blend[*i];
							glColor4fv(col);
							glVertex2f(CGAL::to_double(p2.x()), CGAL::to_double(p2.y()));

							col[3] = BORDER_FADE_FACTOR * fit->vertex(0)->info().border_blend[*i];
							glColor4fv(col);
							glVertex2f(CGAL::to_double(p3.x()), CGAL::to_double(p3.y()));
						}
#endif
					}
				}

				glEnd();
				glEndList();
			}
			mNeedRecalcMeshHiAlpha = false;
			RF_ProgressFunc(0, 1, "Updating graphics for terrain mesh colored preview...", 1.0);
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
	
	GLdouble	m1[16], scale[16];
	setIdentityMatrix(m1);

	applyTranslation(m1,pl,pb,0.0);
	
	setIdentityMatrix(scale);
	m1[0] = pw / lw;
	m1[5] = ph / lh;
	multMatrices(xfrm, m1, scale);

	applyTranslation(xfrm,-ll, -lb, 0.0);
		
	
//	glTranslated(pl, pb, 0.0);
//	glScaled(pw / lw, ph / lh, 1.0);
//	glTranslated(-ll, -lb, 0.0);

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
		state->SetState(0, do_relief ? 2 : 1, 0,   0, 0,  0, 0);
		state->BindTex(mTexID, do_relief ? 1 : 0);
		if (do_relief)
		{
			state->BindTex(mReliefID, 0);
			SetupNormalShading();
		}
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		if (do_relief)
		{
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0  , 0.0  );glTexCoord2f(0.0     , 0.0     );		xxVertex2d((mDEMBounds[0]), (mDEMBounds[1]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0  , mTexT);glTexCoord2f(0.0     , mReliefT);		xxVertex2d((mDEMBounds[0]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexS, mTexT);glTexCoord2f(mReliefS, mReliefT);		xxVertex2d((mDEMBounds[2]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexS, 0.0  );glTexCoord2f(mReliefS, 0.0     );		xxVertex2d((mDEMBounds[2]), (mDEMBounds[1]));
		} else {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  , 0.0  );xxVertex2d((mDEMBounds[0]), (mDEMBounds[1]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  , mTexT);xxVertex2d((mDEMBounds[0]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mTexS, mTexT);xxVertex2d((mDEMBounds[2]), (mDEMBounds[3]));
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mTexS, 0.0  );xxVertex2d((mDEMBounds[2]), (mDEMBounds[1]));
		}
		glEnd();

		if (mHasRelief)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

	}

	if(mHasFlow && sShowTensors)
	{
		state->SetState(0, 11, 0,   0, 0,  0, 0);
		state->BindTex(mFlowID, 0);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  ,  0.0   );xxVertex2d((mFlowBounds[0]), (mFlowBounds[1]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0  ,  mFlowT);xxVertex2d((mFlowBounds[0]), (mFlowBounds[3]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mFlowS, mFlowT);xxVertex2d((mFlowBounds[2]), (mFlowBounds[3]));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, mFlowS, 0.0   );xxVertex2d((mFlowBounds[2]), (mFlowBounds[1]));
		glEnd();
	}

	/***************************************************************************************************************************************
	 * VECTOR MAP
	 ***************************************************************************************************************************************/

	if (sShowMap)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixd(xfrm);
		
		DrawMapBucketed(state, gMap,
			ll, lb, lr, lt,
//			pl, pb, pr, pt,
			gVertexSelection,
			gEdgeSelection,
			gFaceSelection,
			gPointFeatureSelection);
			
		glPopMatrix();
	}

	/***************************************************************************************************************************************
	 * DEBUG: PTS AND LINES
	 ***************************************************************************************************************************************/

	state->SetState(0, 0, 0,    0, 1,    0, 0);

	if (sShowMeshPoints)
	{
		glPointSize(3);
		glBegin(GL_POINTS);
		for (vector<pair<Point2, Point3> >::iterator i = gMeshPoints.begin(); i != gMeshPoints.end(); ++i)
		{
			glColor3f(i->second.x, i->second.y, i->second.z);
			xxVertex2d((i->first.x()),
					   (i->first.y()));
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
			xxVertex2d((i->first.x()),
					   (i->first.y()));
		}
		glEnd();
		
		for(vector<pair<Bezier2, pair<Point3, Point3> > >::iterator i = gMeshBeziers.begin(); i != gMeshBeziers.end(); ++i)
		{	
			glBegin(GL_LINE_STRIP);
			for(double t = 0.0; t <= 1.0; t += 0.0625)
			{
				glColor4f(
					interp(0,i->second.first.x,1,i->second.second.x,t),
					interp(0,i->second.first.y,1,i->second.second.y,t),
					interp(0,i->second.first.z,1,i->second.second.z,t),
					0.5);
				Point2 mp(i->first.midpoint(t));
				xxVertex2d(mp.x(),mp.y());				
			}
			glEnd();
		}
		
		glLineWidth(1);
	}
	
	if(sShowGrids)
	{
		int my_dem = kDEMs[sDEMType].dem;
		if(gDem.count(my_dem))
		{
			DEMGeo& d(gDem[my_dem]);
			glColor4f(1,1,1,0.2);
			glBegin(GL_LINES);
			for(int x = 0; x <= (d.mWidth - d.mPost); ++x)
			{
				double ew = d.x_to_lon_double((double) x - d.pixel_offset());
				xxVertex2d(ew,d.mSouth);
				xxVertex2d(ew,d.mNorth);
			}
			for(int y = 0; y <= (d.mHeight - d.mPost); ++y)			
			{
				double ns = d.y_to_lat_double((double) y - d.pixel_offset());
				xxVertex2d(d.mEast,ns);
				xxVertex2d(d.mWest,ns);
			}
			glEnd();
		}
	}
	
	if (sShowExtent)
	{
		glColor3f(1,1,0);
		glBegin(GL_LINE_LOOP);
		xxVertex2d(gMapWest,gMapSouth);
		xxVertex2d(gMapEast,gMapSouth);
		xxVertex2d(gMapEast,gMapNorth);
		xxVertex2d(gMapWest,gMapNorth);
		glEnd();
	}

	/***************************************************************************************************************************************
	 * MESH AS LINES AND TERRAIN (DISPLAY LISTS - EASY)
	 ***************************************************************************************************************************************/

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixd(xfrm);

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

	glPopMatrix();

	/***************************************************************************************************************************************
	 * AIRPORTS
	 ***************************************************************************************************************************************/

	if (sShowAirports)
	{
		double e, s, n, w;
		mZoomer->GetMapVisibleBounds(w, s, e, n);
		Bbox2	vis_area(w, s, e, n);
		state->SetState(0, 0, 0, 1, 1, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
//				xxVertex2d((gApts[n].bounds.xmin()),
//							(gApts[n].bounds.ymin()));
//				xxVertex2d((gApts[n].bounds.xmin()),
//							(gApts[n].bounds.ymax()));
//				xxVertex2d((gApts[n].bounds.xmax()),
//							(gApts[n].bounds.ymax()));
//				xxVertex2d((gApts[n].bounds.xmax()),
//							(gApts[n].bounds.ymin()));

				for(vector<AptInfo_t::AptLineLoop_t>::iterator ogl = gApts[n].ogl.begin(); ogl != gApts[n].ogl.end(); ++ogl)
				{
					glColor3fv(ogl->rgb);
					glBegin(GL_LINE_LOOP);
					for(vector<Point2>::iterator pt = ogl->pts.begin(); pt != ogl->pts.end(); ++pt)
						xxVertex2d(pt->x(),pt->y());
					glEnd();
				}
				glPointSize(3);
				glBegin(GL_POINTS);
				for(vector<AptInfo_t::AptLineLoop_t>::iterator ogl = gApts[n].ogl.begin(); ogl != gApts[n].ogl.end(); ++ogl)
				{
					glColor3fv(ogl->rgb);
					for(vector<Point2>::iterator pt = ogl->pts.begin(); pt != ogl->pts.end(); ++pt)
						xxVertex2d(pt->x(),pt->y());
				}
				glEnd();
				glPointSize(1);
//				glEnd();
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				float col[3] = { 1.0, 1.0, 1.0 };
//				XPLMDrawString(col, (gApts[n].bounds.xmax()),(gApts[n].bounds.ymin()),
//														gApts[n].icao.c_str(), NULL, xplmFont_Basic);

//				state->SetState(0, 0, 0, 1, 1, 0, 0);
//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
		}
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
					xxVertex2d(	(p1.x()),
								(p1.y()));
					xxVertex2d(	(p2.x()),
								(p2.y()));
				}
				if (fit->info().beaches[1] != -1)
				{
					xxVertex2d(	(p1.x()),
								(p1.y()));
					xxVertex2d(	(p3.x()),
								(p3.y()));
				}
				if (fit->info().beaches[2] != -1)
				{
					xxVertex2d(	(p2.x()),
								(p2.y()));
					xxVertex2d(	(p3.x()),
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

			xxVertex2d(	(p1.x()),
						(p1.y()));
			xxVertex2d(	(p2.x()),
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

				xxVertex2d(	(p1.x()),
							(p1.y()));
				xxVertex2d(	(p2.x()),
							(p2.y()));
				xxVertex2d(	(p3.x()),
							(p3.y()));

			}
		}
		glEnd();
	}
#endif

	/***************************************************************************************************************************************
	 * MISC CLEANUP AND CAPTIONS
	 ***************************************************************************************************************************************/

	for (int n = 0; n < mTools.size(); ++n)
		mTools[n]->DrawFeedbackOverlay(state,mTools[n] == cur);

	glDisable(GL_SCISSOR_TEST);

		int w = 10, h = GUI_GetLineHeight(font_UI_Basic);
		static GLfloat	white[4] = { 1.0, 1.0, 1.0, 1.0 };

	if (mon)
	{
		FontDrawDarkBox(state, font_UI_Basic, white, l + 7, b + 80 + 1, r - l - 350, mon);
	}
	if (status)
	{
		GUI_FontDraw(state, font_UI_Basic, white, l + 7, b + 7, status);
	}

	{
		char buf[1024];
		int	x, y;
		GetMouseLocNow(&x, &y);
		double	lat, lon;
		lat = mZoomer->YPixelToLat(y);
		lon = mZoomer->XPixelToLon(x);


		int	k = t - 30;

		for (int n = 0; n < DEMChoiceCount; ++n)
		{
			if (n == 0)
			{
				sprintf(buf, "Viewing: %s", kDEMs[sDEMType].cmdName);
				FontDrawDarkBox(state, font_UI_Basic, white, l + 5, k, 9999, buf);
				k -= (h+1);
			}
			else if (sShowDEMData[n-1] || n == sDEMType)
			{
				if (gDem.count(	kDEMs[n].dem ))
				{
					float hh = gDem[kDEMs[n].dem].xy_nearest_raw(lon, lat);

					// HACK city - for certain DEMs, do the trig on the fly.

					if (kDEMs[n].dem == dem_Slope)
						hh = RAD_TO_DEG * acos(1.0 - hh);
					else if (kDEMs[n].dem == dem_SlopeHeading)
						hh = RAD_TO_DEG * acos(hh);

					if (kDEMs[n].is_enum)
						sprintf(buf,kDEMs[n].format_string,FetchTokenString(hh));
					else
						sprintf(buf,kDEMs[n].format_string,hh);
					FontDrawDarkBox(state,font_UI_Basic, white, l + 5, k, 9999,buf);
					k -= (h+1);
				}
			}
		}
		
		k -= 10;
		
		if(gFaceSelection.size() == 1)
		{
			Pmwx::Face_handle f = *gFaceSelection.begin();
			for(GISParamMap::iterator i = f->data().mParams.begin(); i != f->data().mParams.end(); ++i)
			{
				if(i->second == floor(i->second) && i->second >= 0 && i->second < gTokens.size())
				{
					sprintf(buf,"%s: %s (%d)", FetchTokenString(i->first), FetchTokenString(floor(i->second)), (int) floor(i->second));
				}
				else
					sprintf(buf,"%s: %lf", FetchTokenString(i->first), i->second);				
				FontDrawDarkBox(state, font_UI_Basic, white, l+5,k,9999, buf);
				k -= (h+1);
			}
			
			for(GISPointFeatureVector::iterator i = f->data().mPointFeatures.begin(); i != f->data().mPointFeatures.end(); ++i)
			{
				sprintf(buf,"%s",FetchTokenString(i->mFeatType));
				FontDrawDarkBox(state, font_UI_Basic, white, l+5,k,9999, buf);
				k -= (h+1);
			}

			for(GISPolygonFeatureVector::iterator i = f->data().mPolygonFeatures.begin(); i != f->data().mPolygonFeatures.end(); ++i)
			{
				sprintf(buf,"%s",FetchTokenString(i->mFeatType));
				FontDrawDarkBox(state, font_UI_Basic, white, l+5,k,9999, buf);
				k -= (h+1);
			}

			for(GISObjPlacementVector::iterator i = f->data().mObjs.begin(); i != f->data().mObjs.end(); ++i)
			{
				sprintf(buf,"%s",FetchTokenString(i->mRepType));
				FontDrawDarkBox(state, font_UI_Basic, white, l+5,k,9999, buf);
				k -= (h+1);
			}

			for(GISPolyObjPlacementVector::iterator i = f->data().mPolyObjs.begin(); i != f->data().mPolyObjs.end(); ++i)
			{
				sprintf(buf,"%s",FetchTokenString(i->mRepType));
				FontDrawDarkBox(state, font_UI_Basic, white, l+5,k,9999, buf);
				k -= (h+1);
			}
		}				
	}

	const char * nat = QuickToFile(gNaturalTerrainFile);
	const char * obj = QuickToFile(gObjPlacementFile);

	FontDrawDarkBox(state,font_UI_Basic,white, r - strlen(nat) * w - 20, t - 30, 9999,nat);

	FontDrawDarkBox(state,font_UI_Basic,white, r - gLanduseTransFile.size() * w - 20, t - 50, 9999,gLanduseTransFile.c_str());

	FontDrawDarkBox(state,font_UI_Basic,white, r - gNaturalTerrainFile.size() * w - 20, t - 30, 9999,gNaturalTerrainFile.c_str());

	FontDrawDarkBox(state,font_UI_Basic,white, r - gReplacementClimate.size() * w - 20, t - 70, 9999,gReplacementClimate.c_str());

	FontDrawDarkBox(state,font_UI_Basic,white, r - gReplacementRoads.size() * w - 20, t - 90, 9999,gReplacementRoads.c_str());

	FontDrawDarkBox(state,font_UI_Basic,white, r - strlen(obj) * w - 20, t - 120, 9999,obj);


	char	buf[50];
	int	x, y;
	int	lat, lon;
	GetMouseLocNow(&x, &y);
	lat = mZoomer->YPixelToLat(y) * 1000000.0;
	lon = mZoomer->XPixelToLon(x) * 1000000.0;
	char	ns = 'N';
	char	ew = 'E';
	if (lat < 0) { lat = -lat; ns = 'S'; }
	if (lon < 0) { lon = -lon; ew = 'W'; }
	sprintf(buf,"Lat: %02d.%06d%c Lon: %03d.%06d%c",
		lat / 1000000, lat % 1000000, ns,
		lon / 1000000, lon % 1000000, ew);
	FontDrawDarkBox(state,font_UI_Basic,white, r - 300, b + 20, 9999,buf);
}



/***************************************************************************************************************************************
 * MAP INTERACTION
 ***************************************************************************************************************************************/

int		RF_MapView::MouseMove(int x, int y			  )
{
	return 0;
}

int		RF_MapView::MouseDown(int x, int y, int button)
{
	RF_MapTool * cur = CurTool();
	if (cur && cur->HandleClick(xplm_MouseDown, x, y, button, GetModifiersNow())) { return 1; }

	mOldX = x;
	mOldY = y;
}

void	RF_MapView::MouseDrag(int x, int y, int button)
{
	RF_MapTool * cur = CurTool();
	if (cur && cur->HandleClick(xplm_MouseDrag, x, y, button, GetModifiersNow())) return;

	mZoomer->PanPixels(mOldX, mOldY, x, y);
	mOldX = x;
	mOldY = y;
}

void	RF_MapView::MouseUp  (int x, int y, int button)
{
	RF_MapTool * cur = CurTool();
	if (cur && cur->HandleClick(xplm_MouseUp, x, y, button, GetModifiersNow()))
		return;
	mZoomer->PanPixels(mOldX, mOldY, x, y);
}

int		RF_MapView::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int		RF_MapView::ScrollWheel(int x, int y, int direction, int axis)
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

RF_MapTool * RF_MapView::CurTool(void)
{
	if (mCurTool < 0 || mCurTool >= mTools.size()) return NULL;
	return mTools[mCurTool];
}

void	RF_MapView::HandleNotification(int catagory, int message, void * param)
{
	int n;
	switch(catagory) {
	case rf_Cat_File:
		switch(message) {
		case rf_Msg_FileLoaded:
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
					Point_2 sw, ne;
					CalcBoundingBox(gMap, sw, ne);
					full.p1 = cgal2ben(sw);
					full.p2 = cgal2ben(ne);
					mZoomer->SetMapLogicalBounds(
								(full.p1.x()),
								(full.p1.y()),
								(full.p2.x()),
								(full.p2.y()));
					mZoomer->SetAspectRatio(1.0 / cos((CGAL::to_double(full.p1.y()) + CGAL::to_double(full.p2.y())) * 0.5 * PI / 180.0));
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
				if(sZoomLoad)
					mZoomer->ZoomShowAll();

				for (int y = 0; y < MESH_BUCKET_SIZE; ++y)
				for (int x = 0; x < MESH_BUCKET_SIZE; ++x)
				{
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p1.x_ = full.p1.x() + (full.p2.x() - full.p1.x()) * (float) (x  ) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p1.y_ = full.p1.y() + (full.p2.y() - full.p1.y()) * (float) (y  ) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p2.x_ = full.p1.x() + (full.p2.x() - full.p1.x()) * (float) (x+1) / (float) MESH_BUCKET_SIZE;
					mDLBuckets[x + y * MESH_BUCKET_SIZE].p2.y_ = full.p1.y() + (full.p2.y() - full.p1.y()) * (float) (y+1) / (float) MESH_BUCKET_SIZE;
				}
			}
			break;
		case rf_Msg_RasterChange:
			mNeedRecalcDEM = true;
			mNeedRecalcRelief = true;
			break;
		case rf_Msg_VectorChange:
			mNeedRecalcMapFull = true;
			break;
		case rf_Msg_VectorMetaChange:
			mNeedRecalcMapMeta = true;
			break;
		case rf_Msg_TriangleHiChange:
			mNeedRecalcMeshHi = true;
			mNeedRecalcMeshHiAlpha = true;
			break;
		case rf_Msg_AirportsLoaded:
			break;
		}
		break;
	case rf_Cat_Selection:
		switch(message) {
		case rf_Msg_SelectionModeChanged:
			{
			}
			break;
		}
		break;
	}
}


bool	RF_MapView::RecalcDEM(bool do_relief)
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

	const DEMGeo *	master = &gDem[param];
	DEMGeo	resized;
	
	GLint maxDim;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxDim);
	if(maxDim > 4096) maxDim = 4096;

	int want_x = min(maxDim, master->mWidth);
	int want_y = min(maxDim, master->mHeight);
	if(want_x != master->mWidth || want_y != master->mHeight)
	{
		resized.resize(want_x,want_y);
		resized.copy_geo_from(*master);
		resized.mPost = master->mPost;
		ResampleDEM(*master, resized);
		master = &resized;
	}

	if (DEMToBitmap(*master, image, mode) == 0)
	{
		mDEMBounds[0] = master->x_to_lon_double(-0.5);
		mDEMBounds[1] = master->y_to_lat_double(-0.5);
		mDEMBounds[2] = master->x_to_lon_double((double) master->mWidth - 0.5);
		mDEMBounds[3] = master->y_to_lat_double((double) master->mHeight - 0.5);

		if (LoadTextureFromImage(image, mTexID, tex_Mipmap + (nearest ? 0 : tex_Linear), NULL, NULL, &mTexS, &mTexT))
		{
			mHasTex = true;
		}
		DestroyBitmap(&image);
		
		if(mode == dem_Elevation)
		{
			if (DEMToBitmap(*master, image, dem_Normals) == 0)
			{
				if (LoadTextureFromImage(image, mReliefID, tex_Mipmap + (tex_Linear), NULL, NULL, &mReliefS, &mReliefT))
				{
					mHasRelief = true;
				}
				DestroyBitmap(&image);			
				do_relief = false;
			}
		}
	}

	if (do_relief)
	{
		if (gDem.count(dem_Elevation) == 0)
			mHasRelief = false;
		else {
			const DEMGeo *	master = &gDem[dem_Elevation];
			DEMGeo resized;
			int want_x = min(maxDim, master->mWidth);
			int want_y = min(maxDim, master->mHeight);
			if(want_x != master->mWidth || want_y != master->mHeight)
			{
				resized.resize(want_x,want_y);
				resized.copy_geo_from(*master);
				resized.mPost = master->mPost;
				ResampleDEM(*master, resized);
				master = &resized;
			}

			if (DEMToBitmap(*master, image, dem_Normals) == 0)
			{
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

char * RF_MapView::MonitorCaption(void)
{
	static float then = (float) clock() / float (CLOCKS_PER_SEC);
	static char buf[4096];
	int n = 0;
	float now = (float) clock() / float (CLOCKS_PER_SEC);
	float elapsed = now - then;
	then = now;
	float fps = (elapsed == 0.0) ? 60.0 : 1.0 / elapsed;
	n += sprintf(buf+n, "Framerate: %03d ", (int) fps);

	n += sprintf(buf+n,"Hires: %llu ", (unsigned long long)gTriangulationHi.number_of_faces()/*, (unsigned long long)gTriangulationLo.number_of_faces()*/);


	CDT::Face_handle	recent;
		int	x, y;
		GetMouseLocNow(&x, &y);
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
		n += sprintf(buf+n,"(sd=%.0f,st=%.0f,t=%.1f,tr=%.1f,r=%.0f,h=%.2f,re=%.2f,er=%.2f,%s) original: %s ",
					acos(1.0-recent->info().debug_slope_dem) * RAD_TO_DEG,
					acos(1.0-recent->info().debug_slope_tri) * RAD_TO_DEG,
					recent->info().mesh_temp,
					recent->info().debug_temp_range,
					recent->info().mesh_rain,
//					-asin(recent->info().debug_heading) * RAD_TO_DEG + 90.0);
					recent->info().debug_heading,
//					FetchTokenString(recent->info().debug_lu[0]),
//					FetchTokenString(recent->info().debug_lu[1]),
//					FetchTokenString(recent->info().debug_lu[2]),
//					FetchTokenString(recent->info().debug_lu[3]),
					recent->info().debug_re,
					recent->info().debug_er,
					FetchTokenString(recent->info().debug_lu[4]),
					FetchTokenString(recent->info().debug_terrain_orig));
#endif

#if DEBUG_PRINT_FOREST_TYPE
	n += sprintf(buf+n,"(%s) ", FetchTokenString(tri_forest_type(recent)));
#endif

#if DEBUG_PRINT_CORNERS
		n += sprintf(buf+n,"%.0f,%.0f,%.0f ",
						recent->vertex(0)->info().height,
						recent->vertex(1)->info().height,
						recent->vertex(2)->info().height);
#endif

#if DEBUG_PRINT_WAVES
	n += sprintf(buf+n,"Wave:%0.f,%0.f,%0.f ",
						recent->vertex(0)->info().wave_height,
						recent->vertex(1)->info().wave_height,
						recent->vertex(2)->info().wave_height);
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
//			n += sprintf(buf+n, "%s ", FetchTokenString(*border));
			n += sprintf(buf+n, "%s (%f,%f,%f) ", FetchTokenString(*border),
						recent->vertex(0)->info().border_blend[*border],
						recent->vertex(1)->info().border_blend[*border],
						recent->vertex(2)->info().border_blend[*border]);
		}
#endif
	}

	return buf;
}


void	RF_MapView::SetFlowImage(
							ImageInfo&				image,
							double					bounds[4])
{
	for(int n = 0; n < 4; ++n)
	mFlowBounds[n] = bounds[n];
	if (LoadTextureFromImage(image, mFlowID, tex_Mipmap + tex_Linear, NULL, NULL, &mFlowS, &mFlowT))
		mHasFlow = true;
}

void	RF_MapView::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
}

void	RF_MapView::TimerFired(void)
{
	Refresh();
}
