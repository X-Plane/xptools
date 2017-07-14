#include "WED_Sign_Parser.h"
#include <assert.h>

struct glyph_info_t {
	parser_glyph_t		glyph;
	const char *		inside_name;
	const char *		outside_name;	// NULL if not legal outside {}
	int					yellow_ok;
	int					red_ok;
	int					location_ok;
	int					black_ok;
	int					independent_ok;
};

static const glyph_info_t	k_glyph_metadata[] = {
/*	glyph			inside name	outside name	Y	R	L	B	I	*/
	glyph_A,		"A",		"A",			1,	1,	1,	0,	0,
	glyph_B,		"B",		"B",			1,	1,	1,	0,	0,
	glyph_C,		"C",		"C",			1,	1,	1,	0,	0,
	glyph_D,		"D",		"D",			1,	1,	1,	0,	0,
	glyph_E,		"E",		"E",			1,	1,	1,	0,	0,
	glyph_F,		"F",		"F",			1,	1,	1,	0,	0,
	glyph_G,		"G",		"G",			1,	1,	1,	0,	0,
	glyph_H,		"H",		"H",			1,	1,	1,	0,	0,
	glyph_I,		"I",		"I",			1,	1,	1,	0,	0,
	glyph_J,		"J",		"J",			1,	1,	1,	0,	0,
	glyph_K,		"K",		"K",			1,	1,	1,	0,	0,
	glyph_L,		"L",		"L",			1,	1,	1,	0,	0,
	glyph_M,		"M",		"M",			1,	1,	1,	0,	0,
	glyph_N,		"N",		"N",			1,	1,	1,	0,	0,
	glyph_O,		"O",		"O",			1,	1,	1,	0,	0,
	glyph_P,		"P",		"P",			1,	1,	1,	0,	0,
	glyph_Q,		"Q",		"Q",			1,	1,	1,	0,	0,
	glyph_R,		"R",		"R",			1,	1,	1,	0,	0,
	glyph_S,		"S",		"S",			1,	1,	1,	0,	0,
	glyph_T,		"T",		"T",			1,	1,	1,	0,	0,
	glyph_U,		"U",		"U",			1,	1,	1,	0,	0,
	glyph_V,		"V",		"V",			1,	1,	1,	0,	0,
	glyph_W,		"W",		"W",			1,	1,	1,	0,	0,
	glyph_X,		"X",		"X",			1,	1,	1,	0,	0,
	glyph_Y,		"Y",		"Y",			1,	1,	1,	0,	0,
	glyph_Z,		"Z",		"Z",			1,	1,	1,	0,	0,

	glyph_0,		"0",		"0",			1,	1,	1,	1,	0,
	glyph_1,		"1",		"1",			1,	1,	1,	1,	0,
	glyph_2,		"2",		"2",			1,	1,	1,	1,	0,
	glyph_3,		"3",		"3",			1,	1,	1,	1,	0,
	glyph_4,		"4",		"4",			1,	1,	1,	1,	0,
	glyph_5,		"5",		"5",			1,	1,	1,	1,	0,
	glyph_6,		"6",		"6",			1,	1,	1,	1,	0,
	glyph_7,		"7",		"7",			1,	1,	1,	1,	0,
	glyph_8,		"8",		"8",			1,	1,	1,	1,	0,
	glyph_9,		"9",		"9",			1,	1,	1,	1,	0,

	glyph_dash,		"-",		"-",			1,	1,	0,	0,	0,
	glyph_dot,		"*",		"*",			1,	1,	0,	0,	0,
	glyph_period,	".",		".",			1,	1,	0,	0,	0,
	glyph_slash,	"/",		"/",			1,	1,	0,	0,	0,
	glyph_space,	"_",		"_",			1,	1,	0,	0,	0,
	glyph_separator,"|",		"|",			1,	1,	1,	0,	0,
	glyph_comma,	"comma",	",",			1,	1,	0,	0,	0,
	
	glyph_up,		"^u",		NULL,			1,	1,	0,	0,	0,
	glyph_down,		"^d",		NULL,			1,	1,	0,	0,	0,
	glyph_left,		"^l",		NULL,			1,	1,	0,	0,	0,
	glyph_right,	"^r",		NULL,			1,	1,	0,	0,	0,
	glyph_leftup,	"^lu",		NULL,			1,	1,	0,	0,	0,
	glyph_rightup,	"^ru",		NULL,			1,	1,	0,	0,	0,
	glyph_leftdown,	"^ld",		NULL,			1,	1,	0,	0,	0,
	glyph_rightdown,"^rd",		NULL,			1,	1,	0,	0,	0,
	
	glyph_critical,	"critical",	NULL,			0,	0,	0,	0,	1,
	glyph_hazard,	"hazard",	NULL,			0,	0,	0,	0,	1,
	glyph_no_entry,	"no-entry",	NULL,			0,	0,	0,	0,	1,
	glyph_safety,	"safety",	NULL,			0,	0,	0,	0,	1,
	glyph_r1,		"r1",		NULL,			1,	1,	0,	0,	0,
	glyph_r2,		"r2",		NULL,			1,	1,	0,	0,	0,
	glyph_r3,		"r3",		NULL,			1,	1,	0,	0,	0
};

