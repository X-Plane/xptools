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
#include "DEMIO.h"
#include "WED_Thing.h"
#include "WED_Airport.h"
#include "WED_ATCFrequency.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCTimeRule.h"
#include "WED_ATCWindRule.h"
#include "WED_AirportNode.h"
#include "WED_Group.h"

#include "BitmapUtils.h"
#include "GISUtils.h"
#include "FileUtils.h"
#include "PlatformUtils.h"

#include "WED_Ring.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_UIDefs.h"
#include "ILibrarian.h"
#include "WED_MapZoomerNew.h"
#include "WED_OverlayImage.h"
#include "WED_ObjPlacement.h"
#include "WED_AirportChain.h"
#include "WED_TextureNode.h"
#include "WED_Airport.h"
#include "XESConstants.h"
#include "WED_TaxiRouteNode.h"
#include "WED_TruckParkingLocation.h"
#include "WED_RoadNode.h"
#include "WED_ObjPlacement.h"
#include "WED_LibraryMgr.h"
#include "WED_RampPosition.h"
#include "WED_Menus.h"
#include "WED_MetaDataKeys.h"
#include "WED_ResourceMgr.h"
#include "XObjDefs.h"
#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"
#include "WED_GISEdge.h"
#include "GISUtils.h"
#include "MathUtils.h"
#include "WED_EnumSystem.h"
#include "CompGeomUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_Orthophoto.h"
#include "WED_FacadePlacement.h"

#include <iterator>
#include <sstream>

#if DEV
#include "WED_Globals.h"
#include <iostream>
#endif

#define DOUBLE_PT_DIST (1.0 * MTR_TO_DEG_LAT)

int		WED_CanGroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	WED_Thing * wrl = WED_GetWorld(inResolver);
	DebugAssert(sel != NULL);

	// Can't group the world itself!
	if (sel->IterateSelectionOr(Iterate_MatchesThing,wrl)) return 0;

	// Can't group a piece of a structured object - would break its internal make-up.
	if (sel->IterateSelectionOr(Iterate_IsPartOfStructuredObject, NULL)) return 0;

	int has_airport = sel->IterateSelectionOr(Iterate_IsClass, (void*) WED_Airport::sClass);

	WED_Thing * global_parent = WED_FindParent(sel, NULL, NULL);
	if (global_parent == NULL) return 0;

	if (Iterate_IsOrParentClass(global_parent, (void*) WED_Airport::sClass))
	{
		// We are going into an airport.  DO NOT allow an airport into another one.
		if (sel->IterateSelectionOr(Iterate_IsOrChildClass, (void *) WED_Airport::sClass)) return 0;
	}
	else
	{
		// Not going into an airport.  If we need to, well, we can't do this.
		if (sel->IterateSelectionOr(	Iterate_ChildRequiresClass, (void *) WED_Airport::sClass)) return 0;

	}
	return 1;
}

int		WED_CanUngroup(IResolver * inResolver)
{
	ISelection * sel = WED_GetSelect(inResolver);
	DebugAssert(sel != NULL);

	// Can't ungroup something that is not a group.  
	if(sel->IterateSelectionOr(Iterate_IsNotGroup, NULL)) return 0;

	// The world is a group.  If the user tries to ungroup it, the world is destroyed and, well, life on the Erf ends.  So...don't allow that!
	WED_Thing * wrl = WED_GetWorld(inResolver);
	if (sel->IterateSelectionOr(Iterate_MatchesThing,wrl)) return 0;

	// No selection, no ungrouping.
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
	vector<ISelectable *> items;
	sel->GetSelectionVector(items);

	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Ungroup");

	sel->Clear();

	for (vector<ISelectable *>::iterator i = items.begin(); i != items.end(); ++i)
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

void	WED_DoMakeNewOverlay(IResolver * inResolver, WED_MapZoomerNew * zoomer)
{
	char * path = GetMultiFilePathFromUser("Please pick an image file", "Open", FILE_DIALOG_PICK_IMAGE_OVERLAY);
	if(path)
	{
		WED_Thing *    wrl = WED_GetWorld(inResolver);
		WED_Archive * arch = wrl->GetArchive();
		ISelection *   sel = WED_GetSelect(inResolver);
		ILibrarian *   lib = WED_GetLibrarian(inResolver);
		char * free_me = path;

		wrl->StartOperation("Add Overlay Image");
		sel->Clear();
		
		while(*path)
		{
			WED_Ring * rng = WED_RingfromImage(path, arch, zoomer, false);
			if (rng)
			{
				WED_OverlayImage * img = WED_OverlayImage::CreateTyped(arch);
				rng->SetParent(img,0);
				img->SetParent(wrl,0);
				sel->Select(img);

				string img_path(path);
				lib->ReducePath(img_path);
				img->SetImage(img_path);
				img->SetName(img_path);
			}
			path = path + strlen(path)+1;
		}
		
		if(sel->GetSelectionCount() == 0)
			wrl->AbortOperation();
		else
			wrl->CommitOperation();
		free(free_me);
	}
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

bool	WED_CanAddMetaData(IResolver * inResolver, int command)
{
	ISelection * sel = WED_GetSelect(inResolver);
	if (sel->GetSelectionCount() != 1) return 0;

	WED_Airport * sel_airport = SAFE_CAST(WED_Airport, sel->GetNthSelection(0));
	if (sel_airport == NULL)
	{
		return 0;
	}
	else
	{
		return !sel_airport->ContainsMetaDataKey(META_KeyName(command));
	}
}

void WED_DoAddMetaData(IResolver * inResolver, int command)
{
	ISelection * sel = WED_GetSelect(inResolver);
	if (sel->GetSelectionCount() != 1) return;

	WED_Airport * want_sel = SAFE_CAST(WED_Airport, sel->GetNthSelection(0));
	if (want_sel == NULL) return;

	MetaDataKey key_info = META_KeyInfo(command);
	want_sel->StartOperation(string("Add Metadata Key " + key_info.display_text).c_str());
	want_sel->StateChanged();
	want_sel->AddMetaDataKey(key_info.name, "");
	want_sel->CommitOperation();
}

int		WED_CanMakeNewATCFreq(IResolver * inResolver)
{
	return WED_HasSingleSelectionOfType(inResolver, WED_Airport::sClass) != NULL;
}

#if AIRPORT_ROUTING

int		WED_CanMakeNewATCFlow(IResolver * inResolver)
{
	return WED_HasSingleSelectionOfType(inResolver, WED_Airport::sClass) != NULL;
}

int		WED_CanMakeNewATCRunwayUse(IResolver * inResolver)
{
	return WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass) != NULL;
}

int		WED_CanMakeNewATCTimeRule(IResolver * inResolver)
{
	return WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass) != NULL;
}

int		WED_CanMakeNewATCWindRule(IResolver * inResolver)
{
	return WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass) != NULL;
}

#endif

void	WED_DoMakeNewATCFreq(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_Airport::sClass);
	now_sel->StartOperation("Add ATC Frequency");
	WED_ATCFrequency * f=  WED_ATCFrequency::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	now_sel->CommitOperation();
}

#if AIRPORT_ROUTING

void	WED_DoMakeNewATCFlow(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_Airport::sClass);
	now_sel->StartOperation("Add ATC Flow");
	WED_ATCFlow * f=  WED_ATCFlow::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	f->SetName("Unnamed ATC Flow");
	
	const WED_Airport * airport = WED_GetParentAirport(f);
	if(airport)
	{
		set<int> legal;
		WED_GetAllRunwaysOneway(airport, legal);
		
		if(!legal.empty())
			f->SetPatternRunway(*legal.begin());		
	}
	
	now_sel->CommitOperation();
}

void	WED_DoMakeNewATCRunwayUse(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass);
	now_sel->StartOperation("Add ATC Runway Use");
	WED_ATCRunwayUse * f=  WED_ATCRunwayUse::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	f->SetName("Unnamed Runway Use");
	
	const WED_Airport * airport = WED_GetParentAirport(f);
	if(airport)
	{
		set<int> legal;
		WED_GetAllRunwaysOneway(airport, legal);
		
		if(!legal.empty())
			f->SetRunway(*legal.begin());		
	}
	
	now_sel->CommitOperation();
}

void	WED_DoMakeNewATCWindRule(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass);
	now_sel->StartOperation("Add ATC Wind Rule");
	WED_ATCWindRule * f=  WED_ATCWindRule::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	f->SetName("Unnamed Wind Rule");
	now_sel->CommitOperation();
}

void	WED_DoMakeNewATCTimeRule(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass);
	now_sel->StartOperation("Add ATC Time Rule");
	WED_ATCTimeRule * f=  WED_ATCTimeRule::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	f->SetName("Unnamed Time Rule");
	now_sel->CommitOperation();
}

#endif

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

static bool WED_NoLongerViable(WED_Thing * t, bool strict)
{
	IGISPointSequence * sq = dynamic_cast<IGISPointSequence *>(t);
	if (sq)
	{
		int min_children = 2;
		WED_Thing * parent = t->GetParent();
		WED_FacadePlacement * facade;
		if (parent && dynamic_cast<WED_OverlayImage *>(parent))
			min_children = 4;
		else if (parent && (facade = dynamic_cast<WED_FacadePlacement *>(parent)))
			min_children = facade->GetTopoMode() == WED_FacadePlacement::topo_Chain ? 2 : 3;  // allow some 2-node facades. No strict check, as hafaces can not have holes
		else if (parent && dynamic_cast<WED_GISPolygon *>(parent))		// Strict rules for delete key require 3 points to a polygon - prevents degenerate holes.
			min_children = strict ? 3 : 2;								// Loose requirements for repair require 2 - matches minimum apt.dat spec.
		if(t->CountSources() == 2 && t->GetNthSource(0) == NULL) return true;
		if(t->CountSources() == 2 && t->GetNthSource(1) == NULL) return true;

		if ((t->CountChildren() + t->CountSources()) < min_children)
			return true;
	}

	if(SAFE_CAST(WED_TaxiRouteNode,t) &&
		SAFE_CAST(IGISComposite,t->GetParent()) &&
		t->CountViewers() == 0)
		return true;
#if ROAD_EDITING
	if(SAFE_CAST(WED_RoadNode,t) &&
		SAFE_CAST(IGISComposite,t->GetParent()) &&
		t->CountViewers() == 0)
		return true;
#endif
	IGISPolygon * p = dynamic_cast<IGISPolygon *>(t);
	if (p && t->CountChildren() == 0)
		return true;
		
	return false;
}

static void WED_RecursiveDelete(set<WED_Thing *>& who)
{
	// This is sort of a scary mess.  We are going to delete everyone in 'who'.  But this might have
	// some reprecussions on other objects.
	while(!who.empty())
	{
		set<WED_Thing *>	chain;		// Chain - dependents who _might_ need to be nuked!
	
		for (set<WED_Thing *>::iterator i = who.begin(); i != who.end(); ++i)
		{
			// Children get detached...just in case.  They should be fully 
			// contained in our recursive selection.
			while((*i)->CountChildren())
				(*i)->GetNthChild(0)->SetParent(NULL,0);

			// Our parent has to be reconsidered - maybe the parent can't live without its kids?
			WED_Thing * p = (*i)->GetParent();
			if (p)
				chain.insert(p);

			set<WED_Thing *> viewers;
			(*i)->GetAllViewers(viewers);			
			
			// All of our viewers lose a source.  
			for(set<WED_Thing *>::iterator v = viewers.begin(); v != viewers.end(); ++v)
				(*v)->RemoveSource(*i);
			
			// And - any one of our viewers might now be hosed, due to a lack of sources!
			chain.insert(viewers.begin(), viewers.end());
			
			while((*i)->CountSources() > 0)
			{
				chain.insert((*i)->GetNthSource(0));
				(*i)->RemoveSource((*i)->GetNthSource(0));
			}
			
			(*i)->SetParent(NULL, 0);
			(*i)->Delete();
		}
		
		// If we had a guy who was going to be potentially unviable, but he was elsewherein the selection,
		// we need to not consider him.  With viewers, this can happen!
		for (set<WED_Thing *>::iterator i = who.begin(); i != who.end(); ++i)
			chain.erase(*i);

		who.clear();
		for(set<WED_Thing *>::iterator i = chain.begin(); i != chain.end(); ++i)
		{
			if (WED_NoLongerViable(*i, true))		// Strict viability for delete key - be aggressive about not making junk data DURING editing.
				who.insert(*i);						// User can alwys hit undo.
		}
	}
}

