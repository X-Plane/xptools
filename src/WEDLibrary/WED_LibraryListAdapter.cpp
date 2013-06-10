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

//IDK why having these in the header is giving me "identifier is undefined problems" in the watcher so they are now here
int mCatLocInd = 0;
int mCatLibInd = 0;
//string	mCatChanger = "";

WED_LibraryListAdapter::WED_LibraryListAdapter(WED_LibraryMgr * who) :
		GUI_SimpleTableGeometry(1,kDefCols,20),
	mCacheValid(false), mLibrary(who),
	mMap(NULL),
	mPreview(NULL),
	mLocalStr("Local/"),
	mLibraryStr("Library/"),
	mCatChanger("Local/")/*,
	mCatLocInd(0),
	mCatLibInd(0)*/
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
	
	RebuildCache();
	
	c.content_type = (cell_y < mCache.size()) ? gui_Cell_EditText : gui_Cell_None;
	
	c.can_edit = false;
	c.can_disclose = 0;

	//Key variable: r is used for spliting apart the full file path into sections
	string r = cell_y < mCache.size() ? mCache[cell_y] : "";

	//Set mCatChanger based on the r string, else it is carried over from whatever it was last set at
	//if the string at mCache is the last one (aka Local/)

	/*mCache by this point will look something like 
	* index 0
	* ...
	* index mCatLibInd
	* ...
	* index mCatLocInd
	* Therefore anything between 0 and mCatLibInd is underneath Library
	* and anything mCatLibInd+1 and mCatLocInd is underneath Local
	*
	* Since we are already dealting with indecies we can imediantly snip off the prefix
	*/
	if(cell_y < mCatLibInd)
	{
		mCatChanger = mLibraryStr;
		//Erase the prefix + /
		PrefixStripper(r);
	}
	else if(cell_y > mCatLibInd && cell_y < mCatLocInd)
	{
		mCatChanger = mLocalStr;
		//Erase the prefix + /
		PrefixStripper(r);
	}
	/*
	* Because none of the ranges of checking are <= or >=
	* We'll handle those cases here.
	* This is done so we don't accidentally cut off the word Local or Library
	* and yet still be able to make sure CatChanger is correct.
	* The content flag is also changed since in here we know we are one of the special Catagory Labels
	*/
	else if(cell_y == mCatLibInd)
	{
		mCatChanger = mLibraryStr;
		c.can_disclose = 1;
	}
	else if(cell_y == mCatLocInd)
	{
		mCatChanger = mLocalStr;
		c.can_disclose = 1;
	}
	//c.can_disclose = mLibrary->GetResourceType(r) == res_Directory;
	if( mLibrary->GetResourceType(r) == res_Directory)
	{
		c.can_disclose = 1;
	}

	c.can_select = true;
	
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

	//Add the prefix back in
	//If R is not either Local/ or Library/
	if(r != mCatChanger) 
	{
		//Erase the prefix
		PrefixAdder(r);
	}

	//Cut is a variable that helps with cuting the string apart
	//It starts at -1 to offset 
	int cut = -1;
	c.indent_level = 0;
	if(r != mCatChanger)
	{
		for(int n = 1; n < r.size(); ++n)
		{
			if(r[n] == '/')
			{
				cut = n;
				++c.indent_level;
			}
		}
		c.text_val = r.substr(cut+1);
	}
	else
	{
		r.erase(r.size()-1);
		c.text_val = r;
	}
	c.string_is_resource = 0;
#if DEV
	c.printCellInfo(true,true,true,true,false,true,true,false,true,0,0,0,0,1);
#endif
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
	if(is_toggle && r == mSel)
	{
		SetSel("");
	}
	else
	{
		PrefixStripper(r);
		SetSel(r);
		PrefixAdder(r);
	}

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
	if(mCacheValid) 
	{
		return;
	}

	//A collection of root paths, formerly known as seeds
	vector<string> rootItems;

	//Set the cache to be valid
	mCacheValid = true;

	//Clear out all strings inside
	mCache.clear();
	
	mCatChanger = mLocalStr;
	mCache.push_back(mCatChanger);
	
	if(IsOpen(mLocalStr))
	{
		//Goes to the data model and gets all of the root items that are local
		mLibrary->GetResourceChildren("",pack_Local,rootItems);

		//For all the root items
		for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		{
	
			//Try to find their children
			RebuildCacheRecursive(*s);
	
		}
	}

	mCatChanger = mLibraryStr;
	mCache.push_back(mCatChanger);
	
	if(IsOpen(mLibraryStr))
	{
		//Goes to the data model and gets all of the root items that are in the library
		mLibrary->GetResourceChildren("",pack_Library,rootItems);
	
		//For all root items
		for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		{
		
			//Try to find their children
			RebuildCacheRecursive(*s);
		
		}
	}

	//If there is something in the filte
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
		std::swap(keepers,mCache);
	}
	//Reverse the order.
	reverse(mCache.begin(),mCache.end());

	//Set the locations of mCatLocInd and mCatLibInd
	for(vector<string>::iterator itr = mCache.begin(); itr != mCache.end(); ++itr)
	{
		if(*itr == mLocalStr)
		{
			mCatLocInd = distance(mCache.begin(),itr);
		}
		if(*itr == mLibraryStr)
		{
			mCatLibInd = distance(mCache.begin(),itr);
		}
	}
}

void	WED_LibraryListAdapter::RebuildCacheRecursive(const string& r)
{
	//Add the string to the cache.
	mCache.push_back(mCatChanger + /*"/"+*/r);

	//If the item is open or the filter has something in it
	if(IsOpen(mCache.back()) || !mFilter.empty())
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
			{

				int type =  mLibrary->GetResourceType(s);
				mPreview->SetResource(s,type);
			}
			if(mMap)
				mMap->SetResource(s, mLibrary->GetResourceType(s));
		}
		
		mSel = s;
	}
}

void WED_LibraryListAdapter::PrefixAdder(string& path)
{
	path.insert(0,mCatChanger);
}

void WED_LibraryListAdapter::PrefixStripper(string& path, int extra)
{
	path.erase(0, mCatChanger.size()+extra);
}