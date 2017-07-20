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

#include "WED_ToolUtils.h"
#include "ISelection.h"
#include "WED_Thing.h"
#include "WED_EnumSystem.h"
#include "GUI_Pane.h"
#include "IResolver.h"
#include "ILibrarian.h"
#include "GISUtils.h"
#include "ITexMgr.h"
#include "WED_Airport.h"
#include "STLUtils.h"
#include <list>

#include "GUI_Clipboard.h"
#include "WED_Group.h"
#include "WED_AirportBeacon.h"
#include "WED_AirportBoundary.h"
#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
#include "WED_AirportSign.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_RampPosition.h"
#include "WED_Runway.h"
#include "WED_RunwayNode.h"
#include "WED_TextureNode.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_Sealane.h"
#include "WED_Taxiway.h"
#include "WED_TowerViewpoint.h"
#include "WED_Windsock.h"
#include "WED_ResourceMgr.h"
#include "WED_ATCFlow.h"
#include "WED_ATCFrequency.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCTimeRule.h"
#include "WED_ATCWindRule.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_RoadNode.h"
#include "WED_LibraryMgr.h"

using std::list;

// FIRST thing is root, last is WHO
static void GetLineage(WED_Thing * who, list<WED_Thing *>& parents)
{
	parents.clear();
	while(who)
	{
		parents.push_front(who);
		who = who->GetParent();
	}
}

static WED_Thing *	FindCommonParent(WED_Thing * a, WED_Thing * b)
{
	if (a == NULL) return b->GetParent();
	if (b == NULL) return a->GetParent();
	list<WED_Thing *> a_lin, b_lin;
	GetLineage(a,a_lin);
	GetLineage(b,b_lin);

	WED_Thing * best = NULL;

	list<WED_Thing *>::iterator a_it = a_lin.begin(), b_it = b_lin.begin();
	while (a_it != a_lin.end() && b_it != b_lin.end() && *a_it == *b_it)
	{
		best = *a_it;
		++a_it, ++b_it;
	}

	return best;
}



WED_Thing *	WED_FindParent(ISelection * isel, WED_Thing * require_this, WED_Thing * backup_choice)
{
	vector<ISelectable *> sel;
	isel->GetSelectionVector(sel);

	WED_Thing * common_parent = NULL;

	if (sel.empty()) return backup_choice;

	for (vector<ISelectable *>::iterator iter = sel.begin(); iter != sel.end(); ++iter)
	{
		WED_Thing * obj = SAFE_CAST(WED_Thing, *iter);
		if (obj)
		{
			WED_Thing * new_parent = FindCommonParent(obj, common_parent);
			DebugAssert(new_parent);
			common_parent = new_parent;
		}
	}

	DebugAssert(common_parent);

	if (require_this)
	if (FindCommonParent(common_parent, require_this) != require_this)
		return require_this;

	return common_parent;
}


WED_Airport *		WED_GetCurrentAirport(IResolver * resolver)
{
	return SAFE_CAST(WED_Airport,resolver->Resolver_Find("choices.airport"));
}

void			WED_SetCurrentAirport(IResolver * resolver, WED_Airport * airport)
{
	IDirectoryEdit * keys = SAFE_CAST(IDirectoryEdit, resolver->Resolver_Find("choices"));
	keys->Directory_Edit("airport", airport);
}

const WED_Airport * WED_GetParentAirport(const WED_Thing * who)
{
	while(who)
	{
		const WED_Airport * a = dynamic_cast<const WED_Airport *>(who);
		if(a) return a;
		who = who->GetParent();
	}
	return NULL;
}

WED_Airport * WED_GetParentAirport(WED_Thing * who)
{
	while(who)
	{
		WED_Airport * a = dynamic_cast<WED_Airport *>(who);
		if(a) return a;
		who = who->GetParent();
	}
	return NULL;
}

static WED_Airport *			FindAnyAirport(WED_Thing * who)
{
	if (who->GetClass() == WED_Airport::sClass)
	{
		WED_Airport * a = dynamic_cast<WED_Airport *>(who);
		if (a) return a;
	}
	int nc = who->CountChildren();
	for (int n = 0; n < nc; ++n)
	{
		WED_Airport * aa = FindAnyAirport(who->GetNthChild(n));
		if (aa) return aa;
	}
	return NULL;
}


