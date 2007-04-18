#ifndef WED_MAPLAYER_H
#define WED_MAPLAYER_H

class	WED_MapZoomerNew;
class	GUI_GraphState;
class	IGISEntity;
class	IResolver;
class	GUI_Pane;

class	WED_MapLayer {
public:

						 WED_MapLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_MapLayer();

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g)	{ }
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawSelected			(int inCurrent, GUI_GraphState * g) { }
	
	// These draw specific entities.  Use these to draw pieces of the data model.  Only visible entities will be passed in!	
	virtual	void		DrawEntityVisualization	(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { }
	virtual	void		DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { }
	
protected:

	inline	WED_MapZoomerNew *	GetZoomer(void) const { return mZoomer; }
	inline	IResolver *			GetResolver(void) const { return mResolver; }
	inline	GUI_Pane *			GetHost(void) const { return mHost; }
	
private:

						 WED_MapLayer();

	WED_MapZoomerNew *		mZoomer;
	IResolver *				mResolver;
	GUI_Pane *				mHost;
	
};
		

#endif /* WED_MAPLAYER_H */
