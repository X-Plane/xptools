/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_ATCFlow.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"

#if AIRPORT_ROUTING

DEFINE_PERSISTENT(WED_ATCFlow)
TRIVIAL_COPY(WED_ATCFlow, WED_Thing)

WED_ATCFlow::WED_ATCFlow(WED_Archive * a, int i) : 
	WED_Thing(a,i),
	icao(this,"METAR ICAO","WED_atcflow","icao",""),
	cld_min(this,"Minimum Ceiling","WED_atcflow","cld_min", 2500, 4),
	vis_min(this,"Minimum Visibility","WED_atcflow","vis_min", 2, 2),
	wnd_spd_max(this,"Wind Speed Maximum","WED_atcflow","wnd_spd_max", 0, 3),
	wnd_dir_min(this,"Wind Direction Minimum","WED_atcflow","wnd_dir_min", 0, 3),
	wnd_dir_max(this,"Wind Direction Maximum","WED_atcflow","wnd_dir_max", 360, 3),
	time_min(this,"Start Time (Local)","WED_atcflow","time_min", 0, 3),
	time_max(this,"End Time (Local)","WED_atcflow","time_max", 2400, 3),
	traffic_dir(this,"Pattern Direction","WED_atcflow","pattern_side",ATCPatternSide,atc_Left),
	pattern_rwy(this,"Pattern Runway", "WED_atcflow","pattern_rwy","4L")
{
	
}

WED_ATCFlow::~WED_ATCFlow()
{
}

void	WED_ATCFlow::Import(const AptFlow_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	StateChanged();
	SetName(info.name);
	icao = info.icao;
	cld_min = info.ceiling;
	vis_min = info.visibility;
	wnd_spd_max = info.wind_speed_max;
	wnd_dir_min = info.wind_dir_min;
	wnd_dir_max = info.wind_dir_max;
	time_min = info.time_min;
	time_max = info.time_max;
	traffic_dir = ENUM_Import(traffic_dir.domain, info.pattern_side);
	if(traffic_dir == -1)
	{
		traffic_dir = atc_Left;
		print_func(ref,"Error: illegal traffic pattern code %d", info.pattern_side);
	}
	pattern_rwy = info.pattern_runway;	
}

void	WED_ATCFlow::Export(		 AptFlow_t& info) const
{
	GetName(info.name);
	info.icao = icao.value;
	info.ceiling = cld_min.value;
	info.visibility = vis_min.value;
	info.wind_speed_max = wnd_spd_max.value;
	info.wind_dir_min = wnd_dir_min.value;
	info.wind_dir_max = wnd_dir_max.value;
	info.time_min = time_min.value;
	info.time_max = time_max.value;
	info.pattern_side = ENUM_Export(traffic_dir.value);
	info.pattern_runway = pattern_rwy.value;
}

#endif