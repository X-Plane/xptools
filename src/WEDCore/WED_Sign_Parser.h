#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

/* Theory of Operation - The sign parser takes in a taxiway sign and analyses it for any errors (syntactic or semantic.)

There are two outputs for this, for 2 different groups of clients. 
The error message collection (vector<string> & msgBuf) collects human readable errors to be shown to the user.

The out_info struct that is generated contains a version of of the sign that tags every glyph with its color.
Ex: in_info: {@Y}CAT{@@}{@L,D,O,G}
	out_info: out_sign.front = "/YC/YA/YT"
			 out_sign.back = "/LD/LO/LG"

This could be used for some drawing utility or anything that would like to know the color of every glyph.

The other part of out_info is information about about errors it has collected
*/


//Contains all the possible error types that the par can generate
enum error_t
{
	sem_glyph_color_mismatch,
	sem_not_real_instruction,//Found under I_ANY_CONTROL
	sem_not_real_multiglyph,
	
	syn_found_lowercase_outside_curly,//Found under O_ACCUM_GLYPHS
	syn_expected_seperator,//Found under I_WAITING_SEPERATOR
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

struct error_info {
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

struct out_info
{
	//Error codes in generating the outstring
	vector<error_info> errors;

	finished_sign out_sign;

	void AddError(string message, error_t error_code, int position, int length)
	{
		error_info e = {message,error_code,position,length};
		errors.push_back(e);
	}
};

struct in_info
{
	//The input for the FSM, with the text from the sign
	const string & input;
	//int position;//TODO - store the current parsing position here?
	in_info(const string & signText):input(signText)/*,position(0)*/{}
	~in_info()	{}
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
	
	//--Semantic checking and handling-------------------------
	//A calls all necissary semantic checks for a glyph and makes changes to the output

	bool preform_semantic_checks(const in_info & inStr, int position, out_info & output);

	//Checks to see if a current multi-glyph or single glyph is allowed with a certain color
	//Returns true if there was an error
	bool check_color(const string & inGlyph, int position,out_info & output);

	//Checks to see if a certain multi_glyph is a valid glyph
	bool check_multi_glyph(const string & inGlyph, int position, out_info & output);
	//---------------------------------------------------------
	
	//--out_info modifying code--------------------------------
	void append_out_info(const string & inGlyph, int position, out_info & output);
	//---------------------------------------------------------


	//Askes if the glyph is currently one of the special independant glyphs
	//"hazard","safety","critical","no-entry"
	bool IsIndependentGlyph(string inLetters);//may be a secret duplicate of check_multi_glyph

	

	//--Syntax Checking functions------------------------------
	//takes in the char and an optional boolean to say wheather to only do lowercase
	bool IsSupportedChar(char inChar);

	bool ValidateCurly(const in_info & inStr, int position, out_info & output);
	bool ValidateBasics(const in_info & inStr, out_info & output);
	//---------------------------------------------------------

	//--FSM functions------------------------------------------
	//A state-to-string conversion function, it is simply for generating messages
	const string & EnumToString(FSM in);

	//The heart of the parser, the FSM look up table. Position and output are simply for error messages
	FSM LookUpTable(FSM curState, char curChar, int position, out_info & output);
	//---------------------------------------------------------

	//--FSM data members---------------------------------------
	//The current color, allowed values are Y,R,L,B,I(for the magic independant glyphs), and X for invalid
	color_t curColor;
	
	//If we are still on the front
	bool on_front;

	//Glyph Buffer, stores a glyph that is form
	//{c->{co->{com->{comm->{comma
	string glyphBuf;
	//---------------------------------------------------------
public:
	WED_Sign_Parser(void);
	~WED_Sign_Parser(void);
	
	void MainLoop(const in_info & inStr, out_info & output);
};

