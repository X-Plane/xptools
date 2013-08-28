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

#include "WED_FilterBar.h"
#include "WED_PackageMgr.h"
#include "WED_Colors.h"
#include "GUI_Fonts.h"
#include "GUI_Resources.h"

static int cols[2] = { 100, 100 };

WED_FilterBar::WED_FilterBar(
			GUI_Commander *	cmdr,
			intptr_t		in_msg, 
			intptr_t		in_param, 
			const string&	in_label, 
			const string&	in_def,
			WED_LibraryMgr *mLibrary,
			bool havePacks) :
	GUI_Table(1),
	GUI_SimpleTableGeometry(2, cols, GUI_GetImageResourceHeight("property_bar.png") / 2),
	mCurPakVal(pack_Library), //aka -2
	mCurPak(""),
	mTextTable(cmdr,0,1),
	mLabel(in_label),
	mText(in_def),
	mMsg(in_msg),
	mParam(in_param),
	mLibrary(mLibrary),
	mHavePacks(havePacks)
{
	this->SetGeometry(this);
	this->SetContent(&mTextTable);
	mTextTable.SetProvider(this);
	this->SizeShowAll();
	mTextTable.SetParentTable(this);
	mTextTable.AddListener(this);
	mTextTable.SetImage("property_bar.png", 2);
	mTextTable.SetColors(
				WED_Color_RGBA(wed_Table_Gridlines),
				WED_Color_RGBA(wed_Table_Select),
				WED_Color_RGBA(wed_Table_Text),
				WED_Color_RGBA(wed_PropertyBar_Text),
				WED_Color_RGBA(wed_Table_Drag_Insert),
				WED_Color_RGBA(wed_Table_Drag_Into));
	mTextTable.SetFont(font_UI_Small);

	this->AddListener(this);
	
}

int			WED_FilterBar::GetColCount(void)
{
	return 2;
}

int			WED_FilterBar::GetRowCount(void)
{
	if(mHavePacks == false)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

void	WED_FilterBar::GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content)
{
	/* Filter Bar Table
	* 0        1
	* Lable | Text Field									1
	* Lable | Enum Dictionary (Build from PackageManager)	0
	*/
	//Cell 0,0 and 1,0
	if(cell_y == 1 || mHavePacks == false)
	{
		the_content.content_type=gui_Cell_EditText;
		the_content.can_edit=(cell_x==1);
		the_content.can_disclose=0;
		the_content.can_select=0;
		the_content.can_drag=0;

		the_content.is_disclosed=0;
		the_content.is_selected=0;
		the_content.indent_level=0;

		if(cell_x == 0)
			the_content.text_val = mLabel;
		else
			the_content.text_val = mText;
		the_content.string_is_resource=0;
	}

	if(cell_y == 0 && mHavePacks == true)
	{
		//Label
		if(cell_x == 0)
		{
			the_content.content_type=gui_Cell_EditText;
			the_content.can_edit=0;
			the_content.can_disclose=0;
			the_content.can_select=0;
			the_content.can_drag=0;

			the_content.is_disclosed=0;
			the_content.is_selected=0;
			the_content.indent_level=0;
		
			the_content.text_val = "Choose Pack:";
			the_content.string_is_resource=0;
		}
		//Enum
		if(cell_x == 1)
		{
			the_content.content_type=gui_Cell_Enum;
			the_content.can_edit=1;
			the_content.can_disclose=0;
			the_content.can_select=0;
			the_content.can_drag=0;

			the_content.is_disclosed=0;
			the_content.is_selected=0;
			the_content.indent_level=0;
			the_content.int_val = mCurPakVal;

			//switch on the current int_val
			//Special cases for Local, Library, and All
			//Default for any other value
			switch(mCurPakVal)
			{
				case pack_Library: the_content.text_val = "Library"; break;
				case pack_Default: the_content.text_val = "Laminar Library"; break;
				default: gPackageMgr->GetNthPackageName(mCurPakVal,the_content.text_val); break;
			}
			the_content.string_is_resource=0;
		}
	}
}

void	WED_FilterBar::GetEnumDictionary(
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
	* To make something the default enum choice make sure you update the inilized mCurPakVal this AND in the LibraryAdapter
	*/
	out_dictionary.insert(GUI_EnumDictionary::value_type(i+pack_Library,make_pair("Library",true)));
	out_dictionary.insert(GUI_EnumDictionary::value_type(i+pack_Default,make_pair("Laminar Library",true))); //Aka the default library aka pack_Default

	while(i < gPackageMgr->CountPackages())
	{
		string temp = "";
		gPackageMgr->GetNthPackageName(i,temp);

		if(mLibrary->DoesPackHaveLibraryItems(i) == true)
		{
			out_dictionary.insert(GUI_EnumDictionary::value_type(i,make_pair(temp,true)));
		}
		i++;
	}

}

void	WED_FilterBar::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content,
						int							apply_all)
{
	if(cell_x == 1 && cell_y == 0 && mHavePacks)
	{
		mCurPakVal = the_content.int_val;
		mCurPak = the_content.text_val;

		BroadcastMessage(mMsg, mParam);
	}
	if(cell_x == 1 && cell_y == 1 || mHavePacks == false)
	{
		if(mText != the_content.text_val)
		{
			mText = the_content.text_val;
			BroadcastMessage(mMsg, mParam);
		}
	}
}
