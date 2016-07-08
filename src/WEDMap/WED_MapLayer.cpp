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

static pair<string, string>	get_type_for_entity(WED_Thing * ent, int slash_count)
{
	WED_Thing * p = ent;
	string parents = "";

	int i = 0;
	do
	{
		p = p->GetParent();
		if(p == NULL)
		{
			break;
		}
		else
		{
			if(parents != "")
			{
				parents += '/';
			}
			string parent_type = p->GetClass();

			//Handles nesting for CSS selection type filters
			if((parent_type == "WED_Airport" || parent_type == "WED_Group") &&
				slash_count > 0)
			{
				parent_type = "WED_GISComposite";
			}

			parents += parent_type;
		}
		++i;
	}
	while(i < slash_count);

	return make_pair(ent->GetClass(), p ? parents : "");
}

static pair<string, string>	get_type_for_entity(IGISEntity * ent, int slash_count) //This is WED_GetParent(), not C++ parent
{
	return get_type_for_entity(dynamic_cast<WED_Thing *>(ent), slash_count);
}

static bool matches_filter(const string& child_type, const string& parent_type, const string& filter)
{
	DebugAssert(child_type != "");
	DebugAssert(filter != "");

	//If there is no / or no parent, just compare the child and filter
	if(filter.find_first_of('/') == filter.npos || parent_type == "")
	{
		return child_type == filter;
	}
	else
	{
		return (child_type + '/' + parent_type) == filter;
	}
}

bool	WED_MapLayer::IsVisibleNow(IGISEntity * ent) const
{
	if(mHideFilter)
	{
		for(vector<const char *>::const_iterator filter = mHideFilter->begin(); filter != mHideFilter->end(); ++filter)
		{
			string f(*filter);
			int slash_count = count(f.begin(), f.end(), '/');
			
			pair<string, string> leaf_and_parent = get_type_for_entity(ent, slash_count);
			if(matches_filter(leaf_and_parent.first, leaf_and_parent.second, *filter))
			{
				return false;
			}
		}
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
		for(vector<const char *>::const_iterator filter = mLockFilter->begin(); filter != mLockFilter->end(); ++filter)
		{
			string f(*filter);
			int slash_count = count(f.begin(), f.end(), '/');
			
			pair<string, string> leaf_and_parents = get_type_for_entity(ent, slash_count);
			if(matches_filter(leaf_and_parents.first, leaf_and_parents.second, *filter))
			{
				return true;
			}
		}
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
		for(vector<const char *>::const_iterator filter = mHideFilter->begin(); filter != mHideFilter->end(); ++filter)
		{
			string f(*filter);
			int slash_count = count(f.begin(), f.end(), '/');
			
			pair<string, string> leaf_and_parent = get_type_for_entity(ent, slash_count);
			if(matches_filter(leaf_and_parent.first, leaf_and_parent.second, *filter))
			{
				return false;
			}
		}
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
		for(vector<const char *>::const_iterator filter = mLockFilter->begin(); filter != mLockFilter->end(); ++filter)
		{
			string f(*filter);
			int wed_parent_count = count(f.begin(), f.end(), '/');
			
			pair<string, string> leaf_and_parents = get_type_for_entity(ent, wed_parent_count);
			if(matches_filter(leaf_and_parents.first, leaf_and_parents.second, *filter))
			{
				return true;
			}
		}
	}

	WED_Entity * e = dynamic_cast<WED_Entity *>(ent);
	if(!e)
		return false;
	return e->GetLocked();
}
