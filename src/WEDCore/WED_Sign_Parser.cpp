#pragma once
#include "WED_Sign_Parser.h"

//--WED_Sign_Parser class decleration--------------------------
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
	
	//Preforms any last semantic checking that is inconvient to do during the FSM, mostly | stuff
	//Returns true if error was found
	bool preform_final_semantic_checks(const parser_in_info & inStr, parser_out_info & output);

	//Checks to see if a current multi-glyph or single glyph is allowed with a certain color
	//Returns true if there was an error
	bool check_color(const string & inGlyph, int position,parser_out_info & output);

	//Checks to see if a certain multi_glyph is a valid glyph
	bool check_multi_glyph(const string & inGlyph, int position, parser_out_info & output);

	//Askes if the glyph is currently one of the special independant glyphs
	//"hazard","safety","critical","no-entry"
	bool IsIndependentGlyph(const string & inGlyph);//may be a secret duplicate of check_multi_glyph
	//---------------------------------------------------------
	
	//--parser_out_info modifying code--------------------------------
	void append_parser_out_info(const string & inGlyph, int position, parser_out_info & output);
	//---------------------------------------------------------


	//--Syntax Checking functions------------------------------
	//takes in the char and an optional boolean to say wheather to only do lowercase
	bool IsSupportedChar(char inChar);

	bool ValidateCurly(const parser_in_info & inStr, int position, parser_out_info & output);
	bool ValidateBasics(const parser_in_info & inStr, parser_out_info & output);
	//---------------------------------------------------------

	//--FSM functions------------------------------------------
	//A state-to-string conversion function, it is simply for generating messages
	const string & EnumToString(FSM in);

	//The heart of the parser, the FSM look up table. Position and output are simply for error messages
	FSM LookUpTable(FSM curState, char curChar, int position, parser_out_info & output);
	//---------------------------------------------------------

	//--FSM data members---------------------------------------
	//The current color, allowed values are Y,R,L,B,I(for the magic independant glyphs),P for pipe, and X for invalid
	parser_color_t curColor;
	
	//If we are still on the front
	bool on_front;

	//Glyph Buffer, stores a glyph that is form
	//{c->{co->{com->{comm->{comma
	string glyphBuf;
	//---------------------------------------------------------
public:
	WED_Sign_Parser(void);
	~WED_Sign_Parser(void);
	
	void MainLoop(const parser_in_info & input, parser_out_info & output);
};
//------------------------------------------------------------------------
WED_Sign_Parser::WED_Sign_Parser(void)
{
	curColor = 'X';//Must start as X and no other color because further code makes assumptions about it being X (preform_final_sem_checks)!
	on_front = true;//Also must start as writing to the front
}

WED_Sign_Parser::~WED_Sign_Parser()
{
}

