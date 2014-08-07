#include "ParserMain.h"
#include "UnitTester.h"
#define _CRT_SECURE_NO_WARNINGS
#define LOG_FLD_PATH "C:\\Users\\Ted\\Desktop\\Logs\\%s%s%s"
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
	Test tests[MAX_TESTS];

	//For all spaces in the array fill it with blanks
	for (int i = 0; i < MAX_TESTS; i++)
	{
		tests[i]=Test();
	}

	//A counter for placing the tests in the array
	int counter = 0;

	//To Add a test create the following
	//testString,expectedResults,printToConsole?,printToFile?
	//tests[counter] = Test(&InString("text"),&OutInfo("front","optional back"),true,true); counter++;

	/* Test List
	--Basics--
	0. Whitespace
	1. Is all ASCII
	2. Has non-supported char
	3. Starts with {@Y/R/L/B
	--CurlyBraces--
	4. Matching Pairs
	5. Empty curly braces
	6. Find Nesting
	--Syntactic--
	7. , or } after a comma
	8. A lowercase letter that is outside a pair { }
	--Semantic--
	9.Color-L has non A-Z,0-9
	10.Color-B has non 0-9
	11.MultiGlyph-not real
	12.MultiGlyph-longer than any known (atleast 8)
	13.Only valid instructions
	*/

	//The following should all produce errors
	string s0 = "{@Y}TES T";
	InString i00 = InString(s0);
	string s1 = "{@Y}";//Types the sentance {@Y}¾
	s1.operator+=('\xFF');
	InString i01 = InString(s1);
	string s2 = "{@Y}~";
	InString i02 = InString(s2);
	string s3 = "{@NOTAVALIDINSTRUCTION}";
	InString i03 = InString(s3);
	string s4 = "{@Y}CASE{hazard";
	InString i04 = InString(s4);
	string s5 = "{@Y}TEST{}";
	InString i05 = InString(s5);
	string s6 = "{@Y}TEST{@L}}";
	InString i06 = InString(s6);
	string s7 = "{@R,T,E,S,,T}";
	InString i07 = InString(s7);
	string s8 = "{@Y}critical{@@}hzrd";//spelled wrong but supporting characters
	InString i08 = InString(s8);
	string s9 = "{@L,critical}";
	InString i09 = InString(s9);
	string s10 = "{@B}D";
	InString i10 = InString(s10);
	string s11 = "{@Y,not-real}";
	InString i11 = InString(s11);
	string s12 = "{@Y,tooloooooonnn}";
	InString i12 = InString(s12);
	
	//string s13 = "{@Y}TEST{@V}CASE{@K}BAD";
	string s13 = "{criticl}";
	InString i13 = InString(s13);


	OutInfo o0; 
//o0.fRes = "";
//o0.bRes = "";
//
OutInfo o1; 
//o1.fRes = "";
//o1.bRes = "";
//
OutInfo o2; 
//o2.fRes = "";
//o2.bRes = "";
//
OutInfo o3; 
//o3.fRes = "";
//o3.bRes = "";
//
OutInfo o4; 
//o4.fRes = "";
//o4.bRes = "";
//
OutInfo o5; 
//o5.fRes = "";
//o5.bRes = "";
//
OutInfo o6; 
//o6.fRes = "";
//o6.bRes = "";
//
OutInfo o7; 
//o7.fRes = "";
//o7.bRes = "";
//
OutInfo o8; 
//o8.fRes = "";
//o8.bRes = "";
//
OutInfo o9; 
//o9.fRes = "";
//o9.bRes = "";
//
OutInfo o10; 
//o10.fRes = "";
//o10.bRes = "";
//
OutInfo o11; 
//o11.fRes = "";
//o11.bRes = "";
//
OutInfo o12; 
//o12.fRes = "";
//o12.bRes = "";
//
OutInfo o13; 
//o13.fRes = "";
//o13.bRes = "";

	
	/*tests[counter] = Test(&i00,o0,false,true); counter++;
	tests[counter] = Test(&i01,o0,false,true); counter++;
	tests[counter] = Test(&i02,o0,false,true); counter++;
	tests[counter] = Test(&i03,o0,false,true); counter++;
	tests[counter] = Test(&i04,o0,false,true); counter++;
	tests[counter] = Test(&i05,o0,false,true); counter++;
	tests[counter] = Test(&i06,o0,false,true); counter++;
	tests[counter] = Test(&i07,o0,false,true); counter++;
	tests[counter] = Test(&i08,o0,false,true); counter++;
	tests[counter] = Test(&i09,o0,false,true); counter++;
	tests[counter] = Test(&i10,o0,false,true); counter++;
	tests[counter] = Test(&i11,o0,false,true); counter++;
	tests[counter] = Test(&i12,o0,false,true); counter++;*/
	tests[counter] = Test(&i13,o0,false,true); counter++;
	
	vector<string> msgBuf;
	//For all the tests
	for (int i = counter-1; i >= 0; i--)
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
}