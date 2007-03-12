#ifndef WED_MAPLAYER_H
#define WED_MAPLAYER_H

class	WED_MapZoomerNew;
class	GUI_GraphState;
class	IUnknown;
class	IResolver;

class	WED_MapLayer {
public:

						 WED_MapLayer(WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_MapLayer();

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g)	{ }
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawSelected			(int inCurrent, GUI_GraphState * g) { }
	
	// These draw specific entities.  Use these to draw pieces of the data model.  Only visible entities will be passed in!	
	virtual	void		DrawEntityVisualization	(int inCurrent, IUnknown * entity, GUI_GraphState * g) { }
	virtual	void		DrawEntityStructure		(int inCurrent, IUnknown * entity, GUI_GraphState * g) { }
	virtual	void		DrawEntitySelected		(int inCurrent, IUnknown * entity, GUI_GraphState * g) { }
	
protected:

	inline	WED_MapZoomerNew *	GetZoomer(void) const { return mZoomer; }
	inline	IResolver *			GetResolver(void) const { return mResolver; }

private:

						 WED_MapLayer();

	WED_MapZoomerNew *		mZoomer;
	IResolver *				mResolver;
	
};
		

#endif /* WED_MAPLAYER_H */