//--Semantic checking and handling-------------------------
bool WED_Sign_Parser::preform_final_semantic_checks(const parser_in_info & inStr, parser_out_info & output)
{
	/*Final Checks
	* Did we have some actual glyph in the whole sign?
	* Pipe Bar Rules
		* A Pipe must have a non-pipe,non-sign-flip, on both sides of it now known as A and B
		** A and B must be of the same color
		*** Sign cannot begin or end with a pipe bar
	*/
	stringstream ss;
	bool foundError = false;

	
	//glyphCount is a persistent counter so when the sign flips over the counter doesn't get lost

	int glyphCount = 0;
	for (int i = 0; i < output.out_sign.front.size(); i++, glyphCount++)
	{
		if(output.out_sign.front[i].glyph_color == 'X')
		{
			ss << "Glyph " << glyphCount + 1 << ": Glyph " << output.out_sign.front[i].glyph_name << " has no color, must declare color instruction before it";
			output.AddError(ss.str(),sem_no_color,i,output.out_sign.front[i].glyph_name.size());
			ss.str("");
			ss.clear();
			
		}
	}
	
	for (int i = 0; i < output.out_sign.back.size(); i++, glyphCount++)
	{
		if(output.out_sign.back[i].glyph_color == 'X')
		{
			ss << "Glyph " << glyphCount + 1 << ": Glyph "  << output.out_sign.back[i].glyph_name << " has no color, must declare color instruction before it";
			output.AddError(ss.str(),sem_no_color,i,output.out_sign.back[i].glyph_name.size());
			ss.str("");
			ss.clear();
		}
	}
		
	//Pipebar rules
	bool followsPipeJuxRules = false;

	//Start at at place where you could have a {@@}
	for (int i = 4; i < inStr.input.size(); i++)
	{
		if(inStr.input[i] == '|')
		{
			string threeBefore = inStr.input.substr(i-3,3);
			//Try to catch ..@@}|
			if(threeBefore == "@@}")
			{
				followsPipeJuxRules = false;

				ss << "Charecter " << i + 1 << ": Pipe bar has sign face flip instruction directly to its left";
				output.AddError(ss.str(),sem_pipe_l_sign_flip_juxed,i,3);
				ss.str("");
				ss.clear();
				foundError = true;
			}
						
			//If there is space for a {@@
			if(i + 3 < inStr.input.size())
			{
				string threeAfter = inStr.input.substr(i+1,3);
				
				//Try to catch any |{@@...
				if(threeAfter == "{@@")
				{
					followsPipeJuxRules = false;

					ss << "Charecter " << i + 1 << ": Pipe bar has sign face flip instruction directly to its right";
					output.AddError(ss.str(),sem_pipe_r_sign_flip_juxed,i,3);
					
					ss.str("");
					ss.clear();
					foundError = true;
				}
			}
		}
	}//End for loop

	//Now check for adjacent pipebars
	for (int i = 0; i < inStr.input.size(); i++)
	{
		if(inStr.input[i] == '|' && (i + 1 < inStr.input.size()))
		{
			if(inStr.input[i + 1] == '|')
			{
				followsPipeJuxRules = false;

				ss << "Charecter " << i + 1 << ": Pipebar has pipebar to its right";
				output.AddError(ss.str(),sem_pipe_double_juxed,i,1);
				ss.str("");
				ss.clear();
				foundError = true;
			}
		}
	}

	//Now we check that if there is a pipebar that on either side is a glyph of the same color
	//Start it in a place where we could ~reasonably have a glyph, the shortest independant
	int fSize = output.out_sign.front.size();
	for (int i = 1; i < fSize; i++)
	{
		if(output.out_sign.front[i].glyph_color == 'P' && (i + 1 < fSize))
		{
			//A|B
			parser_glyph_info A = output.out_sign.front[i-1];
			parser_glyph_info B = output.out_sign.front[i+1];

			//If A or B is an independant glyph
			if(A.glyph_color == 'I')
			{
				followsPipeJuxRules = false;
				ss << "Glyph " << i-1 << " " << A.glyph_name << " cannot be used with a pipebar";

				output.AddError(ss.str(),sem_glyph_color_mismatch,i-1,A.glyph_name.size());
				ss.str("");
				ss.clear();
				foundError = true;
			}
			if(B.glyph_color == 'I')
			{
				followsPipeJuxRules = false;
				ss << "Glyph " << i+1 << " " << B.glyph_name << " cannot be used with a pipebar";
				output.AddError(ss.str(),sem_glyph_color_mismatch,i+1,B.glyph_name.size());
				ss.str("");
				ss.clear();
				foundError = true;
			}
			//If A is not equal to B and (A is not 'P' and B is not 'P')
			if(A.glyph_color != B.glyph_color)
			{
				followsPipeJuxRules = false;
				ss << "Charcter " << i + 1 << ": Pipebar is not surrounded with glyphs of matching types: /" 
																					<< A.glyph_color << A.glyph_name 
																					<< ", /" << B.glyph_color << B.glyph_name;
				output.AddError(ss.str(),sem_pipe_color_mismatch,i,3);
				ss.str("");
				ss.clear();
				foundError = true;
			}
		}
	}

	//Now do the same for the back
	int bSize = output.out_sign.back.size();
	for (int i = 1; i < bSize; i++)
	{
		if(output.out_sign.back[i].glyph_color == 'P' && (i + 1 < bSize))
		{
			//A|B
			parser_glyph_info A = output.out_sign.back[i-1];
			parser_glyph_info B = output.out_sign.back[i+1];

			if(A.glyph_color != B.glyph_color)
			{
				followsPipeJuxRules = false;
				ss << "Charcter " << i + 1 << ": Pipebar is not surrounded with glyphs of matching types: /" 
																					<< A.glyph_color << A.glyph_name 
																					<< ", /" << B.glyph_color << B.glyph_name;
				output.AddError(ss.str(),sem_pipe_color_mismatch,i,3);
				ss.str("");
				ss.clear();
				foundError = true;
			}
		}
	}
	
	//Finally test if the sign begins or ends with a pipe bar
	if(inStr.input[0] == '|')
	{
		ss << "Sign cannot begin with a pipe bar";
		output.AddError(ss.str(),sem_pipe_begins_sign,0,1);
		ss.str("");
		ss.clear();
		foundError = true;
	}
	
	
	if(inStr.input[inStr.input.length()-1] == '|')
	{
		ss << "Sign cannot end with a pipe bar";
		output.AddError(ss.str(),sem_pipe_ends_sign,inStr.input.length()-1,1);
		ss.str("");
		ss.clear();
		foundError = true;
	}
	return foundError;
}

