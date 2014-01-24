#include "ParserMain.h"
#include "UnitTester.h"
#define _CRT_SECURE_NO_WARNINGS

ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}


int main(int argc, const char* argv[])
{
	Test tests[256] = {
		Test(&InString("{@Y}CAT"),&OutString("{@Y}CAT"),true,true),
		Test(&InString("{@Y}CAT{@@}safety"),&OutString("{@Y}CAT{@@}safety"),true,true)
		/*Test(&InString("{@Y}CAT"),&OutString(),true,true),
		Test(&InString("{@Y}CAT"),&OutString(),true,true),
		Test(&InString("{@Y}CAT"),&OutString(),true,true),
		Test(&InString("{@Y}CAT"),&OutString(),true,true),
		Test(&InString("{@Y}CAT"),&OutString(),true,true),
		Test(&InString("{@Y}CAT"),&OutString(),true,true)*/
		};
	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
	{
		char temp[256];
		char number[24];
		_itoa(i,number,10);
		sprintf(temp,"C:\\Users\\Ted\\Desktop\\Logs\\%s%s%s","log_",number,".txt");
		UnitTester::RunTest(&tests[i],temp);
	}
	UnitTester::RunTest(NULL);
}