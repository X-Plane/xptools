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

#include "WED_GroupCommands.h"
#include "WED_ToolUtils.h"
#include "AssertUtils.h"
#include "ISelection.h"
#include "WED_Thing.h"
#include "WED_Airport.h"
#include "WED_Group.h"


int		WED_CanGroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	DebugAssert(sel != NULL);

	if (sel->IterateSelection(Iterate_IsPartOfStructuredObject, NULL)) return 0;
	
	int has_airport = sel->IterateSelection(Iterate_IsAirport, NULL);
	
	WED_Thing * global_parent = WED_FindParent(sel, NULL, NULL);
	if (global_parent == NULL) return 0;
	
	if (Iterate_IsOrParentAirport(global_parent, NULL))
	{
		// We are going into an airport.  DO NOT allow an airport into another one.
		if (sel->IterateSelection(Iterate_IsOrChildAirport, NULL)) return 0;
	}
	else
	{
		// Not going into an airport.  If we need to, well, we can't do this.
		if (sel->IterateSelection(	Iterate_ChildRequiresAirport, NULL)) return 0;
	
	}
	return 1;
}

int		WED_CanUngroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	DebugAssert(sel != NULL);
	
	if(sel->IterateSelection(Iterate_IsNotGroup, NULL)) return 0;

	if (sel->GetSelectionCount() == 0) return 0;
	return 1;
}

void	WED_DoGroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);

	static int grp_count = 0;
	char buf[100];

	WED_Thing *	parent = WED_FindParent(sel, NULL, NULL);
	
	parent->StartCommand("Group");

	vector<WED_Thing *> items;
	WED_GetSelectionInOrder(inResolver, items);
	
	WED_Group * group = WED_Group::CreateTyped(parent->GetArchive());
	++grp_count;
	sprintf(buf,"Group %d", grp_count);
	group->SetName(buf);	
	
	DebugAssert(items.front()->GetParent() == parent);
	group->SetParent(parent,items.front()->GetMyPosition());
	
	for (vector<WED_Thing *>::iterator i = items.begin(); i != items.end(); ++i)
	{
		(*i)->SetParent(group, group->CountChildren());
	}
	sel->Select(group);
	parent->CommitCommand();
}

void	WED_DoUngroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	vector<IUnknown *> items;
	sel->GetSelectionVector(items);
	
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Ungroup");
	
	sel->Clear();
	
	for (vector<IUnknown *>::iterator i = items.begin(); i != items.end(); ++i)
	{
		WED_Thing * dead_group = dynamic_cast<WED_Thing *>(*i);
		DebugAssert(dead_group != NULL);

		int insert_pos = dead_group->GetMyPosition();			
		
		while (dead_group->CountChildren() > 0)
		{
			WED_Thing * who = dead_group->GetNthChild(0);
			who->SetParent(dead_group->GetParent(), insert_pos);
			sel->Insert(who);
			++insert_pos;
		}

		dead_group->SetParent(NULL, 0);
		dead_group->Delete();
	}
	
	op->CommitOperation();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

int		WED_CanMakeNewAirport(IResolver * inResolver)
{
	return 1;
}

void	WED_DoMakeNewAirport(IResolver * inResolver)
{
	WED_Thing * wrl = WED_GetWorld(inResolver);
	wrl->StartCommand("Create Airport");
	
	static int apt_count = 0;
	WED_Airport * apt = WED_Airport::CreateTyped(wrl->GetArchive());
	++apt_count;
	char buf[200];
	sprintf(buf,"New Airport %d",apt_count);
	apt->SetParent(wrl, wrl->CountChildren());

	WED_SetCurrentAirport(inResolver, apt);
	
	wrl->CommitCommand();
}

int		WED_CanSetCurrentAirport(IResolver * inResolver, string& io_cmd_name)
{
	ISelection * sel = WED_GetSelect(inResolver);
	if (sel->GetSelectionCount() != 1) return 0;
	
	WED_Airport * want_sel = SAFE_CAST(WED_Airport, sel->GetNthSelection(0));
	if (want_sel == NULL) return 0;
	
	WED_Airport * now_sel = WED_GetCurrentAirport(inResolver);
	
	string n;
	want_sel->GetName(n);
	if (want_sel != now_sel)
	io_cmd_name = string("Edit Airport ") + n;
	else
	io_cmd_name = string("Editing Airport ") + n;
	
	return want_sel != now_sel;
}


