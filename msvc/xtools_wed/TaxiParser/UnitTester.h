#pragma once
#include "..\..\..\src\WEDCore\WED_Sign_Parser.h"
#include <stdio.h>
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
	OutInfo expctRes;
	
	//Prints to console
	bool  printToCon;
	
	//Prints to file
	bool  printToFile;
	
	Errors error;

	Test(void)
	{
		tInStr = NULL;
		expctRes = OutInfo();
		printToCon = false;
		printToFile = true;
		error = none;
	}
	Test(InString * in, OutInfo out, bool toCon, bool toFile)
	{
		tInStr = in;
		expctRes = out;
		printToCon = toCon;
		printToFile = toFile;
		error = none;
	}
	~Test(){}
};

class UnitTester
{
public:
	UnitTester(void);
	~UnitTester(void);
	static int RunTest(Test * t, vector<string> & msgBuf, char * filePath)
	{
		OutInfo outStr;
		/*1.) Run the main loop from a test string or user input
		* 2.) Examine the the results of the output and assaign an error
		* 3.) If there is an option to print, do so
		*/
		outStr = WED_Sign_Parser::MainLoop(*t->tInStr,msgBuf);
		
		
		//X is default and cannot exists after a succesful parse
		if(outStr.GetCurColor() == 'X')
		{
			t->error = exit_early;
			//system("pause");
		}
		
		//Compare the front to the expected front
		if(outStr.fRes != t->expctRes.fRes)
		{
			t->error = no_front_match;
		}
		//If the back exists and it's the same
		if(outStr.bRes != t->expctRes.bRes && outStr.bRes.length() > 0)
		{
			t->error = no_back_match;
		}
		//system("cls");
		
		if(t->printToCon)
		{
			printf("InString: %s",t->tInStr->input.c_str());
			printf("\nOutInfo (Expected): (Front) %s ",t->expctRes.fRes);
				
			if(t->expctRes.bRes.size() > 0) 
				printf("(Back) %s",t->expctRes.bRes.c_str());
				
			printf("\nOutInfo (Final): (Front) %s ",outStr.fRes);
				
			if(outStr.bRes.size() > 0)
				printf("(Back) %s",outStr.bRes.c_str());
				
			printf("\nErrors: ");
				
			switch(t->error)
			{
			case none:
				printf("None");
				break;
			case no_front_match:
				printf("Front does not match, expected %s",t->expctRes.fRes);
				break;
			case no_back_match:
				printf("Back does not match, expected %s",t->expctRes.bRes);
				break;
			case exit_early:
				printf("The test exited early");
			}
			printf("MsgBuf:\n");
			for (int i = 0; i < msgBuf.size(); i++)
			{
				printf("%s\n",msgBuf[i].c_str());
			}
		}
		if(t->printToFile)
		{
			FILE * fi = fopen(filePath,"w");
			if(fi != NULL)
			{
				fprintf(fi,"InString: %s",t->tInStr->input.c_str());
				fprintf(fi,"\nOutInfo (Expected): (Front) %s ",t->expctRes.fRes);
				
				if(t->expctRes.bRes.size() > 0) 
					fprintf(fi,"(Back) %s",t->expctRes.bRes.c_str());
				
				fprintf(fi,"\nOutInfo (Final): (Front) %s ",outStr.fRes);
				
				if(outStr.bRes.size() > 0)
					fprintf(fi,"(Back) %s",outStr.bRes.c_str());
				
				fprintf(fi,"\nErrors: ");
				
				switch(t->error)
				{
				case none:
					fprintf(fi,"None");
					break;
				case no_front_match:
					fprintf(fi,"Front does not match, expected %s",t->expctRes.fRes);
					break;
				case no_back_match:
					fprintf(fi,"Back does not match, expected %s",t->expctRes.bRes);
					break;
				case exit_early:
					fprintf(fi,"The test exited early");
				}
				fprintf(fi,"\n");
				fprintf(fi,"MsgBuf:\n");
				for (int i = 0; i < msgBuf.size(); i++)
				{
					fprintf(fi,"%s\n",msgBuf[i].c_str());
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
};

