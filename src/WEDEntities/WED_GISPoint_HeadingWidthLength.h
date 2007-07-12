#ifndef WED_GISPOINT_HEADINGWIDTHLENGTH_H
#define WED_GISPOINT_HEADINGWIDTHLENGTH_H

#include "WEd_GISPoint_Heading.h"

class	WED_GISPoint_HeadingWidthLength : public WED_GISPoint_Heading, public virtual IGISPoint_WidthLength {

DECLARE_INTERMEDIATE(WED_GISPoint_HeadingWidthLength)

public:

	virtual	GISClass_t		GetGISClass		(void				 ) const;

	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtOnFrame		(const Point2& p, double dist) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual	void			Rescale(
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds);

	// IGISPoint_WidthLength
	virtual	double	GetWidth (void		 ) const;
	virtual	void	SetWidth (double width)      ;
	virtual	double	GetLength(void		 ) const;
	virtual	void	SetLength(double width)      ;

	virtual	void	GetCorners(Point2 corners[4]) const;

	virtual	void	MoveCorner(int corner, const Vector2& delta);
	virtual	void	MoveSide(int side, const Vector2& delta);

	virtual	void	ResizeSide(int side, const Vector2& delta, bool symetric);
	virtual	void	ResizeCorner(int side, const Vector2& delta, bool symetric);

private:

		WED_PropDoubleTextMeters		width;
		WED_PropDoubleTextMeters		length;

};

#endif /* WED_GISPOINT_HEADINGWIDTHLENGTH_H */
