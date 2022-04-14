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

#include "ISelection.h"
#include "ILibrarian.h"

#include "AssertUtils.h"
#include "BitmapUtils.h"
#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"
#include "FileUtils.h"
#include "MathUtils.h"
#include "MemFileUtils.h"
#include "PlatformUtils.h"

#include "AptDefs.h"
#include "XObjDefs.h"
#include "XESConstants.h"
#include "XObjDefs.h"

#include "WED_AirportBoundary.h"
#include "WED_AirportChain.h"
#include "WED_AirportSign.h"
#include "WED_Airport.h"
#include "WED_ATCFrequency.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCTimeRule.h"
#include "WED_ATCWindRule.h"
#include "WED_AirportNode.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_FacadePlacement.h"
#include "WED_FacadeRing.h"
#include "WED_FacadeNode.h"
#include "WED_Group.h"
#include "WED_LinePlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_Orthophoto.h"
#include "WED_OverlayImage.h"
#include "WED_PolygonPlacement.h"
#include "WED_RampPosition.h"
#include "WED_Ring.h"
#include "WED_RoadEdge.h"
#include "WED_RoadNode.h"
#include "WED_Runway.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_TextureNode.h"
#include "WED_TaxiRouteNode.h"
#include "WED_Taxiway.h"
#include "WED_TruckParkingLocation.h"
#include "WED_Windsock.h"

#include "WED_EnumSystem.h"
#include "WED_Globals.h"
#include "WED_GISUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_LibraryMgr.h"
#include "WED_Menus.h"
#include "WED_MetaDataKeys.h"
#include "WED_MapZoomerNew.h"
#include "WED_MarqueeTool.h"
#include "WED_ResourceMgr.h"
#include "WED_Sign_Editor.h"
#include "WED_ToolUtils.h"
#include "WED_UIDefs.h"

#include <sstream>

#define DOUBLE_PT_DIST (1.0 * MTR_TO_DEG_LAT)

#define DEBUG_EDGE_CROSSING 0

namespace std
{
	template <> struct less<Point2>
	{
		bool operator()(const Point2 & lhs, const Point2 & rhs) const
		{
			if (lhs.x() != rhs.x())
				return lhs.x() < rhs.x();
			else
				return lhs.y() < rhs.y();
		}
	};
}

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

void	WED_DoMakeNewATCFreq(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_Airport::sClass);
	now_sel->StartOperation("Add ATC Frequency");
	WED_ATCFrequency * f=  WED_ATCFrequency::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	now_sel->CommitOperation();
}

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
	struct AptFlow_t info;                                    // pre-fill METAR ICAO
	dynamic_cast<WED_ATCFlow *>(now_sel)->Export(info);
	f->SetICAO(info.icao);
	now_sel->CommitOperation();
}