void			WED_SetAnyAirport(IResolver * resolver)
{
	IBase * has_an_airport = resolver->Resolver_Find("choices.airport");
	if(has_an_airport)
		return;

	WED_Thing * t = WED_GetWorld(resolver);
	WED_SetCurrentAirport(resolver,FindAnyAirport(t));
}


ISelection *	WED_GetSelect(IResolver * resolver)
{
	return SAFE_CAST(ISelection,resolver->Resolver_Find("selection"));
}

WED_Thing *	WED_GetWorld(IResolver * resolver)
{
	return SAFE_CAST(WED_Thing,resolver->Resolver_Find("world"));
}

ILibrarian *	WED_GetLibrarian(IResolver * resolver)
{
	return SAFE_CAST(ILibrarian,resolver->Resolver_Find("librarian"));
}

ITexMgr *		WED_GetTexMgr(IResolver * resolver)
{
	return SAFE_CAST(ITexMgr,resolver->Resolver_Find("texmgr"));
}

WED_ResourceMgr*WED_GetResourceMgr(IResolver * resolver)
{
	return SAFE_CAST(WED_ResourceMgr,resolver->Resolver_Find("resmgr"));
}

WED_LibraryMgr*WED_GetLibraryMgr(IResolver * resolver)
{
	return SAFE_CAST(WED_LibraryMgr,resolver->Resolver_Find("libmgr"));
}

WED_Thing * WED_GetCreateHost(IResolver * resolver, bool require_airport, bool needs_spatial, int& idx)
{
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * wrl = WED_GetWorld(resolver);

	if (sel->GetSelectionCount() == 1)
	{
		WED_Thing * obj = SAFE_CAST(WED_Thing, sel->GetNthSelection(0));
		if (obj != wrl)
		if (obj->GetParent())
		if (!needs_spatial || dynamic_cast<IGISEntity *>(obj->GetParent()))
		if (!Iterate_IsPartOfStructuredObject(obj,NULL))
		if (!require_airport || Iterate_IsOrParentClass(obj->GetParent(), (void *) WED_Airport::sClass))
		{
			idx = obj->GetMyPosition();
			return obj->GetParent();
		}
	}

	idx = 0;

	WED_Thing *	parent_of_sel = WED_FindParent(sel, NULL, NULL);

	if (Iterate_IsStructuredObject(parent_of_sel, NULL))	parent_of_sel = NULL;

	if (parent_of_sel && require_airport && !Iterate_IsOrParentClass(parent_of_sel, (void *) WED_Airport::sClass))
		parent_of_sel = NULL;

	if (parent_of_sel && needs_spatial && dynamic_cast<IGISEntity *>(parent_of_sel) == NULL)
		parent_of_sel = NULL;

	if (parent_of_sel == NULL)
	{
		if (require_airport)
			parent_of_sel = WED_GetCurrentAirport(resolver);
		else
			parent_of_sel = wrl;
	}

	if(parent_of_sel == wrl->GetParent()) parent_of_sel = wrl;
	return parent_of_sel;
}

WED_Thing *		WED_GetContainerForHost(IResolver * resolver, WED_Thing * host, bool require_airport, int& idx)
{
	if(host->GetClass() == WED_Airport::sClass)	return host;
	WED_Thing * wrl = WED_GetWorld(resolver);
	if(!require_airport && host == wrl)	return host;
	
	idx = 0;
	
	WED_Airport * apt = WED_GetParentAirport(host);
	if(apt != NULL)
		return apt;
	
	if(require_airport)
		return WED_GetCurrentAirport(resolver);
	else
		return wrl;
}

static void	WED_GetSelectionInOrderRecursive(ISelection * sel, WED_Thing * who, vector<WED_Thing *>& out_sel)
{
	if (sel->IsSelected(who)) out_sel.push_back(who);
	int c = who->CountChildren();
	for (int n = 0; n < c; ++n)
		WED_GetSelectionInOrderRecursive(sel, who->GetNthChild(n), out_sel);
}

void	WED_GetSelectionInOrder(IResolver * resolver, vector<WED_Thing *>& out_sel)
{
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * wrl = WED_GetWorld(resolver);
	out_sel.clear();
	WED_GetSelectionInOrderRecursive(sel, wrl, out_sel);
}

static int			WED_GetSelectionRecursiveOne(ISelectable * what, void * container)
{
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	if (who == NULL) return 0;

	set<WED_Thing *> * sel = (set<WED_Thing *> *) container;

	sel->insert(who);
	int c = who->CountChildren();
	for (int n = 0; n < c; ++n)
		WED_GetSelectionRecursiveOne(who->GetNthChild(n), sel);
	return 0;
}

