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

#include "WED_LibraryPane.h"
#include "WED_PackageMgr.h"
#include "GUI_ScrollerPane.h"
#include "WED_Colors.h"
#include "WED_LibraryFilterBar.h"
#include "GUI_Resources.h"
#include "GUI_Messages.h"
#include "WED_Menus.h"

WED_LibraryPane::WED_LibraryPane(GUI_Commander * commander, WED_LibraryMgr * mgr) :
	GUI_Commander(commander),
	mTextTable(this,12,0),
	mLibraryList(mgr)
{
//	int bounds[4] = { 0, 0, 100, 100 };
//	SetBounds(bounds);
	mScroller = new GUI_ScrollerPane(1,1);

	mScroller->SetParent(this);
	mScroller->Show();
	mScroller->SetSticky(1,1,1,1);

	//Library Pane's TextTable's provider is set to be the Library List Adapter
	mTextTable.SetProvider(&mLibraryList);
	
	mTextTable.SetGeometry(&mLibraryList);

	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_Table_SelectText),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetTextFieldColors(
				WED_Color_RGBA(wed_TextField_Text),
				WED_Color_RGBA(wed_TextField_Hilite),
				WED_Color_RGBA(wed_TextField_Bkgnd),
				WED_Color_RGBA(wed_TextField_FocusRing));

	mTable = new GUI_Table(true);
	mTable->SetGeometry(&mLibraryList);
	mTable->SetContent(&mTextTable);
	mTable->SetParent(mScroller);
	mTable->Show();
	mScroller->PositionInContentArea(mTable);
	mScroller->SetContent(mTable);
	mTextTable.SetParentTable(mTable);

	mTextTable.AddListener(mTable);				// Table listens to text table to know when content changes in a resizing way
	mLibraryList.AddListener(mTable);			// Table listens to actual property content to know when data itself changes

	GUI_EnumDictionary dict;
	dict[pack_Default] = make_pair("Laminar Library", true); //Aka the default library aka pack_Default
	dict[pack_Library] = make_pair("All Libraries", true);
	dict[pack_New] = make_pair("Newly Released Items", true);

	for (int i = 0; i < gPackageMgr->CountPackages(); i++)
	{
		string temp;
		gPackageMgr->GetNthPackageName(i, temp);
		if (gPackageMgr->HasPublicItems(i))
			dict[i] = make_pair(temp, true);
	}

	mFilter = new GUI_FilterBar(this, GUI_FILTER_FIELD_CHANGED, 0, "Search:", "", dict, pack_Default, "Show only:");
	// mFilter->SetEnumDictionary(dict, pack_Default);

	mFilter->Show();
	mFilter->SetParent(this);
	mFilter->AddListener(this);
	mFilter->SetSticky(1,0,1,1);
	this->PackPane(mFilter,gui_Pack_Top);
	this->PackPane(mScroller, gui_Pack_Center);

					#if DEV
					//PrintDebugInfo();
					#endif
}

WED_LibraryPane::~WED_LibraryPane()
{
}

int	WED_LibraryTextTable::CellGetHelpTip(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, string& tip)
{
	if (cell_x == 0)
	{
		auto path = mAdap->GetNthCacheIndex(cell_y, true);
		if (mAdap->mLibrary->GetResourceType(path) >= res_Object)
		{
			if (mAdap->mLibrary->IsResourceLocal(path))
				tip = path;
			else
			{
				tip = mAdap->mLibrary->GetResourcePath(path);
				int i = tip.rfind("default scenery");
				if (i > 0)
					i += sizeof("default scenery");
				else
					i = tip.rfind("Custom Scenery") + sizeof("Custom Scenery");
				tip.erase(0, i);
			}
			return 1;
		}
	}
	else
	{
		tip.clear();
		if (mAdap->mCache[cell_y].hasSeasons) tip = ",seaonally";
		if (mAdap->mCache[cell_y].hasRegions) tip += ",regionally";
		if (mAdap->mCache[cell_y].variants)   tip += ",randomly";
		if (tip.length() > 0)
		{
			tip = string("Varies ") + tip.substr(1);
			return 1;
		}
	}
	return 0;
}

int		WED_LibraryPane::MouseMove(int x, int y)
{
	int b[4];
	GetBounds(b);
	if(b[2] - b[0] > 0 && b[2] - b[0] < 100)
	{
		DispatchHandleCommand(wed_autoOpenLibPane);
	}
	return 1;
}

void	WED_LibraryPane::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	if(inMsg == GUI_FILTER_FIELD_CHANGED)
		mLibraryList.SetFilter(mFilter->GetText(),mFilter->GetEnumValue());
}

void		WED_LibraryPane::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Packer::SetBounds(x1, y1, x2, y2);
	int w = x2 - x1;
	if (w > 220)
		mLibraryList.SetCellWidth(0, max(220, w - 70));
}

void		WED_LibraryPane::SetBounds(int inBounds[4])
{
	GUI_Packer::SetBounds(inBounds);
	int w = inBounds[2] - inBounds[0];
	if (w > 220)
		mLibraryList.SetCellWidth(0, max(220, w - 70));
}
