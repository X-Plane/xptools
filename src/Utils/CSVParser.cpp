#include "CSVParser.h"
#if LIN
#include "math.h"
#endif


CSVParser::CSVParser(char delimiter, const string& input) : mDelimiter(delimiter),
															mInput(input)
{
}

CSVParser::CSVTable CSVParser::ParseCSV()
{
	CSVTable invalid_table = CSVTable(CSVParser::CSVTable::CSVHeader(),vector<CSVParser::CSVTable::CSVRow>());
	//Find newline
	
	//Check to see if we have any "\r\n" or empty input
	if(mInput.find("\r\n") != mInput.npos)
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
		
		//Initial state of the parser
		int pos = 0;
		FSM mode = FSM_NORMAL;
		string current_token = "";

		while(pos < mInput.size())
		{
			FSM transition = CSVParser::LookupTable(mode, pos, current_token);
			
			++pos;

			//Cases of reaching a comma or the end of file
			if(transition == FSM_DELIMITER || pos == mInput.size())
			{
				rows.back().push_back(current_token);
				current_token.clear();
			}
			else if(transition == FSM_NEWLINE)
			{
				rows.back().push_back(current_token);
				current_token.clear();

				//Ensure that the table is perfectly rectangular
				while(rows.back().size() < rows.front().size())
				{
					rows.back().push_back("");
				}
				rows.push_back(CSVTable::CSVRow());
			}
			else if(transition == FSM_INVALID)
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
		case FSM_NORMAL:
		case FSM_DELIMITER:
		case FSM_NEWLINE:
			if(c == '"')
			{
				return FSM_QOUTE;
			}
			else if(c == mDelimiter)
			{
				//End token
				return FSM_DELIMITER;
			}
			else if(c == '\n')
			{
				return FSM_NEWLINE;
			}
			else
			{
				//Append to token
				token += c;
				return FSM_NORMAL;
			}
		case FSM_QOUTE:
			if(c == mDelimiter || c == '\r' || c == '\t' || c == '\n')
			{
				//Add escaped delimiter
				token += c;
				return FSM_WAITING_2ND_QOUTE;
			}
			else if(c == '"')
			{
				token += c;
				return FSM_NORMAL;
			}
			else
			{
				return FSM_INVALID;
			}
		case FSM_WAITING_2ND_QOUTE:
			if(c != '"')
			{
				return FSM_INVALID;
			}
			else
			{
				return FSM_NORMAL;
			}
		default:
			return FSM_INVALID;
	}
}

#if DEV
#include <fstream>
#include <ctime>
#define LOG_FILE_PATH "C:\\CSVParser_test_log.txt"

char get_new_char()
{
	int min_inc = 0;
	int max_exc = '~' + 1;
	return static_cast<char>(min_inc + rand() % (max_exc - min_inc));
}

//Generates a test case for CSVParser, make_junk controls whether a test should
//succeed or not, str_len is the intended length of the test case
string csv_parser_generate_test(bool make_junk, int str_len)
{
	string output = make_junk ? "BAD": "GOOD";
	std::srand(std::time(0));

	//Table dimensions
	enum garuntee_next {
		any,
		qoute_or_esc,//'"' or '\\'
		qoute //We're at the closing escape char
	};

	garuntee_next find = any;

	char c = '\0';
	while(output.size() < str_len)
	{
		c = get_new_char();
		if(make_junk == true)
		{
			output += c;
			//break before we validate anything
			continue; 
		}

		if(c < ' ' && 
		  (c != '\r' && c != '\n' && c != '\t'))
		{
			//roll again
			continue;
		}
		
		//If we are attempting to {find}, and we've done it
		if (find == any)
		{
			output += c;
			if(c == '"')
			{
				find = qoute_or_esc;
			}
			else
			{
				find = any;
			}
		}
		else if(find == qoute_or_esc && (c == '"' || c == '\r' || c == '\n' || c == '\t'))
		{
			output += c;
			if(c == '"')
			{ 
				find = any;
			}
			else
			{
				//"\r... need next "
				find = qoute;
			}
		}
		else if(find == qoute && c == '"')
		{
			output += c;
			find = any;
		}
		//roll again for a new char we like
	}

	return output;
}

//Runs n number of tests and saves the result to a file one can review
void csv_parser_run_tests(int count)
{
	ofstream ofs(LOG_FILE_PATH);
	int str_len = 500;
	for (int i = 0; i < count; i++)
	{
		bool make_junk = round(((double)rand())/((double)RAND_MAX));
		string str = csv_parser_generate_test(make_junk, str_len);
		CSVParser::CSVTable table = CSVParser(',', str).ParseCSV();
		
		ofs << "Test #" << i << " Str: " + str << endl << "---" << endl << "Res: Table Header Size: " << table.GetHeader().size() << " Table Row Count: " << table.GetRows().size() << endl;
		if(make_junk == true)
		{
			if(table.GetHeader().size() == 0)
			{
				ofs << "Success: Bad In, Empty Table Out";
			}
			else
			{
				ofs << "Fail: Bad In, Table Out";
			}
		}
		else
		{
			if(table.GetHeader().size() > 0)
			{
				ofs << "Success: Good In, Non-Empty Table";
			}
			else
			{
				ofs << "Fail: Good In, Empty Table";
			}
		}
		ofs << endl << endl;
	}
}
#endif
