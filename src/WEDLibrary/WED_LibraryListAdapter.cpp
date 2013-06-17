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
int printcounter = 0;
WED_LibraryListAdapter::WED_LibraryListAdapter(WED_LibraryMgr * who) :
		GUI_SimpleTableGeometry(1,kDefCols,20),
	mCacheValid(false), mLibrary(who),
	mMap(NULL),
	mPreview(NULL),
	//Set to diffrent numbers so as not to cause conflicts
	//in GetNthCacheIndex
	mCatLocInd(-1234),
	mCatLibInd(-5678)
{
	mLibrary->AddListener(this);
	//this->AddListener(
	this->mLocalStr = "Local/";
	this->mLibraryStr = "Library/";
	mOpen[mLocalStr] = 0;
	mOpen[mLibraryStr] = 0;
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
	/*How this works
	* 1.) Rebuild the cache
	* 2.) Get the path from mCache (with the prefixes)
	* 3.) Set Flags
	* 4.) Cut and draw (see more below)
	*/
	RebuildCache();

	string pPath = cell_y < mCache.size() ? GetNthCacheIndex(cell_y,true) : "";

	string path = cell_y < mCache.size() ? GetNthCacheIndex(cell_y,false) : "";
	
	c.content_type = (cell_y < mCache.size()) ? gui_Cell_EditText : gui_Cell_None;
	
	//Defaults 0, makes !special or !normal
	c.can_edit = false;
	c.can_disclose = 0; //Default no.
	
	c.can_select = true;
	c.can_drag = false;
	c.indent_level = 0;
	c.is_disclosed = IsOpen(path);
	c.is_selected = path == mSel;
	c.string_is_resource = 0;
	c.text_val = path;

	//If the fourth to last charecter in the path is a . then it must be a file
	if( c.text_val.find_last_of('.',c.text_val.size()) == c.text_val.size()-4)
	{
		c.can_disclose = 0;
	}
	else
	{
		c.can_disclose = 1;
	}
	
	if(cell_y == mCatLocInd || cell_y == mCatLibInd )
	{
		c.text_val = pPath;
		return;
	}
	else
	{
		c.text_val = path;
	}

	//Go through the string and increase the indent everytime one see's a /
	int cut = 0;
	for(int i = 0; i < c.text_val.size(); ++i)
	{
		if(c.text_val[i] == '/' && c.text_val != pPath)
		{
			//Update where to cut
			cut = i;
			++c.indent_level;
		}
	}

	//Cut here
	c.text_val = c.text_val.substr(cut+1);
#if DEV
	//c.printCellInfo(true,true,true,true,false,true,true,false,true,0,0,0,0,1);
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
	string r = GetNthCacheIndex(cell_y,false);
	if(cell_y < mCache.size())
	{
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
		if(GetNthCacheIndex(n,false) == mSel)
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
	string r = GetNthCacheIndex(start_y,false);
	string noPrefix = GetNthCacheIndex(start_y,true);

	if(is_toggle && r == mSel)
	{
		SetSel("",noPrefix);
	}
	else
	{
		SetSel(r,noPrefix);
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
	/* Find the mSel, remove the prefix (correctly)
	* Test if the current selection is not a file
	* If it is not a file, do what it needs to do
	* Else return nothing
	*/
	vector<string>::iterator itr = find(mCache.begin(),mCache.end(),mSel);
	string tempMSel = "";
	
	if(*itr == mLocalStr || *itr == mLibraryStr)
	{
		tempMSel = GetNthCacheIndex(distance(mCache.begin(),itr),false);
	}
	else
	{
		tempMSel = GetNthCacheIndex(distance(mCache.begin(),itr),true);
	}
	
	if (!mSel.empty() && mLibrary->GetResourceType(tempMSel) == res_Directory ||
		mSel == mLocalStr || mSel == mLibraryStr)
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
	if(inMsg == GUI_FILTER_MENU_CHANGED)
	{
		mCacheValid = false;
	}

}

void WED_LibraryListAdapter::DoFilter()
{
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
	
	mCache.push_back(mLocalStr);
	mCatLocInd = mCache.size()-1;

	if(IsOpen(GetNthCacheIndex(mCatLocInd,false)))
	{
		//Goes to the data model and gets all of the root items that are local
		mLibrary->GetResourceChildren("",pack_Local,rootItems);

		//For all the root items
		for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		{
			//Add the prefix
			//s->insert(0,mLocalStr);
			//Try to find their children
			RebuildCacheRecursive(*s,pack_Local,mLocalStr);
		}
	}

	mCache.push_back(mLibraryStr);
	mCatLibInd = mCache.size()-1;

	if(IsOpen(GetNthCacheIndex(mCatLibInd,false)))
	{
		//Goes to the data model and gets all of the root items that are in the library
		mLibrary->GetResourceChildren("",pack_Library,rootItems);
	
		//For all root items
		for(vector<string>::iterator s = rootItems.begin(); s != rootItems.end(); ++s)
		{
			//Try to find their children
			RebuildCacheRecursive(*s,pack_Library,mLibraryStr);
		}
	}
	
	DoFilter();
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

void	WED_LibraryListAdapter::RebuildCacheRecursive(const string& r, int packType, const string& prefix)
{
	//Add the string to the cache.
	mCache.push_back(prefix+r);

	//If the item is open or the filter has something in it
	if(IsOpen(mCache.back()) || !mFilter.empty())
	{
		//Recuse again
		vector<string> kids;
		mLibrary->GetResourceChildren(r,packType,kids);
		for(vector<string>::iterator k = kids.begin(); k != kids.end(); ++k)
			RebuildCacheRecursive(*k, packType, prefix);
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
			{

				int type =  mLibrary->GetResourceType(noPrefix);
				mPreview->SetResource(noPrefix,type);
			}
			if(mMap)
				mMap->SetResource(noPrefix, mLibrary->GetResourceType(noPrefix));
		}
		
		mSel = s;
	}
}

string WED_LibraryListAdapter::GetNthCacheIndex (int index, bool noPrefix)
{
	string path = mCache[index];
	/*mCache by this point will look something like 
	* index 0
	* ...
	* index mCatLibInd
	* ...
	* index mCatLocInd
	* Therefore anything between 0 and mCatLibInd is underneath Library
	* and anything mCatLibInd+1 and mCatLocInd is underneath Local
	*/
	if(index < mCatLibInd)
	{
		if(noPrefix)
		{
			return path.erase(0,mLibraryStr.length());
		}
		return path;
	}
	else if(index > mCatLibInd && index < mCatLocInd)
	{
		if(noPrefix)
		{
			return path.erase(0,mLocalStr.length());
		}
		return path;
	}
	/*
	* Because none of the ranges of checking are <= or >=
	* We'll handle those cases here.
	*/
	else if(index == mCatLibInd)
	{
		if(noPrefix)
		{
			return path.erase(path.size()-1);
		}
		return path;
	}
	else if(index == mCatLocInd)
	{
		if(noPrefix)
		{
			return path.erase(path.size()-1);
		}
		return path;
	}
}

#if DEV
//Prints the contents of mOpen
void WED_LibraryListAdapter::PrintMOpen(string path)
{
	hash_map<string,int>::iterator mOpItr; 
	printf("\n ---------- \n %s \n ~~ \n", path);
	for(mOpItr = mOpen.begin(); mOpItr != mOpen.end(); mOpItr++)
	{
		printf("Path: %s", mOpItr->first.c_str());
		printf(", Open? %d \n", mOpItr->second);
	}
	printf("----------");
	printcounter++;
}
#endif