void			WED_GetSelectionRecursive(IResolver * resolver, set<WED_Thing *>& out_sel)
{
	out_sel.clear();
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * wrl = WED_GetWorld(resolver);

	sel->IterateSelectionOr(WED_GetSelectionRecursiveOne, &out_sel);
	out_sel.erase(wrl);
}


bool			WED_IsSelectionNested(IResolver * resolver)
{
	ISelection * sel = WED_GetSelect(resolver);
	return sel->IterateSelectionOr(Iterate_HasSelectedParent, sel);
}

#pragma mark -

bool WED_IsFolder(WED_Thing * what)
{
	IGISEntity * ent;
	if(dynamic_cast<IGISPolygon*>(what))					return false;
	if(dynamic_cast<IGISPointSequence*>(what))				return false;
	if(dynamic_cast<IGISComposite*>(what))					return true;
#if AIRPORT_ROUTING
	if(strcmp(what->GetClass(), WED_ATCFlow::sClass)==0)	return true;
#endif
	return false;
}

WED_Thing *		WED_HasSingleSelectionOfType(IResolver * resolver, const char * in_class)
{
	ISelection * sel = WED_GetSelect(resolver);
	vector<WED_Thing *> who;
	sel->IterateSelectionOr(Iterate_CollectThings, &who);
	if(who.size() != 1) return NULL;
	if(strcmp(who[0]->GetClass(), in_class) != 0) return NULL;
	return who[0];
}

// A given entity class is allowe to require a parent - the parent can be _any_ parent 
// in the hierarchy, not just its immediate parent.  
//
// We use this to enforce certain hierarchy semantic relationships like: no runway 
// outside of an airport, no atc time rule outside of an ATC flow, etc.
//
// TODO: there is some relatively weird stuff going on.  Arguably the parent class for
// specialized nodes should be their parent geometry.  The main reason this doens't
// blow up is that the hieararchy also tries to disallow the reorganization of 
// non-folders - in other words, you can't move a runway node ANYWHERE as a user 
// because you don't know it exists.
const char *	WED_GetParentForClass(const char * in_class)
{
	if(strcmp(in_class,WED_AirportBeacon::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_AirportBoundary::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_AirportChain::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_AirportNode::sClass)==0)			return WED_Airport::sClass;
	if(strcmp(in_class,WED_AirportSign::sClass)==0)			return WED_Airport::sClass;
	if(strcmp(in_class,WED_Helipad::sClass)==0)				return WED_Airport::sClass;
	if(strcmp(in_class,WED_LightFixture::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_RampPosition::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_Runway::sClass)==0)				return WED_Airport::sClass;
	if(strcmp(in_class,WED_RunwayNode::sClass)==0)			return WED_Airport::sClass;
	if(strcmp(in_class,WED_Sealane::sClass)==0)				return WED_Airport::sClass;
	if(strcmp(in_class,WED_Taxiway::sClass)==0)				return WED_Airport::sClass;
	if(strcmp(in_class,WED_TowerViewpoint::sClass)==0)		return WED_Airport::sClass;
	if(strcmp(in_class,WED_Windsock::sClass)==0)			return WED_Airport::sClass;

	if(strcmp(in_class,WED_ATCFrequency::sClass)==0)		return WED_Airport::sClass;
#if AIRPORT_ROUTING
	if(strcmp(in_class,WED_ATCFlow::sClass)==0)				return WED_Airport::sClass;
	if(strcmp(in_class,WED_ATCRunwayUse::sClass)==0)		return WED_ATCFlow::sClass;
	if(strcmp(in_class,WED_ATCTimeRule::sClass)==0)			return WED_ATCFlow::sClass;
	if(strcmp(in_class,WED_ATCWindRule::sClass)==0)			return WED_ATCFlow::sClass;

	if(strcmp(in_class,WED_TaxiRoute::sClass)==0)			return WED_Airport::sClass;
	if(strcmp(in_class,WED_TaxiRouteNode::sClass)==0)		return WED_Airport::sClass;

#endif
	return NULL;
}

