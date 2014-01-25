#pragma once
#include <stdio.h>
#include <string>
#include <conio.h>
#define BUFLEN 256 //controls how long the out string (fRes,bRes) can be
#define PAUSE 0 

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

	InString::InString(const char * inString)
	{
		//Original position
		oPos = inString;

		//Moveable position
		nPos = oPos;

		//End of the string (\0)
		//endPos = strlen(inString) * sizeof(char) + oPos;
		endPos = nPos;
	}
	InString::~InString()
	{

	}

	//Resets the NPos to where you want it to be,
	//Called at the end of an operation on the string
	//be careful when and where you reset the nPos!
	void ResetNPos(const char * optionalOPos=NULL)
	{
		//Reset to the optionalOPos if there is one
		if(optionalOPos != NULL)
		{
			nPos=optionalOPos;
		}
		else
		{
			nPos = oPos;//Otherwise put the nPos back to the
			//original
		}
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
	OutString::OutString(char * front=NULL,char * back=NULL)
	{
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
	
	OutString::~OutString()
	{

	}

	//True for all good, false for buffer overflow
	bool AccumBuffer(char inLet)
	{
		if(strlen(curlyBuf) < 8)
		{
			curlyBuf[strlen(curlyBuf)] = inLet;
			return true;
		}
		else
		{
			printf("\nLonger than anyknown glyph!");//Semantic
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

	void SemCheckColor(char inLetter)
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
				printf("\n%c cannot belong to color type %c",inLetter,curColor);
			}
			break;
		case 'B':
			//B can only support 0-9 (ASCII letters 48 through 57)
			if(!(inLetter >= 48 && inLetter <= 57))
			{
				printf("\n%c cannot belong to color type %c",inLetter,curColor);
			}
			break;
		default:
			break;
		}
	}

	void SemCheckMultiGlyph(char * inLetters)
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
			printf("\n%s is not a real multiglyph!\n",inLetters);
		}
	}

	//Returns true if it does, false if not
	bool ContainsLower(char * inLetters, int count)
	{
		for (int i = 0; i < count; i++)
		{
			if(inLetters[i] >=97 && inLetters[i] <= 122)
			{
				return true;
			}
		}
		return false;
	}
	//a letter to appened, front mode = 0, back mode = 1
	void AppendLetter(char * inLetters, int count)
	{
		//Before actually appending them see if they're
		//correct semantically

		//Meaning we are in multiglyph mode
		if(count > 1)
		{
			SemCheckMultiGlyph(inLetters);
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
				SemCheckColor(*(inLetters+i));
				fRes[strlen(fRes)] = *(inLetters+i);
			}
		}
		else
		{
			bRes[strlen(bRes)] = '/';
			bRes[strlen(bRes)] = curColor;
			for (int i = 0; i < count; i++)
			{
				SemCheckColor(*(inLetters+i));
				bRes[strlen(bRes)] = *(inLetters+i);
			}
		}
	}

	void PrintString()
	{
		for (int i = 0; i < strlen(fRes); i++)
		{
			printf("%c",fRes[i]);
		}
		//if there is a back side
		if(writeToF == false)
		{
			printf("\n");
			for (int i = 0; i < strlen(bRes); i++)
			{
				printf("%c",bRes[i]);
			}
		}
	}
};
class ParserValer
{
public:
	ParserValer(void);
	~ParserValer(void);
	static bool ValidateCurly(InString * inStr);
	static bool ValidateBasics(InString * inStr);
	//takes in the char and an optional boolean to say wheather to only do lowercase
	static bool IsSupportedChar(char inChar);
	static char * EnumToString(FSM in);
	static FSM LookUpTable(FSM curState, char curChar, OutString * str);
	//The main loop plus and optional InString
	static OutString MainLoop(InString * opInStr=NULL);
};