static const char * k_err_msgs[parser_error_count] = {
	"Invalid @ instruction",
	"Not a real glyph",
	"Not a single-character real glyph",

	"@ outside of {}",
	"Expected } or ,",
	"Empty glyph",

	"Missing { at end of sign",
	"Nested {",
	"} is not after }",

	"Sign side begins with a separator",
	"Color mismatch around separator",
	"Double separator",
	"Sign ends with separator",

	"Illegal color for glyph",
	"Too many side switches"
};

static const int k_glyph_info_count = sizeof(k_glyph_metadata) / sizeof(k_glyph_metadata[0]);

static const glyph_info_t * get_glyph_info(parser_glyph_t glyph)
{
	for(int i = 0; i < k_glyph_info_count; ++i)
	if(k_glyph_metadata[i].glyph == glyph)
		return k_glyph_metadata+i;
	return NULL;
}

string	parser_name_for_glyph(parser_glyph_t glyph)
{
	const glyph_info_t * i = get_glyph_info(glyph);
	return i ? i->inside_name : "";
}

string	short_name_for_glyph(parser_glyph_t glyph)
{
	const glyph_info_t * i = get_glyph_info(glyph);
	if(!i)
		return string();
	
	if(i->outside_name == NULL)
		return string();
	
	return i->outside_name;
}

bool	parser_is_color_legal(parser_glyph_t glyph, parser_color_t c)
{
	const glyph_info_t * i = get_glyph_info(glyph);
	if(!i) return false;
	switch(c) {
	case sign_color_yellow: return i->yellow_ok;
	case sign_color_red:	return i->red_ok;
	case sign_color_location: return i->location_ok;
	case sign_color_black:	return i->black_ok;
	case sign_color_independent: return i->independent_ok;
	default: return false;
	}
}

parser_glyph_t	glyph_for_short_name(const string& s)
{
	for(int i = 0; i < k_glyph_info_count; ++i)
	if(k_glyph_metadata[i].inside_name && s == k_glyph_metadata[i].inside_name)
		return k_glyph_metadata[i].glyph;
	return glyph_Invalid;
}




//--WED_Sign_Parser class decleration--------------------------
class WED_Sign_Parser {
public:
	WED_Sign_Parser(const parser_in_info & input, parser_out_info & output);
	~WED_Sign_Parser(void);
	
	void MainLoop();

private:

	// An important note on glyph buffer handling:
	// If the parser is in state I_ACCUM_GLYPHS, all output is dumped into the glyph buffer until we're ready to flush it.
	// In all other states, the glyph buffer is empty.
	// Therefore at any given time when we need to accumulate an error, either:
	//
	// (a) we are working on the glyph buffer, which is non-emtpy, and the current position is at the NEXT character after the glyph
	// buf's characters.  We thus know where in the source string the glyph buffer came from and we can attribute the error to the
	// glyph buffer or
	//
	// (b) we are parsing a single character at mPosition.
	//
	// Code that does both (e.g. if we hit a { inside a {} expression with the glyph buf non-emtpy in I_ACCUM_GLYPHS) makes sure to
	// (1) first flush the glyph buf, attributing any errors to that buffer, e.g. if the glyph name is silly or the color is wrong
	// and (2) then clears the glyph buf and copes with additional errors (e.g. nested braces), attributing that to the separatolr.


