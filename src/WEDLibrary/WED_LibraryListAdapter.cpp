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
#include "WED_CreatePointTool.h"
#include "WED_CreatePolygonTool.h"
#include "WED_LibraryPreviewPane.h"
#include "WED_MapPane.h"
#include "STLUtils.h"
#include "MathUtils.h"

static int kDefCols[] = { 220, 20 };

WED_LibraryListAdapter::WED_LibraryListAdapter(WED_LibraryMgr* who) :
	GUI_SimpleTableGeometry(GetColCount(), kDefCols),
	mCacheValid(false), mLibrary(who),
	mFilterChanged(false),
	mMap(NULL),
	mPreview(NULL),
	//Set to diffrent numbers so as not to cause conflicts
	//in GetNthCacheIndex
	mCatLocInd(-1234),
	mCatLibInd(-5678),
	mCurPakVal(pack_Library)
{
	mLibrary->AddListener(this);

	this->mLocalStr = "Local/";
	this->mLibraryStr = "Library/";
}


WED_LibraryListAdapter::~WED_LibraryListAdapter()
{
}

void	WED_LibraryListAdapter::SetMap(WED_MapPane * amap, WED_LibraryPreviewPane * apreview)
{
	mMap = amap;
	mPreview = apreview;
	mPreview->AddListener(this);
}

void	WED_LibraryListAdapter::SetFilter(const string& f, int int_val)
{
	mFilter.clear();
	mFilterChanged = true;
	mCurPakVal = int_val;
	//Ensures that even with no library heirarchy things
	//Can still be searched for

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
	RebuildCache();
	if (cell_y >= mCache.size())
	{
		c.content_type = gui_Cell_None;
		return;
	}

	c.content_type = gui_Cell_EditText;
	c.is_selected =  mCache[cell_y].vpath == mSel;
	c.can_delete = false;
	c.can_edit = false;
	c.can_drag = false;
	c.string_is_resource = 0;
	c.indent_level = 0;

	if (cell_x == 1)
	{
		c.can_select = false;
		c.is_disclosed = false;
		c.can_disclose = false;
		c.text_val = mCache[cell_y].hasSeasons ? "seasons.png" : "";
		if (mCache[cell_y].variants)
		{
			if (c.text_val.length()) c.text_val += ",";
			c.text_val += "variants.png";
		}
		if (mCache[cell_y].hasRegions)
		{
			if (c.text_val.length()) c.text_val += ",";
			c.text_val += "regions.png";
		}
		if (c.text_val.length())
			c.string_is_resource = 1;
	}
	else
	{
		c.can_select = true;
		c.is_disclosed = mCache[cell_y].isOpen;
		c.can_disclose = mCache[cell_y].isDir;
		if (cell_y == mCatLocInd)
		{
			c.text_val = mLocalStr;
			c.text_val.pop_back();
		}
		else if (cell_y == mCatLibInd)
		{
			c.text_val = mLibraryStr;
			c.text_val.pop_back();
		}
		else
		{
			c.indent_level = std::count(mCache[cell_y].vpath.begin(), mCache[cell_y].vpath.end(), '/');
			c.text_val = mCache[cell_y].vpath.substr(mCache[cell_y].vpath.find_last_of('/') + 1);
		}
	}
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
		mCache[cell_y].isOpen = 1 - mCache[cell_y].isOpen;
		mCacheValid = false;
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
	}
}

void	WED_LibraryListAdapter::DoDrag(
			GUI_Pane *					drag_emitter,
			int							mouse_x,
			int							mouse_y,
			int							button,
			int							bounds[4])
{
}

