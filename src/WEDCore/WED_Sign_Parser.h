#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

/* Theory of Operation - The sign parser takes in a taxiway sign and analyses it for any errors (syntactic or semantic.)

There are two outputs for this, for 2 different groups of clients. 
The error message collection (vector<string> & msgBuf) collects human readable errors to be shown to the user.

The OutInfo struct that is generated contains a version of of the sign that tags every glyph with its color.
Ex: InString: {@Y}CAT{@@}{@L,D,O,G}
	OutInfo: fRes = "/YC/YA/YT"
			 bRes = "/LD/LO/LG"

This could be used for some drawing utility or anything that would like to know the color of every glyph.

The other part of OutInfo is information about about errors it has collected
*/
enum FSM
{
	//The inside curly braces portion, starts with I_
	I_COMMA,//We just hit a comma and are now expecting single
	//or multiglyphs
	I_INCUR,//For when we hit {
	I_ACCUM_GLPHYS,//For collecting glpyhs
	I_ANY_CONTROL,//When it hits a @
	I_WAITING_SEPERATOR,//For when it is waiting for a , or }
	//The outside curly braces portion, starts with O_
	O_ACCUM_GLYPHS,
	O_END,//For when the string ends
	LOOKUP_ERR//Return code for any errors in the lookup table
};

enum parser_error_code_t
{

};


struct InString
{
	//The input for the FSM, with the text from the sign
	const string & input;
	
	InString(const string & signText):input(signText){}
	~InString()	{}
};

class OutInfo
{
friend class UnitTester;
private:
	//Front Sign results
	string fRes;
	string bRes;

	//Glyph Buffer, stores a glyph that is form
	//{c->{co->{com->{comm->{comma
	string glyphBuf;//TODO - Replace all these inLetters stuff

	//Write to the front buffer
	bool writeMode;

	//The current color
	char curColor;
	
	//Error codes in generating the outstring
	int error;

	//Check the color, inLetter:the Letter to check, position: the position in the array of chars, msgBuf: the message buffer
	//Returns true if there was an error
	bool SemCheckColor(char inLetter)
	{
		//Go in as far as it can go
		switch(curColor)
		{
		//Y and R allow for all characters
		case 'L':
			//L can only support A-Z and 0-9, the bellow checks the relevant
			if(!((inLetter >= 65 && inLetter <= 90) ||
				(inLetter >= 48 && inLetter <= 57)))
			{
				return true;
			}
			break;
		case 'B':
			//B can only support 0-9 (ASCII letters 48 through 57)
			if(!(inLetter >= 48 && inLetter <= 57))
			{
				return true;
			}
			break;
		default:
			break;
		}
		return false;
	}

	//Check a multi glyph
	//Returns true if there was an error
	bool SemCheckMultiGlyph(const string & inLetters)
	{
		//Assume there is something wrong until otherwise noted
		bool semError = true;

		//Based on the letter, preform a bunch of string compares
		//if it is a perfect match for any of the real multiletter glyphs
		switch(inLetters[0])
		{
		case '^':
			if((inLetters == "^u") == true ||
				(inLetters == "^d") == true ||
				(inLetters == "^r") == true ||
				(inLetters == "^l") == true||
				(inLetters == "^lu") == true ||
				(inLetters == "^ru") == true ||
				(inLetters == "^ld") == true ||
				(inLetters == "^rd") == true)
			{
				semError = false;
			}
			break;
		case 'c':
			if( (inLetters == "critical") == true||
				(inLetters == "comma") == true)
			{
				semError = false;
			}
			break;
		case 'h':
			if((inLetters == "hazard") == true)
			{
				semError = false;
			}
			break;
		case 'n':
			if((inLetters == "no-entry") == true)
			{
				semError = false;
			}
			break;
		case 'r':
			if((inLetters == "r1") == true||
				(inLetters == "r2") == true||
				(inLetters == "r3") == true)
			{
				semError = false;
			}
			break;
		case 's':
			if((inLetters == "safety") == true)
			{
				semError = false;
			}
			break;
		//For all other letters
		default:
			semError = true;//It will next print the error warning
			break;
		}
		return semError;
	}

public:
	OutInfo()
	{
		writeMode = true;
		curColor = 'X';//An obviously fake choice
	}
	
	~OutInfo()
	{

	}

	//Attempts to add a collection of letters
	void AccumOutputString(const string & inLetters, int position, vector<string> & msgBuf)
	{
		//Before actually appending them see if they're
		//correct semantically

		//Meaning we are in multiglyph mode
		if(inLetters.length() > 1)
		{
			bool checkResult = SemCheckMultiGlyph(inLetters);
			if(checkResult == true)
			{
				stringstream ss;
				ss << "Character " << position - inLetters.length() + 1 << "-" << position << ": " << inLetters << " is not a real multiglyph";
				msgBuf.push_back(ss.str());
			}
		}
		
		bool disableSemChecks = false;
		if( inLetters == "critical" ||
			inLetters == "no-entry" ||
			inLetters == "safety"   ||
			inLetters == "hazard"   )
		{
			disableSemChecks = true;
		}
		//Check to see if the letter is supported by the 
		/* 1.) Choose the front or back
		* 2.) Add /(Y,R,L,B)
		* 3.) Add the letters
		*/
		if(writeMode)
		{
			fRes += '/';
			fRes += curColor;
			for (int i = 0; i < inLetters.length(); i++)
			{
				if(disableSemChecks == false)
				{
					bool checkResult = SemCheckColor(inLetters[i]);
					if(checkResult == true)
					{
						stringstream ss;
						ss << "Character " << position + 1 << ": " << inLetters[i] << " cannot belong to color type " << curColor;
						msgBuf.push_back(ss.str());
					}
				}
				fRes += inLetters[i];
			}
		}
		else
		{
			bRes += '/';
			bRes += curColor;
			for (int i = 0; i < inLetters.length(); i++)
			{
				if(disableSemChecks == false)
				{
					bool checkResult = SemCheckColor(inLetters[i]);
					if(checkResult == true)
					{
						stringstream ss;
						ss << "Character " << position + 1 << ": " << inLetters[i] << " cannot belong to color type " << curColor;
						msgBuf.push_back(ss.str());
					}
				}
				bRes += inLetters[i];
			}
		}
	}

	//Attempts to add letters to the glyph buffer
	//TODO - Seems true or false doesn't matter because glyph size is semantic
	//not syntactic
	bool AccumGlyphBuf(char inLetter)
	{
		if(true)//if(glyphBuf.length() < 8)
		{
			glyphBuf += inLetter;
			return true;
		}
		else
		{
			return false;
		}
	}	

	void ClearBuf()
	{
		glyphBuf.clear();
	}

	string GetGlyphBuf()
	{
		return glyphBuf;
	}

	void SetWriteMode(bool mode)
	{
		writeMode = mode;
	}

	char GetCurColor()
	{
		return curColor;
	}

	void SetCurColor(char in)
	{
		curColor = in;
	}
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
	static FSM LookUpTable(FSM curState, char curChar, int position, OutInfo & str, vector<string> & msgBuf);
	//The main loop plus and optional InString
	static OutInfo MainLoop(const InString & opInStr, vector<string> & msgBuf);
};

