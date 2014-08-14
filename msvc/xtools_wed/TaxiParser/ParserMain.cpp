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
	2.) An array of in_infos are automatically created from the test strings
	3.) An array of tests are automatically created from the in_infoArray
	4.) All of the tests are run with the log files being created
	5.) Clean up is done on the allocated in_info array
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
	* , after {
	* I_WAITING_SEPERATOR 
	* A lowercase letter that is outside a pair { }
	--Semantic--
	* Color-L has non A-Z,0-9
	* Color-B has non 0-9
	* MultiGlyph-not real
	* Only valid instructions
	* Only one side switch per sign
	* Has glyphs
	--Semantic Pipe--
	* Has Sign instruction on left side
	* Has Sign instruction on right side
	* Has Sign instruction on both sides
	* Has Adjecent pipebars
	* Non matching colors (independent)
	* Non matching colors (dependant)
	* Sign starts with pipe
	* sign ends with pipe
	*/

	//--TestString creation section----------------------------
	string testStrings[MAX_TESTS] = { string() };
	int testCount = 0;
	
	//Basics
	ADD_TEST("{@Y}WHITESPACE TEST")
	ADD_TEST("{@Y}NONSUPPORT~")
	
	//CurlyBraces
	ADD_TEST("{@Y}{MISSINGPAIR")
	ADD_TEST("{@Y}EMPTYPAIR{}")
	ADD_TEST("{@Y}NESTING{@L}}")

	//Syntactic
	ADD_TEST("{@Y}SEP_AFTER_COMMA{A,,}")
	ADD_TEST("{@Y}COMMA_AFTER_CUR{,}")
	ADD_TEST("{@YWAITINGSEPERATOR}")
	ADD_TEST("{@Y}hazard")

	//Semantic Color
	ADD_TEST("{@Y}L_HAS_NON_A_Z_O_0_9{@L}NOT{^lu}GOOD")
	ADD_TEST("{@Y}B_HAS_NON_A_Z_O_0_9{@B}1234NOWBAD5678")

	//Semantic Multyglpyhs
	ADD_TEST("{@Y}{not-real}")
	ADD_TEST("{@Y}REAL{@N}FAKE{@R}REAL")

	//Semantic Sign Flips
	ADD_TEST("{@Y}FRONT{@@}BACK{@@}TOOMANY")

	//Has glyphs
	ADD_TEST("{@Y}")

	//Pipe tests
	ADD_TEST("{@Y}LEFT{@@}|ONWARDS")
	ADD_TEST("{@Y}BEFORE|{@@}ONWARDS")
	ADD_TEST("{@Y}LEFT{@@}|{@@}RIGHT")

	ADD_TEST("{@Y}A||BC|D||E")
	ADD_TEST("{critical}|{@Y,B}")
	ADD_TEST("{@Y,A}{@@}BACK|{@R,B}")

	ADD_TEST("|{@Y}FRONT")
	ADD_TEST("{@Y}BACK|")






	/*
	//Old Tests
	ADD_TEST("{@Y}CASE{hazard")
	ADD_TEST("{@Y}TEST{}")
	ADD_TEST("{@R,T,E,S,,T}")
	ADD_TEST("{@Y}critical{@@}hzrd")//spelled wrong but supporting characters
	ADD_TEST("{@L,critical}")
	ADD_TEST("{@B}D")
	
	
	//--Mandatory Glyph Testing--------------------------------
	//ADD_TEST("{critical}")
	//ADD_TEST("{@B}{hazard}")
	ADD_TEST("{L}")
	ADD_TEST("L}")
	ADD_TEST("{@B}{N,O,T,A,L,L,O,W,E,D}{@@}{@Y}TIMETOBREAK{,}")
	ADD_TEST("{@R}SHOULD{^lu}PASS{@@}{@L,F,I,N,E}{@B}100{critical}200")
	ADD_TEST("{@@}|{@@}F|F")*/
	//---------------------------------------------------------

	//--in_info array-----------------------------------------
	//An array of in_infos
	vector<in_info> inStringArr;
	
	for (int i = 0; i < testCount; i++)
	{
		inStringArr.push_back(in_info(testStrings[i]));
	}
	//---------------------------------------------------------

	//--Test Array Creation------------------------------------
	//Create our array of tests
	Test tests[MAX_TESTS];

	//For all spaces in the array fill it with blanks
	for (int i = 0; i < testCount; i++)
	{
		out_info out;//If we implement something else we'll need to do something similar to the string arr/in_info arr
		tests[i]=Test(&inStringArr[i],out,false,true);
	}
	//---------------------------------------------------------

	//--Run All Tests------------------------------------------
	//For all the tests
	for (int i = testCount-1; i >= 0; i--)
	{
		//Temp char *'s for itoa
		char temp[256];
		char number[24];
		_itoa(i,number,10);

		//Create the file path
		sprintf(temp,LOG_FLD_PATH,"log_",number,".txt");
		
		UnitTester::RunTest(&tests[i],temp);
	}
	//---------------------------------------------------------
}