int		WED_CanClear(IResolver * resolver)
{
	ISelection * s = WED_GetSelect(resolver);
	return s->GetSelectionCount() > 0;
}

void	WED_DoClear(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *> (sel);

	set<WED_Thing *>	who;		// Who - objs to be nuked!

	WED_GetSelectionRecursive(resolver, who);
	if (who.empty()) return;

	op->StartOperation("Clear");

	sel->Clear();

	set<WED_AirportNode *>	common_nodes;
	for (set<WED_Thing *>::iterator i = who.begin(); i != who.end(); ++i)
	{
		WED_AirportNode * n = dynamic_cast<WED_AirportNode*>(*i);
		if(n && n->CountViewers() == 2)
			common_nodes.insert(n);
	}
	for(set<WED_AirportNode *>::iterator n = common_nodes.begin(); n != common_nodes.end(); ++n)
	{
		set<WED_Thing *> viewers;
		(*n)->GetAllViewers(viewers);
		DebugAssert(viewers.size() == 2);
		set<WED_Thing *>::iterator v =viewers.begin();
		WED_Thing * e1 = *v;
		++v;
		WED_Thing * e2 = *v;
		
		// We are goin to find E2's destination - that's where E1 will point.
		WED_Thing *				other_node = e2->GetNthSource(0);
		if(other_node == *n)	other_node = e2->GetNthSource(1);
		DebugAssert(other_node != *n);
		
		// Adjust E1 to span to E2's other node.
		e1->ReplaceSource(*n, other_node);
		
		// Now nuke E2 and ourselves.
		
		e2->RemoveSource(*n);
		e2->RemoveSource(other_node);
		who.insert(e2);
	}

	WED_RecursiveDelete(who);

	WED_SetAnyAirport(resolver);

	op->CommitOperation();

}

int		WED_CanCrop(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0)	return 0;
										return 1;
}

static int	AccumSelectionAndParents(ISelectable * what, void * ref)
{
	set<WED_Thing *> * container = (set<WED_Thing *> *) ref;
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	while(who)
	{
		container->insert(who);
		who = who->GetParent();
	}
	return 0;
}

static void AccumDead(WED_Thing * who, set<WED_Thing *>& nuke_em, const set<WED_Thing *>& must_keep, ISelection * sel)
{
	if (must_keep.count(who) == 0)
	{
		nuke_em.insert(who);
	}
	if (!sel->IsSelected(who))
	{
		int nc = who->CountChildren();
		for (int n = 0; n < nc; ++n)
			AccumDead(who->GetNthChild(n), nuke_em, must_keep, sel);
	}
}

void	WED_DoCrop(IResolver * resolver)
{
	ISelection *	sel = WED_GetSelect(resolver);
	WED_Thing *		wrl = WED_GetWorld(resolver);
	set<WED_Thing *>	must_keep;
	set<WED_Thing *>	nuke_em;
	set<WED_Thing *>	chain;

	sel->IterateSelectionOr(AccumSelectionAndParents, &must_keep);
	AccumDead(wrl, nuke_em, must_keep, sel);

	if (nuke_em.empty()) return;

	wrl->StartOperation("Crop");
	sel->Clear();

	while(!nuke_em.empty())
	{
		for (set<WED_Thing *>::iterator i = nuke_em.begin(); i != nuke_em.end(); ++i)
		{
			WED_Thing * p = (*i)->GetParent();
			if (p && nuke_em.count(p) == 0)
//			if (must_keep.count(p) == 0)
				chain.insert(p);
			(*i)->SetParent(NULL, 0);
			(*i)->Delete();
		}

		nuke_em.clear();
		for(set<WED_Thing *>::iterator i = chain.begin(); i != chain.end(); ++i)
		{
			if (WED_NoLongerViable(*i, true))
				nuke_em.insert(*i);
		}

		chain.clear();
	}

	WED_SetAnyAirport(resolver);

	wrl->CommitOperation();
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

	if (sel->IterateSelectionOr(Iterate_ParentMismatch, obj->GetParent())) return 0;
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
	if (sel->IterateSelectionOr(Iterate_IsParentOf, dest)) return 0;

	// If our destination isn't a folder, just bail now...only certain types can contain other types, like, at all.
	if(!WED_IsFolder(dest)) return 0;

	// No nested airports.  This is sort of a special case..we need to make sure no airport is inside another airport.
	// Most other types only have demands about their, supervisor, not about who ISN'T their supervisor.
	if (Iterate_IsOrParentClass(dest, (void*) WED_Airport::sClass))
	{
		// We are going into an airport.  DO NOT allow an airport into another one.
		if (sel->IterateSelectionOr(Iterate_IsOrChildClass, (void *) WED_Airport::sClass)) return 0;
	}

	#if AIRPORT_ROUTING
	// No nested flows either...
	if (Iterate_IsOrParentClass(dest, (void*) WED_ATCFlow::sClass))
	{
		if (sel->IterateSelectionOr(Iterate_IsOrChildClass, (void *) WED_ATCFlow::sClass)) return 0;
	}
	#endif

	// If the parent of any selection isn't a folder, don't allow the re-org.
	if(sel->IterateSelectionOr(Iterate_IsPartOfStructuredObject, NULL)) return 0;
	
	// Finally, we need to make sure that everyone in the selection is going to get their needs met.
	set<string>	required_parents;
	sel->IterateSelectionOr(Iterate_CollectRequiredParents, &required_parents);
	for(set<string>::iterator s = required_parents.begin(); s != required_parents.end(); ++s)
		if(!Iterate_IsOrParentClass(dest, (void*) s->c_str()))
			return 0;
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
	if (!sel->IterateSelectionOr(Iterate_NotMatchesThing,WED_GetWorld(resolver))) return 0;
	return 1;
}

void	WED_DoSelectParent(IResolver * resolver)
{
	vector<WED_Thing *>	things;
	WED_Thing * wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelectionOr(Iterate_CollectThings,&things);
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
	return (sel->IterateSelectionOr(Iterate_IsNonEmptyComposite, NULL));
}

void	WED_DoSelectChildren(IResolver * resolver)
{
	IGISComposite * comp;
	vector<WED_Thing *>	things;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelectionOr(Iterate_CollectThings,&things);
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
	if (sel->IterateSelectionOr(Iterate_IsNotStructuredObject, NULL)) return 0;
	return 1;
}

void	WED_DoSelectVertices(IResolver * resolver)
{
	vector<IGISPointSequence *>	seqs;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelectionOr(Iterate_CollectChildPointSequences, &seqs);
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
	if (sel->IterateSelectionOr(Iterate_IsNotPartOfStructuredObject, NULL)) return 0;
	return 1;
}

void	WED_DoSelectPolygon(IResolver * resolver)
{
	vector<WED_Thing *>	things;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelectionOr(Iterate_CollectThings,&things);
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

int		WED_CanSelectConnected(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	return 1;
}

void	WED_DoSelectConnected(IResolver * resolver)
{
	vector<WED_Thing *>	things;
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	sel->IterateSelectionOr(Iterate_CollectThings,&things);
	if (things.empty()) return;
	op->StartOperation("Select Connected");
	set<WED_Thing *>	visited, to_visit;
	std::copy(things.begin(),things.end(), inserter(to_visit,to_visit.end()));
	
	while(!to_visit.empty())
	{
		WED_Thing * i = *to_visit.begin();
		to_visit.erase(to_visit.begin());
		visited.insert(i);
		
		int s = i->CountSources();
		for(int ss = 0; ss < s; ++ss)
		{
			WED_Thing * src = i->GetNthSource(ss);
			if(visited.count(src) == 0)
				to_visit.insert(src);
		}
		set<WED_Thing *>	viewers;
		i->GetAllViewers(viewers);
		set_difference(viewers.begin(), viewers.end(), visited.begin(), visited.end(), inserter(to_visit, to_visit.end()));
	}
	
	for(set<WED_Thing *>::iterator v = visited.begin(); v != visited.end(); ++v)
	{
		sel->Insert(*v);
	}
	op->CommitOperation();
}

void select_zero_recursive(WED_Thing * t, ISelection * s)
{
	IGISEdge * e = dynamic_cast<IGISEdge *>(t);
	if(e)
	if(e->GetNthPoint(0) == e->GetNthPoint(1))
		s->Insert(t);
	int nn = t->CountChildren();
	for(int n = 0; n < nn; ++n)
		select_zero_recursive(t->GetNthChild(n), s);
}

bool WED_DoSelectZeroLength(IResolver * resolver, WED_Thing * sub_tree)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Select Zero-Length Edges");
	sel->Clear();
	select_zero_recursive(sub_tree ? sub_tree : WED_GetWorld(resolver), sel);
	
	if(sel->GetSelectionCount() == 0)
	{
		op->AbortOperation();
		return false;
	}
	else
	{
		op->CommitOperation();
		return true;
	}
}

bool WED_DoSelectDoubles(IResolver * resolver, WED_Thing * sub_tree)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Select Zero-Length Edges");
	sel->Clear();

	vector<WED_Thing *> pts;
	CollectRecursive(sub_tree == NULL ? WED_GetWorld(resolver) : sub_tree, back_inserter(pts), ThingNotHidden, IsGraphNode);
	
	// Ben says: yes this totally sucks - replace it someday?
	for(int i = 0; i < pts.size(); ++i)
	{
		for(int j = i + 1; j < pts.size(); ++j)
		{
			IGISPoint * ii = dynamic_cast<IGISPoint *>(pts[i]);
			IGISPoint * jj = dynamic_cast<IGISPoint *>(pts[j]);
			DebugAssert(ii != jj);
			DebugAssert(ii);
			DebugAssert(jj);
			Point2 p1, p2;
			ii->GetLocation(gis_Geo, p1);
			jj->GetLocation(gis_Geo, p2);
			
			if(p1.squared_distance(p2) < (DOUBLE_PT_DIST*DOUBLE_PT_DIST))
			{
				sel->Insert(pts[i]);
				sel->Insert(pts[j]);
				break;
			}			
		}
	}

	if(sel->GetSelectionCount() == 0)
	{
		op->AbortOperation();
		return false;
	}
	else
	{
		op->CommitOperation();
		return true;
	}	
}

set<WED_GISEdge*> do_select_crossing(vector<WED_GISEdge* > edges)
{
	set<WED_GISEdge*> crossed_edges;
	// Ben says: yes this totally sucks - replace it someday?
	for (int i = 0; i < edges.size(); ++i)
	{
		for (int j = i + 1; j < edges.size(); ++j)
		{
			IGISEdge * ii = edges[i];
			IGISEdge * jj = edges[j];
			DebugAssert(ii != jj);
			DebugAssert(ii);
			DebugAssert(jj);
			Segment2 s1, s2;
			Bezier2 b1, b2;
			bool isb1, isb2;

			isb1 = ii->GetSide(gis_Geo, 0, s1, b1);
			isb2 = jj->GetSide(gis_Geo, 0, s2, b2);
			
			if (isb1 || isb2)
			{   // should never get here, as edges (used for ATC routes only) are not supposed to have bezier segments
				if (b1.intersect(b2, 10))
				{
					crossed_edges.insert(edges[i]);
					crossed_edges.insert(edges[j]);
				}
			}
			else 
			{
				Point2 x;
				if (s1.p1 != s2.p1 &&
					s1.p2 != s2.p2 &&
					s1.p1 != s2.p2 &&
					s1.p2 != s2.p1)
				{
					if (s1.intersect(s2, x))
					{
						crossed_edges.insert(edges[i]);
						crossed_edges.insert(edges[j]);
					}
				}
			}
		}
	}

	return crossed_edges;
}

