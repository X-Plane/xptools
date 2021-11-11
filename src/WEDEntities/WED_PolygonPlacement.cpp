/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_PolygonPlacement.h"
#include "WED_EnumSystem.h"
#include "WED_ToolUtils.h"
#include "WED_LibraryMgr.h"

DEFINE_PERSISTENT(WED_PolygonPlacement)
TRIVIAL_COPY(WED_PolygonPlacement,WED_GISPolygon)

WED_PolygonPlacement::WED_PolygonPlacement(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	heading (this,PROP_Name("Heading", XML_Name("polygon_placement","heading")),10.0,3,1),
	resource(this,PROP_Name("Resource",XML_Name("polygon_placement","resource")),"")
{
}

WED_PolygonPlacement::~WED_PolygonPlacement()
{
}

double WED_PolygonPlacement::GetHeading(void) const
{
	return heading.value;
}

void WED_PolygonPlacement::SetHeading(double h)
{
	heading = h;
}

void		WED_PolygonPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_PolygonPlacement::SetResource(const string& r)
{
	resource = r;
}

// --------- senak in extra property that allows to detect and change to known taxi surface styles ------------

int			WED_PolygonPlacement::CountProperties(void) const
{
	return WED_GISPolygon::CountProperties() + 1;
}

int			WED_PolygonPlacement::FindProperty(const char * in_prop) const
{
	if(string(in_prop) ==  "= Taxi Surface")
		return CountProperties();
	else
		return WED_GISPolygon::FindProperty(in_prop);
}

void		WED_PolygonPlacement::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (n >= WED_GISPolygon::CountProperties())
	{
		info.can_delete = 0;
		info.can_edit  = 1;
		info.prop_kind = prop_Enum;
		info.prop_name = "= Taxi Surface";
		info.synthetic = 1;
		info.domain    = Surface_Type;
		info.exclusive = 1;
	}
	else
		WED_GISPolygon::GetNthPropertyInfo(n, info);
}

void		WED_PolygonPlacement::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	if (n >= WED_GISPolygon::CountProperties())
	{
		map<int, string>		dm;

		DOMAIN_Members(Surface_Type,dm);

		string dummy;
		for(map<int, string>::iterator i = dm.begin(); i != dm.end(); ++i)
		{
			bool surfAvail = WED_GetLibraryMgr(GetArchive()->GetResolver())->GetSurfVpath(i->first, dummy);
			dict.insert(PropertyDict_t::value_type(i->first, make_pair(i->second, surfAvail)));
		}
	}
	else
		WED_GISPolygon::GetNthPropertyDict(n, dict);
}

void		WED_PolygonPlacement::GetNthPropertyDictItem(int n, int e, string& item) const
{
	if (n >= WED_GISPolygon::CountProperties())
	{
		if (e>0)
			item = ENUM_Desc(e);
		else
			item = " ";  // show a blank selection (rather than 'None' when the resource matches no known surface type
	}
	else
		WED_GISPolygon::GetNthPropertyDictItem(n, e, item);
}

void		WED_PolygonPlacement::GetNthProperty(int n, PropertyVal_t& val) const
{
	if (n >= WED_GISPolygon::CountProperties())
	{
			val.prop_kind = prop_Enum;
			val.int_val = WED_GetLibraryMgr(GetArchive()->GetResolver())->GetSurfEnum(resource.value);
	}
	else
		WED_GISPolygon::GetNthProperty(n, val);
}

void		WED_PolygonPlacement::SetNthProperty(int n, const PropertyVal_t& val)
{
	if (n >= WED_GISPolygon::CountProperties())
	{
//		int surftype = ENUM_Export(*(val.int_val));
//		printf("SET int_val %d set_val.begin %d (%ld) linetype %d\n",val.int_val, *(val.set_val.cbegin()), val.set_val.size(), linetype);
#if WED
		string vpath;
		if(WED_GetLibraryMgr(GetArchive()->GetResolver())->GetSurfVpath(val.int_val, vpath))
			resource = vpath;
#endif
	}
	else
		WED_GISPolygon::SetNthProperty(n, val);
}
