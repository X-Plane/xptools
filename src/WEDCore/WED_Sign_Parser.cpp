#include "WED_Sign_Parser.h"


ParserValer::ParserValer(void)
{
}


ParserValer::~ParserValer(void)
{
}
bool ParserValer::IsSupportedChar(char inChar)
{
	if((inChar >= 65 && inChar <= 90) || //A-Z
			(inChar >= 48 && inChar <= 57)  || //0-9
			inChar == '.'||//These take care of specials and
			inChar == '*'||//parts of multiletter glyphs
			inChar == ','||
			inChar == '-'||
			inChar == '.'||
			inChar == '_'||
			inChar == '|'||
			inChar == '/'||
			inChar == '@'||
			inChar == '^'||
			inChar == 'a'||
			inChar == 'c'||
			inChar == 'd'||
			inChar == 'e'||
			inChar == 'f'||
			inChar == 'h'||
			inChar == 'i'||
			inChar == 'l'||
			inChar == 'm'||
			inChar == 'n'||
			inChar == 'o'||
			inChar == 'r'||
			inChar == 's'||
			inChar == 't'||
			inChar == 'u'||
			inChar == 'y'||
			inChar == 'z'||
			inChar == '{'||
			inChar == '}'
			)
	{
		return true;
	}
	return false;
}
//Takes in the place where a '{' is as the start
//Returns true if there was an error
bool ParserValer::ValidateCurly(InString * inStr, char * msgBuf)
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
			sprintf(msgBuf,"You have no end to this pair starting from %d",(lOPos - inStr->oPos));
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
		sprintf(msgBuf,"Empty curly braces detected!");
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
				sprintf(msgBuf,"Char %c at location %d is invalid ", *lNPos,(lOPos - inStr->oPos));
				return true;
			}
		}
		lNPos++;
	}
	//---------------------------------------------
	return false;
}

//Return if there was an error or not
bool ParserValer::ValidateBasics(InString * inStr, char * msgBuf)
{
	bool error = false;

	//---White Space-----------------------------------------
	while(inStr->nPos != inStr->endPos)//Loop for the whole string
	{
		//If the current charecter is white space
		if(isspace(*inStr->nPos))
		{
			sprintf(msgBuf,"Char %d is whitespace.",(inStr->nPos-inStr->oPos));
			return true;
		}
		//Increase the pointer and counter
		inStr->nPos++;
	}
	//-------------------------------------------------------

	//Reset variable
	inStr->nPos = inStr->oPos;

	//--ASCII or supported char------------------------------------------------
	while(inStr->nPos != inStr->endPos)
	{	
		if( ((int) *inStr->nPos < 33 ) || ((int) *inStr->nPos > 126))
		{
			sprintf(msgBuf,"nChar %c at location %d is not valid ASCII. ", *inStr->nPos, (inStr->nPos-inStr->oPos));
			return true;
		}
		//Check if it is a non supported char (aka NOT A-Z,0-9,.,* etc
		if(!IsSupportedChar(*inStr->nPos))
		{
			sprintf(msgBuf,"Char %c at location %d is not supported. ", *inStr->nPos, (inStr->nPos-inStr->oPos));
			return true;
		}
		inStr->nPos++;
	}
	//-------------------------------------------------------
	inStr->nPos = inStr->oPos;
	
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
			sprintf(msgBuf,"%c%c%c is not a valid instruction",*(inStr->oPos),*(inStr->oPos+1),*(inStr->oPos+2));
			return true;
		}
	}
	else
	{
		sprintf(msgBuf,"%c%c%c is not a valid instruction",*(inStr->oPos),*(inStr->oPos+1),*(inStr->oPos+2));
		return true;
	}

	//Validate all curly brace rules
	while(inStr->nPos != inStr->endPos)
	{
		if(*inStr->nPos == '{')
		{
			error = ValidateCurly(inStr,msgBuf);
			if(error) return error;
		}
		inStr->nPos++;
	}
	inStr->nPos = inStr->oPos;
	return error;
}


const char * ParserValer::EnumToString(FSM in)
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
//The heart of all this
FSM ParserValer::LookUpTable(FSM curState, char curChar, OutString * str, char * msgBuf)
{
	#if DEV
	printf("%c",curChar);
	#endif
	//If you have reached a \0 FOR ANY REASON exit now
	if(curChar == '\0')
	{
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
			if(str->AccumBuffer(curChar,msgBuf));
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_INCUR:
		switch(curChar)
		{
		case '@':
			return I_ANY_CONTROL;
		default:
			//otherwise accumulate the glyphs
			str->AccumBuffer(curChar,msgBuf);
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ACCUM_GLPHYS:	
		switch(curChar)
		{
		//Cases to make it stop accumulating
		case '}':
			str->AppendLetter(str->curlyBuf,strlen(str->curlyBuf),msgBuf);
			str->ClearBuf();
			return O_ACCUM_GLYPHS;
		case ',':
			str->AppendLetter(str->curlyBuf,strlen(str->curlyBuf),msgBuf);
			str->ClearBuf();
			return I_COMMA;
		default:
			//otherwise accumulate the glyphs
			str->AccumBuffer(curChar,msgBuf);
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
			str->writeToF = false;
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
			sprintf(msgBuf,"Was expecting , or }, got %c",curChar);//No way you should end up with something like @YX or {@@X
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
			//Takes care of any supported lower case
			//Outside of curly braces. Ex: {@Y}acdefhilmnorstuyz
			if(!(curChar>=97 && curChar <= 122))
			{
				str->AppendLetter(&curChar,1,msgBuf);
				return O_ACCUM_GLYPHS;
			}
			else
			{
				sprintf(msgBuf,"Char %c is not allowed outside curly braces",curChar);
				return LOOKUP_ERR;
			}
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

OutString ParserValer::MainLoop(InString * opInStr, string * msg)
{
	char msgBuf[256] = { 0 };
	
	InString inStr(*opInStr);
	//Make the front and back outStrings
	OutString outStr;

	//Set the endPos to something valid
	inStr.endPos = strlen(inStr.oPos) * sizeof(char) + inStr.oPos;

	//Validate if there is any whitesapce or non printable ASCII charecters (33-126)
	if(ParserValer::ValidateBasics(&inStr,msgBuf) == true)
	{
		sprintf(msgBuf,"String doesn't follow basic rules!");
		*msg=string(msgBuf);
		return outStr;
	}
//	system("cls");
	
	FSM FSM_MODE = O_ACCUM_GLYPHS;
	while(FSM_MODE != O_END)
	{
		//Look up the transition
		FSM transition = ParserValer::LookUpTable(FSM_MODE,*(inStr.nPos), &outStr,msgBuf);
		if(transition != LOOKUP_ERR)
		{
			FSM_MODE = transition;
			inStr.nPos++;
		}
		else
		{
			sprintf(msgBuf,"Fatal lookup error! State: %s, Char: %c, Location: %d",ParserValer::EnumToString(FSM_MODE),*(inStr.nPos),(inStr.nPos-inStr.oPos));
			break;
		}
	}
	*msg=string(msgBuf);
	return outStr;
}