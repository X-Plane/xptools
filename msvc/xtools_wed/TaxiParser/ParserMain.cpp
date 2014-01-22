#include "ParserMain.h"
#define _CRT_SECURE_NO_WARNINGS

ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}


int main(int argc, const char* argv[])
{
	printf("Welcome to the Taxi Sign Parser. \nPlease input the string now \n");
	
	//Take input and create and input string from it
	char buf[1024];
	scanf("%[^\n]",buf);
	InString inStr(buf);

	//Make the front and back outStrings
	OutString outStr;
	//Validate if there is any whitesapce or non printable ASCII charecters (33-126)
	if(ParserValer::ValidateBasics(&inStr) == true)
	{
		printf("\nString not basically valid \n");
		system("pause");
		return 0;
	}
	system("pause");
	system("cls");
	
	FSM FSM_MODE = O_ACCUM_GLYPHS;
	while(FSM_MODE != O_END)
	{
		//Look up the transition
		FSM transition = ParserValer::LookUpTable(FSM_MODE,*(inStr.nPos), &outStr);
		if(transition != LOOKUP_ERR)
		{
			FSM_MODE = transition;
			inStr.nPos++;
		}
		else
		{
																							//1 based for users who wouldn't expect something to be 0 based
			printf("\nFatal lookup error! State: %s, Char: %c, Location: %d\n",ParserValer::EnumToString(FSM_MODE),*(inStr.nPos),(inStr.nPos-inStr.oPos)+1);
			break;
		}
	}
	printf("\n");
	outStr.PrintString();
	printf("\n");
	system("pause");

	return 0;
}