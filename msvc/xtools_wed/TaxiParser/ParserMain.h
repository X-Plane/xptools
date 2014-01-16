#pragma once
#include <stdio.h>
#include <string>
#include <conio.h>

enum FSM
{
	//The inside curly braces portion, starts with I_
	I_IDLE,//For when we're not sure what will happen
	I_INCUR,//For when we hit {
	I_ACCUM_GLPHYS,//For collectings glpyhs
	I_ANY_CONTROL,//When it hits a @
	I_DOUBLE_CONTROL,//For when it hits a second @
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
	//Front buffer
	char oFBuf[1024];

	//Back buffer
	char oBBuf[1024];

	//Place count (front/back)
	int fCount;
	int bCount;

	OutString::OutString()
	{
		fCount = 0;
		bCount = 0;
	}
	
	OutString::~OutString()
	{

	}

	//a letter to appened, front mode = 0, back mode = 1
	void AppendLetter(char inLetter, int mode=0)
	{
		if(mode==0)
		{
			oFBuf[fCount] = inLetter;
			fCount++;
		}
		else if(mode == 1)
		{
			oBBuf[bCount] = inLetter;
			bCount++;
		}
		else
		{
			printf("You have put in a bad mode!");
		}
	}

	void PrintString()
	{
		for (int i = 0; i <= fCount; i++)
		{
			printf("%c",oFBuf[fCount]);
		}

		printf("\n");
		for (int i = 0; i <= bCount; i++)
		{
			printf("%c",oFBuf[fCount]);
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

