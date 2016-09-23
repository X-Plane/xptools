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

static bool matches_filter(WED_Thing * thing ,const  vector<const char *> * filter )
{
	for(vector<const char *>::const_iterator filterit = filter->begin(); filterit != filter->end(); ++filterit)
	{
		const char * p = *filterit;
		int slash_count = 0;
		while (*p != 0 )
		{
			if (*p == '/') ++slash_count;
			++p;
		}

		string type = thing->GetClass();
		WED_Thing * parent = thing;
		for (unsigned int i = 0 ; i < slash_count; ++i)
		{
			parent = parent->GetParent();
			if(parent == NULL)
			{
				return false;
			}
			else
			{
				type += '/';
				string parent_type = parent->GetClass();
				//Handles nesting for CSS selection type filters
				if( parent_type == "WED_Airport"  || parent_type == "WED_Group" )
				{
					parent_type = "WED_GISComposite";
				}
				type += parent_type;
			}
		}

		if(type.length() == strlen(*filterit))
			if(type == *filterit)
				return true;
	}

	return false;
}

bool	WED_MapLayer::IsVisibleNow(IGISEntity * ent) const
{
	if(mHideFilter)
	{
		if( matches_filter(dynamic_cast<WED_Thing *>(ent), mHideFilter ))
			return false;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
	{
		return false;
	}
	return !e->GetHidden();
}

bool	WED_MapLayer::IsLockedNow(IGISEntity * ent) const
{
	if(mLockFilter)
	{
		if( matches_filter(dynamic_cast<WED_Thing *>(ent), mLockFilter ))
			return true;
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
	{
		return false;
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
