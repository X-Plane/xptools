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

#include "WED_PropertyTable.h"
#include "WED_Archive.h"
#include "WED_Thing.h"
#include "ISelection.h"
#include "IOperation.h"
#include "IGIS.h"
#include "IHasResource.h"
#include "ILibrarian.h"
#include "WED_Entity.h"

#include "STLUtils.h"

#include "WED_GISComposite.h"
#include "WED_Airport.h"
#include "WED_Group.h"
#include "WED_Root.h"
#include "WED_ATCFlow.h"

#include "PlatformUtils.h"
#include "WED_UIDefs.h"
#include "GUI_Messages.h"
#include "WED_Messages.h"
#include "WED_ToolUtils.h"
#include "WED_GroupCommands.h"
#include "WED_UIMeasurements.h"
#include "WED_EnumSystem.h"

inline int count_strs(const char ** p) { if (!p) return 0; int n = 0; while(*p) ++p, ++n; return n; }

inline bool AnyLocked(WED_Thing * t)
{
	if (t == NULL) return false;
	WED_Entity * e = dynamic_cast<WED_Entity *>(t);
	if (e == NULL) return false;
	if (e->GetLocked()) return true;
	return AnyLocked(t->GetParent());
}

inline bool AnyHidden(WED_Thing * t)
{
	if (t == NULL) return false;
	WED_Entity * e = dynamic_cast<WED_Entity *>(t);
	if (e == NULL) return false;
	if (e->GetHidden()) return true;
	return AnyHidden(t->GetParent());
}

WED_PropertyTable::WED_PropertyTable(
									IResolver *				resolver,
									const char **			col_names,
									int *					def_col_widths,
									int						vertical,
									int						dynamic_cols,
									int						sel_only,
									const char **			filter)
	:	GUI_SimpleTableGeometry(
				count_strs(col_names),
				def_col_widths,
				WED_UIMeasurement("table_row_height")),
	mVertical(vertical),
	mDynamicCols(dynamic_cols),
	mSelOnly(sel_only),
	mResolver(resolver),
	mCacheValid(false)
{
	RebuildCache();

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
	char buf[100], fmt[16];

	// By the end of this we need to have filled the_content out with
	//  1. Abilities - can_edit, can_disclose, can_drag, etc...
	//  2. State - is_disclosed, is_selected, indent_level
	//  3. Content - content_type, its corrisponding value filled in
	
	//Our default assumptions
	the_content.content_type = gui_Cell_None;
	the_content.string_is_resource = 0;
	the_content.can_delete = false;
	the_content.can_edit = 0;
	the_content.can_disclose = 0;
	the_content.can_select = 0;
	the_content.is_disclosed = 0;
	the_content.is_selected = 0;
	the_content.can_drag = 1;
	the_content.indent_level = 0;

	//Find the row or column we're dealing with
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	if (t == NULL) return;

	ISelection * s = WED_GetSelect(mResolver);

	//Find the property index in said row or column based on the name
	int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
	
	//If there has been one found, use the_content as it is and exit
	if (idx == -1)
		return;

	WED_Thing * my_parent = t->GetParent();
	if(my_parent)
	if(!WED_IsFolder(my_parent))
		the_content.can_drag = 0;

	//With the property index, get the property's value and info
	PropertyInfo_t	inf;
	PropertyVal_t	val;
	t->GetNthPropertyInfo(idx,inf);
	t->GetNthProperty(idx, val);

	the_content.can_select = mSelOnly ? 0 : 1;
	the_content.is_selected = s->IsSelected(t);

	//Based on type turn PropertyVal_t into GUI_CellContent,
	//taking care of "3. Content"
	switch(inf.prop_kind) {
	case prop_Int:
		the_content.content_type = gui_Cell_Integer;
		the_content.int_val = val.int_val;
		sprintf(fmt,"%%%dd", inf.digits);
		snprintf(buf,sizeof(buf),fmt,val.int_val);
		the_content.text_val = buf;
		break;
	case prop_Double:
		the_content.content_type = gui_Cell_Double;
		the_content.double_val = val.double_val;
		sprintf(fmt,"%%%d.%dlf %s",inf.digits, inf.decimals, inf.units);
		if(inf.round_down)
		{
			// We are going to shift our fractional part left 1 more decimal digit to the left than needed.  Why?
			// The answer: we have to round to nearest to reconstruct numbers like 128.839999999 (as 128.84444444.
			// But we don't want the round to bump our last digit up (128.825 should NOT become 128.83).  So we do
			// the round with one EXTRA digit of precision to catch the floating point sliver case.
			double fract = pow(10.0,inf.decimals);
			double v = floor(fract * (val.double_val) + 0.05) / fract;
			snprintf(buf,sizeof(buf),fmt,v);
		}
		else
		{
			snprintf(buf,sizeof(buf),fmt,val.double_val);
		}
		the_content.text_val = buf;
		break;
	case prop_String:
		the_content.content_type = gui_Cell_EditText;
		the_content.text_val = val.string_val;
		break;
	case prop_TaxiSign:
		the_content.content_type = gui_Cell_TaxiText;
		the_content.text_val = val.string_val;
		break;
	case prop_FilePath:
		the_content.content_type = gui_Cell_FileText;
		the_content.text_val = val.string_val;
		break;
	case prop_Bool:
		the_content.content_type = gui_Cell_CheckBox;
		the_content.int_val = val.int_val;
		the_content.bool_val = gui_Bool_Check;
		the_content.bool_partial = 0;
		if (mColNames[mVertical ? cell_y : cell_x] == "Locked")	{ the_content.bool_val = gui_Bool_Lock;		if (!the_content.int_val)	the_content.bool_partial = AnyLocked(t); }
		if (mColNames[mVertical ? cell_y : cell_x] == "Hidden")	{ the_content.bool_val = gui_Bool_Visible;	if (!the_content.int_val)	the_content.bool_partial = AnyHidden(t); }
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
			if (ENUM_Domain(*iter) == LinearFeature)
			{
				label = ENUM_Name(*iter);
				label += ".png";
				the_content.string_is_resource = 1;
			}
			the_content.text_val += label;
		}
		if (the_content.text_val.empty())	the_content.text_val="None";
		if(inf.exclusive && the_content.int_set_val.empty()) the_content.int_set_val.insert(0);
		break;
	}
	int unused_vis, unused_kids;
