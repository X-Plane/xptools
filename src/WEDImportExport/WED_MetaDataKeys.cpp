#include "WED_MetaDataKeys.h"
#include "WED_Menus.h"
#include "AssertUtils.h"

// How to add a new Metadata Key:
// Go into WED_Menus.h and add a entry (keep alphabetically organized)
// Add the corresponding entry in known_keys
// All display names and keys must be unique, key names must not include commas

const MetaDataKey known_keys[] = {
// wed_AddMetaDataBegin
{	"City/Locality",   "city"        },         // wed_AddMetaDataCity
{	"Country",         "country"     },         // wed_AddMetaDataCountry
{	"Datum Latitude",  "datum_lat"   },         // wed_AddMetaDataDatumLat
{	"Datum Longitude", "datum_lon"   },         // wed_AddMetaDataDatumLon
{	"FAA Code",        "faa_code"    },         // wed_AddMetaDataFAA
{	"IATA Code",       "iata_code"   },         // wed_AddMetaDataIATA
{	"ICAO Code",       "icao_code"   },         // wed_AddMetaDataICAO
{	"Local Code",      "local_code"  },         // wed_AddMetaDataLocal
{	"Local Authorithy","local_authority" },     // wed_AddMetaDataLocAuth
{	"Region Code",     "region_code" },         // wed_AddMetaDataRegionCode
{	"State/Province",  "state"       },         // wed_AddMetaDataState
{	"Transition Altitude", "transition_alt"  }, // wed_AddMetaDataTransitionAlt
{	"Transition Level",    "transition_level"}  // wed_AddMetaDataTransitionLevel
// wed_AddMetaDataEnd
};

const MetaDataKey& META_KeyInfo(KeyEnum key_enum)
{
	DebugAssert(key_enum > wed_AddMetaDataBegin && key_enum < wed_AddMetaDataEnd);
	return known_keys[key_enum - (wed_AddMetaDataBegin + 1)];
}

const string& META_KeyDisplayText(KeyEnum key_enum)
{
	return META_KeyInfo(key_enum).display_text;
}

const string& META_KeyName(KeyEnum key_enum)
{
	return META_KeyInfo(key_enum).name;
}

KeyEnum META_KeyEnumFromName(const string& name_str)
{
	for(int key_enum = wed_AddMetaDataBegin + 1; key_enum < wed_AddMetaDataEnd; ++key_enum)
	{
		if(name_str == META_KeyName(key_enum))
		{
			return key_enum;
		}
	}

	return wed_AddMetaDataEnd;
}

KeyEnum META_KeyEnumFromDisplayText(const string& display_text_str)
{
	for(int key_enum = wed_AddMetaDataBegin + 1; key_enum < wed_AddMetaDataEnd; ++key_enum)
	{
		if(display_text_str == META_KeyDisplayText(key_enum))
		{
			return key_enum;
		}
	}

	return wed_AddMetaDataEnd;
}

