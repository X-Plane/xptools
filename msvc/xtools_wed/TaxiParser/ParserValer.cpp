#include "ParserValer.h"


ParserValer::ParserValer(void)
{
}


ParserValer::~ParserValer(void)
{
}
//Takes in the place where a '{' is as the start
//Returns true if there was an error
bool ParserValer::ValidateCurly(InString * inStr)
{
	//Local oPos,nPos,and end
	const char * lOPos = inStr->nPos;
	const char * lNPos = inStr->nPos;
	
	const char * lEndPos = NULL;
		
	//What is currently considered good, a { a }
	//We start by saying that we are looking for a {
	char LFGoodMode = '{';//Used later for nesting nesting

	//1.) All { have a }
	//2.) No pair may be empyt nest
	//3.) No pair may nest

	//--Find if and where the end of the pair is-----
	//Run until it breaks on 1.)Finding }
	//or reaching the end of the string
	while(true)
	{
		//If you've found the other pair
		if(*lNPos == '}')
		{
			lEndPos=lNPos;
			break;
		}
		if(lNPos == inStr->endPos)
		{
			printf("You have no end to this pair starting from %d",(lOPos - inStr->oPos));
			return true;
		}
		lNPos++;
	}
	//---------------------------------------------
	
	//Reset the nPos
	lNPos = lOPos;

	//--Next, find if it is actually empty---------
	if(*(lNPos+1) == '}')
	{
		printf("Empty curly braces detected!\n");
		return true;
	}
	//---------------------------------------------
	
	//--Finally see if there is nesting------------

	while(lNPos != lEndPos)
	{
		/* 1.)Decide whats good or bad
		*		The first curly brace should be open
		*		The second curly brace should be close
		*		It should change never
		* 2.)Test to see if it good or bad
		*/

		//If we are at a { or }
		if((int)*lNPos == '{' || (int) *lNPos == '}')
		{
			//Is it the good mode?
			if((int)*lNPos == LFGoodMode)
			{
				//If so toggle what you are looking for
				LFGoodMode = (*lNPos == '{') ? '}' : '{';					
			}
			else
			{
				printf("Char %c at location %d is invalid \n", *lNPos,(lOPos - inStr->oPos));
				return true;
			}
		}
		lNPos++;
	}
	//---------------------------------------------
	return false;
}

