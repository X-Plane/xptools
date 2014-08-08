#include "ParserMain.h"
#include "UnitTester.h"
#define _CRT_SECURE_NO_WARNINGS

#define LOG_FLD_PATH "C:\\Users\\Ted\\Desktop\\Logs\\%s%s%s"
#define ADD_TEST(in) testStrings[testCount] = string(in); testCount++;

ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}


int main(int argc, const char* argv[])
{
	//An array of tests
	const int MAX_TESTS=256;
	/*How the unit testing works
	1.) Add a string to test by adding a line such as 
	ADD_TEST("{@Y}STUFF")
	2.) An array of InStrings are automatically created from the test strings
	3.) An array of tests are automatically created from the InStringArray
	4.) All of the tests are run with the log files being created
	5.) Clean up is done on the allocated InString array
	*/
	//To Add a test add a line like this
	

	/* Test List
	--Basics--
	* Whitespace
	* Has non-supported char
	--CurlyBraces--
	* Matching Pairs
	* Empty curly braces
	* Find Nesting
	--Syntactic--
	* , or } after a comma
	* A lowercase letter that is outside a pair { }
	--Semantic--
	* Color-L has non A-Z,0-9
	* Color-B has non 0-9
	* MultiGlyph-not real
	* MultiGlyph-longer than any known (atleast 8)
	* Only valid instructions
	*/

	//--TestString creation section----------------------------
	string testStrings[MAX_TESTS] = { string() };
	int testCount = 0;
	ADD_TEST("{@Y}TES T")
	ADD_TEST("{@Y}~")
	ADD_TEST("{@NOTAVALIDINSTRUCTION}")
	ADD_TEST("{@Y}CASE{hazard")
	ADD_TEST("{@Y}TEST{}")
	ADD_TEST("{@Y}TEST{@L}}")
	ADD_TEST("{@R,T,E,S,,T}")
	ADD_TEST("{@Y}critical{@@}hzrd")//spelled wrong but supporting characters
	ADD_TEST("{@L,critical}")
	ADD_TEST("{@B}D")
	ADD_TEST("{@Y,not-real}")
	ADD_TEST("{@Y,tooloooooonnn}")

	//--Mandatory Glyph Testing--------------------------------
	//ADD_TEST("{critical}")
	//ADD_TEST("{@B}{hazard}")
	ADD_TEST("{L}")
	ADD_TEST("L}")
	ADD_TEST("{@B}{N,O,T,A,L,L,O,W,E,D}{@@}{@Y}TIMETOBREAK{,}")
	//---------------------------------------------------------

	//--InString array-----------------------------------------
	//An array of InStrings
	vector<InString> inStringArr;
	
	for (int i = 0; i < testCount; i++)
	{
		inStringArr.push_back(InString(testStrings[i]));
	}
	//---------------------------------------------------------

	//--Test Array Creation------------------------------------
	//Create our array of tests
	Test tests[MAX_TESTS];

	//For all spaces in the array fill it with blanks
	for (int i = 0; i < testCount; i++)
	{
		OutInfo out;//If we implement something else we'll need to do something similar to the string arr/InString arr
		tests[i]=Test(&inStringArr[i],out,false,true);
	}
	//---------------------------------------------------------

	//--Run All Tests------------------------------------------
	vector<string> msgBuf;
	//For all the tests
	for (int i = testCount-1; i >= 0; i--)
	{
		//Temp char *'s for itoa
		char temp[256];
		char number[24];
		_itoa(i,number,10);

		//Create the file path
		sprintf(temp,LOG_FLD_PATH,"log_",number,".txt");
		
		UnitTester::RunTest(&tests[i],msgBuf,temp);
		msgBuf.clear();
	}
	//---------------------------------------------------------
}