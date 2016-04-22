#include "WED_MetaDataDefaults.h"
#include "WED_Airport.h"
#include "CSVParser.h"

#if DEV
#define FROM_DISK 1
	#if FROM_DISK
	#define CSV_ON_DISK "C://airport_metadata.csv"
	#include <fstream>
	#endif
#endif


void	fill_in_meta_data_defaults(WED_Airport & airport)
{
#if DEV && FROM_DISK
	std::ifstream t(CSV_ON_DISK);
	std::string str((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
	
	CSVParser::CSVTable table = CSVParser(',', str).ParseCSV();
	
	t.close();
#endif
	
	CSVParser::CSVTable::CSVRow default_values;

	//Find the airport in the table match
	string icao;
	airport.GetICAO(icao);
	for (int i = 0; i < table.GetRows().size(); ++i)
	{
		if(table.GetRows()[i][0] == icao)
		{
			default_values = table.GetRows()[i];
			break;
		}
	}

	//For every column (excluding airport_id), copy if missing key or key's value is ""
	CSVParser::CSVTable::CSVHeader column_headers = table.GetHeader();
	for (int i = 1; i < default_values.size(); i++)
	{
		string key = column_headers[i];
		string default_value = default_values[i];

		//For every of our column do They (airport) have this?
		bool has_key = airport.ContainsMetaDataKey(key);
		if(has_key)
		{
			if(airport.GetMetaDataValue(key) == "")
			{
				airport.EditMetaDataKey(key,default_value);
			}
		}
		else
		{
			airport.AddMetaDataKey(key, default_value);
		}
	}
}
