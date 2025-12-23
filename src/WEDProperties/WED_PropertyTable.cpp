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

#include "WED_MapLayer.h"
#include "WED_Taxiway.h"
#include "WED_Select.h"
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
#include "WED_GISPolygon.h"
#include "WED_Airport.h"
#include "WED_AirportNode.h"
#include "WED_Group.h"
#include "WED_Root.h"
#include "WED_ATCFlow.h"

#include "PlatformUtils.h"
#include "WED_UIDefs.h"
#include "GUI_Messages.h"
#include "WED_Messages.h"
#include "WED_ToolUtils.h"
#include "WED_GroupCommands.h"
#include "WED_EnumSystem.h"
#include "GUI_Commander.h"
#include "WED_Menus.h"
#include "WED_RampPosition.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_Runway.h"
#include "WED_MapPane.h"
#include "WED_PackageListAdapter.h"

inline int count_strs(const char ** p) { if (!p) return 0; int n = 0; while(*p) ++p, ++n; return n; }

static bool AnyLocked(WED_Thing * t)
{
	if (t == NULL) return false;
	WED_Entity * e = dynamic_cast<WED_Entity *>(t);
	if (e == NULL) return false;
	if (e->GetLocked()) return true;
	return AnyLocked(t->GetParent());
}

static bool AnyHidden(WED_Thing * t)
{
	if (t == NULL) return false;
	WED_Entity * e = dynamic_cast<WED_Entity *>(t);
	if (e == NULL) return false;
	if (e->GetHidden()) return true;
	return AnyHidden(t->GetParent());
}

WED_PropertyTable::WED_PropertyTable(
									GUI_Commander *         cmdr,
									IResolver *				resolver,
									const char **			col_names,
									int *					def_col_widths,
									int						vertical,
									int						dynamic_cols,
									int						sel_only,
									const char **			filter)
	:	GUI_SimpleTableGeometry(
				count_strs(col_names),
				def_col_widths),
	GUI_Commander(cmdr),
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

	RecalculateColumns();

	if (filter)
	while (*filter)
		mFilter.insert(*filter++);

}

WED_PropertyTable::~WED_PropertyTable()
{
}

void WED_PropertyTable::RecalculateColumns()
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
		sprintf(fmt,"%%%d.%dlf %.6s",inf.digits, inf.decimals, inf.units);  // info.units may be not zero terminated
		snprintf(buf,sizeof(buf),fmt,val.double_val);
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
	case prop_RoadType:
		the_content.content_type = gui_Cell_RoadType;
		t->GetNthPropertyDictItem(idx, val.int_val,the_content.text_val);
		the_content.int_val = val.int_val;
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

		if((mColNames[mVertical ? cell_y : cell_x] == "Locked" || mColNames[mVertical ? cell_y : cell_x] == "Hidden") &&
			SAFE_CAST(WED_GISPolygon,my_parent))
			{
				the_content.bool_partial = 1;    // don't give the impression the inner/outer rings of polygons could/should be hidden or locked, ever
				return;
			}
		break;
	case prop_Enum:
		the_content.content_type = gui_Cell_Enum;
		t->GetNthPropertyDictItem(idx, val.int_val,the_content.text_val);
		the_content.int_val = val.int_val;
		break;
	case prop_EnumSet:
		the_content.content_type = (inf.domain == LinearFeature ? gui_Cell_LineEnumSet : gui_Cell_EnumSet);
		the_content.int_set_val = val.set_val;
		the_content.text_val.clear();
		for(set<int>::iterator iter=val.set_val.begin();iter != val.set_val.end(); ++iter)
		{
			if(*iter == 0) continue;                                // SetUnion can now insert 0 to indicate lines with partial blank setgemnts
			if (!the_content.text_val.empty()) the_content.text_val += ",";
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
		if(inf.exclusive && the_content.int_set_val.empty()) the_content.int_set_val.insert(0);   // not needed any more now that SetUnion adds this ?
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
		if (the_content.can_disclose)
		{
			the_content.is_disclosed = true;
		}
	}
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
		if (inf.prop_kind == prop_TaxiSign	&& content.content_type != gui_Cell_TaxiText)	continue;
		if (inf.prop_kind == prop_FilePath	&& content.content_type != gui_Cell_FileText)	continue;
		if (inf.prop_kind == prop_Bool		&& content.content_type != gui_Cell_CheckBox)	continue;
		if (inf.prop_kind == prop_Enum		&& content.content_type != gui_Cell_Enum	)	continue;
		if (inf.prop_kind == prop_RoadType	&& content.content_type != gui_Cell_RoadType)	continue;
		if (inf.prop_kind == prop_EnumSet	&& ( content.content_type != gui_Cell_EnumSet &&
												 content.content_type != gui_Cell_LineEnumSet ))	continue;

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
			if((mColNames[mVertical ? cell_y : cell_x] == "Locked" || mColNames[mVertical ? cell_y : cell_x] == "Hidden") &&
				 SAFE_CAST(WED_GISPolygon,t->GetParent()) )
				{
					val.int_val = 0;    // don't let anyone ever set polygon inner/outer rings to be hidden or locked.
				}
			break;
		case prop_Enum:
			val.prop_kind = prop_Enum;
			val.int_val = content.int_val;
			break;
		case prop_RoadType:
			val.prop_kind = prop_RoadType;
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

	if ( gModeratorMode == 1 )
	{
		return 0;
	}

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

   if(gModeratorMode) // special behavior requested by Julian
   {
      DispatchHandleCommand(wed_ZoomSelection);

      ISelectable * sel0 = s->GetNthSelection(0);
		WED_Group * grp = SAFE_CAST(WED_Group, sel0);
		string grpnam;
		if (grp) grp->GetName(grpnam);

		if (SAFE_CAST(WED_RampPosition, sel0) || SAFE_CAST(WED_TaxiRoute, sel0) ||  SAFE_CAST(WED_TaxiRouteNode, sel0) ||
          SAFE_CAST(WED_TruckParkingLocation, sel0) || SAFE_CAST(WED_TruckDestination, sel0) ||
			 SAFE_CAST(WED_Runway, sel0) || grpnam == "Runways" ||
			 grpnam == "Ramp Starts" || grpnam == "Ground Vehicles" || grpnam == "Taxi Routes" || grpnam == "Ground Routes" )
		{
			DispatchHandleCommand(wed_MapATC);
		}
//        else if (SAFE_CAST(WED_FacadePlacement, sel0) || SAFE_CAST(WED_ObjPlacement, sel0))
//        {
//             mMapPane->SetTabFilterMode(tab_3D);
//            DispatchHandleCommand(wed_Map3D);
//        }
//        else if(SAFE_CAST(WED_Runway, sel0) || grpnam == "Runways" )
//        {
//            DispatchHandleCommand(wed_MapPavement);
//        }
      else
		{
			DispatchHandleCommand(wed_MapSelection);
		}
	}
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
	} while ((start_x != io_x || start_y != io_y || tries <= 1)
				&& tries < 100);                 // prevent infinite loop if nothing in table is "advanceable" to, e.g. a taxi route edge runway segment
	return 0;
}


int WED_PropertyTable::DoubleClickCell(int cell_x, int cell_y)
{
	return 1;
}