//Returns true if there was an error
bool WED_Sign_Parser::check_color(const string & inGlyph, int position, parser_out_info & output)
{
	//This test assumes that if curColor is a certain color then it is that certain color
	//for a reason, aka it is safe to make some assumptions about the state of the parser based
	//off that information

	//Go in as far as it can go
	switch(curColor)
	{
	case 'Y':
	case 'R':
		return false;//Y and R allow for all characters
	case 'L':
		{
			bool foundError = false;
			for (int i = 0; i < inGlyph.length(); i++)
			{
				//L can only support A-Z and 0-9, the bellow checks the relevant
				if(!((inGlyph[i] >= 65 && inGlyph[i] <= 90) ||
					(inGlyph[i] >= 48 && inGlyph[i] <= 57)))
				{
					stringstream ss;
					ss << "Character " << position + 1 << ": " << inGlyph[i] << " cannot belong to color type " << curColor;
					parser_error_info e = {ss.str(),sem_glyph_color_mismatch,position,1};
					output.errors.push_back(e);
					foundError = true;
				}
			}
			return foundError;
		}
	case 'B':
		{
			bool foundError = false;
			for (int i = 0; i < inGlyph.length(); i++)
			{
				//B can only support 0-9 (ASCII letters 48 through 57)
				if(!(inGlyph[i] >= 48 && inGlyph[i] <= 57))
				{
					stringstream ss;
					ss << "Character " << position + 1 << ": " << inGlyph[i] << " cannot belong to color type " << curColor;
					parser_error_info e = {ss.str(),sem_glyph_color_mismatch,position,1};
					output.errors.push_back(e);
					foundError = true;
				}
			}
			return foundError;
		}
	case 'I':
		return false;//If I was chosen it can support it self
	case 'P'://No test need
		return false;
	case 'X'://No color has been selected
		return false;
	default:
		break;
	}
	return false;
}

//Check a multi glyph
//Returns true if there was an error
bool WED_Sign_Parser::check_multi_glyph(const string & inGlyph, int position, parser_out_info & output)
{
	//Assume there is something wrong until otherwise noted
	bool semError = true;

	//Based on the letter, preform a bunch of string compares
	//if it is a perfect match for any of the real multiletter glyphs
	switch(inGlyph[0])
	{
	case '^':
		if((inGlyph == "^u") == true ||
			(inGlyph == "^d") == true ||
			(inGlyph == "^r") == true ||
			(inGlyph == "^l") == true||
			(inGlyph == "^lu") == true ||
			(inGlyph == "^ru") == true ||
			(inGlyph == "^ld") == true ||
			(inGlyph == "^rd") == true)
		{
			semError = false;
		}
		else if(inGlyph == "^")
		{
			semError = true;
		}
		break;
	case 'c':
		if( (inGlyph == "critical") == true||
			(inGlyph == "comma") == true)
		{
			semError = false;
		}
		break;
	case 'h':
		if((inGlyph == "hazard") == true)
		{
			semError = false;
		}
		break;
	case 'n':
		if((inGlyph == "no-entry") == true)
		{
			semError = false;
		}
		break;
	case 'r':
		if((inGlyph == "r1") == true||
			(inGlyph == "r2") == true||
			(inGlyph == "r3") == true)
		{
			semError = false;
		}
		break;
	case 's':
		if((inGlyph == "safety") == true)
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
		ss << "Character " << position - inGlyph.length() + 1 << "-" << position << ": " << inGlyph << " is not a real multiglyph";
		parser_error_info e = {ss.str(),sem_not_real_multiglyph,position, position - position - inGlyph.length()};
		output.errors.push_back(e);
	}
	return semError;
}

bool WED_Sign_Parser::IsIndependentGlyph(const string & inLetters)
{
	if( inLetters == "critical" ||
		inLetters == "no-entry" ||
		inLetters == "safety"   ||
		inLetters == "hazard"   )
	{
		return true;
	}
	return false;
}

