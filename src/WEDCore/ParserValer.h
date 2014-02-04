#pragma once
#include <stdio.h>
#include <string>
#if IBM
#include <conio.h>
#endif
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
	//The original position
	const char * oPos;
	//The moving positino used to parse
	const char * nPos;
	//The end of string, strlen*char+oPos;
	const char * endPos;

	InString(const char * inString)
	{
		//Original position
		oPos = inString;

		//Moveable position
		nPos = oPos;

		//End of the string (\0)
		//endPos = strlen(inString) * sizeof(char) + oPos;
		endPos = nPos;
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
	OutString(char * front=NULL,char * back=NULL)
	{
		//Wipe out the contents of outStr
		for (int i = 0; i < BUFLEN; i++)
		{
			fRes[i]='\0';
			bRes[i]='\0';
		}
		if(front != NULL)
		{
			strcpy(fRes,front);
		}
		if(back != NULL)
		{
			strcpy(bRes,back);
		}
		ClearBuf();
		writeToF = true;
		curColor = 'X';//An obviously fake choice
	}
	
	~OutString()
	{

	}

	//True for all good, false for buffer overflow
	bool AccumBuffer(char inLet,char * msgBuf)
	{
		if(strlen(curlyBuf) < 8)
		{
			curlyBuf[strlen(curlyBuf)] = inLet;
			return true;
		}
		else
		{
			strcpy(msgBuf,"Longer than anyknown glyph!");//Semantic
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
	
	//Check the color
	void SemCheckColor(char inLetter, char * msgBuf)
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
				sprintf(msgBuf,"%c cannot belong to color type %c",inLetter,curColor);
			}
			break;
		case 'B':
			//B can only support 0-9 (ASCII letters 48 through 57)
			if(!(inLetter >= 48 && inLetter <= 57))
			{
				sprintf(msgBuf,"%c cannot belong to color type %c",inLetter,curColor);
			}
			break;
		default:
			break;
		}
	}

	//Check a multi glyph
	void SemCheckMultiGlyph(char * inLetters, char * msgBuf)
	{
		//Assume there is something wrong until otherwise noted
		bool semError = true;

		//Based on the letter, preform a bunch of string compares
		//if it is a perfect match for any of the real multiletter glyphs
		switch(*(inLetters))
		{
		case '^':
			if(strcmp("^u",inLetters) == 0 ||
				strcmp("^d",inLetters) == 0 ||
				strcmp("^r",inLetters) == 0 ||
				strcmp("^l",inLetters) == 0||
				strcmp("^lu",inLetters) == 0 ||
				strcmp("^ru",inLetters) == 0 ||
				strcmp("^ld",inLetters) == 0 ||
				strcmp("^rd",inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'c':
			if( strcmp("critical",inLetters) == 0||
				strcmp("comma",inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'h':
			if(strcmp("hazard",inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'n':
			if(strcmp("no-entry",inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 'r':
			if(strcmp("r1",inLetters) == 0||
				strcmp("r2",inLetters) == 0||
				strcmp("r3",inLetters) == 0)
			{
				semError = false;
			}
			break;
		case 's':
			if(strcmp("safety",inLetters) == 0)
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
			sprintf(msgBuf,"%s is not a real multiglyph!",inLetters);
		}
	}

	//a letter to appened, front mode = 0, back mode = 1
	void AppendLetter(char * inLetters, int count, char * msgBuf)
	{
		//Before actually appending them see if they're
		//correct semantically

		//Meaning we are in multiglyph mode
		if(count > 1)
		{
			SemCheckMultiGlyph(inLetters,msgBuf);
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
				SemCheckColor(*(inLetters+i),msgBuf);
				fRes[strlen(fRes)] = *(inLetters+i);
			}
		}
		else
		{
			bRes[strlen(bRes)] = '/';
			bRes[strlen(bRes)] = curColor;
			for (int i = 0; i < count; i++)
			{
				SemCheckColor(*(inLetters+i),msgBuf);
				bRes[strlen(bRes)] = *(inLetters+i);
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
class ParserValer
{
public:
	ParserValer(void);
	~ParserValer(void);
	static bool ValidateCurly(InString * inStr, char * msgBuf);
	static bool ValidateBasics(InString * inStr, char * msgBuf);
	//takes in the char and an optional boolean to say wheather to only do lowercase
	static bool IsSupportedChar(char inChar);
	static const char * EnumToString(FSM in);
	static FSM LookUpTable(FSM curState, char curChar, OutString * str, char * msgBuf);
	//The main loop plus and optional InString
	static OutString MainLoop(InString * opInStr, string * msg);
};