	enum FSM {
		//The inside curly braces portion, starts with I_
		I_COMMA,		//	We just hit a comma inside braces and are now expecting some kind of multi-glyph or command token
		I_INCUR,		//	We are inside curly braces - we can accept a } or a token
		I_ACCUM_GLYPHS,	//	We have at least one char and are possibly collecting more for multi-char glyphs in {}
		I_ANY_CONTROL,	//	We parsed  @ and are waiting for the control letter
		I_WAITING_SEPERATOR,//For when it is waiting for a , or } after finishing a known control, e.g. @@
		//The outside curly braces portion, starts with O_
		O_ACCUM_GLYPHS,	//	We are consuming single-char glyphs - we don't have any yet.
						//	(Since they are single char, each scanned char is immediately turned into a glyph and we
						//	fall back into this state.  This is different from inside {} where I_ACCUM_GLYPHS is only
						//	when we are mid-glyph)
	};

	// Appends an error code for the current parse.  If the glyph buf is not empty, we attribute the buf to
	// the error; otherwise we attribute the single char at the current position.
	void			append_error(parser_error_t code);
	
	// These check a glyph, and either accumulate errors _or_ return the valid glyph.
	parser_glyph_t	check_multi_glyph(const string & inGlyph);
	parser_glyph_t	check_single_glyph(char inGlyph);

	// Given a valid glyph in the current glyph buf _or_ position (if glyph buf empty),
	// accumulate it, and add any semantic errors that we hit along the way
	void			append_parser_out_info(parser_glyph_t glyph);

	// Given the current state, consume the current character and return the new state, taking all actions needed.
	FSM				LookUpTable(FSM curState);
	//---------------------------------------------------------

	//--FSM data members---------------------------------------
	parser_color_t			mCurColor;		// Current color as we parse
	bool					mOnFront;		// True if in sign front, false if in sign back
	string					mGlyphBuf;		// Accumulated chars for multi-char glyph
	int						mPosition;		// Index into input string
	
	const parser_in_info&	mInput;
	parser_out_info&		mOutput;
};
//------------------------------------------------------------------------
WED_Sign_Parser::WED_Sign_Parser(const parser_in_info & input, parser_out_info & output) :
	mCurColor(sign_color_invalid),
	mOnFront(true),
	mPosition(0),
	mInput(input),
	mOutput(output)
{
}

WED_Sign_Parser::~WED_Sign_Parser()
{
}

void WED_Sign_Parser::append_error(parser_error_t code)
{
	parser_error_info e;
	
	e.err_code = code;
	if(mGlyphBuf.empty())
	{
		e.position = mPosition;
		e.length = 1;
	}
	else
	{
		// When the glyph buf is in-tact, it's the post-glyph-buf terminating character that provkes us.
		// so if we have:
		//  01234567
		//  {@Y,FOO,
		// position is 7, the cur char is , and we get called with FOO in our buffer and detemrine that 4-7
		// is the bad glyph.
		// If the code wants us to push an err on 7, it'll clear out the glyph buf first.
		e.position = mPosition - mGlyphBuf.size();
		e.length = mGlyphBuf.size();
	}

	stringstream str;
	str << "Chars " << e.position << " to " << e.position + e.length - 1 << ": " << k_err_msgs[code] << " " << mInput.input.substr(e.position,e.length);
	e.msg = str.str();

	mOutput.errors.push_back(e);
}


//Check a multi glyph
//Returns true if there was an error
parser_glyph_t WED_Sign_Parser::check_multi_glyph(const string & inGlyph)
{
	for(int i = 0; i < k_glyph_info_count; ++i)
	if(k_glyph_metadata[i].inside_name)
	if(inGlyph == k_glyph_metadata[i].inside_name)
	{
		return k_glyph_metadata[i].glyph;
	}
	
	append_error(syn_not_real_multiglyph);
	return glyph_Invalid;
}

parser_glyph_t WED_Sign_Parser::check_single_glyph(const char inGlyph)
{
	for(int i = 0; i < k_glyph_info_count; ++i)
	if(k_glyph_metadata[i].outside_name)
	if(k_glyph_metadata[i].outside_name[0] == inGlyph)
	{
		return k_glyph_metadata[i].glyph;
	}
	
	append_error(syn_not_real_singleglyph);
	return glyph_Invalid;
}

