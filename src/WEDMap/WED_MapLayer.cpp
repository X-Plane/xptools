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

#include "WED_MapLayer.h"
#include "MathUtils.h"
#include "GUI_Resources.h"
#include "WED_UIMeasurements.h"
#include "IGIS.h"
#include "WED_Entity.h"

#include "WED_MapZoomerNew.h"

WED_MapLayer::WED_MapLayer(GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) :
	mZoomer(z), mResolver(i), mHost(h), mHideFilter(NULL), mLockFilter(NULL)
{
	int dims[2];

	GUI_GetImageResourceSize("map_airport.png",dims);
	mAirportRadius = 0.5 * (double)(intmin2(dims[0],dims[1]));

	GUI_GetImageResourceSize("map_towerview.png",dims);
	mFurnitureRadius = 0.5 * (double)(intmin2(dims[0],dims[1]));

	mAirportFactor = WED_UIMeasurement("airport_icon_scale");
	mFurnitureFactor = WED_UIMeasurement("furniture_icon_scale");

	mAirportTransWidth = WED_UIMeasurement("airport_trans_width");
	mVisible = true;
}

WED_MapLayer::~WED_MapLayer()
{
}

double		WED_MapLayer::GetFurnitureIconScale(void) const
{
	return doblim(GetZoomer()->GetPPM() * mFurnitureFactor,0.001,1.0);
}

double		WED_MapLayer::GetFurnitureIconRadius(void) const
{
	return doblim(GetZoomer()->GetPPM() * mFurnitureFactor,0.001,1.0) * mFurnitureRadius;
}

double		WED_MapLayer::GetAirportIconScale(void) const
{
	return doblim(GetZoomer()->GetPPM() * mAirportFactor,0.5,1.0);
}

double		WED_MapLayer::GetAirportIconRadius(void) const
{
	return doblim(GetZoomer()->GetPPM() * mAirportFactor, 0.5,1.0) * mAirportRadius;
}

bool		WED_MapLayer::IsVisible(void) const
{
	return mVisible;
}

void		WED_MapLayer::SetVisible(bool visibility)
{
	mVisible = visibility;
}

void		WED_MapLayer::ToggleVisible(void)
{
	mVisible = !mVisible;
	GetHost()->Refresh();
}

void		WED_MapLayer::SetFilter(const vector<const char *> * hide_filter_ptr, const vector<const char *> * lock_filter_ptr)
{
	mHideFilter = hide_filter_ptr;
	mLockFilter = lock_filter_ptr;
}

static pair<const char *, const char *>	get_type_for_entity(WED_Thing * ent)
{
	WED_Thing * p = ent->GetParent();
	return make_pair(ent->GetClass(), p ? p->GetClass() : NULL);
}


static pair<const char *, const char *>	get_type_for_entity(IGISEntity * ent)
{
	return get_type_for_entity(dynamic_cast<WED_Thing *>(ent));
}

static bool matches_filter(const char * child_type, const char * parent_type, const char * filter)
{
	DebugAssert(child_type);
	const char * c = child_type;
	const char * f = filter;
	while(*c && *f && *c == *f)
		++f, ++c;
	
	if(*c) return false;
	if(*f && *f != '/')
		return false;
	
	if(!*f)
		return true;
	
	DebugAssert(*f == '/');
	++f;
	
	if((*f == 0) != (parent_type == NULL))
		return false;
	
	if(parent_type == NULL)
		return true;
	
	DebugAssert(*f);
	
	const char * p = parent_type;
	
	while(*p && *f && *p == *f)
		++p, ++f;
	
	return *p == 0 && *f == 0;
	
}

bool	WED_MapLayer::IsVisibleNow(IGISEntity * ent) const
{
	if(mHideFilter)
	{
		pair<const char *, const char *> me_and_daddy_type = get_type_for_entity(ent);
		for(vector<const char *>::const_iterator c = mHideFilter->begin(); c != mHideFilter->end(); ++c)
			if(matches_filter(me_and_daddy_type.first, me_and_daddy_type.second, *c))
				return false;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
		return false;
	return !e->GetHidden();
}

bool	WED_MapLayer::IsLockedNow(IGISEntity * ent) const
{
	if(mLockFilter)
	{
		pair<const char *, const char *> me_and_daddy_type = get_type_for_entity(ent);
		for(vector<const char *>::const_iterator c = mLockFilter->begin(); c != mLockFilter->end(); ++c)
			if(matches_filter(me_and_daddy_type.first, me_and_daddy_type.second, *c))
				return true;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
		return false;
	return e->GetLocked();
}

bool	WED_MapLayer::IsVisibleNow(WED_Thing * ent) const
{
	if(mHideFilter)
	{
		pair<const char *, const char *> me_and_daddy_type = get_type_for_entity(ent);
		for(vector<const char *>::const_iterator c = mHideFilter->begin(); c != mHideFilter->end(); ++c)
			if(matches_filter(me_and_daddy_type.first, me_and_daddy_type.second, *c))
				return false;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
		return false;
	return !e->GetHidden();
}

bool	WED_MapLayer::IsLockedNow(WED_Thing * ent) const
{
	if(mLockFilter)
	{
		pair<const char *, const char *> me_and_daddy_type = get_type_for_entity(ent);
		for(vector<const char *>::const_iterator c = mLockFilter->begin(); c != mLockFilter->end(); ++c)
			if(matches_filter(me_and_daddy_type.first, me_and_daddy_type.second, *c))
				return true;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
		return false;
	return e->GetLocked();
}
