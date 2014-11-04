#include "ParserMain.h"
#define _CRT_SECURE_NO_WARNINGS

#define LOG_FLD_PATH "C:\\Users\\Ted\\Desktop\\Logs\\%s%s%s"
#define ADD_TEST(in) testStrings[testCount] = string(in); testCount++;

ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}

enum Errors
{
	none,
	no_front_match,
	no_back_match,
	exit_early
};

struct Test
{
	parser_in_info * tInStr;
	parser_out_info expctRes;
	
	//Prints to console
	bool  printToCon;
	
	//Prints to file
	bool  printToFile;
	
	Errors error;

	Test(void)
	{
		tInStr = NULL;
		expctRes = parser_out_info();
		printToCon = false;
		printToFile = true;
		error = none;
	}
	Test(parser_in_info * in, parser_out_info out, bool toCon, bool toFile)
	{
		tInStr = in;
		expctRes = out;
		printToCon = toCon;
		printToFile = toFile;
		error = none;
	}
	~Test(){}
};

static int RunTest(Test * t, char * filePath)
{
	parser_out_info outStr;
	/*1.) Run the main loop from a test string or user input
	* 2.) Examine the the results of the output and assaign an error
	* 3.) If there is an option to print, do so
	*/
	ParserTaxiSign(*t->tInStr,outStr);
		
	string outSignFront = outStr.out_sign.toString(outStr.out_sign.front);
	string outSignBack = outStr.out_sign.toString(outStr.out_sign.back);

	string exptSignFront = t->expctRes.out_sign.toString(t->expctRes.out_sign.front);
	string exptSignBack = t->expctRes.out_sign.toString(t->expctRes.out_sign.back);

	//X is default and cannot exists after a succesful parse
	/*if(outStr.GetCurColor() == 'X')
	{
		t->error = exit_early;
	}
		
	//Compare the front to the expected front
	if(outSignFront != exptSignFront)
	{
		t->error = no_front_match;
	}

	//If the back exists and it's the not same
	if(outSignBack.size() > 0 && (outSignBack != exptSignBack))
	{
		t->error = no_back_match;
	}*/

	if(t->printToCon)
	{
		//Print the original input
		printf("in_info: %s\n",t->tInStr->input.c_str());

		//Print the expected Front and Back, if the back exists
		printf("out_info (Expected):(Front)%s\n",exptSignFront.c_str());
		printf("\t\t\t\t\t(Back) ");
		if(exptSignBack.c_str() > 0) 
		{
			printf("%s\n",exptSignBack.c_str());
		}
		else
		{
			printf("\n");
		}
		//Print the actual Front and Back, if the back exists
		printf("out_info (Actual):\t(Front)%s\n",outSignFront.c_str());
		printf("\t\t\t\t\t(Back) ");
		if(outSignBack.c_str() > 0) 
		{
			printf("%s\n",outSignBack.c_str());
		}
		else
		{
			printf("\n");
		}
		//Print out the errors that the test generated
		printf("Test Errors: \n");
				
		/*switch(t->error)
		{
		case none:
			printf("None\n");
			break;
		case no_front_match:
			printf("Front does not match, expected %s\n",exptSignFront.c_str());
			break;
		case no_back_match:
			printf("Back does not match, expected %s\n",exptSignBack.c_str());
			break;
		case exit_early:
			printf("The test exited early\n");
		}*/

		//Print out all the errors that were generated during parsing
		printf("Parsing Errors:\n");
		for (int i = 0; i < t->expctRes.errors.size(); i++)
		{
			printf("%s\n",outStr.errors[i].msg.c_str());
		}
	}

	if(t->printToFile)
	{
		FILE * fi = fopen(filePath,"w");
		if(fi != NULL)
		{
			//Print the original input
			fprintf(fi,"in_info: %s\n",t->tInStr->input.c_str());

			//Print the expected Front and Back, if the back exists
			fprintf(fi,"out_info (Expected):(Front)%s\n",exptSignFront.c_str());
			fprintf(fi,"\t\t\t\t\t(Back) ");
			if(exptSignBack.length() > 0) 
			{
				fprintf(fi,"%s\n",exptSignBack.c_str());
			}
			else
			{
				fprintf(fi,"\n");
			}
			//Print the actual Front and Back, if the back exists
			fprintf(fi,"out_info (Actual):\t(Front)%s\n",outSignFront.c_str());
			fprintf(fi,"\t\t\t\t\t(Back) ");
			if(outSignBack.length() > 0) 
			{
				fprintf(fi,"%s\n",outSignBack.c_str());
			}
			else
			{
				fprintf(fi,"\n");
			}
				
			//Print out the errors that the test generated
			fprintf(fi,"Test Errors: \n");
				
			/*switch(t->error)
			{
			case none:
				fprintf(fi,"None\n");
				break;
			case no_front_match:
				fprintf(fi,"Front does not match, expected %s\n",exptSignFront.c_str());
				break;
			case no_back_match:
				fprintf(fi,"Back does not match, expected %s\n",exptSignBack.c_str());
				break;
			case exit_early:
				fprintf(fi,"The test exited early\n");
			}*/

			//Print out all the errors that were generated during parsing
			fprintf(fi,"Parsing Errors:\n");
			for (int i = 0; i < outStr.errors.size(); i++)
			{
				fprintf(fi,"%s\n",outStr.errors[i].msg.c_str());
			}
		}
		else
		{
			fclose(fi);
			return ferror(fi);
		}
		fclose(fi);
	}
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
	* Has caret by itself, {@Y,^} or {@Y}^
	--Semantic Pipe--
	* Has Sign instruction on left side
	* Has Sign instruction on right side
	* Has Sign instruction on both sides
	* Has Adjecent pipebars
	* Non matching colors (independent)
	* Non matching colors (dependant)
	* Sign starts with pipe
	* sign ends with pipe
	* Pipebar is inbetween two independant glyphs
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

	//Caret Tests
	ADD_TEST("{@Y,^}")
	ADD_TEST("{@Y}^")

	//Pipe tests
	ADD_TEST("{@Y}LEFT{@@}|ONWARDS")
	ADD_TEST("{@Y}BEFORE|{@@}ONWARDS")
	ADD_TEST("{@Y}LEFT{@@}|{@@}RIGHT")

	ADD_TEST("{@Y}A||BC|D||E")
	ADD_TEST("{critical}|{@Y,B}")
	ADD_TEST("{@Y,A}{@@}BACK|{@R,B}")

	ADD_TEST("|{@Y}FRONT")
	ADD_TEST("{@Y}BACK|")
	ADD_TEST("{critical}|{no-entry}")
	ADD_TEST("10R-28L")
	ADD_TEST("ALL{^r}1234{@@}BAD{comma}{@Y}UNTILNOW{hazard}")
	//ADD_TEST("{@Y}ALL{@R}VALID{@L}UNTIL{@Y}@{N,O,W}")
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
	vector<parser_in_info> inStringArr;
	
	for (int i = 0; i < testCount; i++)
	{
		inStringArr.push_back(parser_in_info(testStrings[i]));
	}
	//---------------------------------------------------------

	//--Test Array Creation------------------------------------
	//Create our array of tests
	Test tests[MAX_TESTS];

	//For all spaces in the array fill it with blanks
	for (int i = 0; i < testCount; i++)
	{
		parser_out_info out;//If we implement something else we'll need to do something similar to the string arr/in_info arr
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
		
		RunTest(&tests[i],temp);
	}
	//---------------------------------------------------------
}