//Attempts to add a collection of letters
void WED_Sign_Parser::append_parser_out_info(parser_glyph_t glyph)
{
	//Before actually appending them see if they're
	parser_color_t glyphColor = mCurColor;
	
	if(parser_is_color_legal(glyph, sign_color_independent))
		glyphColor = sign_color_independent;
	
	if(!parser_is_color_legal(glyph, glyphColor))
	{
		append_error(sem_glyph_color_mismatch);
		glyphColor = sign_color_invalid;
	}

	vector<parser_glyph_info>& side(mOnFront ? mOutput.out_sign.front : mOutput.out_sign.back);

	if(side.empty())
	{
		// For the first char in a side, do the start-with-pipe check.
		if(glyph == glyph_separator)
		{
			return; // silently drop unwanted separator at very front of sign
// 			append_error(sem_pipe_begins_sign);
		}
	}
	else
	{
		// For all others do adjaceny checks.
		if(side.back().glyph_name == glyph_separator && glyph == glyph_separator)
		{
			return; // silently drop unwanted separator next to existing separator
//			append_error(sem_pipe_double_juxed);
		}
		
		if(side.back().glyph_name == glyph_separator || glyph == glyph_separator)
		if(side.back().glyph_color != glyphColor)
		{
			return; // silently drop unwanted separator next to a color change in the sign
			append_error(sem_pipe_color_mismatch);
		}
	}
	
	side.push_back(parser_glyph_info(glyphColor, glyph));

}

//Take in the current (and soon to be past) state  and the current letter being processed
//The heart of all this
//Takes in the current state of the FSM, the current character being processes
//The position, Outstr, and msgBuf are all part of reporting errors and are not integral to the FSM
WED_Sign_Parser::FSM WED_Sign_Parser::LookUpTable(FSM curState)
{
	char curChar = mInput.input[mPosition];
	stringstream ss;
	parser_glyph_t glyph;
	
	// Important: we should not have gotten out of I_ACCUM_GLYPHS without flushing the glyph buf,
	// and we should not have added to the glyph buf without going into I_ACCUM_GLYPHS.
	assert(mGlyphBuf.empty() || curState == I_ACCUM_GLYPHS);
	
 	switch(curState) {
	case I_COMMA:
		switch(curChar) {
		case '{':
			append_error(syn_curly_pair_nested);
			return I_COMMA;
		case '}':
			append_error(syn_empty_multiglyph);
			return O_ACCUM_GLYPHS;
		case ',':
			append_error(syn_empty_multiglyph);
			return I_COMMA;
		case '@':
			return I_ANY_CONTROL;
		default:
			//if it was able to accumulate the the glyph
			mGlyphBuf += curChar;
			return I_ACCUM_GLYPHS;
		}
		break;
	case I_INCUR:
		switch(curChar) {
		case '{':
			append_error(syn_curly_pair_nested);
			return I_INCUR;
		case '}':
			return O_ACCUM_GLYPHS;
		case ',':
			append_error(syn_empty_multiglyph);
			return I_COMMA;
		case '@':
			return I_ANY_CONTROL;
		default:
			mGlyphBuf += curChar;
			return I_ACCUM_GLYPHS;
		}
		break;
	case I_ACCUM_GLYPHS:
		switch(curChar)	{
		case '{':
			glyph = check_multi_glyph(mGlyphBuf);
			if(glyph != glyph_Invalid)
				append_parser_out_info(glyph);
			mGlyphBuf.clear();

			append_error(syn_curly_pair_nested);

			return I_ACCUM_GLYPHS;
		case '}':
			glyph = check_multi_glyph(mGlyphBuf);
			if(glyph != glyph_Invalid)
				append_parser_out_info(glyph);
			mGlyphBuf.clear();
			return O_ACCUM_GLYPHS;
		case ',':
			glyph = check_multi_glyph(mGlyphBuf);
			if(glyph != glyph_Invalid)
				append_parser_out_info(glyph);
			mGlyphBuf.clear();
			return I_COMMA;
		case '@':
		default:
			mGlyphBuf += curChar;
			return I_ACCUM_GLYPHS;
		}
		break;
	case I_ANY_CONTROL:	
		switch(curChar)	{
		case '{':
			append_error(syn_curly_pair_nested);
			return I_ANY_CONTROL;
		case '}':
			append_error(syn_not_real_instruction);
			return O_ACCUM_GLYPHS;
		case ',':
			append_error(syn_not_real_instruction);
			return I_COMMA;
		case '@':
			if(mOnFront == true)
			{
				if(!mOutput.out_sign.front.empty())
				if(mOutput.out_sign.front.back().glyph_name == glyph_separator)
					append_error(sem_pipe_ends_sign);
				mOnFront = false;
			}
			else
				append_error(sem_mutiple_side_switches);
			return I_WAITING_SEPERATOR;
		case 'Y':
			mCurColor = sign_color_yellow;
			return I_WAITING_SEPERATOR;			
		case 'L':
			mCurColor = sign_color_location;
			return I_WAITING_SEPERATOR;			
		case 'R':
			mCurColor = sign_color_red;
			return I_WAITING_SEPERATOR;			
		case 'B':
			mCurColor = sign_color_black;
			return I_WAITING_SEPERATOR;			
		default:
			append_error(syn_not_real_instruction);
			return I_WAITING_SEPERATOR;
		}
		break;
	case I_WAITING_SEPERATOR:	
		switch(curChar) {
		case '{':
			append_error(syn_curly_pair_nested);
			return I_INCUR;
		case '}':
			return O_ACCUM_GLYPHS;
		case ',':
			return I_COMMA;
		case '@':
			append_error(syn_expected_seperator);
			return I_WAITING_SEPERATOR;
		default:
			append_error(syn_expected_seperator);
			return I_WAITING_SEPERATOR;
		}
		break;
	case O_ACCUM_GLYPHS:	
		switch(curChar) {
		case '{':
			return I_INCUR;
		case '}':
			append_error(syn_curly_unbalanced);
			return O_ACCUM_GLYPHS;
		case '@':
			append_error(syn_found_at_symbol_outside_curly);
			return O_ACCUM_GLYPHS;
		case ',':
		default:
			glyph = check_single_glyph(curChar);
			if(glyph != glyph_Invalid)
			{
				append_parser_out_info(glyph);
			}
			return O_ACCUM_GLYPHS;
		}
		break;
	}
}
//---------------------------------------------------------

