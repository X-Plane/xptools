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
#include "WED_LibraryPreviewPane.h"
#include "WED_MapPane.h"
#include "GUI_Messages.h"
#include "STLUtils.h"

static int kDefCols[] = { 100, 100 };

WED_LibraryListAdapter::WED_LibraryListAdapter(WED_LibraryMgr * who) :
		GUI_SimpleTableGeometry(1,kDefCols,20),
	mCacheValid(false), mLibrary(who),
	mMap(NULL),
	mPreview(NULL)
{
	mLibrary->AddListener(this);
}


WED_LibraryListAdapter::~WED_LibraryListAdapter()
{
}

void	WED_LibraryListAdapter::SetMap(WED_MapPane * amap, WED_LibraryPreviewPane * apreview)
{
	mMap = amap;
	mPreview = apreview;
}

void	WED_LibraryListAdapter::SetFilter(const string& f)
{
	mFilter.clear();
	tokenize_string_func(f.begin(),f.end(),back_inserter(mFilter),::isspace);
	mCacheValid = false;
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

/********************************************************************************************************************************************
 * TABLE CALLBACKS
 ********************************************************************************************************************************************/

void	WED_LibraryListAdapter::GetCellContent(
			int							cell_x,
			int							cell_y,
			GUI_CellContent&			c)
{
	//creates a new copy of mCache
	RebuildCache();
	//If the cell_y is less than the size of the cache, it is an EditText cell, else
	//if the cell_y is greater than the size of the cache it is a Cell None
	c.content_type = (cell_y < mCache.size()) ? gui_Cell_EditText : gui_Cell_None;
	//Set unable to edit
	c.can_edit = false;

	//Key point: string r is equal to...
	//if cell_y is less than the cache than r is equal to the string in mCache at index of cell_y
	//else it is equal to nothing
	//r is used for spliting apart the full file path into sections
	string r = cell_y < mCache.size() ? mCache[cell_y] : "";

	//if the resource type is a directory than it can disclose
	c.can_disclose = mLibrary->GetResourceType(r) == res_Directory;
	//It is selectable
	c.can_select = true;
	//You cannot drag this content
	c.can_drag = false;

	//If it can be disclosed
	if(c.can_disclose)
	{
		//is disclosed is equal to the status of r as checked by IsOpen
		c.is_disclosed = IsOpen(r);
	}
	else
	{
		//Otherwish it is not disclosed
		c.is_disclosed = false;
	}

	//if r is the string selected in the library than c.is_selected is true
	c.is_selected = r == mSel;

	//Cut is a variable that helps with cuting the string apart
	//It starts at -1 to offset 
	int cut = -1;
	c.indent_level = 0;
	for(int n = 1; n < r.size(); ++n)
	{
		if(r[n] == '/')
		{
			cut = n;
			++c.indent_level;
		}
	}
	c.text_val = r.substr(cut+1);
	c.string_is_resource = false;
}

void	WED_LibraryListAdapter::GetEnumDictionary(
			int							cell_x,
			int							cell_y,
			GUI_EnumDictionary&			out_dictionary)
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

	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
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
		return 1;
	}
	return 0;
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
	return 0;
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
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
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
	//If the cache is valid, exit early because it doesn't need to rebuild
	if(mCacheValid) return;

	//Set the cache to be valid
	mCacheValid = true;

	//Clear out all strings inside
	mCache.clear();

	//A collection of root paths, formerly known as seeds
	vector<string> rootItems;

	//Goes to the data model and gets all of the root items that are local
	mLibrary->GetResourceChildren("",pack_Local,rootItems);

	//For all the root items
	for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		//Try to find their children
		RebuildCacheRecursive(*s);

	//Goes to the data model and gets all of the root items that are in the library
	mLibrary->GetResourceChildren("",pack_Library,rootItems);
	
	//For all root items
	for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		//Try to find their children
		RebuildCacheRecursive(*s);

	//If there is something in the filter
	if(!mFilter.empty())
	{
		//A collection strings to keep
		vector<string>	keepers;
		int last = -1;

		//For all the strings in the cache
		for(int i = 0; i < mCache.size(); ++i)
		{
			//If the current string in mCache matches whats in the filter
			if(filter_match(mCache[i],mFilter.begin(),mFilter.end()))
			{

				for(int p = last+1; p < i; ++p)
				{
					if(mCache[p].size() < mCache[i].size() &&
						strncasecmp(mCache[p].c_str(),mCache[i].c_str(),mCache[p].size()) == 0)
					{
						keepers.push_back(mCache[p]);					
					}
				}
				//Add the string to keepers
				keepers.push_back(mCache[i]);
				last = i;
			}
		}

		//Swap keepers and mCache so mCache only has the strings to keep
		swap(keepers,mCache);
	}
	//Reverse the order.
	reverse(mCache.begin(),mCache.end());
}

void	WED_LibraryListAdapter::RebuildCacheRecursive(const string& r)
{
	//Add the string to the cache.
	mCache.push_back(r);
	//If the item is open or the filter has something in it
	if(IsOpen(r) || !mFilter.empty())
	{
		//Recuse again
		vector<string> kids;
		mLibrary->GetResourceChildren(r,pack_All,kids);
		for(vector<string>::iterator k = kids.begin(); k != kids.end(); ++k)
			RebuildCacheRecursive(*k);
	}
}

void WED_LibraryListAdapter::SetSel(const string& s)
{
	if(s != mSel)
	{
		if(s.empty())
		{
			if(mPreview)
				mPreview->ClearResource();
		}
		else
		{
			if(mPreview)
				mPreview->SetResource(s, mLibrary->GetResourceType(s));
			if(mMap)
				mMap->SetResource(s, mLibrary->GetResourceType(s));
		}
		
		mSel = s;
	}
}
