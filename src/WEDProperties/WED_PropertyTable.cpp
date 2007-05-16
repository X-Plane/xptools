#include "WED_PropertyTable.h"
#include "WED_Archive.h"
#include "WED_Thing.h"
#include "ISelection.h"
#include "IOperation.h"
#include "IGIS.h"
#include "WED_Messages.h"
#include "GUI_Messages.h"
#include "WED_ToolUtils.h"
#include "WED_GroupCommands.h"

inline int count_strs(const char ** p) { if (!p) return 0; int n = 0; while(*p) ++p, ++n; return n; }

WED_PropertyTable::WED_PropertyTable(
									IResolver *				resolver,
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
	mResolver(resolver)
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
	the_content.can_drag = 1;	
	the_content.indent_level = 0;
	
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	if (t == NULL) return;
	
	ISelection * s = WED_GetSelect(mResolver);	
	
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
	int unused_vis, unused_kids;
	if (cell_x == 0)
	GetFilterStatus(t, s, unused_vis, unused_kids, the_content.can_disclose,the_content.is_disclosed);

	the_content.can_edit = inf.can_edit;
//	the_content.can_disclose = !mVertical && (cell_x == 0) && t->CountChildren() > 0;
//	the_content.can_disclose = !mVertical && (cell_x == 0) && e->GetGISClass() == gis_Composite;
//	the_content.is_disclosed = 	GetOpen(t->GetID()) && the_content.can_disclose;
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
		ToggleOpen(t->GetID());
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void	WED_PropertyTable::DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							bounds[4])
{
	WED_DoDragSelection(drag_emitter, mouse_x, mouse_y, bounds);
}


void	WED_PropertyTable::SelectionStart(
						int							clear)
{
	ISelection * s = WED_GetSelect(mResolver);
	IOperation * op = dynamic_cast<IOperation *>(s);
	op->StartOperation("Change Selection");
	if (clear) s->Clear();
	
	s->GetSelectionVector(mSelSave);
}

int		WED_PropertyTable::SelectGetExtent(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y)
{
	#if OPTIMIZE
		speed of this sux
	#endif
	ISelection * s = WED_GetSelect(mResolver);
	
	int num = mVertical ? GetColCount() : GetRowCount();

	int has = 0;
	low_x = low_y = num;
	high_x = high_y = 0;
	
	for (int n = 0; n < num; ++n)
	{
		WED_Thing * t = FetchNth(n);
		if (t)
		{
			if (s->IsSelected(t))
			{
				has = 1;
				low_x = low_y = min(low_x, n);
				high_x = high_y = max(high_x, n);
			}
		}
	}
	return has;
}

int		WED_PropertyTable::SelectGetLimits(
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


void	WED_PropertyTable::SelectRange(
						int							start_x,
						int							start_y,
						int							end_x,
						int							end_y,
						int							is_toggle)
{
	ISelection * s = WED_GetSelect(mResolver);
	
	s->Clear();
	for (vector<IUnknown *>::iterator u = mSelSave.begin(); u != mSelSave.end(); ++u)
		s->Insert(*u);
	#if OPTIMIZE
		provide accelerated sel-save-restore ops!
	#endif

	for (int n = (mVertical ? start_x : start_y); n <= (mVertical ? end_x : end_y); ++n)
	{
		#if OPTIMIZE
			for loop is n-squared perf - fix this!
		#endif
		WED_Thing * t = FetchNth(n);
		if (t)
		{
			if (is_toggle)	s->Toggle(t);
			else			s->Insert(t);
		}
	}
}

void	WED_PropertyTable::SelectionEnd(void)
{
	ISelection * s = WED_GetSelect(mResolver);
	IOperation * op = dynamic_cast<IOperation *>(s);
	op->CommitOperation();
	mSelSave.clear();
}

int		WED_PropertyTable::TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content)
{
	int start_x = io_x;
	int start_y = io_y;
	
	int width = GetColCount();
	int height =GetRowCount(); 
	
	if (height == 0 || width == 0) return 0;
	
	do 
	{
		if (reverse)	--io_x;
		else			++io_x;
		if (io_x >= width) { io_x = 0; --io_y; }
		if (io_x < 0	 ) { io_x = width-1; ++io_y; }
		if (io_y >=height) { io_y = 0;		   }
		if (io_y < 0	 ) { io_y = height-1;   }
	
		GetCellContent(io_x, io_y, the_content);
		if (the_content.can_edit && (
			the_content.content_type == gui_Cell_EditText ||
			the_content.content_type == gui_Cell_Integer ||
			the_content.content_type == gui_Cell_Double))
		{
			return 1;
		}
	
	} while (start_x != io_x || start_y != io_y);
	return 0;
}

