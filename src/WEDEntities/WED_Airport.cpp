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

#include "WED_Airport.h"
#include "IODefs.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"
#include "XESConstants.h"
#include "WED_XMLWriter.h"

#if DEV
#include <random>
#include <iostream>
#endif

DEFINE_PERSISTENT(WED_Airport)
TRIVIAL_COPY(WED_Airport, WED_GISComposite)


enum wed_airport_props
{
	airport_prop_airport_type,
	airport_prop_field_elevation,
	airport_prop_has_atc,
	airport_prop_icao_id,
	airport_prop_scenery_id,
	airport_prop_meta_data, //For our c_str purposes, this is slot 5. For the properties purposes
							//Meta Data appears at the end
	airport_prop_count
};

const char * airport_props_strs[airport_prop_count] =
{
	"Type",			
	"Field Elevation",
	"Has ATC",
	"ICAO Identifier",
	"Scenery ID",
	"Meta Data"
};

WED_Airport::WED_Airport(WED_Archive * a, int i) : WED_GISComposite(a,i),
	airport_type	(this, "Type",				SQL_Name("WED_airport",	"kind"),		XML_Name("airport",	"kind"),		Airport_Type, type_Airport),
	elevation		(this, "Field Elevation",	SQL_Name("WED_airport",	"elevation"),	XML_Name("airport",	"elevation"),	0,6,1),
	has_atc			(this, "Has ATC",			SQL_Name("WED_airport",	"has_atc"),		XML_Name("airport",	"has_atc"),		1),
	icao			(this, "ICAO Identifier",	SQL_Name("WED_airport",	"icao"),		XML_Name("airport",	"icao"),		"xxxx"),
	scenery_id		(this, "Scenery ID",		SQL_Name("WED_airport", "scenery_id"),	XML_Name("airport", "scenery_id"), -1, 8),
	meta_data_hashmap ()
{
}

WED_Airport::~WED_Airport()
{
}

void	WED_Airport::GetICAO(string& i) const
{
	i = icao.value;
}

int		WED_Airport::GetAirportType(void) const
{
	return airport_type.value;
}

int		WED_Airport::GetSceneryID(void) const
{
	return scenery_id.value;
}

void		WED_Airport::SetAirportType(int x) { airport_type = x; }
void		WED_Airport::SetElevation(double x) { elevation = x; }
void		WED_Airport::SetHasATC(int x) { has_atc= x; }
void		WED_Airport::SetICAO(const string& x) { icao = x; }
void		WED_Airport::SetSceneryID(int x) { scenery_id = x; }

void		WED_Airport::Import(const AptInfo_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	airport_type = ENUM_Import(Airport_Type, info.kind_code);
	if (airport_type == -1)
	{
		print_func(ref,"Error importing airport: airport type code %d is illegal (not a member of type %s).\n", info.kind_code, DOMAIN_Desc(airport_type.domain));
		airport_type = type_Airport;
	}

	elevation = info.elevation_ft * FT_TO_MTR;
	has_atc = info.has_atc_twr;
	icao = info.icao;
	SetName(info.name);
}

void		WED_Airport::Export(AptInfo_t& info) const
{
	info.kind_code = ENUM_Export(airport_type);
	info.elevation_ft = elevation.value * MTR_TO_FT;
	info.has_atc_twr = has_atc;
	info.icao = icao.value;
	GetName(info.name);
	info.default_buildings = 0;	// deprecated field.  Not supported in x-plane so not supported in WED!

	Bbox2	bounds;
	GetBounds(gis_Geo, bounds);
	info.tower.location = info.beacon.location = Segment2(bounds.p1,bounds.p2).midpoint();
	info.tower.draw_obj = -1;
	info.tower.height_ft = 50.0;
	info.beacon.color_code = apt_beacon_none;
}

// IPropertyObject
int			WED_Airport::FindProperty(const char * in_prop) const
{
	//Test if the property name is one of the property strings
	if (strcmp(in_prop, airport_props_strs[airport_prop_meta_data])==0)
	{
		return WED_Airport::CountProperties() - 1;
	}
	else
	{
		//If it wasn't found before, try it's parent, WED_GISComposite.
		return WED_Airport::WED_GISComposite::FindProperty(in_prop);
	}
}

int			WED_Airport::CountProperties(void) const
{
#if DEV && 0
	std::cout << "Number of properties" << endl;
	std::cout << WED_GISComposite::CountProperties() + 1 << endl
	//+1 for the 1 synthetic value
#endif
	return WED_GISComposite::CountProperties() + 1;
}

void		WED_Airport::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
#if 0
	cout << "GetNthPropertyInfo:" << n << endl;