//--parser_out_info modifying code-------------------------
//Attempts to add a collection of letters
void WED_Sign_Parser::append_parser_out_info(const string & inGlyph, int position, parser_out_info & output)
{
	//Before actually appending them see if they're
	//correct semantically
	/* 
	1.) Findout if we the glyph can be supported in the current color
	2.) Write the results to the correct finished sign
	*/

	//Save the real current color
	char realColor = curColor;
	
	//If the glyph buffer is one of the independants
	if(IsIndependentGlyph(inGlyph) == true)
	{
		//Make it the color 'I' for this session
		curColor = 'I';
	}
	
	if(inGlyph.length() == 1 && inGlyph[0] == '|')
	{
		curColor = 'P';
	}

	bool invalid_for_color = check_color(inGlyph,position,output);

	if(invalid_for_color == true)
	{
		curColor = 'X';
	}

	bool invalid_multi_glyph = true;
	if(inGlyph.length() > 1)
	{
		invalid_multi_glyph = check_multi_glyph(inGlyph,position,output);
	}
	
	if(on_front == true)
	{
		output.out_sign.front.push_back(parser_glyph_info(curColor, inGlyph));
	}
	else
	{
		output.out_sign.back.push_back(parser_glyph_info(curColor, inGlyph));
	}
	
	//Reset it back to the original color, regardless if it had to be set to I or not
	curColor = realColor;
}
//---------------------------------------------------------

//--Syntax Checking functions------------------------------
bool WED_Sign_Parser::IsSupportedChar(char inChar)
{
	if((inChar >= 65 && inChar <= 90) || //A-Z
			(inChar >= 48 && inChar <= 57)  || //0-9
			inChar == '.'||//These take care of specials and
			inChar == '*'||//parts of multiletter glyphs
			inChar == ','||
			inChar == '-'||
			inChar == '.'||
			inChar == '_'||
			inChar == '|'||//Pipe bar
			inChar == '/'||
			inChar == '@'||
			inChar == '^'||
			inChar == 'a'||
			inChar == 'c'||
			inChar == 'd'||
			inChar == 'e'||
			inChar == 'f'||
			inChar == 'h'||
			inChar == 'i'||
			inChar == 'l'||
			inChar == 'm'||
			inChar == 'n'||
			inChar == 'o'||
			inChar == 'r'||
			inChar == 's'||
			inChar == 't'||
			inChar == 'u'||
			inChar == 'y'||
			inChar == 'z'||
			inChar == '{'||
			inChar == '}'
			)
	{
		return true;
	}
	return false;
}

//Takes in the place where a '{' is as the start
//Returns true if there was an error
bool WED_Sign_Parser::ValidateCurly(const parser_in_info & inStr, int position, parser_out_info & output)
{		
	//What is currently considered good, a { a }
	//We start by saying that we are looking for a {
	char LFGoodMode = '{';//Used later for nesting nesting
	int rCurlyIndex = 0;//Index where the matching } is found, 0 means not found
	//1.) All { have a }
	//2.) No pair may be empty nest
	//3.) No pair may nest

	//--Find if and where the end of the pair is-----
	//Run until it breaks on 1.)Finding }
	//or reaching the end of the string
	int i = position;
	while(true)
	{
		//If you've found the other pair
		if(inStr.input[i] == '}')
		{
			rCurlyIndex = i;
			break;
		}
		if(i == inStr.input.length())
		{
			stringstream ss;
			ss << "Curly brace pair starting at " << position + 1 << " is missing its end brace";
			output.AddError(ss.str(),syn_curly_pair_missing,position,inStr.input.length()-i);
			return true;
		}
		i++;
	}
	//---------------------------------------------
	
	//Reset the nPos
	i = position;

	//--Next, find if it is actually empty---------
	if(inStr.input[i+1] == '}')
	{
		stringstream ss;
							//This i+1 is for the user, not refering to the next char
		ss << "Empty curly braces detected starting at character " << i+1;
		output.AddError(ss.str(),syn_curly_pair_empty,i,2);
		return true;
	}
	//---------------------------------------------
	
	//--Finally see if there is nesting------------

	while(i < inStr.input.length())
	{
		/* 1.)Decide whats good or bad
		*		The first curly brace should be open
		*		The second curly brace should be close
		*		It should change never
		* 2.)Test to see if it good or bad
		*/

		//If we are at a { or }
		if(inStr.input[i] == '{' || inStr.input[i] == '}')
		{
			//Is it the good mode?
			if(inStr.input[i] == LFGoodMode)
			{
				//If so toggle what you are looking for
				LFGoodMode = (inStr.input[i] == '{') ? '}' : '{';					
			}
			else
			{
				stringstream ss;
				ss << "Character " << i + 1 << ": Brace " << inStr.input[i] << " is invalid in this situation, causes nesting";
				output.AddError(ss.str(),syn_curly_pair_nested,i,1);
				return true;
			}
		}
		i++;
	}
	//---------------------------------------------
	i = position;
	return false;
}

