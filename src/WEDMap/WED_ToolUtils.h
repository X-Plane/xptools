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

#ifndef WED_TOOLUTILS_H
#define WED_TOOLUTILS_H

#include "GUI_Defs.h"

struct Point2;
struct Vector2;
class	GUI_Pane;
class	ISelectable;
class	ITexMgr;
class	WED_ResourceMgr;
class	WED_LibraryMgr;
class	WED_Airport;
class	WED_Thing;
class	ISelection;
class	ILibrarian;
class	IResolver;
class	IGISEntity;
class	GUI_DragData;

WED_Thing *		WED_FindParent(ISelection * isel,	// Selected objects
					WED_Thing * require_this,		// Our common container will be at or below this in the hiearchy
					WED_Thing * backup_choice);		// If selection is empty, we'll use this.

void			WED_GetSelectionInOrder(IResolver * resolver, vector<WED_Thing *>& out_sel);
void			WED_GetSelectionRecursive(IResolver * resolver, set<WED_Thing *>& out_sel);
bool			WED_IsSelectionNested(IResolver * resolver);		// Returns true if there are parent-children who are selected!


// This API attempts to find the right "container" thing for creating a new entity.  You get back the WED_thing that should be your parent and the index
// YOU should be in when you become a child.
//
// require_airport - if this is true, the parent will be inside the currently edited airport.  Set this to true if you are making an entity that MUST be airport
// contained, like a runway or ATC frequency.  Set this to false if your new thing can be in any part of the hierarchy, e.g. a forest.
//
// needs spatial - the host must be a map item like a group or airport, not a non-map item like an ATC flow
WED_Thing *		WED_GetCreateHost(IResolver * resolver,
									bool	require_airport,
									bool	needs_spatial,
									int&	idx);

WED_Thing *		WED_GetContainerForHost(IResolver * resolver, WED_Thing * host, bool require_airport, int& idx);

WED_Airport *	WED_GetCurrentAirport(IResolver * resolver);
WED_Airport *	WED_GetParentAirport(WED_Thing * who);
const WED_Airport *	WED_GetParentAirport(const WED_Thing * who);
void			WED_SetCurrentAirport(IResolver * resolver, WED_Airport * airport);		// Does NOT create a command!!!!!!
void			WED_SetAnyAirport(IResolver * resolver);

ISelection *	WED_GetSelect(IResolver * resolver);
WED_Thing	*	WED_GetWorld(IResolver * resolver);
ILibrarian *	WED_GetLibrarian(IResolver * resolver);
ITexMgr *		WED_GetTexMgr(IResolver * resolver);
WED_ResourceMgr*WED_GetResourceMgr(IResolver * resolver);
WED_LibraryMgr*	WED_GetLibraryMgr(IResolver * resolver);

bool			WED_IsIconic(IGISEntity * what);

double			WED_CalcDragAngle(const Point2& ctr, const Point2& handle, const Vector2& drag);

// This tells us if the WED thing can contain other things, from a property view perspective.
bool			WED_IsFolder(WED_Thing * what);

// Do we have a single selection of a known type?  We need this
WED_Thing*		WED_HasSingleSelectionOfType(IResolver * resolver, const char * in_class);

// What ancestor type do we need to be nested within?
const char *	WED_GetParentForClass(const char * in_class);


void			WED_GetAllRunwaysOneway(const WED_Airport * airport, set<int>& runways);
void			WED_GetAllRunwaysTwoway(const WED_Airport * airport, set<int>& runways);

//---------------------------------------------------------------------------------------------------------------------------------
// FILTERS:
//---------------------------------------------------------------------------------------------------------------------------------
// These routines return properties of an object.  The void * param is uusally unused.  They return 1 if true, 0 if false.
// They are designed such that we can run these on the selection (as an iterator) and get a response if ANY part of
// the selection meets this.

// Basic matching filters
int Iterate_ParentMismatch(ISelectable * what, void * ref);				// This object's parent is not the "ref" param.
int Iterate_IsParentOf(ISelectable * what, void * ref);					// This object is a parent of (or is) "ref".
int	Iterate_MatchesThing(ISelectable * what, void * ref);					// ref is a thing to match
int	Iterate_NotMatchesThing(ISelectable * what, void * ref);				// ref is a thing to match
// Type containment filters - ref is really a class string ptr.
int	Iterate_RequiresClass(ISelectable * what, void * ref);				// This object MUST have a certain type as part of its ancestors.
int	Iterate_ChildRequiresClass(ISelectable * what, void * ref);			// This object MUST have an certain type as part of its ancestors.  Or one of our children requires this.
int	Iterate_IsClass(ISelectable * what, void * ref);						// This object is an certain type.
int	Iterate_IsOrParentClass(ISelectable * what, void * ref);				// This object is an certain type, or its parent is or something.
int	Iterate_IsOrChildClass(ISelectable * what, void * ref);				// This object is an certain type, or its child is or something.
// Grouping and structured obj filters
int	Iterate_IsStructuredObject(ISelectable * what, void * ref);		// This object is part of a polygon or something.  DO NOT reorder it.
int	Iterate_IsNotStructuredObject(ISelectable * what, void * ref);		// This object is part of a polygon or something.  DO NOT reorder it.
int	Iterate_IsPartOfStructuredObject(ISelectable * what, void * ref);		// This object is part of a polygon or something.  DO NOT reorder it.
int	Iterate_IsNotPartOfStructuredObject(ISelectable * what, void * ref);		// This object is part of a polygon or something.  DO NOT reorder it.
int Iterate_IsNotGroup(ISelectable * what, void * ref);					// This object is not a group.
int	Iterate_IsNonEmptyComposite(ISelectable * what, void * ref);			// We are a composite and we have at least one child.
// Selection filters
int Iterate_HasSelectedParent(ISelectable * what, void * ref);				// ref is ISelection.

// Collecting - these
int	Iterate_CollectThings(ISelectable * what, void * ref);				// ref is ptr to vector<wed_thing>
int Iterate_CollectRequiredParents(ISelectable * what, void * ref);		// ref is a set of strings that are the required classes.
int Iterate_CollectChildPointSequences(ISelectable * what, void * ref);	// ref is a ptr to a vector<IGISPointSequence *>
int Iterate_CollectEntities  (ISelectable * what, void * ref);			// ref is a ptr to a vector<IGISEntity *>
int Iterate_CollectEntitiesUV(ISelectable * what, void * ref);			// ref is a ptr to a vector<IGISEntity *>  - only take entities with UV maps!

//---------------------------------------------------------------------------------------------------------------------------------
// WORLD ITERATION
//---------------------------------------------------------------------------------------------------------------------------------

bool IsGraphNode(WED_Thing * what);
bool IsGraphEdge(WED_Thing * what);
//void CollectRecursive(WED_Thing * root, bool(* filter)(WED_Thing *			  ),			 vector<WED_Thing *>& items);
//void CollectRecursive(WED_Thing * root, bool(* filter)(WED_Thing *, void * ref), void * ref, vector<WED_Thing *>& items);

//---------------------------------------------------------------------------------------------------------------------------------
// DRAG & DROP
//---------------------------------------------------------------------------------------------------------------------------------

void				WED_RegisterDND(void);
GUI_DragOperation	WED_DoDragSelection(
								GUI_Pane *				pane,
								int						x,
								int						y,
								int						button,
								int						where[4]);
bool				WED_IsDragSelection(
								GUI_DragData *			drag);



#endif /* WED_TOOLUTILS_H */
