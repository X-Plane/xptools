#ifndef WED_GISLINE_WIDTH_H
#define WED_GISLINE_WIDTH_H

#include "WED_GISLine.h"
#include "IGIS.h"

class	WED_GISLine_Width : public WED_GISLine, public virtual IGISLine_Width{

DECLARE_INTERMEDIATE(WED_GISLine_Width)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;

	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtOnFrame		(const Point2& p, double dist) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual	void			Rescale(
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds);
	
	// IGISLine_Width
	virtual	double	GetWidth (void		 ) const;
	virtual	void	SetWidth (double width)      ;
		
	virtual	void	GetCorners(Point2 corners[4]) const;

	virtual	void	MoveCorner(int corner, const Vector2& delta);
	virtual	void	MoveSide(int side, const Vector2& delta);

	virtual	void	ResizeSide(int side, const Vector2& delta, bool symetric);
	virtual	void	ResizeCorner(int side, const Vector2& delta, bool symetric);

private:

	WED_PropDoubleTextMeters		width;
	
};

#endif /* WED_GISLINE_WIDTH_H */
