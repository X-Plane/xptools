/* 
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_LibraryListAdapter.h"
#include "WED_LibraryMgr.h"
#include "WED_Messages.h"
#include "GUI_Messages.h"
#include "WED_CreatePointTool.h"
#include "WED_CreatePolygonTool.h"
#include "WED_MapPane.h"
static int kDefCols[] = { 100, 0 };

WED_LibraryListAdapter::WED_LibraryListAdapter(WED_LibraryMgr * who) : 
		GUI_SimpleTableGeometry(1,kDefCols,20),	
	mCacheValid(false), mLibrary(who),
	mMap(NULL)
{
	mLibrary->AddListener(this);
}
	
	
WED_LibraryListAdapter::~WED_LibraryListAdapter()
{
}

void	WED_LibraryListAdapter::SetMap(WED_MapPane * amap)
{
	mMap = amap;
}

/********************************************************************************************************************************************
 * TABLE CALLBACKS
 ********************************************************************************************************************************************/

void	WED_LibraryListAdapter::GetCellContent(
			int							cell_x, 
			int							cell_y, 
			GUI_CellContent&			c)
{
	RebuildCache();
	c.content_type = (cell_y < mCache.size()) ? gui_Cell_EditText : gui_Cell_None;
	c.can_edit = false;
	string r = cell_y < mCache.size() ? mCache[cell_y] : "";
	c.can_disclose = mLibrary->GetResourceType(r) == res_Directory;
	c.can_select = true;
	c.can_drag = false;
	if(c.can_disclose)	c.is_disclosed = IsOpen(r);
	else				c.is_disclosed = false;
	c.is_selected = r == mSel;
	int cut = -1;
	c.indent_level = 0;
	for(int n = 1; n < r.size(); ++n)
		if(r[n] == '/') cut = n, ++c.indent_level;	
	c.text_val = r.substr(cut+1);
	c.string_is_resource = false;
}

void	WED_LibraryListAdapter::GetEnumDictionary(
			int							cell_x, 
			int							cell_y, 
			map<int, string>&			out_dictionary)
{
}

void	WED_LibraryListAdapter::AcceptEdit(
			int							cell_x,
			int							cell_y,
			const GUI_CellContent&		the_content,
			int							apply_all)
{
}

void	WED_LibraryListAdapter::ToggleDisclose(
			int							cell_x,
			int							cell_y)
{
	RebuildCache();
	if(cell_y < mCache.size())
	{
		string r(mCache[cell_y]);
		SetOpen(r,1-IsOpen(r));
		mCacheValid = false;
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);				
	}
}

void	WED_LibraryListAdapter::DoDrag(
			GUI_Pane *					drag_emitter,
			int							mouse_x,
			int							mouse_y,
			int							bounds[4])
{
}

void	WED_LibraryListAdapter::SelectionStart(
			int							clear)
{
	if(clear)
		SetSel("");
}

int		WED_LibraryListAdapter::SelectGetExtent(
			int&						low_x,
			int&						low_y,
			int&						high_x,
			int&						high_y)
{
	RebuildCache();
	low_x = high_x = 0;
	for(int n = 0; n < mCache.size(); ++n)
	if(mCache[n] == mSel)
	{
		low_y = high_y = n;
		return true;
	}
	return false;
}

int		WED_LibraryListAdapter::SelectGetLimits(
			int&						low_x,
			int&						low_y,
			int&						high_x,
			int&						high_y)
{
	low_x = low_y = 0;
	high_x = GetColCount()-1;
	high_y = GetRowCount()-1;
	return (high_x >= 0 && high_y >= 0);
}

void	WED_LibraryListAdapter::SelectRange(
			int							start_x,
			int							start_y,
			int							end_x,
			int							end_y,
			int							is_toggle)
{
	RebuildCache();
	string r = mCache[start_y];
	if(is_toggle && r == mSel)	SetSel("");
	else						SetSel(r);
	
	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,NULL);
}

void	WED_LibraryListAdapter::SelectionEnd(void)
{
}

int		WED_LibraryListAdapter::SelectDisclose(
			int							open_it,
			int							all)
{
	if (!mSel.empty() && mLibrary->GetResourceType(mSel) == res_Directory)
	{
		SetOpen(mSel, open_it);
		mCacheValid = false;
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);				
	}
}

int		WED_LibraryListAdapter::TabAdvance(
			int&						io_x,
			int&						io_y,
			int							reverse,
			GUI_CellContent&			the_content)
{
	return 0;
}			

int		WED_LibraryListAdapter::DoubleClickCell(
			int							cell_x,
			int							cell_y)
{
}

/********************************************************************************************************************************************
 * GEOMETRY AND CACHE MANAGEMENT
 ********************************************************************************************************************************************/

int		WED_LibraryListAdapter::GetColCount(void)
{
	return 1;
}

int		WED_LibraryListAdapter::GetRowCount(void)
{
	RebuildCache();
	return mCache.size();
}

void	WED_LibraryListAdapter::GetHeaderContent(
						int							cell_x, 
						GUI_HeaderContent&			c)
{
	c.title = "Library";
	c.is_selected=false;
	c.can_resize=false;
	c.can_select=false;

}						

void	WED_LibraryListAdapter::ReceiveMessage(
						GUI_Broadcaster *		inSrc,
						intptr_t				inMsg,
						intptr_t				inParam)
{
	if(inMsg == msg_LibraryChanged)
	{
		mCacheValid = false;
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,NULL);
	}
}

int		WED_LibraryListAdapter::IsOpen(const string& r)
{
	hash_map<string,int>::iterator i = mOpen.find(r);
	if(i == mOpen.end()) return false;
	return i->second;
}

void	WED_LibraryListAdapter::SetOpen(const string& r, int open)
{
	mOpen[r] = open;
}

void	WED_LibraryListAdapter::RebuildCache()
{
	if(mCacheValid) return;
	mCacheValid = true;
	mCache.clear();
	
	vector<string> seeds;
	mLibrary->GetResourceChildren("",pack_All,seeds);
	for(vector<string>::iterator s = seeds.begin(); s != seeds.end(); ++s)
		RebuildCacheRecursive(*s);	
		
	reverse(mCache.begin(),mCache.end());
}
	
void	WED_LibraryListAdapter::RebuildCacheRecursive(const string& r)
{
	mCache.push_back(r);
	if(IsOpen(r))
	{
		vector<string> kids;
		mLibrary->GetResourceChildren(r,pack_All,kids);
		for(vector<string>::iterator k = kids.begin(); k != kids.end(); ++k)
			RebuildCacheRecursive(*k);			
	}
}

void WED_LibraryListAdapter::SetSel(const string& s)
{
	if(s != mSel && !s.empty()) mMap->SetResource(s, mLibrary->GetResourceType(s));
	mSel = s;
}