static void WED_LookupRunwayRecursiveOneway(const WED_Thing * thing, set<int>& runways)
{
	const WED_Runway * rwy = (thing->GetClass() == WED_Runway::sClass) ? dynamic_cast<const WED_Runway *>(thing) : NULL;
	if(rwy)
	{
		pair<int,int> e = rwy->GetRunwayEnumsOneway();
		if(e.first != atc_Runway_None) runways.insert(e.first);
		if(e.second != atc_Runway_None) runways.insert(e.second);
	}
	for(int n = 0; n < thing->CountChildren(); ++n)
	{
		WED_LookupRunwayRecursiveOneway(thing->GetNthChild(n), runways);
	}
}

static void WED_LookupRunwayRecursiveTwoway(const WED_Thing * thing, set<int>& runways)
{
	const WED_Runway * rwy = (thing->GetClass() == WED_Runway::sClass) ? dynamic_cast<const WED_Runway *>(thing) : NULL;
	if(rwy)
	{
		int e = rwy->GetRunwayEnumsTwoway();
		if(e != atc_rwy_None)
			runways.insert(e);
	}
	for(int n = 0; n < thing->CountChildren(); ++n)
	{
		WED_LookupRunwayRecursiveTwoway(thing->GetNthChild(n), runways);
	}
}

void			WED_GetAllRunwaysOneway(const WED_Airport * airport, set<int>& runways)
{
	runways.clear();
	WED_LookupRunwayRecursiveOneway(airport,runways);
}

void			WED_GetAllRunwaysTwoway(const WED_Airport * airport, set<int>& runways)
{
	runways.clear();
	WED_LookupRunwayRecursiveTwoway(airport,runways);
}

#pragma mark -

int	Iterate_RequiresClass(ISelectable * what, void * ref)
{
	WED_Persistent * t = dynamic_cast<WED_Persistent *>(what);
	if(t == NULL) return 0;

	const char * parent_class = WED_GetParentForClass(t->GetClass());
	if(parent_class == NULL) return 0;

	const char * query_class = (const char *) ref;
	return strcmp(parent_class, query_class) == 0;
}

int	Iterate_ChildRequiresClass(ISelectable * what, void * ref)
{
	if (Iterate_RequiresClass(what, ref)) return 1;
	WED_Thing * o = dynamic_cast<WED_Thing *>(what);
	if (o == NULL) return 0;
	if (dynamic_cast<WED_Airport *>(what)) return 0;				// Ben says: if we are an airport, do not eval our kids - their needs are met!
	for (int n = 0; n < o->CountChildren(); ++n)
	if (Iterate_ChildRequiresClass(o->GetNthChild(n), ref))
		return 1;
	return 0;
}

int	Iterate_IsClass(ISelectable * what, void * ref)
{
	WED_Persistent * t = dynamic_cast<WED_Persistent*>(what);
	if(t == NULL) return 0;
	return strcmp(t->GetClass(), (const char *) ref) == 0;
}

int	Iterate_IsOrParentClass(ISelectable * what, void * ref)
{
	if (what == NULL) return 0;
	WED_Thing * o = dynamic_cast<WED_Thing *>(what);
	if (o == NULL) return 0;
	while (o)
	{
		if(strcmp(o->GetClass(), (const char *) ref)==0) return 1;
		o = o->GetParent();
	}
	return 0;
}

int	Iterate_IsOrChildClass(ISelectable * what, void * ref)
{
	if (what == NULL) return 0;
	WED_Thing * o = dynamic_cast<WED_Thing *>(what);
	if (o == NULL) return 0;
	if(strcmp(o->GetClass(), (const char *) ref)==0) return 1;

	for (int n = 0; n < o->CountChildren(); ++n)
	{
		if (Iterate_IsOrChildClass(o->GetNthChild(n), ref))
			return 1;
	}
	return 0;
}

#pragma mark -

int	Iterate_IsStructuredObject(ISelectable * what, void * ref)
{
	IGISEntity * e = dynamic_cast<IGISEntity *>(what);
	if (!e) return 0;

	if(dynamic_cast<IGISPolygon*>(e)) return 1;
	if(dynamic_cast<IGISPointSequence*>(e)) return 1;
	switch(e->GetGISClass()) {
	case gis_PointSequence:
	case gis_Line:
	case gis_Line_Width:
	case gis_Ring:
	case gis_Chain:
	case gis_Polygon:	return 1;
	default:			return 0;
	}
}

int	Iterate_IsNotStructuredObject(ISelectable * what, void * ref)
{
	return Iterate_IsStructuredObject(what,ref) ? 0 : 1;
}

