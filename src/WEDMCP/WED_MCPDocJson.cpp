/*
 * Copyright (c) 2026, Laminar Research.
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

#include "WED_MCPDocJson.h"
#include "WED_Thing.h"
#include "WED_Airport.h"
#include "WED_AirportChain.h"
#include "WED_Root.h"
#include "WED_Select.h"
#include "WED_KeyObjects.h"
#include "WED_EnumSystem.h"
#include "WED_Globals.h"
#include "IGIS.h"

#include <cmath>

// Lengths are stored in meters; WED_PropDoubleTextMeters converts to feet when the UI is in feet.  We always want
// the stored value, so switch the UI units off for the duration of a dump or inject.  Nothing redraws in between.
class	meters_please {
public:
	meters_please() : mWas(gIsFeet) { gIsFeet = 0; }
	~meters_please() { gIsFeet = mWas; }
private:
	int mWas;
};

// The persisted properties of an object: every item except the synthetic ones and the class-name field, keyed
// by its earth.wed.xml name.
static bool	is_persisted(WED_PropertyItem * item, PropertyInfo_t& info)
{
	if (dynamic_cast<WED_TypeField *>(item))
		return false;
	info.synthetic = 0;
	item->GetPropertyInfo(info);
	return !info.synthetic;
}

static string	prop_key(WED_PropertyItem * item)
{
	string key(item->GetXmlName());
	const char * attr = item->GetXmlAttrName();
	if (attr && *attr)
		key += string(".") + attr;
	return key;
}

static Json::Value	value_to_json(const PropertyInfo_t& info, const PropertyVal_t& val, bool file_precision)
{
	switch(info.prop_kind) {
	case prop_Int:
	case prop_RoadType:
		return Json::Value(val.int_val);
	case prop_Bool:
		return Json::Value(val.int_val != 0);
	case prop_Double:
		if (file_precision && info.decimals > 0)
		{
			double scale = pow(10.0, info.decimals);
			return Json::Value(round(val.double_val * scale) / scale);
		}
		return Json::Value(val.double_val);
	case prop_String:
	case prop_FilePath:
	case prop_TaxiSign:
		return Json::Value(val.string_val);
	case prop_Enum:
		return Json::Value(ENUM_Desc(val.int_val));
	case prop_EnumSet:
		{
			Json::Value a(Json::arrayValue);
			for(set<int>::const_iterator e = val.set_val.begin(); e != val.set_val.end(); ++e)
				a.append(ENUM_Desc(*e));
			return a;
		}
	default:
		return Json::Value();
	}
}

static Json::Value	valid_enums(WED_PropertyItem * item)
{
	PropertyDict_t dict;
	item->GetPropertyDict(dict);
	Json::Value v(Json::arrayValue);
	for(PropertyDict_t::iterator d = dict.begin(); d != dict.end(); ++d)
		v.append(ENUM_Desc(d->first));
	return v;
}

static bool	json_to_value(WED_PropertyItem * item, const PropertyInfo_t& info, const string& key, const Json::Value& j, PropertyVal_t& val, WED_MCPError& err)
{
	val.prop_kind = info.prop_kind;
	err.code = "invalid_value_type";
	switch(info.prop_kind) {
	case prop_Int:
	case prop_RoadType:
		if (!j.isIntegral() || j.isBool()) { err.message = "property " + key + " takes an integer"; return false; }
		val.int_val = j.asInt();
		return true;
	case prop_Bool:
		if (!j.isBool()) { err.message = "property " + key + " takes a boolean"; return false; }
		val.int_val = j.asBool() ? 1 : 0;
		return true;
	case prop_Double:
		if (!j.isNumeric() || j.isBool()) { err.message = "property " + key + " takes a number"; return false; }
		val.double_val = j.asDouble();
		return true;
	case prop_String:
	case prop_FilePath:
	case prop_TaxiSign:
		if (!j.isString()) { err.message = "property " + key + " takes a string"; return false; }
		val.string_val = j.asString();
		return true;
	case prop_Enum:
		{
			int e = j.isString() ? ENUM_LookupDesc(info.domain, j.asString().c_str()) : -1;
			if (e == -1)
			{
				err.code = "invalid_enum_value";
				err.message = "property " + key + " takes one of the listed enum strings";
				err.extra["valid"] = valid_enums(item);
				return false;
			}
			val.int_val = e;
			return true;
		}
	case prop_EnumSet:
		{
			if (!j.isArray()) { err.message = "property " + key + " takes an array of enum strings"; return false; }
			val.set_val.clear();
			for(Json::ArrayIndex n = 0; n < j.size(); ++n)
			{
				int e = j[n].isString() ? ENUM_LookupDesc(info.domain, j[n].asString().c_str()) : -1;
				if (e == -1)
				{
					err.code = "invalid_enum_value";
					err.message = "property " + key + " takes an array of the listed enum strings";
					err.extra["valid"] = valid_enums(item);
					return false;
				}
				val.set_val.insert(e);
			}
			return true;
		}
	default:
		err.code = "unsupported_property";
		err.message = "property " + key + " can't be set";
		return false;
	}
}

#pragma mark -

struct	dump_ctx {
	const WED_MCPDumpOptions *	opts;
	map<int, Json::Value> *		names;
	int							next_ref;
};

static void	assign_names(WED_Thing * t, int depth, dump_ctx& ctx)
{
	(*ctx.names)[t->GetID()] = ctx.opts->ids ? Json::Value(t->GetID()) : Json::Value("o" + to_string(ctx.next_ref++));
	if (ctx.opts->max_depth < 0 || depth < ctx.opts->max_depth)
		for(int c = 0; c < t->CountChildren(); ++c)
			assign_names(t->GetNthChild(c), depth + 1, ctx);
}

static Json::Value	dump_thing(WED_Thing * t, int depth, dump_ctx& ctx)
{
	Json::Value o(Json::objectValue);
	if (ctx.opts->ids)
		o["id"] = t->GetID();
	else
		o["ref"] = (*ctx.names)[t->GetID()];
	o["class"] = t->GetClass();

	Json::Value props(Json::objectValue);
	for(int n = 0; n < t->mItems.size(); ++n)
	{
		WED_PropertyItem * item = t->mItems[n];
		PropertyInfo_t info;
		if (!is_persisted(item, info))
			continue;
		PropertyVal_t val;
		item->GetProperty(val);
		props[prop_key(item)] = value_to_json(info, val, ctx.opts->file_precision);
	}
	o["props"] = props;

	// Exact-class matches: this is per-class persisted state (their AddExtraXML), not a GIS category.
	Json::Value extra(Json::objectValue);
	if (t->GetClass() == WED_Airport::sClass)
	{
		const vector<WED_Airport::meta_data_entry>& md = static_cast<WED_Airport *>(t)->GetMetaData();
		Json::Value m(Json::objectValue);
		for(int n = 0; n < md.size(); ++n)
			m[md[n].first] = md[n].second;
		extra["meta_data"] = m;
	}
	else if (t->GetClass() == WED_AirportChain::sClass)
		extra["closed"] = static_cast<WED_AirportChain *>(t)->IsClosed();
	if (!extra.empty())
		o["extra"] = extra;

	if (t->CountSources() > 0)
	{
		Json::Value s(Json::arrayValue);
		for(int n = 0; n < t->CountSources(); ++n)
		{
			WED_Thing * src = t->GetNthSource(n);
			map<int, Json::Value>::iterator nm = ctx.names->find(src->GetID());
			s.append(nm != ctx.names->end() ? nm->second : Json::Value(src->GetID()));	// outside the dump: plain ID
		}
		o["sources"] = s;
	}

	if (ctx.opts->geo)
	{
		IGISEntity * e = dynamic_cast<IGISEntity *>(t);
		if (e)
		{
			Bbox2 b;
			e->GetBounds(gis_Geo, b);
			Json::Value g;
			g["gis_class"] = (int) e->GetGISClass();
			g["bounds"].append(b.xmin()); g["bounds"].append(b.ymin());
			g["bounds"].append(b.xmax()); g["bounds"].append(b.ymax());
			o["geo"] = g;
		}
	}

	if (t->CountChildren() > 0)
	{
		if (ctx.opts->max_depth >= 0 && depth >= ctx.opts->max_depth)
			o["child_count"] = t->CountChildren();
		else
		{
			Json::Value kids(Json::arrayValue);
			for(int c = 0; c < t->CountChildren(); ++c)
				kids.append(dump_thing(t->GetNthChild(c), depth + 1, ctx));
			o["children"] = kids;
		}
	}
	return o;
}

Json::Value	WED_MCP_DumpTree(WED_Thing * root, const WED_MCPDumpOptions& opts, map<int, Json::Value>& out_names)
{
	meters_please m;
	out_names.clear();
	dump_ctx ctx = { &opts, &out_names, 1 };
	assign_names(root, 0, ctx);
	return dump_thing(root, 0, ctx);
}

#pragma mark -

// Structural singletons every document already has exactly one of.
static bool	injectable_class(const string& c)
{
	return c != WED_Root::sClass && c != WED_Select::sClass && c != WED_KeyObjects::sClass;
}

bool	WED_MCP_SetProperties(WED_Thing * t, const Json::Value& props, WED_MCPError& err)
{
	meters_please m;
	if (!props.isObject())
	{
		err.code = "invalid_argument";
		err.message = "props must be an object";
		return false;
	}
	for(Json::Value::const_iterator p = props.begin(); p != props.end(); ++p)
	{
		string key = p.name();
		WED_PropertyItem * found = nullptr;
		PropertyInfo_t found_info;
		Json::Value valid(Json::arrayValue);
		for(int n = 0; n < t->mItems.size(); ++n)
		{
			PropertyInfo_t info;
			if (!is_persisted(t->mItems[n], info))
				continue;
			string k = prop_key(t->mItems[n]);
			valid.append(k);
			if (k == key)
			{
				found = t->mItems[n];
				found_info = info;
			}
		}
		if (!found)
		{
			err.code = "unknown_property";
			err.message = string(t->GetClass()) + " has no property '" + key + "'";
			err.extra["valid"] = valid;
			return false;
		}
		PropertyVal_t val;
		if (!json_to_value(found, found_info, key, *p, val, err))
			return false;
		found->SetProperty(val, t);
	}
	return true;
}

struct	pending_sources_t {
	WED_Thing *		thing;
	Json::Value		sources;
};

static WED_Thing *	create_one(WED_Thing * parent, int position, const Json::Value& o, map<string,int>& refs,
									vector<pending_sources_t>& sources, WED_MCPError& err)
{
	static const char * kKeys[] = { "class", "props", "extra", "sources", "ref", "children", "id", "geo", nullptr };

	if (!o.isObject())
	{
		err.code = "invalid_fixture";
		err.message = "each object must be a JSON object";
		return nullptr;
	}
	for(Json::Value::const_iterator k = o.begin(); k != o.end(); ++k)
	{
		bool ok = false;
		for(const char ** kk = kKeys; *kk; ++kk)
			if (k.name() == *kk)
				ok = true;
		if (!ok)
		{
			err.code = "invalid_fixture";
			err.message = k.name() == "child_count"
				? "object has child_count but no children - it was cut off by max_depth when dumped"
				: "unknown object key '" + k.name() + "' (keys are class, props, extra, sources, ref, children)";
			return nullptr;
		}
	}

	string cls = o.get("class", "").asString();
	WED_Archive * archive = parent->GetArchive();
	WED_Persistent * p = injectable_class(cls) ? WED_Persistent::CreateByClass(cls.c_str(), archive, archive->NewID()) : nullptr;
	WED_Thing * t = dynamic_cast<WED_Thing *>(p);
	if (!t)
	{
		if (p) p->Delete();
		err.code = "unknown_class";
		err.message = "cannot create an object of class '" + cls + "'";
		return nullptr;
	}
	t->SetParent(parent, position);

	if (o.isMember("ref"))
	{
		string ref = o["ref"].asString();
		if (refs.count(ref))
		{
			err.code = "invalid_fixture";
			err.message = "duplicate ref '" + ref + "'";
			return nullptr;
		}
		refs[ref] = t->GetID();
	}

	if (o.isMember("props") && !WED_MCP_SetProperties(t, o["props"], err))
		return nullptr;

	const Json::Value& extra = o["extra"];
	if (!extra.isNull())
	{
		for(Json::Value::const_iterator e = extra.begin(); e != extra.end(); ++e)
		{
			if (cls == WED_Airport::sClass && e.name() == "meta_data" && e->isObject())
			{
				WED_Airport * apt = static_cast<WED_Airport *>(t);
				apt->StateChanged();		// AddMetaDataKey doesn't record undo state itself
				for(Json::Value::const_iterator m = e->begin(); m != e->end(); ++m)
					apt->AddMetaDataKey(m.name(), m->asString());
			}
			else if (cls == WED_AirportChain::sClass && e.name() == "closed" && e->isBool())
				static_cast<WED_AirportChain *>(t)->SetClosed(e->asBool());
			else
			{
				err.code = "invalid_fixture";
				err.message = "extra '" + e.name() + "' is not valid for " + cls;
				return nullptr;
			}
		}
	}

	if (o.isMember("sources"))
	{
		pending_sources_t ps = { t, o["sources"] };
		sources.push_back(ps);
	}

	const Json::Value& kids = o["children"];
	for(Json::ArrayIndex c = 0; c < kids.size(); ++c)
		if (!create_one(t, c, kids[c], refs, sources, err))
			return nullptr;

	return t;
}

bool	WED_MCP_InjectObjects(WED_Thing * parent, int position, const Json::Value& objects,
								map<string,int>& out_refs, vector<int>& out_ids, WED_MCPError& err)
{
	meters_please m;
	if (!objects.isArray())
	{
		err.code = "invalid_argument";
		err.message = "objects must be an array";
		return false;
	}

	// Sources are wired after everything exists, so a fixture can reference objects in any order.
	vector<pending_sources_t> sources;
	for(Json::ArrayIndex n = 0; n < objects.size(); ++n)
	{
		WED_Thing * t = create_one(parent, position + n, objects[n], out_refs, sources, err);
		if (!t)
			return false;
		out_ids.push_back(t->GetID());
	}

	WED_Archive * archive = parent->GetArchive();
	for(int n = 0; n < sources.size(); ++n)
	{
		const Json::Value& s = sources[n].sources;
		for(Json::ArrayIndex i = 0; i < s.size(); ++i)
		{
			WED_Thing * src = nullptr;
			if (s[i].isString())
			{
				map<string,int>::iterator r = out_refs.find(s[i].asString());
				if (r != out_refs.end())
					src = dynamic_cast<WED_Thing *>(archive->Fetch(r->second));
			}
			else if (s[i].isIntegral())
				src = dynamic_cast<WED_Thing *>(archive->Fetch(s[i].asInt()));
			if (!src)
			{
				err.code = "invalid_fixture";
				err.message = "source " + s[i].toStyledString() + " is neither a ref in this fixture nor an existing object ID";
				return false;
			}
			sources[n].thing->AddSource(src, i);
		}
	}
	return true;
}