void					WED_PropertyTable::GetLegalDropOperations(
							int&						allow_between_col,
							int&						allow_between_row,
							int&						allow_into_cell)
{ 
	allow_between_col = mVertical && !mSelOnly && mFilter.empty(); 
	allow_between_row = !mVertical && !mSelOnly && mFilter.empty(); 
	allow_into_cell = !mSelOnly && mFilter.empty(); 
}

GUI_DragOperation		WED_PropertyTable::CanDropIntoCell(
							int							cell_x,
							int							cell_y,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended,
							int&						whole_col,
							int&						whole_row) 
{ 
	if (mSelOnly) return gui_Drag_None;
	if (!mFilter.empty()) return gui_Drag_None;
	whole_col = mVertical; 
	whole_row = !mVertical; 
	
	if (!WED_IsDragSelection(drag)) return gui_Drag_None;
	
	#if !DEV
		address allow/recommend
	#endif
	
	WED_Thing * who = FetchNth(mVertical ? cell_x : cell_y);
	if (who)
		return WED_CanMoveSelectionTo(mResolver, who, who->CountChildren()) ? gui_Drag_Move : gui_Drag_None;
	
	return gui_Drag_None; 
}

GUI_DragOperation		WED_PropertyTable::CanDropBetweenColumns(
							int							cell_x,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended)
{
	if (mSelOnly) return gui_Drag_None;
	if (!mFilter.empty()) return gui_Drag_None;

	if (!mVertical) return gui_Drag_None;
	if (!WED_IsDragSelection(drag)) return gui_Drag_None;
	
	if (cell_x == GetColCount())
	{
		WED_Thing * who = FetchNth(cell_x-1);
		if (who && who->GetParent())
			return WED_CanMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()+1) ? gui_Drag_Move : gui_Drag_None;
	}
	else
	{
		WED_Thing * who = FetchNth(cell_x);
		if (who && who->GetParent())
			return WED_CanMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()) ? gui_Drag_Move : gui_Drag_None;
	}	
	
	return gui_Drag_None; 
}

GUI_DragOperation		WED_PropertyTable::CanDropBetweenRows(
							int							cell_y,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended)
{
	if (mSelOnly) return gui_Drag_None;
	if (!mFilter.empty()) return gui_Drag_None;

	if (mVertical) return gui_Drag_None;
	if (!WED_IsDragSelection(drag)) return gui_Drag_None;
	
	if (cell_y == GetRowCount())
	{
		// We are dragging into the top slot of the table.  Go right before the top entity.
		WED_Thing * who = FetchNth(cell_y-1);
		if (who && who->GetParent())
			return WED_CanMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()) ? gui_Drag_Move : gui_Drag_None;
	} 
	else
	{
		// All drags other than the top slot are handled by finding teh guy above us.  If above us is open, we drop right into his
		// first slot, otherwise we go right behind him in his parent.
		WED_Thing * who = FetchNth(cell_y);
		if (who && who->GetParent())
		{
			int v,r,c,i;
			GetFilterStatus(who, WED_GetSelect(mResolver),v,r,c,i);
			if (i)		return WED_CanMoveSelectionTo(mResolver, who, 0) ? gui_Drag_Move : gui_Drag_None;
			else		return WED_CanMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()+1) ? gui_Drag_Move : gui_Drag_None;
		}
	}
	
	return gui_Drag_None; 
}


GUI_DragOperation		WED_PropertyTable::DoDropIntoCell(
							int							cell_x,
							int							cell_y,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended)
{
	WED_Thing * who = FetchNth(mVertical ? cell_x : cell_y);
	if (who)
		WED_DoMoveSelectionTo(mResolver, who, who->CountChildren());
	return gui_Drag_Copy;
}

GUI_DragOperation		WED_PropertyTable::DoDropBetweenColumns(
							int							cell_x,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended)
{ 	
	if (!mVertical) return gui_Drag_None;
	if (cell_x == GetColCount())
	{	
		WED_Thing * who = FetchNth(cell_x-1);
		if (who && who->GetParent())
			WED_DoMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()+1);
	} else {
		WED_Thing * who = FetchNth(cell_x);
		if (who && who->GetParent())
			WED_DoMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition());
	}
	return gui_Drag_Copy;
}

