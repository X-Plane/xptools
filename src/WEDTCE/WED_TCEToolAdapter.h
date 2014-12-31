/*
 * Copyright (c) 2009, Laminar Research.  All rights reserved.
 *
 */

/*

	TODOC

	This needs more docs, but basically: the control handle base tool has a HUGE amount of smart behavior to make structured CAD-style editing easy.

	The basic plug-in format for the TCE is different from the map.  So our base classes are different.

	This class ATTEMPTS to recycle a map tool into a TCE tool by mapping the overrides.  For some map tools there is no way in hell that this can wokr,
	but - at our own risk, we can attempt to do so.  This is being done to recycle the handle tool base into the TCE, so that handle-based editing can
	be the same everywhere...the handel tool is just way, way, way too much code to have in two places at once.

*/

#ifndef WED_TCEToolAdapter_H
#define WED_TCEToolAdapter_H

#include "WED_TCEToolNew.h"

class	WED_MapToolNew;

class WED_TCEToolAdapter : public WED_TCEToolNew {
public:

	WED_TCEToolAdapter(const char * tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver, WED_MapToolNew * brains);
	~WED_TCEToolAdapter();

	virtual	int					HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void				HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void				HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	int					HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  );
	virtual	void				KillOperation(bool mouse_is_down);		// Called when another tool is picked.  If a shape is half built, ABORT!

	virtual	const char *		GetStatusText(void);

	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g) ;
	virtual	void		DrawStructure			(bool inCurrent, GUI_GraphState * g) ;
	virtual	void		DrawSelected			(bool inCurrent, GUI_GraphState * g) ;

	virtual	void		DrawEntityVisualization	(bool inCurrent, IGISEntity * entity, GUI_GraphState * g) ;
	virtual	void		DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g) ;

	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s);

	// IPropertyObject

	virtual	int			FindProperty(const char * in_prop) const;
	virtual int			CountProperties(void) const;
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;			// Ben says: dictionary ops are broken out (and one vs all lookup are split too) for performance.
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) const;		// It may be slow to get all enums, so give the UI code a way to say if it needs this info.

	virtual void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);
	

private:

	WED_MapToolNew * brains;

};

#endif /* WED_TCEToolAdapter_H */
