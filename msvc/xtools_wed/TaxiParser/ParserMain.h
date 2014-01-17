#pragma once
#include <stdio.h>
#include <string>
#include <conio.h>

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

//Enum for actions to take
enum ACTIONS
{
	APPEND_ACCUMED,
	PREP_CHANGE_COLOR,
	CHANGE_COLOR,
	TOGGLE_SIDE,
	PRINT_MESSAGE,
	THROW_ERROR
};

struct InString
{
	//The original position
	const char * oPos;
	//The moving positino used to parse
	const char * nPos;
	//The end of string, strlen*char+oPos;
	const char * endPos;
	//Place count
	int count;
	InString::InString(const char * inString)
	{
		//Original position
		oPos = inString;

		//Moveable position
		nPos = oPos;

		//End of the string (\0)
		endPos = strlen(inString) * sizeof(char) + oPos;
		count = 0;
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
		count=0;
	}
};

struct OutString
{
	//Front Sign results
	char fRes[1024];

	//Back sign results
	char bRes[1024];

	//Place count (front/back)
	int fCount;
	int bCount;

	//The temporary buffer to fill up
	char curlyBuf[8];
	int curBufCNT;

	//Write to the front buffer
	bool writeToF;

	//The current color
	char curColor;

	OutString::OutString()
	{
		fCount = 0;
		bCount = 0;
		ClearBuf();
		writeToF = true;
		curColor = 'X';//An obviously fake choice
	}
	
	OutString::~OutString()
	{

	}
	void SwitchFrontBack()
	{
		if(writeToF)
			writeToF = false;
	}

	//True for all good, false for buffer overflow
	bool AccumBuffer(char inLet)
	{
		if(curBufCNT < 8)
		{
			curlyBuf[curBufCNT] = inLet;
			curBufCNT++;
			return true;
		}
		else
		{
			printf("Longer than anyknown glyph!");//Semantic
			return false;
		}
	}

	//wipe the contents and reset the counter
	void ClearBuf()
	{
		for (int i = 0; i < 8; i++)
		{
			curlyBuf[i]='\0';
		}
			
		curBufCNT = 0;
	}
	//a letter to appened, front mode = 0, back mode = 1
	void AppendLetter(char * inLetters, int count)
	{
		/* 1.) Choose the front or back
		* 2.) Add /(Y,R,L,B)
		* 3.) Add the letters
		*/
		if(writeToF)
		{
			fRes[fCount] = '/';
			fCount++;
			fRes[fCount] = curColor;
			fCount++;
			for (int i = 0; i < count; i++)
			{
				fRes[fCount] = *(inLetters+i);
				fCount++;
			}
		}
		else
		{
			bRes[bCount] = '/';
			bCount++;
			bRes[bCount] = curColor;
			bCount++;
			for (int i = 0; i < count; i++)
			{
				bRes[bCount] = *(inLetters+i);
				bCount++;
			}
		}
	}

	void PrintString()
	{
		for (int i = 0; i < fCount; i++)
		{
			printf("%c",fRes[i]);
		}
		//if there is a back side
		if(writeToF == false)
		{
			printf("\n");
			for (int i = 0; i < bCount; i++)
			{
				printf("%c",bRes[i]);
			}
		}
	}
};

class ParserMain
{
public:
	ParserMain(void);
	~ParserMain(void);

	int main(int argc, const char* argv[]);
};

