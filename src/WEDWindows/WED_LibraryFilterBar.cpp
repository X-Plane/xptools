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
	GUI_FilterBar(cmdr, GUI_FILTER_FIELD_CHANGED, 0, "Search:", "", true, pack_Default),
	mLibrary(mLibrary)
{
}

void	WED_LibraryFilterBar::GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content)
{
	GUI_FilterBar::GetCellContent(cell_x, cell_y, the_content);
	/* Filter Bar Table
	* 0        1
	* Lable | Text Field									1
	* Lable | Enum Dictionary (Build from PackageManager)	0
	*/
	//Cell 0,0 and 1,0
	if(cell_y == 1 || GUI_FilterBar::GetHaveEnumDict() == false)
	{
		//if(cell_x == 0)
		//	the_content.text_val = mLabel;
		//else
		//	the_content.text_val = mText;
		//the_content.string_is_resource=0;
	}

	if(cell_y == 0 && GUI_FilterBar::GetHaveEnumDict() == true)
	{
		//Label
		if(cell_x == 0)
		{
			the_content.text_val = "Filter Libraries:";
		}
		//Enum
		if(cell_x == 1)
		{
			the_content.content_type=gui_Cell_Enum;
			the_content.int_val = GetEnumValue();

			//switch on the current int_val
			//Special cases for Local, Library, and All
			//Default for any other value
			int cur_val = GUI_FilterBar::GetEnumValue();
			switch(cur_val)
			{
				case pack_Library: the_content.text_val = "All Libraries"; break;
				case pack_Default: the_content.text_val = "Laminar Library"; break;
				case pack_New:     the_content.text_val = "Newly Released Items"; break;
				default: 	gPackageMgr->GetNthPackageName(GetEnumValue(),the_content.text_val);
			}
			the_content.string_is_resource=0;
		}
	}
}

void	WED_LibraryFilterBar::GetEnumDictionary(
						int							cell_x,
						int							cell_y,
						GUI_EnumDictionary&			out_dictionary)
{
	/* Force the dictionary to have Local, Library, and All
	* their numbers correspond with the enum values found in the
	* Library Managers
	*
	* Loop through the number of packages and add them to the dictionary
	*/
	int i = 0;

	/*An important note!
	* To make something the default enum choice make sure you update the inilized mCurEnumVal this AND in the LibraryAdapter
	*/
	out_dictionary.insert(GUI_EnumDictionary::value_type(i+pack_Default,make_pair("Laminar Library",true))); //Aka the default library aka pack_Default
	out_dictionary.insert(GUI_EnumDictionary::value_type(i+pack_Library,make_pair("All Libraries",true)));
	out_dictionary.insert(GUI_EnumDictionary::value_type(i+pack_New,make_pair("Newly Released Items",true)));

	while(i < gPackageMgr->CountPackages())
	{
		string temp = "";
		gPackageMgr->GetNthPackageName(i,temp);

		if(gPackageMgr->IsPackagePublicItems(i))
		{
			out_dictionary.insert(GUI_EnumDictionary::value_type(i,make_pair(temp,true)));
		}
		i++;
	}

}