//	if (cell_x == 0)
	if (!mVertical && !mSelOnly)
	if (mColNames[mVertical ? cell_y : cell_x] == "Name")
	{
		//Fill in more about abilities and state, see method for more comments
		GetFilterStatus(t, s, unused_vis, unused_kids, the_content.can_disclose,the_content.is_disclosed);
		the_content.indent_level = GetThingDepth(t);	/// as long as "cell 0" is the diclose level, might as well have it be the indent level too.
	}


	the_content.can_delete = inf.can_delete;
	the_content.can_edit = inf.can_edit;
	if (the_content.can_edit)
	if (WED_GetWorld(mResolver) == t)	the_content.can_edit = 0;

	//THIS IS A HACK to stop the user from being able to disclose arrows during search mode
	if (mSearchFilter.empty() == false)
	{
		if (the_content.can_disclose == true)
		{
			the_content.is_disclosed = true;
		}
	}

//	the_content.can_disclose = !mVertical && (cell_x == 0) && t->CountChildren() > 0;
//	the_content.can_disclose = !mVertical && (cell_x == 0) && e->GetGISClass() == gis_Composite;
//	the_content.is_disclosed = 	GetOpen(t->GetID()) && the_content.can_disclose;
	#if DEV
		//the_content.printCellInfo(true,true,false,true,false,false,true,false,true,false,false,false,false,false);
	#endif
}

void	WED_PropertyTable::GetEnumDictionary(
						int							cell_x,
						int							cell_y,
						GUI_EnumDictionary&			out_dictionary)
{
	out_dictionary.clear();
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);

	int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
	if (idx == -1) return;

	t->GetNthPropertyDict(idx, out_dictionary);
	PropertyInfo_t info;
	t->GetNthPropertyInfo(idx,info);
	if(info.prop_kind == prop_EnumSet)
	if(info.exclusive)
		out_dictionary.insert(GUI_EnumDictionary::value_type(0,make_pair(string("None"),true)));
}

