#include "WED_Sign_Parser.h"


WED_Sign_Parser::WED_Sign_Parser(void)
{
}


WED_Sign_Parser::~WED_Sign_Parser(void)
{
}
bool WED_Sign_Parser::IsSupportedChar(char inChar)
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
bool WED_Sign_Parser::ValidateCurly(const InString & inStr, int position, vector<string> & msgBuf)
{
	//Local oPos,nPos,and end
	/*string::const_iterator lOPos = inStr.input.->nPos;
	const char * lNPos = inStr->nPos;
	
	const char * lEndPos = NULL;*/
		
	//What is currently considered good, a { a }
	//We start by saying that we are looking for a {
	char LFGoodMode = '{';//Used later for nesting nesting
	int rCurlyIndex = 0;//Index where the matching } is found, 0 means not found
	//1.) All { have a }
	//2.) No pair may be empyt nest
	//3.) No pair may nest

	//--Find if and where the end of the pair is-----
	//Run until it breaks on 1.)Finding }
	//or reaching the end of the string
	int i = position;
	while(true)
	{
		//If you've found the other pair
		if(inStr.input[i] == '}')
		{
			rCurlyIndex = i;
			break;
		}
		if(i == inStr.input.length())
		{
			stringstream ss;
			ss << "Curly brace pair starting at " << position + 1 << " is missing its end brace";
			msgBuf.push_back(ss.str());
			return true;
		}
		i++;
	}
	//---------------------------------------------
	
	//Reset the nPos
	i = position;

	//--Next, find if it is actually empty---------
	if(inStr.input[i+1] == '}')
	{
		stringstream ss;
							//This i+1 is for the user, not refering to the next char
		ss << "Empty curly braces detected starting at character " << i+1;
		msgBuf.push_back(ss.str());
		return true;
	}
	//---------------------------------------------
	
	//--Finally see if there is nesting------------

	while(i < inStr.input.length())
	{
		/* 1.)Decide whats good or bad
		*		The first curly brace should be open
		*		The second curly brace should be close
		*		It should change never
		* 2.)Test to see if it good or bad
		*/

		//If we are at a { or }
		if(inStr.input[i] == '{' || inStr.input[i] == '}')
		{
			//Is it the good mode?
			if(inStr.input[i] == LFGoodMode)
			{
				//If so toggle what you are looking for
				LFGoodMode = (inStr.input[i] == '{') ? '}' : '{';					
			}
			else
			{
				stringstream ss;
				ss << "Character " << i + 1 << ": Brace " << inStr.input[i] << " is invalid in this situation, causes nesting";
				msgBuf.push_back(ss.str());
				return true;
			}
		}
		i++;
	}
	//---------------------------------------------
	i = position;
	return false;
}

//Return if there was an error or not
bool WED_Sign_Parser::ValidateBasics(const InString & inStr, vector<string> & msgBuf)
{
	bool error = false;

	//--Too long---------------------------------------------
	//TODO - Find out if there is a max length

	//-------------------------------------------------------

	int i = 0;
	//---White Space-----------------------------------------
	while(i < inStr.input.length())//Loop for the whole string
	{
		char c = inStr.input[i];
		//If the current character is white space
		//(isspace blows up on a non ascii character, this is our implenetation)
		if( c == ' '  ||
			c == '\t' ||
			c == '\n' ||
			c == '\v' ||
			c == '\f' ||
			c == '\r')
		{
			stringstream ss;
			ss << "Character " << i + 1 << ": Found whitespace";
			msgBuf.push_back(ss.str());
			return true;
		}
		//Increase the pointer and counter
		i++;
	}
	//-------------------------------------------------------

	//Reset variable
	i = 0;

	//--ASCII or supported char------------------------------------------------
	while(i < inStr.input.length())
	{	
		if( ((int) inStr.input[i] < 33 ) || ((int) inStr.input[i] > 126))
		{
			stringstream ss;
			ss << "Character " << i + 1 << ": Character is not valid ASCII";
			msgBuf.push_back(ss.str());
			return true;
		}
		//Check if it is a non supported char (aka NOT A-Z,0-9,.,* etc
		if(!IsSupportedChar(inStr.input[i]))
		{
			stringstream ss;
			ss << "Character " << i + 1 << ": " << inStr.input[i] << " is not supported";
			msgBuf.push_back(ss.str());
			return true;
		}
		i++;
	}
	//-------------------------------------------------------
	i = 0;
	
	//--Starts with valid instruction {@(Y/R/L/B)
	if(inStr.input[0] == '{' && inStr.input[1] == '@')
	{
		switch(inStr.input[2])
		{
		case 'Y':
		case 'R':
		case 'L':
		case 'B':
			error = false;
			break;
		default:
			stringstream ss;
			ss << "Chacters 1-3: " << inStr.input[0] << inStr.input[1] << inStr.input[2] << " is not a valid instruction";
			msgBuf.push_back(ss.str());
			return true;
		}
	}
	else
	{
		stringstream ss;
		ss << "Chacters 1-3: " << inStr.input[0] << inStr.input[1] << inStr.input[2] << " is not a valid instruction";
		msgBuf.push_back(ss.str());
		return true;
	}

	//Validate all curly brace rules
	while(i < inStr.input.length())
	{
		if(inStr.input[i] == '{')
		{
			error = ValidateCurly(inStr,i,msgBuf);
			if(error)
			{
				return error;
			}
		}
		i++;
	}
	i = 0;
	return error;
}