int	Iterate_IsPartOfStructuredObject(ISelectable * what, void * ref)
{
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	if (who == NULL) return 0;
	WED_Thing * parent = who->GetParent();
	if (parent == NULL) return 0;
	return Iterate_IsStructuredObject(parent, NULL);
}

int	Iterate_IsNotPartOfStructuredObject(ISelectable * what, void * ref)
{
	return Iterate_IsPartOfStructuredObject(what, ref) ? 0 : 1;
}


int Iterate_IsNotGroup(ISelectable * what, void * ref)
{
	if (dynamic_cast<WED_Group *>(what) == NULL) return 1;
	return 0;
}

int Iterate_IsNonEmptyComposite(ISelectable * what, void * ref)
{
	IGISComposite * comp = dynamic_cast<IGISComposite *> (what);
	if (comp == NULL) return 0;
	if (comp->GetGISClass() != gis_Composite) return 0;
	return comp->GetNumEntities() > 0;
}



int Iterate_ParentMismatch(ISelectable * what, void * ref)
{
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	WED_Thing * parent = (WED_Thing *) ref;
	if (who == NULL)				return 1;
	if (who->GetParent() != parent) return 1;
									return 0;
}

int Iterate_IsParentOf(ISelectable * what, void * ref)					// This object is a parent of (or is) "ref".
{
	WED_Thing * child = dynamic_cast<WED_Thing *>(what);
	WED_Thing * parent = (WED_Thing *) ref;

	while(parent)
	{
		if (child == parent) return 1;
		parent = parent->GetParent();
	}
	return 0;
}

int	Iterate_MatchesThing(ISelectable * what, void * ref)					// ref is a thing to match
{
	WED_Thing * target = (WED_Thing *) ref;
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	return who == target;
}

int	Iterate_NotMatchesThing(ISelectable * what, void * ref)					// ref is a thing to match
{
	WED_Thing * target = (WED_Thing *) ref;
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	return who != target;
}

int Iterate_HasSelectedParent(ISelectable * what, void * ref)
{
	WED_Thing * p = dynamic_cast<WED_Thing *>(what);
	if (p == NULL) return 0;
	ISelection * sel = (ISelection *) ref;
	p = p->GetParent();
	while (p)
	{
		if (sel->IsSelected(p)) return 1;
		p = p->GetParent();
	}
	return 0;
}

int Iterate_CollectChildPointSequences(ISelectable * what, void * ref)
{
	vector<IGISPointSequence *> * container = (vector<IGISPointSequence *> *) ref;
	IGISPolygon * poly = dynamic_cast<IGISPolygon *>(what);
	IGISPointSequence * ps = dynamic_cast<IGISPointSequence *>(what);
	if (ps) container->push_back(ps);
	if (poly) {
		container->push_back(poly->GetOuterRing());
		int hc = poly->GetNumHoles();
		for (int h = 0; h < hc; ++h)
			container->push_back(poly->GetNthHole(h));
	}
	return 0;
}


int Iterate_CollectEntities(ISelectable * what, void * ref)
{
	vector<IGISEntity *> * container = (vector<IGISEntity *> *) ref;
	IGISEntity * who = dynamic_cast<IGISEntity *>(what);
	if (who) container->push_back(who);
	return 0;
}

int Iterate_CollectEntitiesUV(ISelectable * what, void * ref)
{
	vector<IGISEntity *> * container = (vector<IGISEntity *> *) ref;
	IGISEntity * who = dynamic_cast<IGISEntity *>(what);
	if (who && who->HasLayer(gis_UV)) container->push_back(who);
	return 0;
}

int Iterate_CollectThings(ISelectable * what, void * ref)
{
	vector<WED_Thing *> * container = (vector<WED_Thing *> *) ref;
	WED_Thing * who = dynamic_cast<WED_Thing *>(what);
	if (who) container->push_back(who);
	return 0;
}

