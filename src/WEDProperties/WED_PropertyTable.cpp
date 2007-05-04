#include "WED_PropertyTable.h"
#include "WED_Archive.h"
#include "WED_Thing.h"
#include "WED_Select.h"
#include "WED_Messages.h"
#include "GUI_Messages.h"

inline int count_strs(const char ** p) { if (!p) return 0; int n = 0; while(*p) ++p, ++n; return n; }

WED_PropertyTable::WED_PropertyTable(
									WED_Thing *				root,
									WED_Select *			selection,
									const char **			col_names,
									int *					def_col_widths,
									int						vertical,
									int						dynamic_cols,
									int						sel_only,
									const char **			filter)
	:	GUI_SimpleTableGeometry(count_strs(col_names),def_col_widths,20),
	mVertical(vertical),
	mDynamicCols(dynamic_cols),
	mSelOnly(sel_only),
	mArchive(root->GetArchive()), mEntity(root->GetID()), mSelect(selection->GetID())
{
	if (col_names)
	while(*col_names)
		mColNames.push_back(*col_names++);
		
	if (filter)
	while (*filter)
		mFilter.insert(*filter++);
//	selection->AddListener(this);
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
	the_content.can_edit = 0;
	the_content.can_disclose = 0;
	the_content.can_select = 0;
	the_content.is_disclosed = 0;
	the_content.is_selected = 0;
	the_content.indent_level = 0;
	
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	if (t == NULL) return;
	
	WED_Select * s = SAFE_CAST(WED_Select,mArchive->Fetch(mSelect));
	
	int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
	if (idx == -1) return;
	
	PropertyInfo_t	inf;
	PropertyVal_t	val;
	t->GetNthPropertyInfo(idx,inf);
	t->GetNthProperty(idx, val);
	
	the_content.can_select = 1;
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
		the_content.int_val = val.int_val;
		break;
	case prop_EnumSet:	
		the_content.content_type = gui_Cell_EnumSet;		
		the_content.int_set_val = val.set_val;
		the_content.text_val.clear();
		for(set<int>::iterator iter=val.set_val.begin();iter != val.set_val.end(); ++iter)
		{
			if (iter!=val.set_val.begin()) the_content.text_val += ",";
			string label;
			t->GetNthPropertyDictItem(idx,*iter,label);
			the_content.text_val += label;
		}		
		if (the_content.text_val.empty())	the_content.text_val="-";
		break;		
	}
	
	the_content.can_edit = inf.can_edit;
	the_content.can_disclose = !mVertical && (cell_x == 0) && t->CountChildren() > 0;
	the_content.is_disclosed = 	mOpen[t->GetID()] != 0 && the_content.can_disclose;
	the_content.indent_level = (!mVertical && cell_x == 0) ? GetThingDepth(t) : 0;	/// as long as "cell 0" is the diclose level, might as well have it be the indent level too.
	#if !DEV
		enforce entity locking here?
	#endif
}

void	WED_PropertyTable::GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						map<int, string>&			out_dictionary)
{
	out_dictionary.clear();
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	
	int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
	if (idx == -1) return;	
	
	t->GetNthPropertyDict(idx, out_dictionary);
}

void	WED_PropertyTable::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content)
{
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);	
	int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
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
		val.int_val = the_content.int_val;
		break;
	case prop_EnumSet:	
		val.prop_kind = prop_EnumSet;
		val.set_val = the_content.int_set_val;
		break;			
	}
	string foo = string("Change ") + inf.prop_name;
	t->StartCommand(foo);
	t->SetNthProperty(idx, val);
	t->CommitCommand();
}

void	WED_PropertyTable::ToggleDisclose(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	if (t)
		mOpen[t->GetID()] = 1 - mOpen[t->GetID()];
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void	WED_PropertyTable::SelectCell(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
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
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
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
	ISelection * sel = SAFE_CAST(ISelection,mArchive->Fetch(mSelect));
	if (!root) return NULL;
	// Ben says: tables are indexed bottom=0 to match OGL coords.
	// INvert our numbers here because we really need to count up when we traverse the tree!
	if (!mVertical)
		row = GetRowCount() - row - 1;
	return FetchNthRecursive(root, row, sel);
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


WED_Thing *	WED_PropertyTable::FetchNthRecursive(WED_Thing * e, int& row, ISelection * sel)
{
	if (e == NULL) return NULL;

	int filtered = 1;
	if (!mSelOnly || !sel || sel->IsSelected(e))
	if (mFilter.empty() || mFilter.count(e->GetClass()))
	{	
		if (row == 0) return e;	
		filtered = 0;
		--row;
	}
	
	if (mOpen[e->GetID()] != 0 || mVertical || filtered)	
	for (int n = 0; n < e->CountChildren(); ++n)
	{
		WED_Thing * c = FetchNthRecursive(e->GetNthChild(n), row, sel);
		if (c) return c;
	}
	return NULL;
}

int			WED_PropertyTable::GetColCount(void)
{
	if (!mVertical)
		return mColNames.size();

	WED_Thing * root = SAFE_CAST(WED_Thing,mArchive->Fetch(mEntity));
	ISelection * sel = SAFE_CAST(ISelection,mArchive->Fetch(mSelect));
	return CountRowsRecursive(root, sel);
}

int			WED_PropertyTable::GetRowCount(void)
{
	if (mVertical)
		return mColNames.size();

	WED_Thing * root = SAFE_CAST(WED_Thing,mArchive->Fetch(mEntity));
	ISelection * sel = SAFE_CAST(ISelection,mArchive->Fetch(mSelect));
	
	return CountRowsRecursive(root, sel);
}

int			WED_PropertyTable::CountRowsRecursive(WED_Thing * e, ISelection * sel)
{
	if (e == NULL) return 0;
	int total = 0;
	
	int filtered = 1;
	
	if (!mSelOnly || !sel || sel->IsSelected(e))
	if (mFilter.empty() || mFilter.count(e->GetClass()))
	{
		++total;
		filtered = 0;
	}
	
	int cc = e->CountChildren();
	if (!mVertical)
	if (!filtered)
	if (mOpen[e->GetID()] == 0)	cc = 0;	
	for (int c = 0; c < cc; ++c)
	{
		total += CountRowsRecursive(e->GetNthChild(c), sel);
	}
	return total;
}

void	WED_PropertyTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
//	if (inMsg == msg_SelectionChanged)		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
	if (inMsg == msg_ArchiveChanged)
	{
		if (mDynamicCols)
		{
			set<string>	cols;
			cols.insert("Name");
			mColNames.clear();
			mColNames.push_back("Name");
			int total_objs = mVertical ? GetColCount() : GetRowCount();
			for (int i = 0; i < total_objs; ++i)
			{
				WED_Thing * t = FetchNth(i);
				if (t)
				{
					int pcount = t->CountProperties();
					for (int p = 0; p < pcount; ++p)
					{
						PropertyInfo_t info;
						t->GetNthPropertyInfo(p,info);
						if (cols.count(info.prop_name) == 0)
						{
							cols.insert(info.prop_name);
							mColNames.insert(mColNames.begin(), info.prop_name);
						}
					}
				}
			}			
		}		
		BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
	}
}


void	WED_PropertyTable::GetHeaderContent(
				int							cell_x, 
				GUI_HeaderContent&			the_content)
{
	the_content.is_selected = 0;
	the_content.can_resize = 0;
	the_content.can_select = 0;
	if (cell_x >= 0 && cell_x < mColNames.size())
	{
		the_content.title = mColNames[cell_x];
		the_content.can_resize = true;		
	}
}

