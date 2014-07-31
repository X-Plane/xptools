#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#define BUFLEN 256 //controls how long the out string (fRes,bRes) can be

enum FSM
{
	//The inside curly braces portion, starts with I_
	I_COMMA,//We just hit a comma and are now expecting single
	//or multiglyphs
	I_INCUR,//For when we hit {
	I_ACCUM_GLPHYS,//For collectings glpyhs
	I_ANY_CONTROL,//When it hits a @
	I_WAITING_SEPERATOR,//For when it is waiting for a , or }
	//The outside curlybraces portion, starts with O_
	O_ACCUM_GLYPHS,
	O_END,//For when the string ends
	LOOKUP_ERR//Return code for any errors in the lookup table
	
};

struct InString
{
	//The input for the FSM, with the text from the sign
	const string & input;
	
	InString(const string & signText):input(signText)
	{
		
	}

	~InString()
	{

	}
};

struct OutString
{
	//Front Sign results
	char fRes[BUFLEN];

	//Back sign results
	char bRes[BUFLEN];

	//The temporary buffer to fill up, make 8 chars + \0
	char curlyBuf[9];

	//Write to the front buffer
	bool writeToF;

	//The current color
	char curColor;
	
	//Error codes in generating the outstring
	int error;

	OutString(string & front, string & back)
	{
		//Wipe out the contents of outStr
		for (int i = 0; i < BUFLEN; i++)
		{
			fRes[i]='\0';
			bRes[i]='\0';
		}
		
		ClearBuf();
		writeToF = true;
		curColor = 'X';//An obviously fake choice
	}
	
	~OutString()
	{

	}

	//True for all good, false for buffer overflow
	bool AccumBuffer(char inLet, vector<string> & msgBuf)
	{
		if(strlen(curlyBuf) < 8)
		{
			curlyBuf[strlen(curlyBuf)] = inLet;
			return true;
		}
		else
		{
			stringstream ss;
			ss << "Longer than anyknown glyph!";//Semantic
			msgBuf.push_back(ss.str());
			return false;
		}
	}

	//wipe the contents and reset the counter
	void ClearBuf()
	{
		for (int i = 0; i < 9; i++)
		{
			curlyBuf[i]='\0';
		}
	}
	
	//Check the color, inLetter:the Letter to check, position: the positoin in the array of chars, msgBuf: the message buffer
	void SemCheckColor(char inLetter, int position, vector<string> & msgBuf)
	{
		//Go in as far as it can go
		switch(curColor)
		{
		//Y and R allow for all charecters
		case 'L':
			//L can only support A-Z and 0-9, the bellow checks the relevant
			if(!((inLetter >= 65 && inLetter <= 90) ||
				(inLetter >= 48 && inLetter <= 57)))
			{
				stringstream ss;
				ss << "Charecter " << position + 1 << ": " << inLetter << " cannot belong to color type " << curColor;
				msgBuf.push_back(ss.str());
			}
			break;
		case 'B':
			//B can only support 0-9 (ASCII letters 48 through 57)
			if(!(inLetter >= 48 && inLetter <= 57))
			{
				stringstream ss;
				ss << "Charecter " << position + 1 << ": " << inLetter << " cannot belong to color type " << curColor;
				msgBuf.push_back(ss.str());
			}
			break;
		default:
			break;
		}
	}

	//Check a multi glyph
	void SemCheckMultiGlyph(const string & inLetters, int position, vector<string> & msgBuf)
	{
		//Assume there is something wrong until otherwise noted
		bool semError = true;

		//Based on the letter, preform a bunch of string compares
		//if it is a perfect match for any of the real multiletter glyphs
		switch(inLetters[0])
		{
		case '^':
			if(("^u" == inLetters) == 0 ||
				("^d" == inLetters) == 0 ||
				("^r" == inLetters) == 0 ||
				("^l" == inLetters) == 0||
				("^lu" == inLetters) == 0 ||
				("^ru" == inLetters) == 0 ||
				("^ld" == inLetters) == 0 ||
				("^rd" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'c':
			if( ("critical" == inLetters) == 0||
				("comma" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'h':
			if(("hazard" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'n':
			if(("no-entry" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'r':
			if(("r1" == inLetters) == 0||
				("r2" == inLetters) == 0||
				("r3" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 's':
			if(("safety" == inLetters) == 0)
			{
				semError = false;
			}
			break;
		//For all other letters
		default:
			semError = true;//It will next print the error warning
			break;
		}
		if(semError == true)
		{
			stringstream ss;
			ss << "Character " << inLetters.length() - position + 1 << "-" << position + 1 << ": " << inLetters << " is not a real multiglyph";
			msgBuf.push_back(ss.str());
		}
	}

	//a letter to appened, front mode = 0, back mode = 1
	void AppendLetter(const string & inLetters, int count, vector<string> & msgBuf)
	{
		//Before actually appending them see if they're
		//correct semantically

		//Meaning we are in multiglyph mode
		if(count > 1)
		{
			SemCheckMultiGlyph(inLetters,count,msgBuf);
		}
		
		//Check to see if the letter is supported by the 
		/* 1.) Choose the front or back
		* 2.) Add /(Y,R,L,B)
		* 3.) Add the letters
		*/
		if(writeToF)
		{
			fRes[strlen(fRes)] = '/';
			fRes[strlen(fRes)] = curColor;
			for (int i = 0; i < count; i++)
			{
				SemCheckColor(inLetters[i],i,msgBuf);
				fRes[strlen(fRes)] = inLetters[i];
			}
		}
		else
		{
			bRes[strlen(bRes)] = '/';
			bRes[strlen(bRes)] = curColor;
			for (int i = 0; i < count; i++)
			{
				SemCheckColor(inLetters[i],i,msgBuf);
				bRes[strlen(bRes)] = inLetters[i];
			}
		}
	}

#if DEV
	void PrintString()
	{
		for (int i = 0; i < strlen(fRes); i++)
		{
			printf("%c",fRes[i]);
		}
		//if there is a back side
		if(writeToF == false)
		{
			printf("");
			for (int i = 0; i < strlen(bRes); i++)
			{
				printf("%c",bRes[i]);
			}
		}
	}
#endif
};
class WED_Sign_Parser
{
public:
	WED_Sign_Parser(void);
	~WED_Sign_Parser(void);
	static bool ValidateCurly(const InString & inStr, int position, vector<string> & msgBuf);
	static bool ValidateBasics(const InString & inStr, vector<string> & msgBuf);
	//takes in the char and an optional boolean to say wheather to only do lowercase
	static bool IsSupportedChar(char inChar);
	static const string & EnumToString(FSM in);
	static FSM LookUpTable(FSM curState, char curChar, int position, OutString & str, vector<string> & msgBuf);
	//The main loop plus and optional InString
	static OutString MainLoop(const InString & opInStr, vector<string> & msgBuf);
};