void	WED_DoSetCurrentAirport(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	if (sel->GetSelectionCount() != 1) return;
	
	WED_Airport * want_sel = SAFE_CAST(WED_Airport, sel->GetNthSelection(0));
	if (want_sel == NULL) return;

	string apt_name;
	want_sel->GetName(apt_name);
	string cmd = string("Make ") + apt_name + string("current");

	want_sel->StartCommand(cmd.c_str());
	
	WED_SetCurrentAirport(inResolver, want_sel);
	
	want_sel->CommitCommand();

	
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

int		WED_CanClear(IResolver * resolver)
{
	ISelection * s = WED_GetSelect(resolver);
	return s->GetSelectionCount() > 0;
}

void	WED_DoClear(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *> (sel);

	set<WED_Thing *>	who;
	WED_GetSelectionRecursive(resolver, who);
	if (who.empty()) return;
	
	op->StartOperation("Clear");
	
	sel->Clear();
	
	for (set<WED_Thing *>::iterator i = who.begin(); i != who.end(); ++i)
	{
		(*i)->SetParent(NULL, 0);
		(*i)->Delete();
	}
	
	op->CommitOperation();
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

int		WED_CanReorder(IResolver * resolver, int direction, int to_end)
{
	ISelection * sel = WED_GetSelect(resolver);
	
	if (sel->GetSelectionCount() == 0) return 0;
	
	WED_Thing * obj = dynamic_cast<WED_Thing *>(sel->GetNthSelection(0));
	if (obj == NULL) return 0;
	if (obj->GetParent() == NULL) return 0;
	
	if (sel->IterateSelection(Iterate_ParentMismatch, obj->GetParent())) return 0;	
																		 return 1;
}

void	WED_DoReorder (IResolver * resolver, int direction, int to_end)
{
	vector<WED_Thing *>	sel;
	vector<WED_Thing *>::iterator it;
	vector<WED_Thing *>::reverse_iterator rit;
	WED_GetSelectionInOrder(resolver, sel);
	
	if (sel.empty()) return;
	
	WED_Thing * parent = sel.front()->GetParent();
	int count = parent->CountChildren();
	
	parent->StartCommand("Reorder");
	
	int insert_slot = sel.front()->GetMyPosition();
	if (!to_end)
	{
		for (it = sel.begin(); it != sel.end(); ++it)
		if (direction > 0)		insert_slot = max(insert_slot,(*it)->GetMyPosition());
		else					insert_slot = min(insert_slot,(*it)->GetMyPosition());
		
		if (direction < 0)  { --insert_slot; if (insert_slot < 0	 ) insert_slot = 0; }
		else				{ ++insert_slot; if (insert_slot >= count) insert_slot = count-1; }
	} else {
		insert_slot = (direction < 0) ? 0 : (count-1);
	}
	
	if (direction < 0)
	for (rit = sel.rbegin(); rit != sel.rend(); ++rit)
	{
		(*rit)->SetParent(parent, insert_slot);
	}
	else
	for (it = sel.begin(); it != sel.end(); ++it)
	{
		(*it)->SetParent(parent, insert_slot);
	}
	
	parent->CommitCommand();
}

int		WED_CanMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot)
{
	ISelection * sel = WED_GetSelect(resolver);
	
	// If the selection is nested, e.g. a parent of the selection is part of the selection, well, we can't
	// reorder, as it would involve "flattening" the selection, which is NOT what the user expects!!
	if (WED_IsSelectionNested(resolver)) return 0;	
	
	// We cannot move a grandparent of the container INTO the container - that'd make a loop.
	// (This includes moving the container into itself.
	if (sel->IterateSelection(Iterate_IsParentOf, dest)) return 0;
	
	if (dynamic_cast<IGISComposite *>(dest) == NULL) return 0;
	
	if (Iterate_IsOrParentAirport(dest, NULL))
	{
		// We are going into an airport.  DO NOT allow an airport into another one.
		if (sel->IterateSelection(Iterate_IsOrChildAirport, NULL)) return 0;
	}
	else
	{
		// Not going into an airport.  If we need to, well, we can't do this.
		if (sel->IterateSelection(	Iterate_ChildRequiresAirport, NULL)) return 0;
	}	
	return 1;
}

void	WED_DoMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot)
{
	vector<WED_Thing *>	sel;
	vector<WED_Thing *>::iterator it;
	
	WED_GetSelectionInOrder(resolver, sel);
	
	if (sel.empty()) return;
	
	dest->StartCommand("Reorder");
	
	for (it = sel.begin(); it != sel.end(); ++it)
	{
		// Note that if we are moving an object to LATER in its OWN parent, then 
		// 1. We don't need to increment our destination, because moving this guy effectively moves everyone up a notch and
		// 2. We need to move the object one slot higher because the positio is counted WTIHOUT its old position being taken into account.
		if ((*it)->GetParent() == dest && (*it)->GetMyPosition() < dest_slot)		(*it)->SetParent(dest, dest_slot-1);	
		else																		(*it)->SetParent(dest, dest_slot++);	
	}
	
	dest->CommitCommand();	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

int		WED_CanSelectAll(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	return wrl->CountChildren() > 0;
}

void	WED_DoSelectAll(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	wrl->StartOperation("Select All");
	sel->Clear();
	int ct = wrl->CountChildren();
	for (int n = 0; n < ct; ++n)
		sel->Insert(wrl->GetNthChild(n));
	wrl->CommitOperation();
}

int		WED_CanSelectNone(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	return sel->GetSelectionCount() > 0;
}

void	WED_DoSelectNone(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	if (op) op->StartOperation("Select None");
	sel->Clear();
	if (op) op->CommitOperation();
}

int		WED_CanSelectParent(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	// Can't sel if sel is empty
	if (sel->GetSelectionCount() == 0) return 0;
	
	// IF we don't have at least ONE non-world sel, we can't sel
	if (!sel->IterateSelection(Iterate_NotMatchesThing,WED_GetWorld(resolver))) return 0;
	return 1;
}

void	WED_DoSelectParent(IResolver * resolver)
{
	vector<WED_Thing *>	things;
	WED_Thing * wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelection(Iterate_GetSelectThings,&things);
	if (things.empty()) return;
	op->StartOperation("Select Parent");
	sel->Clear();
	for (vector<WED_Thing *>::iterator i = things.begin(); i != things.end(); ++i)
	if (*i == wrl)
		sel->Select(*i);
	else
		sel->Select((*i)->GetParent());
	op->CommitOperation();
}

int		WED_CanSelectChildren(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	return (sel->IterateSelection(Iterate_IsNonEmptyComposite, NULL));
}

void	WED_DoSelectChildren(IResolver * resolver)
{
	IGISComposite * comp;
	vector<WED_Thing *>	things;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelection(Iterate_GetSelectThings,&things);
	if (things.empty()) return;
	op->StartOperation("Select Children");
	sel->Clear();
	
	int ctr;
	int n;
	for (vector<WED_Thing *>::iterator i = things.begin(); i != things.end(); ++i)
	{
		if ((comp = dynamic_cast<IGISComposite *>(*i)) != NULL && comp->GetGISClass() == gis_Composite && (ctr=comp->GetNumEntities()) > 0)
		for (n = 0; n < ctr; ++n)
			sel->Insert(comp->GetNthEntity(n));
		else
			sel->Insert(*i);
	}
	op->CommitOperation();	
}

int		WED_CanSelectVertices(IResolver * resolver)
{
	// we can select vertices if all sel items are of gis type polygon or point seq
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelection(Iterate_IsNotStructuredObject, NULL)) return 0;
	return 1;
}

