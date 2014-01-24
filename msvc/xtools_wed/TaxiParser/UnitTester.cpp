#include "UnitTester.h"
#include "ParserValer.h"

UnitTester::UnitTester(void)
{
}


UnitTester::~UnitTester(void)
{
}

int UnitTester::RunTest(Test * t,char * filePath)
{
	OutString outStr;
	if(t == NULL)
	{
		outStr = ParserValer::MainLoop();
	}
	else
	{
		outStr = ParserValer::MainLoop(t->tInStr);
	}
	system("cls");

	if(outStr.fRes != t->expctRes->fRes)
	{
		t->error = no_front_match;
	}
	if(outStr.bRes != t->expctRes->bRes && outStr.bCount > 0)
	{
		t->error = no_back_match;
	}
	if(t->printToCon)
	{
		t->PrintToConsole();
	}
	if(t->printToFile)
	{
		t->PrintToFile(filePath);
	}
	
	return 0;
}