GUI_DragOperation		WED_PropertyTable::DoDropBetweenRows(
							int							cell_y,
							GUI_DragData *				drag, 
							GUI_DragOperation			allowed, 
							GUI_DragOperation			recommended)  
{
	if (mVertical) return gui_Drag_None;

	
	if (cell_y == GetRowCount())
	{
		WED_Thing * who = FetchNth(cell_y-1);
		if (who && who->GetParent())
			WED_DoMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()+1);
	} 
	else
	{
		WED_Thing * who = FetchNth(cell_y);
		if (who && who->GetParent())
		{
			int v,r,c,i;
			GetFilterStatus(who, WED_GetSelect(mResolver),v,r,c,i);
			if (i)				WED_DoMoveSelectionTo(mResolver, who, 0);
			else				WED_DoMoveSelectionTo(mResolver, who->GetParent(), who->GetMyPosition()+1);
		}
	}

	return gui_Drag_Copy;
}

WED_Thing *	WED_PropertyTable::FetchNth(int row)
{
	WED_Thing * root = WED_GetWorld(mResolver);
	ISelection * sel = WED_GetSelect(mResolver);
	if (!root) return NULL;
	// Ben says: tables are indexed bottom=0 to match OGL coords.
	// INvert our numbers here because we really need to count up when we traverse the tree!
	if (!mVertical)
		row = GetRowCount() - row - 1;
	return FetchNthRecursive(root, row, sel);
}

int			WED_PropertyTable::GetThingDepth(WED_Thing * d)
{
	WED_Thing * root = WED_GetWorld(mResolver);
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

	int vis,kids,can_disclose,is_disclose;
	GetFilterStatus(e,sel,vis,kids,can_disclose, is_disclose);

	if (vis)
	{
		if (row == 0) return e;	
		--row;
	}
	
	for (int n = 0; n < kids; ++n)
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

	WED_Thing * root = WED_GetWorld(mResolver);
	ISelection * sel = WED_GetSelect(mResolver);
	return CountRowsRecursive(root, sel);
}

int			WED_PropertyTable::GetRowCount(void)
{
	if (mVertical)
		return mColNames.size();

	WED_Thing * root = WED_GetWorld(mResolver);
	ISelection * sel = WED_GetSelect(mResolver);
	
	return CountRowsRecursive(root, sel);
}

int			WED_PropertyTable::CountRowsRecursive(WED_Thing * e, ISelection * sel)
{
	if (e == NULL) return 0;
	int total = 0;

	int vis,kids,can_disclose,is_disclose;
	GetFilterStatus(e,sel,vis,kids,can_disclose, is_disclose);
	
	if (vis)
	{
		++total;
	}
	
	for (int c = 0; c < kids; ++c) 
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

// These routines encapsulate the hash table that tracks the disclosure of various WED things.  We wrap it like this so we can
// easily map "no entry" to open.  This makes new entities default to open, which seems to be preferable.  It'd be easy to customize
// the behavior.

bool WED_PropertyTable::GetOpen(int id)
{
	return mOpen.count(id) == 0 || mOpen[id] != 0;
}

void WED_PropertyTable::ToggleOpen(int id)
{
	mOpen[id] = GetOpen(id) ? 0 : 1;
}

// This is the main "filter" function - it determines four properties at once about an entity:
// 1. Can we actually see this entity?
// 2. How many kids do we recurse (basically forcing this to 0 "hides" it.
// 3. Is there UI to disclose this element?
// 4. Is it disclosed now?
// This routine assures some useful sync:
// - It understands that vertical layouts (entities across the top) don't have tree behavior.
// - It makes sure the children are not listed if the item is not disclosed.
// - It makes sure that filtered items are inherently open (since we can't disclose them).
// - Right now it is programmed not to iterate on the children of not-truly-composite GIS entities
//  (Thus it hides the guts of a polygon).

void		WED_PropertyTable::GetFilterStatus(WED_Thing * what, ISelection * sel, 
									int&	visible, 
									int&	recurse_children,
									int&	can_disclose,
									int&	is_disclose)
{
	visible = recurse_children = can_disclose = is_disclose = 0;
	if (what == NULL) return;

	int is_composite = 0;
	visible = 0;
	
	IGISEntity * e = SAFE_CAST(IGISEntity, what);
	if (e) is_composite = e->GetGISClass() == gis_Composite;

	if (!mSelOnly || !sel || sel->IsSelected(what))
	if (mFilter.empty() || mFilter.count(what->GetClass()))
		visible = 1;

	recurse_children = what->CountChildren();
	
	if (!visible || mVertical)
	{	
		if (!is_composite) recurse_children = 0;
	}
	else
	{
		can_disclose = is_composite;
		is_disclose = can_disclose && GetOpen(what->GetID());
		if (!is_disclose) recurse_children = 0;
	}
}