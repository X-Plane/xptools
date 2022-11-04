#include "WED_MetaDataDefaults.h"
#include "WED_Airport.h"
#include "WED_MetaDataKeys.h"
#include "WED_GatewayExport.h"

#include "WED_Menus.h" // for wed_AddMetaDataBegin and wed_AddMetaDataEnd
#include "CSVParser.h"
#include <fstream>

#if DEV
#define FROM_DISK 0
	#if FROM_DISK
	#define CSV_ON_DISK "C://airport_metadata.csv"	
	#endif
#endif

bool	fill_in_airport_metadata_defaults(WED_Airport & airport, const string& file_path)
{
#if DEV && FROM_DISK
	std::ifstream t(CSV_ON_DISK);
#else
	std::ifstream t(file_path.c_str());
#endif

	if(t.bad() == true)
	{
		t.close();
		return false;
	}

	std::string str((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
	
	CSVParser::CSVTable table = CSVParser(',', str).ParseCSV();
	
	if (table.GetRows().size() < 1)                         //  See WED-701, file on server had syntax error
        LOG_MSG("E/MDDef while parsing %s !\n", file_path.c_str());  //  DoUserAlert() might be over the top

	t.close();
	return fill_in_airport_metadata_defaults(airport, table);
}

bool fill_in_airport_metadata_defaults(WED_Airport & airport, const CSVParser::CSVTable &table)
{
	CSVParser::CSVTable::CSVRow default_values;

	//Find the airport in the table match
	string icao;
	if(airport.ContainsMetaDataKey(wed_AddMetaDataICAO))
		icao = airport.GetMetaDataValue(wed_AddMetaDataICAO);
	if (icao.empty() && airport.ContainsMetaDataKey(wed_AddMetaDataFAA))
		icao = airport.GetMetaDataValue(wed_AddMetaDataFAA);
	if (icao.empty() && airport.ContainsMetaDataKey(wed_AddMetaDataLocal))
	{
		icao = airport.GetMetaDataValue(wed_AddMetaDataLocal);
		if (!icao.empty()) return false;
	}
	if(icao.empty()) airport.GetICAO(icao);

	int i = 0;
	for ( ; i < table.GetRows().size(); ++i)
	{
		if(table.GetRows()[i][0] == icao)
		{
			default_values = table.GetRows()[i];
			break;
		}
	}

	bool found_data = i < table.GetRows().size();

	if(found_data)
	{
		//For every column (excluding airport_id), copy if missing key or key's value is ""
		CSVParser::CSVTable::CSVHeader column_headers = table.GetHeader();
		for (i = 1; i < default_values.size(); i++)
		{
			string key = column_headers[i];
			string default_value = default_values[i];

			const KeyEnum key_enum = META_KeyEnumFromName(key);
			if(key_enum > wed_AddMetaDataBegin && key_enum < wed_AddMetaDataEnd) // We *only* want to insert keys we recognize
			{
				//For every of our column do They (airport) have this?
				if(airport.ContainsMetaDataKey(key))
				{
					if(airport.GetMetaDataValue(key).empty())
						airport.EditMetaDataKey(key,default_value);
				}
				else
					airport.AddMetaDataKey(key, default_value);
			}
		}
	}
	
	found_data |= Enforce_MetaDataGuiLabel(&airport);

	return found_data;
}

// these are NOT exact country names, but rather partial string suitable to differentiate them
// some have trailing spaces to facilitate end-of-word matching
// the string they are compared to also needs to have this trailng space

extern vector<vector<const char*> > iso3166_codes = {

{"ABW",  "ARUBA",},			{"AFG",  "AFGHANISTAN",},	{"AGO",  "ANGOLA",},		{"AIA",  "ANGUILLA",},
{"ALA",  "ÅLAND",},			{"ALB",  "ALBANIA",},		{"AND",  "ANDORRA",},		{"ARE",  "EMIRATES",},
{"ARG",  "ARGENTINA",},		{"ARM",  "ARMENIA",},		{"ASM",  "SAMOA",},			{"ATA",  "ANTARCTIC",},
{"ATF",  "FRENCH SOUTHERN",},			{"ATG",  "ANTIGUA", "BARBUDA",},			{"AUS",  "AUSTRALIA",},
{"AUT",  "AUSTRIA",},		{"AZE",  "AZERBAIJAN",},	{"BDI",  "BURUNDI",},		{"BEL",  "BELGIUM",},
{"BEN",  "BENIN",},			{"BES",  "BONAIRE", "SABA",},							{"BFA",  "BURKINA",},
{"BGD",  "BANGLADESH",},	{"BGR",  "BULGARIA",},		{"BHR",  "BAHRAIN",},		{"BHS",  "BAHAMAS",},
{"BIH",  "BOSNIA",},		{"BLM",  "BARTHéLEMY", "BARTHéLEMY",},					{"BLR",  "BELARUS",},
{"BLZ",  "BELIZE",},		{"BMU",  "BERMUDA",},		{"BOL",  "BOLIVIA",},		{"BRA",  "BRAZIL",},
{"BRB",  "BARBADOS",},		{"BRN",  "BRUNEI",},		{"BTN",  "BHUTAN",},		{"BVT",  "BOUVET",},
{"BWA",  "BOTSWANA",},		{"CAF",  "CENTRAL AFRICAN",},							{"CAN",  "CANADA",},
{"CCK",  "COCOS",},			{"CHE",  "SWITZERLAND", "SWISS",},						{"CHL",  "CHILE",},
{"CHN",  "CHINA",},			{"CIV",  "IVOIRE", "IVORY"},{"CMR",  "CAMEROON",},		{"COD",  "CONGO DEMO",},
{"COG",  "CONGO",},			{"COK",  "COOK",},			{"COL",  "COLOMBIA",},		{"COM",  "COMOROS",},
{"CPV",  "CABO VERDE",},	{"CRI",  "COSTA RICA",},	{"CUB",  "CUBA",},			{"CUW",  "CURAçAO",	"CURACAO"},
{"CXR",  "CHRISTMAS",},		{"CYM",  "CAYMAN",},		{"CYP",  "CYPRUS",},		{"CZE",  "CZECHIA",},
{"DEU",  "GERMANY",},		{"DJI",  "DJIBOUTI",},		{"DMA",  "DOMINICA ",},		{"DNK",  "DENMARK",},
{"DOM",  "DOMINICAN R",},								{"DZA",  "ALGERIA",},		{"ECU",  "ECUADOR",},
{"EGY",  "EGYPT",},			{"ERI",  "ERITREA",},		{"ESH",  "SAHARA",},		{"ESP",  "SPAIN",},
{"EST",  "ESTONIA",},		{"ETH",  "ETHIOPIA",},		{"FIN",  "FINLAND",},		{"FJI",  "FIJI",},
{"FLK",  "FALKLAND",},		{"FRA",  "FRANCE",},		{"FRO",  "FAROE",},			{"FSM",  "MICRONESIA",},
{"GAB",  "GABON",},			{"GBR",  "UNITED KINGDOM", "BRITAIN",},					{"GEO",  "GEORGIA",},
{"GGY",  "GUERNSEY",},		{"GHA",  "GHANA",},			{"GIB",  "GIBRALTAR",},		{"GIN",  "GUINEA",},
{"GLP",  "GUADELOUPE",},	{"GMB",  "GAMBIA",},		{"GNB",  "GUINEA-BISSAU",},	{"GNQ",  "EQUATORIAL GUINEA",},
{"GRC",  "GREECE",},		{"GRD",  "GRENADA",},		{"GRL",  "GREENLAND",},		{"GTM",  "GUATEMALA",},
{"GUF",  "FRENCH GUIANA",},	{"GUM",  "GUAM",},			{"GUY",  "GUYANA",},		{"HKG",  "HONG KONG", "HONGKONG",},
{"HMD",  "HEARD ISLAND",},	{"HND",  "HONDURAS",},		{"HRV",  "CROATIA",},		{"HTI",  "HAITI",},
{"HUN",  "HUNGARY",},		{"IDN",  "INDONESIA",},		{"IMN",  "ISLE OF MAN",},	{"IND",  "INDIA",},
{"IOT",  "BRITISH INDIAN",},{"IRL",  "IRELAND",},		{"IRN",  "IRAN",},			{"IRQ",  "IRAQ",},
{"ISL",  "ICELAND",},		{"ISR",  "ISRAEL",},		{"ITA",  "ITALY",},			{"JAM",  "JAMAICA",},
{"JEY",  "JERSEY",},		{"JOR",  "JORDAN",},		{"JPN",  "JAPAN",},			{"KAZ",  "KAZAKHSTAN",},
{"KEN",  "KENYA",},			{"KGZ",  "KYRGYZSTAN",},	{"KHM",  "CAMBODIA",},		{"KIR",  "KIRIBATI",},
{"KNA",  "KITTS",},			{"KOR",  "SOUTH KOREA",},	{"KWT",  "KUWAIT",},		{"LAO",  "LAO",},
{"LBN",  "LEBANON",},		{"LBR",  "LIBERIA",},		{"LBY",  "LIBYA",},			{"LCA",  "LUCIA",},
{"LIE",  "LIECHTENSTEIN",},	{"LKA",  "LANKA",},			{"LSO",  "LESOTHO",},		{"LTU",  "LITHUANIA",},
{"LUX",  "LUXEMBOURG",},	{"LVA",  "LATVIA",},		{"MAC",  "MACAO",},			{"MAF",  "MARTIN",},
{"MAR",  "MOROCCO",},		{"MCO",  "MONACO",},		{"MDA",  "MOLDOVA",},		{"MDG",  "MADAGASCAR",},
{"MDV",  "MALDIVES",},		{"MEX",  "MEXICO",},		{"MHL",  "MARSHALL",},		{"MKD",  "MACEDONIA",},
{"MLI",  "MALI",},			{"MLT",  "MALTA",},			{"MMR",  "MYANMAR",},		{"MNE",  "MONTENEGRO",},
{"MNG",  "MONGOLIA",},		{"MNP",  "MARIANA",},		{"MOZ",  "MOZAMBIQUE",},	{"MRT",  "MAURITANIA",},
{"MSR",  "MONTSERRAT",},	{"MTQ",  "MARTINIQUE",},	{"MUS",  "MAURITIUS",},		{"MWI",  "MALAWI",},
{"MYS",  "MALAYSIA",},		{"MYT",  "MAYOTTE",},		{"NAM",  "NAMIBIA",},		{"NCL",  "CALEDONIA",},
{"NER",  "NIGER",},			{"NFK",  "NORFOLK",},		{"NGA",  "NIGERIA",},		{"NIC",  "NICARAGUA",},
{"NIU",  "NIUE",},			{"NLD",  "NETHERLANDS",},	{"NOR",  "NORWAY",},		{"NPL",  "NEPAL",},
{"NRU",  "NAURU",},			{"NZL",  "ZEALAND",},		{"OMN",  "OMAN ",},			{"PAK",  "PAKISTAN",},
{"PAN",  "PANAMA",},		{"PCN",  "PITCAIRN",},		{"PER",  "PERU",},			{"PHL",  "PHILIPPINES",},
{"PLW",  "PALAU",},			{"PNG",  "PAPUA", "NEW GUINEA",},						{"POL",  "POLAND",},
{"PRI",  "PUERTO RICO",},	{"PRK",  "NORTH KOREA",},	{"PRT",  "PORTUGAL",},		{"PRY",  "PARAGUAY",},
{"PSE",  "PALESTINE",},		{"PYF",  "POLYNESIA",},		{"QAT",  "QATAR",},			{"REU",  "RéUNION",},
{"ROU",  "ROMANIA",},		{"RUS",  "RUSSIA",},		{"RWA",  "RWANDA",},
{"SAU",  "SAUDI ARABIA",},	{"SDN",  "SUDAN",},			{"SEN",  "SENEGAL",},		{"SGP",  "SINGAPORE",},
{"SGS",  "H GEORGIA", "SANDWICH",},						{"SHN",  "HELENA", "ASCENSION", "TRISTAN",},
{"SJM",  "SVALBARD",},		{"SLB",  "SOLOMON",},		{"SLE",  "SIERRA LEONE",},	{"SLV",  "SALVADOR",},
{"SMR",  "SAN MARINO",},	{"SOM",  "SOMALIA",},		{"SPM",  "PIERRE", "MIQUELON",},
{"SRB",  "SERBIA",},		{"SSD",  "SUDAN",},			{"STP",  "TOME", "PRINCIPE",},
{"SUR",  "SURINAME",},		{"SVK",  "SLOVAKIA",},		{"SVN",  "SLOVENIA",},		{"SWE",  "SWEDEN",},
{"SWZ",  "ESWATINI",},		{"SXM",  "MAARTEN",},		{"SYC",  "SEYCHELLES",},	{"SYR",  "SYRIA",},
{"TCA",  "TURKS", "CAICOS",},							{"TCD",  "CHAD",},			{"TGO",  "TOGO",},
{"THA",  "THAILAND",},		{"TJK",  "TAJIKISTAN",},	{"TKL",  "TOKELAU",},		{"TKM",  "TURKMENISTAN",},
{"TLS",  "TIMOR",},			{"TON",  "TONGA",},			{"TTO",  "TRINIDAD", "TOBAGO",},
{"TUN",  "TUNISIA",},		{"TUR",  "TURKEY",},		{"TUV",  "TUVALU",},		{"TWN",  "TAIWAN",},
{"TZA",  "TANZANIA",},		{"UGA",  "UGANDA",},		{"UKR",  "UKRAINE",},		{"UMI",  "UNITED STATES MINOR",},
{"URY",  "URUGUAY",},		{"USA",  "UNITED STATES", "U.S.A.",},
{"UZB",  "UZBEKISTAN",},	{"VAT",  "HOLY SEE",},				{"VCT",  "VINCENT", "GRENADINES",},
{"VEN",  "VENEZUELA",},		{"VGB",  "VIRGIN ISLANDS (B",},		{"VIR",  "VIRGIN ISLANDS (U",},
{"VNM",  "VIETNAM", "VIET NAM",},						{"VUT",  "VANUATU",},		{"WLF",  "WALLIS", "FUTUNA",},
{"WSM",  "SAMOA",},			{"YEM",  "YEMEN",},			{"ZAF",  "SOUTH AFRICA", "SOUTH-AFRICA",},
{"ZMB",  "ZAMBIA",},		{"ZWE",  "ZIMBABWE",}, };


bool add_iso3166_country_metadata(WED_Airport & apt, bool inProgress)
{
	//-- upgrade Country Metadata -------------
	string country;
	bool has_iso = false;

	if (apt.ContainsMetaDataKey(wed_AddMetaDataCountry))
	{
		country = apt.GetMetaDataValue(wed_AddMetaDataCountry);

		bool has_iso = country.size() >= 3;
		for (int i = 0; i < 3 && has_iso; i++)
			has_iso &= (bool)isalpha(country[i]);
		if (country.size() > 3)
			has_iso &= country[3] == ' ';

		if (has_iso)
		{
			has_iso = false;
			for (const auto& iso : iso3166_codes)
				if (strncmp(iso.front(), country.c_str(), 3) == 0)
				{
					has_iso = true;
					break;
				}
		}
	}
	if (!has_iso)
	{
		if (country == "US") country = "United States";
		else if (country == "CA") country = "Canada";
		else if (country == "FR") country = "France";
		else if (country == "DE") country = "Germany";
		else if (country == "RK") country = "South Korea";

		int matches = 0;
		string code3;
		if (country.size())
		{
			string country_l(country);
			std::transform(country_l.begin(), country_l.end(), country_l.begin(), [](unsigned char c) { return std::toupper(c); });
			country_l += " ";

			for (const auto& list : iso3166_codes)
				for (auto cty = list.begin() + 1; cty < list.end(); cty++)
					if (country_l.find(*cty) != string::npos)
					{
						code3 = list.front();
						code3 += " ";
						matches++;
						break;
					}
		}
		if (matches == 0)
		{
			string ICAO_code;
			if (apt.ContainsMetaDataKey(wed_AddMetaDataICAO))
				ICAO_code = apt.GetMetaDataValue(wed_AddMetaDataICAO);
			else                          // trust LR assigned airport ID's to have meaninful region prefix
			{
				string s;
				apt.GetName(s);
				if (s[0] == 'X' && s.size() > 4)
					ICAO_code = s.substr(1);
			}
			if (ICAO_code.size())         // trust explicit set and plausibility checked ICAO meta adata
			{
				ICAO_code = ICAO_code.substr(0, 2);
				if (ICAO_code[0] == 'K')			code3 = "USA ";
				else if (ICAO_code[0] == 'C')		code3 = "CAN ";
				else if (ICAO_code[0] == 'E')
				{
					if (ICAO_code[1] == 'D')		code3 = "DEU ";
					else if (ICAO_code[1] == 'G')	code3 = "GBR ";
					else if (ICAO_code[1] == 'S')	code3 = "SWE ";
					else if (ICAO_code[1] == 'N')	code3 = "NOR ";
					else if (ICAO_code[1] == 'K')	code3 = "DAN ";
					else if (ICAO_code[1] == 'P')	code3 = "POL ";
				}
				else if (ICAO_code[0] == 'L')
				{
					if (ICAO_code[1] == 'F')		code3 = "FRA ";
					else if (ICAO_code[1] == 'E')	code3 = "ESP ";
					else if (ICAO_code[1] == 'O')	code3 = "AUT ";
					else if (ICAO_code[1] == 'I')	code3 = "ITA ";
					else if (ICAO_code[1] == 'S')	code3 = "CHE ";
					else if (ICAO_code[1] == 'K')	code3 = "SWI ";
					else if (ICAO_code[1] == 'C')	code3 = "CYP ";
					else if (ICAO_code[1] == 'P')	code3 = "PRT ";
					else if (ICAO_code[1] == 'T')	code3 = "TUR ";
				}
				else if (ICAO_code[0] == 'Y')		code3 = "AUS ";
				else if (ICAO_code[0] == 'Z' && ICAO_code[1] != 'K' && ICAO_code[1] != 'M')
				{
					code3 = "CHN ";
					if (country.empty()) country = "China";
				}
				else if (ICAO_code == "FA")			code3 = "ZAF ";
				else if (ICAO_code == "GV")			code3 = "CPV ";
				else if (ICAO_code == "MG")			code3 = "GTM ";
				else if (ICAO_code == "SA")			code3 = "ARG ";
				else if (ICAO_code == "SB" || ICAO_code == "SD" || ICAO_code == "SI" || ICAO_code == "SJ" || ICAO_code == "SN"
					|| ICAO_code == "SW")			code3 = "BRA ";
				else if (ICAO_code == "SP")			code3 = "PER ";
				else if (ICAO_code == "VY")			code3 = "MMR ";
				else if (ICAO_code[0] == 'U' && ICAO_code[1] > 'D' && ICAO_code[1] != 'G' && ICAO_code[1] != 'K' && ICAO_code[1] != 'M'
					&& ICAO_code[1] != 'T')		code3 = "RUS ";
				else if (ICAO_code == "WA" || ICAO_code == "WI" || ICAO_code == "WQ" || ICAO_code == "WR") code3 = "IDN ";

				if (code3.empty())
					LOG_MSG("'%s' failed to resolve by ICAO\n", ICAO_code.c_str());
			}
		}
		if (matches > 1)
			LOG_MSG("'%s' matches %dx using %s\n", country.c_str(), matches, code3.c_str());

		if(!inProgress) 
			apt.StartCommand("Add country code");
		apt.AddMetaDataKey(META_KeyName(wed_AddMetaDataCountry), code3 + country);
		if (!inProgress) 
			apt.CommitCommand();
		return true;
	}

	return false;
}