//Return if there was an error or not
bool ParserValer::ValidateBasics(InString * inStr)
{
	bool error = false;

	//---White Space-----------------------------------------
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
	//-------------------------------------------------------

	//Reset variable
	inStr->ResetNPos();

	//--ASCII or supported char------------------------------------------------
	while(inStr->nPos != inStr->endPos)
	{	
		if( ((int) *inStr->nPos < 33 ) || ((int) *inStr->nPos > 126))
		{
			printf("Char %c at location %d is not valid ASCII. \n", *inStr->nPos, inStr->count);
			return true;
		}
		//Check if it is a non supported char (aka NOT A-Z,0-9,.,* etc
		if(! ((*inStr->nPos >= 65 && *inStr->nPos <= 90) || //A-Z
			(*inStr->nPos >= 48 && *inStr->nPos <= 57)  || //0-9
			*inStr->nPos == '.'||
			*inStr->nPos == '*'||
			*inStr->nPos == '-'||
			*inStr->nPos == '{'||
			*inStr->nPos == '}'||
			*inStr->nPos == '@'||
			*inStr->nPos == ','||
			*inStr->nPos == '^'||
			*inStr->nPos == '/'||
			*inStr->nPos == 'r')
			)
		{
			printf("Char %c at location %d is not supported. \n", *inStr->nPos, inStr->count);
			return true;
		}
		inStr->nPos++;
		inStr->count++;
	}
	//-------------------------------------------------------
	inStr->ResetNPos();
	
	//--Starts with valid instruction {@(Y/R/L/B)
	if(*(inStr->oPos) == '{' && *(inStr->oPos+1) == '@')
	{
		switch(*(inStr->oPos+2))
		{
		case 'Y':
		case 'R':
		case 'L':
		case 'B':
			error = false;
			break;
		default:
			printf("Doesn't start with valid instruction\n");
			printf("%c%c%c is not a valid instruction",*(inStr->oPos),*(inStr->oPos+1),*(inStr->oPos+2));
			return true;
		}
	}
	else
	{
		printf("Doesn't start with valid instruction\n");
		printf("%c%c%c is not a valid instruction",*(inStr->oPos),*(inStr->oPos+1),*(inStr->oPos+2));
		return true;
	}

	//Validate all curly brace rules
	while(inStr->nPos != inStr->endPos)
	{
		if(*inStr->nPos == '{')
		{
			error = ValidateCurly(inStr);
			if(error) return error;
		}
		inStr->nPos++;
	}
	inStr->ResetNPos();
	return error;
}
char * ParserValer::EnumToString(FSM in)
{
	switch(in)
	{
	case I_COMMA:
		return "I_COMMA";
	case I_INCUR:
		return "I_INCUR";
	case I_ACCUM_GLPHYS:	
		return "I_ACCUM_GLPHYS";
	case I_ANY_CONTROL:
		return "I_ANYCONTROL";
	case I_WAITING_SEPERATOR:	
		return "I_WAITING_SEPERATOR";
	case O_ACCUM_GLYPHS:
		return "O_ACCUM_GLYPHS";
	case O_END:	
		return "O_END";
	}
	return "NOT REAL STATE";
}
//Take in the current (and soon to be past) state  and the current letter being processed
FSM ParserValer::LookUpTable(FSM curState, char curChar, OutString * str)
{
	printf("%c",curChar);

	//If you have reached a \0 FOR ANY REASON exit now
	if(curChar == '\0')
	{
		//FireAction(
		return O_END;
	}
 	switch(curState)
	{
	case I_COMMA:
		switch(curChar)
		{
		//You will always enter IDLE from a place where a seperator is
		//not allowed
		case '}':
		case ','://since comma's always go into idle you have hit ,,
			//FireAction(throw error)
			return LOOKUP_ERR;
		case '@':
			return I_ANY_CONTROL;
		default:
			//otherwise accumulate the glyphs
			str->AccumBuffer(curChar);
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_INCUR:
		switch(curChar)
		{
		case '@':
			//FireAction(
			return I_ANY_CONTROL;
		default:
			//otherwise accumulate the glyphs
			str->AccumBuffer(curChar);
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ACCUM_GLPHYS:	
		switch(curChar)
		{
		//Cases to make it stop accumulating
		case '}':
			str->AppendLetter(str->curlyBuf,str->curBufCNT);
			str->ClearBuf();
			return O_ACCUM_GLYPHS;
		case ',':
			str->AppendLetter(str->curlyBuf,str->curBufCNT);
			str->ClearBuf();
			return I_COMMA;
		default:
			//otherwise accumulate the glyphs
			str->AccumBuffer(curChar);
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ANY_CONTROL:	
		switch(curChar)
		{
		case 'Y':
		case 'L':
		case 'R':
		case 'B':
			
			//Do action, change color
			str->curColor = curChar;
			return I_WAITING_SEPERATOR;
			
		case '@':
			str->SwitchFrontBack();
			return I_WAITING_SEPERATOR;
		}
		break;
	case I_WAITING_SEPERATOR:	
		switch(curChar)
		{
		case ',':
			return I_COMMA;
		case '}':
			return O_ACCUM_GLYPHS;
		default:
			printf("\nWas expecting , or }, got %c",curChar);//No way you should end up with something like @YX or {@@X
			return LOOKUP_ERR;
		}
		break;
	case O_ACCUM_GLYPHS:	
		switch(curChar)
		{
		case '{':
			return I_INCUR;
			break;
		default:
			str->AppendLetter(&curChar,1);
			return O_ACCUM_GLYPHS;
			break;
		}
		break;
	case O_END:	
		switch(curChar)
		{
			
		}
		break;
	}
	return LOOKUP_ERR;
}