void	WED_PropertyTable::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content,
						int							apply_all)
{
	vector<WED_Thing *>	apply_vec;

	GUI_CellContent content(the_content);


	if (content.content_type == gui_Cell_FileText)
	{
		ILibrarian * librarian = WED_GetLibrarian(mResolver);
		librarian->LookupPath(content.text_val);

		char fbuf[2048];
		strcpy(fbuf,content.text_val.c_str());
		if (!GetFilePathFromUser(getFile_Open,"Pick file", "Open", FILE_DIALOG_PROPERTY_TABLE, fbuf,sizeof(fbuf)))
			return;
		content.text_val = fbuf;
		librarian->ReducePath(content.text_val);
	}

	if (apply_all)
	{
		ISelection * sel = WED_GetSelect(mResolver);
		sel->IterateSelectionOr(Iterate_CollectThings, &apply_vec);
	}
	else
	{
		WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
		if (t != NULL)  apply_vec.push_back(t);
	}

		WED_Thing * started = NULL;

	for (int iter = 0; iter < apply_vec.size(); ++iter)
	{
		WED_Thing * t = apply_vec[iter];

		int idx = t->FindProperty(mColNames[mVertical ? cell_y : cell_x].c_str());
		if (idx == -1) continue;
		PropertyInfo_t	inf;
		PropertyVal_t	val;
		t->GetNthPropertyInfo(idx,inf);

		if (inf.prop_kind == prop_Int		&& content.content_type != gui_Cell_Integer	)	continue;
		if (inf.prop_kind == prop_Double	&& content.content_type != gui_Cell_Double	)	continue;
		if (inf.prop_kind == prop_String	&& content.content_type != gui_Cell_EditText)	continue;
		if (inf.prop_kind == prop_TaxiSign && content.content_type != gui_Cell_TaxiText) continue;
		if (inf.prop_kind == prop_FilePath	&& content.content_type != gui_Cell_FileText)	continue;
		if (inf.prop_kind == prop_Bool		&& content.content_type != gui_Cell_CheckBox)	continue;
		if (inf.prop_kind == prop_Enum		&& content.content_type != gui_Cell_Enum	)	continue;
		if (inf.prop_kind == prop_EnumSet	&& content.content_type != gui_Cell_EnumSet	)	continue;

		switch(inf.prop_kind) {
		case prop_Int:
			val.prop_kind = prop_Int;
			val.int_val = content.int_val;
			break;
		case prop_Double:
			val.prop_kind = prop_Double;
			val.double_val = content.double_val;
			break;
		case prop_String:
			val.prop_kind = prop_String;
			val.string_val = content.text_val;
			break;
		case prop_TaxiSign:
			val.prop_kind = prop_TaxiSign;
			val.string_val = content.text_val;
			break;
		case prop_FilePath:
			val.prop_kind = prop_FilePath;
			val.string_val = content.text_val;
			break;
		case prop_Bool:
			val.prop_kind = prop_Bool;
			val.int_val = content.int_val;
			break;
		case prop_Enum:
			val.prop_kind = prop_Enum;
			val.int_val = content.int_val;
			break;
		case prop_EnumSet:
			val.prop_kind = prop_EnumSet;
			if (inf.exclusive)
			{
				val.set_val.clear();
				if (content.int_val != 0)
					val.set_val.insert(content.int_val);
			} else
				val.set_val = content.int_set_val;
			break;
		}
		string foo = string("Change ") + inf.prop_name;
		if (!started) {	started = t; started->StartCommand(foo); }
		t->SetNthProperty(idx, val);
	}
	if (started) started->CommitCommand();
}

void	WED_PropertyTable::ToggleDisclose(
						int							cell_x,
						int							cell_y)
{
	WED_Thing * t = FetchNth(mVertical ? cell_x : cell_y);
	if (t)
		ToggleOpen(t->GetID());
	mCacheValid = false;
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void	WED_PropertyTable::DoDeleteCell(
						int							cell_x,
						int							cell_y)
{
	//Get the airport
	WED_Airport * airport = static_cast<WED_Airport * >(FetchNth(0));
	
	airport->StartCommand("Delete Meta Data Key");
	//To be in uniform with other IPropertyMethods we'll transform cell_y->NS_META_DATA
	int ns_meta_data = (airport->WED_GISComposite::CountProperties());
	airport->DeleteNthProperty(ns_meta_data + airport->CountMetaDataKeys() - cell_y - 1);
	airport->CommitCommand();

	//TODO - Is this needed?
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED, 0);
}

