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

#ifndef ICONTROLHANDLES_H
#define ICONTROLHANDLES_H

/*

	IControlHandles - THEORY OF OPERATION

	The control handles interface provids an abstraction for a set of structured UI elements.

	- Each control handle provider can specify 0 or more "entities" by arbitrary IDs.  This is
	  really a convenience interface to allow providers to map control handles to specific bits
	  of data model.

	- Each control handle can be moved.

	- Control handle quadruples make "links" which are bezier curves.  Note that moving the link is
	  NOT the same as moving the handles, so the client can decide the rules for dragging links.

	- Entire entities may also be moved, and the interface provides hit testing.

	AN IMPORTANT NOTE ON PTR SIZE!!!

	The control handle interface is iterated by index..there is an integer number of entities, and each entity has an
	integer number of links and nodes, referenced by zero-based array indices.  So any "nth" parameter (e.g. array index)
	and any count parameter are all of type int.

	BUT entity IDs are opaque handles - that is, the Nth entity's ID is an opaque handle decided by the implementer.  Since
	the handle might contain a ptr, intptr_t is used for 64-bit safety.

	Basically one control handle object might represent an ARRAY of objects in its implementation.  Since entities are
	referenced by ID and not by index (E.g. from the index, we get the ID and then use that), it allows the client to
	efficiently provide access by ptr.

*/

#include "CompGeomDefs2.h"
#include "IBase.h"

// Handle types - these specify how they are drawn.

enum HandleType_t {
	handle_None,		// Draw nothing
	handle_Square,		// Default square for marquee
	handle_Vertex,		// Movable vertex (forms a bezier curve)
	handle_VertexSharp,	// Movable vertex (no bezier curve)
	handle_Bezier,		// Bezier control handle endpoint
	handle_ClosePt,		// Point to hit to close a loop
	handle_Cross,		// Cross for precise placement
	handle_ArrowHead,	// Arrow head, no stem
	handle_Arrow,		// Arrow with stem
	handle_RotateHead,	// Rotate head, no stem
	handle_Rotate,		// Rotate with stem
	handle_Icon			// Hande is an icon - icon is drawn by someone else!
};

enum LinkType_t {
	link_None,			// Draw nothing
	link_Solid,			// Draw solid line...
	link_BezierCtrl,	// Thin line for bezier handles
	link_Ghost,			// implicit ghost line
	link_Marquee,
	link_Handle			// link can be virtual a handle
};

class	IControlHandles : public virtual IBase {
public:

	// Operations...all SETS of operations on control handles will
	// be part of an "edit".  This way if we get our control handle set
	// about 1000 times while the mouse is dragged, we know this is part
	// of one gesture.
	virtual		void	BeginEdit(void)=0;
	virtual		void	EndEdit(void)=0;

	// Entities - many entities perhaps?
	virtual		int			CountEntities(void) const=0;
	virtual		intptr_t	GetNthEntityID(int n) const=0;

	// Control handles - by number - they can be moved around.
	virtual		int				CountControlHandles(intptr_t id										) const=0;
	virtual		void			GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const=0;

	// Links are structural lines between the control handles.  We have 0 or more links
	// and each one has a start and end control handle index number.  This is not editable -
	// it can only be queried.
	virtual		int				GetLinks		    (intptr_t id) const=0;
	virtual		void			GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const =0;
	virtual		int				GetNthLinkSource   (intptr_t id, int n) const=0;
	virtual		int				GetNthLinkSourceCtl(intptr_t id, int n) const=0;	// -1 if no bezier ctl point!
	virtual		int				GetNthLinkTarget   (intptr_t id, int n) const=0;
	virtual		int				GetNthLinkTargetCtl(intptr_t id, int n) const=0;

	// Generic query as to whether a point is on the structure.  Some control-handle-eable entities
	// may have "fill" area other than the structural linkeage.
	// NOTE: control handle interface providers are NOT required to return point-on-structure for linkeage...
	// only for ADDITIONAL areas.
	virtual		bool	PointOnStructure(intptr_t id, const Point2& p) const=0;

	// Move ALL control handles (that is, the whole entity) by a delta.
	virtual		void	ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_handle)=0;
	virtual		void	ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_handle)=0;
	virtual		void	ControlsMoveBy	 (intptr_t id,        const Vector2& delta, Point2& io_handle)=0;

};

#endif /* ICONTROLHANDLES_H */
