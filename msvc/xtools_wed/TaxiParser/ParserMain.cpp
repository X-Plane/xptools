#include "ParserMain.h"
#include <stdio.h>
#include <string>
#include <conio.h>

#define _CRT_SECURE_NO_WARNINGS
ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}

static bool ValidateBasics(const char * inString)
{
	//What validation level you're out
	int VAL_LEVEL = VLEV_WHITESPACE;
	
	//Original position
	const char * oPos = inString;

	//Moveable position
	const char * nPos = oPos;

	//End of the string (\0)
	const char * endPos = strlen(inString) * sizeof(char) + nPos;
	
	int count = 0;
	bool error = false;

	//---White Space-----------------------------------------
	printf("Checking to see if there is any whitespace...\n");
	while(nPos != endPos)//Loop for the whole string
	{
		//If the current charecter is white space
		if(isspace(*nPos))
		{
			error = true;
			printf("FAIL: Char %d is whitespace.\n", count);
		}
		//Increase the pointer and counter
		nPos++;
		count++;
	}
	if(error == false)
	{
		system ( "cls" );
		printf("No whitespace errors detected. Moving on to ASCII Checking...\n");
		VAL_LEVEL = VLEV_ISALLASCII;
	}
	else
	{
		return false;
	}
	//-------------------------------------------------------

	//Reset variables
	nPos=oPos;
	count = 0;
	error = false;

	//--ASCII------------------------------------------------
	while(nPos != endPos)
	{
				
		if( (33 >= (int) *nPos) || ((int) *nPos <= 126))
		{
			error = true;
			printf("FAIL: Char %d is not valid ASCII. \n", count);
		}
		nPos++;
		count++;
	}
	if(error == false)
	{
		system("cls");
		printf("All ASCII valid\n");
	}
	else
	{
		return false;
	}

	return true;
}
static bool ValidateCurlyBraces(const char * inString)
{
	//Original position
	const char * oPos = inString;

	//Moveable position
	const char * nPos = oPos;

	//End of the string (\0)
	const char * endPos = strlen(inString) * sizeof(char) + nPos;
	
	int count = 0;
	bool error = false;
}

int main(int argc, const char* argv[])
{
	printf("Welcome to the Taxi Sign Parser. \n Please input the string now \n");
	char buf[1024];
	scanf("%[^\n]",buf);
	//Validate if there is any whitesapce or non printable ASCII charecters (33-126)
	if(ValidateBasics(buf) == false)
	{
		printf("String not valid");
		return 0;
	}
	if(ValidateCurlyBraces(buf) == false)
	{

	}
	system("pause");
	return 0;
}
