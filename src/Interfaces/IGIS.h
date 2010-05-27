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

#ifndef IGIS_H
#define IGIS_H

#include "CompGeomDefs2.h"
#include "IBase.h"
#include "ISelection.h"

//------------------------------------------------------------------------------------------------------------
// ENTITY INTERFACE
//------------------------------------------------------------------------------------------------------------
//
// All entity objects must support the entity interface, providing some basic info:
//
// - What "GIS class" of entity are we?  This lets us make an immediate designation of topology.
// - What is our overall bounds.
// - All GIS entities must support a linear 2-d rescaling operation (described by remapping bounding boxes.
//
// Design note: the GIS interfaces are designe to promote the abstraction of a data model!  That is, they don't
// say HOW to edit something, just that this data has a certain amount of spatial extent that we can muck with.
//
// (This means that for a wide variety of spatial data model, we can reduce it to a finite number of cases
// and write generic editing code for all of it.  This promotes the separation of GIS algorithms from the base
// class because we can write an algorithm using only the interface.)
//
// The GIS class is the most specific class - that is, a bezier point will return bezier point, but not point.
// To correctly case out GIS classes, a client must handle the entire GIS spatial tree (or for points, at least
// point and all of its derivatives.
//
// (Someday we could make an "is-class" predicate to provide simpler casing.)
//
// The "sub-type" string provides an app-defined run-time type.  This is really a bit of a hack...WED does most
// ops on GIS-type, but in a few cases casts down to a real WED type.  The "subtype" is actually the
// WED-persistent-class string.  If we wanted to be more pure we'd derive airport-spatial entities off of the
// GIS-spatial entities, e.g. a runway (line-width with more points to edit0, a taxiway (poly with pavement type),
// etc.


enum GISClass_t {

	gis_Point,
		gis_Point_Bezier,
		gis_Point_Heading,
			gis_Point_HeadingWidthLength,
	gis_PointSequence,
		gis_Line,
			gis_Line_Width,
		gis_Edge,
		gis_Ring,
		gis_Chain,
	gis_Area,
		gis_Polygon,
		gis_BoundingBox,
	gis_Composite
};

enum GISLayer_t {
	gis_Geo,
	gis_UV
};

class	IGISEntity : public virtual ISelectable {
public:

	virtual	GISClass_t		GetGISClass		(void										  ) const=0;
	virtual	const char *	GetGISSubtype	(void										  ) const=0;
	virtual	bool			HasLayer		(GISLayer_t layer							  ) const=0;

	virtual	void			GetBounds		(GISLayer_t layer,	    Bbox2&  bounds		  ) const=0;
	virtual	bool			IntersectsBox	(GISLayer_t layer,const Bbox2&  bounds		  ) const=0;
	virtual	bool			WithinBox		(GISLayer_t layer,const Bbox2&  bounds		  ) const=0;
	virtual bool			PtWithin		(GISLayer_t layer,const Point2& p			  ) const=0;
	virtual bool			PtOnFrame		(GISLayer_t layer,const Point2& p, double dist) const=0;
	virtual bool			Cull			(				  const Bbox2& bounds		  ) const=0;	// Returns true if visible.  Different from bounds, because some objects "hang off" the screen a bit.

	virtual	void			Rescale(
								GISLayer_t layer,
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds)=0;
	virtual	void			Rotate(
								GISLayer_t layer,
								const Point2& center,
								double angle)=0;
};

class	IGISQuad : public virtual IGISEntity {
public:

	virtual	void	GetCorners(GISLayer_t layer, Point2 corners[4]) const=0;

	virtual	void	MoveCorner(GISLayer_t layer,int corner, const Vector2& delta)=0;
	virtual	void	MoveSide(GISLayer_t layer,int side, const Vector2& delta)=0;

	virtual	void	ResizeSide(GISLayer_t layer,int side, const Vector2& delta, bool symetric)=0;
	virtual	void	ResizeCorner(GISLayer_t layer,int side, const Vector2& delta, bool symetric)=0;

};

//------------------------------------------------------------------------------------------------------------
// POINT INTERFACES
//------------------------------------------------------------------------------------------------------------
//
// These are used for all point-derived entities - with interfaces added on for more "aspects".

class	IGISPoint : public virtual IGISEntity {
public:

	virtual	void	GetLocation(GISLayer_t layer,      Point2& p) const=0;
	virtual	void	SetLocation(GISLayer_t layer,const Point2& p)      =0;

};

class	IGISPoint_Bezier : public virtual IGISPoint {
public:

	virtual	bool	GetControlHandleLo (GISLayer_t layer,      Point2& p ) const=0;
	virtual	bool	GetControlHandleHi (GISLayer_t layer,      Point2& p ) const=0;
	virtual	bool	IsSplit			   (void							 ) const=0;
	virtual	void	GetBezierLocation  (GISLayer_t layer, BezierPoint2& p) const=0;

