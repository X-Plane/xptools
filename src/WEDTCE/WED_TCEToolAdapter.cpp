/*
 * Copyright (c) 2009, Laminar Research.  All rights reserved.
 *
 */

#include "hl_types.h"
#include "WED_TCEToolAdapter.h"
#include "WED_MapToolNew.h"

WED_TCEToolAdapter::WED_TCEToolAdapter(const char * tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver, WED_MapToolNew * ibrains) :
	WED_TCEToolNew(tool_name, host, zoomer, resolver), brains(ibrains)
{
}

WED_TCEToolAdapter::~WED_TCEToolAdapter()
{
	delete brains;
}

int					WED_TCEToolAdapter::HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	return brains->HandleClickDown(inX, inY, inButton, modifiers);
}

void				WED_TCEToolAdapter::HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	brains->HandleClickDrag(inX, inY, inButton, modifiers);
}

void				WED_TCEToolAdapter::HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
	brains->HandleClickUp(inX, inY, inButton, modifiers);
}

int					WED_TCEToolAdapter::HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  )
{
	return brains->HandleToolKeyPress(inKey, inVK, inFlags);
}

void				WED_TCEToolAdapter::KillOperation(bool mouse_is_down)
{
	brains->KillOperation(mouse_is_down);
}

const char *		WED_TCEToolAdapter::GetStatusText(void)
{
	return brains->GetStatusText();
}

void		WED_TCEToolAdapter::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
	brains->DrawVisualization(inCurrent,g);
}

void		WED_TCEToolAdapter::DrawStructure			(bool inCurrent, GUI_GraphState * g)
{
	brains->DrawStructure(inCurrent,g);
}

void		WED_TCEToolAdapter::DrawSelected			(bool inCurrent, GUI_GraphState * g)
{
	brains->DrawSelected(inCurrent,g);
}

void		WED_TCEToolAdapter::DrawEntityVisualization	(bool inCurrent, IGISEntity * entity, GUI_GraphState * g)
{
	bool v,s,w,c;
	brains->GetCaps(v,s,w,c);
	if(w)
		brains->DrawEntityVisualization(inCurrent, entity, g, true);
}

void		WED_TCEToolAdapter::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g)
{
	bool v,s,w,c;
	brains->GetCaps(v,s,w,c);
	if(w)
		brains->DrawEntityStructure(inCurrent, entity, g, true);
}

void		WED_TCEToolAdapter::GetCaps(bool& draw_ent_v, bool& draw_ent_s)
{
	bool want_sel, click;
	brains->GetCaps(draw_ent_v, draw_ent_s,want_sel, click);
}

int			WED_TCEToolAdapter::FindProperty(const char * in_prop)
{
	return brains->FindProperty(in_prop);
}

int			WED_TCEToolAdapter::CountProperties(void) const
{
	return brains->CountProperties();
}

void		WED_TCEToolAdapter::GetNthPropertyInfo(int n, PropertyInfo_t& info)
{
	brains->GetNthPropertyInfo(n, info);
}

void		WED_TCEToolAdapter::GetNthPropertyDict(int n, PropertyDict_t& dict)
{
	brains->GetNthPropertyDict(n,dict);
}

void		WED_TCEToolAdapter::GetNthPropertyDictItem(int n, int e, string& item)
{
	brains->GetNthPropertyDictItem(n,e,item);
}

void		WED_TCEToolAdapter::GetNthProperty(int n, PropertyVal_t& val) const
{
	brains->GetNthProperty(n,val);
}

void		WED_TCEToolAdapter::SetNthProperty(int n, const PropertyVal_t& val)
{
	brains->SetNthProperty(n,val);
}