bool WED_DoSelectCrossing(IResolver * resolver, WED_Thing * sub_tree)
{
	//--Keep-----------
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Select Crossing Edges");
	sel->Clear();
	//-----------------

	vector<WED_GISEdge *> edges;
	CollectRecursive(sub_tree == NULL ? WED_GetWorld(resolver) : sub_tree, back_inserter(edges), ThingNotHidden, IsGraphEdge);
	
	set<WED_GISEdge *> crossed_edges = do_select_crossing(edges);
	sel->Insert(set<ISelectable*>(crossed_edges.begin(), crossed_edges.end()));

	//--Keep-------------------------
	if(sel->GetSelectionCount() == 0)
	{
		op->AbortOperation();
		return false;
	}
	else
	{
		op->CommitOperation();
		return true;
	}
	//-------------------------------
}

static bool get_any_resource_for_thing(WED_Thing * thing, string& r)
{
	if(thing->GetClass() == WED_ObjPlacement::sClass)
	{
		WED_ObjPlacement * o = dynamic_cast<WED_ObjPlacement *>(thing);
		o->GetResource(r);
		return true;
	}
	return false;
}

bool HasMissingResource(WED_Thing * t)
{
	static WED_LibraryMgr * mgr = WED_GetLibraryMgr(t->GetArchive()->GetResolver());
	string r;
	if(!get_any_resource_for_thing(t,r))
		return false;
	
	return mgr->GetResourceType(r) == res_None;	
}

bool HasLocalResource(WED_Thing * t)
{
	static WED_LibraryMgr * mgr = WED_GetLibraryMgr(t->GetArchive()->GetResolver());
	string r;
	if(!get_any_resource_for_thing(t,r))
		return false;
	
	return mgr->IsResourceLocal(r);
}

bool HasLibraryResource(WED_Thing * t)
{
	static WED_LibraryMgr * mgr = WED_GetLibraryMgr(t->GetArchive()->GetResolver());
	string r;
	if(!get_any_resource_for_thing(t,r))
		return false;
	
	return mgr->IsResourceLibrary(r);
}

bool HasDefaultResource(WED_Thing * t)
{
	static WED_LibraryMgr * mgr = WED_GetLibraryMgr(t->GetArchive()->GetResolver());
	string r;
	if(!get_any_resource_for_thing(t,r))
		return false;
	
	return mgr->IsResourceDefault(r);
}

bool HasThirdPartyResource(WED_Thing * t)
{
	static WED_LibraryMgr * mgr = WED_GetLibraryMgr(t->GetArchive()->GetResolver());
	string r;
	if(!get_any_resource_for_thing(t,r))
		return false;
	
	return !mgr->IsResourceDefault(r) && mgr->IsResourceLibrary(r);
}


static void DoSelectWithFilter(const char * op_name, bool (* filter)(WED_Thing * t), IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation(op_name);
	sel->Clear();

	WED_LibraryMgr * mgr = WED_GetLibraryMgr(resolver);

	vector<WED_Thing *> who;
	CollectRecursive(WED_GetWorld(resolver), back_inserter(who), ThingNotHidden, filter);
	for(vector<WED_Thing *>::iterator w = who.begin(); w != who.end(); ++w)
	{
		sel->Insert(*w);
	}
	
	op->CommitOperation();
}

void	WED_DoSelectMissingObjects(IResolver * resolver)
{
	DoSelectWithFilter("Select Missing Art Assets", HasMissingResource, resolver);
}

void	WED_DoSelectLocalObjects(IResolver * resolver)
{
	DoSelectWithFilter("Select Local Art Assets", HasLocalResource, resolver);
}

void	WED_DoSelectLibraryObjects(IResolver * resolver)
{
	DoSelectWithFilter("Select Library Art Assets", HasLibraryResource, resolver);
}

void	WED_DoSelectDefaultObjects(IResolver * resolver)
{
	DoSelectWithFilter("Select Default Art Assets", HasDefaultResource, resolver);
}

void	WED_DoSelectThirdPartyObjects(IResolver * resolver)
{
	DoSelectWithFilter("Select Third Party Art Assets", HasThirdPartyResource, resolver);
}

// Given a vector of nodes all in the same place, this routine merges them all, returning the one surviver,
// and nukes the rest.  All incoming edges of all of them are merged.  Note that any edges liknking two nodes in
// nodes are now zero length.

//Return the winning node
static WED_Thing* run_merge(vector<WED_Thing *> nodes)
{
	DebugAssert(nodes.size() > 1);
	WED_Thing * winner = nodes.front();
	
	// This takes the centroid of the nodes - if the user provides a cluster of nodes this function can snap them together.
	Point2	l(0.0,0.0);
	for(int i = 0; i < nodes.size(); ++i)
	{
		IGISPoint * p = dynamic_cast<IGISPoint *>(nodes[i]);
		DebugAssert(p);
		Point2 ll;
		p->GetLocation(gis_Geo, ll);
		l.x_ += ll.x_;
		l.y_ += ll.y_;
	}
	double r = nodes.size();
	r = 1.0f / r;
	l.x_ *= r;
	l.y_ *= r;
	
	IGISPoint * w = dynamic_cast<IGISPoint *>(winner);
	w->SetLocation(gis_Geo,l);
	
	for(int i = 1; i < nodes.size(); ++i)
	{
		WED_Thing * victim = nodes[i];
		set<WED_Thing *> viewers;
		victim->GetAllViewers(viewers);
		for(set<WED_Thing *>::iterator v = viewers.begin(); v != viewers.end(); ++v)
			(*v)->ReplaceSource(victim, winner);
		
		victim->SetParent(NULL, 0);
		victim->Delete();
	}
	return winner;
}

static int	unsplittable(ISelectable * base, void * ref)
{
	WED_Thing * t = dynamic_cast<WED_Thing *>(base);
	if (!t) return 1;
	
	// Network edges are always splittable
	if(dynamic_cast<IGISEdge *>(base))
		return 0;
	
	IGISPoint * p = dynamic_cast<IGISPoint *>(base);
	if (!p) return 1;
//	WED_AirportNode * a = dynamic_cast<WED_AirportNode *>(base);
//	if (!a) return 1;

	WED_Thing * parent = t->GetParent();
	if (!parent) return 1;

	IGISPointSequence * s = dynamic_cast<IGISPointSequence*>(parent);
	if (!s) return 1;

	if (s->GetGISClass() != gis_Ring && s->GetGISClass() != gis_Chain) return 1;

	int pos = t->GetMyPosition();
	int next = (pos							  + 1) % parent->CountChildren();
	int prev = (pos + parent->CountChildren() - 1) % parent->CountChildren();
	int okay_next = (s->GetGISClass() == gis_Ring) || next > pos;
	int okay_prev = (s->GetGISClass() == gis_Ring) || prev < pos;

	WED_Thing * tnext = okay_next ? parent->GetNthChild(next) : NULL;
	WED_Thing * tprev = okay_prev ? parent->GetNthChild(prev) : NULL;

	ISelection * sel = (ISelection*) ref;

	int okay = ((tnext && sel->IsSelected(tnext)) ||
			    (tprev && sel->IsSelected(tprev)));
	return !okay;
}

typedef	pair<ISelection *, vector<WED_Thing *> * >	hack_t;

static int	collect_splits(ISelectable * base, void * ref)
{
	hack_t * info = (hack_t *) ref;

	WED_Thing * t = dynamic_cast<WED_Thing *>(base);
	if (!t) return 0;
	IGISPoint * p = dynamic_cast<IGISPoint *>(base);
	if (!p) return 0;
//	WED_AirportNode * a = dynamic_cast<WED_AirportNode *>(base);
//	if (!a) return 0;

	WED_Thing * parent = t->GetParent();
	if (!parent) return 0;

	IGISPointSequence * s = dynamic_cast<IGISPointSequence*>(parent);
	if (!s) return 0;

	if (s->GetGISClass() != gis_Ring && s->GetGISClass() != gis_Chain) return 0;

	int pos = t->GetMyPosition();
	int next = (pos							  + 1) % parent->CountChildren();
	int okay_next = (s->GetGISClass() == gis_Ring) || next > pos;

	WED_Thing * tnext = okay_next ? parent->GetNthChild(next) : NULL;

	ISelection * sel = info->first;

	int okay = tnext && sel->IsSelected(tnext);
	if (okay)
		info->second->push_back(t);
	return 0;
}


// This functor sorts points radially from a point.  When the points are on a line segment this is a cheap way to
// order them from the anchor to the other end.
struct sort_by_distance {
	Point2	anchor;
	bool operator()(const Point2& lhs, const Point2& rhs) const {
		return anchor.squared_distance(lhs) < anchor.squared_distance(rhs);
	}
	sort_by_distance(const Point2& a) : anchor(a) { }
};

// For a given edge, this stores the splits that we found - we later sort them once they are all found.
//struct split_edge_info_t {
	//IGISEdge *				edge;
	//vector<Point2>			splits;

split_edge_info_t::split_edge_info_t(WED_GISEdge * e, bool a) : edge(e), active(a)
{ 
}

void split_edge_info_t::sort_along_edge()
{
	Point2 a;
	edge->GetNthPoint(0)->GetLocation(gis_Geo, a);
	sort(splits.begin(),splits.end(), sort_by_distance(a));
}
//};

// Simple collector of all GIS Edges in the selection.
static int collect_edges(ISelectable * base, void * ref)
{
	vector<split_edge_info_t> * edges = (vector<split_edge_info_t>*) ref;
	WED_GISEdge * e = dynamic_cast<WED_GISEdge *>(base);
	if(e)
		edges->push_back(split_edge_info_t(e,true));
	return 0;
}

int		WED_CanSplit(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return false;
	if (sel->IterateSelectionOr(unsplittable, sel)) return 0;
	return 1;
}



