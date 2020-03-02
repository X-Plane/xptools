/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_ToolInfoAdapter.h"
#include "WED_MapToolNew.h"
#include "GUI_Messages.h"
#include "GUI_Fonts.h"
#include "WED_EnumSystem.h"
#include "IPropertyObject.h"

#define OUR_FONT font_UI_Small

WED_ToolInfoAdapter::WED_ToolInfoAdapter(int height) : mTool(NULL), mRowHeight(height)
{
}

WED_ToolInfoAdapter::~WED_ToolInfoAdapter()
{
}

void	WED_ToolInfoAdapter::SetTool(IPropertyObject * tool)
{
	mTool = tool;
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void	WED_ToolInfoAdapter::GetCellContent(
			int							cell_x,
			int							cell_y,
			GUI_CellContent&			the_content)
{
	PropertyInfo_t	inf;
	PropertyVal_t	val;

	the_content.string_is_resource = 0;
	the_content.content_type = gui_Cell_None;
	the_content.can_delete = false;
	the_content.can_disclose = 0;
	the_content.can_select = 0;
	the_content.is_disclosed = 0;
	the_content.is_selected = 0;
	the_content.can_drag = 0;
	the_content.indent_level = 0;
	the_content.bool_val = gui_Bool_Check;
	the_content.bool_partial = 0;
	if (!mTool) return;

	if (cell_x == mTool->CountProperties() * 2) return;

	if (cell_x % 2)
	{
		char buf[100], fmt[10];

		mTool->GetNthPropertyInfo(cell_x / 2, inf);
		mTool->GetNthProperty(cell_x / 2, val);

		switch(inf.prop_kind) {
		case prop_Int:
			the_content.content_type = gui_Cell_Integer;
			the_content.int_val = val.int_val;
			sprintf(fmt,"%%%dd", inf.digits);
			sprintf(buf,fmt,val.int_val);
			the_content.text_val = buf;
			break;
		case prop_Double:
			the_content.content_type = gui_Cell_Double;
			the_content.double_val = val.double_val;
			sprintf(fmt,"%%%d.%dlf",inf.digits, inf.decimals);
			sprintf(buf,fmt,val.double_val);
			the_content.text_val = buf;
			break;
		case prop_String:	the_content.content_type = gui_Cell_EditText;		the_content.text_val = val.string_val;		break;
		case prop_FilePath:	the_content.content_type = gui_Cell_FileText;		the_content.text_val = val.string_val;		break;
		case prop_TaxiSign:	the_content.content_type = gui_Cell_TaxiText;		the_content.text_val = val.string_val;		break;
		case prop_Bool:		the_content.content_type = gui_Cell_CheckBox;		the_content.int_val = val.int_val;			break;
		case prop_Enum:		the_content.content_type = gui_Cell_Enum;			the_content.int_val = val.int_val;			break;
		case prop_EnumSet:	the_content.content_type = gui_Cell_EnumSet;		the_content.int_set_val = val.set_val;		break;
		}
		the_content.can_edit = inf.can_edit;

		if (inf.prop_kind == prop_Enum)
			mTool->GetNthPropertyDictItem(cell_x / 2, val.int_val,the_content.text_val);

		if (inf.prop_kind == prop_EnumSet)
		{
			the_content.content_type = (inf.domain == LinearFeature ? gui_Cell_LineEnumSet : gui_Cell_EnumSet);
			the_content.text_val.clear();
			for(set<int>::iterator iter=val.set_val.begin();iter != val.set_val.end(); ++iter)
			{
				if (iter!=val.set_val.begin()) the_content.text_val += ",";
				string label;
				mTool->GetNthPropertyDictItem(cell_x / 2,*iter,label);
#if 0			// for now print the full style name, not the icon - as the icons don't allow to distinguish some wide lines from regular ones
				if (ENUM_Domain(*iter) == LinearFeature)
				{
					label = ENUM_Name(*iter);
					label += ".png";
					the_content.string_is_resource = 1;
				}
#endif
				the_content.text_val += label;
			}
			if (the_content.text_val.empty())	the_content.text_val="None";
			if(inf.exclusive && the_content.int_set_val.empty()) the_content.int_set_val.insert(0);
		}

	}
	else
	{
		mTool->GetNthPropertyInfo(cell_x / 2, inf);
		the_content.content_type = gui_Cell_EditText;
		the_content.can_delete = false;
		the_content.can_edit = 0;
		the_content.text_val = inf.prop_name;
		the_content.indent_level = 1;
	}
}

void	WED_ToolInfoAdapter::GetEnumDictionary(
			int							cell_x,
			int							cell_y,
			GUI_EnumDictionary&			out_dictionary)
{
	out_dictionary.clear();
	PropertyInfo_t inf;
	inf.exclusive=false;

	if (mTool)
	{
		mTool->GetNthPropertyDict(cell_x / 2, out_dictionary);
		mTool->GetNthPropertyInfo(cell_x / 2, inf);
	}
	if(inf.exclusive)
		out_dictionary.insert(GUI_EnumDictionary::value_type(0,make_pair(string("None"),true)));
}
void	WED_ToolInfoAdapter::AcceptEdit(
			int							cell_x,
			int							cell_y,
			const GUI_CellContent&		the_content,
			int							apply_all)
{
	if (mTool == NULL) return;
	PropertyInfo_t	inf;
	PropertyVal_t	val;
	mTool->GetNthPropertyInfo(cell_x / 2,inf);
	switch(inf.prop_kind) {
	case prop_Int:
		val.prop_kind = prop_Int;
		val.int_val = the_content.int_val;
		break;
	case prop_Double:
		val.prop_kind = prop_Double;
		val.double_val = the_content.double_val;
		break;
	case prop_String:
		val.prop_kind = prop_String;
		val.string_val = the_content.text_val;
		break;
	case prop_TaxiSign:
		val.prop_kind = prop_TaxiSign;
		val.string_val = the_content.text_val;
		break;
	case prop_FilePath:
		val.prop_kind = prop_FilePath;
		val.string_val = the_content.text_val;
		break;
	case prop_Bool:
		val.prop_kind = prop_Bool;
		val.int_val = the_content.int_val;
		break;
	case prop_Enum:
		val.prop_kind = prop_Enum;
		val.int_val = the_content.int_val;
		break;
	case prop_EnumSet:
		val.prop_kind = prop_EnumSet;
		if (inf.exclusive)
		{
			val.set_val.clear();
			if (the_content.int_val != 0)
				val.set_val.insert(the_content.int_val);
		} else
			val.set_val = the_content.int_set_val;
		break;
	}
	mTool->SetNthProperty(cell_x / 2, val);
	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
}

void	WED_ToolInfoAdapter::ToggleDisclose(
			int							cell_x,
			int							cell_y)
{
}

void	WED_ToolInfoAdapter::DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							button,
						int							bounds[4])
{
}

