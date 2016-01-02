#ifndef WED_Sign_Parser_H
#define WED_Sign_Parser_H

#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

/* Theory of Operation - The sign parser takes in a taxiway sign and analyses it for any errors (syntactic or semantic.)

There are (currently) two outputs for this, for 2 different groups of clients. These are stored in parser_out_info

vector<parser_error_info> errors contains a vector of error_infos which store an error code and human readable form
parser_finished_sign out_sign is a version of of the sign that tags every glyph with its color.
Ex: in_info: {@Y}CAT{@@}{@L,D,O,G}
	out_info: out_sign.front = "/YC/YA/YT"
			  out_sign.back = "/LD/LO/LG"

This could be used for some drawing utility or anything that would like to know the color of every glyph.
*/


//Contains all the possible error types that the par can generate
enum parser_error_t {
	// Syntax errors.  When these happen we -may- get cascading error hell.
	// Error recovery varies based on what the user intended and how screwed up
	// the sign is.

	syn_not_real_instruction,			// @ command with an illegal code.
	syn_not_real_multiglyph,			// Bad glyph name inside {}
	syn_not_real_singleglyph,			// BAd glyph name outside {}

	syn_found_at_symbol_outside_curly,	// @ command outside {}
	syn_expected_seperator,				// After an @ command there's more junk before a } or ,
	syn_empty_multiglyph,				// The multi-glyph is empty, e.g. {,B}

	syn_curly_pair_missing,				// when we get { and the glyph ends
	syn_curly_pair_nested,				// When we get {{
	syn_curly_unbalanced,				// When we get a } without a {

	// Semantic errors.  When these happen, the parse is unambiguous,
	// it's just that something illegal has been requested.
	
	sem_pipe_begins_sign,				// {@Y}|FOO
	sem_pipe_color_mismatch,			// {@Y}FOO{@L}|{@Y}BAR
	sem_pipe_double_juxed,				// {@Y}FOO||BAR
	sem_pipe_ends_sign,					// {@Y}BAR|

	sem_glyph_color_mismatch,			// Glyph has illegal color
	sem_mutiple_side_switches,			// More than one @@ command.
	
	parser_error_count
};

struct parser_error_info {
	string msg;//The human readable version of the error, DOES NOT end with a \n. The client decides if they want one
	parser_error_t err_code;
	int position;//The position in the string the error starts at

	//How many characters the error lasts for. Ex syn_nonsupported_char would be 1
	//while syn_not_real_multiglyph would be 3
	int length;
};

enum parser_color_t{
	sign_color_yellow		= 'Y',	// Yellow (with black text)
	sign_color_red			= 'R',	// Red (with white text)
	sign_color_location		= 'L',	// Location (black with yellow text and border)
	sign_color_black		= 'B',	// Black (with white text for distance remaining)
	sign_color_independent	= 'I',	// Independent
	sign_color_invalid		= 'X'	// Invalid
};

enum parser_glyph_t {
	glyph_Invalid = -1,
	glyph_A = 0,			// Letters
	glyph_B,
	glyph_C,
	glyph_D,
	glyph_E,
	glyph_F,
	glyph_G,
	glyph_H,
	glyph_I,
	glyph_J,
	glyph_K,
	glyph_L,
	glyph_M,
	glyph_N,
	glyph_O,
	glyph_P,
	glyph_Q,
	glyph_R,
	glyph_S,
	glyph_T,
	glyph_U,
	glyph_V,
	glyph_W,
	glyph_X,
	glyph_Y,
	glyph_Z,
	glyph_0,
	glyph_1,				// Numbers
	glyph_2,
	glyph_3,
	glyph_4,
	glyph_5,
	glyph_6,
	glyph_7,
	glyph_8,
	glyph_9,
	glyph_dash,				// Hyphen -
	glyph_dot,				// dot is * in sign code
	glyph_period,
	glyph_slash,
	glyph_space,			// space is _ in sign code
	glyph_separator,		// pipe | separator
	glyph_comma,			// , outside braces or comma inside
	
	glyph_up,				// 8 direction arrows
	glyph_down,
	glyph_left,
	glyph_right,
	glyph_leftup,
	glyph_rightup,
	glyph_leftdown,
	glyph_rightdown,
	glyph_critical,			// "ladder" - ILS critical area
	glyph_hazard,			// diagonal yellow/black slashes - hazardous
	glyph_no_entry,			// Do not enter - white sign and slash on red background
	glyph_safety,			// hold short for safety - solid and dashed lines
	glyph_r1,				// roman numeral cat I, II, III
	glyph_r2,
	glyph_r3
};

//#error TODO
/*
	create a glyph table off of the glyph IDs with valid colors, in and out of bracket spellings, etc.
*/

string	parser_name_for_glyph(parser_glyph_t glyph);
bool	parser_is_color_legal(parser_glyph_t, parser_color_t);


//Represents the information about a single or multi glyphs
struct parser_glyph_info
{
	parser_color_t glyph_color;
	parser_glyph_t glyph_name;

	parser_glyph_info(parser_color_t color, parser_glyph_t name)
	{
		glyph_color = color;
		glyph_name = name;
	}
};

//Represents a sign that has been fully encoded with glyph information
//Instead of simply being the input string version
struct parser_finished_sign
{
	vector<parser_glyph_info> front;
	vector<parser_glyph_info> back;

	string toDebugString(const vector<parser_glyph_info> & side)
	{
		string tmp;
		for (int i = 0; i < side.size(); i++)
		{
			parser_glyph_info curGlyph = side[i];
			tmp += '/' + ((char)curGlyph.glyph_color + parser_name_for_glyph(curGlyph.glyph_name));
		}
		return tmp;
	}
};

//A struct containing the collected information durring the parse
struct parser_out_info
{
	//Errors collected during the process
	vector<parser_error_info> errors;

	//A "per-glyph" version of the input sign string with a front and a back
	parser_finished_sign out_sign;

	void AddError(string message, parser_error_t error_code, int position, int length)
	{
		parser_error_info e = {message,error_code,position,length};
		errors.push_back(e);
	}
};

struct parser_in_info
{
	//The input for the FSM, with the text from the sign
	const string & input;
	//int position;//TODO - store the current parsing position here?
	parser_in_info(const string & signText):input(signText)/*,position(0)*/{}
	~parser_in_info(){}
};

//All you need to do to parse a sign is pass in an input and a blank output
void ParserTaxiSign(const parser_in_info & input, parser_out_info & output);

#endif