map<WED_Thing*,vector<WED_Thing*> > run_split_on_edges(vector<split_edge_info_t>& edges)
{
	map<WED_Thing*, vector<WED_Thing*> > new_pieces;
	//
	// This block splits overlapping GIS edges anywhere they cross.
	//

	// Step 1: run a nested for loop and find all intersections between all
	// segments...if the intersection is in the interior, we accumulate it on
	// the edge.  This is O(N^2) - a sweep line would be better if we ever have
	// data sets big enough to need it.
	for (int i = 0; i < edges.size(); ++i)
	{
		Segment2 is;

		edges[i].edge->GetNthPoint(0)->GetLocation(gis_Geo, is.p1);
		edges[i].edge->GetNthPoint(1)->GetLocation(gis_Geo, is.p2);
		for (int j = 0; j < i; ++j)
		if(edges[i].active || edges[j].active)								// At least one edge MUST be active or we do not split.
		{
			Segment2 js;
			edges[j].edge->GetNthPoint(0)->GetLocation(gis_Geo, js.p1);
			edges[j].edge->GetNthPoint(1)->GetLocation(gis_Geo, js.p2);

			if (is.p1 != is.p2 &&
				js.p1 != js.p2 &&
				is.p1 != js.p1 &&
				is.p2 != js.p2 &&
				is.p1 != js.p2 &&
				is.p2 != js.p1)
			{
				Point2 x;
				if (is.intersect(js, x))
				{
					edges[i].splits.push_back(x);
					edges[j].splits.push_back(x);
				}
			}
		}
	}

	// This will be a collection of all the nodes we _create_ by splitting, bucketed by their split point.
	// When A and B cross, we create two new nodes, Xa and Xb, in the middle of each...when done we have
	// to merge Xa and Xb to cross-link A1, A2, B1 and B2.  So we bucket Xa and Xb at point X.
	map<Point2, vector<WED_Thing *>, lesser_x_then_y >	splits;

	for (int i = 0; i < edges.size(); ++i)
	{
		// Sort in order from source to dest - we need to go in order to avoid making Z shapes
		// when splitting more than once.
		edges[i].sort_along_edge();

		// If the edge is uncrossed the user is just subdividing it - split it at the midpoint.
		if (edges[i].splits.empty())
		{
			Segment2 s;
			edges[i].edge->GetNthPoint(0)->GetLocation(gis_Geo, s.p1);
			edges[i].edge->GetNthPoint(1)->GetLocation(gis_Geo, s.p2);
			edges[i].splits.push_back(s.midpoint());
		}

		// Now we go BACKWARD from high to low - we do this because the GIS Edge's split makes the clone
		// on the "dst" side - so by breaking off the very LAST split first, we keep as "us" the part of
		// the segment containing all other splits.  We work backward.
		for (vector<Point2>::reverse_iterator r = edges[i].splits.rbegin(); r != edges[i].splits.rend(); ++r)
		{
			// If we had a 'T' then in theory SplitSide could return NULL?
			IGISPoint * split = edges[i].edge->SplitSide(*r, 0.0);
			if (split)
			{
				// Bucket our new node for merging later
				WED_Thing * t = dynamic_cast<WED_Thing *>(split);
				DebugAssert(t);
				splits[*r].push_back(t);

				// Select every incident segment - some already selected but that's okay.

				//key observation, runs before node is merged in run merge
				set<WED_Thing *> incident;
				t->GetAllViewers(incident);
				for (set<WED_Thing *>::iterator itr = incident.begin(); itr != incident.end(); ++itr)
				{
					//here is access of only two new edges map
					edge_to_child_edges_map_t::mapped_type& child_edges = new_pieces[(WED_Thing*)edges[i].edge];
					child_edges.push_back(*itr);
				}
			}
		}
	}

	// Finally for each bucketed set of nodes, merge them down to get topology.
	for (map<Point2, vector<WED_Thing *>, lesser_x_then_y>::iterator s = splits.begin(); s != splits.end(); ++s)
	{
		if (s->second.size() > 1)
			//ufuse nodes
			run_merge(s->second);
	}

	return new_pieces;
}

void	WED_DoSplit(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);

	vector<WED_Thing *> who;
	hack_t	info;
	info.first = sel;
	info.second = &who;

	sel->IterateSelectionOr(collect_splits, &info);
	
	vector<split_edge_info_t> edges;
	
	sel->IterateSelectionOr(collect_edges, &edges);
	
	if (who.empty() && edges.empty()) return;

	op->StartOperation("Split Segments.");

	//
	// This super-obtuse block splits pairs of points in a GIS Chain.
	//

	for (vector<WED_Thing *>::iterator w = who.begin(); w != who.end(); ++w)
	{
		WED_Thing * parent = (*w)->GetParent();
		IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(parent);
		WED_Thing * new_w = (WED_Thing *) (*w)->Clone();

		IGISPoint * as_p = dynamic_cast<IGISPoint *>(new_w);
		IGISPoint_Bezier * as_bp = dynamic_cast<IGISPoint_Bezier *>(new_w);

		Segment2	seg;
		Bezier2		bez;

//		set<int> attrs;
//		node->GetAttributes(attrs);
///		new_node->SetAttributes(attrs);

		if (seq->GetSide(gis_Geo,(*w)->GetMyPosition(),seg,bez))
		{
			IGISPoint_Bezier * pre = dynamic_cast<IGISPoint_Bezier *>(*w);
			IGISPoint_Bezier * follow = dynamic_cast<IGISPoint_Bezier *>(parent->GetNthChild(((*w)->GetMyPosition()+1) % parent->CountChildren()));
			DebugAssert(as_bp);
			DebugAssert(pre);
			DebugAssert(follow);
			Bezier2	b1, b2;
			bez.partition(b1,b2);
			as_bp->SetLocation(gis_Geo,b2.p1);
			as_bp->SetSplit(false);
			as_bp->SetControlHandleHi(gis_Geo,b2.c1);
			pre->SetSplit(true);
			pre->SetControlHandleHi(gis_Geo,b1.c1);
			follow->SetSplit(true);
			follow->SetControlHandleLo(gis_Geo,b2.c2);
			if(as_bp->HasLayer(gis_UV))
			{
				seq->GetSide(gis_UV,(*w)->GetMyPosition(),seg,bez);
				bez.partition(b1,b2);
				as_bp->SetLocation(gis_UV,b2.p1);
				as_bp->SetControlHandleHi(gis_UV,b2.c1);
				as_bp->SetControlHandleLo(gis_UV,b1.c2);
				pre->SetControlHandleHi(gis_UV,b1.c1);
				follow->SetControlHandleLo(gis_UV,b2.c2);			
			}
		}
		else
		{
			DebugAssert(as_p);
			as_p->SetLocation(gis_Geo,seg.midpoint());
			if(as_p->HasLayer(gis_UV))
			{
				seq->GetSide(gis_UV,(*w)->GetMyPosition(),seg,bez);			
				as_p->SetLocation(gis_UV,seg.midpoint());
			}
		}
		new_w->SetParent(parent, (*w)->GetMyPosition() + 1);
		string name;
		new_w->GetName(name);
		name += ".1";
		new_w->SetName(name);

		sel->Insert(new_w);
	}
	
	//Add every single edge and child edges generated
	edge_to_child_edges_map_t new_pieces = run_split_on_edges(edges);
	for (edge_to_child_edges_map_t::iterator itr = new_pieces.begin(); itr != new_pieces.end(); ++itr)
	{
		sel->Insert(itr->first);
		for(vector<WED_Thing *>::iterator nt = itr->second.begin(); nt != itr->second.end(); ++nt)
			sel->Insert(*nt);
	}

	op->CommitOperation();
}

static int collect_pnts(ISelectable * base,void * ref)
{
	vector<IGISPoint *> * points = (vector<IGISPoint *> *) ref;
	IGISPoint * p = dynamic_cast<IGISPoint *>(base);
	if(p)
	{
		points->push_back(p);
		return 1;
	}
	return 0;
}

int		WED_CanAlign(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() < 3 ) return false;
	if (sel->IterateSelectionOr(Iterate_IsStructuredObject, NULL)) return 0;
	// taxi route nodes are part of structured objects
	//if (sel->IterateSelectionOr(Iterate_IsNotPartOfStructuredObject, NULL)) return 0;
	return 1;
}

// Align in line
// between the farthest away points
void	WED_DoAlign(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);

	vector<IGISPoint *> pnts;
	if (!sel->IterateSelectionAnd(collect_pnts, &pnts))
		return ;
	if(pnts.size() < 3) return;

	static CoordTranslator2 translator;
	Bbox2 bb;
	Point2 p1,p2;
	double fdist = 0;
    IGISPoint * s = pnts[0];
	IGISPoint * d = pnts[1];

	// find farthest away points and adjust bbox for translator
	for( int i = 0; i < pnts.size(); ++i)
	{
		pnts[i]->GetLocation(gis_Geo,p1);
		bb += p1;
		for( int j = i+1 ; j < pnts.size(); ++j)
		{
			pnts[j]->GetLocation(gis_Geo,p2);
			double dist = LonLatDistMeters(p1.x_,p1.y_,p2.x_,p2.y_);
			if ( dist > fdist)
			{
				fdist = dist;
				s = pnts[i];
				d = pnts[j];
			}
		}
	}
	// if bbox area = 0 then translator fails
	// anyhow , the points are already aligned vertical and horizontal
	if( bb.xspan() == 0.0 || bb.yspan() == 0.0) return;

	op->StartOperation("Align in line");
	set<WED_DrapedOrthophoto *> os;

	s->GetLocation(gis_Geo,p1);
	d->GetLocation(gis_Geo,p2);

	CreateTranslatorForBounds(bb,translator);
	Segment2 l(translator.Forward(p1),translator.Forward(p2));
	// move the other points on the line
	for(vector<IGISPoint *>::iterator it = pnts.begin(); it != pnts.end();++it)
	{
		if(*it == s || *it == d ) continue;
		Point2 ll,p;
		(*it)->GetLocation(gis_Geo,ll);
		p = l.projection(translator.Forward(ll));
		(*it)->SetLocation(gis_Geo,translator.Reverse(p));

		//collect DrapedOrtho's involved
		WED_Thing * thing = dynamic_cast<WED_Thing *>(*it);
		if(thing)
		{
			WED_Thing * parent = thing->GetParent();
			if(parent)
			{
				WED_Thing * grandparent = parent->GetParent();
				if(grandparent && (strcmp(grandparent->GetClass() , "WED_DrapedOrthophoto") == 0))
				{
					WED_DrapedOrthophoto * ortho = dynamic_cast<WED_DrapedOrthophoto *>(grandparent);
					if(ortho) os.insert(ortho);
				}
			}
		}
	}

	// redrape DrapedOthosphoto's upon modification of points
	for(set<WED_DrapedOrthophoto *>::iterator it = os.begin(); it != os.end();++it)
	{
		 (*it)->Redrape();
	}

	op->CommitOperation();
}

static int IterateCanOrthogonalize(ISelectable * what, void * ref)
{
	if(!Iterate_IsStructuredObject(what, ref)) return 0;
	IGISPolygon * pol = dynamic_cast<IGISPolygon *>(what);
	if(pol && pol->GetOuterRing()->GetNumPoints() > 3) return 1;
	IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(what);
	if(!seq ) return 0;
	int numpnts = seq->GetNumPoints();
	if (numpnts > 3) return 1;
	if(!seq->IsClosed() && numpnts > 2) return 1;
	return 0;
}

int		WED_CanOrthogonalize(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelectionAnd(IterateCanOrthogonalize, NULL)) return 1;
	return 0;
}

static void RotatePolygon(Polygon2 * pol,const Point2& ctr, double angle )
{
	for( int i = 0 ; i < pol->size() ; ++i )
	{
		Vector2 v_old = VectorLLToMeters(ctr,Vector2(ctr,pol->at(i)));
		double old_len = sqrt(v_old.squared_length());
		double old_ang = VectorMeters2NorthHeading(ctr,ctr,v_old);
		double new_ang = old_ang - angle ;
		Vector2 v_new;
		NorthHeading2VectorMeters(ctr, ctr, new_ang,v_new);
		v_new.normalize();
		v_new *= old_len;
		v_new = VectorMetersToLL(ctr,v_new);
		pol->at(i) = ctr + v_new;
	}
}