//Return if there was an error or not
bool WED_Sign_Parser::ValidateBasics(const parser_in_info & inStr, parser_out_info & output)
{
	bool error = false;

	//--Too long---------------------------------------------
	//TODO - Find out if there is a max length

	//-------------------------------------------------------

	int i = 0;
	//---White Space-----------------------------------------
	while(i < inStr.input.length())//Loop for the whole string
	{
		char c = inStr.input[i];
		//If the current character is white space
		//(isspace blows up on a non ascii character, this is our implenetation)
		if( c == ' '  ||
			c == '\t' ||
			c == '\n' ||
			c == '\v' ||
			c == '\f' ||
			c == '\r')
		{
			stringstream ss;
			ss << "Character " << i + 1 << ": Found whitespace";
			output.AddError(ss.str(),syn_whitespace_found,i,1);
			return true;
		}
		//Increase the pointer and counter
		i++;
	}
	//-------------------------------------------------------

	//Reset variable
	i = 0;

	//--ASCII or supported char------------------------------------------------
	while(i < inStr.input.length())
	{	
		//Check if it is a non supported char (aka NOT A-Z,0-9,.,* etc
		if(!IsSupportedChar(inStr.input[i]))
		{
			stringstream ss;
			ss << "Character " << i + 1 << ": " << inStr.input[i] << " is not supported";
			output.AddError(ss.str(),syn_nonsupported_char,i,1);
			return true;
		}
		i++;
	}
	//-------------------------------------------------------
	i = 0;

	//Validate all curly brace rules
	while(i < inStr.input.length())
	{
		if(inStr.input[i] == '{')
		{
			error = ValidateCurly(inStr,i,output);
			if(error)
			{
				return error;
			}
		}
		i++;
	}
	i = 0;
	return error;
}
//---------------------------------------------------------

//--FSM functions------------------------------------------
const string & WED_Sign_Parser::EnumToString(FSM in)
{
	switch(in)
	{
	case I_COMMA:
		return "I_COMMA";
	case I_INCUR:
		return "I_INCUR";
	case I_ACCUM_GLPHYS:	
		return "I_ACCUM_GLPHYS";
	case I_ANY_CONTROL:
		return "I_ANYCONTROL";
	case I_WAITING_SEPERATOR:	
		return "I_WAITING_SEPERATOR";
	case O_ACCUM_GLYPHS:
		return "O_ACCUM_GLYPHS";
	case O_END:	
		return "O_END";
	}
	return "NOT REAL STATE";
}

