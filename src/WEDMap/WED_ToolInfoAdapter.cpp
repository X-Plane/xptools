#include "WED_ToolInfoAdapter.h"
#include "WED_MapToolNew.h"
#include "GUI_Messages.h"
#include "IPropertyObject.h"

const int COL_WIDTH = 100;
const int ROW_HEIGHT = 16;

WED_ToolInfoAdapter::WED_ToolInfoAdapter() : mTool(NULL)
{
}

WED_ToolInfoAdapter::~WED_ToolInfoAdapter()
{
}

void	WED_ToolInfoAdapter::SetTool(WED_MapToolNew * tool)
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
	
	the_content.content_type = gui_Cell_None;
	the_content.can_disclose = 0;
	the_content.can_select = 0;
	the_content.is_disclosed = 0;
	the_content.is_selected = 0;
	the_content.indent_level = 0;

	if (!mTool) return;
	
	if (cell_x % 2)
	{
		mTool->GetNthPropertyInfo(cell_x / 2, inf);
		mTool->GetNthProperty(cell_x / 2, val);
		
		switch(inf.prop_kind) {
		case prop_Int:		the_content.content_type = gui_Cell_Integer;		the_content.int_val = val.int_val;					break;
		case prop_Double:	the_content.content_type = gui_Cell_Double;			the_content.double_val = val.double_val;			break;
		case prop_String:	the_content.content_type = gui_Cell_EditText;		the_content.text_val = val.string_val;				break;
		case prop_Bool:		the_content.content_type = gui_Cell_CheckBox;		the_content.int_val = val.int_val;					break;
		case prop_Enum:		the_content.content_type = gui_Cell_Enum;			the_content.int_val = val.int_val;					break;
		case prop_EnumSet:	the_content.content_type = gui_Cell_EnumSet;		the_content.int_set_val = val.set_val;				break;
		}
		the_content.can_edit = inf.can_edit;
		
		if (inf.prop_kind == prop_Enum)
			mTool->GetNthPropertyDictItem(cell_x / 2, val.int_val,the_content.text_val);

		if (inf.prop_kind == prop_EnumSet)
		{
			the_content.text_val.clear();
			for(set<int>::iterator iter=val.set_val.begin();iter != val.set_val.end(); ++iter)
			{
				if (iter!=val.set_val.begin()) the_content.text_val += ",";
				string label;
				mTool->GetNthPropertyDictItem(cell_x/2,*iter,label);
				the_content.text_val += label;
			}
			if (the_content.text_val.empty())	the_content.text_val="-";
		}

	}
	else
	{
		mTool->GetNthPropertyInfo(cell_x / 2, inf);
		the_content.content_type = gui_Cell_EditText;
		the_content.can_edit = 0;
		the_content.text_val = inf.prop_name;		
	}
}

void	WED_ToolInfoAdapter::GetEnumDictionary(
			int							cell_x, 
			int							cell_y, 
			map<int, string>&			out_dictionary)
{
	out_dictionary.clear();
	
	if (mTool)
		mTool->GetNthPropertyDict(cell_x / 2, out_dictionary);

}			
void	WED_ToolInfoAdapter::AcceptEdit(
			int							cell_x,
			int							cell_y,
			const GUI_CellContent&		the_content)
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

int		WED_ToolInfoAdapter::TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content)
{
	return 0;
}

int			WED_ToolInfoAdapter::GetColCount(void)
{
	return mTool ? (mTool->CountProperties() * 2) : 0;
}

int			WED_ToolInfoAdapter::GetRowCount(void)
{
	return 1;
}

int			WED_ToolInfoAdapter::GetCellLeft (int n)
{
	return n * COL_WIDTH;
}

int			WED_ToolInfoAdapter::GetCellRight(int n)
{
	return (n+1) * COL_WIDTH;
}

int			WED_ToolInfoAdapter::GetCellWidth(int n)
{
	return COL_WIDTH;
}

int			WED_ToolInfoAdapter::GetCellBottom(int n)
{
	return n * ROW_HEIGHT;
}

int			WED_ToolInfoAdapter::GetCellTop	 (int n)
{
	return (n+1) * ROW_HEIGHT;
}

int			WED_ToolInfoAdapter::GetCellHeight(int n)
{
	return ROW_HEIGHT;
}

int			WED_ToolInfoAdapter::ColForX(int n)
{
	return n / COL_WIDTH;
}

int			WED_ToolInfoAdapter::RowForY(int n)
{
	return n / ROW_HEIGHT;
}

void		WED_ToolInfoAdapter::SetCellWidth (int n, int w)
{
}

void		WED_ToolInfoAdapter::SetCellHeight(int n, int h)
{
}