static void DoMakeOrthogonal(IGISPointSequence * seq )
{
	int numpoints = seq->GetNumPoints();
	int maxpoints = numpoints;

	bool is_closed = seq->IsClosed() ;
	if (is_closed)
	{
		if(numpoints < 4) return;
	}
	else
	{
	   if(numpoints < 3 ) return;
		maxpoints = numpoints - 1;
	}

	Polygon2 pol;
	for( int i = 0 ; i < numpoints ; ++i )
	{
		Point2 p;
		seq->GetNthPoint(i)->GetLocation(gis_Geo,p);
		pol.push_back(p);
	}

	Point2 ctr = pol.centroid();

	Segment2 seg = pol.side(0);

	double heading = VectorDegs2NorthHeading(ctr,ctr,Vector2(seg.p1,seg.p2)) - 90.;
	// rotate to realy east-west
	RotatePolygon(&pol,ctr,heading);

	int next_dir = 0;
	int last_dir = 0;
	int node_cnt = 0;
	double   sum = 0;
	bool use_first_value = false;

	for( int i = 0 ; i < maxpoints ; ++i )
	{
		int prv = i ;
		int pos = (i + 1) % pol.size() ;
		int nxt = (i + 2) % pol.size() ;

		Point2 p1 = pol.at(prv);
		Point2 p2 = pol.at(pos);
		Point2 p3 = pol.at(nxt);

		Vector2	v_prv = VectorLLToMeters(p1,Vector2(p1,p2));
		Vector2	v_nxt = VectorLLToMeters(p2,Vector2(p2,p3));

		int turn_dir = v_prv.turn_direction(v_nxt);
		v_prv.normalize();
		v_nxt.normalize();
		double cosa = v_prv.dot(v_nxt);
		bool dir_change = false;

		if(cosa < cos (45. * DEG_TO_RAD))
		{
			if(turn_dir == LEFT_TURN )
				next_dir = (last_dir + 1) % 4;
			else
				next_dir = (last_dir + 3) % 4;
			dir_change = true;
		}
		else
		{
			dir_change = false;
		}

		bool prv_is_vert = ( last_dir == 1 || last_dir == 3);
		bool nxt_is_vert = ( next_dir == 1 || next_dir == 3);

		last_dir = next_dir;

		if(use_first_value)
			sum += prv_is_vert ? pol.at(1).x_ : pol.at(1).y_ ;
		else
			sum += prv_is_vert ? pol.at(pos).x_ : pol.at(pos).y_ ;
		++node_cnt;

		if( dir_change || pos < 2 || (!is_closed && nxt == 0))
		{
			double avg;
			if( pos == 0 )
			{
				avg = prv_is_vert ? pol.at(pos).x_ : pol.at(pos).y_ ;
			}
			else
			{
				avg  = sum / node_cnt ;
			}
			for ( int k = 0 ; k < node_cnt ; ++k)
			{
				int n = pos - k ; n = n < 0 ? n + pol.size() : n;
				if(prv_is_vert) pol.at(n).x_ = avg;
				else 			pol.at(n).y_ = avg;
			}

			sum = nxt_is_vert ? pol.at(pos).x_ : pol.at(pos).y_ ;
			node_cnt = 1;
			use_first_value = (pos == 1) ? true : false;
		}
	}
	//rotate back
	RotatePolygon(&pol,ctr,-heading);

	for( int i = 0 ; i < numpoints ; ++i  )
	{
		seq->GetNthPoint(i)->SetLocation(gis_Geo,pol.at(i));
	}

	// redrape DrapedOthosphoto's upon modification of point sequence
	WED_Thing * thing = dynamic_cast <WED_Thing *> (seq);
	if(thing)
	{
		WED_Thing * parent = thing->GetParent();
		if(parent)
		{
			WED_DrapedOrthophoto * ortho = dynamic_cast <WED_DrapedOrthophoto *>(parent);
			if (ortho) ortho->Redrape();
		}
	}
}

void	WED_DoOrthogonalize(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);

	vector<WED_Thing *> things;
	sel->IterateSelectionOr(Iterate_CollectThings, &things);
	if(things.empty()) return;

	op->StartOperation("Orthogonalize");

	for(vector<WED_Thing *>::iterator it = things.begin(); it != things.end();++it)
	{
		IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(*it);
		if(seq)
		{
			DoMakeOrthogonal(seq);
			continue;
		}
		IGISPolygon * pol = dynamic_cast<IGISPolygon *>(*it);
		if(pol)
		{
			for(int i = -1; i < pol->GetNumHoles(); ++i)
			{
				seq = ( i == -1 ? pol->GetOuterRing() : pol->GetNthHole(i));
				DoMakeOrthogonal(seq);
			}
		}
	}

	op->CommitOperation();
}

static int IterateCanMakeRegularPoly(ISelectable * what, void * ref)
{
	if(!Iterate_IsStructuredObject(what, ref)) return 0;
	IGISPolygon * pol = dynamic_cast<IGISPolygon *>(what);
	if(pol && pol->GetOuterRing()->GetNumPoints() > 2) return 1;
	IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(what);
	if(seq && seq->GetNumPoints() > 2) return 1;
	return 0;
}

int		WED_CanMakeRegularPoly(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelectionAnd(IterateCanMakeRegularPoly, NULL)) return 1;
	return 0;
}

static void DoMakeRegularPoly(IGISPointSequence * seq )
{
	int n = seq->GetNumPoints();
	if(n < 3 ) return;
	
	Point2 p1,p2;
	Polygon2 pol;

	double l  = 0.0;
	for( int i = 0 ; i < n ; ++i )
	{
		seq->GetNthPoint(i)->GetLocation(gis_Geo,p1);
		seq->GetNthPoint((i+1) % n)->GetLocation(gis_Geo,p2);
		l += LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
		pol.push_back(p1);
	}
	//avg edge length
	l = l/n ;
	//TODO: mroe : cannot find a good centerpoint , take this for now
	Point2 ctr = pol.centroid();
	//centri angle
	double w = (2.0*PI) / n ;
	//outer radius
	double ru = (l/2.0) / sin(w/2.0);
	if (pol.is_ccw()) w = -w;

	//http://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
	double c = (4.0/3.0) * tan(w/4.0) * ru;

	//initial heading of first segment
	double a1 = VectorDegs2NorthHeading(ctr,ctr,Vector2(pol.at(0),pol.at(1)));

	for( int i = 0 ; i < n ; ++i)
	{	
		double h = i*w;
		Vector2	v;
		v.dx = ru*sin(h);
		v.dy = ru*cos(h);

		pol[i] = ctr + VectorMetersToLL(ctr,v);

		BezierPoint2 bp;
		IGISPoint_Bezier * bez;
		if(seq->GetNthPoint(i)->GetGISClass() == gis_Point_Bezier)
		{
			if((bez = dynamic_cast<IGISPoint_Bezier *>(seq->GetNthPoint(i))) != NULL)
			{
				v = v.perpendicular_ccw();
				v.normalize();
				v *= c;

				bez->GetBezierLocation(gis_Geo,bp);
				bp.hi = bp.has_hi() ? pol[i] - VectorMetersToLL(ctr, v) : pol[i];
				bp.lo = bp.has_lo() ? pol[i] + VectorMetersToLL(ctr, v) : pol[i];
				bp.pt = pol[i] ;

				bez->SetBezierLocation(gis_Geo,bp);
				bez->SetSplit(bp.is_split());
			}
		}
		else
		{
			seq->GetNthPoint(i)->SetLocation(gis_Geo,pol[i]);
		}
	}

	double a2 = VectorDegs2NorthHeading(ctr,ctr,Vector2(pol.at(0),pol.at(1)));
    //rotate to inital heading
	seq->Rotate(gis_Geo,ctr,a1-a2);

	// redrape DrapedOthosphoto's upon modification of point sequence
	WED_Thing * thing = dynamic_cast <WED_Thing *> (seq);
	if(thing)
	{
		WED_Thing * parent = thing->GetParent();
		if(parent)
		{
			WED_DrapedOrthophoto * ortho = dynamic_cast <WED_DrapedOrthophoto *>(parent);
			if (ortho) ortho->Redrape();
		}
	}
}

void	WED_DoMakeRegularPoly(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);

	vector<WED_Thing *> things;
	sel->IterateSelectionOr(Iterate_CollectThings, &things);
	if(things.empty()) return;

	op->StartOperation("Make Regular Poly");

	for(vector<WED_Thing *>::iterator it = things.begin(); it != things.end();++it)
	{
		IGISPointSequence * seq = dynamic_cast<IGISPointSequence *>(*it);
		if(seq)
		{
			DoMakeRegularPoly(seq);
			continue;
		}
		IGISPolygon * pol = dynamic_cast<IGISPolygon *>(*it);
		if(pol)
		{
			for(int i = -1; i < pol->GetNumHoles(); ++i)
			{
				seq = ( i == -1 ? pol->GetOuterRing() : pol->GetNthHole(i));
				DoMakeRegularPoly(seq);
			}
		}
	}

	op->CommitOperation();
}

typedef vector<pair<Point2, pair<const char *, WED_Thing *> > > merge_class_map;

//
static bool lesser_y_then_x_merge_class_map(const pair<Point2, pair<const char *, WED_Thing * > >& lhs, const pair<Point2, pair<const char *, WED_Thing * > > & rhs)
{
	return (lhs.first.y() == rhs.first.y()) ? (lhs.first.x() < rhs.first.x()) : (lhs.first.y() < rhs.first.y());
}

static bool is_within_snapping_distance(const merge_class_map::iterator& first_thing, const merge_class_map::iterator& second_thing, const CoordTranslator2& translator)
{
	const int MAX_DIST_M_SQ = 1;

	Point2 first_pos_m       = translator.Forward(first_thing->first);
	Point2 second_thing_pos_m = translator.Forward(second_thing->first);
	double a_sqr = pow((second_thing_pos_m.x() - first_pos_m.x()), 2);
	double b_sqr = pow (second_thing_pos_m.y() - first_pos_m.y(), 2);
	double sum_a_b = a_sqr + b_sqr;
	bool is_snappable =  sum_a_b < MAX_DIST_M_SQ;
	return is_snappable;
}

static const char * get_merge_tag_for_thing(IGISPoint * ething)
{
	// In order to merge, we haveto at least be a thing AND a point,
	// and have a parent that is a thing and an entity.  (If that's
	// not true, @#$ knows what is selected.)
	if(ething == NULL)
		return NULL;
	WED_Thing * thing = dynamic_cast<WED_Thing *>(ething);
	if(thing == NULL)
		return NULL;

	WED_Thing * parent = thing->GetParent();
	if(parent == NULL)
		return NULL;
	IGISEntity * eparent = dynamic_cast<IGISEntity *>(parent);
	if(eparent == NULL)
		return NULL;
	
	if(eparent->GetGISClass() == gis_Composite)
	{
		// If our parent is a composite, we are a point or vertex.
		// Merge nodes of edges, but not just raw points.  Don't let
		// the user select two windsocks and, um, "merge" them.
		if(thing->CountViewers() > 0)
			return ething->GetGISSubtype();
		else
			return NULL;
	}
	else
	{
		return NULL;
	}
}

static int iterate_can_merge(ISelectable * who, void * ref)
{
	merge_class_map * sinks = (merge_class_map *) ref;
	IGISPoint * p = dynamic_cast<IGISPoint *>(who);
	if(p == NULL)
		return 0;
	WED_Thing * t = dynamic_cast<WED_Thing *>(who);
	const char * tag = get_merge_tag_for_thing(p);
	if(tag == NULL)
		return 0;
	if(t == NULL)
		return 0;
	
	Point2	loc;
	p->GetLocation(gis_Geo, loc);

	sinks->push_back(make_pair(loc, make_pair(tag, t)));
	return 1;
}

