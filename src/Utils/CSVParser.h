/* 
 * Copyright (c) 2016, Laminar Research.
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

#ifndef CSVPARSER_H
#define CSVPARSER_H
//Parses a CSV file with any delimiter, escape chars '\r','\n','\t'. Only supports UNIX line endings.
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

		const CSVHeader& GetHeader() const { return mHeader; }
		const vector<CSVRow>& GetRows() const { return mRows; }
	private:
		CSVHeader      mHeader;
		vector<CSVRow> mRows;
	};

	CSVParser(char delimiter, const string& input);
	CSVTable ParseCSV();
	
private:
	enum FSM {
		FSM_NORMAL,           //Hit anything but first " or delimiter, add char to token
		FSM_QOUTE,            //Hit ", expecting ["delimiter]
		FSM_WAITING_2ND_QOUTE,//Hit a [delimiter\r\n], waiting for closing qoute escape
		FSM_DELIMITER,        //Hit [delimiter], waiting for anything
		FSM_NEWLINE,          //Hit \r or \n. If \r, skip \n
		FSM_END,              //Hit mInput::npos
		FSM_INVALID           //Hit an invalid character or some other problem
	};

	// Given the current state, consume the current character and return the new state, taking all actions needed.
	FSM LookupTable(FSM current_state, int pos, string & token);
	
	//Delimiter to look for (,,\t,;,etc)
	char mDelimiter;

	//Input for the parser
	const string& mInput;
};

#if DEV
void csv_parser_run_tests(int count);
#endif

#endif