void WED_Sign_Parser::MainLoop()
{
//	//Validate if there is any whitesapce or non printable ASCII characters (33-126)
//	if(WED_Sign_Parser::ValidateBasics(input,output) == true)
//	{
//		return;
//	}
	
	FSM FSM_MODE = O_ACCUM_GLYPHS;
	mPosition = 0;
	while(mPosition < mInput.input.size())
	{
		//Look up the transition
		FSM transition = WED_Sign_Parser::LookUpTable(FSM_MODE);
		FSM_MODE = transition;
		++mPosition;
	}

	// When we hit end of text, if we are accumulating glyphs,
	// use the EOF as a hint to flush the glyph buffer if needed.
	// That way if the user writes {hazard
	// we flush hazard as a valid sign, and clear the glyph buf.
	
	if(FSM_MODE == I_ACCUM_GLYPHS)
	{
		assert(!mGlyphBuf.empty());
		parser_glyph_t glyph = check_multi_glyph(mGlyphBuf);
		if(glyph != glyph_Invalid)
			append_parser_out_info(glyph);
		mGlyphBuf.clear();
	}
	
	// Now with the glyph buf if we haven't sealed off a { sequence,
	// the glyph buf is empty and we can squawk that our end position is fubar.

	if(FSM_MODE != O_ACCUM_GLYPHS)
	{
		append_error(syn_curly_pair_missing);
	}
	
	if(mOnFront)
	{
		if( !mOutput.out_sign.front.empty() && mOutput.out_sign.front.back().glyph_name == glyph_separator )
		{
			mOutput.out_sign.front.pop_back(); // silently delete undesired separator
//			append_error(sem_pipe_ends_sign);
		}
	}
	else
	{
		if( !mOutput.out_sign.back.empty() &&  mOutput.out_sign.back.back().glyph_name == glyph_separator )
		{
			mOutput.out_sign.back.pop_back(); // silently delete undesired separator
//			append_error(sem_pipe_ends_sign);
		}
	}

//	bool foundError = preform_final_semantic_checks();
}
//---------------------------------------------------------

void ParserTaxiSign(const parser_in_info & input, parser_out_info & output)
{
	WED_Sign_Parser parser(input,output);
	parser.MainLoop();
}