void	WED_DoSelectVertices(IResolver * resolver)
{
	vector<IGISPointSequence *>	seqs;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelection(Iterate_CollectChildPointSequences, &seqs);
	op->StartOperation("Select Vertices");
	sel->Clear();
	for(vector<IGISPointSequence *>::iterator s=  seqs.begin(); s != seqs.end(); ++s)
	{
		int pc = (*s)->GetNumPoints();
		for (int p = 0; p < pc; ++p)
			sel->Insert((*s)->GetNthPoint(p));
	}
	op->CommitOperation();
}

int		WED_CanSelectPolygon(IResolver * resolver)
{
	// we can select our parent poly if everyone's parent is a point seq
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelection(Iterate_IsNotPartOfStructuredObject, NULL)) return 0;
	return 1;
}

void	WED_DoSelectPolygon(IResolver * resolver)
{
	vector<WED_Thing *>	things;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelection(Iterate_GetSelectThings,&things);
	if (things.empty()) return;
	op->StartOperation("Select Polygon");
	sel->Clear();
	for (vector<WED_Thing *>::iterator i = things.begin(); i != things.end(); ++i)
	{
		WED_Thing * parent = (*i)->GetParent();
		WED_Thing * keeper = NULL;
		if (parent)
		{
			if (Iterate_IsStructuredObject(parent, NULL)) keeper = parent;
			WED_Thing * grandparent = parent->GetParent();
			if (grandparent)
			{
				if (Iterate_IsStructuredObject(grandparent, NULL)) keeper = grandparent;
			}
		}
		if (keeper) sel->Insert(keeper);
	}

	op->CommitOperation();
}
