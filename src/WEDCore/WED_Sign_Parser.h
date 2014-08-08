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
	OutInfo: out_sign.front = "/YC/YA/YT"
			 out_sign.back = "/LD/LO/LG"

This could be used for some drawing utility or anything that would like to know the color of every glyph.

The other part of OutInfo is information about about errors it has collected
*/


//Contains all the possible error types that the par can generate
enum error_t
{
	sem_glyph_color_mismatch,
	sem_not_real_instruction,//Found under I_ANY_CONTROL
	sem_not_real_multiglyph,
	
	syn_found_lowercase_outside_curly,//Found under O_ACCUM_GLYPHS
	syn_expected_seperator,//Found under I_Waiting_Seperator
	syn_expected_non_comma_after_incur,//Found under I_INCUR
	syn_expected_non_seperator_after_comma,//Found under I_COMMA
	
	//These are found under validate basic
	syn_nonsupported_char,
	syn_whitespace_found,
	//These are found under ValidateCurly
	syn_curly_pair_missing,
	syn_curly_pair_empty,
	syn_curly_pair_nested
};

struct error {
	string msg;//The human readable version of the error
	error_t err_code;
	int position;//The position in the string the error starts at

	//How many characters the error lasts for. Ex syn_nonsupported_char would be 1
	//while syn_not_real_multiglyph would be 3
	int length;
};

typedef char color_t;
typedef string glyph_t;

//Represents the information about a single or multi glyphs
struct glyph_info
{
	color_t glyph_color;
	glyph_t glyph_name;

	glyph_info(color_t color, glyph_t name)
	{
		glyph_color = color;
		glyph_name = name;
	}
};

//Represents a sign that has been fully encoded with glyph information
//Instead of simply being the input string version
struct finished_sign
{
	vector<glyph_info> front;
	vector<glyph_info> back;

	string toString(const vector<glyph_info> & side)
	{
		string tmp;
		for (int i = 0; i < side.size(); i++)
		{
			glyph_info curGlyph = side[i];
			tmp += '/' + (curGlyph.glyph_color + curGlyph.glyph_name);
		}
		return tmp;
	}
};

class OutInfo
{
friend class UnitTester;
private:
	finished_sign out_sign;

	//Glyph Buffer, stores a glyph that is form
	//{c->{co->{com->{comm->{comma
	string glyphBuf;//TODO - Replace all these inLetters stuff

	//Write to the front buffer
	bool writeMode;

	//The current color
	color_t curColor;
	
	//Error codes in generating the outstring
	vector<error_t> errors;

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
		case 'X'://No color has been selected
			return true;
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
			for (int i = 0; i < inLetters.length(); i++)
			{
				if(disableSemChecks == false)
				{
					bool checkResult = SemCheckColor(inLetters[i]);
					if(checkResult == true)
					{
						stringstream ss;
						ss << "Character " << position << ": " << inLetters[i] << " cannot belong to color type " << curColor;
						msgBuf.push_back(ss.str());
					}
				}
			}
			out_sign.front.push_back(glyph_info(curColor, inLetters));
		}
		else
		{
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
			}
			out_sign.back.push_back(glyph_info(curColor, inLetters));
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

struct InString
{
	//The input for the FSM, with the text from the sign
	const string & input;
	
	InString(const string & signText):input(signText){}
	~InString()	{}
};

class WED_Sign_Parser
{
private:
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

public:
	WED_Sign_Parser(void);
	~WED_Sign_Parser(void);
	static bool ValidateCurly(const InString & inStr, int position, vector<string> & msgBuf);
	static bool ValidateBasics(const InString & inStr, vector<string> & msgBuf);

	//Askes if the glyph is currently one of the special independant glyphs
	//"hazard","safety","critical","no-entry"
	static bool IsMandatoryGlyph(string inLetters);
	//takes in the char and an optional boolean to say wheather to only do lowercase
	static bool IsSupportedChar(char inChar);
	static const string & EnumToString(FSM in);
	static FSM LookUpTable(FSM curState, char curChar, int position, OutInfo & str, vector<string> & msgBuf);
	//The main loop plus and optional InString
	static OutInfo MainLoop(const InString & opInStr, vector<string> & msgBuf);
};

