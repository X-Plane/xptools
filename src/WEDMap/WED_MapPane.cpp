/* 
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_MapPane.h"
#include "WED_Archive.h"
#include "WED_Menus.h"
#include "GUI_ToolBar.h"
#include "XESConstants.h"
#include "IGIS.h"
#include "ISelection.h"
#include "WED_Thing.h"
#include "WED_Map.h"
#include "WED_MapBkgnd.h"
#include "WED_ToolUtils.h"
#include "WED_MarqueeTool.h"
#include "WED_CreatePolygonTool.h"
#include "WED_CreatePointTool.h"
#include "WED_CreateLineTool.h"
#include "WED_StructureLayer.h"
#include "WED_WorldMapLayer.h"
#include "WED_VertexTool.h"
//#include "WED_TileServerLayer.h"
#include "WED_TerraserverLayer.h"
#include "GUI_Fonts.h"
#include "GUI_Table.h"
#include "GUI_TextTable.h"
#include "WED_Colors.h"
#include "GUI_Resources.h"
#include "WED_ToolInfoAdapter.h"
#include "WED_UIMeasurements.h"
#include "WED_GroupCommands.h"
#include "IDocPrefs.h"
char	kToolKeys[] = { 
	'b', 'w', 'e', 'o',
	'a', 'f', 'g', 'l',
	'k', 't', 'h', 's',
	'r', 'm', 'v' 
};

static void GetExtentAll(Bbox2& box, IResolver * resolver)
{
	box = Bbox2();
	WED_Thing * wrl = WED_GetWorld(resolver);
	IGISEntity * ent = dynamic_cast<IGISEntity *>(wrl);
	if (ent) ent->GetBounds(box);
}

static int accum_box(ISelectable * who, void * ref)
{
	Bbox2 * total = (Bbox2 *) ref;
	IGISEntity * ent = dynamic_cast<IGISEntity *>(who);
	if (ent)
	{
		Bbox2 ent_box;
		ent->GetBounds(ent_box);
		*total += ent_box;
	}
	return 0;
}

static void GetExtentSel(Bbox2& box, IResolver * resolver)
{
	box = Bbox2();
	ISelection * sel = WED_GetSelect(resolver);
	sel->IterateSelection(accum_box,&box);
}


WED_MapPane::WED_MapPane(GUI_Commander * cmdr, double map_bounds[4], IResolver * resolver, WED_Archive * archive) : mResolver(resolver)
{
	mMap = new WED_Map(resolver);

	// Visualizatoin layers
	mLayers.push_back(					new WED_MapBkgnd(mMap, mMap, resolver));
	mLayers.push_back(mWorldMap =		new WED_WorldMapLayer(mMap, mMap, resolver));
	mLayers.push_back(mStructureLayer = new WED_StructureLayer(mMap, mMap, resolver));
	mLayers.push_back(mTerraserver = 	new WED_TerraserverLayer(mMap, mMap, resolver));
//	mLayers.push_back(mTileserver =		new WED_TileServerLayer(mMap, mMap, resolver));
	
	// TOOLS
	mTools.push_back(					new WED_CreatePolygonTool("Boundary",mMap, mMap, resolver, archive, create_Boundary));
	mTools.push_back(					new WED_CreatePointTool("Windsock", mMap, mMap, resolver, archive, create_Windsock));
	mTools.push_back(					new WED_CreatePointTool("Airport Beacon", mMap, mMap, resolver, archive, create_Beacon));
	mTools.push_back(					new WED_CreatePointTool("Tower Viewpoint", mMap, mMap, resolver, archive, create_TowerViewpoint));
	mTools.push_back(					new WED_CreatePointTool("Ramp Start", mMap, mMap, resolver, archive, create_RampStart));
	mTools.push_back(					new WED_CreatePointTool("Light Fixture", mMap, mMap, resolver, archive, create_Lights));
	mTools.push_back(					new WED_CreatePointTool("Sign", mMap, mMap, resolver, archive, create_Sign));
	mTools.push_back(					new WED_CreatePolygonTool("Taxilines",mMap, mMap, resolver, archive, create_Marks));
	mTools.push_back(					new WED_CreatePolygonTool("Hole",mMap, mMap, resolver, archive, create_Hole));
	mTools.push_back(					new WED_CreatePolygonTool("Taxiway",mMap, mMap, resolver, archive, create_Taxi));
	mTools.push_back(					new WED_CreatePointTool("Helipad", mMap, mMap, resolver, archive, create_Helipad));
	mTools.push_back(					new WED_CreateLineTool("Sealane", mMap, mMap, resolver, archive, create_Sealane));
	mTools.push_back(					new WED_CreateLineTool("Runway", mMap, mMap, resolver, archive, create_Runway));
//	mTools.push_back(mImageOverlay = 	new WED_ImageOverlayTool("Overlay Picture",mMap, mMap, resolver));
	mTools.push_back(					new WED_MarqueeTool("Marquee",mMap, mMap, resolver));
	mTools.push_back(					new WED_VertexTool("Vertex",mMap, mMap, resolver, 1));

	mInfoAdapter = new WED_ToolInfoAdapter(GUI_GetImageResourceHeight("property_bar.png") / 2);
	mTextTable = new GUI_TextTable(cmdr,10);
	mTable = new GUI_Table(1);
	
	mTextTable->SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_PropertyBar_Text),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable->SetFont(font_UI_Small);
	

	mTable->SetGeometry(mInfoAdapter);
	mTable->SetContent(mTextTable);
	mTextTable->SetProvider(mInfoAdapter);
//	mTable->SetParent(this);
//	mTable->Show();
	mTable->SizeShowAll();
	mTextTable->SetParentTable(mTable);
//	mTable->SetSticky(1,0,1,1);
//	this->PackPane(mTable, gui_Pack_Top);
	mTextTable->AddListener(mTable);
	mTextTable->SetImage("property_bar.png", 2);

	mInfoAdapter->AddListener(mTable);

	mToolbar = new GUI_ToolBar(1,15,"map_tools.png");
	mToolbar->SizeToBitmap();
	mToolbar->Show();
	mToolbar->SetParent(this);
	mToolbar->SetSticky(1,0,0,1);
	this->PackPane(mToolbar,gui_Pack_Left);
	mToolbar->SizeToBitmap();
	mToolbar->AddListener(this);	
	vector<string>	tips;
	for (int n = 0; n < mTools.size(); ++n)
	{
		string tip(mTools[n]->GetToolName());
		if (kToolKeys[n])
		{
			char buf[5] = { " [x]" };
			buf[2] = toupper(kToolKeys[n]);
			tip += buf;
		}
		tips.push_back(tip);
	}
	mToolbar->SetToolTips(tips);	
	

	GUI_ScrollerPane * map_scroller = new GUI_ScrollerPane(1,1);
	map_scroller->SetParent(this);
	map_scroller->Show();
	map_scroller->SetSticky(1,1,1,1);
	
	this->PackPane(map_scroller, gui_Pack_Center);
	
	mMap->SetParent(map_scroller);
	mMap->Show();
	map_scroller->PositionInContentArea(mMap);
	map_scroller->SetContent(mMap);
	
//	mMap->SetMapVisibleBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);
	mMap->SetMapLogicalBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);

	mMap->ZoomShowAll();

	for(vector<WED_MapLayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		mMap->AddLayer(*l);
	for(vector<WED_MapToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		mMap->AddLayer(*t);

	mMap->SetTool(mTools[0]);
	mInfoAdapter->SetTool(mTools[0]);
	mToolbar->SetValue(mTools.size()-1);
	
	// This is a bit of a hack.  The archive provides whole-doc "changed" messages at the minimum global times:
	// 1. On the commit of any operation.
	// 2. On the undo or redo of any operation.
	// So ... for lack of a better idea right now, we simply broker a connection between the source opf these
	// messages (secretly it's our document's GetArchive() member) and anyone who needs it (our map).
	
	archive->AddListener(mMap);
}

GUI_Pane *	WED_MapPane::GetTopBar(void)
{
	return mTable;
}

WED_MapPane::~WED_MapPane()
{
	for(vector<WED_MapLayer *>::iterator l = mLayers.begin(); l != mLayers.end(); ++l)
		delete *l;
	for(vector<WED_MapToolNew *>::iterator t = mTools.begin(); t != mTools.end(); ++t)
		delete *t;

	delete mTextTable;
	delete mInfoAdapter;
	
}



void	WED_MapPane::ZoomShowAll(void)
{
//	double l,b,r,t;
//	mMap->GetMapLogicalBounds(l,b,r,t);
//	mMap->SetAspectRatio(1.0 / cos((b+t) * 0.5 * DEG_TO_RAD));
	mMap->ZoomShowAll();
}

int		WED_MapPane::Map_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mMap->KeyPress(inKey, inVK, inFlags)) return 1;
	for (int n = 0; n < sizeof(kToolKeys) / sizeof(kToolKeys[0]); ++n)
	if (kToolKeys[n])
	if (kToolKeys[n]==inKey)
	{
		mToolbar->SetValue(n);
		return 1;
	}
	return 0;
}

int		WED_MapPane::Map_HandleCommand(int command)
{
	Bbox2 box;
	switch(command) {
	case wed_PickOverlay:	WED_DoMakeNewOverlay(mResolver, mMap); return 1;
	case wed_ToggleWorldMap:mWorldMap->ToggleVisible(); return 1;
//	case wed_ToggleOverlay:	if (mImageOverlay->CanShow()) { mImageOverlay->ToggleVisible(); return 1; }
	case wed_ToggleTerraserver:	mTerraserver->ToggleVis(); return 1;
//	case wed_ToggleTileserver: mTileserver->ToggleVis(); return 1;

	case wed_Pavement0:		mStructureLayer->SetPavementTransparency(0.0f);		return 1;
	case wed_Pavement25:	mStructureLayer->SetPavementTransparency(0.25f);	return 1;
	case wed_Pavement50:	mStructureLayer->SetPavementTransparency(0.5f);		return 1;
	case wed_Pavement75:	mStructureLayer->SetPavementTransparency(0.75f);	return 1;
	case wed_Pavement100:	mStructureLayer->SetPavementTransparency(1.0f);		return 1;
	case wed_ToggleLines:	mStructureLayer->SetRealLinesShowing(!mStructureLayer->GetRealLinesShowing());				return 1;
	case wed_ToggleVertices:mStructureLayer->SetVerticesShowing(!mStructureLayer->GetVerticesShowing());				return 1;

	case wed_ZoomWorld:		mMap->ZoomShowArea(-180,-90,180,90);	mMap->Refresh(); return 1;
	case wed_ZoomAll:		GetExtentAll(box, mResolver); mMap->ZoomShowArea(box.p1.x,box.p1.y,box.p2.x,box.p2.y);	mMap->Refresh(); return 1;
	case wed_ZoomSelection:	GetExtentSel(box, mResolver); mMap->ZoomShowArea(box.p1.x,box.p1.y,box.p2.x,box.p2.y);	mMap->Refresh(); return 1;

	default:		return 0;
	}	
}

int		WED_MapPane::Map_CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	Bbox2	box;
	
	switch(command) {
	case wed_PickOverlay:																	return 1;
	case wed_ToggleWorldMap:ioCheck = mWorldMap->IsVisible();								return 1;
//	case wed_ToggleOverlay:	if (mImageOverlay->CanShow()) { ioCheck = mImageOverlay->IsVisible(); return 1; }	break;
	case wed_ToggleTerraserver: ioCheck = mTerraserver->IsVis();							return 1;
//	case wed_ToggleTileserver: ioCheck = mTileserver->IsVis();								return 1;
	case wed_Pavement0:		ioCheck = mStructureLayer->GetPavementTransparency() == 0.0f;	return 1;
	case wed_Pavement25:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.25f;	return 1;
	case wed_Pavement50:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.5f;	return 1;
	case wed_Pavement75:	ioCheck = mStructureLayer->GetPavementTransparency() == 0.75f;	return 1;
	case wed_Pavement100:	ioCheck = mStructureLayer->GetPavementTransparency() == 1.0f;	return 1;
	case wed_ToggleLines:	ioCheck = mStructureLayer->GetRealLinesShowing();				return 1;
	case wed_ToggleVertices:ioCheck = mStructureLayer->GetVerticesShowing();				return 1;
	
	case wed_ZoomWorld:		return 1;
	case wed_ZoomAll:		GetExtentAll(box, mResolver); return !box.is_empty()  && !box.is_null();
	case wed_ZoomSelection:	GetExtentSel(box, mResolver); return !box.is_empty()  && !box.is_null();
	
	default:		return 0;
	}	
	return 0;
}

void	WED_MapPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	int i = mToolbar->GetValue();
	WED_MapToolNew * t = NULL;
	if (i >= 0 && i < mTools.size())
		t = mTools[i];
	mMap->SetTool(t);		
	mInfoAdapter->SetTool(t);
}

void			WED_MapPane::FromPrefs(IDocPrefs * prefs)
{
	if ((mWorldMap->IsVisible() ? 1 : 0) != prefs->ReadIntPref("map/world_map_vis",  mWorldMap->IsVisible() ? 1 : 0))		mWorldMap->ToggleVisible();
	if ((mTerraserver->IsVis () ? 1 : 0) != prefs->ReadIntPref("map/terraserver_vis",mTerraserver->IsVis()  ? 1 : 0))		mTerraserver->ToggleVis();

	mStructureLayer->SetPavementTransparency(prefs->ReadIntPref("map/pavement_alpha",mStructureLayer->GetPavementTransparency()*4) * 0.25f);
	mStructureLayer->SetRealLinesShowing(	 prefs->ReadIntPref("map/real_lines_vis",mStructureLayer->GetRealLinesShowing() ? 1 : 0) != 0);
	mStructureLayer->SetVerticesShowing(	 prefs->ReadIntPref("map/vertices_vis",	 mStructureLayer->GetVerticesShowing() ? 1 : 0) != 0);
	
	double w,s,e,n;
	mMap->GetMapVisibleBounds(w,s,e,n);
	mMap->ZoomShowArea(
		prefs->ReadDoublePref("map/west" ,w),
		prefs->ReadDoublePref("map/south",s),
		prefs->ReadDoublePref("map/east", e),
		prefs->ReadDoublePref("map/north",n));


	for (int t = 0; t < mTools.size(); ++t)
	{
		int pc = mTools[t]->CountProperties();
		for (int p = 0; p < pc; ++p)
		{
			PropertyInfo_t	inf;
			PropertyVal_t	val;
			mTools[t]->GetNthPropertyInfo(p,inf);
			string key = "map_";
			key += mTools[t]->GetToolName();
			key += "_";
			key += inf.prop_name;
			string v;
			string::size_type s, e;
			
			v = prefs->ReadStringPref(key.c_str(),string());
			if (!v.empty())
			{
				val.prop_kind = inf.prop_kind;
				switch(inf.prop_kind) {
				case prop_Int:
				case prop_Bool:
				case prop_Enum:
					val.int_val = atoi(v.c_str());
					break;
				case prop_Double:
					val.double_val = atoi(v.c_str());
					break;			
				case prop_String:	
				case prop_FilePath:
					val.string_val = v;
					break;
				case prop_EnumSet:
					s = 0;
					do {
						e = v.find(',',s);
						val.set_val.insert(atoi(v.c_str() + s));
						if (e != v.npos)
							s = e + 1;
					} while (e != v.npos);					
					break;
				}
				mTools[t]->SetNthProperty(p,val);
			}
		}
	}	
}

void			WED_MapPane::ToPrefs(IDocPrefs * prefs)
{
	prefs->WriteIntPref("map/world_map_vis",mWorldMap->IsVisible() ? 1 : 0);
	prefs->WriteIntPref("map/terraserver_vis",mTerraserver->IsVis() ? 1 : 0);
	prefs->WriteIntPref("map/pavement_alpha",mStructureLayer->GetPavementTransparency()*4);
	prefs->WriteIntPref("map/real_lines_vis",mStructureLayer->GetRealLinesShowing() ? 1 : 0);
	prefs->WriteIntPref("map/vertices_vis",mStructureLayer->GetVerticesShowing() ? 1 : 0);

	double w,s,e,n;
	mMap->GetMapVisibleBounds(w,s,e,n);
	prefs->WriteDoublePref("map/west" ,w);
	prefs->WriteDoublePref("map/south",s);
	prefs->WriteDoublePref("map/east", e);
	prefs->WriteDoublePref("map/north",n);

	for (int t = 0; t < mTools.size(); ++t)
	{
		int pc = mTools[t]->CountProperties();
		for (int p = 0; p < pc; ++p)
		{
			PropertyInfo_t	inf;
			PropertyVal_t	val;
			mTools[t]->GetNthPropertyInfo(p,inf);
			mTools[t]->GetNthProperty(p,val);

			string key = "map_";
			key += mTools[t]->GetToolName();
			key += "_";
			key += inf.prop_name;

			string v;
			char buf[256];
			switch(val.prop_kind) {
			case prop_Int:
			case prop_Bool:
			case prop_Enum:
				sprintf(buf,"%d",val.int_val);
				v = buf;
				break;
			case prop_Double:
				sprintf(buf,"%lf",val.double_val);
				v = buf;
				break;			
			case prop_String:	
			case prop_FilePath:
				v = val.string_val;
				break;
			case prop_EnumSet:
				for (set<int>::iterator it = val.set_val.begin(); it != val.set_val.end(); ++it)
				{
					if (!v.empty()) v += ",";
					sprintf(buf,"%d",*it);
					v += buf;
				}
				break;
			}
			
			prefs->WriteStringPref(key.c_str(),v);
		}
	}	
}

