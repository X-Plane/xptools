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
		
		string outSignFront = outStr.out_sign.toString(outStr.out_sign.front);
		string outSignBack = outStr.out_sign.toString(outStr.out_sign.back);

		string exptSignFront = t->expctRes.out_sign.toString(t->expctRes.out_sign.front);
		string exptSignBack = t->expctRes.out_sign.toString(t->expctRes.out_sign.back);

		//X is default and cannot exists after a succesful parse
		if(outStr.GetCurColor() == 'X')
		{
			t->error = exit_early;
			//system("pause");
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
		}

		if(t->printToCon)
		{
			//Print the original input
			printf("InString: %s\n",t->tInStr->input.c_str());

			//Print the expected Front and Back, if the back exists
			printf("OutInfo (Expected):\t(Front)%s\n",exptSignFront.c_str());
				
			if(exptSignBack.c_str() > 0) 
			{
				printf("\t\t\t\t\t(Back) %s\n",exptSignBack.c_str());
			}

			//Print the actual Front and Back, if the back exists
			printf("OutInfo (Actual):\t(Front)%s\n",outSignFront.c_str());
				
			if(outSignBack.c_str() > 0) 
			{
				printf("\t\t\t\t\t(Back) %s\n",outSignBack.c_str());
			}
			
			//Print out the errors that the test generated
			printf("Test Errors: \n");
				
			switch(t->error)
			{
			case none:
				printf("None");
				break;
			case no_front_match:
				printf("Front does not match, expected %s\n",exptSignFront.c_str());
				break;
			case no_back_match:
				printf("Back does not match, expected %s\n",exptSignBack.c_str());
				break;
			case exit_early:
				printf("The test exited early\n");
			}

			//Print out all the errors that were generated during parsing
			printf("Parsing Errors:\n");
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
				//Print the original input
				fprintf(fi,"InString: %s\n",t->tInStr->input.c_str());

				//Print the expected Front and Back, if the back exists
				fprintf(fi,"OutInfo (Expected):\t(Front)%s\n",exptSignFront.c_str());
				
				if(exptSignBack.length() > 0) 
				{
					fprintf(fi,"\t\t\t\t\t(Back) %s\n",exptSignBack.c_str());
				}

				//Print the actual Front and Back, if the back exists
				fprintf(fi,"OutInfo (Actual):\t(Front)%s\n",outSignFront.c_str());
				
				if(outSignBack.length() > 0) 
				{
					fprintf(fi,"\t\t\t\t\t(Back) %s\n",outSignBack.c_str());
				}
			
				//Print out the errors that the test generated
				fprintf(fi,"Test Errors: \n");
				
				switch(t->error)
				{
				case none:
					fprintf(fi,"None");
					break;
				case no_front_match:
					fprintf(fi,"Front does not match, expected %s\n",exptSignFront.c_str());
					break;
				case no_back_match:
					fprintf(fi,"Back does not match, expected %s\n",exptSignBack.c_str());
					break;
				case exit_early:
					fprintf(fi,"The test exited early\n");
				}

				//Print out all the errors that were generated during parsing
				fprintf(fi,"Parsing Errors:\n");
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

