#pragma once
#include <stdio.h>
#include "ParserValer.h"
#include <vector>
#include <iterator>
enum Errors
{
	none,
	no_front_match,
	no_back_match,
	exit_early
};
struct Test
{
	InString * tInStr;
	OutString * expctRes;
	
	//Prints to console
	bool  printToCon;
	
	//Prints to file
	bool  printToFile;
	
	int error;
	Test(void)
	{
		tInStr = NULL;
		expctRes = NULL;
		printToCon = true;
		printToFile = false;
		error = 0;
	}
	Test(InString * in, OutString * out, bool toCon, bool toFile)
	{
		tInStr = in;
		expctRes = out;
		printToCon = toCon;
		printToFile = toFile;
		error = 0;
	}
	~Test(){}
	void PrintToConsole()
	{
		printf("\nInString: %s",tInStr->oPos);
		printf("\nOutString: (Front) %s ",expctRes->fRes);
		if(strlen(expctRes->bRes) > 0) printf("(Back) %s",expctRes->bRes);
		printf("\nErrors: ");
		switch(error)
		{
		case none:
			printf("None");
			break;
		case no_front_match:
			printf("Front does not match");
			break;
		case no_back_match:
			printf("Back does not match");
			break;
		}
		printf("\n");
	}
	int PrintToFile(char * filePath)
	{
		FILE * fi = fopen(filePath,"w");
		if(fi != NULL)
		{
			fprintf(fi,"InString: %s",tInStr->oPos);
			fprintf(fi,"\nOutString: (Front) %s ",expctRes->fRes);
			if(strlen(expctRes->bRes) > 0) fprintf(fi,"(Back) %s",expctRes->bRes);
			fprintf(fi,"\nErrors: ");
			switch(error)
			{
			case none:
				fprintf(fi,"None");
				break;
			case no_front_match:
				fprintf(fi,"Did not get expected result");
				break;
			case no_back_match:
				fprintf(fi,"Back does not match");
				break;
			}
		}
		else
		{
			fclose(fi);
			return ferror(fi);
		}
		fclose(fi);
		return 0;
	}
	int RunTest(Test * t,char * filePath)
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

		if(strcmp(outStr.fRes,t->expctRes->fRes) != 0)
		{
			t->error = no_front_match;
		}
		if(strcmp(outStr.bRes,t->expctRes->bRes) != 0 && strlen(outStr.bRes) > 0)
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
};

class UnitTester
{

public:
	UnitTester(void);
	~UnitTester(void);

	static int RunTest(Test * t,char * filePath=NULL);
};

