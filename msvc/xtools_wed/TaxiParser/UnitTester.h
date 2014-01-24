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
	}
	Test(InString * in, OutString * out, bool toCon, bool toFile)
	{
		tInStr = in;
		expctRes = out;
		printToCon = toCon;
		printToFile = toFile;
	}
	~Test(){}
	void PrintToConsole()
	{
		printf("\nInString: %s",tInStr->oPos);
		printf("\nOutString: (Front) %s (Back) %s",expctRes->fRes,expctRes->bRes);
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
	}
	int PrintToFile(char * filePath)
	{
		FILE * fi = fopen(filePath,"w");
		if(fi != NULL)
		{
			fprintf(fi,"\nInString: %s",tInStr->oPos);
			fprintf(fi,"\nOutString: (Front) %s (Back) %s",expctRes->fRes,expctRes->bRes);
			fprintf(fi,"\nErrors: ");
			switch(error)
			{
			case none:
				fprintf(fi,"None");
			case no_front_match:
				fprintf(fi,"Did not get expected result");
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
};

class UnitTester
{

public:
	UnitTester(void);
	~UnitTester(void);

	static int RunTest(Test * t,char * filePath=NULL);
};

