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
};

class UnitTester
{
public:
	UnitTester(void);
	~UnitTester(void);
	static int RunTest(Test * t,char * filePath)
	{
		OutString outStr;
		/*1.) Run the main loop from a test string or user input
		* 2.) Examine the the results of the output and assaign an error
		* 3.) If there is an option to print, do so
		*/
		if(t == NULL)
		{
			outStr = ParserValer::MainLoop();
		}
		else
		{
			outStr = ParserValer::MainLoop(t->tInStr);
		}
		
		//X is default and cannot exists after a succesful parse
		if(outStr.curColor == 'X')
		{
			t->error = exit_early;
			system("pause");
		}
		
		//Compare the front to the expected front
		if(strcmp(outStr.fRes,t->expctRes->fRes) != 0)
		{
			t->error = no_front_match;
		}
		//If the back exists and it's the same
		if(strcmp(outStr.bRes,t->expctRes->bRes) != 0 && strlen(outStr.bRes) > 0)
		{
			t->error = no_back_match;
		}
		system("cls");
		
		if(t->printToCon)
		{
			FILE * fi = fopen(filePath,"w");
			if(fi != NULL)
			{
			printf("InString: %s",t->tInStr->oPos);
			printf("\nOutString (Expected): (Front) %s ",t->expctRes->fRes);
			if(strlen(t->expctRes->bRes) > 0) printf("(Back) %s",t->expctRes->bRes);
			printf("\nOutString (Final): (Front) %s ",outStr.fRes);
			if(strlen(outStr.bRes) > 0) printf("(Back) %s",outStr.bRes);
			printf("\nErrors: ");
			switch(t->error)
			{
			case none:
				printf("None");
				break;
			case no_front_match:
				printf("Front does not match, expected %s",t->expctRes->fRes);
				break;
			case no_back_match:
				printf("Back does not match, expected %s",t->expctRes->bRes);
				break;
			case exit_early:
				printf("The test exited early");
			}
			printf("\n");
			}
		}
		if(t->printToFile)
		{
			FILE * fi = fopen(filePath,"w");
			if(fi != NULL)
			{
			fprintf(fi,"InString: %s",t->tInStr->oPos);
			fprintf(fi,"\nOutString (Expected): (Front) %s ",t->expctRes->fRes);
			if(strlen(t->expctRes->bRes) > 0) fprintf(fi,"(Back) %s",t->expctRes->bRes);
			fprintf(fi,"\nOutString (Final): (Front) %s ",outStr.fRes);
			if(strlen(outStr.bRes) > 0) fprintf(fi,"(Back) %s",outStr.bRes);
			fprintf(fi,"\nErrors: ");
			switch(t->error)
			{
			case none:
				fprintf(fi,"None");
				break;
			case no_front_match:
				fprintf(fi,"Front does not match, expected %s",t->expctRes->fRes);
				break;
			case no_back_match:
				fprintf(fi,"Back does not match, expected %s",t->expctRes->bRes);
				break;
			case exit_early:
				fprintf(fi,"The test exited early");
			}
			fprintf(fi,"\n");
			}
			else
			{
				fclose(fi);
				return ferror(fi);
			}
			fclose(fi);
		}
	}
};