//Take in the current (and soon to be past) state  and the current letter being processed
//The heart of all this
//Takes in the current state of the FSM, the current character being processes
//The position, Outstr, and msgBuf are all part of reporting errors and are not integral to the FSM
WED_Sign_Parser::FSM WED_Sign_Parser::LookUpTable(FSM curState, char curChar, int position, parser_out_info & output)
{
	stringstream ss;
	//If you have reached a \0 FOR ANY REASON exit now
	if(curChar == '\0')
	{
		return O_END;
	}
 	switch(curState)
	{
	case I_COMMA:
		switch(curChar)
		{
		//You will always enter IDLE from a place where a seperator is
		//not allowed
		case '}':
		case ','://since comma's always go into idle you have hit ,,
			{
			ss << "Character " << position + 1 << ": " << curChar << " is not allowed there, expected @ or a glyph";
			output.AddError(ss.str(),syn_expected_non_seperator_after_comma,position,1);
			}
			return LOOKUP_ERR;
		case '@':
			return I_ANY_CONTROL;
		default:
			//if it was able to accumulate the the glyph
			glyphBuf += curChar;
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_INCUR:
		switch(curChar)
		{
		case ',':
			ss << "Charecter " << position + 1 << ": " << curChar << " is not allowed here, expected glyphs or an instruction";
			output.AddError(ss.str(),syn_expected_non_seperator_after_comma,position,1);
			return LOOKUP_ERR;
		case '@':
			return I_ANY_CONTROL;
		default:
			//otherwise accumulate the glyphs
			glyphBuf += curChar;
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ACCUM_GLPHYS:	
		switch(curChar)
		{
		//Cases to make it stop accumulating
		case '}':
			append_parser_out_info(glyphBuf,position,output);
			glyphBuf.clear();
			return O_ACCUM_GLYPHS;
		case ',':
			append_parser_out_info(glyphBuf,position,output);
			glyphBuf.clear();
			return I_COMMA;
		default:
			//otherwise accumulate the glyphs
			glyphBuf += curChar;
			return I_ACCUM_GLPHYS;
		}
		break;
	case I_ANY_CONTROL:	
		switch(curChar)
		{
		case 'Y':
		case 'L':
		case 'R':
		case 'B':
			//Do action, change color
			curColor = curChar;
			return I_WAITING_SEPERATOR;
			
		case '@':
			if(on_front == true)
			{
				on_front = false;
			}
			else
			{
				//Sementic error found extra @@
				ss << "Charecter " << position + 1 << ": Cannot switch sign sides more than once";
				output.AddError(ss.str(),sem_mutiple_side_switches,position,1);
			}
			return I_WAITING_SEPERATOR;
		default:
			ss << "Character " << position + 1 << ": {@" << curChar << " is not a real instruction. Use {@Y,{@R,{@L, or {@B";
			output.AddError(ss.str(),sem_not_real_instruction,position,1);
		}
		break;
	case I_WAITING_SEPERATOR:	
		switch(curChar)
		{
		case ',':
			return I_COMMA;
		case '}':
			return O_ACCUM_GLYPHS;
		default:
			ss << "Character " << position + 1 << ": Was expecting , or }, got " << curChar;//No way you should end up with something like @YX or {@@X
			output.AddError(ss.str(),syn_expected_seperator,position,1);
			return LOOKUP_ERR;
		}
		break;
	case O_ACCUM_GLYPHS:	
		switch(curChar)
		{
		case '{':
			return I_INCUR;
			break;
		case '@':
		case '^':
			ss << "Character " << position + 1 << ": " << curChar << " is not allowed outside curly braces";
			output.AddError(ss.str(),syn_found_at_symbol_outside_curly,position,1);
			return LOOKUP_ERR;
		default:
			//If the current letter is NOT lower-case(part of something like critical or hazard)
			if(!(curChar>=97 && curChar <= 122))
			{
				string c;
				c += curChar;
				append_parser_out_info(c,position,output);
				return O_ACCUM_GLYPHS;
			}
			else
			{
				ss << "Character " << position + 1 << ": " << curChar << " is not allowed outside curly braces";//This may be impossible to get to
				output.AddError(ss.str(),syn_found_lowercase_outside_curly,position,1);
				return LOOKUP_ERR;
			}
		}
		break;
	case O_END:	
		switch(curChar)
		{
			
		}
		break;
	}
	return LOOKUP_ERR;
}
//---------------------------------------------------------

void WED_Sign_Parser::MainLoop(const parser_in_info & input, parser_out_info & output)
{
	//Validate if there is any whitesapce or non printable ASCII characters (33-126)
	if(WED_Sign_Parser::ValidateBasics(input,output) == true)
	{
		return;
	}
	
	FSM FSM_MODE = O_ACCUM_GLYPHS;
	int i = 0;
	while(FSM_MODE != O_END)
	{
		//Look up the transition
		FSM transition = WED_Sign_Parser::LookUpTable(FSM_MODE,input.input[i], i, output);
		if(transition != LOOKUP_ERR)
		{
			FSM_MODE = transition;
			i++;
		}
		else
		{
			//stringstream ss
			//ss << "Fatal lookup error! State: %s, Char: %c, Location: %d",WED_Sign_Parser::EnumToString(FSM_MODE),*(inStr.nPos),(inStr.nPos-inStr.oPos));
			//msgBuf.push_back(ss.str());
			break;
		}
	}

	bool foundError = preform_final_semantic_checks(input,output);
}
//---------------------------------------------------------

void ParserTaxiSign(const parser_in_info & input, parser_out_info & output)
{
	WED_Sign_Parser parser;
	parser.MainLoop(input,output);
}