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

#include "WED_LinePlacement.h"
#include "WED_EnumSystem.h"
#include "WED_ToolUtils.h"
#include "WED_LibraryMgr.h"

DEFINE_PERSISTENT(WED_LinePlacement)
TRIVIAL_COPY(WED_LinePlacement,WED_GISChain)

WED_LinePlacement::WED_LinePlacement(WED_Archive * a, int i) : WED_GISChain(a,i),
	closed  (this,PROP_Name("Closed",   XML_Name("line_placement","closed")),0),
	resource(this,PROP_Name("Resource", XML_Name("line_placement","resource")),"")
{
}

WED_LinePlacement::~WED_LinePlacement()
{
}

bool WED_LinePlacement::IsClosed(void) const
{
	return closed.value;
}

void WED_LinePlacement::SetClosed(int h)
{
	closed = h;
}

void		WED_LinePlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_LinePlacement::SetResource(const string& r)
{
	resource = r;
}

// --------- senak in extra property that allows to modify known line styles via the line style GUI ------------------

int			WED_LinePlacement::CountProperties(void) const
{
	return WED_GISChain::CountProperties() + 1;
}

int			WED_LinePlacement::FindProperty(const char * in_prop) const
{
	if(string(in_prop) ==  "= Airport Line")
		return CountProperties();
	else
		return WED_GISChain::FindProperty(in_prop);
}

void		WED_LinePlacement::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (n >= WED_GISChain::CountProperties())
	{
		info.can_delete = 0;
		info.can_edit  = 1;
		info.prop_kind = prop_EnumSet;
		info.prop_name = "= Airport Line";
		info.synthetic = 1;
		info.domain    = LinearFeature;
		info.exclusive = 1;
	}
	else
		WED_GISChain::GetNthPropertyInfo(n, info);
}

void		WED_LinePlacement::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	if (n >= WED_GISChain::CountProperties())
	{
		map<int, string>		dm;

		DOMAIN_Members(LinearFeature,dm);

		for(map<int, string>::iterator i = dm.begin(); i != dm.end(); ++i)
		{
			if(ENUM_Export(i->first) < 100)                                                // filter out the light styles
				dict.insert(PropertyDict_t::value_type(i->first, make_pair(i->second,true)));
		}
	}
	else
		WED_GISChain::GetNthPropertyDict(n, dict);
}

void		WED_LinePlacement::GetNthPropertyDictItem(int n, int e, string& item) const
{
	if (n >= WED_GISChain::CountProperties())
	{
		if (e>0)
			item = ENUM_Desc(e);
		else
			item = " ";  // show a blank selection (rather than 'None' when the resource matches no known airport marking line
	}
	else
		WED_GISChain::GetNthPropertyDictItem(n, e, item);
}

void		WED_LinePlacement::GetNthProperty(int n, PropertyVal_t& val) const
{
	if (n >= WED_GISChain::CountProperties())
	{
			val.prop_kind = prop_EnumSet;
			val.set_val.clear();
			if(resource.value.size() > strlen("lib/airport/lines/") && resource.value[strlen("lib/airport/lines/")] >= '0' && resource.value[strlen("lib/airport/lines/")] <= '9')
			{
				int linetype;
				if(sscanf(resource.value.c_str(),"lib/airport/lines/%d",&linetype) == 1)
				{
					int e = ENUM_Import(LinearFeature,linetype);
					val.set_val.insert(e);
					return;
				}
			}
			val.set_val.insert(-1);
	}
	else
		WED_GISChain::GetNthProperty(n, val);
}

void		WED_LinePlacement::SetNthProperty(int n, const PropertyVal_t& val)
{
	if (n >= WED_GISChain::CountProperties())
	{
		int linetype = ENUM_Export(*(val.set_val.cbegin()));

//		printf("SET int_val %d set_val.begin %d (%ld) linetype %d\n",val.int_val, *(val.set_val.cbegin()), val.set_val.size(), linetype);
#if WED
		IResolver * resolver = GetArchive()->GetResolver();
		if (resolver)
		{
			string vpath;
			if(WED_GetLibraryMgr(resolver)->GetLineVpath(linetype, vpath))
				resource = vpath;
		}
#endif
	}
	else
		WED_GISChain::SetNthProperty(n, val);
}