void	WED_ToolInfoAdapter::SelectionStart(
						int							clear)
{
}

int		WED_ToolInfoAdapter::SelectGetExtent(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y)
{
	return 0;
}

int		WED_ToolInfoAdapter::SelectGetLimits(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y)
{
	return 0;
}


void	WED_ToolInfoAdapter::SelectRange(
						int							start_x,
						int							start_y,
						int							end_x,
						int							end_y,
						int							is_toggle)
{
}

void	WED_ToolInfoAdapter::SelectionEnd(void)
{
}

int		WED_ToolInfoAdapter::SelectDisclose(
						int							open_it,
						int							all)
{
	return 0;
}

int		WED_ToolInfoAdapter::TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content)
{
	return 0;
}

int		WED_ToolInfoAdapter::DoubleClickCell(
						int							cell_x,
						int							cell_y)
{
	return 0;
}

int			WED_ToolInfoAdapter::GetColCount(void)
{
	return mTool ? (mTool->CountProperties() * 2) + 1 : 1;
}

int			WED_ToolInfoAdapter::GetRowCount(void)
{
	return 1;
}

int			WED_ToolInfoAdapter::GetCellLeft (int n)
{
	int t = 0;
	for (int i = 0; i < n; ++i)
	t += GetCellWidth(i);
	return t;
}

int			WED_ToolInfoAdapter::GetCellRight(int n)
{
	int t = 0;
	for (int i = 0; i <= n; ++i)
	t += GetCellWidth(i);
	return t;
}

int			WED_ToolInfoAdapter::GetCellWidth(int n)
{
	if (mTool == NULL) return 10;
	if (n >= mTool->CountProperties() * 2) return 10;
	PropertyInfo_t	inf;
	const char * zero = "0";

	mTool->GetNthPropertyInfo(n / 2, inf);

//	PropertyDict_t	dict;

	if (!inf.prop_name.empty() && inf.prop_name[0] == '.') return 1;

	if (n % 2)
	switch(inf.prop_kind) {
	// Check runway-creation tool if you retune these...it is the most space constrained up top!
	case prop_Int:
	case prop_Double:		return inf.digits * GUI_MeasureRange(OUR_FONT, zero,zero+1) + 10;
	case prop_String:		return 200;
	case prop_FilePath:		return 200;
	case prop_Bool:			return 25;
	case prop_Enum:
	case prop_EnumSet:		
							switch (inf.domain)
							{
								case ATCVehicleClass:
								case RampOperationType:
								case ATCTrafficType: return 100;
								case ShowLevel:
								case ExclusionTypes:
								case ATCServiceTruckType:
								case Light_Fixt:
								case Sign_Size:
								case LinearFeature: return 150;
								case ATCIcaoWidth:
								case Edge_Lights:   return 50;
								
								default:            return 75;
							}
	case prop_TaxiSign:		return 150;
//		mTool->GetNthPropertyDict(n / 2, dict);
//		for(PropertyDict_t::iterator d = dict.begin(); d != dict.end(); ++d)
//			w = max(w,(int) GUI_MeasureRange(OUR_FONT, &*d->second.begin(),&*d->second.end())+20);
//		return w;
	default:				return 50;
	}
	else
		return GUI_MeasureRange(OUR_FONT, &*inf.prop_name.begin(), &*inf.prop_name.end()) + 18;
}

int			WED_ToolInfoAdapter::GetCellBottom(int n)
{
	return n * mRowHeight;
}

int			WED_ToolInfoAdapter::GetCellTop	 (int n)
{
	return (n+1) * mRowHeight;
}

int			WED_ToolInfoAdapter::GetCellHeight(int n)
{
	return mRowHeight;
}

int			WED_ToolInfoAdapter::ColForX(int n)
{
	if (n < 0) return -1;
	int cc = GetColCount();
	for (int c = 0; c < cc; ++c)
	{
		int w = GetCellWidth(c);
		if (n < w) return c;
		n -= w;
	}
	return cc-1;
}

int			WED_ToolInfoAdapter::RowForY(int n)
{
	if(n < 0) return -1; //Fixes being able to click an imaginary last cell and select the real last cell
	return n / mRowHeight;
}

bool		WED_ToolInfoAdapter::CanSetCellWidth(void) const
{
	return false;
}

bool		WED_ToolInfoAdapter::CanSetCellHeight(void) const
{
	return false;
}

void		WED_ToolInfoAdapter::SetCellWidth (int n, int w)
{
}

void		WED_ToolInfoAdapter::SetCellHeight(int n, int h)
{
}

