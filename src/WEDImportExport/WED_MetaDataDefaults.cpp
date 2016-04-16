#include "WED_MetaDataDefaults.h"
#include "WED_Airport.h"

#if DEV
#define FROM_DISK 1
	#if FROM_DISK
	#define CSV_ON_DISK "C:\\airport_metadata.csv"
	#include <fstream>
	#endif
#endif

//Parses a CSV file with any delimiter and only UNIX line endings
//Returns an immutable CSVTable when finished. An empty table indicates there was an error.
class CSVParser
{
public:
	
	class CSVTable
	{
	public:
		typedef vector<string> CSVRow;
		
		//A header is just a special row
		typedef CSVRow CSVHeader;
		
	
		CSVTable(const CSVHeader& header, const vector<CSVRow>& rows) : mHeader(header), mRows(rows) { }
		~CSVTable() { }

		const CSVHeader& GetHeader() { return mHeader; }
		const vector<CSVRow>& GetRows() { return mRows; }
	private:
		CSVHeader      mHeader;
		vector<CSVRow> mRows;
	};

	CSVParser(char delimiter, const string& input);
	CSVTable ParseCSV();
	
private:
	enum FSM {
		NORMAL,           //Hit anything but first " or delimiter, add char to token
		QOUTE,            //Hit ", expecting ["delimiter]
		WAITING_2ND_QOUTE,//Hit a [delimiter\r\n], waiting for closing qoute escape
		DELIMITER,        //Hit [delimiter], waiting for anything
		NEWLINE,          //Hit \r or \n. If \r, skip \n
		END,              //Hit mInput::npos
		INVALID           //Hit an invalid character or some other problem
	};

	// Given the current state, consume the current character and return the new state, taking all actions needed.
	FSM LookupTable(FSM current_state, int pos, string & token);
	
	//Delimiter to look for (,,\t,;,etc)
	char mDelimiter;

	//Input for the parser
	const string& mInput;
};

CSVParser::CSVParser(char delimiter, const string& input) : mDelimiter(delimiter),
															mInput(input)
{
}

CSVParser::CSVTable CSVParser::ParseCSV()
{
	CSVTable invalid_table = CSVTable(CSVParser::CSVTable::CSVHeader(),vector<CSVParser::CSVTable::CSVRow>());
	//Find newline
	
	//Check to see if we have any '\r' or empty input
	if(mInput.find_first_of('\r') != mInput.npos)
	{
		return invalid_table;
	}
	else if(mInput.size() < 1)
	{
		return invalid_table;
	}
	else
	{
		vector<CSVTable::CSVRow> rows;
		//Pushback our header
		rows.push_back(CSVTable::CSVRow());
		
		//Parser position in the input string
		int pos = 0;
		FSM mode = FSM::NORMAL;
		string current_token = "";
		while(pos < mInput.size())
		{
			FSM transition = CSVParser::LookupTable(mode, pos, current_token);
			
			++pos;

			if(transition == FSM::DELIMITER || pos == mInput.size())
			{
				rows.back().push_back(current_token);
				current_token.clear();
			}
			else if(transition == FSM::NEWLINE)
			{
				rows.back().push_back(current_token);
				current_token.clear();
				rows.push_back(CSVTable::CSVRow());
			}
			else if(transition == FSM::INVALID)
			{
				return invalid_table;
			}

			mode = transition;
		}

		if(rows.size() < 2)
		{
			return invalid_table;
		}

		//Return our completed table
		return CSVTable(rows[0],vector<CSVParser::CSVTable::CSVRow>(rows.begin()+1,rows.end()));
	}
}

CSVParser::FSM CSVParser::LookupTable(FSM current_state, int pos, string & token)
{
	char c = mInput[pos];
	
	switch(current_state)
	{
		case NORMAL:
		case DELIMITER:
		case NEWLINE:
			if(c == '"')
			{
				return QOUTE;
			}
			else if(c == mDelimiter)
			{
				//End token
				return DELIMITER;
			}
			else if(c == '\n')
			{
				return NEWLINE;
			}
			else
			{
				//Append to token
				token += c;
				return NORMAL;
			}
		case QOUTE:
			if(c == mDelimiter || c == '\n')
			{
				//Add escaped delimiter
				token += c;
				return WAITING_2ND_QOUTE;
			}
			else if(c == '"')
			{
				token += c;
				return NORMAL;
			}
			else
			{
				return INVALID;
			}
		case WAITING_2ND_QOUTE:
			if(c != '"')
			{
				return INVALID;
			}
			else
			{
				return NORMAL;
			}
		default:
			return INVALID;
	}
}

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