int Iterate_CollectRequiredParents(ISelectable * what, void * ref)
{
	set<string> * classes = (set<string> *) ref;
	WED_Thing * w = dynamic_cast<WED_Thing *>(what);
	if(w)
	{
		const char * p = WED_GetParentForClass(w->GetClass());
		if(p) classes->insert(p);
		
		// This is slightly tricky - if our chilren have picky needs, we must cater
		// to them.  So if I am a group and my kid is a wind rule, I need to be in a flow!
		// Buuuut if I _already_ meet my kids needs (e.g I am a flow and my wind-rule child needs
		// to be in a flow) then those needs are ALREADY met.

		// So.  Gather up all of the kids reqs.
		set<string>  child_reqs;
		for(int n = 0; n < w->CountChildren(); ++n)
			Iterate_CollectRequiredParents(w->GetNthChild(n), &child_reqs);

		// And filter out the one that I need first.		
		child_reqs.erase(w->GetClass());
		
		// Pass on the rest.
		for(set<string>::iterator i = child_reqs.begin(); i != child_reqs.end(); ++i)
			classes->insert(*i);
			
		// Since this is recursive, the only required parents that 'flow out' are the ones not met
		// WITHIN the sub-tree of this item.
		
			
	}
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
// DRAG & DROP
//---------------------------------------------------------------------------------------------------------------------------------

static	GUI_ClipType	sSelectionType = gui_ClipType_Invalid;

void				WED_RegisterDND(void)
{
	sSelectionType = GUI_RegisterPrivateClipType("WED_Selection");
}

GUI_DragOperation	WED_DoDragSelection(
								GUI_Pane *				pane,
								int						x,
								int						y,
								int						button,
								int						where[4])
{
	if(sSelectionType == gui_ClipType_Invalid) WED_RegisterDND();
	
	void * dummy = NULL;
	const void *	ptrs[1] = { &dummy };
	int		sizes[1] = { sizeof(dummy) };
	return pane->DoDragAndDrop(x,y,button,where,
							gui_Drag_Move,
							1,
							&sSelectionType,
							sizes,
							ptrs,
							NULL, NULL);
}

bool				WED_IsDragSelection(
								GUI_DragData *			drag)
{
	if(sSelectionType == gui_ClipType_Invalid) WED_RegisterDND();

	if (drag->CountItems() != 1) return false;
	return drag->NthItemHasClipType(0, sSelectionType);
}

bool			WED_IsIconic(IGISEntity * what)
{
	switch(what->GetGISClass()) {
	case gis_Point:
	case gis_Point_Heading:
		// Ben says: this is a little fugly.  All point types turn into icons (e.g. windsocks, etc.) except for a few exceptions.
		// This is only for POINTS, not for BEZIER points, hence the "non-icon" bezier classes are NOT listed.
		return what->GetGISSubtype() != WED_RunwayNode::sClass &&			// Runways have a special node type.  Special type avoids wed-airportnode with taxiway lines and bezier caps
			what->GetGISSubtype() != WED_TextureNode::sClass &&			// This is for non-bezier scenery UV mapped stuff
			what->GetGISSubtype() != WED_SimpleBoundaryNode::sClass &&	// This is for non-bezier scenery non-UV mapped stuff.
			what->GetGISSubtype() != WED_TaxiRouteNode::sClass
#if ROAD_EDITING
			&&  what->GetGISSubtype() != WED_RoadNode::sClass
#endif
			;
	default:
		return false;
	}
}

double			WED_CalcDragAngle(const Point2& ctr, const Point2& handle, const Vector2& drag)
{
	Point2 handle_new = handle + drag;
	double a1 = VectorDegs2NorthHeading(ctr, ctr, Vector2(ctr, handle));
	double b1 = VectorDegs2NorthHeading(ctr, ctr, Vector2(ctr, handle_new));
	return b1 - a1;
}


bool IsGraphNode(WED_Thing * what)
{
	if(what->CountViewers() == 0) return false;
	WED_Thing * parent = what->GetParent();
	IGISComposite * c = SAFE_CAST(IGISComposite,parent);
	if (c == NULL || c->GetGISClass() != gis_Composite) return false;
	return SAFE_CAST(IGISPoint, what) != NULL;
}

bool IsGraphEdge(WED_Thing * what)
{
	return dynamic_cast<IGISEdge*>(what) != NULL;
}

/*
void CollectRecursive(WED_Thing * root, bool(* filter)(WED_Thing *), vector<WED_Thing *>& items)
{
	if(filter(root)) items.push_back(root);
	int nn = root->CountChildren();
	for(int n = 0; n < nn; ++n)
		CollectRecursive(root->GetNthChild(n), filter, items);
}

void CollectRecursive(WED_Thing * root, bool(* filter)(WED_Thing *, void * ref), void * ref, vector<WED_Thing *>& items)
{
	if(filter(root,ref)) items.push_back(root);
	int nn = root->CountChildren();
	for(int n = 0; n < nn; ++n)
		CollectRecursive(root->GetNthChild(n), filter, ref, items);
}*/