#endif
	//Meta Data is the last slot
	if (n == WED_Airport::CountProperties() - 1)
	{
		info.can_edit = true;
		info.prop_name = airport_props_strs[airport_prop_meta_data];
		info.prop_kind = prop_String;
		info.synthetic = true;
	}
	else
	{
		WED_GISComposite::GetNthPropertyInfo(n, info);
	}
}

//WED_Airport contains no WED_PropItems that requires it
void		WED_Airport::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_GISComposite::GetNthPropertyDict(n, dict);
}

void		WED_Airport::GetNthPropertyDictItem(int n, int e, string& item) const
{
	WED_GISComposite::GetNthPropertyDictItem(n, e, item);
}

void		WED_Airport::GetNthProperty(int n, PropertyVal_t& val) const
{
#if 0
	cout << "GetNthProperty:" << n << endl;
#endif
	//Meta Data is the last slot
	if (n == WED_Airport::CountProperties() - 1)
	{
		for(map<string,string>::const_iterator itr = meta_data_hashmap.begin(); itr != meta_data_hashmap.end(); ++itr)
		{
#if DEV
			cout << val.string_val << endl;
			val.string_val += itr->first + ", " + itr->second + "|";
			cout << val.string_val << endl;
#endif
		}
	}
	else
	{
		WED_GISComposite::GetNthProperty(n, val);
	}
}

void		WED_Airport::SetNthProperty(int n, const PropertyVal_t& val)
{
	cout << "SetNthProperty: " << n << endl;
	//This SetNthProperty is just asking about wed_airport_props
	if(n == WED_Airport::CountProperties() - 1)
	{
#if DEV
		char rand_key[12];
		itoa(rand(),rand_key,10);

		char rand_val[12];
		itoa(rand(),rand_val,10);
		//For now, whenever you try and change it you insert random_int, val.string_val
		meta_data_hashmap.insert(meta_data_entry(rand_key, rand_val));
#endif
	}
	else
	{
		WED_GISComposite::SetNthProperty(n, val);
	}
}

void 			WED_Airport::ReadFrom(IOReader * reader)
{
	WED_GISComposite::ReadFrom(reader);
	
	meta_data_hashmap.clear();

	//Loop counter to fill the hashmap back up
	int i;
	reader->ReadInt(i);
	
	while (i--)
	{
		//Read the key string length, then string
		int key_len;
		reader->ReadInt(key_len);
		
		string key(key_len,'\0');
		reader->ReadBulk(&(*key.begin()), key_len, false);
		
		//Read the value string length, then string
		int value_len;
		reader->ReadInt(value_len);

		string val (value_len, '\0');
		reader->ReadBulk (&(*val.begin()), value_len, false);
		
		meta_data_hashmap.insert(meta_data_entry(key,val));
	}
}

void 			WED_Airport::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	
	//Write the hashmap size
	writer->WriteInt(meta_data_hashmap.size());
	for (map<string,string>::iterator it = meta_data_hashmap.begin(); it != meta_data_hashmap.end(); ++it)
	{
		//Write the key string length and c-string
		writer->WriteInt(it->first.size());
		writer->WriteBulk(it->first.c_str(), it->first.length(), false);

		//Write the value string length and c-string
		writer->WriteInt(it->second.size());
		writer->WriteBulk(it->second.c_str(), it->second.length(), false);
	}
}

void 			WED_Airport::AddExtraXML(WED_XMLElement * obj)
{
	WED_GISComposite::AddExtraXML(obj);

	WED_XMLElement * xml = obj->add_sub_element("meta_data");
	for(map<string,string>::iterator i = meta_data_hashmap.begin(); i != meta_data_hashmap.end(); ++i)
	{
		
		WED_XMLElement * c = xml->add_sub_element("meta_data_entry");
		
		//Due to the fact we don't know what meta_data key we're going to have
		//We have to save it as "pair","FAA,BDL". Sadly, its not a one to one mapping
		//of Data Structure and Data Format, and is a bit redundant
		c->add_attr_stl_str("pair", i->first + "," + i->second);
	}
}

void			WED_Airport::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	WED_GISComposite::StartElement(reader, name, atts);
	if(strcasecmp(name,"meta_data_entry") == 0)
	{
		const XML_Char * entry_value = get_att("pair",atts);
		if(entry_value != NULL)
		{
			//Will be something like "name_es, Talavera la Real Badajoz Airport"
			const string entry_string = string(entry_value);

			const string key = entry_string.substr( 0, entry_string.find_first_of(','));
			const string value = entry_string.substr(entry_string.find_first_of(',') + 1);
			meta_data_hashmap.insert(std::pair<string,string>( key, value));
		}
		else
		{
			reader->FailWithError("Attribute pair is missing from meta_data_entry element");
		}
	}
	else
	{
		WED_GISComposite::StartElement(reader, name, atts);
	}
}

/*
void			WED_Airport::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
}

void			WED_Airport::ToDB(sqlite3 * db)
{
}*/
