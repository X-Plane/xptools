#include "ParserMain.h"
#define _CRT_SECURE_NO_WARNINGS

ParserMain::ParserMain(void)
{
}


ParserMain::~ParserMain(void)
{
}

//Return if there was an error or not
static bool ValidateBasics(InString * inStr)
{
	bool error = false;

	//---White Space-----------------------------------------
	printf("Checking to see if there is any whitespace...\n");
	while(inStr->nPos != inStr->endPos)//Loop for the whole string
	{
		//If the current charecter is white space
		if(isspace(*inStr->nPos))
		{
			printf("Char %d is whitespace.\n", inStr->count);
			return true;
		}
		//Increase the pointer and counter
		inStr->nPos++;
		inStr->count++;
	}
	printf("No whitespace errors detected. Moving on to ASCII Checking...\n");
	//-------------------------------------------------------

	//Reset variable
	inStr->ResetNPos();

	//--ASCII------------------------------------------------
	while(inStr->nPos != inStr->endPos)
	{	
		if( ((int) *inStr->nPos < 33 ) || ((int) *inStr->nPos > 126))
		{
			printf("Char at location %d is not valid ASCII. \n", inStr->nPos, inStr->count);
			return true;
		}
		inStr->nPos++;
		inStr->count++;
	}
	if(error == false)
	{
		printf("All ASCII valid\n");
	}
	//-------------------------------------------------------
	inStr->ResetNPos();
		
	//end of the } brace pair, starts as the end of the string for safety
	const char * curEnd = inStr->endPos;
		
	//What is currently considered good, a { a }
	//We start by saying that we are looking for a {
	char LFGoodMode = '{';//Used later for nesting nesting

	//--Validate some basic curly brace rules
	//1.) Must have atleast 1 pair
	//2.) All { have a }
	//3.) No pair may be empyt nest
	//4.) No pair may nest

	//--Find if and where the end of the pair is-----
	//Run until it breaks on 1.)Finding }
	//or reaching the end of the string
	while(true)
	{
		//If you've found the other pair
		if(*inStr->nPos == '}')
		{
			curEnd=inStr->nPos;
			break;
		}
		if(inStr->nPos == inStr->endPos)
		{
			printf("You have no end to this pair starting from %d",inStr->count);
			return true;
		}
		inStr->nPos++;
	}
	//---------------------------------------------

	//Now that you are at done finding the bounds, reset
	inStr->ResetNPos();

	//--Next, find if it is actually empty---------
	if(*(inStr->nPos+1) == '}')
	{
		printf("Empty curly braces detected!\n");
		return true;
	}
	//---------------------------------------------
			
	//--Finally see if there is nesting------------

	while(inStr->nPos != curEnd)
	{
		/* 1.)Decide whats good or bad
		*		The first curly brace should be open
		*		The second curly brace should be close
		*		It should change never
		* 2.)Test to see if it good or bad
		*/

		//If we are at a { or }
		if((int)*inStr->nPos == '{' || (int) *inStr->nPos == '}')
		{
			//Is it the good mode?
			if((int)*inStr->nPos == LFGoodMode)
			{
				//If so toggle what you are looking for
				LFGoodMode = (*inStr->nPos == '{') ? '}' : '{';					
			}
			else
			{
				error = true;
				printf("Char %c at location %d is invalid \n", *inStr->nPos,inStr->count);
			}
		}
		inStr->count++;
		inStr->nPos++;
	}
	//---------------------------------------------
	inStr->ResetNPos();

	return error;
}

static bool FireAction(ACTIONS todo, char * optional=0)
{
	switch(todo)
	{
		case APPEND_ACCUMED:
			break;
		case CHANGE_COLOR:
			break;
		case TOGGLE_SIDE:
			break;
		case PRINT_MESSAGE:
			break;
		case THROW_ERROR:
			break;
		default:
			return false;
	}
}

//Take in the current (and soon to be past) state  and the current letter being processed
static FSM LookUpTable(FSM curState, char curChar)
{
	//If you have reached a \0 FOR ANY REASON exit now
	if(curChar == '\0')
	{
		//FireAction(
		return O_END;
	}
	switch(curState)
	{
	case I_IDLE:
		switch(curChar)
		{
		}
		break;
	case I_INCUR:
		switch(curChar)
		{
		case '@':
			//FireAction(
			return I_ANY_CONTROL;
			break;
		}
		break;
	case I_ACCUM_GLPHYS:	
		switch(curChar)
		{
		}

		break;
	case I_ANY_CONTROL:	
		switch(curChar)
		{
		case 'Y':
		case 'L':
		case 'R':
		case 'B':
			//FireAction(Change color
			return I_WAITING_SEPERATOR;
			
		case '@':
			//FireAction(Change sides
			return I_WAITING_SEPERATOR;
		}
		break;
	case I_DOUBLE_CONTROL:	//This one may not be necissary
		switch(curChar)
		{

		}

		break;
	case I_WAITING_SEPERATOR:	
		switch(curChar)
		{
		}
		break;
	case O_ACCUM_GLYPHS:	
		switch(curChar)
		{
		case '{':
			return I_INCUR;
			break;
		}
		break;
	case O_END:	
		switch(curChar)
		{
		}
		break;
	}
	return I_IDLE;
}

int main(int argc, const char* argv[])
{
	printf("Welcome to the Taxi Sign Parser. \nPlease input the string now \n");
	
	//Take input and create and input string from it
	char buf[1024];
	scanf("%[^\n]",buf);
	InString inStr(buf);

	//Make the front and back outStrings
	OutString outStr();

	//Validate if there is any whitesapce or non printable ASCII charecters (33-126)
	if(1)
	{
		if(ValidateBasics(&inStr) == true)
		{
			printf("\n String not basically valid \n");
			system("pause");
			return 0;
		}
		system("cls");
	}
	FSM FSM_MODE = O_ACCUM_GLYPHS;

	while(FSM_MODE != O_END)
	{
		//Look up the transition
		FSM transition = LookUpTable(FSM_MODE,*(inStr.nPos));
		if(transition != LOOKUP_ERR)
		{
			FSM_MODE = transition;
			inStr.nPos++;
		}
		else
		{
			printf("Fatal lookup error! State: %d, Char: %c, Location: %d\n",(int)FSM_MODE,*(inStr.nPos),inStr.count);
			break;
		}

	}
	system("pause");
	return 0;
}
#if 0
//Return if there was an error or not, validates only a set of them
static bool ValidateCurlyBraces(InString * inStr)
{
}
#endif