//Returns true if every node can be merged with each other, by type and by location
int	WED_CanMerge(IResolver * resolver)
{
	//Preformance notes 1/6/2017:
	//Release build -> 1300 nodes collected
	//Completed test in
	//   0.001130 seconds.
	//   0.020325 seconds.
	//   0.000842 seconds.
	//   0.020704 seconds.
	//   0.016719 seconds.
	//StElapsedTime can_merge_timer("WED_CanMerge");
	ISelection * sel = WED_GetSelect(resolver);
	if(sel->GetSelectionCount() < 2)
		return 0;		// can't merge 1 thing!

	//1. Ensure all of the selection is mergeable, collect
	merge_class_map sinkmap;
	if (!sel->IterateSelectionAnd(iterate_can_merge, &sinkmap))
	{
		return 0;
	}

	if (sinkmap.size() > 10000 || sinkmap.size() < 2)
	{
		return 0;
	}

	//2. Sort by location, a small optimization
	sort(sinkmap.begin(), sinkmap.end(), lesser_y_then_x_merge_class_map);

	//Find the bounds for the current airport
	WED_Airport* apt = WED_GetCurrentAirport(resolver);
	Bbox2 bb;
	CoordTranslator2 translator;
	apt->GetBounds(gis_Geo, bb);
	CreateTranslatorForBounds(bb, translator);

	//Keeps track of which objects we've discovered we can snap (hopefully all)
	set<merge_class_map::iterator> can_snap_objects;

	//For each item in the sink map
	for (merge_class_map::iterator thing_1_itr = sinkmap.begin(); thing_1_itr != sinkmap.end() - 1; ++thing_1_itr)
	{
		//For each item after that
		for (merge_class_map::iterator merge_pair_itr = thing_1_itr + 1; merge_pair_itr != sinkmap.end(); ++merge_pair_itr)
		{
			//If the two things are within snapping distance of each other, record so
			if (is_within_snapping_distance(thing_1_itr, merge_pair_itr, translator))
			{
				can_snap_objects.insert(thing_1_itr);
				can_snap_objects.insert(merge_pair_itr);
			}
		}
	}

	//Ensure expected UI behavior - Only perfect merges are allowed
	if (can_snap_objects.size() == sinkmap.size())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//WED_DoMerge will merge every node in the selection possible, any nodes that could not be merged are left as is.
//Ex: 0-0-0-0---------------------0 will turn into 0-------------------------0.
//If WED_CanMerge is called first this behavior would not be possible
void WED_DoMerge(IResolver * resolver)
{
	//Preformance notes 1/6/2017:
	//Release build -> 1300 nodes collected
	//Completed snapping and merged to ~900 in
	//   0.023290 seconds
	//StElapsedTime can_merge_timer("WED_DoMerge");
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Merge Nodes");

	DebugAssert(sel->GetSelectionCount() >= 2);

	//Validate and collect from selection
	merge_class_map sinkmap;
	if(!sel->IterateSelectionAnd(iterate_can_merge, &sinkmap))
	{
		DebugAssert(!"Merge was not legal");
		op->AbortOperation();
		return;
	}

	//Ensure sinkmap is not rediculous or unmergable
	if (sinkmap.size() > 10000)
	{
		DoUserAlert("You have too many things selected to merge them, deselect some of them first");
		return;
	}
	else if (sinkmap.size() < 2)
	{
		return;
	}

	//2. Sort by location, a small optimization
	sort(sinkmap.begin(), sinkmap.end(), lesser_y_then_x_merge_class_map);

	WED_Airport* apt = WED_GetCurrentAirport(resolver);
	Bbox2 bb;
	CoordTranslator2 translator;
	apt->GetBounds(gis_Geo, bb);
	CreateTranslatorForBounds(bb, translator);

	//All the nodes that will end up snapped
	set<WED_Thing*> snapped_nodes;
	merge_class_map::iterator start_thing = sinkmap.begin();
	while (start_thing != sinkmap.end())
	{
		if(start_thing + 1 != sinkmap.end())
		{
			merge_class_map::iterator merge_pair = start_thing+1;

			while (merge_pair != sinkmap.end())
			{
				const char * tag_1 = start_thing->second.first;
				const char * tag_2 = merge_pair->second.first;

				if (is_within_snapping_distance(start_thing, merge_pair, translator) && tag_1 == tag_2)
				{
					vector<WED_Thing*> sub_list = vector<WED_Thing*>();
					sub_list.push_back(start_thing->second.second);
					sub_list.push_back(merge_pair->second.second);

					merge_pair = sinkmap.erase(merge_pair);
					snapped_nodes.insert(run_merge(sub_list));
				}
				else
				{
					++merge_pair;
				}
			}
		}

		++start_thing;
	}

	sel->Clear();
	
	for(set<WED_Thing *>::iterator node = snapped_nodes.begin(); node != snapped_nodes.end(); ++node)
	{
		set<WED_Thing *>	viewers;
		(*node)->GetAllViewers(viewers);
		for(set<WED_Thing *>::iterator v = viewers.begin(); v != viewers.end(); ++v)
		{
			if((*v)->GetNthSource(0) == (*v)->GetNthSource(1))
			{
				(*v)->RemoveSource((*node));
				(*v)->SetParent(NULL,0);
				(*v)->Delete();
			}
		}
	
		// Ben says: DO NOT delete the "unviable" isolated vertex here..if the user merged this down, maybe the user will link to it next?
		// User can clean this by hand - it is in the selection when we are done.
		sel->Insert((*node));
	}

	op->CommitOperation();
}


static int IterateNonReversable(ISelectable * what, void * ref)
{
	if (dynamic_cast<IGISPolygon*>(what)) return 0;
	if (dynamic_cast<IGISPointSequence*>(what)) return 0;
	return 1;

}

int		WED_CanReverse(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelectionOr(IterateNonReversable, NULL)) return 0;
	return 1;
}

static int IterateDoReverse(ISelectable * what, void * ref)
{
	IGISPolygon * p;
	IGISPointSequence * ps;
	if ((p =  dynamic_cast<IGISPolygon*      >(what))!= NULL) p->Reverse(gis_Geo);
	if ((ps = dynamic_cast<IGISPointSequence*>(what))!= NULL) ps->Reverse(gis_Geo);
	return 0;
}

int		WED_CanRotate(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	if (sel->GetSelectionCount() == 0) return 0;
	if (sel->IterateSelectionOr(IterateNonReversable, NULL)) return 0;
	return 1;
}

static int IterateDoRotate(ISelectable * what, void * ref)
{
	IGISPolygon * p;
	IGISPointSequence * ps;
	if ((p =  dynamic_cast<IGISPolygon*      >(what))!= NULL) p->Shuffle(gis_Geo);
	if ((ps = dynamic_cast<IGISPointSequence*>(what))!= NULL) ps->Shuffle(gis_Geo);
	return 0;
}

void	WED_DoRotate(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Rotate");
	sel->IterateSelectionOr(IterateDoRotate, NULL);
	op->CommitOperation();
	
}


void	WED_DoReverse(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Reverse");
	sel->IterateSelectionOr(IterateDoReverse, NULL);
	op->CommitOperation();
}


int		WED_CanDuplicate(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * wrl = WED_GetWorld(resolver);
	return sel->GetSelectionCount() > 0 && !sel->IsSelected(wrl);
}

void	WED_DoDuplicate(IResolver * resolver, bool wrap_in_cmd)
{
	set<ISelectable *>	sel_set;
	vector<WED_Thing *>	dupe_targs;
	ISelection *		sel;
	WED_Thing *			t;
	sel = WED_GetSelect(resolver);
	sel->GetSelectionSet(sel_set);
	WED_Thing * wrl = WED_GetWorld(resolver);
	for (set<ISelectable *>::iterator s = sel_set.begin(); s != sel_set.end(); ++s)
	if ((t = dynamic_cast<WED_Thing *>(*s)) != NULL)
	{
		if (t == wrl) continue;
		set<WED_Thing *> v;
		t->GetAllViewers(v);
		bool edge_sel = false;
		for(set<WED_Thing *>::iterator vv = v.begin(); vv != v.end(); ++vv)
		{
			if(sel->IsSelected(*vv))
			{
				edge_sel = true;
				break;
			}
		}
		
		if(!edge_sel)
		{
			bool par_sel = false;
			WED_Thing * p = t->GetParent();
			while(p)
			{
				if (sel->IsSelected(p))
				{
					par_sel = true;
					break;
				}
				p = p->GetParent();
			}
			if (!par_sel) dupe_targs.push_back(t);
		}
	}

	if (dupe_targs.empty()) return;
	if (wrap_in_cmd)		wrl->StartOperation("Duplicate");

	sel->Clear();
	
	map<WED_Thing *,WED_Thing *>	src_map;
	vector<WED_Thing *>				new_things;
	
	for (vector<WED_Thing *>::iterator i = dupe_targs.begin(); i != dupe_targs.end(); ++i)
	{
		WED_Thing * orig = *i;
		int ss = orig->CountSources();
		for(int s = 0; s < ss; ++s)
			src_map.insert(make_pair(orig->GetNthSource(s),(WED_Thing *)NULL));
		WED_Persistent * np = orig->Clone();
		t = dynamic_cast<WED_Thing *>(np);
		DebugAssert(t);
		t->SetParent(orig->GetParent(), orig->GetMyPosition());
		sel->Insert(t);
		new_things.push_back(t);
	}
	
	for(map<WED_Thing*,WED_Thing *>::iterator s = src_map.begin(); s != src_map.end(); ++s)
	{
		s->second = dynamic_cast<WED_Thing *>(s->first->Clone());
		s->second->SetParent(s->first->GetParent(),s->first->GetMyPosition());
	}
	for(vector<WED_Thing*>::iterator nt = new_things.begin(); nt != new_things.end(); ++nt)
	{
		int ss = (*nt)->CountSources();
		for(int s = 0; s < ss; ++s)
		{
			WED_Thing * orig = (*nt)->GetNthSource(s);
			if(src_map.count(orig))
				(*nt)->ReplaceSource(orig, src_map[orig]);
		}
	}

	if (wrap_in_cmd)		wrl->CommitOperation();
}

static void accum_unviable_recursive(WED_Thing * who, set<WED_Thing *>& unviable)
{
	if(WED_NoLongerViable(who, false))		// LOOSE viability for file repair - only freak out when the alternative is seg fault.
		unviable.insert(who);
	
	int nn = who->CountChildren();
	for(int n = 0; n < nn; ++n)
		accum_unviable_recursive(who->GetNthChild(n), unviable);
}

int		WED_Repair(IResolver * resolver)
{
	WED_Thing * root = WED_GetWorld(resolver);
	set<WED_Thing *> unviable;
	accum_unviable_recursive(root,unviable);
	if(unviable.empty())
		return false;
	root->StartOperation("Repair");
	WED_RecursiveDelete(unviable);
	WED_SetAnyAirport(resolver);
	root->CommitOperation();
	return 1;
}

//----------------------------------------------------------------------------
// Obj and Agp Replacement
//----------------------------------------------------------------------------

template <typename T>
static int CountChildOfTypeRecursive(WED_Thing* thing, bool must_be_visible)
{
	int num_analyzed = 0;
	return CountChildOfTypeRecursive<T>(thing, must_be_visible, 0, num_analyzed); //Needed to offset counting "thing" as a child if it matches type T
}

//Warning: Don't call this overload, call the wrapper version
template <typename T>
static int CountChildOfTypeRecursive(WED_Thing* thing, bool must_be_visible, int accumulator, int& num_analyzed)
{
	T* test_thing = dynamic_cast<T*>(thing);
	++num_analyzed;

	if(test_thing != NULL)
	{
		WED_Entity* test_ent = dynamic_cast<WED_Entity*>(thing);
		if (test_ent == NULL)
		{
			return accumulator;
		}
		else if(test_ent->GetHidden() == true && must_be_visible == true)
		{
			return accumulator;
		}
		else
		{
			if (num_analyzed > 1)
			{
				accumulator += 1;
			}
		}
	}

	int nc = thing->CountChildren();
	for(int n = 0; n < nc; ++n)
	{
		int old_accum = accumulator;
		int new_accum = CountChildOfTypeRecursive<T>(thing->GetNthChild(n), must_be_visible, accumulator, num_analyzed);

		if(new_accum != old_accum)
		{
			accumulator = new_accum;
			continue;
		}
	}

	return accumulator;
}

//template <typename OutputIterator>
/*static void CollectRecursive(WED_Thing * thing, OutputIterator oi)
{
	// TODO: do fast WED type ptr check on sClass before any other casts?
	// Factor out WED_Entity check to avoid second dynamic cast?
	WED_Entity * ent = dynamic_cast<WED_Entity*>(thing);
	if(ent && ent->GetHidden())
	{
		return;
	}
	
	typedef typename OutputIterator::container_type::value_type VT;
	VT ct = dynamic_cast<VT>(thing);
	bool took_it = false;
	if(ct)
	{	
		oi = ct;
		took_it = true;
	}
	
	if(!took_it)
	{
		int nc = thing->CountChildren();
		for(int n = 0; n < nc; ++n)
		{
			CollectRecursive(thing->GetNthChild(n), oi);
		}
	}
}*/

