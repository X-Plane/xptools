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
#include "mathutils.h"
#include "GUI_Resources.h"
#include "WED_UIMeasurements.h"

#include "WED_MapZoomerNew.h"

WED_MapLayer::WED_MapLayer(GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) : mZoomer(z), mResolver(i), mHost(h)
{
	int dims[2];

	GUI_GetImageResourceSize("map_airport.png",dims);
	mAirportRadius = 0.5 * (double)(intmin2(dims[0],dims[1]));

	GUI_GetImageResourceSize("map_towerview.png",dims);
	mFurnitureRadius = 0.5 * (double)(intmin2(dims[0],dims[1]));
	
	mAirportFactor = WED_UIMeasurement("airport_icon_scale");
	mFurnitureFactor = WED_UIMeasurement("furniture_icon_scale");
	
	mAirportTransWidth = WED_UIMeasurement("airport_trans_width");
	
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