void	WED_DoMakeNewATCTimeRule(IResolver * inResolver)
{
	WED_Thing * now_sel = WED_HasSingleSelectionOfType(inResolver, WED_ATCFlow::sClass);
	now_sel->StartOperation("Add ATC Time Rule");
	WED_ATCTimeRule * f=  WED_ATCTimeRule::CreateTyped(now_sel->GetArchive());
	f->SetParent(now_sel,now_sel->CountChildren());
	now_sel->CommitOperation();
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

static bool WED_NoLongerViable(WED_Thing * t, bool strict)
{
	if(t->CountChildren() < 4)  // avoid the dynamic_casting and facade loading if its obviously obsolete
	{
		if(dynamic_cast<IGISPointSequence *>(t))
		{
			int min_children = 2;
			WED_Thing * parent = t->GetParent();
			if (parent && parent->GetClass() == WED_OverlayImage::sClass)
				min_children = 4;
			else if (t->GetClass() == WED_FacadeRing::sClass)                           // avoid having to load the facade as much as possible
			{
				min_children = 3;
				if (t->CountChildren() < 3 && !static_cast<WED_FacadeRing*>(t)->IsClosed())
					min_children = 2;
			}
			else if (parent && strict && dynamic_cast<WED_GISPolygon *>(parent))		// Strict rules for delete key require 3 points to a polygon - prevents degenerate holes.
				min_children = 3;
			if (t->CountSources() == 2 && t->GetNthSource(0) == NULL) return true;
			if (t->CountSources() == 2 && t->GetNthSource(1) == NULL) return true;

			if ((t->CountChildren() + t->CountSources()) < min_children)
				return true;
		}
	}

	if(t->GetClass() == WED_TaxiRouteNode::sClass &&
		SAFE_CAST(IGISComposite,t->GetParent()) &&
		t->CountViewers() == 0)
		return true;
#if ROAD_EDITING
	if (t->GetClass() == WED_RoadNode::sClass &&
		/*SAFE_CAST(IGISComposite,t->GetParent()) &&*/		// orphaned road nodes have no parent
		t->CountViewers() == 0)
		return true;
#endif
	if (t->CountChildren() == 0 &&
		dynamic_cast<IGISPolygon *>(t))
		return true;

	return false;
}

// For every object in 'who', adds all of its descendents to 'who'.
void WED_AddChildrenRecursive(set<WED_Thing *>& who)
{
	// Make a copy of the roots of the search, as we don't want to modify 'who' while we're
	// iterating over it.
	vector<WED_Thing*> roots(who.begin(), who.end());

	for (size_t i = 0; i < roots.size(); ++i)
		CollectRecursive(roots[i], inserter(who, who.end()), IgnoreVisiblity, TakeAlways);
}

// Deletes everything in 'who', along with any parents, sources and viewers that the deletion
// makes unviable.
// Requirement: For every object in 'who', all of its children must also be be in 'who'.
void WED_RecursiveDelete(set<WED_Thing *>& who)
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
	set<WED_Thing *>	shape_points;

	for (auto i : who)
	{
		auto n = dynamic_cast<WED_AirportNode *>(i);
		if(n && n->CountViewers() == 2)
			common_nodes.insert(n);

		auto r = dynamic_cast<WED_RoadNode *>(i);
		if(r)
		{
			set<WED_Thing *> viewers;
			r->GetAllViewers(viewers);
			for(auto v : viewers)
			{
				auto e = dynamic_cast<WED_GISEdge *>(v);
				if(e && e->GetNumSides() > 1)
				{
					DebugAssert(e->GetNthSource(0) == i || e->GetNthSource(1) == i);

					WED_RoadNode * rn = dynamic_cast<WED_RoadNode *>(r->Clone());
					rn->SetParent(e->GetParent(),e->GetMyPosition()+1);

					Bezier2 b;
					if(e->GetNthSource(0) == i)
					{
						e->GetSide(gis_Geo, 1, b);
						e->ReplaceSource(r,rn);
						e->SetSideBezier(gis_Geo, b, 0);
						shape_points.insert(e->GetNthChild(0));
					}
					else
					{
						e->GetSide(gis_Geo, e->GetNumSides()-2, b);
						e->ReplaceSource(r,rn);
						e->SetSideBezier(gis_Geo, b, e->GetNumSides()-1);
						shape_points.insert(e->GetNthChild(e->CountChildren()-1));
					}
				}
			}
		}
	}
	for(auto s : shape_points)
		who.insert(s);

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
	if (!dest) return 0; // on empty archives, the initial selection is a NULL ptr

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

	// No nested flows either...
	if (Iterate_IsOrParentClass(dest, (void*) WED_ATCFlow::sClass))
	{
		if (sel->IterateSelectionOr(Iterate_IsOrChildClass, (void *) WED_ATCFlow::sClass)) return 0;
	}

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

void WED_select_zero_recursive(WED_Thing * t, set<WED_GISEdge *> *s)
{
	WED_GISEdge * e = dynamic_cast<WED_GISEdge *>(t);
	if(e)
	if(e->GetNthPoint(0) == e->GetNthPoint(1))
		s->insert(e);
	int nn = t->CountChildren();
	for(int n = 0; n < nn; ++n)
		WED_select_zero_recursive(t->GetNthChild(n), s);
}

bool WED_DoSelectZeroLength(IResolver * resolver, WED_Thing * sub_tree)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Select Zero-Length Edges");
	sel->Clear();

	set<WED_GISEdge *> edges;
	WED_select_zero_recursive(sub_tree ? sub_tree : WED_GetWorld(resolver), &edges);

	sel->Insert(set<ISelectable*>(edges.begin(), edges.end()));

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

set<WED_Thing *> WED_select_doubles(WED_Thing * t)
{
	vector<WED_Thing *> pts;
/*	CollectRecursive(t, back_inserter(pts), ThingNotHidden, IsGraphNode);

	We can not trust the ThingNotHidden - as it stops looking into levels that are hidden.
	But even a node inside a hidden hierachy could still be used by a TaxiRoute Edge
	outside that hierachy that is NOT hidden.
	On the other hand, we do not want to check nodes that are only connected to hidden edges,
	as those do not matter. So go check which nodes are actually in use. */
	{
		vector<WED_GISEdge *> edges;
		CollectRecursive(t, back_inserter(edges), ThingNotHidden, IsGraphEdge);

		set<WED_Thing *> nodes;
		for(vector<WED_GISEdge *>::iterator e = edges.begin(); e != edges.end(); ++e)
		{
			DebugAssert(*e);
			nodes.insert( (*e)->GetNthSource(0) );
			nodes.insert( (*e)->GetNthSource(1) );
		}
		for(set<WED_Thing *>::iterator s = nodes.begin(); s != nodes.end(); ++s)
			pts.push_back(*s);
	}

	set<WED_Thing *> doubles;

	// Ben says: yes this totally sucks - replace it someday?
	for(int i = 0; i < pts.size(); ++i)
	{
		Point2 p1, p2;
		IGISPoint * ii = dynamic_cast<IGISPoint *>(pts[i]);
		ii->GetLocation(gis_Geo, p1);

		for(int j = i + 1; j < pts.size(); ++j)
		{
			IGISPoint * jj = dynamic_cast<IGISPoint *>(pts[j]);
			jj->GetLocation(gis_Geo, p2);
			DebugAssert(ii != jj);
			DebugAssert(ii);
			DebugAssert(jj);

//			if(!(ii->GetGISSubtype() == jj->GetGISSubtype())) continue;

//			Point2 p1, p2;
//			ii->GetLocation(gis_Geo, p1);
//			jj->GetLocation(gis_Geo, p2);

			if(p1.squared_distance(p2) < (DOUBLE_PT_DIST*DOUBLE_PT_DIST))
			{
				doubles.insert(pts[i]);
				doubles.insert(pts[j]);
				break;
			}
		}
	}
	return doubles;
}

bool WED_DoSelectDoubles(IResolver * resolver, WED_Thing * sub_tree)
{
	ISelection * sel = WED_GetSelect(resolver);
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Select Double Nodes");

	set<WED_Thing*> things = WED_select_doubles(sub_tree == NULL ? WED_GetWorld(resolver) : sub_tree);

	sel->Clear();
	sel->Insert(set<ISelectable*>(things.begin(), things.end()));

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

set<WED_GISEdge *> WED_do_select_crossing(WED_Thing * t)
{
	vector<WED_GISEdge *> edges;
	CollectRecursive(t, back_inserter(edges), ThingNotHidden, IsGraphEdge);
	Bbox2 emptybox(0,0,0,0);
	return WED_do_select_crossing(edges,emptybox);
}

set<WED_GISEdge *> WED_do_select_crossing(const vector<WED_GISEdge *>& edges , Bbox2& cull_bounds)
{
	#if DEV && DEBUG_EDGE_CROSSING
	printf("select crossing on %ld edges\n",edges.size());
	#endif
	set<WED_GISEdge*> crossed_edges;
	Bbox2 edge_bounds;
	// Ben says: yes this totally sucks - replace it someday?
	for (int i = 0; i < edges.size(); ++i)
	{
		Bezier2 b1, b2;
		IGISEdge * ii = edges[i];
		DebugAssert(ii);
		ii->GetBounds(gis_Geo,edge_bounds);
		if(!cull_bounds.is_empty() && !cull_bounds.overlap(edge_bounds))
		{
			#if DEV && DEBUG_EDGE_CROSSING
			printf("edge %d outside cull_bounds\n",i);
			#endif
			continue;
		}

		bool isb1 = ii->GetSide(gis_Geo, 0, b1);

		for (int j = i + 1; j < edges.size(); ++j)
		{
			IGISEdge * jj = edges[j];
			DebugAssert(jj);
			DebugAssert(ii != jj);

			jj->GetBounds(gis_Geo,edge_bounds);
			if(!cull_bounds.is_empty() && !cull_bounds.overlap(edge_bounds))
			{
				#if DEV && DEBUG_EDGE_CROSSING
				printf("edges %d %d bounds dont overlap\n",i,j);
				#endif
				continue;
			}

			if(ii->GetGISSubtype() != jj->GetGISSubtype()) continue;
			#if DEV && DEBUG_EDGE_CROSSING
			printf("edges %d %d bounds do overlap !!\n",i,j);
			#endif
			for(int si = 0; si < ii->GetNumSides(); si++)
				for(int sj = 0; sj < jj->GetNumSides(); sj++)
				{
					Bezier2 b1, b2;

					bool isb1 = ii->GetSide(gis_Geo, si, b1);
					bool isb2 = jj->GetSide(gis_Geo, sj, b2);

					if (isb1 || isb2)
					{
						if (b1.intersect(b2, 10))
						{
							crossed_edges.insert(edges[i]);
							crossed_edges.insert(edges[j]);
						}
					}
					else
					{
						Point2 x;
						if (b1.p1 != b2.p1 &&
							b1.p2 != b2.p2 &&
							b1.p1 != b2.p2 &&
							b1.p2 != b2.p1)
						{
							if (b1.as_segment().intersect(b2.as_segment(), x))
							{
								crossed_edges.insert(edges[i]);
								crossed_edges.insert(edges[j]);
							}
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

	set<WED_GISEdge *> crossed_edges = WED_do_select_crossing(sub_tree == NULL ? WED_GetWorld(resolver) : sub_tree);

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
	IHasResource * has_resource_thing = dynamic_cast<IHasResource*>(thing);
	if (has_resource_thing != NULL)
	{
		has_resource_thing->GetResource(r);
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

	if(dynamic_cast<IGISEdge *>(base)) return t->CountChildren() != 0; // multi-segmented edges can't be split from the menu - down-click on a segment instead

//	WED_AirportNode * a = dynamic_cast<WED_AirportNode *>(base);
//	if (!a) return 1;

	WED_Thing * parent = t->GetParent();
	if (!parent) return 1;

	IGISPointSequence * s = dynamic_cast<IGISPointSequence*>(parent);
	if (!s) return 1;

	if(s->GetGISClass() == gis_Edge) return 0;

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

namespace
{
struct chain_split_info_t {
	WED_GISChain * c;
	WED_GISEdge *  e;
	WED_GISPoint * p;
};

struct ring_split_info_t {
	WED_GISChain * chain;
	WED_GISPoint * p0;
	WED_GISPoint * p1;
	int pos_0;
	int pos_1;
	bool cut_to_hole;
	WED_GISChain* hole;
};
}

static bool is_chain_split(ISelection * sel, chain_split_info_t * info)
{
	// Must have exactly one point selected
	if(sel->GetSelectionCount() != 1)
		return false;
	WED_GISPoint * p = dynamic_cast<WED_GISPoint*>(sel->GetNthSelection(0));
	if(!p) return false;

	// The point must have a WED_GISChain parent
	WED_GISChain * c = dynamic_cast<WED_GISChain *>(p->GetParent());
	WED_GISEdge * e = dynamic_cast<WED_GISEdge *>(p->GetParent());
	if(!c && !e) return false;

	if(c)
	if(c->IsClosed())
	{
		// If the chain is closed, it must be a WED_AirportChain, and its parent must not be a WED_GISPolygon.
		if (!dynamic_cast<WED_AirportChain *>(c) || dynamic_cast<WED_GISPolygon *>(c->GetParent()))
			return false;
	}
	else
	{
		// If the chain is open, the point must not be the first or last point in the chain.
		int pos = p->GetMyPosition();
		if (pos == 0 || pos == c->CountChildren()-1)
			return false;
	}

	if(e)
	{
			// If its an edge, the point must not be one of the sources
			WED_Thing * t = p;
			if(e->GetNthSource(0) == t || e->GetNthSource(1) ==  t)
				return false;
	}
	if(info)
	{
		info->c = c;
		info->e = e;
		info->p = p;
	}

	return true;
}

static bool is_ring_split(ISelection * sel, ring_split_info_t * info)
{
	if (sel->GetSelectionCount() != 2) 
		return false;

	int pos_0 = 0, pos_1 = 0;

	auto p0 = dynamic_cast<WED_GISPoint*>(sel->GetNthSelection(0));
	auto p1 = dynamic_cast<WED_GISPoint*>(sel->GetNthSelection(1));
	if(!p0 || !p1)
		return false;

	auto c0 = dynamic_cast<WED_GISChain*>(p0->GetParent());
	auto c1 = dynamic_cast<WED_GISChain*>(p1->GetParent());
	if(!c0 || !c1)
		return false;

	if (!c0->IsClosed() || !c1->IsClosed())
		return false;
	
	if (c0 != c1) // points on separate chains must be of same entity (e.g. outer rinsg vs hole0
	{
		auto gp = dynamic_cast<WED_GISPolygon*>(c0->GetParent());
		if (!gp || gp != dynamic_cast<WED_GISPolygon*>(c1->GetParent()))
			return false;

		// the current also is limited to cuts from the outer ring to a hole
		auto o = gp->GetOuterRing();
		if (o != dynamic_cast<IGISPointSequence*>(c0))
		{
			if (o != dynamic_cast<IGISPointSequence*>(c1))
				return false;
			else
			{
				swap(p0, p1);
				swap(c0, c1);
			}
		}
		pos_0 = p0->GetMyPosition();
		pos_1 = p1->GetMyPosition();
	}
	else 	// both points on same WED_GISChain must not be adjacent in that chain
	{
		pos_0 = p0->GetMyPosition();
		pos_1 = p1->GetMyPosition();
		if (pos_0 > pos_1)
		{
			std::swap(pos_0, pos_1);
			std::swap(p0, p1);
		}
		if (pos_1 == pos_0 + 1)
			return false;
		if (pos_0 == 0 && pos_1 == c0->CountChildren() - 1)
			return false;
	}

	if(info)
	{
		info->p0 = p0;
		info->p1 = p1;
		info->pos_0 = pos_0;
		info->pos_1 = pos_1;
		info->chain = c0;
		info->cut_to_hole = c0 != c1;
		info->hole = c1;
	}

	return true;
}

static bool is_edge_split(ISelection * sel)
{
	if (sel->GetSelectionCount() == 0)
		return false;
	if (sel->IterateSelectionOr(unsplittable, sel))
		return false;
	return true;
}

int		WED_CanSplit(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	return is_chain_split(sel, NULL) || is_ring_split(sel, NULL) || is_edge_split(sel);
}

static void do_chain_split(ISelection * sel, const chain_split_info_t & info)
{
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Split chain");

	int pos = info.p->GetMyPosition();

	if(info.e)
	{
		Point2 pt;
		info.p->GetLocation(gis_Geo, pt);
		IGISPoint * gp =  info.e->SplitEdge(pt, 0.0);
		if(gp)
		{
			sel->Clear();
			sel->Insert(gp);
		}
	}
	else if (info.c->IsClosed())
	{
		WED_AirportChain * ac = dynamic_cast<WED_AirportChain *>(info.c);
		if (!ac)
		{
			op->AbortOperation();
			return;
		}

		for (int i = 0; i < pos; ++i)
		{
			WED_Thing * t = ac->GetNthChild(0);
			t->SetParent(NULL, 0);
			t->SetParent(ac, ac->CountChildren());
		}

		WED_Thing * clone = dynamic_cast<WED_Thing *>(ac->GetNthChild(0)->Clone());
		if (clone)
		{
			clone->SetParent(ac, ac->CountChildren());
			sel->Insert(clone);
		}
		else
			clone->Delete();

		ac->SetClosed(0);
	}
	else
	{
		WED_GISPolygon * polygon = dynamic_cast<WED_GISPolygon*>(info.c->GetParent());
		WED_GISChain * chain_clone = NULL;

		// Is the chain the outer "ring" of a polygon?
		if (polygon && info.c == polygon->GetOuterRing())
		{
			// Clone the entire polygon
			WED_GISPolygon * polygon_clone = dynamic_cast<WED_GISPolygon*>(polygon->Clone());
			polygon_clone->SetParent(polygon->GetParent(), polygon->GetMyPosition()+1);
			chain_clone = dynamic_cast<WED_GISChain*>(polygon_clone->GetOuterRing());
		}
		else
		{
			// Clone just the chain
			chain_clone = dynamic_cast<WED_GISChain*>(info.c->Clone());
			chain_clone->SetParent(info.c->GetParent(), info.c->GetMyPosition()+1);
		}

		sel->Insert(chain_clone->GetNthChild(pos));

		set<WED_Thing*> to_delete;
		for (int i = 0; i < info.c->CountChildren(); ++i)
		{
			if (i < pos)
				to_delete.insert(chain_clone->GetNthChild(i));
			if (i > pos)
				to_delete.insert(info.c->GetNthChild(i));
		}

		WED_AddChildrenRecursive(to_delete);
		WED_RecursiveDelete(to_delete);
	}

	op->CommitOperation();
}

// Returns which side of the line formed by p1 and p2 the hole is on
// (LEFT_TURN or RIGHT_TURN). Returns COLLINEAR if the hole intersects the
// line.
static int hole_side(IGISPointSequence * hole, IGISPoint * p1, IGISPoint * p2)
{
	vector<Bezier2> pol;
	WED_BezierVectorForPointSequence(hole, pol);

	Point2 point1, point2;
	p1->GetLocation(gis_Geo, point1);
	p2->GetLocation(gis_Geo, point2);
	Segment2 segment(point1, point2);

	// Collect all points in the hole, including control points.
	vector<Point2> points;
	for (int i = 0; i < hole->GetNumPoints(); ++i)
	{
		IGISPoint * igis_point = hole->GetNthPoint(i);
		Point2 hole_point;
		igis_point->GetLocation(gis_Geo, hole_point);
		points.push_back(hole_point);

		IGISPoint_Bezier * bezier_point = dynamic_cast<IGISPoint_Bezier*>(igis_point);
		if (bezier_point)
		{
			bezier_point->GetControlHandleLo(gis_Geo, hole_point);
			points.push_back(hole_point);
			bezier_point->GetControlHandleHi(gis_Geo, hole_point);
			points.push_back(hole_point);
		}
	}

	if (points.empty())
		return COLLINEAR;

	int side = segment.side_of_line(points[0]);
	for (size_t i = 1; i < points.size(); ++i)
	{
		if (segment.side_of_line(points[i]) != side)
			return COLLINEAR;
	}

	return side;
}

static void delete_bezier_handle(IGISPoint * p, int handle) {
	IGISPoint_Bezier * bezier = dynamic_cast<IGISPoint_Bezier*>(p);
	if (!bezier)
		return;

	bezier->SetSplit(true);
	if (handle == 0)
		bezier->DeleteHandleLo();
	else
		bezier->DeleteHandleHi();
}

static void do_ring_split(ISelection* sel, const ring_split_info_t& info)
{
	auto parent = dynamic_cast<WED_Thing*>(info.chain->GetParent());
	if (!parent)
		return;

	auto polygon = dynamic_cast<WED_GISPolygon*>(info.chain->GetParent());

	IOperation* op = dynamic_cast<IOperation*>(sel);
	set<WED_Thing*> to_delete;

	if(!info.cut_to_hole)
	{
		vector<int> hole_sides;
		if (polygon && info.chain == polygon->GetOuterRing())
		{
			// For each hole, check which side of the split it is on.
			hole_sides.resize(polygon->GetNumHoles());
			for (int i = 0; i < polygon->GetNumHoles(); ++i)
			{
				IGISPointSequence* hole = polygon->GetNthHole(i);
				hole_sides[i] = hole_side(hole, info.p0, info.p1);
				if (hole_sides[i] == COLLINEAR)
				{
					// We could theoretically do this check already in
					// is_ring_split(), but it would be too hard for the user to
					// understand why the Split function is sometimes available
					// and sometimes greyed out. Instead, we do the check here so
					// we can display a meaningful message.
					DoUserAlert("Cannot split through holes");
					return;
				}
			}
		}

		op->StartOperation("Split ring");

		WED_GISChain* chain_clone = NULL;

		// Is the ring the outer ring of a polygon?
		if (polygon && info.chain == polygon->GetOuterRing())
		{
			// Clone the entire polygon
			auto polygon_clone = dynamic_cast<WED_GISPolygon*>(polygon->Clone());
			polygon_clone->SetParent(polygon->GetParent(), polygon->GetMyPosition() + 1);
			chain_clone = dynamic_cast<WED_GISChain*>(polygon_clone->GetOuterRing());

			// Distribute the holes between the original polygon and the clone
			for (int i = 0; i < polygon->GetNumHoles(); ++i)
			{
				if (hole_sides[i] == RIGHT_TURN)
					to_delete.insert(dynamic_cast<WED_Thing*>(polygon->GetNthHole(i)));
				else
					to_delete.insert(dynamic_cast<WED_Thing*>(polygon_clone->GetNthHole(i)));
			}
		}
		else
		{
			// Clone just the chain
			chain_clone = dynamic_cast<WED_GISChain*>(info.chain->Clone());
			chain_clone->SetParent(info.chain->GetParent(), info.chain->GetMyPosition() + 1);
		}

		sel->Insert(chain_clone->GetNthChild(info.pos_0));
		sel->Insert(chain_clone->GetNthChild(info.pos_1));

		// On the two shared points, delete the Bezier handles that face the other
		// polygon to make the two halves fit together exactly
		delete_bezier_handle(info.chain->GetNthPoint(info.pos_0), 1);
		delete_bezier_handle(info.chain->GetNthPoint(info.pos_1), 0);
		delete_bezier_handle(chain_clone->GetNthPoint(info.pos_0), 0);
		delete_bezier_handle(chain_clone->GetNthPoint(info.pos_1), 1);

		// Distribute the points among the the original and the clone
		for (int i = 0; i < info.chain->CountChildren(); ++i)
		{
			if (i > info.pos_0 && i < info.pos_1)
				to_delete.insert(info.chain->GetNthChild(i));
			if (i < info.pos_0 || i > info.pos_1)
				to_delete.insert(chain_clone->GetNthChild(i));
		}
	}
	else // cut from outer ring into hole
	{
		// check the cut does not cut through any unrelated hole
		
		Segment2 cut;
		info.p0->GetLocation(gis_Geo, cut.p1);
		info.p1->GetLocation(gis_Geo, cut.p2);
		for (int i = 0; i < polygon->GetNumHoles(); ++i)
		{
			auto hole = dynamic_cast<WED_GISChain*>(polygon->GetNthHole(i));
			for (int i = 0; i < hole->GetNumSides(); i++)
			{
				Bezier2 bez;
				Point2 pt;
				if (hole->GetSide(gis_Geo, i, bez) ? bez.intersect(cut, 10) : bez.as_segment().intersect(cut, pt))
					if(pt != cut.p1 && pt != cut.p2)
					{
						DoUserAlert("Cannot split through holes");
						return;
					}
			}
		}

		op->StartOperation("Split ring + merge hole");

		// duplicate the node in outer ring node AND the hole node;
		auto orng_clone = dynamic_cast<WED_Thing*>(info.p0->Clone());
		orng_clone->SetParent(info.chain, info.pos_0 + 1);
		auto hole_clone = dynamic_cast<WED_Thing*>(info.p1->Clone());
		hole_clone->SetParent(info.hole, info.pos_1 + 1);

		delete_bezier_handle(info.chain->GetNthPoint(info.pos_0), 1);
		delete_bezier_handle(info.chain->GetNthPoint(info.pos_0 + 1), 0);
		delete_bezier_handle(info.hole->GetNthPoint(info.pos_1), 1);
		delete_bezier_handle(info.hole->GetNthPoint(info.pos_1 + 1), 0);

		// Move all hole vertices in reverse order to fall in between the outer ring duplicate nodes
		int hole_vert_to_move = info.hole->GetNumPoints();
		for (int i = 0; i < hole_vert_to_move; i++)
		{
			int move_pos = (info.pos_1 + 1) >= info.hole->GetNumPoints() ? 0 : info.pos_1 + 1;
			dynamic_cast<WED_Thing*>(info.hole->GetNthPoint(move_pos))->SetParent(info.chain,info.pos_0 + i + 1);
		}

		// delete the hole chain - as the hole is gone now
		to_delete.insert(info.hole);
	}

	WED_AddChildrenRecursive(to_delete);
	WED_RecursiveDelete(to_delete);

	op->CommitOperation();
}

map<WED_Thing*,vector<WED_Thing*> > run_split_on_edges(vector<split_edge_info_t>& edges ,bool no_uncrossed)
{
	map<WED_Thing*, vector<WED_Thing*> > new_pieces;
	//
	// This block splits overlapping GIS edges anywhere they cross.
	//

	// Step 1: run a nested for loop and find all intersections between all
	// segments...if the intersection is in the interior, we accumulate it on
	// the edge.  This is O(N^2) - a sweep line would be better if we ever have
	// data sets big enough to need it.
	for (int i = 0; i < edges.size(); ++i)                        // MM: It would likely be more efficient to keep track of the particular segment(s) of overlap
	{                                                             // within each edge. And factor out a generic IGIS PointSequnce intersect algorithm.
		Bezier2 ib;                                               // Its done during validation for self-insersecting polygons as well.
		for(int ii = 0; ii < edges[i].edge->GetNumSides(); ii++)
		{
			bool ibIsBez = edges[i].edge->GetSide(gis_Geo,ii,ib);

			for (int j = 0; j < i; ++j)
			if(edges[i].active || edges[j].active)								// At least one edge MUST be active or we do not split.
			{
				Bezier2 jb;
				for(int jj = 0; jj < edges[j].edge->GetNumSides(); jj++)
				{
					bool jbIsBez = edges[j].edge->GetSide(gis_Geo,jj, jb);

					if (ib.p1 != ib.p2 &&    // MM: do we really need2test for zero-length/degenerated edges intersecting ?
						jb.p1 != jb.p2 &&    // same here
						ib.p1 != jb.p1 &&
						ib.p2 != jb.p2 &&
						ib.p1 != jb.p2 &&
						ib.p2 != jb.p1)
					{
						Point2 x;
						if( ibIsBez || jbIsBez )
						{
							//ToDo:mroe check precision and calculation time
							if (ib.intersect(jb,10,x))
							{
								edges[i].splits.push_back(x);
								edges[j].splits.push_back(x);
							}
						}
						else
						{
							if (ib.as_segment().intersect(jb.as_segment(), x))
							{
								edges[i].splits.push_back(x);
								edges[j].splits.push_back(x);
							}
						}
					}
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
		if (!no_uncrossed && edges[i].splits.empty())
		{
			Bezier2 b;
			if(edges[i].edge->GetSide(gis_Geo,-1,b))
			{
				edges[i].splits.push_back(b.midpoint(0.5));
			}
			else
			{
				edges[i].splits.push_back(b.as_segment().midpoint());
			}
		}

		// Now we go BACKWARD from high to low - we do this because the GIS Edge's split makes the clone
		// on the "dst" side - so by breaking off the very LAST split first, we keep as "us" the part of
		// the segment containing all other splits.  We work backward.
		for (vector<Point2>::reverse_iterator r = edges[i].splits.rbegin(); r != edges[i].splits.rend(); ++r)
		{
			// If we had a 'T' then in theory SplitSide could return NULL?
			IGISPoint * split = edges[i].edge->SplitEdge(*r, 0.0);
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

void do_edge_split(ISelection * sel)
{
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

		Bezier2		bez;

//		set<int> attrs;
//		node->GetAttributes(attrs);
///		new_node->SetAttributes(attrs);

		if (seq->GetSide(gis_Geo,(*w)->GetMyPosition(),bez))
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
				seq->GetSide(gis_UV,(*w)->GetMyPosition(),bez);
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
			as_p->SetLocation(gis_Geo,bez.as_segment().midpoint());
			if(as_p->HasLayer(gis_UV))
			{
				seq->GetSide(gis_UV,(*w)->GetMyPosition(),bez);
				as_p->SetLocation(gis_UV,bez.as_segment().midpoint());
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

void	WED_DoSplit(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);

	chain_split_info_t chain_info;
	if (is_chain_split(sel, &chain_info))
	{
		do_chain_split(sel, chain_info);
		return;
	}

	ring_split_info_t ring_info;
	if (is_ring_split(sel, &ring_info))
	{
		do_ring_split(sel, ring_info);
		return;
	}

	do_edge_split(sel);
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
			double dist = LonLatDistMeters(p1,p2);
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

static void get_bezier_points(WED_Thing * t, vector<WED_GISPoint_Bezier *> & points)
{
	WED_GISPoint_Bezier * bezier = dynamic_cast<WED_GISPoint_Bezier *>(t);
	if (bezier)
	{
		points.push_back(bezier);
		return;
	}

	for (int i = 0; i < t->CountChildren(); ++i)
		get_bezier_points(t->GetNthChild(i), points);
}

int		WED_CanMatchBezierHandles(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	if (selected.empty())
		return false;

	for (size_t i = 0; i < selected.size(); ++i)
	{
		WED_Thing * t = dynamic_cast<WED_Thing *>(selected[i]);
		if (!t)
			return false;
		vector<WED_GISPoint_Bezier *> points;
		get_bezier_points(t, points);
		if (points.empty())
			return false;
	}

	return true;
}

// Implementation helper for get_snapped_bezier_points().
static void get_snapped_bezier_points_impl(WED_Thing * t, const set<WED_GISPoint_Bezier *> & ignore,
	const set<Point2> & points, const Bbox2 & bounds, multimap<Point2, WED_GISPoint_Bezier *> & snapped)
{
	IGISEntity * ent = dynamic_cast<IGISEntity *>(t);
	if (!ent || !ent->IntersectsBox(gis_Geo, bounds))
		return;

	WED_GISPoint_Bezier * bezier = dynamic_cast<WED_GISPoint_Bezier *>(t);
	if (bezier && !ignore.count(bezier))
	{
		Point2 location;
		bezier->GetLocation(gis_Geo, location);

		if (points.count(location))
			snapped.insert(std::pair<const Point2, WED_GISPoint_Bezier *>(location, bezier));
	}

	for (int i = 0; i < t->CountChildren(); ++i)
		get_snapped_bezier_points_impl(t->GetNthChild(i), ignore, points, bounds, snapped);
}

// For the given set of Bezier points in the world 'wrl', finds all Bezier
// points that are snapped to them and returns them as a map of locations to
// points.
static void get_snapped_bezier_points(WED_Thing * wrl,
	const vector<WED_GISPoint_Bezier *> & points,
	multimap<Point2, WED_GISPoint_Bezier *> & snapped)
{
	set<WED_GISPoint_Bezier *> points_set;
	set<Point2> locations;
	Bbox2 bounds;
	for (size_t i = 0; i < points.size(); ++i)
	{
		points_set.insert(points[i]);
		Point2 location;
		points[i]->GetLocation(gis_Geo, location);
		locations.insert(location);
		bounds += location;
	}

	get_snapped_bezier_points_impl(wrl, points_set, locations, bounds, snapped);
}

// Finds the location of the point that is at a position 'relative_pos' relative
// to 'p' within the containing chain. Returns true if the point was found or
// false if the chain is not closed and the relative position fell outside the
// chain.
static bool get_relative_point(WED_GISPoint * p, int relative_pos, Point2 & location)
{
	WED_GISChain * c = dynamic_cast<WED_GISChain *>(p->GetParent());
	if (!c)
		return false;

	int absolute_pos = p->GetMyPosition() + relative_pos;
	if (c->IsClosed())
	{
		absolute_pos = absolute_pos % c->GetNumPoints();
		if (absolute_pos < 0)
			absolute_pos += c->GetNumPoints();
	}
	else
	{
		if (absolute_pos < 0 || absolute_pos >= c->GetNumPoints())
			return false;
	}

	IGISPoint * point = c->GetNthPoint(absolute_pos);
	point->GetLocation(gis_Geo, location);

	return true;
}

namespace {
// One of the handles of a Bezier point.
struct BezierHandle {
	// Side the handle is on. Values are chosen so that they can be passed to
	// get_relative_point().
	enum Side { LO = -1, HI = 1 };

	BezierHandle() : p(NULL), side(LO) {}
	BezierHandle(WED_GISPoint_Bezier * new_p, Side new_side) : p(new_p), side(new_side) {}
	WED_GISPoint_Bezier * p;
	Side side;
};
}

// Finds the handle of p2 that corresponds to the handle h1. Prerequisites:
// - p2 must be snapped to h1.p
// - The neighbors of h1.p and p2 on the side of the matching handles must also
//   be snapped together
static bool get_matching_handle(const BezierHandle & h1, WED_GISPoint_Bezier * p2, BezierHandle & h2)
{
	Point2 p1_neighbor;
	if (!get_relative_point(h1.p, h1.side, p1_neighbor))
		return false;
	BezierHandle::Side sides[2] = { BezierHandle::LO, BezierHandle::HI };
	for (int i = 0; i < 2; ++i)
	{
		Point2 p2_neighbor;
		if (get_relative_point(p2, sides[i], p2_neighbor) && p1_neighbor == p2_neighbor)
		{
			h2.p = p2;
			h2.side = sides[i];
			return true;
		}
	}

	return false;
}

// Copies the location of one Bezier handle to another.
static void copy_bezier_handle(const struct BezierHandle & dst, const struct BezierHandle & src)
{
	Point2 location;
	if (src.side == BezierHandle::LO)
		src.p->GetControlHandleLo(gis_Geo, location);
	else
		src.p->GetControlHandleHi(gis_Geo, location);
	if (dst.side == BezierHandle::LO)
		dst.p->SetControlHandleLo(gis_Geo, location);
	else
		dst.p->SetControlHandleHi(gis_Geo, location);
}

void	WED_DoMatchBezierHandles(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Match Bezier Handles");

	// Find all Bezier points to be matched.
	vector<WED_GISPoint_Bezier *> points;
	for (size_t i = 0; i < selected.size(); ++i)
	{
		WED_Thing * t = dynamic_cast<WED_Thing *>(selected[i]);
		if (!t)
			continue;
		get_bezier_points(t, points);
	}

	typedef multimap<Point2, WED_GISPoint_Bezier *> Snapped_t;
	Snapped_t snapped;
	get_snapped_bezier_points(wrl, points, snapped);

	for (size_t i = 0; i < points.size(); ++i)
	{
		Point2 location;
		points[i]->GetLocation(gis_Geo, location);

		// Among the Bezier points snapped to us, find matching handles on both sides.
		vector<BezierHandle> lo_matches, hi_matches;
		std::pair<Snapped_t::iterator, Snapped_t::iterator> range = snapped.equal_range(location);
		for (Snapped_t::iterator iter = range.first; iter != range.second; ++iter)
		{
			WED_GISPoint_Bezier * p = iter->second;

			BezierHandle handle;
			if (get_matching_handle(BezierHandle(points[i], BezierHandle::LO), p, handle))
				lo_matches.push_back(handle);
			if (get_matching_handle(BezierHandle(points[i], BezierHandle::HI), p, handle))
				hi_matches.push_back(handle);
		}

		if (lo_matches.empty() && hi_matches.empty())
			continue;

		// If we matched with handles of the same point on both sides, our
		// splitness is equal to the splitness of that point. Otherwise, we're
		// definitely split.
		if (lo_matches.size() == 1 && hi_matches.size() == 1 && lo_matches.front().p == hi_matches.front().p)
			points[i]->SetSplit(lo_matches.front().p->IsSplit());
		else
			points[i]->SetSplit(true);

		if (lo_matches.size() == 1)
			copy_bezier_handle(BezierHandle(points[i], BezierHandle::LO), lo_matches.front());
		if (hi_matches.size() == 1)
			copy_bezier_handle(BezierHandle(points[i], BezierHandle::HI), hi_matches.front());
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
		l += LonLatDistMeters(p1,p2);
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

	//edges are special , start and end bez ctr are owned by the edge
	if( seq->GetGISClass() == gis_Edge)
	{
		IGISEdge * ge = dynamic_cast< IGISEdge *>(seq);
		if(ge != nullptr)
		{
			Bezier2 b;
			if(ge->GetSide(gis_Geo,-1,b))
			{
				Point2 p;
				Vector2	v;
				v.dx = 0.0;
				v.dy = ru;
				p = ctr + VectorMetersToLL(ctr,v);
				v = v.perpendicular_ccw();
				v.normalize();
				v *= c;
				b.c1 = b.p1 != b.c1  ? p - VectorMetersToLL(ctr, v) : p;
				b.p1 = p;

				v.dx = ru*sin((n-1) * w);
				v.dy = ru*cos((n-1) * w);
				p = ctr + VectorMetersToLL(ctr,v);
				v = v.perpendicular_ccw();
				v.normalize();
				v *= c;
				b.c2 = b.p2 != b.c2  ? p + VectorMetersToLL(ctr, v) : p;
				b.p2 = p;
				ge->SetSideBezier(gis_Geo,b,-1);
			}
		}
	}

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

static bool is_within_snapping_distance(const merge_class_map::iterator& first_thing, const merge_class_map::iterator& second_thing)
{
	// since we're not using the coordTranslator any more, we have no way to cache the cos(lattitude) to calculate distances quick.
	// so we shortcut the more complex calculation by testing for lattitude first

	if (fabs(first_thing->first.y() - second_thing->first.y()) > MTR_TO_DEG_LAT) return 0;

	return LonLatDistMeters(first_thing->first, second_thing->first) < 1.0;
}

static const char * get_merge_tag_for_thing(IGISPoint * ething)
{
	// In order to merge, we haveto at least be a thing AND a point,
	// and have a parent that is a thing and an entity.  (If that's
	// not true, @#$ knows what is selected.)

	WED_Thing * thing = SAFE_CAST(WED_Thing, ething);
	if(thing == NULL)
		return NULL;

	WED_Thing * parent = thing->GetParent();
	IGISEntity * eparent = SAFE_CAST(IGISEntity, parent);
	if(eparent == NULL)
		return NULL;

	if(eparent->GetGISClass() == gis_Composite)
	{
		// If our parent is a composite, we are a point or vertex.
		// Merge nodes of edges, but not just raw points.  Don't let
		// the user select two windsocks and, um, "merge" them.
		if(thing->CountViewers() > 0)
			return ething->GetGISSubtype();
	}
	return NULL;
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

namespace
{
// Information for merging two non-closed chains
struct chain_merge_info_t {
	// The chains in question
	WED_GISChain * c0;
	WED_GISChain * c1;

	// Indexes of the selected points in the two chains (either the first or last point in each case)
	int pos_0;
	int pos_1;

	// Whether to merge the two selected points into one (i.e. whether they are snapped)
	bool merge_points;

	// Whether to select the whole chain once the merge is complete
	// (this is set if two entire chains were selected to merge)
	bool select_whole_chain;
};

// Information for merging two rings
struct ring_merge_info_t {
	// Information for each ring
	struct entry {
		// The ring in question (a closed chain)
		WED_GISChain * c;
		// The ring's polygon parent (null if the ring does not belong to a polygon)
		WED_GISPolygon * poly;
		// The indexes of the first and last points that will be retained after the merge
		int first;
		int last;
	};
	entry e[2];

	// Whether to select the whole ring once the merge is complete
	// (this is set if two entire rings were selected to merge)
	bool select_whole_ring;
};
};

static bool points_snapped(WED_GISPoint * p0, WED_GISPoint * p1)
{
	Point2 loc_0, loc_1;
	p0->GetLocation(gis_Geo, loc_0);
	p1->GetLocation(gis_Geo, loc_1);
	return loc_0 == loc_1;
}

// Tests whether the either of the endpoints of c0 and c1 are snapped together and, if so, returns them in p0 and p1.
static bool chains_snapped(WED_GISChain * c0, WED_GISChain * c1, WED_GISPoint ** p0, WED_GISPoint ** p1)
{
	if (c0->CountChildren() == 0 || c1->CountChildren() == 0)
		return false;

	for (int i0 = 0; i0 < 2; ++i0)
		for (int i1 = 0; i1 < 2; ++i1)
		{
			WED_GISPoint * p0_tmp = dynamic_cast<WED_GISPoint *>(c0->GetNthChild(i0 * (c0->CountChildren() - 1)));
			WED_GISPoint * p1_tmp = dynamic_cast<WED_GISPoint *>(c1->GetNthChild(i1 * (c1->CountChildren() - 1)));

			if (p0_tmp && p1_tmp && points_snapped(p0_tmp, p1_tmp))
			{
				*p0 = p0_tmp;
				*p1 = p1_tmp;
				return true;
			}
		}

	return false;
}

static bool is_chain_merge(ISelection * sel, chain_merge_info_t * info)
{
	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	// Must have exactly two points or two chains selected
	if (selected.size() != 2)
		return false;

	WED_GISPoint * p0 = dynamic_cast<WED_GISPoint*>(selected[0]);
	WED_GISPoint * p1 = dynamic_cast<WED_GISPoint*>(selected[1]);
	bool select_whole_chain = false;
	if (!p0 || !p1)
	{
		WED_GISChain * c0 = dynamic_cast<WED_GISChain*>(selected[0]);
		WED_GISChain * c1 = dynamic_cast<WED_GISChain*>(selected[1]);

		if (!c0 || !c1)
			return false;

		if (!chains_snapped(c0, c1, &p0, &p1))
			return false;

		select_whole_chain = true;
	}

	// Make p0 always be the point that is higher in the hierarchy
	if (!WED_ComesBeforeInHierarchy(p0, p1))
		std::swap(p0, p1);

	// The points must have two WED_GISChain parents that are:
	// - open
	// - of the same class
	// - different or, if they are the same, a WED_AirportChain
	WED_GISChain * c0 = dynamic_cast<WED_GISChain*>(p0->GetParent());
	WED_GISChain * c1 = dynamic_cast<WED_GISChain*>(p1->GetParent());
	if (!c0 || !c1 || c0->IsClosed() || c1->IsClosed() || c0->GetClass() != c1->GetClass())
		return false;
	if (c0 == c1 && !dynamic_cast<WED_AirportChain*>(c0))
		return false;

	// The points must lie at the end of the chain
	int pos_0 = p0->GetMyPosition();
	if (pos_0 != 0 && pos_0 != c0->CountChildren() - 1)
		return false;
	int pos_1 = p1->GetMyPosition();
	if (pos_1 != 0 && pos_1 != c1->CountChildren() - 1)
		return false;

	if (info)
	{
		info->c0 = c0;
		info->c1 = c1;
		info->pos_0 = pos_0;
		info->pos_1 = pos_1;

		info->merge_points = points_snapped(p0, p1);
		info->select_whole_chain = select_whole_chain;
	}

	return true;
}

// Returns whether a set of points in a ring (identified by their indexes 'idx') are adjacent to each other.
// 'num_points' is the number of points in the ring. The points with indexes 0 and num_points-1 are taken to be adjacent.
// If the points are all adjacent, the indexes of the first and last point in the sequence are returned in *first and *last.
static bool are_adjacent(set<int> idx, int num_points, int * first, int * last)
{
	if (idx.empty())
		return false;

	// Pick an arbitrary starting point.
	int start = *idx.begin();

	// Move forwards through the ring until we find a point that is not contained in 'idx'.
	int i = start;
	while (true)
	{
		idx.erase(i);
		int next = (i + 1) % num_points;
		if (!idx.count(next))
		{
			*last = i;
			break;
		}
		i = next;
	}

	// Now move backwards.
	i = start;
	while(true)
	{
		int next = (i + num_points - 1) % num_points;
		if (!idx.count(next))
		{
			*first = i;
			break;
		}
		idx.erase(next);
		i = next;
	}

	// If we visited all points, they were adjacent.
	return idx.empty();
}

// Checks whether the selection identifies two rings to be merged through a number of points selected on each ring.
static bool ring_info_from_points(const vector<ISelectable*> selected, vector<ring_merge_info_t::entry> * rings)
{
	// Must have points selected that belong to exactly two WED_GISChain parents.
	typedef std::map<WED_GISChain *, set<int> > ChainToPoints;
	ChainToPoints chain_to_points;
	for (size_t i = 0; i < selected.size(); ++i)
	{
		WED_GISPoint * p = dynamic_cast<WED_GISPoint*>(selected[i]);
		if (!p)
			return false;
		WED_GISChain * c = dynamic_cast<WED_GISChain*>(p->GetParent());
		if (!c)
			return false;
		chain_to_points[c].insert(p->GetMyPosition());
	}
	if (chain_to_points.size() != 2)
		return false;

	for (ChainToPoints::iterator iter = chain_to_points.begin(); iter != chain_to_points.end(); ++iter)
	{
		// Chains must be closed, and at least two points must be selected per chain.
		if (!iter->first->IsClosed())
			return false;
		if (iter->second.size() < 2)
			return false;

		ring_merge_info_t::entry entry;
		entry.c = iter->first;
		entry.poly = dynamic_cast<WED_GISPolygon *>(entry.c->GetParent());

		// Points must be adjacent. The last selected point is the first to remain in the chain.
		if (!are_adjacent(iter->second, iter->first->GetNumPoints(), &entry.last, &entry.first))
			return false;
		rings->push_back(entry);
	}

	return true;
}

// Checks whether the selection contains two rings or polygons to be merged (with the points to merge snapped together).
static bool ring_info_from_chains(const vector<ISelectable*> selected, vector<ring_merge_info_t::entry> * rings)
{
	// Must have exactly two chains or two polygons selected.
	if (selected.size() != 2)
		return false;

	WED_GISChain * chains[2];
	for (size_t i =0; i < 2; ++i)
	{
		WED_GISPolygon * p = dynamic_cast<WED_GISPolygon *>(selected[i]);
		if (p)
		{
			chains[i] = dynamic_cast<WED_GISChain *>(p->GetOuterRing());
			if (chains[i] == NULL)
				return false;
			continue;
		}
		chains[i] = dynamic_cast<WED_GISChain *>(selected[i]);
		if (!chains[i] || !chains[i]->IsClosed())
			return false;
	}

	// For all points in the first chain, create a map from location to index.
	map<Point2, int> points_0;
	for (int i = 0; i < chains[0]->GetNumPoints(); ++i)
	{
		Point2 p;
		chains[0]->GetNthPoint(i)->GetLocation(gis_Geo, p);
		points_0[p] = i;
	}

	// Find points in both chains that are snapped to counterparts in the other chain.
	set<int> idx[2];
	for (int i = 0; i < chains[1]->GetNumPoints(); ++i)
	{
		Point2 p;
		chains[1]->GetNthPoint(i)->GetLocation(gis_Geo, p);
		map<Point2, int>::iterator iter = points_0.find(p);
		if (iter != points_0.end())
		{
			idx[0].insert(iter->second);
			idx[1].insert(i);
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		// At least two points must be selected per chain.
		if (idx[i].size() < 2)
			return false;

		ring_merge_info_t::entry entry;
		entry.c = chains[i];
		entry.poly = dynamic_cast<WED_GISPolygon *>(entry.c->GetParent());

		// Points must be adjacent. The last selected point is the first to remain in the chain.
		if (!are_adjacent(idx[i], chains[i]->GetNumPoints(), &entry.last, &entry.first))
			return false;
		rings->push_back(entry);
	}

	return true;
}

static bool is_ring_merge(ISelection * sel, ring_merge_info_t * info)
{
	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	vector<ring_merge_info_t::entry> rings;
	bool select_whole_ring = false;
	if (ring_info_from_chains(selected, &rings))
		select_whole_ring = true;
	else if (!ring_info_from_points(selected, &rings))
		return false;

	// Either both chains belong to a polygon or neither does
	bool is_poly_0 = (rings[0].poly != NULL);
	bool is_poly_1 = (rings[1].poly != NULL);
	if (is_poly_0 != is_poly_1)
		return false;

	if (is_poly_0)
	{
		// The polygons must be of the same class
		if (rings[0].poly->GetClass() != rings[1].poly->GetClass())
			return false;

		// The chains must either both be outer rings or they must both be holes
		bool is_outer_ring_0 = (rings[0].c->GetMyPosition() == 0);
		bool is_outer_ring_1 = (rings[1].c->GetMyPosition() == 0);
		if (is_outer_ring_0 != is_outer_ring_1)
			return false;

		// If the chains are outer rings, the polygons must be different.
		// If the chains are holes, the polygons must be the same.
		if (is_outer_ring_0)
		{
			if (rings[0].poly == rings[1].poly)
				return false;
			// Make poly_0 always be the polygon that is higher in the hierarchy
			if (!WED_ComesBeforeInHierarchy(rings[0].poly, rings[1].poly))
				std::swap(rings[0], rings[1]);
		}
		else
		{
			if (rings[0].poly != rings[1].poly)
				return false;
		}
	}
	else
	{
		// The chains must be of the same class
		if (rings[0].c->GetClass() != rings[1].c->GetClass())
			return false;

		// Make c0 always be the chain that is higher in the hierarchy
		if (!WED_ComesBeforeInHierarchy(rings[0].c, rings[1].c))
			std::swap(rings[0], rings[1]);
	}

	if (info)
	{
		info->e[0] = rings[0];
		info->e[1] = rings[1];
		info->select_whole_ring = select_whole_ring;
	}

	return true;
}

//Returns true if every node can be merged with each other, by type and by location
static bool is_node_merge(IResolver * resolver)
{
	//Performance notes 1/6/2017:
	//Release build -> 1300 nodes collected
	//Completed test in
	//   0.001130 seconds.
	//   0.020325 seconds.
	//   0.000842 seconds.
	//   0.020704 seconds.
	//   0.016719 seconds.
	//StElapsedTime can_merge_timer("WED_CanMerge");
	ISelection * sel = WED_GetSelect(resolver);
#if ROAD_EDITING
	if(sel->GetSelectionCount() == 1) // this means merge two edges here into one
	{
		WED_Thing * thing = dynamic_cast<WED_Thing *>(sel->GetNthSelection(0));
		if( thing == nullptr ||
			thing->GetClass() != WED_RoadNode::sClass ||
			thing->CountViewers() != 2) return false;

		set<WED_Thing *> viewers;
		thing->GetAllViewers(viewers);

		if( (*viewers.begin())->GetClass()     != WED_RoadEdge::sClass) return false;
		if( (*(++viewers.begin()))->GetClass() != WED_RoadEdge::sClass) return false;

		WED_RoadEdge * road_edge_1 = static_cast<WED_RoadEdge *>(*viewers.begin());
		WED_RoadEdge * road_edge_2 = static_cast<WED_RoadEdge *>(*(++viewers.begin()));

		// check for road subtype
		if(road_edge_1->GetSubtype() != road_edge_2->GetSubtype()) return false;

		Bezier2 b1,b2;
		road_edge_1->GetSide(gis_Geo,-1,b1);
		road_edge_2->GetSide(gis_Geo,-1,b2);
		// check for direction
		if(b1.p2 != b2.p1 && b1.p1 != b2.p2) return false;
		// check resource match
		string resource_1,resource_2;
		road_edge_1->GetResource(resource_1);
		road_edge_2->GetResource(resource_2);
		if(resource_1 != resource_2) return false;

		// check if one edge erroneous is a ring or would be closed ( start = end node)
		WED_Thing * edge1_src0 = road_edge_1->GetNthSource(0);
		WED_Thing * edge1_src1 = road_edge_1->GetNthSource(1);
		if( edge1_src0 == edge1_src1 ) return false;
		WED_Thing * edge2_src0 = road_edge_2->GetNthSource(0);
		WED_Thing * edge2_src1 = road_edge_2->GetNthSource(1);
		if( edge2_src0 == edge2_src1 ) return false;

		bool add_edge_end = edge1_src0 == thing;
		if( add_edge_end && edge1_src1 == edge2_src0 ) return false;
		if(!add_edge_end && edge1_src0 == edge2_src1 ) return false;

		return true;
	}
#endif

	if(sel->GetSelectionCount() < 2)
		return false;		// can't merging 1 thing mean merge two endges into one

	//1. Ensure all of the selection is mergeable, collect
	merge_class_map sinkmap;
	if (!sel->IterateSelectionAnd(iterate_can_merge, &sinkmap))
		return false;

	if (sinkmap.size() > 10000 || sinkmap.size() < 2)
		return false;

	//2. Sort by location, a small optimization
	sort(sinkmap.begin(), sinkmap.end(), lesser_y_then_x_merge_class_map);

	//Keeps track of which objects we've discovered we can snap (hopefully all)
	set<merge_class_map::iterator> can_snap_objects;

	//For each item in the sink map
	for (merge_class_map::iterator thing_1_itr = sinkmap.begin(); thing_1_itr != sinkmap.end() - 1; ++thing_1_itr)
	{
		//For each item after that
		for (merge_class_map::iterator merge_pair_itr = thing_1_itr + 1; merge_pair_itr != sinkmap.end(); ++merge_pair_itr)
		{
#if ROAD_EDITING
			//do not merge road nodes from same edge or with different resource
			if( thing_1_itr->second.first == WED_RoadNode::sClass
			 && merge_pair_itr->second.first == WED_RoadNode::sClass)
			{
				set<WED_Thing *> viewers1,viewers2;
				thing_1_itr->second.second->GetAllViewers(viewers1);
				merge_pair_itr->second.second->GetAllViewers(viewers2);

				for(auto v1 : viewers1)
					for(auto v2 : viewers2)
					{
						if(v1 == v2) return false;  // same edge , do not merge
						WED_RoadEdge * road_edge_1 = dynamic_cast<WED_RoadEdge *>(v1);
						WED_RoadEdge * road_edge_2 = dynamic_cast<WED_RoadEdge *>(v2);

						if(road_edge_1 == nullptr || road_edge_2 == nullptr) return false;

						string res1,res2;
						road_edge_1->GetResource(res1);
						road_edge_2->GetResource(res2);
						if(res1 != res2) return false;
					}
			}
#endif
			//If the two things are within snapping distance of each other, record so
			if (is_within_snapping_distance(thing_1_itr, merge_pair_itr))
			{
				can_snap_objects.insert(thing_1_itr);
				can_snap_objects.insert(merge_pair_itr);
			}
		}
	}

	//Ensure expected UI behavior - Only perfect merges are allowed
	return can_snap_objects.size() == sinkmap.size();
}

int	WED_CanMerge(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);

	return is_chain_merge(sel, NULL) || is_ring_merge(sel, NULL) || is_node_merge(resolver)? 1 : 0;
}

static void do_chain_merge(ISelection * sel, const chain_merge_info_t & info)
{
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Merge chains");

	sel->Clear();

	// Make sure points to be merged are at end of first chain and beginning of second chain.
	if (info.pos_0 == 0)
		info.c0->Reverse(gis_Geo);
	if (info.pos_1 != 0)
		info.c1->Reverse(gis_Geo);

	WED_Thing * p0 =  info.c0->GetNthChild(info.c0->CountChildren() -1);
	WED_Thing * p1 =  info.c1->GetNthChild(0);

	set<WED_Thing*> to_delete;
	if (info.merge_points)
	{
		p0->SetParent(NULL, 0);
		to_delete.insert(p0);
	}
	else
	{
		if (!info.select_whole_chain)
			sel->Insert(p0);
	}
	if (info.select_whole_chain)
		sel->Insert(info.c0);
	else
		sel->Insert(p1);

	if (info.c0 == info.c1)
	{
		WED_AirportChain * ac = dynamic_cast<WED_AirportChain *>(info.c0);
		if (ac)
			ac->SetClosed(1);
	}
	else
	{
		vector<WED_Thing *> points;
		for (int i = 0; i < info.c1->CountChildren(); ++i)
			points.push_back(info.c1->GetNthChild(i));
		for (size_t i = 0; i < points.size(); ++i)
			points[i]->SetParent(info.c0, info.c0->CountChildren());

		WED_GISPolygon * poly = dynamic_cast<WED_GISPolygon *>(info.c1->GetParent());
		if (poly)
		{
			poly->SetParent(NULL, 0);
			to_delete.insert(poly);
		}
		else
		{
			info.c1->SetParent(NULL, 0);
			to_delete.insert(info.c1);
		}
	}

	WED_AddChildrenRecursive(to_delete);
	WED_RecursiveDelete(to_delete);

	op->CommitOperation();
}

static bool is_ccw(WED_GISChain * c)
{
	vector<Point2> points(c->GetNumPoints());
	for (int i = 0; i < c->GetNumPoints(); ++i)
		c->GetNthPoint(i)->GetLocation(gis_Geo, points[i]);
	return is_ccw_polygon_pt(points.begin(), points.end());
}

static void do_ring_merge(ISelection * sel, ring_merge_info_t info)
{
	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Merge rings");

	sel->Clear();

	// Do we need to merge polygons?
	set<WED_Thing*> to_delete;
	if (info.e[0].poly != info.e[1].poly)
	{
		// Transfer all of the holes to the first polygon
		while (info.e[1].poly->GetNumHoles() > 0)
			info.e[0].poly->AddHole(info.e[1].poly->GetNthHole(0));

		to_delete.insert(info.e[1].poly);
	}
	else
	{
		to_delete.insert(info.e[1].c);
	}

	// Make sure chains are both counter-clockwise.
	for (int i = 0; i < 2; ++i)
	{
		if (!is_ccw(info.e[i].c))
		{
			info.e[i].c->Reverse(gis_Geo);
			std::swap(info.e[i].first, info.e[i].last);
			info.e[i].first = info.e[i].c->CountChildren() - 1 - info.e[i].first;
			info.e[i].last = info.e[i].c->CountChildren() - 1 - info.e[i].last;
		}
	}

	// Collect the points that will make up the finished merged ring.
	vector<WED_GISPoint *> points[2];
	for (int i = 0; i < 2; ++i)
	{
		// First, take all the points out of the ring.
		int num_children = info.e[i].c->CountChildren();
		for (int child = 0; child < num_children; ++child)
		{
			int n = (child + info.e[i].first) % num_children;
			WED_GISPoint * p = dynamic_cast<WED_GISPoint *>(info.e[i].c->GetNthChild(n));
			if (!p)
				continue;
			points[i].push_back(p);
		}
		for (size_t p = 0; p < points[i].size(); ++p)
			points[i][p]->SetParent(NULL, 0);

		// Mark the extraneous points for deletion.
		int points_to_keep = (info.e[i].last - info.e[i].first + num_children) % num_children + 1;
		while (points[i].size() > points_to_keep)
		{
			to_delete.insert(points[i].back());
			points[i].pop_back();
		}

		if (points[i].empty())
		{
			DebugAssert(false);
			op->AbortOperation();
			return;
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		if (!info.select_whole_ring)
			sel->Insert(points[i].front());

		// Keep only one copy of points that are snapped together.
		WED_GISPoint * p_this = points[i].back();
		WED_GISPoint * p_other = points[(i + 1) % 2].front();
		if (points_snapped(p_this, p_other))
		{
			// Merge the low Bezier handle into the point we're snapped to.
			IGISPoint_Bezier * bez_this = dynamic_cast<IGISPoint_Bezier*>(p_this);
			IGISPoint_Bezier * bez_other = dynamic_cast<IGISPoint_Bezier*>(p_other);
			if (bez_this && bez_other)
			{
				Point2 loc_this, loc_other, handle;
				bez_this->GetLocation(gis_Geo, loc_this);
				bez_other->GetLocation(gis_Geo, loc_other);
				bez_this->GetControlHandleLo(gis_Geo, handle);
				handle += Vector2(loc_this, loc_other);
				bez_other->SetSplit(true);
				bez_other->SetControlHandleLo(gis_Geo, handle);
			}

			p_this->SetParent(NULL, 0);
			to_delete.insert(p_this);
			points[i].pop_back();
		}
		else
		{
			if (!info.select_whole_ring)
				sel->Insert(p_this);
		}

		// Re-insert all points into the first chain.
		for (size_t p = 0; p < points[i].size(); ++p)
			points[i][p]->SetParent(info.e[0].c, info.e[0].c->CountChildren());
	}

	if (info.select_whole_ring)
	{
		if (info.e[0].poly != info.e[1].poly)
			sel->Insert(info.e[0].poly);
		else
			sel->Insert(info.e[0].c);
	}

	WED_AddChildrenRecursive(to_delete);
	WED_RecursiveDelete(to_delete);

	op->CommitOperation();
}

//do_node_merge will merge every node in the selection possible, any nodes that could not be merged are left as is.
//Ex: 0-0-0-0---------------------0 will turn into 0-------------------------0.
//If is_node_merge is called first this behavior would not be possible
static void do_node_merge(IResolver * resolver)
{
	//Performance notes 1/6/2017:
	//Release build -> 1300 nodes collected
	//Completed snapping and merged to ~900 in
	//   0.023290 seconds
	//StElapsedTime can_merge_timer("WED_DoMerge");
	ISelection * sel = WED_GetSelect(resolver);

	IOperation * op = dynamic_cast<IOperation *>(sel);
	op->StartOperation("Merge Nodes");

	DebugAssert(sel->GetSelectionCount() >= 1);

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
		op->AbortOperation();
		return;
	}
	else if (sinkmap.size() == 1)  // merge two RoadEdges connected to a RoadNode
	{
		WED_Thing * thing = dynamic_cast<WED_Thing *>(sel->GetNthSelection(0));
		if(thing)
		{
			sel->Clear();
			set<WED_Thing *> viewers;
			thing->GetAllViewers(viewers);
			WED_Thing * edge = *viewers.begin();
			WED_Thing * obsolete_edge = *(++viewers.begin());

			bool add_edge_end = edge->GetNthSource(0) == thing;

			if( ( add_edge_end && edge->GetNthSource(1) == obsolete_edge->GetNthSource(0)) ||
			    (!add_edge_end && edge->GetNthSource(0) == obsolete_edge->GetNthSource(1)) )    // this would endup in closed edge ( start = end node)
			{
				op->AbortOperation();
				return;
			}

			WED_GISEdge * ge_1 = dynamic_cast<WED_GISEdge *>(edge);
			WED_GISEdge * ge_2 = dynamic_cast<WED_GISEdge *>(obsolete_edge);

			if(ge_1 && ge_2)
			{
				auto node = WED_SimpleBezierBoundaryNode::CreateTyped(thing->GetArchive());
				node->SetName("Shape Point");

				Bezier2 b1,b2;
				bool is_bez_1 = ge_1->GetSide(gis_Geo,-1,b1);
				bool is_bez_2 = ge_2->GetSide(gis_Geo,-1,b2);

				int nc = obsolete_edge->CountChildren();

				if(is_bez_1 || is_bez_2) node->SetSplit(true);
				if(add_edge_end == false)
				{
					DebugAssert(b1.p2 == b2.p1);
					node->SetParent(edge, edge->CountChildren());
					node->SetLocation(gis_Geo,b1.p2);

					if(is_bez_1 && (b1.p2 != b1.c2)) node->SetControlHandleLo(gis_Geo, b1.c2);
					if(is_bez_2 && (b2.p1 != b2.c1)) node->SetControlHandleHi(gis_Geo, b2.c1);

					for(int i = 0; i < nc; i++)
							obsolete_edge->GetNthChild(i)->SetParent(edge,edge->CountChildren());

					edge->ReplaceSource(thing, obsolete_edge->GetNthSource(1));
					//TODO:mroe: no clue why we must do it afterwards
					if(is_bez_2 && (b2.p2 != b2.c2)) ge_1->SetSideBezier(gis_Geo,Bezier2(b1.p1,b1.c1,b2.c2,b2.p2),-1);
				}
				else
				{
					DebugAssert(b2.p2 == b1.p1);
					node->SetParent(edge, 0);
					node->SetLocation(gis_Geo,b2.p2);
					if(is_bez_1 && (b1.p1 != b1.c1)) node->SetControlHandleHi(gis_Geo, b1.c1);
					if(is_bez_2 && (b2.p2 != b2.c2)) node->SetControlHandleLo(gis_Geo, b2.c2);

					for(int i = nc - 1; i >= 0; i--)
						obsolete_edge->GetNthChild(i)->SetParent(edge,0);

					edge->ReplaceSource(thing, obsolete_edge->GetNthSource(0));
					//TODO:mroe: no clue why we must do it afterwards
					if(is_bez_2 && (b2.p1 != b2.c1)) ge_1->SetSideBezier(gis_Geo,Bezier2(b2.p1,b2.c1,b1.c2,b1.p2),-1);
				}

				sel->Insert(node);
				viewers.clear();
				viewers.insert(thing);
				WED_RecursiveDelete(viewers);	// this makes obsolete_edge unviable and thus removed it as well
			}
		}

		op->CommitOperation();
		return;
	}

	//2. Sort by location, a small optimization
	sort(sinkmap.begin(), sinkmap.end(), lesser_y_then_x_merge_class_map);

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
				bool can_be_merged = tag_1 == tag_2 ;
#if ROAD_EDITING
				//do not merge road nodes from same edge or with different resource
				if(can_be_merged && tag_1 == WED_RoadNode::sClass )
				{
					set<WED_Thing *> viewers1,viewers2;
					start_thing->second.second->GetAllViewers(viewers1);
					merge_pair->second.second->GetAllViewers(viewers2);

					for(auto v1 : viewers1)
					{
						for(auto v2 : viewers2)
						{
							if(v1 == v2)					//do not merge start and end node of same edge
							{
								can_be_merged = false;
								break;
							}

							WED_RoadEdge * road_edge_1 = dynamic_cast<WED_RoadEdge *>(v1);
							WED_RoadEdge * road_edge_2 = dynamic_cast<WED_RoadEdge *>(v2);

							if(road_edge_1 != nullptr && road_edge_2 != nullptr)
							{
								string res1,res2;
								road_edge_1->GetResource(res1);
								road_edge_2->GetResource(res2);
								if(res1 != res2)
								{
									can_be_merged = false;
									break;
								}
							}
						}

						if(!can_be_merged) break;
					}
				}
#endif
				if (can_be_merged && is_within_snapping_distance(start_thing, merge_pair))
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

void WED_DoMerge(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);

	chain_merge_info_t chain_info;
	if (is_chain_merge(sel, &chain_info))
	{
		do_chain_merge(sel, chain_info);
		return;
	}

	ring_merge_info_t ring_info;
	if (is_ring_merge(sel, &ring_info))
	{
		do_ring_merge(sel, ring_info);
		return;
	}

	do_node_merge(resolver);
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

int WED_CanCopyToAirport(IResolver * resolver, string& aptName)
{
    bool good_selection = WED_CanDuplicate(resolver);

   	WED_Airport * curApt = WED_GetCurrentAirport(resolver);
	if (curApt)
	{
		bool can_move = WED_CanMoveSelectionTo(resolver, curApt, 0);

// do we want to allow moving items that are already at this airport ? Might confuse user to copy *into* the LEGO airport

		curApt->GetName(aptName);
		aptName = string("Duplicate + Move to Airport '") + aptName + "'";
		return good_selection && can_move;
	}
	else
		return 0;
}

#define PAD_POINTS_FOR_ZOOM_MTR 50.0

static int accum_box(ISelectable * who, void * ref)
{
	Bbox2 * total = (Bbox2 *) ref;
	IGISEntity * ent = dynamic_cast<IGISEntity *>(who);
	if (ent)
	{
		Bbox2 ent_box;
		ent->GetBounds(gis_Geo,ent_box);
		GISClass_t gc = ent->GetGISClass();
		if(gc == gis_Point || gc == gis_Point_Bezier || gc == gis_Point_Heading)
		{
			if(ent_box.is_empty())
			{
				double lat = ent_box.ymin();
				double MTR_TO_DEG_LON = MTR_TO_DEG_LAT * cos(lat * DEG_TO_RAD);
				ent_box.expand(PAD_POINTS_FOR_ZOOM_MTR * MTR_TO_DEG_LON,PAD_POINTS_FOR_ZOOM_MTR * MTR_TO_DEG_LAT);
			}
		}

		if(!ent_box.is_null())
			*total += ent_box;
	}
	return 0;
}

void WED_DoCopyToAirport(IResolver * resolver)
{
  	WED_Airport * curApt = WED_GetCurrentAirport(resolver);

    Bbox2 apt_bounds;
    curApt->GetBounds(gis_Geo, apt_bounds);
    Point2 newCtr = apt_bounds.centroid();

    Bbox2 sel_bounds;
	ISelection *		sel;
	sel = WED_GetSelect(resolver);
	sel->IterateSelectionOr(accum_box,&sel_bounds);
    Point2 selCtr = sel_bounds.centroid();

    Bbox2 new_bounds(sel_bounds);
    new_bounds.p1 += Vector2(selCtr,newCtr);
    new_bounds.p2 += Vector2(selCtr,newCtr);

	IOperation * op = dynamic_cast<IOperation *>(sel);
    if(op)
    {
        op->StartOperation("Duplicate to Airport");
        WED_DoDuplicate(resolver, false);

    // simplified version of
    //   WED_MarqueeTool::ApplyRescale(bounds,new_bounds);
    // that does not care about locked or hidden

        vector<ISelectable *>	iu;
        set<IGISEntity *>		ent_set;

        sel->GetSelectionVector(iu);
        for (vector<ISelectable *>::iterator i = iu.begin(); i != iu.end(); ++i)
        {
            IGISEntity * ent = SAFE_CAST(IGISEntity,*i);
            if (ent)
            {
                if(ent->GetGISClass() == gis_Edge)
                {
                    IGISPointSequence * ps = dynamic_cast<IGISPointSequence *>(ent);
                    if(ps)
                    {
                        int np = ps->GetNumPoints();
                        for(int n = 0; n < np; ++n)
                            ent_set.insert(ps->GetNthPoint(n));
                    }
                }
                else
                    ent_set.insert(ent);
            }
        }
        for(set<IGISEntity *>::iterator e = ent_set.begin(); e != ent_set.end(); ++e)
            (*e)->Rescale(gis_Geo,sel_bounds,new_bounds);

        op->CommitOperation();
    }

    WED_DoMoveSelectionTo(resolver, curApt, 0);
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
	LOG_MSG("E/Repair:\n");
	for(auto u : unviable)
	{
		string nam;
		u->GetName(nam);
		LOG_MSG("  Deleting %s '%s'",u->HumanReadableType(), nam.c_str());
		if (WED_Thing * parent = u->GetParent())
		{
			parent->GetName(nam);
			LOG_MSG(" from parent %s '%s'", parent->HumanReadableType(), nam.c_str());
		}
		LOG_MSG("\n");
	}
	root->StartOperation("Repair");
	WED_RecursiveDelete(unviable);
	WED_SetAnyAirport(resolver);
	root->CommitOperation();
	return 1;
}

//----------------------------------------------------------------------------
// Obj and Agp Replacement
//----------------------------------------------------------------------------

static set<string> build_agp_list()
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

int		WED_CanBreakApartAgps(IResolver* resolver)
{
	//Returns true if the selection contains only .agp objects

	vector<ISelectable*> selected;
	WED_GetSelect(resolver)->GetSelectionVector(selected);

	if (selected.empty()) return false;

	for (auto itr : selected)
	{
		WED_AgpPlacement* agp = dynamic_cast<WED_AgpPlacement*>(itr);
		if (!agp) return false;

		string agp_resource;
		agp->GetResource(agp_resource);
		if(FILE_get_file_extension(agp_resource) != "agp") return false;
	}
	return true;
}

static void replace_all_obj_in_agp(WED_AgpPlacement* agp, const agp_t * agp_data, WED_Archive* archive, vector<WED_ObjPlacement*>& out_added_objs)
{
	Point2 agp_origin_geo;
	agp->GetLocation(gis_Geo, agp_origin_geo);

	auto ti = agp_data->tiles.front();
	for (auto& agp_obj : ti.objs)
	{
		Vector2 torotate(agp_obj.x, agp_obj.y);

		//Note!! WED has clockwise heading, C's cos and sin functions are ccw in radians. We reverse directions and negate again
		torotate.rotate_by_degrees(agp->GetHeading()*-1);
		torotate *= -1;

		Point2 new_point_geo = agp_origin_geo -VectorMetersToLL(agp_origin_geo, torotate);

		WED_ObjPlacement* new_obj = WED_ObjPlacement::CreateTyped(archive);
		new_obj->SetLocation(gis_Geo, new_point_geo);

		//Other data that is important to resetting up the object
		new_obj->SetHeading(agp_obj.r + agp->GetHeading());
		new_obj->SetName(FILE_get_file_name(agp_obj.name));
		new_obj->SetParent(agp->GetParent(), agp->GetMyPosition());
		new_obj->SetResource(agp_obj.name);
		new_obj->SetShowLevel(agp->GetShowLevel());

		out_added_objs.push_back(new_obj);
	}
}

int wed_break_apart_special_agps(const vector<WED_AgpPlacement*>& agp_placements, WED_ResourceMgr * rmgr, vector<WED_ObjPlacement*>& out_added_objs)
{
	//The list of agp files we've decided to be special "service truck related"
	set<string> agp_list = build_agp_list();
	vector<WED_AgpPlacement*> replaced_agps;

	for (auto agp : agp_placements)
	{
		string agp_resource;

		agp->GetResource(agp_resource);
		//Is the agp found in the special agp list?
		if (agp_list.find(agp_resource) != agp_list.end())
		{
			//Break it all up here
			const agp_t * agp_data;
			if (rmgr->GetAGP(agp_resource, agp_data))
			{
				replace_all_obj_in_agp(agp, agp_data, agp->GetArchive(), out_added_objs);
				replaced_agps.push_back(agp);
			}
		}
	}

	for (auto itr_agp : replaced_agps)
	{
		itr_agp->SetParent(NULL, 0);
		itr_agp->Delete();
	}

	return replaced_agps.size();
}

void	WED_DoBreakApartAgps(IResolver* resolver)
{
	WED_Thing * root 		= WED_GetWorld(resolver);
	WED_ResourceMgr * rmgr	= WED_GetResourceMgr(resolver);
	WED_LibraryMgr * lmgr	= WED_GetLibraryMgr(resolver);
	ISelection * sel 		= WED_GetSelect(resolver);

	vector<ISelectable*> selected;
	sel->GetSelectionVector(selected);

	vector<WED_AgpPlacement*> agp_placements;
	for (auto itr : selected)
	{
		WED_AgpPlacement* agp = dynamic_cast<WED_AgpPlacement*>(itr);
		if (agp)
			agp_placements.push_back(agp);
	}

	root->StartOperation("Break Apart Agps");

	vector<WED_AgpPlacement*> replaced_agps;
	vector<WED_ObjPlacement*> added_objs;

	for(auto agp : agp_placements)
	{
		string agp_resource;
		const agp_t * agp_data;

		agp->GetResource(agp_resource);
		if(rmgr->GetAGP(agp_resource, agp_data))
		{
			bool all_obj_public = true;
			for (auto& agp_obj : agp_data->tiles.front().objs)
			{
				if(lmgr->IsResourceDeprecatedOrPrivate(agp_obj.name))
				{
					all_obj_public = false;
					break;
				}
			}
			if(all_obj_public)
			{
				replace_all_obj_in_agp(agp, agp_data, root->GetArchive(), added_objs);
				replaced_agps.push_back(agp);
			}
		}
	}

	for (auto itr_agp : replaced_agps)
	{
		itr_agp->SetParent(NULL, 0);
		itr_agp->Delete();
	}

	if(replaced_agps.size() == 0)
	{
		root->AbortOperation();
		DoUserAlert("No agp's with all public objects found. Nothing replaced."); //IMPORTANT: Do not call DoUserAlert during an operation!!!
	}
	else
	{
		sel->Clear();
		sel->Insert(set<ISelectable*>(added_objs.begin(), added_objs.end()));
		root->CommitOperation();

		stringstream ss;
		ss << "Replaced " << replaced_agps.size() << " agp's by " << sel->GetSelectionCount() << " individual objects.";
		if (replaced_agps.size() < agp_placements.size())
			ss << "\nSome agp referenced non-public objects and were not broken up.";
		DoUserAlert(ss.str().c_str());
	}
}

template <typename T>
static bool HasChildOfTypeRecursive(WED_Thing* thing, bool must_be_visible)
{
	if(thing->GetClass() == T::sClass)
	{
		if(must_be_visible)
		{
			if(auto e = dynamic_cast<WED_Entity *>(thing))
				return !e->GetHidden();
		}
		else
			return true;
	}
	int nc = thing->CountChildren();
	for(int n = 0; n < nc; ++n)
	{
		if(HasChildOfTypeRecursive<T>(thing->GetNthChild(n), must_be_visible))
			return true;
	}
	return false;
}

int	WED_CanReplaceVehicleObj(WED_Airport* apt)
{
	//Returns true if there are any Obj files in the airport.
	if(!apt) return false;
	return HasChildOfTypeRecursive<WED_ObjPlacement>(apt, true);  // takes way too log if its a big airport. We only need ONE !
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

	table.insert(make_pair("lib/airport/Ramp_Equipment/Belt_Loader.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Loader, 0)));
	table.insert(make_pair("lib/airport/vehicles/baggage_handling/belt_loader.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Loader, 0)));
	for(int i = 1; i <= 5; ++i)
	{
 		string s = "lib/airport/Ramp_Equipment/Lugg_Train_Straight" + to_string(i) + ".obj";
		table.insert(make_pair(s, vehicle_replacement_info(atc_ServiceTruck_Baggage_Train,i)));
	}
	table.insert(make_pair("lib/airport/Ramp_Equipment/Luggage_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_Baggage_Train,0)));
	table.insert(make_pair("lib/airport/vehicles/servicing/crew_car.obj", vehicle_replacement_info(atc_ServiceTruck_Crew_Car,0)));
	table.insert(make_pair("lib/airport/vehicles/servicing/crew_ferrari.obj", vehicle_replacement_info(atc_ServiceTruck_Crew_Ferrari, 0)));
	//atc_ServiceTruck_Crew_Limo
	//TODO: Waiting for art asset
	table.insert(make_pair("lib/airport/vehicles/servicing/catering_truck.obj", vehicle_replacement_info(atc_ServiceTruck_Food,0)));
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/hyd_disp_truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Liner, 0)));
	//!!Important!! - Large and Small are reversed on purpose!
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/Small_Fuel_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Jet,0)));
	table.insert(make_pair("lib/airport/Common_Elements/vehicles/Large_Fuel_Truck.obj", vehicle_replacement_info(atc_ServiceTruck_FuelTruck_Prop,0)));
	table.insert(make_pair("lib/airport/vehicles/baggage_handling/tractor.obj", vehicle_replacement_info(atc_ServiceTruck_Ground_Power_Unit,0)));
	table.insert(make_pair("lib/airport/Ramp_Equipment/GPU_1.obj", vehicle_replacement_info(atc_ServiceTruck_Ground_Power_Unit,0)));
	table.insert(make_pair("lib/airport/Ramp_Equipment/Tow_Tractor_1.obj", vehicle_replacement_info(atc_ServiceTruck_Pushback,0)));
	table.insert(make_pair("lib/airport/Ramp_Equipment/Tow_Tractor_2.obj", vehicle_replacement_info(atc_ServiceTruck_Pushback,0)));

	return table;
}

void	WED_DoReplaceVehicleObj(IResolver* resolver, WED_Airport* apt)
{
	WED_Thing * root = WED_GetWorld(resolver);
	vector<WED_ObjPlacement*> obj_placements;

	WED_Thing* collection_start = apt == NULL ? root : apt;
	CollectRecursive(collection_start, back_inserter(obj_placements), WED_ObjPlacement::sClass);

	if(!obj_placements.empty())
	{
		int replace_count = 0;
		root->StartOperation("Replace Objects");
		map<string,vehicle_replacement_info> table = build_replacement_table();

#if !TYLER_MODE
		ISelection * sel = WED_GetSelect(resolver);
		sel->Clear();
#endif
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
#if !TYLER_MODE
				sel->Insert(parking_loc);
#endif
			}
		}

		if(replace_count == 0)
		{
#if !TYLER_MODE
			sel->Clear();
#endif
			root->AbortOperation();
#if !TYLER_MODE
			DoUserAlert("Nothing to replace");
#endif
		}
		else
		{
			root->CommitOperation();
#if !TYLER_MODE
			stringstream ss;
			ss << "Replaced " << replace_count << " objects";
			DoUserAlert(ss.str().c_str());
#endif
		}
	}
#if !TYLER_MODE
	else
		DoUserAlert("Nothing to replace");
#endif
}

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
		out_ramps.push_back(static_cast<WED_RampPosition *>(who));
	}
	else if(who->GetClass() == WED_ObjPlacement::sClass)
	{
		WED_ObjPlacement * obj = static_cast<WED_ObjPlacement *>(who);
		DebugAssert(obj);
		obj_conflict_info r;
		string vpath;
		obj->GetResource(vpath);
		const XObj8 * obj8;

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
	else if(who->GetClass() == WED_Group::sClass || who->GetClass() == WED_Airport::sClass)
	{
		int nn = who->CountChildren();
		for(int n = 0; n < nn; ++n)
			collect_ramps_recursive(who->GetNthChild(n), out_ramps, out_conflicting_objs, rmgr);
	}
}

static const vector<Point2> canada {{-135,50}, {-123.3,48.2}, {-123.2,49}, {-94.5,49}, {-83.1,46}, {-81.8,43.6}, {-83.14,42.13}, {-82,41.7}, {-74.8,45},
									{-72,45}, {-68,47}, {-67,45}, {-66,42}, {-40,50}, {-73,77}, {-50,84}, {-141,84}, {-141,60}, {-135,60}, {-130,56}};

static string get_regional_codes(const Point2& loc, int ac_size, int ops_type)
{
	string code;
	
	if(ops_type == ramp_operation_Cargo)   // cargo isn't regionalized for now
	{
		return "";      // dont add FDX UPS etc - as it would prevent any 3rd party other csargo airlines to show up
	}
	
	if (loc.x() < -32.0)
	{
		code = "aal ual dal ";
		if(loc.x() < -150.0 && loc.y() > 10.0 && loc.y() < 40.0) // hawaii
		{
			if(ac_size > width_B) code += "hal swa asa ";
			else                  code += "hal fdy ";
		}
		else if(loc.y() < 13.0)
		{
			if (loc.x() > -120.0)                   // south america
				code += "tam lan glo azu ava arg ame ";
			else                                    // south pacific
				code += "";
		}
		else if(inside_polygon_pt(canada.rbegin(), canada.rend(), loc))  // canada
			code += "aca wja ";
		else
		{
			if(ac_size < width_D)
			{
				if(loc.x() < - 103.0)            // USA west
					code += "swa asa qxe ";
				else	                         // USA east
					code += "swa jbu nks ezy egf ";
			}
		}
	}
	else if(loc.x() < 60.0)
	{
		code = "baw afr klm dlh vir ";
		if(loc.x() > 37.0 && loc.y() > 12.0 && loc.y() < 34.0)    // near east
			code += "uae etd qtr ";
		else if(loc.y() > 34.0)                   // europe
		{
			code += "sas aza ibe sva ";
			if(ac_size <= width_C) 
			{
				code += "ber ryr vlg ";
				if(LonLatDistMeters(loc, Point2(11,47)) < 300e3) code += "wlc tyr lpv aua "; // within 300 km of LOWI
			}
		}
		else                                       // africa
			code += "eth saa msr ram ";
	}
	else
	{
		if(loc.y() < -10.5)                        // australia, nz
			code = "qfa anz qlk ";
		else
		{
			if(loc.x() < 86.0)                     // india
				code = "aic igo ";
			else if(loc.x() < 124.0 && loc.y() > 20.0)  // china
				code = "csn ces cca chh cxa ";
			else                                        // far east asia
				code = "lni tlm sia cpa ana jal kal gia ";
		}
	}

	return code;
}

static string get_xplane_codes(int width_enum, const set<int>& eq, int ops_type, WED_LibraryMgr* lmgr)
{
	const char *ops_str = ops_type == ramp_operation_Airline ? "lib/airport/aircraft/airliners" : "lib/airport/aircraft/cargo";
	vector<string> static_ac_vpaths;
	lmgr->GetResourceChildren(ops_str, pack_Default, static_ac_vpaths, true);
	set<string>	codes_matching_start;
	
	char width_char = width_enum - width_A + 'a';
	char width_char2 = max((char) (width_char - 1), 'a');
	
	for(auto v : static_ac_vpaths)
	{
		string s = v.substr(strlen(ops_str) + 1);
		if(eq.count(atc_Turbos) && s.find("turboprop_") == 0)
			if((s[10] == width_char || s[10] == width_char2 )&& s[11] != '.')
				codes_matching_start.insert(s.substr(12,3));
				
		if(eq.count(atc_Jets) && s.find("jet_") == 0)
			if((s[4] == width_char || s[4] == width_char2 )&& s[5] != '.')
				codes_matching_start.insert(s.substr(6,3));
				
		if(eq.count(atc_Heavies) && s.find("heavy_") == 0)
			if((s[6] == width_char || s[6] == width_char2) && s[7] != '.')
				codes_matching_start.insert(s.substr(8,3));
	}

	string out;
	for(auto c : codes_matching_start)
		out += c + " ";

	// printf("%s for size %c: %s\n", ops_str, width_char, out.c_str());
	return out;
}

int wed_upgrade_one_airport(WED_Thing* who, WED_ResourceMgr* rmgr, ISelection* sel)
{
// gMeshLines.clear();
// for(int i = 0; i < canada.size(); i++)
//		debug_mesh_line(canada[i], canada[(i+1) % canada.size()], 1,0,0,1,0,0);

	int did_work = 0;
	vector<WED_RampPosition *> ramps;
	vector<obj_conflict_info> objs;
	collect_ramps_recursive(who, ramps, objs, rmgr);

	auto lmgr = WED_GetLibraryMgr(who->GetArchive()->GetResolver());
	
	Point2 apt_loc;
	if(auto apt = dynamic_cast<WED_Airport*>(who))
	{
		Bbox2 bounds;
		apt->GetBounds(gis_Geo, bounds);
		apt_loc = bounds.centroid();
	}
	else return 0;

	srand( 100 * (apt_loc.x()+180) + 36000 * (apt_loc.y()+90) ); // for repeatable patterns per airport

	for (auto r : ramps)
	{
		if (r->GetRampOperationType() == ramp_operation_None)
		{
			// fill in ops types
			switch(r->GetType())
			{
				case atc_Ramp_Gate:
					r->SetRampOperationType(ramp_operation_Airline);
					did_work = 1;
					break;			
				case atc_Ramp_TieDown:
				{
					set<int> eq;
					r->GetEquipment(eq);

					if(eq.count(atc_Heavies))
						r->SetRampOperationType(ramp_operation_Airline);
					else
						r->SetRampOperationType(ramp_operation_GeneralAviation);
					did_work = 1;
					break;
				}
			}
		}
		// fill in airline types
		if (r->GetRampOperationType() == ramp_operation_Airline || r->GetRampOperationType() == ramp_operation_Cargo)
		{
			string old_codes =  WED_RampPosition::CorrectAirlinesString(r->GetAirlines());
			string new_codes;
			if(r->GetWidth() < width_D || rand() & 1)     // regionalize only half the large ones, as large birds roam the whole world
			{
				set<int> eq;
				r->GetEquipment(eq);
				string available_codes = get_xplane_codes(r->GetWidth(), eq, r->GetRampOperationType(), lmgr);
				string regional_codes = get_regional_codes(apt_loc,r->GetWidth(), r->GetRampOperationType());

				bool old_codes_good_enough = false;
				while (old_codes.size() >= 3)
				{
					if(available_codes.find(old_codes.substr(0,3)) != string::npos)
					{
						new_codes = old_codes;
						old_codes_good_enough = true;
						break;
					}
					new_codes += old_codes.substr(0, 3) + " ";
					old_codes.erase(0, intmin2(old_codes.size(), 4));
				}
				
				if(new_codes.empty() || !old_codes_good_enough)
					new_codes += regional_codes;
			}
			std::transform(new_codes.begin(), new_codes.end(), new_codes.begin(), [](unsigned char c) {return toupper(c);} );
			r->SetAirlines(new_codes);
			did_work = 1;
		}
	}
	// nuke static aircraft objects near ramps
	for(auto o : objs)
	{
		for(auto r : ramps)
		{
			Point2 rp; double rs;
			center_and_radius_for_ramp_start(r, rp, rs);

			if(LonLatDistMeters(rp, o.loc_ll) < (o.approx_radius_m + rs))
			{
				//debug_mesh_line(rp, o.loc_ll, 1,0,0,1,0,0);
				sel->Erase(o.obj);
				o.obj->SetParent(NULL, 0);
				o.obj->Delete();
				did_work = 1;
				break;
			}
		}
	}
	return did_work;
}

static int wed_upgrade_airports_recursive(WED_Thing * who, WED_ResourceMgr * rmgr, ISelection * sel)
{
	int did_work = 0;
	if(who->GetClass() == WED_Airport::sClass)
	{
		did_work = wed_upgrade_one_airport(who, rmgr, sel);
	}
	int nn = who->CountChildren();
	for(int n = 0; n < nn; ++n)
		if(wed_upgrade_airports_recursive(who->GetNthChild(n), rmgr, sel))
			did_work = 1;
	return did_work;

}

void WED_UpgradeRampStarts(IResolver * resolver)
{
	WED_Thing * root = WED_GetWorld(resolver);
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	root->StartCommand("Upgrade Ramp Positions");

	int did_work = wed_upgrade_airports_recursive(root, rmgr, sel);

	if (did_work)
		root->CommitOperation();
	else
		root->AbortOperation();
}

// ****** Runway Auto Rename & Move Stuff. Ugly code, its a BETA !!! *****

struct changelist_t
{
	string ICAO;
	int new_rwy;
	Point2 thr_pt0;        // the location of that runways thresholds
	Point2 thr_pt1;
	Point2 rwy_pt0;        // the location of that runways ends
	Point2 rwy_pt1;
	float  disp0, disp1;   // displacemnt distances of runways
};

#define MIN_ERROR_TO_MOVE    5.0    // average threshold location error must be over this to move the whole airport
#define PEAK_ERR_AFT_MOVE   10.0    // move needs to yield a WC threshold location error for all runways under this
#define MAX_ERROR_TO_FIND  100.0    // Both thresholds must be within this distance of the CIFP location for any runway to be considered
                                    // to be matching the one in the CIFP data. Also implies - no airport is moved any further than this.

static bool IsRwyMatching(const WED_Runway * rwy, const struct changelist_t * entry, double loc_err_allowed, vector<WED_Runway *>& rwys, vector<Vector2>& errors)
{
	// match criteria is location, only

    Point2 thr_loc0, thr_loc1;
    Point2 rwy_loc0, rwy_loc1;
	Point2 corners[4];

	if (rwy->GetSurface() >= surf_Grass) return 0;      // if its not solid data - we're not moving the airport because of it

	rwy->GetTarget()->GetLocation(gis_Geo, rwy_loc1);
	if (rwy->GetCornersDisp2(corners))
		thr_loc1 = Midpoint2(corners[0],corners[3]);
	else
		thr_loc1 = rwy_loc1;

	rwy->GetSource()->GetLocation(gis_Geo, rwy_loc0);
	if (rwy->GetCornersDisp1(corners))
		thr_loc0 = Midpoint2(corners[1],corners[2]);
	else
		thr_loc0 = rwy_loc0;

	// match the thresholds
	float thr_err0 = LonLatDistMeters(thr_loc0, entry->thr_pt0);
	float thr_err1 = LonLatDistMeters(thr_loc1, entry->thr_pt1);
	// the runway end could be listed in the changelist in reverse order, so test the other way round as well
	float thr_err0r = LonLatDistMeters(thr_loc0, entry->thr_pt1);
	float thr_err1r = LonLatDistMeters(thr_loc1, entry->thr_pt0);

	// match the rwy ends
	float end_err0 = LonLatDistMeters(rwy_loc0, entry->rwy_pt0);
	float end_err1 = LonLatDistMeters(rwy_loc1, entry->rwy_pt1);
	float end_err0r = LonLatDistMeters(rwy_loc0, entry->rwy_pt1);
	float end_err1r = LonLatDistMeters(rwy_loc1, entry->rwy_pt0);

	float good_match = 10.0;

	// see if the thrsholds fit well
	if(thr_err0 < good_match && thr_err1 < good_match)
	{
		errors.push_back(Vector2(thr_loc0, entry->thr_pt0));
		errors.push_back(Vector2(thr_loc1, entry->thr_pt1));
		return true;
	}
	if(thr_err0r < good_match && thr_err1r < good_match)
	{
		errors.push_back(Vector2(thr_loc0, entry->thr_pt1));
		errors.push_back(Vector2(thr_loc1, entry->thr_pt0));
		return true;
	}

	// next try the runway ends
	if(end_err0 < good_match && end_err1 < good_match)
	{
		// ends match, thresholds not => displacemnt is set wrong in scenery. Could fix this right here and now ?
		printf("      NOT fixing wrong displaced threshold entry for below apt\n");
		errors.push_back(Vector2(rwy_loc0, entry->rwy_pt0));
		errors.push_back(Vector2(rwy_loc1, entry->rwy_pt1));
		return true;
	}
	if(end_err0r < good_match && end_err1r < good_match)
	{
		printf("      NOT fixing wrong displaced threshold entry for below apt\n");
		errors.push_back(Vector2(rwy_loc0, entry->rwy_pt1));
		errors.push_back(Vector2(rwy_loc1, entry->rwy_pt0));
		return true;
	}

	// if there is no good match for either, go by the thresholds, be as lenient as allowed
	if(thr_err0 < loc_err_allowed && thr_err1 < loc_err_allowed)
	{
		errors.push_back(Vector2(thr_loc0, entry->thr_pt0));
		errors.push_back(Vector2(thr_loc1, entry->thr_pt1));
		return true;
	}
	if(thr_err0r < loc_err_allowed && thr_err1r < loc_err_allowed)
	{
		errors.push_back(Vector2(thr_loc0, entry->thr_pt1));
		errors.push_back(Vector2(thr_loc1, entry->thr_pt0));
		return true;
	}

	return false;
}


void WED_AlignAirports(IResolver * resolver)
{
	vector<struct changelist_t> changelist;

	char fn[200];
	if (!GetFilePathFromUser(getFile_Open,"Pick file with runway coordinates", "Start processing",
								FILE_DIALOG_PICK_VALID_RUNWAY_TABLE, fn,sizeof(fn)))
		return;

	MFMemFile * mf = MemFile_Open(fn);

	if (mf)
	{
		MFScanner	s;
		MFS_init(&s, mf);
		// skip first line, so Tyler can put a comment/version in there
		MFS_string_eol(&s,NULL);

		bool second_end = false;
		string first_rwy;
		string icao, rnam;
		struct changelist_t entry;
		double disp0;

		int lnum=1;

		while(!MFS_done(&s))
		{
			MFS_string(&s,&icao);
			MFS_string(&s,&rnam);
			double lat = MFS_double(&s);
			double lon = MFS_double(&s);
			double disp= MFS_double(&s);

			if (second_end)
			{
				if (entry.ICAO == icao)
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
					{
//						printf("Read changelist: Using Lon %.6lf Lat %.6lf, Lon %.6lf Lat %.6lf\n",entry.rwy_pt0.x(),entry.rwy_pt0.y(),entry.rwy_pt1.x(),entry.rwy_pt1.y());
						float rwy_len_cifp = LonLatDistMeters(entry.rwy_pt0, entry.rwy_pt1);
						entry.thr_pt0 = entry.rwy_pt0 + Vector2(entry.rwy_pt0, entry.rwy_pt1) / rwy_len_cifp * disp0;
						entry.thr_pt1 = entry.rwy_pt1 + Vector2(entry.rwy_pt1, entry.rwy_pt0) / rwy_len_cifp * disp;
//						printf("Read changelist: Added Lat %.6lf Lat %.6lf, Lon %.6lf Lat %.6lf\n",entry.thr_pt0.x(),entry.thr_pt0.y(),entry.thr_pt1.x(),entry.thr_pt1.y());

						changelist.push_back(entry);
					}
					else
						printf("Read changelist: Ignoring bad rwy spec pair %s ending in line %d\n",rwy_2wy.c_str(),lnum);

					second_end = false;
				}
				else  // we got out of sync. Let see if we can recover.
				{
					printf("Read changelist: ICAO in lines %d and %d do not match, ignoring line %d\n",lnum-1, lnum, lnum-1);
					entry.ICAO = icao;
					first_rwy = rnam;
					entry.rwy_pt0 = Point2(lon,lat);
					disp0 = disp;
				}
			}
			else
			{
				entry.ICAO = icao;
				first_rwy = rnam;
				entry.rwy_pt0 = Point2(lon,lat);
				disp0 = disp;
				second_end = true;
			}
			lnum++;
			MFS_string_eol(&s,NULL);
		}
		MemFile_Close(mf);
		printf("Read changelist completed, found %d valid runway definitions\n", (int) changelist.size());
	}

	if (!changelist.empty())
	{
		WED_Thing * wrl = WED_GetWorld(resolver);
		ISelection * sel = WED_GetSelect(resolver);

		vector<WED_Airport *> apts;
		int renamed_count = 0;
		int moved_count = 0;
		int unchanged_count = 0;

		CollectRecursiveNoNesting(wrl, back_inserter(apts),WED_Airport::sClass);

		wrl->StartCommand("Align Airports");
		sel->Clear();

		for(vector<WED_Airport *>::iterator apt = apts.begin(); apt !=apts.end(); ++apt)
		{
			vector<WED_Runway *> rwys;
			vector<Vector2> coord_errors;
			bool apt_changed = false;
			string thisICAO; (*apt)->GetICAO(thisICAO);

			CollectRecursive(*apt, back_inserter(rwys),  WED_Runway::sClass);

			// first look if its a real close location match. If so, take it, regardless of name match, as we assume its just misnamed
			float search_radius = MAX_ERROR_TO_FIND;

			// determine the closest runway spacing at this airport
			if (rwys.size() > 1)
				for(vector<WED_Runway *>::iterator i = rwys.begin(); i != rwys.end(); ++i)
				{
					Point2 i0,i1;
					(*i)->GetSource()->GetLocation(gis_Geo, i0);
					(*i)->GetTarget()->GetLocation(gis_Geo, i1);
					for(vector<WED_Runway *>::iterator j = i+1; j != rwys.end(); ++j)
					{
						Point2 j0,j1;
						(*j)->GetSource()->GetLocation(gis_Geo, j0);
						(*j)->GetTarget()->GetLocation(gis_Geo, j1);

						float d1 = LonLatDistMeters(i0,j0);
						float d2 = LonLatDistMeters(i0,j1);
						float d3 = LonLatDistMeters(i1,j0);
						float d4 = LonLatDistMeters(i1,j1);
						float min_dist = fltmin4(d1,d2,d3,d4) - 0.1;
						search_radius = min(search_radius, min_dist);
					}
				}

			for(vector<WED_Runway *>::iterator r = rwys.begin(); r != rwys.end(); ++r)
			{
				int old_rwy_enum = (*r)->GetRunwayEnumsTwoway();
				for(vector<struct changelist_t>::iterator cl_entry = changelist.begin(); cl_entry != changelist.end(); ++cl_entry)
				{
					if(cl_entry->ICAO == thisICAO)
					{
						// see if we have runways that roughly match those locations
						// printf("Testing %s against %s at %s\n",ENUM_Desc(old_rwy_enum), ENUM_Desc((*c).new_rwy),thisICAO.c_str());

						if (IsRwyMatching(*r, &(*cl_entry), search_radius, rwys, coord_errors))
						{
							if(cl_entry->new_rwy != old_rwy_enum)
							{
								printf("%s: NOT renaming Rwy %s to %s\n",thisICAO.c_str(),ENUM_Desc(old_rwy_enum),ENUM_Desc(cl_entry->new_rwy));
//								(*r)->SetName(ENUM_Desc(cl_entry->new_rwy));
//								renamed_count++;
//								apt_changed = true;
							}
						}
					}
				}
			}
			if (!coord_errors.empty())
			{
				printf("%s: radius %3.0lfm matched %d/%d runways: ",thisICAO.c_str(), search_radius, (int) coord_errors.size()/2, (int) rwys.size());

				// check if we could make the runway coordinate errors smaller if we were to move the whole airport
				Vector2 avg_errVec;
				Bbox2 old_apt_pos;
				(*apt)->GetBounds(gis_Geo, old_apt_pos);

				for(vector<Vector2>::iterator i = coord_errors.begin(); i != coord_errors.end(); ++i)
				{
					avg_errVec += *i;
//					printf("threshold error = ~%.1lf EW ~%.1lf NS\n", 1.1e5*i->x(), 0.8e5*i->y());
				}
				avg_errVec /= coord_errors.size();

				float peak_errDist_aft = 0.0, peak_errDist = 0.0;

				for(vector<Vector2>::iterator i = coord_errors.begin(); i != coord_errors.end(); ++i)
				{
					float loc_errDist = LonLatDistMeters(old_apt_pos.centroid(), old_apt_pos.centroid() + *i);    // sqrt(i->squared_length());
					(*i) -= avg_errVec;
					float loc_errDist_aft = LonLatDistMeters(old_apt_pos.centroid(), old_apt_pos.centroid() + *i);
//					printf("thr errDist %.1lf fixed to %.1lf\n", loc_errDist, loc_errDist_aft);

					peak_errDist_aft = max(peak_errDist_aft,loc_errDist_aft);
					peak_errDist     = max(peak_errDist,    loc_errDist    );
				}

				float avg_errDist = LonLatDistMeters(old_apt_pos.centroid(), old_apt_pos.centroid() + avg_errVec);

				if(avg_errDist > MIN_ERROR_TO_MOVE && peak_errDist_aft < PEAK_ERR_AFT_MOVE)
				{
					printf("Moving ~%.1lf East ~%.1lf North, WC error b4/aft move %.1lf / %.1lfm \n", 1.1e5*avg_errVec.x(), 0.8e5*avg_errVec.y(), peak_errDist, peak_errDist_aft);

					Bbox2 new_pos(old_apt_pos);
					new_pos += avg_errVec;
					(*apt)->Rescale(gis_Geo, old_apt_pos, new_pos);

					moved_count++;
					apt_changed = true;
				}
				else
					printf("NOT moving, avg err %.1lfm, WC error b4/aft move %.1lf / %.1lfm \n",avg_errDist, peak_errDist, peak_errDist_aft);
			}
			if(!apt_changed)
			{
				sel->Insert(*apt);   // selects all unchanged airports. So they can be deleted and whats left re-submitted to the gateway ...
				unchanged_count++;
			}
		}
//		if(renamed_count || moved_count)
			wrl->CommitOperation();
//		else
//			wrl->AbortOperation();

		stringstream ss;
		ss << "Renamed " << renamed_count << " runways.\n";
		ss << "Moved " << moved_count << " airports.\n\n" << unchanged_count << " airports with *NO* changes have been selected.";
		DoUserAlert(ss.str().c_str());
	}
}

static void dummy_func(void* ref, const char* fmt, ...) { return; }

int WED_DoConvertToJW(WED_Airport* apt, int statistics[4])
{
	vector<WED_RampPosition*> ramps;
	vector<WED_ObjPlacement*> all_objects, jw_tun, jw_ext;
	vector<WED_FacadePlacement*> jw_facs;
	WED_ResourceMgr* rmgr = WED_GetResourceMgr(apt->GetArchive()->GetResolver());

	CollectRecursive(apt, back_inserter(ramps), ThingNotHidden, TakeAlways, WED_RampPosition::sClass);
	CollectRecursive(apt, back_inserter(all_objects), ThingNotHidden, TakeAlways, WED_ObjPlacement::sClass);
	CollectRecursive(apt, back_inserter(jw_facs), ThingNotHidden, [&](WED_Thing* v)
		{
			if (auto f = dynamic_cast<WED_FacadePlacement*>(v))
				return f->IsJetway();
			else
				return false;
		}, WED_FacadePlacement::sClass);

	int obj2JW_count = 0;
	for (auto o : all_objects)
	{
		string res;
		o->GetResource(res);
		if (res.compare(0, strlen("lib/airport/Ramp_Equipment/Jetway_"), "lib/airport/Ramp_Equipment/Jetway_") == 0)
			jw_tun.push_back(o);
		else if (res.compare(0, strlen("lib/airport/Ramp_Equipment/JetWayEx"), "lib/airport/Ramp_Equipment/JetWayEx") == 0)
			jw_ext.push_back(o);
		else if (res == "lib/airport/Ramp_Equipment/250cm_Jetway_Group.agp" ||
				 res == "lib/airport/Ramp_Equipment/400cm_Jetway_2.agp" ||
				 res == "lib/airport/Ramp_Equipment/400cm_Jetway_3.agp" ||
				 res == "lib/airport/Ramp_Equipment/400cm_Jetway_Group.agp" ||
				 res == "lib/airport/Ramp_Equipment/500cm_Jetway_Group.agp")
		{
			vector<WED_ObjPlacement*> added_objs;
			const agp_t* agp_info;
			if (rmgr->GetAGP(res, agp_info))
			{
				replace_all_obj_in_agp(o, agp_info, apt->GetArchive(), added_objs);
				o->SetParent(NULL, 0);
				o->Delete();
			}
			for (auto ao : added_objs)
			{
				ao->GetResource(res);
				if (res.compare(0, strlen("lib/airport/Ramp_Equipment/Jetway_"), "lib/airport/Ramp_Equipment/Jetway_") == 0)
				{
					jw_tun.push_back(ao);
					break;
				}
			}
		}
	}

	// determine the position in front of the cab where the A/C is expected to be parked and the nearest ramp start to each.
	// then find all extensions leading up to them
	if(ramps.size() > 0 && jw_tun.size() > 0)
	{
		for (auto c : jw_tun)
		{
			Point2 acf_pos, tun_pos, jw_pos;
			Vector2 tun_dir;
			c->GetLocation(gis_Geo, jw_pos);
			string res;
			c->GetResource(res);
			double tun_len = res[strlen("lib/airport/Ramp_Equipment/Jetway_")] == '5' ? 20 : 15;
			double tun_hdg = c->GetHeading();
			NorthHeading2VectorDegs(tun_pos, tun_pos, tun_hdg, tun_dir);
			tun_pos = jw_pos + tun_dir * 2.7 * MTR_TO_DEG_LAT;

			NorthHeading2VectorDegs(tun_pos, tun_pos, tun_hdg - 30.0, tun_dir);  // hdg to place in front of cabin where the acf would be
			tun_dir *= (tun_len + 2.0) * MTR_TO_DEG_LAT;
			acf_pos = tun_pos + tun_dir;

			double min_dist = 99999.0;
			WED_RampPosition* closest_ramp = nullptr;
			for(auto r : ramps)
			{
				Point2 pt;
				r->GetLocation(gis_Geo, pt);
				double d = LonLatDistMeters(pt, acf_pos);
				if (d < min_dist)
				{
					min_dist = d;
					closest_ramp = r;
				}
			}
			if (min_dist < 15.0)  // only convert jetway objects into facades that are somewhat close to an actual ramp start
			{
				auto fac = WED_FacadePlacement::CreateTyped(apt->GetArchive());
				fac->SetParent(c->GetParent(), c->GetMyPosition());
				string nam;
				c->GetName(nam);
				fac->SetName(nam + " (conv)");

				Jetway_t jw_info;
				jw_info.size_code = tun_len < 20.0 ? 1 : 2;
				jw_info.style_code = 1;
				jw_info.location = tun_pos;
				jw_info.parked_tunnel_length = tun_len;
				jw_info.parked_tunnel_heading = tun_hdg - 21.0;           // exact tunnel heading plus pulled back a bit to ensure cabin clearance
				jw_info.parked_cab_heading = jw_info.parked_tunnel_heading - 65.0;
				fac->WED_FacadePlacement::ImportJetway(jw_info, dummy_func, nullptr);
				
				auto rng = fac->GetNthChild(0);

				c->SetParent(NULL, 0);
				c->Delete();

				bool is_extended = false;
				for (auto e = jw_ext.begin(); e != jw_ext.end(); e++)
				{
					// those extension can be placed either way around, so check both ends for proximity to our jetway base
					Point2 p1, p2;
					(*e)->GetLocation(gis_Geo, p1);
					double hdg = (*e)->GetHeading();
					string ext_nam;
					(*e)->GetResource(ext_nam);
					double len;
					int pos = strlen("lib/airport/Ramp_Equipment/JetWayEx");
					if(ext_nam[pos] == 't') pos++;
					sscanf(ext_nam.c_str() + pos + 1, "%lf", &len);
					Vector2 ext_dir;
					NorthHeading2VectorDegs(p1, p1, hdg, ext_dir);
					p2 = p1 + ext_dir * (len + 2.0) * MTR_TO_DEG_LAT;
					double d1 = LonLatDistMeters(p1, tun_pos);
					double d2 = LonLatDistMeters(p2, tun_pos);
					if (d1 < d2)
					{
						p1 = p2;
						d2 = d1;
					}
					if (d2 < 3.0)
					{
						auto p = WED_FacadeNode::CreateTyped(apt->GetArchive());
						p->SetParent(rng, 0);
						p->SetLocation(gis_Geo, p1);

						(*e)->SetParent(NULL, 0);
						(*e)->Delete();
						is_extended = true;
						jw_ext.erase(e);
						break;
					}
				}
				if (!is_extended)
				{
					auto p = WED_FacadeNode::CreateTyped(apt->GetArchive());
					p->SetParent(rng, 0);
					p->SetLocation(gis_Geo, jw_pos);
				}
				jw_facs.push_back(fac);
				obj2JW_count++;
			}
		}
	}

	int JW_longer = 0, JW_shorter = 0, JW_inactive = 0;
	for(auto r : ramps)
	{
		Point2 ramp_loc;
		struct jw_info {
			WED_FacadePlacement* f;
			Point2 cabin_loc, tunnel_orig;
			double cabin_dist;
			IGISPointSequence* ps;
			int last_pt;
		};
		vector<struct jw_info> jw_serving_us;

		r->GetLocation(gis_Geo, ramp_loc);
		for (auto f : jw_facs)
		{
			// find ALL close jw that face us, i.e. are intended to serve this ramp.
			if (f->HasDockingCabin())
			{
				jw_info jw;
				jw.f = f;
				jw.ps = jw.f->GetOuterRing();
				jw.last_pt = jw.ps->GetNumPoints() - 1;
				jw.ps->GetNthPoint(jw.last_pt - 1)->GetLocation(gis_Geo, jw.cabin_loc);
				jw.ps->GetNthPoint(jw.last_pt - 2)->GetLocation(gis_Geo, jw.tunnel_orig);
				jw.cabin_dist = LonLatDistMeters(jw.cabin_loc, ramp_loc);
				if (jw.cabin_dist <= 25.0)
				{
					double door_hdg = VectorDegs2NorthHeading(jw.tunnel_orig, jw.tunnel_orig, Vector2(jw.tunnel_orig, ramp_loc));
					double tun_hdg = VectorDegs2NorthHeading(jw.tunnel_orig, jw.tunnel_orig, Vector2(jw.tunnel_orig, jw.cabin_loc));
					double rel_angle = dobwrap(tun_hdg - door_hdg, 0, 360);
					if (rel_angle < 90.0 || rel_angle > 340.0)
						jw_serving_us.push_back(jw);
				}
			}
		}

		if (jw_serving_us.size() > 1)
		{
			// figure which one can most freely reach ramp, make all others non-docking
			double closest_cabin_dist = 999;
			jw_info closest_jw;
			for(auto jw : jw_serving_us)
			{
				if (jw.cabin_dist < closest_cabin_dist)
				{
					closest_cabin_dist = jw.cabin_dist;
					closest_jw = jw;
				}
			}
			if (closest_cabin_dist < 100)
			{
				for (auto jw : jw_serving_us)
				{
					if (jw.f != closest_jw.f)
					{
						auto last_node = dynamic_cast<WED_FacadeNode*>(jw.ps->GetNthPoint(jw.last_pt));
						last_node->SetWallType(39);
						JW_inactive++;
					}
				}
				jw_serving_us.clear();
				jw_serving_us.push_back(closest_jw);
			}
		}

		if(jw_serving_us.size() > 0)
		{
			// check if tunnel length is suitable to reach ramp with some margin for actual door locations
			string res;
			const fac_info_t* info;
			jw_serving_us[0].f->GetResource(res);
			if (rmgr->GetFac(res, info))
			{
				auto tun_node = dynamic_cast<WED_FacadeNode*>(jw_serving_us[0].ps->GetNthPoint(jw_serving_us[0].last_pt - 2));
				double tun_dist = LonLatDistMeters(jw_serving_us[0].tunnel_orig, ramp_loc);
				int wall;
				wall = tun_node->GetWallType();

				bool tunnel_is_short = false;
				bool tunnel_is_long = false;
				for (auto t : info->tunnels)
					if (wall == t.idx)
					{
						double tun_len = LonLatDistMeters(jw_serving_us[0].cabin_loc, jw_serving_us[0].tunnel_orig);
						switch (t.size_code)        // deliberately test for shorter range - allows some margin for actual cabin door locations
						{
						case 1:	tunnel_is_short = tun_dist > 21.0; 
								break;
						case 2:	tunnel_is_short = tun_dist > 26.0; 
								tunnel_is_long = tun_len < 14.0 || tun_dist < 19.0;
								break;
						case 3:	tunnel_is_short = tun_dist > 36.0; 
								tunnel_is_long = tun_len < 17.0 || tun_dist < 22.0;
								break;
						case 4:	// tunnel_is_short = tun_dist > 40.0; break; // would have to move the tunnel base !! to make it reach further.
								tunnel_is_long = tun_len < 20.0 || tun_dist < 25.0;
								break;
						}
						if (tunnel_is_short)
							for (auto t_longer : info->tunnels)
							{
								if (t_longer.size_code == t.size_code + 1)
								{
									tun_node->SetWallType(t_longer.idx);
									JW_longer++;
									break;
								}
							}
						else if (tunnel_is_long)
							for (auto t_shorter : info->tunnels)
							{
								if (t_shorter.size_code == t.size_code - 1)
								{
									tun_node->SetWallType(t_shorter.idx);
									JW_shorter++;
									break;
								}
							}
						break;
					}
			}
		}
	}
	if (statistics)
	{
		*statistics++ += obj2JW_count;
		*statistics++ += JW_longer;
		*statistics++ += JW_shorter;
		*statistics++ += JW_inactive;
	}
	return obj2JW_count + JW_longer + JW_shorter + JW_inactive;
}


void WED_UpgradeJetways(IResolver* resolver)
{
	WED_Thing* wrl = WED_GetWorld(resolver);
	vector<WED_Airport *> all_apts;
	int changes = 0, statistics[4] = { 0 };

	CollectRecursiveNoNesting(wrl, back_inserter(all_apts), WED_Airport::sClass);

	wrl->StartOperation("Upgrade Jetways");
	for (auto a : all_apts)
		changes += WED_DoConvertToJW(a, statistics);

	if (changes > 0)
	{
		wrl->CommitOperation();
		string msg("Created ");
		msg += to_string(statistics[0]) + " JW facades from JW objects\n";
		msg += to_string(statistics[1]) + " tunnels made longer to reach A/C\n";
		msg += to_string(statistics[2]) + " tunnels made shorter to reach A/C\n";
		msg += to_string(statistics[3]) + " set non-docking to avoid conflicts at ramps reached by multiple JW";
		DoUserAlert(msg.c_str());
	}
	else
		wrl->AbortOperation();
}

static int get_aged_surf(int surf, int age)
{
	if (surf == surf_Concrete)
		return age == 1 ? surf_Concrete_8 : surf_Concrete_1;
	else
		return age == 1 ? surf_Asphalt_4 : surf_Asphalt_12;
}

int WED_DoAgePavement(WED_Airport* apt, int age)  // age 1 = older
{
	vector<WED_Runway*> rwys;
	vector<WED_Taxiway*> twys;
	vector<WED_PolygonPlacement*> pols;

	int changes = 0;

	CollectRecursive(apt, back_inserter(rwys));
	CollectRecursive(apt, back_inserter(twys));
	CollectRecursive(apt, back_inserter(pols));

	for (auto r : rwys)
	{
		int surf = r->GetSurface();
		if (surf == surf_Asphalt || surf == surf_Concrete)
		{
			r->SetSurface(get_aged_surf(surf, age));
			changes++;
		}

		surf = r->GetShoulder();
		if (surf == surf_Asphalt || surf == surf_Concrete)
		{
			r->SetShoulder(get_aged_surf(surf, age));
			changes++;
		}
	}

	for (auto t : twys)
	{
		int surf = t->GetSurface();
		if (surf == surf_Asphalt || surf == surf_Concrete)
		{
			t->SetSurface(get_aged_surf(surf, age));
			changes++;
		}
	}

	for (auto p : pols) // thats a pretty basic upgrade, any lighter/darker than default pavements are NOT converted
	{
		string res;
		p->GetResource(res);
		if (res == "lib/airport/pavement/asphalt_3D.pol" || "lib/airport/pavements/Concrete_1D.pol")
		{
			int surf = surf_Asphalt;
			if (res.find("Concrete)") != string::npos) surf = surf_Concrete;
			surf = get_aged_surf(surf, age);
			WED_GetLibraryMgr(p->GetArchive()->GetResolver())->GetSurfVpath(surf, res);
			p->SetResource(res);
			changes++;
		}
	}
	return changes;
}

void WED_AgePavement(IResolver* resolver)
{
	WED_Thing* wrl = WED_GetWorld(resolver);
	vector<WED_Airport*> all_apts;
	int count = 0;

	int age = ConfirmMessage("Change all X-Plane 11 default Pavement to X-Plane 12 old/worn ? Otherwise change is to newer looking pavement.", "Yes", "No");

	CollectRecursiveNoNesting(wrl, back_inserter(all_apts), WED_Airport::sClass);

	wrl->StartOperation("Age Pavement");
	for (auto a : all_apts)
		count += WED_DoAgePavement(a, age);
	if (count > 0)
	{
		wrl->CommitOperation();
		string msg("Converted ");
		msg += to_string(count) + " items changed";
		DoUserAlert(msg.c_str());
	}
	else
		wrl->AbortOperation();
}

static vector<WED_PolygonPlacement *> PolygonsForWED_Polygon(WED_Thing * parent, const vector<Polygon2>& poly)
{
	vector<WED_PolygonPlacement *> mpol;
	WED_Archive * arch = parent->GetArchive();
	
//	printf("conv size %d\n", poly.size());
	
	for(const auto& p : poly)
	{
		if(p.is_ccw())
		{
			mpol.push_back(WED_PolygonPlacement::CreateTyped(arch));
			mpol.back()->SetParent(parent, 0);
/*			printf("CCW %d\n", p.size());
			for(int i = 0; i < p.size(); i++)
				debug_mesh_segment(p.side(i), 1,0,0, 1,0,0);
		}
		else
		{
			printf("CW %d\n", p.size());
			for(int i = 0; i < p.size(); i++)
				debug_mesh_segment(p.side(i), 0,0,1, 0,0,1);
*/		}
		if(mpol.size() == 0) 
			continue;

		DebugAssert(mpol.size() > 0);
		auto ring = WED_Ring::CreateTyped(arch);
		ring->SetParent(mpol.back(), mpol.back()->CountChildren());
		
		for(int i = 0; i < p.size(); i++)
		{
			auto n = WED_SimpleBezierBoundaryNode::CreateTyped(arch);
			n->SetParent(ring, i);
			n->SetLocation(gis_Geo, p[i]);
			n->SetName(string("Node ") + to_string(i));
		}
	}
	return mpol;
}

static bool inside_pt(const vector<Polygon2>& vec_poly, const Point2 pt)
{
	int inside = 0;
	for(const auto& p : vec_poly)
	{
		if(p.size())
		if(p.inside(pt))
			if(p.is_ccw())
				inside++;
			else
				inside--;
	}
	return inside > 0;
}

static void make_ter_FX_exist(WED_Group** grp, WED_Thing* parent)
{
	if(!*grp)
	{
		*grp = WED_Group::CreateTyped(parent->GetArchive());
		(*grp)->SetParent(parent, 0);
		(*grp)->SetName("Terrain FX");
	}
}

class coord_translator {
public:
	coord_translator(double ref_lat, double heading)
	{
		cos_lat = cos(ref_lat * DEG_TO_RAD);
		cos_hdg = cos(heading * DEG_TO_RAD);
		sin_hdg = sin(heading * DEG_TO_RAD);
	}
	Point2 to_uv(Point2 ll) const
	{
		double lon = ll.x() * cos_lat;
		double lat = ll.y();
		return { lon * cos_hdg - lat * sin_hdg , lon* sin_hdg + lat * cos_hdg };
	}
	Point2 to_ll(Point2 uv) const
	{
		double u =  uv.x() * cos_hdg + uv.y() * sin_hdg;
		double v = -uv.x() * sin_hdg + uv.y() * cos_hdg;
		return { u / cos_lat, v };
	}

private:
	double cos_hdg, sin_hdg, cos_lat;
};

bool WED_DoMowGrass(WED_Airport* apt, int statistics[4])
{
	// gMeshLines.clear();

	Bbox2 bounds;
	apt->GetBounds(gis_Geo, bounds);
	Point2 apt_loc = bounds.centroid();
	srand( 100 * (apt_loc.x()+180) + 36000 * (apt_loc.y()+90) ); // for repeatable patterns per airport

	vector<WED_Runway*> rwys;
	vector<WED_Taxiway*> twys;
	vector<WED_PolygonPlacement*> polys;
	vector<WED_AirportBoundary*> bdys;

	typedef vector<Polygon2> vPoly2;
	vPoly2 apt_boundary, all_grass_poly, all_pave_poly;

	vector<pair<vPoly2, double> > grass;
	
	WED_LibraryMgr* lmgr = WED_GetLibraryMgr(apt->GetArchive()->GetResolver());
	WED_Group * art_grp = nullptr;
	
	CollectRecursive(apt, back_inserter(bdys), WED_AirportBoundary::sClass);
	for(auto b : bdys)
		WED_BezierPolygonWithHolesForPolygon(b, apt_boundary);
	if(apt_boundary.size() == 0) return 0;
	
	CollectRecursive(apt, back_inserter(rwys), WED_Runway::sClass);
    std::sort(rwys.begin(), rwys.end(), [&](WED_Runway* a, WED_Runway* b)   // mow largest runway first, so most of the moving is aligned with this one

		{
			return a->GetWidth() * a->GetLength() > b->GetWidth() * b->GetLength();
		});

	// create grass patches aligned with airports
	for(auto r: rwys)
	{
		if (r->GetSurface() > surf_Grass) continue;

		Point2 	tmp[8];
		r->GetCornersShoulders(tmp);
		tmp[2] = tmp[6];
		tmp[3] = tmp[7];

		if (r->GetSurface() < surf_Grass)
		{
			all_pave_poly.push_back(Polygon2());
			for(int i = 3; i >= 0; i--)
				all_pave_poly.back().push_back(tmp[i]);
		}

		Vector2 len_vec_1m = Vector2(tmp[0], tmp[1]) / LonLatDistMeters(tmp[0], tmp[1]);
		Vector2 len_ext  = len_vec_1m * 400.0;
		Vector2 wid_vec_1m = Vector2(tmp[1], tmp[2]) / LonLatDistMeters(tmp[1], tmp[2]);
		Vector2 side_ext = wid_vec_1m * r->GetWidth() * (r->GetSurface() == surf_Grass ? 1.0 : 4.0);
		tmp[0] -= len_ext + side_ext;
		tmp[1] += len_ext - side_ext;
		tmp[2] += len_ext + side_ext;
		tmp[3] -= len_ext - side_ext;

		grass.push_back(make_pair(vPoly2(), r->GetHeading()));
		vPoly2 * this_grass = &grass.back().first;

		this_grass->push_back(Polygon2());
		for(int i = 3; i >= 0; i--)
			this_grass->back().push_back(tmp[i]);

		vector<Polygon2> tmp_poly = PolygonCut(apt_boundary, all_grass_poly);
		*this_grass = PolygonIntersect(*this_grass, tmp_poly);
		if(this_grass->empty())
		{
			grass.pop_back();
			continue;
		}

		make_ter_FX_exist(&art_grp, apt);
		auto new_p = PolygonsForWED_Polygon(art_grp, *this_grass);
		if(statistics) statistics[0] += new_p.size();
		
		string nam;
		r->GetName(nam);
		nam = string("Mowing along ") + nam;
		for(auto p: new_p)
		{
			p->SetName(nam);
			p->SetHeading(r->GetHeading());
			p->SetResource("lib/airport/ground/terrain_FX/lawn_tracks/area_2.pol");
		}
		all_grass_poly = PolygonUnion(all_grass_poly, *this_grass);
	}
	
	// get all pavement
	CollectRecursive(apt, back_inserter(twys), ThingNotHidden, [&](WED_Thing* v)
		{
			if (auto p = dynamic_cast<WED_Taxiway*>(v))
			{
				if(p->GetSurface() <  surf_Grass)
					return true;
			}
			return false;
		}, WED_Taxiway::sClass);
	CollectRecursive(apt, back_inserter(polys), ThingNotHidden, [&](WED_Thing* v)
		{
			if (auto p = dynamic_cast<WED_PolygonPlacement*>(v))
			{
				string res;
				p->GetName(res);
				p->GetResource(res);
				if(res.compare(0, strlen("lib/airport/pavement/"),"lib/airport/pavement/") == 0) 
					return true;
				auto surf = lmgr->GetSurfEnum(res);
				return surf > 0;
			}
			else
				return false;
		}, WED_PolygonPlacement::sClass);

	for(auto t : twys)
		WED_BezierPolygonWithHolesForPolygon(t, all_pave_poly);
	for(auto p : polys)
		WED_BezierPolygonWithHolesForPolygon(p, all_pave_poly);
	
	all_pave_poly = PolygonUnion(all_pave_poly, vector<Polygon2>());
	// from here only we can assume 'flat' topology: No overlapping windings, no nested holes.

	// turning circles where mowing lines hit pavement
	for (auto& g : grass)
	{
		coord_translator tr(apt_loc.y(), g.second);
		Bbox2 bb;
		for (auto pol : g.first)
			for (auto pt : pol)
				bb += tr.to_uv(pt);

		//debug_mesh_segment({tr.to_ll(bb.top_left()), tr.to_ll(bb.top_right())}, 1, 0, 0, 1, 0, 0);
		//debug_mesh_segment({tr.to_ll(bb.bottom_left()), tr.to_ll(bb.bottom_right())}, 1, 0, 0, 1, 0, 0);

		// the algo is stupid - so we have to speed up the polygon testing a bit by clipping the pavement to the area that might matter for us
		vPoly2 clip;
		clip.push_back(Polygon2());
		clip.back().push_back(tr.to_ll(bb.bottom_left()));
		clip.back().push_back(tr.to_ll(bb.bottom_right()));
		clip.back().push_back(tr.to_ll(bb.top_right()));
		clip.back().push_back(tr.to_ll(bb.top_left()));
		auto ap_poly = PolygonIntersect(all_pave_poly, clip);
		
/*		for (auto p : ap_poly)
		for(int i = 0; i < p.size(); i++)
			debug_mesh_segment(p.side(i), 0,0,1, 0,0,1);
*/
		Point2 start_mow(tr.to_ll(bb.bottom_left()));
		Vector2 this_row_dir(start_mow, tr.to_ll(bb.top_left()));
		double mowing_length = LonLatDistMeters(start_mow, tr.to_ll(bb.top_left()));
		this_row_dir /= mowing_length;

		Vector2 next_row_dir(start_mow, tr.to_ll(bb.bottom_right()));
		double next_row_length = LonLatDistMeters(start_mow, tr.to_ll(bb.bottom_right()));
		next_row_dir /= next_row_length;

		const double mow_steps = 4.0;
		const double row_spacing = 12.0;
		const double test_radius = 8.0;

		bool on_pave(false);
		bool on_grass(false);
		for (int v = next_row_length / row_spacing - 1; v >= 0; v--)
		{
			Vector2 mowing_dir = this_row_dir;
			Point2 pt(start_mow + next_row_dir * row_spacing * (0.5 + v));
			for (int u = mowing_length / mow_steps; u > 0; u--)
			{
				bool test_fwd = !on_pave;
				auto test_dir = test_fwd ? mowing_dir : -mowing_dir;
				bool near_pave = inside_pt(ap_poly, pt + test_dir * test_radius);
				if (!near_pave) near_pave = inside_pt(ap_poly, pt + (test_dir + next_row_dir) * test_radius * 0.707);
				if (!near_pave) near_pave = inside_pt(ap_poly, pt + (test_dir - next_row_dir) * test_radius * 0.707);
				bool closing_on_pave = false;
				if (!near_pave)       closing_on_pave = inside_pt(ap_poly, pt + next_row_dir * test_radius);
				if (!closing_on_pave) closing_on_pave = inside_pt(ap_poly, pt - next_row_dir * test_radius);
				near_pave |= closing_on_pave;

				bool now_on_grass = inside_pt(g.first, pt);
				bool grass_bdy = now_on_grass != on_grass;
				if (grass_bdy) on_grass = !on_grass;

				if (now_on_grass || grass_bdy)
				{
					//debug_mesh_segment({ pt, pt + next_row_dir * 4 - mowing_dir * 4 }, 1, 1, 0, 1, 1, 0);
					//debug_mesh_segment({ pt, pt - next_row_dir * 4 - mowing_dir * 4 }, 1, 1, 0, 1, 1, 0);
					if ((near_pave != on_pave && !closing_on_pave) || (grass_bdy && !near_pave))
					{
//						Point2 turn_loc = (on_pave || grass_bdy) ? pt : pt - mowing_dir * mow_steps;
						Point2 turn_loc = grass_bdy ? pt  - mowing_dir * mow_steps * 0.5 : on_pave ? pt : pt - mowing_dir * mow_steps;

						// it just looks more 'right' if we try to avoid placing turn circles off an pavement corner
						auto test_right = test_fwd ? -next_row_dir : next_row_dir;
						auto test_fwd = test_dir * (1.0 + mow_steps / test_radius);

						bool pave_fwd_left = inside_pt(ap_poly, turn_loc + (test_fwd - test_right * 0.5) * test_radius) ||
							inside_pt(ap_poly, turn_loc + (test_fwd - test_right) * test_radius);
						bool pave_fwd_right = inside_pt(ap_poly, turn_loc + (test_fwd + test_right * 0.5) * test_radius) ||
							inside_pt(ap_poly, turn_loc + (test_fwd + test_right) * test_radius);
						bool pave_left = inside_pt(ap_poly, turn_loc - test_right * test_radius * 1.2);
						bool pave_right = inside_pt(ap_poly, turn_loc + test_right * test_radius * 1.2);

						//debug_mesh_segment({ turn_loc + (test_fwd + test_right * 0.5 ) * test_radius, turn_loc + (test_fwd + test_right) * test_radius}, 0, 1, 0, 0, 1, 0);
						//debug_mesh_segment({ turn_loc + (test_fwd - test_right * 0.5) * test_radius, turn_loc + (test_fwd - test_right) * test_radius}, 1, 0, 0, 1, 0, 0);

						double offset = 0.0;
						if (pave_fwd_right == pave_fwd_left && !pave_left && !pave_right) offset = rand() % 6 - 2;
						else if (pave_fwd_right && !pave_right) offset =   rand() & 1 + 2;
						else if (pave_fwd_left && !pave_left)  offset =  -(rand() & 1 + 2);

						// if (pave_right != pave_left)
						{
							auto obj = WED_ObjPlacement::CreateTyped(apt->GetArchive());
							obj->SetParent(art_grp, 0);
							if (rand() & 3 > 0)
								obj->SetResource("lib/airport/ground/terrain_FX/lawn_tracks/single_6.obj");
							else
							{
								turn_loc += test_dir * 2;
								obj->SetResource("lib/airport/ground/terrain_FX/lawn_tracks/single_4.obj");
							}
							obj->SetName("Turn");
							obj->SetLocation(gis_Geo, turn_loc + test_right * offset);
							bool rev_hdg = !(grass_bdy ? !now_on_grass : near_pave);
							obj->SetHeading(g.second + (rev_hdg ? 180.0 : 0.0) + (rand() % 31 - 15));
							if (statistics) statistics[2]++;
						}
					}
					if (near_pave != on_pave)
					{
						on_pave = !on_pave;
						pt += mowing_dir * test_radius * 1.2;
					}
				}
				pt += mowing_dir * mow_steps;
			}
		}
	}

	// paved pads and mowing swirls underneath signs and some lights
	vector<WED_AirportSign *> signs;
	CollectRecursive(apt, back_inserter(signs), WED_AirportSign::sClass);
	
	for(auto s : signs)
	{
		Point2 pt;
		s->GetLocation(gis_Geo, pt);
		bool on_pavement = inside_pt(all_pave_poly, pt);
		
		if(!on_pavement)
		{
			make_ter_FX_exist(&art_grp, apt);
			auto obj = WED_ObjPlacement::CreateTyped(apt->GetArchive());
			obj->SetParent(art_grp, 0);
			obj->SetLocation(gis_Geo, pt);
			string label;
			s->GetName(label);
			sign_data tsign;
			tsign.from_code(label);
			int w = max(tsign.calc_width(0), tsign.calc_width(1));

			string res = "lib/airport/ground/terrain_FX/taxi_sign_base/light/";
			switch(s->GetHeight())
			{
				case size_SmallRemaining:
				case size_SmallTaxi:
				case size_MediumTaxi:
					res += "2.2m/";
					if      (w < 120) res += "2.2m.obj";
					else if (w < 200) res += "3.4m.obj";
					else 		 	  res += "4.0m.obj";
					break;
				default:
					res += "3.6m/";
					if      (w < 150) res += "3.6m.obj";
					else if (w < 250) res += "5.4m.obj";
					else              res += "6.6m.obj";
			}
			obj->SetResource(res);
			obj->SetName("Sign Base");
			obj->SetHeading(s->GetHeading() + 90.0);
			if(statistics) statistics[3]++;
		
			// check for grass all around and add moving crles
		
			double hdg = s->GetHeading();
			Vector2 test_dir(0,MTR_TO_DEG_LAT * 4.0);
			bool near_pavement = inside_pt(all_pave_poly, pt + test_dir);
			for(int i = 0; i < 3 && !near_pavement; i++)
			{
				test_dir = test_dir.perpendicular_cw();
				near_pavement = inside_pt(all_pave_poly, pt + test_dir);
			}
			if(!near_pavement)
			{
				auto obj = WED_ObjPlacement::CreateTyped(apt->GetArchive());
				obj->SetParent(art_grp, 0);
				obj->SetLocation(gis_Geo, pt);
				obj->SetResource("lib/airport/ground/terrain_FX/lawn_tracks/spot_1.obj");
				obj->SetName("Sign Swirl");
				obj->SetHeading(hdg + 90.0 + 180.0 * (rand() & 1));
				if(statistics) statistics[2]++;
			}
		}
	}
	
	// mow around all winsocks - also enhances their visibility
	vector<WED_Windsock *> socks;
	CollectRecursive(apt, back_inserter(socks), WED_Windsock::sClass);
	for(auto s : socks)
	{
		Point2 pt;
		s->GetLocation(gis_Geo, pt);
		bool on_pavement = inside_pt(all_pave_poly, pt);
		
		if(!on_pavement)
		{
			make_ter_FX_exist(&art_grp, apt);
			auto obj = WED_ObjPlacement::CreateTyped(apt->GetArchive());
			obj->SetParent(art_grp, 0);
			obj->SetLocation(gis_Geo, pt);
			obj->SetResource("lib/airport/ground/terrain_FX/lawn_tracks/spot_2.obj");
			obj->SetName("Windsock Swirl");
			obj->SetHeading(90.0 * (rand() & 3));
			if(statistics) statistics[2]++;
		}
	}
	
	// refactor this to be able to re-use pavement (and grass) for adding pavement art
	// convert everything into meterspace w/propper projection for that as well
	
/*	for (auto p : all_pave_poly)
		for(int i = 0; i < p.size(); i++)
			debug_mesh_segment(p.side(i), 1,0,0, 1,0,0);
*/				
	return all_grass_poly.size() > 0;
}

void WED_MowGrass(IResolver* resolver)
{
	WED_Thing* wrl = WED_GetWorld(resolver);
	vector<WED_Airport *> all_apts;
	int changed_apts = 0, statistics[4] = { 0 };

	CollectRecursiveNoNesting(wrl, back_inserter(all_apts), WED_Airport::sClass);

	wrl->StartOperation("Mow Grass");
	for (auto a : all_apts)
		changed_apts += WED_DoMowGrass(a, statistics);

	if (changed_apts > 0)
	{
		wrl->CommitOperation();
		string msg("Created at ");
		msg += to_string(changed_apts) + " Airport(s)\n\n";
		msg += to_string(statistics[0]) + " Grass Polygons\n";
		msg += to_string(statistics[1]) + " Grass Lines\n";
		msg += to_string(statistics[2]) + " Grass Objects\n";
		msg += to_string(statistics[3]) + " Paved Pads\n";
		
		DoUserAlert(msg.c_str());
	}
	else
		wrl->AbortOperation();
}
