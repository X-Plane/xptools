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
#include "IGIS.h"
#include "WED_Entity.h"

#include "WED_Airport.h"
#include "WED_Group.h"
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

	// This is the scale of the icons for airports themselves.  It is a multiplier.
	// When this is "1" one pixel of the airport icon = one meter on the map.
	mAirportFactor = 20.0;
	
	// This is the scale of the icons for all of the parts of an airport - VASI lights,
	// signs, windsocks, parking spots.  It is a multiplier.
	// When this is "1" one pixel of the airport icon = one meter on the map.
	mFurnitureFactor = 1.0;
	
	// This is the width at which we transition from full airports to icons
	mAirportTransWidth = 20.0;
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

void		WED_MapLayer::SetFilter(const MapFilter_t * hide_filter_ptr, const MapFilter_t * lock_filter_ptr)
{
	mHideFilter = hide_filter_ptr;
	mLockFilter = lock_filter_ptr;
}

static bool matches_filter(WED_Thing * thing ,const  MapFilter_t * filter )
{
	if(thing == NULL) return false;
	for(MapFilter_t::const_iterator filterit = filter->begin(); filterit != filter->end(); ++filterit)
	{
		bool match = true;
		WED_Thing * parent = thing;
		for(auto &i: filterit->e)
		{
			const char * type = parent->GetClass();
			if( type == WED_Airport::sClass ) type = WED_Group::sClass;
			if (!(type == i)) { match = false; break; }
			parent = parent->GetParent();
			if (parent == NULL) { match = false; break; }
		}
		if (match) return true;
	}
	return false;
}

bool	WED_MapLayer::IsVisibleNow(IGISEntity * ent) const
{
	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if (!e)
		return false;
	if(mHideFilter)
	{
		if( matches_filter(e, mHideFilter ))
			return false;
	}
	return !e->GetHidden();
}

bool	WED_MapLayer::IsLockedNow(IGISEntity * ent) const
{
	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if (!e)
		return false;
	if(mLockFilter)
	{
		if( matches_filter(e, mLockFilter ))
			return true;
	}
	return e->GetLocked();
}

bool	WED_MapLayer::IsVisibleNow(WED_Thing * ent) const
{
	if(mHideFilter)
	{
		if( matches_filter(ent, mHideFilter ))
			return false;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
	{
		return false;
	}
	return !e->GetHidden();
}

bool	WED_MapLayer::IsLockedNow(WED_Thing * ent) const
{
	if(mLockFilter)
	{
		if( matches_filter(ent, mLockFilter ))
			return true;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
	{
		return false;
	}
	return e->GetLocked();
}