const string & WED_Sign_Parser::EnumToString(FSM in)
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
//Takes in the current state of the FSM, the current character being processes
//The position, Outstr, and msgBuf are all part of reporting errors and are not integral to the FSM
FSM WED_Sign_Parser::LookUpTable(FSM curState, char curChar, int position, OutInfo & str, vector<string> & msgBuf)
{
	stringstream ss;
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
			ss << "Character " << position + 1 << ": " << curChar << " is not allowed there, expected @ or a glyph";
			msgBuf.push_back(ss.str());
			return LOOKUP_ERR;
		case '@':
			return I_ANY_CONTROL;
		default:
			//if it was able to accumulate the the glyph
			str.AccumGlyphBuf(curChar);
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
			str.AccumGlyphBuf(curChar);
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ACCUM_GLPHYS:	
		switch(curChar)
		{
		//Cases to make it stop accumulating
		case '}':
			str.AccumOutputString(str.GetGlyphBuf(),position,msgBuf);
			str.ClearBuf();
			return O_ACCUM_GLYPHS;
		case ',':
			str.AccumOutputString(str.GetGlyphBuf(),position,msgBuf);
			str.ClearBuf();
			return I_COMMA;
		default:
			//otherwise accumulate the glyphs
			str.AccumGlyphBuf(curChar);
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
			str.SetCurColor(curChar);
			return I_WAITING_SEPERATOR;
			
		case '@':
			str.SetWriteMode(false);
			return I_WAITING_SEPERATOR;
		default:
			ss << "Character " << position + 1 << ": {@" << curChar << " is not a real instruction. Use {@Y,{@R,{@L, or {@B";
			msgBuf.push_back(ss.str());
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
			ss << "Character " << position + 1 << ": Was expecting , or }, got " << curChar;//No way you should end up with something like @YX or {@@X
			msgBuf.push_back(ss.str());
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
			//If the current letter is NOT lower-case(part of something like critical or hazard)
			if(!(curChar>=97 && curChar <= 122))
			{
				str.AccumOutputString(&curChar,1,msgBuf);
				return O_ACCUM_GLYPHS;
			}
			else
			{
				ss << "Character " << position + 1 << ": " << curChar << " is not allowed outside curly braces";//This may be impossible to get to
				msgBuf.push_back(ss.str());
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

OutInfo WED_Sign_Parser::MainLoop(const InString & inStr, vector<string> & msgBuf)
{
	//Make the front and back outStrings
	OutInfo outStr;
		
	//Validate if there is any whitesapce or non printable ASCII characters (33-126)
	if(WED_Sign_Parser::ValidateBasics(inStr,msgBuf) == true)
	{
		//sprintf(msgBuf,"String doesn't follow basic rules!");
		//msg=string(msgBuf);
		return outStr;
	}
	
	FSM FSM_MODE = O_ACCUM_GLYPHS;
	int i = 0;
	while(FSM_MODE != O_END)
	{
		//Look up the transition
		FSM transition = WED_Sign_Parser::LookUpTable(FSM_MODE,inStr.input[i], i, outStr, msgBuf);
		if(transition != LOOKUP_ERR)
		{
			FSM_MODE = transition;
			i++;
		}
		else
		{
			//stringstream ss
			//ss << "Fatal lookup error! State: %s, Char: %c, Location: %d",WED_Sign_Parser::EnumToString(FSM_MODE),*(inStr.nPos),(inStr.nPos-inStr.oPos));
			//msgBuf.push_back(ss.str());
			break;
		}
	}
	
	return outStr;
}