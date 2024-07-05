/* 
 * Copyright (c) 2012, Laminar Research.
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

#include "WED_LibraryFilterBar.h"
#include "WED_PackageMgr.h"
#include "WED_Colors.h"
#include "GUI_Fonts.h"
#include "GUI_Resources.h"
#include "GUI_Messages.h"

WED_LibraryFilterBar::WED_LibraryFilterBar(
	GUI_Commander * cmdr,
	WED_LibraryMgr * mLibrary)
	:
	GUI_FilterBar(cmdr, GUI_FILTER_FIELD_CHANGED, 0, "Search:", "", "Filter Libraries"),
	mLibrary(mLibrary)
{
	GUI_EnumDictionary dict;
	dict[pack_Default] = make_pair("Laminar Library", true); //Aka the default library aka pack_Default
	dict[pack_Library] = make_pair("All Libraries", true);
	dict[pack_New] = make_pair("Newly Released Items", true);

	for(int i = 0; i < gPackageMgr->CountPackages(); i++)
	{
		string temp;
		gPackageMgr->GetNthPackageName(i, temp);
		if (gPackageMgr->HasPublicItems(i))
			dict[i] = make_pair(temp, true);
	}
	SetEnumDictionary(dict, pack_Default);
}

void	WED_LibraryFilterBar::GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content)
{
	GUI_FilterBar::GetCellContent(cell_x, cell_y, the_content);

}

void	WED_LibraryFilterBar::GetEnumDictionary(
						int							cell_x,
						int							cell_y,
						GUI_EnumDictionary&			out_dictionary)
{
	GUI_FilterBar::GetEnumDictionary(cell_x, cell_y, out_dictionary);
}