set<string> build_agp_list()
{
	set<string> agp_list;
	//-------------------------------------------------------------------------
	//10/06/2016 - The code for breaking apart AGPs doesn't take into account if textures should be saved
	//because these agps can have their textures thrown away. Additional AGPs will have to be analyzed
	//and the code will possibly have to be made to be more robust
	agp_list.insert("lib/airport/Ramp_Equipment/250cm_Jetway_Group.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/400cm_Jetway_2.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/400cm_Jetway_3.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/400cm_Jetway_Group.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/500cm_Jetway_Group.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/Ramp_Group_Medium.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/Ramp_Group_Narrow.agp");
	agp_list.insert("lib/airport/Ramp_Equipment/Ramp_Group_Wide.agp");
	//-------------------------------------------------------------------------
	return agp_list;
}

typedef WED_ObjPlacement WED_AgpPlacement;
int		WED_CanBreakApartSpecialAgps(IResolver* resolver)
{
	//Returns true if the selection
	//- is not empty
	//- only has .agp files (of all kinds)
	ISelection* sel = WED_GetSelect(resolver);
	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	if (!selected.empty())
	{
		for (vector<ISelectable*>::iterator itr = selected.begin(); itr != selected.end(); ++itr)
		{
			WED_AgpPlacement* agp = dynamic_cast<WED_AgpPlacement*>(*itr);
			if (agp != NULL)
			{
				string agp_resource;
				agp->GetResource(agp_resource);
				if(FILE_get_file_extension(agp_resource) != ".agp")
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

template <typename From, typename To>
static To cast(From test)
{
	return dynamic_cast<To>(test);
}

template <typename T>
static bool is_null(T test)
{
	return test == NULL;
}

void	WED_DoBreakApartSpecialAgps(IResolver* resolver)
{
	//Collect all obj_placements from the world
	WED_Thing* root = WED_GetWorld(resolver);

	ISelection * sel = WED_GetSelect(resolver);

	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);
	DebugAssert(selected.empty() == false);

	vector<WED_AgpPlacement*> agp_placements;
	std::transform(selected.begin(), selected.end(), back_inserter(agp_placements), cast<ISelectable*, WED_AgpPlacement*>);
	agp_placements.erase(std::remove_if(agp_placements.begin(), agp_placements.end(), is_null<WED_AgpPlacement*>), agp_placements.end());

	if(!agp_placements.empty())
	{
		int agp_replace_count = 0;
		int obj_replace_count = 0;

		//Set up the operation
		root->StartOperation("Break Apart Special Agps");

		sel->Clear();

		//We'll have at least one!
		WED_Airport * apt = WED_GetParentAirport(agp_placements[0]);
		if(apt == NULL)
		{
			root->AbortCommand();
			DoUserAlert("Agp(s) must be in an airport in the heirarchy");
			return;
		}

		if(CountChildOfTypeRecursive<IGISEntity>(apt, false) <= 1)
		{
			sel->Select(apt);
			root->AbortCommand();
			DoUserAlert("Airport only contains one Agp: breaking apart cannot occur. Add something else to the airport first");
			return;
		}

		//The list of agp files we've decided to be special "service truck related"
		set<string> agp_list = build_agp_list();
		
		//To access agp files
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		
		//To translate from lat/lon to meters
		CoordTranslator2 translator;
		Bbox2 box;
		apt->GetBounds(gis_Geo, box);
		CreateTranslatorForBounds(box,translator);

		//A set of all the agps that we're going to replace
		set<WED_AgpPlacement*> replaced_agps;
		set<WED_ObjPlacement*> added_objs;

		//For all agps
		for(vector<WED_AgpPlacement*>::iterator agp = agp_placements.begin(); agp != agp_placements.end(); ++agp)
		{
			//Otherwise we have big problems
			DebugAssert((*agp)->CountChildren() == 0);

			string agp_resource;
			(*agp)->GetResource(agp_resource);

			//Is the agp found in the special agp list?
			if(agp_list.find(agp_resource) != agp_list.end())
			{
				//Break it all up here
				agp_t agp_data;
				if(rmgr->GetAGP(agp_resource, agp_data))
				{
					Point2 agp_origin_geo;
					(*agp)->GetLocation(gis_Geo,agp_origin_geo);
					Point2 agp_origin_m = translator.Forward(agp_origin_geo);

					for (vector<agp_t::obj>::iterator agp_obj = agp_data.objs.begin(); agp_obj != agp_data.objs.end(); ++agp_obj)
					{
						Vector2 torotate(agp_origin_m, Point2(agp_origin_m.x() + agp_obj->x, agp_origin_m.y() + agp_obj->y));

						//Note!! WED has clockwise heading, C's cos and sin functions are ccw in radians. We reverse directions and negate again
						torotate.rotate_by_degrees((*agp)->GetHeading()*-1);
						torotate *= -1;

						Point2 new_point_m = Point2(agp_origin_m.x() - torotate.x(), agp_origin_m.y() - torotate.y());
						Point2 new_point_geo = translator.Reverse(new_point_m);

						WED_ObjPlacement* new_obj = WED_ObjPlacement::CreateTyped(root->GetArchive());
						new_obj->SetLocation(gis_Geo, new_point_geo);

						//Other data that is important to resetting up the object
						new_obj->SetDefaultMSL();
						new_obj->SetHeading(agp_obj->r + (*agp)->GetHeading());
						new_obj->SetName(agp_obj->name);
						new_obj->SetParent((*agp)->GetParent(), (*agp)->GetMyPosition());
						new_obj->SetResource(agp_obj->name);
						new_obj->SetShowLevel((*agp)->GetShowLevel());

						added_objs.insert(new_obj);
					}
				}

				replaced_agps.insert(*agp);
			}
		}

		for (set<WED_AgpPlacement*>::iterator itr_agp = replaced_agps.begin(); itr_agp != replaced_agps.end(); ++itr_agp)
		{
			(*itr_agp)->SetParent(NULL, 0);
			(*itr_agp)->Delete();
		}

		for (set<WED_ObjPlacement*>::iterator itr_obj = added_objs.begin(); itr_obj != added_objs.end(); ++itr_obj)
		{
			string obj_resource;
			(*itr_obj)->GetResource(obj_resource);
			sel->Insert(*itr_obj);
		}

		if(added_objs.size() == 0)
		{
			sel->Clear();
			root->AbortOperation();
			DoUserAlert("Nothing to replace"); //IMPORTANT: Do not call DoUserAlert during an operation!!!
		}
		else
		{
			root->CommitOperation();

			stringstream ss;
			ss << "Replaced " << replaced_agps.size() << " Agp objects with " << added_objs.size() << " Obj files";
			DoUserAlert(ss.str().c_str());
		}
	}
	else
	{
		DoUserAlert("There are no relavent special Agps to break apart");
	}
}

int	WED_CanReplaceVehicleObj(IResolver* resolver)
{
	//Returns true if there are any Obj files in the world.
	//TODO: This Aught to be the current airport
	WED_Thing * root = WED_GetWorld(resolver);
	return CountChildOfTypeRecursive<WED_ObjPlacement>(root,true);
}

struct vehicle_replacement_info
{
	vehicle_replacement_info(/*const vector<string>& resource_strs,*/const int service_truck_type, const int number_of_cars)
		:/*resource_strs(resource_strs),*/
		 service_truck_type(service_truck_type),
		 number_of_cars(number_of_cars)
	{
	}

	//The resource strings that can represent this vehicle_replacement_info
	//vector<string> resource_strs;

	//A member of ATCServiceTruckType
	int service_truck_type;

	//The number of cars in the model
	int number_of_cars;
};

static map<string,vehicle_replacement_info> build_replacement_table()
{
	map<string,vehicle_replacement_info> table;

	//atc_ServiceTruck_Baggage_Loader
	table.insert(make_pair("lib/airport/Ramp_Equipment/Belt_Loader.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Loader, 0)));

	table.insert(make_pair("lib/airport/vehicles/baggage_handling/belt_loader.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Loader, 0)));

	//atc_ServiceTruck_Baggage_Train
	stringstream ss;
	for(int i = 1; i <= 5; ++i)
	{
 		ss << "lib/airport/Ramp_Equipment/Lugg_Train_Straight" << i << ".obj";
		table.insert(make_pair(ss.str(), vehicle_replacement_info(atc_ServiceTruck_Baggage_Train,i)));
		ss.clear();
		ss.str("");
	}

	table.insert(make_pair("lib/airport/Ramp_Equipment/Luggage_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Train,0)));

	//atc_ServiceTruck_Crew_Car
	table.insert(make_pair("lib/airport/vehicles/servicing/crew_car.obj", vehicle_replacement_info(atc_ServiceTruck_Crew_Car,0)));

	//atc_ServiceTruck_Crew_Ferrari
	table.insert(make_pair("lib/airport/vehicles/servicing/crew_ferrari.obj", vehicle_replacement_info(atc_ServiceTruck_Crew_Ferrari, 0)));

	//atc_ServiceTruck_Crew_Limo
	//TODO: Waiting for art asset

	//atc_ServiceTruck_Food
	table.insert(make_pair("lib/airport/vehicles/servicing/catering_truck.obj", vehicle_replacement_info(atc_ServiceTruck_Food,0)));

	//atc_ServiceTruck_FuelTruck_Liner
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/hyd_disp_truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Liner, 0)));

	//!!Important!! - Large and Small are reversed on purpose!

	//atc_ServiceTruck_FuelTruck_Jet
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/Small_Fuel_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Jet,0)));

	//atc_ServiceTruck_FuelTruck_Prop
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/Large_Fuel_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Prop,0)));

	//atc_ServiceTruck_Ground_Power_Unit
	table.insert(make_pair("lib/airport/vehicles/baggage_handling/tractor.obj", vehicle_replacement_info(atc_ServiceTruck_Ground_Power_Unit,0)));
	table.insert(make_pair("ib/airport/Ramp_Equipment/GPU_1.obj", vehicle_replacement_info(atc_ServiceTruck_Ground_Power_Unit,0)));

	//atc_ServiceTruck_Pushback
	table.insert(make_pair("lib/airport/Ramp_Equipment/Tow_Tractor_1.obj", vehicle_replacement_info(atc_ServiceTruck_Pushback,0)));
	table.insert(make_pair("lib/airport/Ramp_Equipment/Tow_Tractor_2.obj", vehicle_replacement_info(atc_ServiceTruck_Pushback,0)));

	return table;
}

void	WED_DoReplaceVehicleObj(IResolver* resolver)
{
	WED_Thing * root = WED_GetWorld(resolver);
	vector<WED_ObjPlacement*> obj_placements;
	//CollectRecursive(root, WED_ObjPlacement::sClass, back_inserter(obj_placements));

	if(!obj_placements.empty())
	{
		int replace_count = 0;
		root->StartOperation("Replace Objects");
		map<string,vehicle_replacement_info> table = build_replacement_table();
		
		ISelection * sel = WED_GetSelect(resolver);
		sel->Clear();

		for(vector<WED_ObjPlacement*>::iterator itr = obj_placements.begin(); itr != obj_placements.end(); ++itr)
		{
			string resource;
			(*itr)->GetResource(resource);
			
			map<string,vehicle_replacement_info>::iterator info_itr = table.find(resource);
			if(info_itr != table.end())
			{
				
				WED_TruckParkingLocation * parking_loc = WED_TruckParkingLocation::CreateTyped(root->GetArchive());
				parking_loc->SetHeading((*itr)->GetHeading());
				
				Point2 p;
				(*itr)->GetLocation(gis_Geo, p);
				parking_loc->SetLocation(gis_Geo,p);

				string name;
				(*itr)->GetName(name);
				parking_loc->SetName(name);
				parking_loc->SetNumberOfCars(info_itr->second.number_of_cars);
				parking_loc->SetParent((*itr)->GetParent(), (*itr)->GetMyPosition());
				parking_loc->SetTruckType(info_itr->second.service_truck_type);
				parking_loc->StateChanged();

				replace_count++;
				(*itr)->SetParent(NULL, 0);
				(*itr)->Delete();

				sel->Insert(parking_loc);
			}
		}

		if(replace_count == 0)
		{
			sel->Clear();
			root->AbortOperation();
			DoUserAlert("Nothing to replace");
		}
		else
		{
			root->CommitOperation();

			stringstream ss;
			ss << "Replaced " << replace_count << " objects";
			DoUserAlert(ss.str().c_str());
		}
	}
	else
	{
		DoUserAlert("Nothing to replace");
	}
}
//-----------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
#pragma mark -
//---------------------------------------------------------------------------------------------------