void	WED_LibraryListAdapter::SelectionStart(
			int							clear)
{
	if(clear)
		SetSel("","");
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
	{
		if(mCache[n].vpath == mSel)
		{
			low_y = high_y = n;
			return true;
		}
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
	string noPrefix = GetNthCacheIndex(start_y,true);

	if(is_toggle && mCache[start_y].vpath == mSel)
		SetSel("",noPrefix);
	else
		SetSel(mCache[start_y].vpath, noPrefix);

	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
}

void	WED_LibraryListAdapter::SelectionEnd(void)
{
}

int		WED_LibraryListAdapter::SelectDisclose(bool open_it, bool all, set<int>* did_open)
{
	/* Test if the current selection is not a file
	* If it is not a file, do what it needs to do
	* Else return nothing
	*/

	if (mSel.empty()) return 0;

	for (auto& c : mCache)
	{
		if (c.vpath == mSel)
		{
			if (c.isDir)
			{
				c.isOpen = 1;
				mCacheValid = false;
				BroadcastMessage(GUI_TABLE_CONTENT_RESIZED, 0);
				return 1;
			}
			else
				return 0;
		}
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
	return 2;
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
	c.title = "Library Pane";
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
	else if(inSrc == mPreview && inMsg == WED_PRIVATE_MSG_BASE)
	{
		string tmp((char *) inParam);
		LOG_MSG("42: %s\n", tmp.c_str()); LOG_FLUSH();

		SelectDisclose(1, 0);
		SetSel(mLibraryStr+tmp, tmp);
		// figure out how to scroll the selected cell into the middle of the scrolled pane
	}
}

void WED_LibraryListAdapter::FilterCache()
{
	if(!mFilter.empty())
	{
		newCache.clear();
		int last = -1;
//      a half-ass attempt as allowing to support undisclose when filtering is active. Works only for a single level
		bool show = 1;

		for(int i = 0; i < mCache.size(); ++i)
		{
			if(filter_match(mCache[i].vpath, mFilter.begin(), mFilter.end()))
			{
				// add all the directory levels above it as needed
				for(int p = last+1; p < i; ++p)
				{
					if(mCache[p].vpath.size() < mCache[i].vpath.size() &&
						strncasecmp(mCache[p].vpath.c_str(), mCache[i].vpath.c_str(), mCache[p].vpath.size()) == 0)
					{
						newCache.push_back(mCache[p]);
						show = mCache[p].isOpen;
					}
				}
				// add the vpath itself to the keepers
				if(show)
					newCache.push_back(mCache[i]);
				last = i;
			}
		}
		swap(newCache, mCache);
	}
	reverse(mCache.begin(),mCache.end());

	mCatLibInd = -1;
	mCatLocInd = -1;

	for(int i = 0; i< mCache.size(); i++)
	{
		if (mCache[i].vpath == mLocalStr)
			mCatLocInd = i;
		else if (mCache[i].vpath == mLibraryStr)
			mCatLibInd = i;
	}
}

void	WED_LibraryListAdapter::RebuildCache()
{
	if (mCacheValid) return;

	newCache.clear();

	RebuildCacheRecursive("", pack_Local, mLocalStr);
	RebuildCacheRecursive("", mCurPakVal, mLibraryStr);
	mFilterChanged = false;

	swap(newCache, mCache);
	FilterCache();
	mCacheValid = true;
}

void	WED_LibraryListAdapter::RebuildCacheRecursive(const string& vpath, int packType, const string& prefix)
{
	newCache.push_back(prefix + vpath);

	if (mLibrary->GetResourceType(vpath) == res_Directory || mLibrary->GetResourceType(vpath) == res_None)
	{
		newCache.back().isDir = 1;
		// persist open status from last status
		for (auto& c : mCache)
			if (c.vpath == newCache.back().vpath)
			{
				if (c.isOpen) newCache.back().isOpen = 1;
				break;
			}

		// force re-open anytime the filter was changed
		if (newCache.back().isDir && mFilterChanged)
			newCache.back().isOpen = !mFilter.empty();

		if (newCache.back().isOpen || !mFilter.empty())
		{
			vector<string> kids;
			mLibrary->GetResourceChildren(vpath, packType, kids);
			for (vector<string>::iterator k = kids.begin(); k != kids.end(); ++k)
				RebuildCacheRecursive(*k, packType, prefix);
		}
	}
	else
	{
		newCache.back().isDir = 0;
		newCache.back().hasSeasons = mLibrary->IsSeasonal(vpath);
		newCache.back().hasRegions = mLibrary->IsRegional(vpath);
		newCache.back().variants   = mLibrary->GetNumVariants(vpath) > 1; // for now WED is reading all regions, regardless. So you get fake variants ...
	}
}

void WED_LibraryListAdapter::SetSel(const string& s,const string& noPrefix)
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
				mPreview->SetResource(noPrefix, mLibrary->GetResourceType(noPrefix), mLibrary->GetNumVariants(noPrefix));
			if(mMap)
				mMap->SetResource(noPrefix, mLibrary->GetResourceType(noPrefix));
		}

		mSel = s;
	}
}

string WED_LibraryListAdapter::GetNthCacheIndex (int index, bool noPrefix)
{
	string path = mCache[index].vpath;

	/*mCache by this point will look something like
	* index 0
	* ...
	* index mCatLibInd
	* ...
	* index mCatLocInd
	* Therefore anything between 0 and mCatLibInd is underneath Library
	* and anything mCatLibInd+1 and mCatLocInd is underneath Local
	*/
	if (!noPrefix)
		return path;
	else if(index < mCatLibInd)
		return path.erase(0,mLibraryStr.length());
	else if(index > mCatLibInd && index < mCatLocInd)
		return path.erase(0,mLocalStr.length());
	/*
	* Because none of the ranges of checking are <= or >=
	* We'll handle those cases here.
	*/
	else if(index == mCatLibInd)
		return path.erase(path.size()-1);
	else if(index == mCatLocInd)
		return path.erase(path.size()-1);
	else
	{
		// This is just to shut the compiler up -
		// mCatLocInd is the LAST item in the array since the cache is
		// upside down.  So if we fall out here, our index is bad.
		DebugAssert(!"Out of bounds index.");
		return path;
	}
}
