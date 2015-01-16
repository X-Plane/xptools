#pragma once
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
enum parser_error_t
{
	sem_glyph_color_mismatch,//Found under check_color
	sem_no_color,//Found under preform_final_semantic_checks
	sem_not_real_instruction,//Found under I_ANY_CONTROL
	sem_not_real_multiglyph,//Found under check_multi_glyph
	sem_not_real_singleglyph,//Foudn nder check_single_glyph
	sem_mutiple_side_switches,//Found under I_ANY_CONTROL:case '@':
	
	//Found in preform final semantic checks
	sem_pipe_begins_sign,
	sem_pipe_color_mismatch,///YF|//RD from {@Y,F}|{@R,D}
	sem_pipe_double_juxed, //||
	sem_pipe_ends_sign,
	sem_pipe_l_sign_flip_juxed,//Juxed = juxtapositioned
	sem_pipe_r_sign_flip_juxed,
	
	syn_found_at_symbol_outside_curly,//Found under O_ACCUM_GLYPHS
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

struct parser_error_info {
	string msg;//The human readable version of the error, DOES NOT end with a \n. The client decides if they want one
	parser_error_t err_code;
	int position;//The position in the string the error starts at

	//How many characters the error lasts for. Ex syn_nonsupported_char would be 1
	//while syn_not_real_multiglyph would be 3
	int length;
};

/* Possible values
	'Y'ellow
	'R'ed
	'L'ocation
	'B'lack
	'P'ipe
	'I'ndependent
	'X'invalid
*/
typedef char parser_color_t;

//The text component of a single or multi letter glyph
typedef string parser_glyph_t;

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

	string toString(const vector<parser_glyph_info> & side)
	{
		string tmp;
		for (int i = 0; i < side.size(); i++)
		{
			parser_glyph_info curGlyph = side[i];
			tmp += '/' + (curGlyph.glyph_color + curGlyph.glyph_name);
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