struct obj_conflict_info {
	WED_ObjPlacement *		obj;
	double					approx_radius_m;
	Point2					loc_ll;
};

static void center_and_radius_for_ramp_start(WED_RampPosition * pos, Point2& out_ctr, double& out_rad)
{
	double mtr = 15;
	float offset = 5;
	Point2 nose_wheel;
	int icao_width = pos->GetWidth();
	
	pos->GetLocation(gis_Geo, nose_wheel);
	switch(icao_width) {
	case width_A:	mtr = 15.0;	offset = 1.85f; break;
	case width_B:	mtr = 27.0;	offset = 2.75f; break;
	case width_C:	mtr = 41.0;	offset = 4.70f; break;
	case width_D:	mtr = 56.0;	offset = 9.50f; break;
	case width_E:	mtr = 72.0;	offset = 8.20f; break;
	case width_F:	mtr = 80.0;	offset = 8.80f; break;
	}

	double slip = (mtr * 0.5) - offset;

	Point2 e[2];
	
	Quad_1to2(nose_wheel, pos->GetHeading(), slip*2.0, e);
	
	out_ctr = e[0];
	out_rad = mtr * 0.5;
}

static void collect_ramps_recursive(WED_Thing * who, vector<WED_RampPosition *>& out_ramps, vector<obj_conflict_info>& out_conflicting_objs, WED_ResourceMgr * rmgr)
{
	if(who->GetClass() == WED_RampPosition::sClass)
	{
		WED_RampPosition * ramp = dynamic_cast<WED_RampPosition *>(who);		
		DebugAssert(ramp);
		out_ramps.push_back(ramp);
	} 
	if(who->GetClass() == WED_ObjPlacement::sClass)
	{
		WED_ObjPlacement * obj = dynamic_cast<WED_ObjPlacement *>(who);
		DebugAssert(obj);		
		obj_conflict_info r;
		string vpath;
		obj->GetResource(vpath);
		XObj8 * obj8;
		
		if(strstr(vpath.c_str(), "lib/airport/aircraft/") != NULL)
		if(rmgr->GetObj(vpath, obj8))
		{
			float b[3] = {
				obj8->xyz_max[0] - obj8->xyz_min[0],
				obj8->xyz_max[1] - obj8->xyz_min[1],
				obj8->xyz_max[2] - obj8->xyz_min[2] };

			Vector2 arm(
						(obj8->xyz_min[0] + obj8->xyz_max[0]) * 0.5,
						(obj8->xyz_min[2] + obj8->xyz_max[2]) *-0.5);
			double c = cos(obj->GetHeading() * DEG_TO_RAD);
			double s = sin(obj->GetHeading() * DEG_TO_RAD);
			
			Vector2 arm_wrl(arm.dx * c + arm.dy * s,
						    arm.dy * c - arm.dx * s);
			
			r.approx_radius_m = pythag(b[0], b[1], b[2]) * 0.5f;
			
			r.obj = obj;
			obj->GetLocation(gis_Geo, r.loc_ll);
			Vector2 arm_ll = VectorMetersToLL(r.loc_ll, arm_wrl);
			r.loc_ll += arm_ll;
					
				
			
			out_conflicting_objs.push_back(r);
		}
	}
	
	int nn = who->CountChildren();
	for(int n = 0; n < nn; ++n)
		collect_ramps_recursive(who->GetNthChild(n), out_ramps, out_conflicting_objs, rmgr);
}

static int wed_upgrade_airports_recursive(WED_Thing * who, WED_ResourceMgr * rmgr, ISelection * sel)
{
	int did_work = 0;
	if(who->GetClass() == WED_Airport::sClass)
	{
		vector<WED_RampPosition *> ramps;
		vector<obj_conflict_info> objs;
		collect_ramps_recursive(who, ramps,objs,rmgr);
		
		for(vector<WED_RampPosition *>::iterator r = ramps.begin(); r != ramps.end(); ++r)
		{
			int rt = (*r)->GetType();
			if(rt == atc_Ramp_Gate)
			{
				(*r)->SetRampOperationType(ramp_operation_Airline);
				did_work = 1;
			}
			if(rt == atc_Ramp_TieDown)
			{
				did_work = 1;
				
				set<int> eq;
				(*r)->GetEquipment(eq);
				
				if(eq.count(atc_Heavies))
					(*r)->SetRampOperationType(ramp_operation_Airline);
				else
					(*r)->SetRampOperationType(ramp_operation_GeneralAviation);
			}
		}		
		
		for(vector<obj_conflict_info>::iterator o = objs.begin(); o != objs.end(); ++o)
		{
			bool alive = true;
			for(vector<WED_RampPosition *>::iterator r = ramps.begin(); r != ramps.end(); ++r)
			{				

				Point2 rp; double rs;
				center_and_radius_for_ramp_start(*r, rp, rs);

				double d = LonLatDistMeters(rp.x(), rp.y(), o->loc_ll.x(), o->loc_ll.y());
				
				if(d < (o->approx_radius_m + rs))
				{
					//debug_mesh_line(rp, o->loc_ll, 1,0,0,1,0,0);
					alive = false;
					break;
				}
			}
			if(!alive)
			{	
				sel->Erase(o->obj);
				o->obj->SetParent(NULL, 0);
				o->obj->Delete();
				did_work = 1;				
			}
			
		}
	}
	int nn = who->CountChildren();
	for(int n = 0; n < nn; ++n)	
		if(wed_upgrade_airports_recursive(who->GetNthChild(n), rmgr, sel))
			did_work = 1;
	return did_work;
	
}

#include "WED_Runway.h"

void WED_UpgradeRampStarts(IResolver * resolver)
{
	WED_Thing * root = WED_GetWorld(resolver);
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	root->StartCommand("Upgrade Ramp Positions");
	int did_work = wed_upgrade_airports_recursive(root, rmgr, sel);
	if(did_work)
		root->CommitOperation();
	else
		root->AbortOperation();
	
}

struct changelist_t
{
	string ICAO;
	int new_rwy;
	Point2 rwy_pt0;
	Point2 rwy_pt1;
};

static bool IsRwyMatching(WED_Runway * rwy, struct changelist_t * entry)
{
	// very crude match criteria: 
	// threshold, expanded by one runway width to the sides,
	// expanded by 5% of runway's length 

    Point2 rwy_corner[4];
	Vector2 dw, dl;
	
	rwy->GetCorners(gis_Geo, rwy_corner);
	dw = Vector2(rwy_corner[2],rwy_corner[1]);
	dw *= 0.5;
	dl = Vector2(rwy_corner[1],rwy_corner[0]);
	dl *= 0.05;
	
    Point2 thr_corner[4];
	
	rwy->GetCornersDisp1(thr_corner);
	
	thr_corner[0]+=dw; thr_corner[1]+=dw;
	thr_corner[3]-=dw; thr_corner[2]-=dw;
	
	thr_corner[0]+=dl; thr_corner[1]-=dl;
	thr_corner[3]+=dl; thr_corner[2]-=dl;
	
	Polygon2 end0;
	for (int i=0; i<4; ++i)
		end0.push_back(thr_corner[i]);

	rwy->GetCornersDisp2(thr_corner);
	
	thr_corner[1]+=dw; thr_corner[0]+=dw;
	thr_corner[2]-=dw; thr_corner[3]-=dw;
	
	thr_corner[1]-=dl; thr_corner[0]+=dl;
	thr_corner[2]-=dl; thr_corner[3]+=dl;

	Polygon2 end1;
	for (int i=0; i<4; ++i)
		end1.push_back(thr_corner[i]);
	
	if(end0.inside(entry->rwy_pt0) &&
	   end1.inside(entry->rwy_pt1))
	{
		printf(" Yup\n");
		return true;
	}
	else if(end0.inside(entry->rwy_pt1) &&
	   end1.inside(entry->rwy_pt0))
	{
		printf(" Rev\n");
		return true;
	}
	else
	{
		printf(" Nope\n");
		return false;
	}
}

static int rename_rwys_recursive(WED_Thing * who, vector<struct changelist_t> clist)
{
	int renamed = 0;
	WED_Airport * apt = dynamic_cast<WED_Airport *> (who);
	if(apt)
	{
		vector<WED_Runway *> rwys;
		CollectRecursive(apt, back_inserter(rwys),  WED_Runway::sClass);
		
		for(vector<WED_Runway *>::iterator r = rwys.begin(); r != rwys.end(); ++r)
		{
			for(vector<struct changelist_t>::iterator c = clist.begin(); c != clist.end(); ++c)
			{
				string s; apt->GetICAO(s);
				if((*c).ICAO == s) 
				{
					int old_rwy_enum = (*r)->GetRunwayEnumsTwoway();
					printf("Testing O=%s against N=%s at %s ... ",ENUM_Desc(old_rwy_enum), ENUM_Desc((*c).new_rwy),s.c_str());
					if (IsRwyMatching(*r,&(*c)))
					{
						printf("Renaming %s Rwy %s to %s\n",s.c_str(),ENUM_Desc(old_rwy_enum),ENUM_Desc((*c).new_rwy));
						(*r)->SetName(ENUM_Desc((*c).new_rwy));
						renamed++;
					}
				}
			}
			
		}
	}
	else            // no need to check for nested airports
	{
		int nn = who->CountChildren();
		for(int n = 0; n < nn; ++n)
			renamed += rename_rwys_recursive(who->GetNthChild(n), clist);
	}
	return renamed;
}

void WED_RenameRunwayNames(IResolver * resolver)
{

	WED_Thing * root = WED_GetWorld(resolver);
	vector<struct changelist_t> changelist;

	char fn[200];
	if (!GetFilePathFromUser(getFile_Open,"Pick file with 'ICAO & Runway entries", "Open",
								FILE_DIALOG_PICK_VALID_RUNWAY_TABLE, fn,sizeof(fn)))
		return;

	FILE* file = fopen(fn,"r");
	if (file)
	{
		bool second_end = false;
		string first_rwy;
		char icao[16], rnam[16];
		float lon,lat,hdg;
		struct changelist_t entry;
		
		int lnum=0;
		while (fscanf(file,"%s%s%f%f%f", icao, rnam, &lat, &lon, &hdg) == 5)
		{
			if (second_end)
			{
				if (entry.ICAO == string(icao))
				{
					string rwy_2wy = first_rwy + "/" + rnam;
					entry.new_rwy = ENUM_LookupDesc(ATCRunwayTwoway,rwy_2wy.c_str());
					if (entry.new_rwy == -1) 
					{
						rwy_2wy = "0" + rwy_2wy;
						entry.new_rwy = ENUM_LookupDesc(ATCRunwayTwoway,rwy_2wy.c_str());
					}
					entry.rwy_pt1 = Point2(lon,lat);
					
					if(entry.new_rwy > atc_rwy_None)
						changelist.push_back(entry);
					else
						printf("Ignoring illegal runway specification pair ending in line %d\n",lnum);
				}
				else
					printf("Ignoring ICAO for not matching preceeding one in line %d\n",lnum);
				second_end = false;
			}
			else
			{
				entry.ICAO = icao;
				first_rwy = rnam;
				entry.rwy_pt0 = Point2(lon,lat);
				second_end = true;
			}
			lnum++;
		}
		fclose(file);
	}
	else
	{
		printf("Can't read file\n");
		return;
	}
	 
	if (!changelist.empty())
	{
		root->StartCommand("Rename Runways");
		int renamed_count = rename_rwys_recursive(root, changelist);
		if(renamed_count)
		{
			stringstream ss;
			ss << "Renamed " << renamed_count << " runways";
			DoUserAlert(ss.str().c_str());
			root->CommitOperation();
		}
		else
			root->AbortOperation();
	}
}
