#ifndef WED_GISCOMPOSITE_H
#define WED_GISCOMPOSITE_H

#include "WED_Entity.h"
#include "IGIS.h"

class	WED_GISComposite : public WED_Entity, public virtual IGISComposite {

DECLARE_INTERMEDIATE(WED_GISComposite)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual bool			PtOnFrame		(const Point2& p, double d) const;
	virtual	void			Rescale			(const Bbox2& old_bounds,const Bbox2& new_bounds);
	virtual	void			Rotate			(const Point2& center, double angle);
	// IGISComposite
	virtual	int				GetNumEntities(void ) const;
	virtual	IGISEntity *	GetNthEntity  (int n) const;

private:

			void			RebuildCache(void) const;

	mutable	Bbox2					mCacheBounds;
	mutable	vector<IGISEntity *>	mEntities;

};

#endif