	virtual	void	SetControlHandleLo (GISLayer_t layer,const Point2& p	   )=0;
	virtual	void	SetControlHandleHi (GISLayer_t layer,const Point2& p	   )=0;
	virtual	void	DeleteHandleLo	   (void								   )=0;
	virtual	void	DeleteHandleHi	   (void								   )=0;
	virtual	void	SetSplit		   (				  bool is_split		   )=0;	// WARNING: unsplitting control handles WITHOUT then moving one handle leaves the resolution of split handles AMBIGUOUS!
	virtual	void	SetBezierLocation  (GISLayer_t layer, const BezierPoint2& p)=0;

};

class	IGISPoint_Heading : public virtual IGISPoint {
public:

	virtual	double	GetHeading(void			 ) const=0;
	virtual	void	SetHeading(double heading)      =0;

};


class	IGISPoint_WidthLength : public virtual IGISPoint_Heading, public virtual IGISQuad {
public:

	virtual	double	GetWidth (void		  ) const=0;
	virtual	void	SetWidth (double width)      =0;
	virtual	double	GetLength(void		  ) const=0;
	virtual	void	SetLength(double width)      =0;

};

//------------------------------------------------------------------------------------------------------------
// POINT SEQUENCE INTERFACES
//------------------------------------------------------------------------------------------------------------
//
// All point sequences use these.  Note that point sequences MAY be closed - we can access this without having
// to know every subclass of point sequence that exists.  Note that the points might be bezier.  We edit
// points by getting non-const interfaces to them.
// Note: we only can create points by splitting a side.  This keeps our points 'well-ordered...'

class	IGISPointSequence : public virtual IGISEntity {
public:

	virtual	int					GetNumPoints(void ) const=0;
//	virtual	void				DeletePoint (int n)		 =0;

	virtual		  IGISPoint *	GetNthPoint (int n)	const=0;

	virtual	int					GetNumSides(void) const=0;
	virtual	bool				GetSide  (GISLayer_t layer, int n, Segment2& s, Bezier2& b) const=0;	// true for bezier

	virtual	bool				IsClosed(void) const=0;

	virtual	void				Reverse(GISLayer_t l)=0;

	// Split the side at this point, returning the new point.  Or return NULL if the split is 
	// impossible/makes no sense.  Dist is a maximum distance from the point where "where" can be 
	// that we would still split.  The split is made on the line as close to "where" as possible.
	virtual		  IGISPoint *	SplitSide   (const Point2& where, double dist)=0;

};

class	IGISEdge : public virtual IGISPointSequence { 
public:

	virtual	bool				IsOneway(void) const=0;
	virtual	void				SetSide(GISLayer_t layer, const Segment2& s)=0;
	virtual	void				SetSideBezier(GISLayer_t layer, const Bezier2& b)=0;

};

class IGISLine : public virtual IGISPointSequence {
public:

	virtual		  IGISPoint *		GetSource(void)	const=0;
	virtual		  IGISPoint *		GetTarget(void)	const=0;

};

class IGISLine_Width : public virtual IGISLine, public virtual IGISQuad {
public:

	virtual	double	GetWidth (void		  ) const=0;
	virtual	void	SetWidth (double width)      =0;

};

//------------------------------------------------------------------------------------------------------------
// AREA entities
//------------------------------------------------------------------------------------------------------------
// Polygon: an outer CCW ring and zero or more inner CW rings.  Please note that you can't actually edit the
// bounds.  What you can do is get a non-const interface to a ring and then use the interface editing of the
// ring to muck with it.

class	IGISPolygon  : public virtual IGISEntity {
public:

	virtual			IGISPointSequence *		GetOuterRing(void )	const=0;
	virtual			int						GetNumHoles (void ) const=0;
	virtual			IGISPointSequence *		GetNthHole  (int n)	const=0;

	virtual			void					DeleteHole  (int n)					=0;
	virtual			void					AddHole		(IGISPointSequence * r) =0;

	virtual			void					Reverse(GISLayer_t l)=0;

};

class	IGISBoundingBox : public virtual IGISEntity {
public:

	virtual			IGISPoint *				GetMin(void) const=0;
	virtual			IGISPoint *				GetMax(void) const=0;

};

//------------------------------------------------------------------------------------------------------------
// Composite GIS objects...
//------------------------------------------------------------------------------------------------------------
// We do not provide a way to "edit" composite relationships, at least not yet.  This is just a way for code
// written entirely from a geo-analysis standpoint to do recursive trees.

class IGISComposite :  public virtual IGISEntity {
public:

	virtual	int				GetNumEntities(void ) const=0;
	virtual	IGISEntity *	GetNthEntity  (int n) const=0;

};

#endif
