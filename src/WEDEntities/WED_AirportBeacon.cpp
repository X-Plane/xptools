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

#include "WED_AirportBeacon.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_AirportBeacon)
TRIVIAL_COPY(WED_AirportBeacon, WED_GISPoint)

WED_AirportBeacon::WED_AirportBeacon(WED_Archive * a, int i) : WED_GISPoint(a,i),
	kind(this,"Type", XML_Name("airport_beacon","type"),Airport_Beacon, beacon_Airport)
{
}

WED_AirportBeacon::~WED_AirportBeacon()
{
}

void		WED_AirportBeacon::SetKind(int k)
{
	kind = k;
}

void	WED_AirportBeacon::Import(const AptBeacon_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo, x.location);
	kind = ENUM_Import(Airport_Beacon, x.color_code);
	if (kind == -1)
	{
		print_func(ref,"Error importing airport beacon: beacon color code %d is illegal (not a member of type %s).\n", x.color_code, DOMAIN_Desc(kind.domain));
		kind = beacon_Airport;
	}
	SetName(x.name);
}

void	WED_AirportBeacon::Export(		 AptBeacon_t& x) const
{
	GetLocation(gis_Geo, x.location);
	x.color_code = ENUM_Export(kind.value);
	GetName(x.name);
}
