#include "WED_PropertyTable.h"
#include "WED_Archive.h"
#include "WED_Thing.h"
#include "WED_Select.h"
#include "WED_Messages.h"
#include "GUI_Messages.h"

inline int count_strs(const char ** p) { int n = 0; while(*p) ++p, ++n; return n; }

WED_PropertyTable::WED_PropertyTable(
									WED_Thing *				root,
									WED_Select *			selection,
									const char **			col_names,
									int *					def_col_widths)
	:	mArchive(root->GetArchive()), mEntity(root->GetID()), mSelect(selection->GetID()),
		mGeometry(count_strs(col_names),this,def_col_widths,20)
{
	while(*col_names)
		mColNames.push_back(*col_names++);
	selection->AddListener(this);
}

WED_PropertyTable::~WED_PropertyTable()
{
}

void	WED_PropertyTable::GetCellContent(
						int							cell_x, 
						int							cell_y, 
						GUI_CellContent&			the_content)
{
	the_content.content_type = gui_Cell_None;
	
	WED_Thing * t = FetchNth(cell_y);
	
	WED_Select * s = SAFE_CAST(WED_Select,mArchive->Fetch(mSelect));
	
	int idx = t->FindProperty(mColNames[cell_x].c_str());
	if (idx == -1) return;
	
	PropertyInfo_t	inf;
	PropertyVal_t	val;
	t->GetNthPropertyInfo(idx,inf);
	t->GetNthProperty(idx, val);
	
	the_content.is_selected = s->IsSelected(t);
	
	switch(inf.prop_kind) {
	case prop_Int:
		the_content.content_type = gui_Cell_Integer;
		the_content.int_val = val.int_val;
		break;
	case prop_Double:
		the_content.content_type = gui_Cell_Double;
		the_content.double_val = val.double_val;
		break;
	case prop_String:
		the_content.content_type = gui_Cell_EditText;
		the_content.text_val = val.string_val;
		break;
	case prop_Bool:
		the_content.content_type = gui_Cell_CheckBox;
		the_content.int_val = val.int_val;
		break;
	case prop_Enum:
		the_content.content_type = gui_Cell_Enum;
		t->GetNthPropertyDictItem(idx, val.int_val,the_content.text_val);
		break;
	}
	
	the_content.can_edit = inf.can_edit;
	the_content.can_disclose = (cell_x == 0) && t->CountChildren() > 0;
	the_content.is_disclosed = 	mOpen[t->GetID()] != 0 && the_content.can_disclose;
	the_content.indent_level = GetThingDepth(t);
	#if !DEV
		enforce entity locking here?
	#endif
}

void	WED_PropertyTable::GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						map<int, string>&			out_dictionary)
{
}

void	WED_PropertyTable::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content)
{
	WED_Thing * t = FetchNth(cell_y);	
	int idx = t->FindProperty(mColNames[cell_x].c_str());
	if (idx == -1) return;
	PropertyInfo_t	inf;
	PropertyVal_t	val;
	t->GetNthPropertyInfo(idx,inf);	
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
//		t->GetNthPropertyDictItem(idx, val.int_val,the_content.text_val);
		break;
	}
	t->SetNthProperty(idx, val);
}

void	WED_PropertyTable::ToggleDisclose(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(cell_y);
	if (t)
		mOpen[t->GetID()] = 1 - mOpen[t->GetID()];
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void	WED_PropertyTable::SelectCell(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(cell_y);
	WED_Select * s = SAFE_CAST(WED_Select,mArchive->Fetch(mSelect));
	if (t && s)
	{
		s->StartCommand("Change Selection.");
		s->Select(t);
		s->CommitCommand();
	}
}

void	WED_PropertyTable::SelectCellToggle(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(cell_y);
	WED_Select * s = SAFE_CAST(WED_Select,mArchive->Fetch(mSelect));
	if (t && s)
	{
		s->StartCommand("Change Selection.");
		s->Toggle(t);
		s->CommitCommand();
	}
}

void	WED_PropertyTable::SelectCellExtend(
						int							cell_x,
						int							cell_y)
{
	#if !DEV
		hello
	#endif
}




WED_Thing *	WED_PropertyTable::FetchNth(int row)
{
	WED_Thing * root = SAFE_CAST(WED_Thing,mArchive->Fetch(mEntity));
	if (!root) return NULL;
	// Ben says: tables are indexed bottom=0 to match OGL coords.
	// INvert our numbers here because we really need to count up when we traverse the tree!
	row = CountRows() - row - 1;
	return FetchNthRecursive(root, row);
}

int			WED_PropertyTable::GetThingDepth(WED_Thing * d)
{
	WED_Thing * root = SAFE_CAST(WED_Thing,mArchive->Fetch(mEntity));
	if (!root) return 0;

	int ret = 0;
	while (d)
	{
		if (d == root) break;
		d = d->GetParent();
		++ret;
	}
	return ret;	
}


WED_Thing *	WED_PropertyTable::FetchNthRecursive(WED_Thing * e, int& row)
{
	if (e == NULL) return NULL;
	
	if (row == 0) return e;
	--row;
	
	if (mOpen[e->GetID()] != 0)	
	for (int n = 0; n < e->CountChildren(); ++n)
	{
		WED_Thing * c = FetchNthRecursive(e->GetNthChild(n), row);
		if (c) return c;
	}
	return NULL;
}

int			WED_PropertyTable::CountRows(void)
{
	WED_Thing * root = SAFE_CAST(WED_Thing,mArchive->Fetch(mEntity));
	CountRowsRecursive(root);
}

int			WED_PropertyTable::CountRowsRecursive(WED_Thing * e)
{
	if (e == NULL) return 0;
	int total = 1;
	int cc = e->CountChildren();
	if (mOpen[e->GetID()] == 0)	cc = 0;	
	for (int c = 0; c < cc; ++c)
	{
		total += CountRowsRecursive(e->GetNthChild(c));
	}
	return total;
}

void	WED_PropertyTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	if (inMsg == msg_SelectionChanged)		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
}


//------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------


 WED_PropertyTableHeader::WED_PropertyTableHeader(
				const char **			col_names,
				int *					def_col_widths)
{
	while(*col_names)
	mColNames.push_back(*col_names++);
}

WED_PropertyTableHeader::~WED_PropertyTableHeader()
{
}

void	WED_PropertyTableHeader::GetHeaderContent(
				int							cell_x, 
				GUI_HeaderContent&			the_content)
{
	the_content.title = mColNames[cell_x];
	the_content.can_resize = true;
}