void	WED_PropertyTable::DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							button,
						int							bounds[4])
{
	WED_DoDragSelection(drag_emitter, mouse_x, mouse_y, button, bounds);
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
	int op = mVertical ? GetRowCount() : GetColCount();

	int has = 0;
	if (mVertical)
	{
		low_y = 0; high_y = op;
		low_x = num; high_x = 0;
	}
	else
	{
		low_x = 0; high_x = op;
		low_y = num; high_y = 0;
	}

	for (int n = 0; n < num; ++n)
	{
		WED_Thing * t = FetchNth(n);
		if (t)
		{
			if (s->IsSelected(t))
			{
				has = 1;

				if (mVertical)
				{
					low_x = min(low_x, n);
					high_x = max(high_x, n);
				}
				else
				{
					low_y = min(low_y, n);
					high_y = max(high_y, n);
				}

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
	for (vector<ISelectable *>::iterator u = mSelSave.begin(); u != mSelSave.end(); ++u)
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

int		WED_PropertyTable::SelectDisclose(
						int							open_it,
						int							all)
{
	if (mVertical) return 0;
	if (all)
	{
		int cc = GetRowCount();
		vector<int>	things(cc);
		for (int n = 0; n < cc; ++n)
		{
			WED_Thing * t = FetchNth(n);
			things.push_back(t->GetID());
		}
		for (int n = 0; n <things.size(); ++n)
			SetOpen(things[n], open_it);
	} else {
		ISelection * sel = WED_GetSelect(mResolver);
		vector<ISelectable *>	sv;
		sel->GetSelectionVector(sv);
		for (int n = 0; n < sv.size(); ++n)
		{
			WED_Thing * t = dynamic_cast<WED_Thing *>(sv[n]);
			if (t)
			{
				SetOpen(t->GetID(), open_it);
			}
		}
	}
	mCacheValid = false;
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
	return 1;
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

	int tries = 0;

	do
	{
		if (mVertical)
		{
				 if(reverse<0		   )	++io_y;
			else if(reverse>0		   )	--io_y;
			if (io_y >=height) { io_y = 0;		 --io_x;	}
			if (io_y < 0	 ) { io_y = height-1;++io_x;	}
			if (io_x >= width) { io_x = 0;					}
			if (io_x < 0	 ) { io_x = width-1;			}
		}
		else
		{
				 if(reverse<0		   )	--io_x;
			else if(reverse>0		   )	++io_x;
			if (io_x >= width) { io_x = 0; --io_y; }
			if (io_x < 0	 ) { io_x = width-1; ++io_y; }
			if (io_y >=height) { io_y = 0;		   }
			if (io_y < 0	 ) { io_y = height-1;   }
		}
		GetCellContent(io_x, io_y, the_content);
		if (the_content.can_edit && (
			the_content.content_type == gui_Cell_EditText ||
			the_content.content_type == gui_Cell_TaxiText ||
			the_content.content_type == gui_Cell_Integer ||
			the_content.content_type == gui_Cell_Double))
		{
			WED_Thing * t = FetchNth(mVertical ? io_x : io_y);
			ISelection * sel = WED_GetSelect(mResolver);
			if (!sel->IsSelected(t))
			{
				t->StartOperation("Select Next");
				sel->Select(t);
				t->CommitOperation();
			}
			return 1;
		}
		if (reverse==0)reverse=1;
		++tries;
	} while (start_x != io_x || start_y != io_y || tries <= 1);
	return 0;
}

int		WED_PropertyTable::DoubleClickCell(
						int							cell_x,
						int							cell_y)
{
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

	#if BENTODO
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

#pragma mark -


void WED_PropertyTable::RebuildCacheRecursive(WED_Thing * e, ISelection * sel, set<WED_Thing *> * sel_and_friends)
{
	if (e == NULL) return;
	int vis,kids,can_disclose,is_disclose;
	GetFilterStatus(e,sel,vis,kids,can_disclose, is_disclose);

	if (vis)
		mThingCache.push_back(e);

	if (sel_and_friends)
	if (sel_and_friends->count(e) == 0)
		return;

	for (int n = 0; n < kids; ++n)
		RebuildCacheRecursive(e->GetNthChild(n), sel, sel_and_friends);
}

// Selection iterator: ref is a ptr to a set.  Accumulate the selected thing and all of its parents.
static int SelectAndParents(ISelectable * what, void * ref)
{
	set<WED_Thing *> * all = (set<WED_Thing *> *) ref;
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	while(who)
	{
		if (all->count(who)) return 0;
		all->insert(who);
		who = who->GetParent();
	}
	return 0;
}

void WED_PropertyTable::RebuildCache(void)
{
	mThingCache.clear();
	mCacheValid = true;
	set<WED_Thing*> all_sel;

	// Performance note: the "selection" hierarchy (all selected entities) is effectively ALWAYS fully disclosed, because we must iterate
	// through everything to find the selection.  In a situation with a huge hierarchy, this gives us a horribly slow time to rebuild
	// the contents of the table, even if only one object is selected.  (But we DO want to iterate, to go in hierarchy order.)

	// It turns out we can do better: we build a set (all_sel) that contains the selection and all views that have a selected thing as its
	// child.  This represents a subset of the total tree that needs iteration.  When we rebulid our cache, if we hit a thing that isn't in
	// "selection and friends" set, we simply stop.

	// This cuts out iteration of whole sub-trees where there is no selection...for a trivial selection this really speeds up rebuild.

	WED_Thing * root = WED_GetWorld(mResolver);
	ISelection * sel = WED_GetSelect(mResolver);
	if (mSelOnly)
		sel->IterateSelectionOr(SelectAndParents,&all_sel);
	if (root)
		RebuildCacheRecursive(root,sel,mSelOnly ? &all_sel : NULL);
}

WED_Thing *	WED_PropertyTable::FetchNth(int row)
{
	if (!mCacheValid)
	{
		if (mSearchFilter.empty() == true)
		{
			RebuildCache();
		}
		else
		{
			Resort();
		}
	}

	vector<WED_Thing*>& current_cache = mSearchFilter.empty() ? mThingCache : mSortedCache;
	if (!mVertical)
	{
		row = current_cache.size() - row - 1;
	}

	return current_cache[row];
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

int			WED_PropertyTable::GetColCount(void)
{
	if (!mVertical)
		return mColNames.size();

	if (!mCacheValid)
	{
		if (mSearchFilter.empty() == true)
		{
			RebuildCache();
		}
		else
		{
			Resort();
		}
	}

	vector<WED_Thing*>& current_cache = mSearchFilter.empty() ? mThingCache : mSortedCache;
	return current_cache.size();
}

int		WED_PropertyTable::ColForX(int n)
{
	int c = GUI_SimpleTableGeometry::ColForX(n);
	int cc = GetColCount();
	return min(c,cc-1);
}

int			WED_PropertyTable::GetRowCount(void)
{
	if (mVertical)
		return mColNames.size();

	if (!mCacheValid)
	{
		if (mSearchFilter.empty() == true)
		{
			RebuildCache();
		}
		else
		{
			Resort();
		}
	}

	vector<WED_Thing*>& current_cache = mSearchFilter.empty() ? mThingCache : mSortedCache;
	return current_cache.size();
}

void	WED_PropertyTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
//	if (inMsg == msg_SelectionChanged)		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
	if (inMsg == msg_ArchiveChanged)
	{
		// Set this to false FIRST, lest we have an explosion due to a stale cache!
		if (inParam & (wed_Change_CreateDestroy | wed_Change_Topology))
   			mCacheValid = false;

		if (mSelOnly && (inParam & wed_Change_Selection))
			mCacheValid = false;

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
						if(!info.prop_name.empty() && info.prop_name[0] != '.')
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

void WED_PropertyTable::SetClosed(const set<int>& closed_list)
{
	WED_Thing * root = WED_GetWorld(mResolver);
	WED_Archive * arch = root->GetArchive();
	for( set<int>::const_iterator it = closed_list.begin(); it != closed_list.end(); ++it)
	if(arch->Fetch(*it) != NULL)
	{
	    SetOpen(*it,0);
	}
	mCacheValid = false;
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}

void WED_PropertyTable::GetClosed(set<int>& closed_list)
{
	closed_list.clear();
	WED_Thing * root = WED_GetWorld(mResolver);
	WED_Archive * arch = root->GetArchive();

	for (hash_map<int, int>::iterator it = mOpen.begin(); it != mOpen.end(); ++it)
	{
		if ((arch->Fetch(it->first) != NULL) && (!GetOpen(it->first)))
		{
			closed_list.insert(it->first);
		}
	}
}

//--IFilterable----------------------------------------------------------------
void WED_PropertyTable::SetFilter(const string & filter)
{
	mSearchFilter = filter;
	Resort();

	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED, 0);
}

//parameters
//thing - the current thing
//search_filter - the search filter
//sorted_cache - the sorted_cache of wed things we're building up
//sorted_open_ids - the hash map of id and true false if its open or not
//returns number of bad leafs
int collect_recusive(WED_Thing * thing, const ci_string& isearch_filter, vector<WED_Thing*>& sorted_cache)
{
	DebugAssert(thing != NULL);

	bool is_group_like = thing->GetClass() == WED_Group::sClass   ||
						 thing->GetClass() == WED_Airport::sClass ||
						 thing->GetClass() == WED_ATCFlow::sClass;

	string thing_name;
	thing->GetName(thing_name);
	ci_string ithing_name(thing_name.begin(),thing_name.end());

	bool is_match = ithing_name.find(isearch_filter) != ci_string::npos;
	//Stop if the cheap test succeeds
	if (is_match == false)
	{
		IHasResource * has_resource_thing = dynamic_cast<IHasResource*>(thing);
		if (has_resource_thing != NULL)
		{
			string res;
			has_resource_thing->GetResource(res);

			is_match |= ci_string(res.begin(), res.end()).find(isearch_filter) != ci_string::npos;
		}
	}

	int nc = thing->CountChildren();
	if (nc == 0) //thing is a leaf
	{
		if (is_match)
		{
			sorted_cache.push_back(thing);
			return 0; //No bad leafs here!
		}
		else
		{
			return 1;
		}
	}
	else
	{
		int current_end_pos = sorted_cache.size();
		int bad_leafs = 0;
		for (int n = 0; n < nc; ++n)
		{
			bad_leafs += collect_recusive(thing->GetNthChild(n), isearch_filter, sorted_cache);
		}

		//If bad_leafs is less than the number of kids it means that there is at least some reason to keep this group
		//Or if the group name exactly matches
		if ((bad_leafs < nc && is_group_like) || is_match)
		{
			sorted_cache.insert(sorted_cache.begin() + current_end_pos, thing);
			return 0;
		}
		else
		{
			return 1; //Ignore me
		}
	}
}

void WED_PropertyTable::Resort()
{
	mSortedCache.clear();
	if (mSearchFilter.empty() == false)
	{
		mSortedCache.reserve(mThingCache.size());
		ci_string isearch_filter(mSearchFilter.begin(), mSearchFilter.end());
		collect_recusive(WED_GetWorld(mResolver), isearch_filter, mSortedCache);
	}
	mCacheValid = true;
}
//-----------------------------------------------------------------------------

// These routines encapsulate the hash table that tracks the disclosure of various WED things.  We wrap it like this so we can
// easily map "no entry" to open.  This makes new entities default to open, which seems to be preferable.  It'd be easy to customize
// the behavior.

bool WED_PropertyTable::GetOpen(int id)
{
	return mOpen.count(id) == 0 || mOpen[id] != 0;
}

void WED_PropertyTable::ToggleOpen(int id)
{
	if (mSearchFilter.empty() == true)
	{
		int old_val = GetOpen(id);
		mOpen[id] = old_val ? 0 : 1;
	}
}

void WED_PropertyTable::SetOpen(int id, int o)
{
	if (mSearchFilter.empty() == true)
	{
		mOpen[id] = o;
	}
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

	is_composite = WED_IsFolder(what);

//	IGISEntity * e = SAFE_CAST(IGISEntity, what);
//	if (e) is_composite = e->GetGISClass() == gis_Composite;
	if (mSelOnly) is_composite = 1;

	if (!mSelOnly || !sel || sel->IsSelected(what))
	if (mFilter.empty() || mFilter.count(what->GetClass()))
		visible = 1;

	//TODO - use mSearchFilter here?

	recurse_children = what->CountChildren();

	if (!visible || mVertical)
	{
		if (!is_composite) recurse_children = 0;
	}
	else
	{
		can_disclose = is_composite;
		is_disclose = can_disclose && GetOpen(what->GetID());
		if (!is_disclose)
		{
			recurse_children = 0;
		}
	}
}
