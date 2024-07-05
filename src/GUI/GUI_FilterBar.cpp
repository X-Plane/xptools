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

#include "GUI_FilterBar.h"
#include "WED_Colors.h"
#include "GUI_Fonts.h"
#include "GUI_Resources.h"

static int cols[2] = { 100, 100 };

GUI_FilterBar::GUI_FilterBar(
			GUI_Commander *	cmdr,
			intptr_t		in_msg, 
			intptr_t		in_param, 
			const string&	in_label,
			const string&	in_def,
			const GUI_EnumDictionary& dict,
			int				enum_def,
			const			string& enum_label) :
	GUI_Table(1),
	GUI_SimpleTableGeometry(2, cols, GUI_GetImageResourceHeight("property_bar.png") / 2),
	mTextTable(cmdr,0,1),
	mLabel(in_label),
	mEnumLabel(enum_label),
	mText(in_def),
	mDict(dict),
	mCurEnumVal(enum_def),
	mMsg(in_msg),
	mParam(in_param)
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
	auto l1 = GUI_MeasureRange(font_UI_Basic, in_label.c_str(), in_label.c_str() + in_label.length());
	auto l2 = GUI_MeasureRange(font_UI_Basic, enum_label.c_str(), enum_label.c_str() + enum_label.length());
	SetCellWidth(0, max(l1, l2) + 3);
}

int			GUI_FilterBar::GetColCount(void)
{
	return 2;
}

int			GUI_FilterBar::GetRowCount(void)
{
	return mDict.empty() ? 1 : 2;
}

void	GUI_FilterBar::GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content)
{
	/* Filter Bar Table
	* 0        1
	* Label | Text Field
	* Label | Enum Dict	
	*/
	the_content.content_type = gui_Cell_EditText;
	the_content.can_delete = false;
	the_content.can_edit = (cell_x == 1);
	the_content.can_disclose = 0;
	the_content.can_select = 0;
	the_content.can_drag = 0;

	the_content.is_disclosed = 0;
	the_content.is_selected = 0;
	the_content.indent_level = 0;
	the_content.string_is_resource = 0;

	if(cell_y == 1 || mDict.empty())
		the_content.text_val = (cell_x == 0) ? mLabel : mText;
	else if(cell_x == 0)
		the_content.text_val = mEnumLabel;
	else
	{
		the_content.content_type = gui_Cell_Enum;
		the_content.int_val = mCurEnumVal;
		the_content.text_val = mDict[mCurEnumVal].first;
	}
}

void	GUI_FilterBar::ClearFilter()
{
	mTextTable.KillEditing(false);
	mText.clear();
	BroadcastMessage(mMsg, mParam);
	Refresh();
}

void	GUI_FilterBar::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content,
						int							apply_all)
{
	/* Filter Bar Table
	* 0        1
	* Lable | Text Field		1
	* Lable | Enum Dictionary	0
	*/

	if (cell_x == 1)
	{
		if (cell_y == 1 || mDict.empty())
		{
			if (mText != the_content.text_val)
			{
				mText = the_content.text_val;
				BroadcastMessage(mMsg, mParam);
			}
		}
		else
		{
			mCurEnumVal = the_content.int_val;
			BroadcastMessage(mMsg, mParam);
		}
	}
}
