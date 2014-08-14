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
	in_info * tInStr;
	out_info expctRes;
	
	//Prints to console
	bool  printToCon;
	
	//Prints to file
	bool  printToFile;
	
	Errors error;

	Test(void)
	{
		tInStr = NULL;
		expctRes = out_info();
		printToCon = false;
		printToFile = true;
		error = none;
	}
	Test(in_info * in, out_info out, bool toCon, bool toFile)
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
	static int RunTest(Test * t, char * filePath)
	{
		out_info outStr;
		/*1.) Run the main loop from a test string or user input
		* 2.) Examine the the results of the output and assaign an error
		* 3.) If there is an option to print, do so
		*/
		WED_Sign_Parser p;
		p.MainLoop(*t->tInStr,outStr);
		
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
};

