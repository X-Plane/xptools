//
//  WED_SignEditor.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/31/15.
//
//

#include "WED_Sign_Editor.h"
#include "AssertUtils.h"
#include "WED_Sign_Parser.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "GUI_Fonts.h"
#include <iterator>

/*
	todo:
	
		BUG: 
			you can have consecutive separators
				nothing stops double separator
				you can also delete the stuff between them
		FACTOR:
			sign metric hard coding:
				inset between measure and plotter UV map calc (both hard coded to inset = 4
				scale for mini palette of half size
		
		TEST:
			arrow keys?
		
		FEATURES:
			
			how to enter arrows?
			
		6. color preview on map with mouse-over
*/

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


struct glyph_tex_info { 
	parser_glyph_t	glyph;
	parser_color_t	color;
	int				x;
	int				y;
	int				width;
};

static const glyph_tex_info k_tex_syms[] = {
	glyph_0,	sign_color_black,	0,	0,	1,
	glyph_1,	sign_color_black,	1,	0,	1,
	glyph_2,	sign_color_black,	2,	0,	1,
	glyph_3,	sign_color_black,	3,	0,	1,
	glyph_4,	sign_color_black,	4,	0,	1,
	glyph_5,	sign_color_black,	5,	0,	1,
	glyph_6,	sign_color_black,	6,	0,	1,
	glyph_7,	sign_color_black,	7,	0,	1,
	glyph_8,	sign_color_black,	8,	0,	1,
	glyph_9,	sign_color_black,	9,	0,	1,
	glyph_hazard,sign_color_independent,10,0,2,
	glyph_safety,sign_color_independent,12,0,2,
	glyph_critical,sign_color_independent,14,0,2,
	
	glyph_0,	sign_color_location,	0,	1,	1,
	glyph_1,	sign_color_location,	1,	1,	1,
	glyph_2,	sign_color_location,	2,	1,	1,
	glyph_3,	sign_color_location,	3,	1,	1,
	glyph_4,	sign_color_location,	4,	1,	1,
	glyph_5,	sign_color_location,	5,	1,	1,
	glyph_6,	sign_color_location,	6,	1,	1,
	glyph_7,	sign_color_location,	7,	1,	1,
	glyph_8,	sign_color_location,	8,	1,	1,
	glyph_9,	sign_color_location,	9,	1,	1,
	glyph_A,	sign_color_location,	10,	1,	1,
	glyph_B,	sign_color_location,	11,	1,	1,
	glyph_C,	sign_color_location,	12,	1,	1,
	glyph_D,	sign_color_location,	13,	1,	1,
	glyph_E,	sign_color_location,	14,	1,	1,
	glyph_F,	sign_color_location,	15,	1,	1,

	glyph_G,	sign_color_location,	0,	2,	1,
	glyph_H,	sign_color_location,	1,	2,	1,
	glyph_I,	sign_color_location,	2,	2,	1,
	glyph_J,	sign_color_location,	3,	2,	1,
	glyph_K,	sign_color_location,	4,	2,	1,
	glyph_L,	sign_color_location,	5,	2,	1,
	glyph_M,	sign_color_location,	6,	2,	1,
	glyph_N,	sign_color_location,	7,	2,	1,
	glyph_O,	sign_color_location,	8,	2,	1,
	glyph_P,	sign_color_location,	9,	2,	1,
	glyph_Q,	sign_color_location,	10,	2,	1,
	glyph_R,	sign_color_location,	11,	2,	1,
	glyph_S,	sign_color_location,	12,	2,	1,
	glyph_T,	sign_color_location,	13,	2,	1,
	glyph_U,	sign_color_location,	14,	2,	1,
	glyph_V,	sign_color_location,	15,	2,	1,

	glyph_W,	sign_color_location,	0,	3,	1,
	glyph_X,	sign_color_location,	1,	3,	1,
	glyph_Y,	sign_color_location,	2,	3,	1,
	glyph_Z,	sign_color_location,	3,	3,	1,
	glyph_0,	sign_color_yellow,		4,	3,	1,
	glyph_1,	sign_color_yellow,		5,	3,	1,
	glyph_2,	sign_color_yellow,		6,	3,	1,
	glyph_3,	sign_color_yellow,		7,	3,	1,
	glyph_4,	sign_color_yellow,		8,	3,	1,
	glyph_5,	sign_color_yellow,		9,	3,	1,
	glyph_6,	sign_color_yellow,		10,	3,	1,
	glyph_7,	sign_color_yellow,		11,	3,	1,
	glyph_8,	sign_color_yellow,		12,	3,	1,
	glyph_9,	sign_color_yellow,		13,	3,	1,
	glyph_A,	sign_color_yellow,		14,	3,	1,
	glyph_B,	sign_color_yellow,		15,	3,	1,

	glyph_C,	sign_color_yellow,		0,	4,	1,
	glyph_D,	sign_color_yellow,		1,	4,	1,
	glyph_E,	sign_color_yellow,		2,	4,	1,
	glyph_F,	sign_color_yellow,		3,	4,	1,
	glyph_G,	sign_color_yellow,		4,	4,	1,
	glyph_H,	sign_color_yellow,		5,	4,	1,
	glyph_I,	sign_color_yellow,		6,	4,	1,
	glyph_J,	sign_color_yellow,		7,	4,	1,
	glyph_K,	sign_color_yellow,		8,	4,	1,
	glyph_L,	sign_color_yellow,		9,	4,	1,
	glyph_M,	sign_color_yellow,		10,	4,	1,
	glyph_N,	sign_color_yellow,		11,	4,	1,
	glyph_O,	sign_color_yellow,		12,	4,	1,
	glyph_P,	sign_color_yellow,		13,	4,	1,
	glyph_Q,	sign_color_yellow,		14,	4,	1,
	glyph_R,	sign_color_yellow,		15,	4,	1,

	glyph_S,	sign_color_yellow,		0,	5,	1,
	glyph_T,	sign_color_yellow,		1,	5,	1,
	glyph_U,	sign_color_yellow,		2,	5,	1,
	glyph_V,	sign_color_yellow,		3,	5,	1,
	glyph_W,	sign_color_yellow,		4,	5,	1,
	glyph_X,	sign_color_yellow,		5,	5,	1,
	glyph_Y,	sign_color_yellow,		6,	5,	1,
	glyph_Z,	sign_color_yellow,		7,	5,	1,
	glyph_dash,	sign_color_yellow,		8,	5,	1,
	glyph_dot,	sign_color_yellow,		9,	5,	1,
	glyph_left,	sign_color_yellow,		10,	5,	1,
	glyph_right,sign_color_yellow,		11,	5,	1,
	glyph_up,	sign_color_yellow,		12,	5,	1,
	glyph_down,	sign_color_yellow,		13,	5,	1,
	glyph_leftdown,	sign_color_yellow,		14,	5,	1,
	glyph_rightdown,sign_color_yellow,		15,	5,	1,

	glyph_leftup,	sign_color_yellow,		0,	6,	1,
	glyph_rightup,	sign_color_yellow,		1,	6,	1,
	glyph_period,	sign_color_yellow,		2,	6,	1,
	glyph_comma,	sign_color_yellow,		3,	6,	1,
	glyph_slash,	sign_color_yellow,		4,	6,	1,
	glyph_r1,	sign_color_yellow,		5,	6,	1,
	glyph_r2,	sign_color_yellow,		6,	6,	1,
	glyph_r3,	sign_color_yellow,		7,	6,	1,
	glyph_no_entry,	sign_color_independent,		8,	6,	1,
	glyph_0,	sign_color_red,		9,	6,	1,
	glyph_1,	sign_color_red,		10,	6,	1,
	glyph_2,	sign_color_red,		11,	6,	1,
	glyph_3,	sign_color_red,		12,	6,	1,
	glyph_4,	sign_color_red,		13,	6,	1,
	glyph_5,	sign_color_red,		14,	6,	1,
	glyph_6,	sign_color_red,		15,	6,	1,

	glyph_7,	sign_color_red,		0,	7,	1,
	glyph_8,	sign_color_red,		1,	7,	1,
	glyph_9,	sign_color_red,		2,	7,	1,
	glyph_A,	sign_color_red,		3,	7,	1,
	glyph_B,	sign_color_red,		4,	7,	1,
	glyph_C,	sign_color_red,		5,	7,	1,
	glyph_D,	sign_color_red,		6,	7,	1,
	glyph_E,	sign_color_red,		7,	7,	1,
	glyph_F,	sign_color_red,		8,	7,	1,
	glyph_G,	sign_color_red,		9,	7,	1,
	glyph_H,	sign_color_red,		10,	7,	1,
	glyph_I,	sign_color_red,		11,	7,	1,
	glyph_J,	sign_color_red,		12,	7,	1,
	glyph_K,	sign_color_red,		13,	7,	1,
	glyph_L,	sign_color_red,		14,	7,	1,
	glyph_M,	sign_color_red,		15,	7,	1,

	glyph_N,	sign_color_red,		0,	8,	1,
	glyph_O,	sign_color_red,		1,	8,	1,
	glyph_P,	sign_color_red,		2,	8,	1,
	glyph_Q,	sign_color_red,		3,	8,	1,
	glyph_R,	sign_color_red,		4,	8,	1,
	glyph_S,	sign_color_red,		5,	8,	1,
	glyph_T,	sign_color_red,		6,	8,	1,
	glyph_U,	sign_color_red,		7,	8,	1,
	glyph_V,	sign_color_red,		8,	8,	1,
	glyph_W,	sign_color_red,		9,	8,	1,
	glyph_X,	sign_color_red,		10,	8,	1,
	glyph_Y,	sign_color_red,		11,	8,	1,
	glyph_Z,	sign_color_red,		12,	8,	1,
	glyph_dash,	sign_color_red,		13,	8,	1,
	glyph_dot,	sign_color_red,		14,	8,	1,
	glyph_left,	sign_color_red,		15,	8,	1,

	glyph_right,	sign_color_red,		0,	9,	1,
	glyph_up,		sign_color_red,		1,	9,	1,
	glyph_down,		sign_color_red,		2,	9,	1,
	glyph_leftdown,	sign_color_red,		3,	9,	1,
	glyph_rightdown,sign_color_red,		4,	9,	1,
	glyph_leftup,	sign_color_red,		5,	9,	1,
	glyph_rightup,	sign_color_red,		6,	9,	1,
	glyph_period,	sign_color_red,		7,	9,	1,
	glyph_comma,	sign_color_red,		8,	9,	1,
	glyph_slash,	sign_color_red,		9,	9,	1,
	glyph_r1,		sign_color_red,		10,	9,	1,
	glyph_r2,		sign_color_red,		11,	9,	1,
	glyph_r3,		sign_color_red,		12,	9,	1,
	glyph_space,	sign_color_yellow,	13,	9,	1,
	glyph_space,	sign_color_red,		14,	9,	1,
	glyph_separator,sign_color_location,15,	9,	1,
	
	glyph_separator,sign_color_yellow,	0,	10,	1,
	glyph_separator,sign_color_red,		1,	10,	1	
};

static const int k_tex_syms_size = sizeof(k_tex_syms) / sizeof(k_tex_syms[0]);

const glyph_tex_info * lookup_glyph(parser_glyph_t g, parser_color_t c)
{
	for(int i = 0; i < k_tex_syms_size; ++i)
	if(k_tex_syms[i].glyph == g &&
		k_tex_syms[i].color == c)
		return k_tex_syms+i;
	return NULL;
}

int sign_token::calc_width() const
{
	if(glyph == glyph_critical ||
		glyph == glyph_safety ||
		glyph == glyph_hazard)
	return 64;

	if(glyph == glyph_no_entry)
		return 32;
	
	return 24 + (has_left_border ? 4 :0) + (has_right_border ? 4 : 0);
}

static sign_token make_from_parser(const parser_glyph_info& g)
{
	sign_token t;
	t.color = g.glyph_color;
	t.glyph = g.glyph_name;
	return t;
}

bool	sign_data::from_code(const string& code)
{
	parser_in_info input(code);
	parser_out_info output;	
	
	ParserTaxiSign(input, output);

	if(!output.errors.empty())
		return false;
		
	front.clear();
	back.clear();
	
	transform(
		output.out_sign.front.begin(),
		output.out_sign.front.end(),
		back_inserter(front),
		make_from_parser);

	transform(
		output.out_sign.back.begin(),
		output.out_sign.back.end(),
		back_inserter(back),
		make_from_parser);
	
	recalc_borders();
	
	return true;
}

string	sign_data::to_code() const
{
	parser_color_t lc = sign_color_invalid;
	bool in_braces = false;
	int side_count = back.empty() ? 1 : 2;
	
	string out_text;
	
	for(int side = 0; side < side_count; ++side)
	{
		const vector<sign_token>& s(side ? back : front);
		
		for(vector<sign_token>::const_iterator t = s.begin(); t != s.end(); ++t)
		{
			if(lc != t->color && t->color != sign_color_independent)
			{
				if(in_braces)
					out_text += ",@";
				else
				{
					out_text += "{@";
					in_braces = true;
				}
				out_text += (char) t->color;
				lc = t->color;
			}
			
			string sn = short_name_for_glyph(t->glyph);
			string ln = parser_name_for_glyph(t->glyph);
			
			if(in_braces && t->glyph == glyph_comma)
				sn.clear();
			
			if(sn.empty())
			{
				if(in_braces)
					out_text += ",";
				else
				{
					out_text += "{";
					in_braces = true;
				}
				out_text += ln;
			}
			else
			{
				if(in_braces)
				{
					out_text += "}";
					in_braces = false;
				}
				out_text += sn;
					
			}
			
		}
		
		if(side_count == 2 && side == 0)
		{
			if(in_braces)
				out_text += ",@@";
			else
			{
				out_text += "{@@";
				in_braces = true;
			}
		}		
	}
	if(in_braces)
		out_text += "}";
	return out_text;
}

void	sign_data::recalc_borders()
{
	for(int side = 0; side < 2; ++side)
	{
		vector<sign_token>& s(side ? back : front);
		if(!s.empty())
		{
			s.front().has_left_border = (s.front().color == sign_color_independent ? 0 : 1);
			s.back().has_right_border = (s.back().color == sign_color_independent ? 0 : 1);
			
			for(int j = 1; j < s.size(); ++j)
			{
				sign_token& prev(s[j-1]);
				sign_token& next(s[j]);
				
				if(prev.color == next.color)
					prev.has_right_border = next.has_left_border = 0;
				else
				{
					prev.has_right_border = (prev.color == sign_color_independent ? 0 : 1);
					next.has_left_border  = (next.color == sign_color_independent ? 0 : 1);
				}
				
			}
		}
	}
}

int		sign_data::calc_width(int side)
{
	vector<sign_token>& s(side ? back : front);
	int total = 0;
	for(vector<sign_token>::iterator t = s.begin(); t != s.end(); ++t)
		total += t->calc_width();
	return total;
}

int		sign_data::left_offset(int side, int token)
{
	vector<sign_token>& s(side ? back : front);
	DebugAssert(token <= s.size());
	int p = 0;
	for(int i = 0; i < token; ++i)
		p += s[i].calc_width();
	return p;
}

int		sign_data::right_offset(int side, int token)
{
	vector<sign_token>& s(side ? back : front);
	DebugAssert(token < s.size());
	int p = 0;
	for(int i = 0; i <= token; ++i)
		p += s[i].calc_width();
	return p;
}

int		sign_data::find_token(int side, int offset)
{
	vector<sign_token>& s(side ? back : front);
	if(offset < 0) return -1;
	int idx = 0;
	while(idx < s.size())
	{
		int w = s[idx].calc_width();
		if(offset < w)
			return idx;
		offset -= w;
		++idx;
	}
	return idx;
}

int		sign_data::insert_point(int side, int offset)
{
	vector<sign_token>& s(side ? back : front);
	if(offset < 0) return 0;
	int idx = 0;
	while(idx < s.size())
	{
		int w = s[idx].calc_width();
		if(offset < w)
		{
			if (offset < (w/2))
				return idx;
			return idx + 1;
		}
		offset -= w;
		++idx;
	}
	return idx;
	
}


void	sign_data::insert_glyph(int side, int position, const sign_token& glyph)
{
	vector<sign_token>& s(side ? back : front);
	DebugAssert(position >= 0);
	DebugAssert(position <= s.size());
	s.insert(s.begin()+position,glyph);
	recalc_borders();
}

void	sign_data::delete_range(int side, int start, int end)
{
	vector<sign_token>& s(side ? back : front);
	DebugAssert(start <= end);
	DebugAssert(start >= 0);
	DebugAssert(end <= s.size());
	s.erase(s.begin()+start,s.begin()+end);
	recalc_borders();	
}

void plot_from_sign(int x, int y, float sign_x, float sign_y, float sign_dx, float sign_dy)
{
	int x1 = x;
	int y1 = y;
	int x2 = x + sign_dx * 32.0f;
	int y2 = y + sign_dy * 32.0f;
	
	float s1 = sign_x * 32.0f;
	float s2 = s1 + sign_dx * 32.0f;
	float t1 = sign_y * 32.0f;
	float t2 = t1 + sign_dy * 32.0f;
	
		s1 /= 512.0;
		s2 /= 512.0;
		t1 /= 512.0;
		t2 /= 512.0;
		
		glBegin(GL_QUADS);
		glTexCoord2f(s1,t1);	glVertex2i(x1,y1);
		glTexCoord2f(s1,t2);	glVertex2i(x1,y2);
		glTexCoord2f(s2,t2);	glVertex2i(x2,y2);
		glTexCoord2f(s2,t1);	glVertex2i(x2,y1);
		glEnd();
}

int plot_token(const sign_token& sign, int x, int y, float scale, GUI_GraphState * g)
{
	int x1 = x;
	int x2 = x + (float) sign.calc_width() * scale;
	int y1 = y;
	int y2 = y + 32.0f * scale;
	
	const glyph_tex_info * info = lookup_glyph(sign.glyph, sign.color);
	if(info)
	{
		g->SetState(0,1,0,  0,0, 0,0);
		
		int tex = GUI_GetTextureResource("taxi_sign.png",0, NULL);
		if(tex)
			g->BindTex(tex,0);

		float s1 = info->x * 32;
		float s2 = s1 + 32 * info->width;
		float t2 = 512 - info->y * 32;
		float t1 = t2 - 32;
		
		if(!sign.has_left_border && sign.color != sign_color_independent)
			s1 += 4;
		if(!sign.has_right_border && sign.color != sign_color_independent)
			s2 -= 4;
		
		s1 /= 512.0;
		s2 /= 512.0;
		t1 /= 512.0;
		t2 /= 512.0;
		
		glBegin(GL_QUADS);
		glTexCoord2f(s1,t1);	glVertex2i(x1,y1);
		glTexCoord2f(s1,t2);	glVertex2i(x1,y2);
		glTexCoord2f(s2,t2);	glVertex2i(x2,y2);
		glTexCoord2f(s2,t1);	glVertex2i(x2,y1);
		glEnd();
	}
	return x2 - x1;
}

#pragma mark -

WED_Sign_Editor::WED_Sign_Editor(GUI_Commander * parent) : GUI_Commander(parent),
	mEditSide(0), mEditStart(0), mEditEnd(0),mCaret(1), mColor(sign_color_yellow)
{
	mData.from_code("{@L}B{@Y}TEST{@@,@R}18-22{@B}1");
}

void	WED_Sign_Editor::Draw(GUI_GraphState * state)
{
	int bounds[4];
	GetBounds(bounds);
	state->SetState(0, 0, 0, 0, 0, 0, 0);
	glColor3f(0.2, 0.2, 0.2);
	glBegin(GL_QUADS);
	glVertex2i(bounds[0],bounds[1]);
	glVertex2i(bounds[0],bounds[3]);
	glVertex2i(bounds[2],bounds[3]);
	glVertex2i(bounds[2],bounds[1]);
	glEnd();
	
	int sel_x1 = 16 + mData.left_offset(mEditSide, mEditStart) + bounds[0];
	int sel_x2 = 16 + mData.left_offset(mEditSide, mEditEnd) + bounds[0];
	if(sel_x1 > sel_x2)
		swap(sel_x1, sel_x2);
	
	glColor3f(1,1,1);

	state->SetState(0, 1, 0, 0, 1, 0, 0);
	int tex = GUI_GetTextureResource("taxi_sign.png",0, NULL);
	if(tex)
		state->BindTex(tex,0);

	plot_from_sign(bounds[0], bounds[3] - 42     , 2.0f, 5, 0.5, 1);
	plot_from_sign(bounds[0], bounds[3] - 42 - 42, 2.5f, 5, 0.5, 1);

	plot_from_sign(bounds[0] + 16, bounds[3] - 42 - 42 - 16 - 8, 3.0f, 5.0f, 2.0f, 0.5f);

	for(int x = bounds[0] + 16; x < (bounds[2] - 16); x += 32)
	{
		plot_from_sign(x, bounds[3] - 42, 5, 5, 1, 1);
		plot_from_sign(x, bounds[3] - 42 - 42, 5, 5, 1, 1);
	}
	
	for(int s = 0; s < 2; ++s)
	{
		int x = bounds[0] + 16;
		int y = bounds[3] - 42 * (s+1);
		
		vector<sign_token>& sign(s ? mData.back : mData.front);
		
		for(int t = 0; t < sign.size(); ++t)
		{	
			x += plot_token(sign[t], x, y, 1.0, state);
		}
	}
	
	state->SetState(0, 0, 0, 0, 1, 0, 0);
	
	int is_focused = IsFocused();
	glColor4f(0.2,0.65,1.0,0.5f);
	if(sel_x1 == sel_x2)
	{
		if(mColor == sign_color_red)
			glColor3f(1,0,0);
		if(mColor == sign_color_yellow)
			glColor3f(1,1,0);
		if(mColor == sign_color_black)
			glColor3f(1,1,1);
		if(mColor == sign_color_location)
			glColor3f(0.2,0.65,1.0);
	}
	int y2 = bounds[3] - 5 - 42* mEditSide;
	int y1 = y2 - 42;
	
	if(sel_x1 == sel_x2)
	{
		if(mCaret)
		{
			glBegin(GL_LINES);
			glVertex2i(sel_x1,y1);
			glVertex2i(sel_x1,y2);
			glEnd();
			glLineWidth(1);
		}
	}
	else
	{
		glBegin(is_focused ? GL_QUADS : GL_LINE_LOOP);
		glVertex2i(sel_x1,y1);
		glVertex2i(sel_x1,y2);
		glVertex2i(sel_x2,y2);
		glVertex2i(sel_x2,y1);
		glEnd();
	}
	
	parser_color_t colors[8] = { sign_color_yellow, sign_color_yellow, sign_color_location, sign_color_location, sign_color_red, sign_color_red, sign_color_black, sign_color_independent };

	glColor3f(1,1,1);

	int y = bounds[3] - 64 - 30 - 32;

	for(int r = 0; r < 8; ++r)
	{
		int x = bounds[0] + 16;
		
		int start = glyph_0;
		int end = glyph_r3;
		if(r == 1 || r == 3 || r == 5)
			start = glyph_A, end = glyph_Z;
		for(int g = start; g <= end; ++g)
		{
			sign_token gl;
			gl.has_left_border = 0;
			gl.has_right_border = 0;
			gl.glyph = (parser_glyph_t) g;
			gl.color = colors[r];
			
			if(parser_is_color_legal(gl.glyph, gl.color))
				x += plot_token(gl, x, y, 0.5, state) + 3;
		}
		y -= 21;
	}
	
	
}

int		WED_Sign_Editor::MouseMove(int x, int y			  )
{
	return 0;
}

int		WED_Sign_Editor::MouseDown(int x, int y, int button)
{
	int bounds[4];
	GetBounds(bounds);
	x -= (bounds[0]+16);
	
	int top_front = bounds[3]-10;
	int bot_front = top_front-32;
	int top_back = bot_front-10;
	int bot_back = top_back-32;
	
	bool in_front = (y >= bot_front && y < top_front);
	bool in_back = (y >= bot_back && y < top_back);
	
	if(in_front)
	{
		mEditSide = 0;
	}
	if(in_back)
	{
		mEditSide = 1;
	}
	
	if(in_front || in_back)
	{
		mEditStart = mEditEnd = mData.insert_point(mEditSide, x);
		selection_changed();	
		if(!IsFocused())
			TakeFocus();
		mIsDrag = 1;
		return 1;
	}


	parser_color_t colors[8] = { sign_color_yellow, sign_color_yellow, sign_color_location, sign_color_location, sign_color_red, sign_color_red, sign_color_black, sign_color_independent };
	int y1 = bounds[3] - 64 - 30 - 32;
	for(int r = 0; r < 8; ++r)
	{
		if(y >= y1 && y < (y1+16))
		{
			int x1 = 0;
			int start = glyph_0;
			int end = glyph_r3;
			if(r == 1 || r == 3 || r == 5)
				start = glyph_A, end = glyph_Z;
			for(int g = start; g <= end; ++g)
			{
				sign_token gl;
				gl.has_left_border = 0;
				gl.has_right_border = 0;
				gl.glyph = (parser_glyph_t) g;
				gl.color = colors[r];
				
				if(parser_is_color_legal(gl.glyph, gl.color))
				{
					int x2 = (x1 + gl.calc_width() / 2);
					if(x >= x1 && x < x2)
					{
						if(gl.color != sign_color_independent)
							mColor = gl.color;
						replace_selection(gl.glyph, gl.color);
						mIsDrag = 0;
						return 1;						
					}

					x1 = x2 + 3;
				}
			}
		}
		y1 -= 21;
	}

	return 1;	
}

void	WED_Sign_Editor::MouseDrag(int x, int y, int button)
{
	if(!mIsDrag)	
		return;
	int bounds[4];
	GetBounds(bounds);
	x -= (bounds[0]+16);

	int pos = mData.insert_point(mEditSide, x);
	
	mEditEnd = pos;
	selection_changed();	
}

void	WED_Sign_Editor::MouseUp  (int x, int y, int button)
{
}

int		WED_Sign_Editor::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if(inFlags & gui_DownFlag)
	switch(inKey) {
//#define GUI_KEY_RETURN      13
//#define GUI_KEY_ESCAPE      27
	case GUI_KEY_BACK:
		if(mEditStart == mEditEnd && mEditStart > 0)
			--mEditStart;
		delete_selection();
		return 1;
	case GUI_KEY_DELETE:
		if(mEditStart == mEditEnd && mEditStart != (mEditSide ? mData.back.size() : mData.front.size()))
			++mEditStart;
		delete_selection();
		return 1;
	case GUI_KEY_LEFT:
	case GUI_KEY_RIGHT:
		if((inFlags & gui_OptionAltFlag) == 0)
		{
			mEditEnd += (inKey == GUI_KEY_RIGHT ? 1 : -1);
			if((inFlags & gui_ShiftFlag) == 0)
				mEditStart = mEditEnd;
			selection_changed();	
			return 1;
		}
		break;
//	case GUI_KEY_TAB:
//		mEditSide = 1 - mEditSide;
//		selection_changed();
//		return 1;
	case GUI_KEY_UP:
		if((inFlags & gui_OptionAltFlag) == 0)
		{
			if(mEditSide == 1)
			{
				mEditSide = 0;
				selection_changed();
			}
			return 1;
		}
		break;
	case GUI_KEY_DOWN:
		if((inFlags & gui_OptionAltFlag) == 0)
		{
			if(mEditSide == 0)
			{
				mEditSide = 1;
				selection_changed();
			}
			return 1;
		}
		break;
	}
	
	if(inFlags & gui_DownFlag)
	{
		if(inFlags & gui_OptionAltFlag)
		{
			if(inVK == GUI_VK_R)
				mColor = sign_color_red;
			if(inVK == GUI_VK_L)
				mColor = sign_color_location;
			if(inVK == GUI_VK_Y)
				mColor = sign_color_yellow;
			if(inVK == GUI_VK_B)
				mColor = sign_color_black;
			Refresh();
		}
		
		string k;
		k += toupper(inKey);
		parser_glyph_t g = glyph_for_short_name(k);
		if(inKey == ' ')
			g = glyph_space;
		if(inKey == ',')
			g = glyph_comma;
		
		if(inFlags & gui_OptionAltFlag)
		{
			if(inKey == GUI_KEY_LEFT)
				g = glyph_left;
			if(inKey == GUI_KEY_RIGHT)
				g = glyph_right;
			if(inKey == GUI_KEY_UP)
				g = glyph_up;
			if(inKey == GUI_KEY_DOWN)
				g = glyph_down;
		}	
		
		if(inVK == GUI_VK_NUMPAD1)
			g = glyph_leftdown;
		if(inVK == GUI_VK_NUMPAD2)
			g = glyph_down;
		if(inVK == GUI_VK_NUMPAD3)
			g = glyph_rightdown;
		if(inVK == GUI_VK_NUMPAD4)
			g = glyph_left;
		if(inVK == GUI_VK_NUMPAD6)
			g = glyph_right;
		if(inVK == GUI_VK_NUMPAD7)
			g = glyph_leftup;
		if(inVK == GUI_VK_NUMPAD8)
			g = glyph_up;
		if(inVK == GUI_VK_NUMPAD9)
			g = glyph_rightup;
			
		if(g != glyph_Invalid)
		{
			if(parser_is_color_legal(g, mColor))
				replace_selection(g, mColor);
			if(parser_is_color_legal(g, sign_color_independent))
				replace_selection(g, sign_color_independent);
			return 1;
		}
	}
	return 0;
}

void		WED_Sign_Editor::TimerFired(void)
{
	mCaret = !mCaret;
	Refresh();
}

int	WED_Sign_Editor::AcceptTakeFocus()
{
	mCaret = 1;
	Start(0.25);
	Refresh();
	return 1;
}

int	WED_Sign_Editor::AcceptLoseFocus(int force)
{
	Stop();
	mCaret = 0;
	Refresh();	
	return 1;
}

void	WED_Sign_Editor::delete_selection()
{
	if(mEditStart != mEditEnd)
	{
		mData.delete_range(mEditSide, min(mEditStart, mEditEnd),max(mEditStart, mEditEnd));
		mEditEnd = mEditStart = min(mEditEnd, mEditStart);
		Refresh();
	}
}

void		WED_Sign_Editor::replace_selection(parser_glyph_t g, parser_color_t c)
{
	vector<sign_token>& side(mEditSide ? mData.back : mData.front);
	int low = min(mEditStart,mEditEnd);
	int high = max(mEditStart,mEditEnd);
	
	if(g == glyph_separator)      // check for various cases where we don't allow a separator to be placed
	{
		if(low < 1) 
			return;
		if(side[low-1].color != c)
			return;
		if(side[low-1].color != sign_color_red &&
			side[low-1].color != sign_color_yellow &&
			side[low-1].color != sign_color_location)
			return;
		if(high == side.size())
			return;
		if(high < side.size())
		{			
			if(side[low-1].color != side[high].color)
				return;
		}
	}	
	
	delete_selection();
	sign_token gl;
	gl.color = c;
	gl.glyph = g;
	mData.insert_glyph(mEditSide, mEditStart, gl);
	mEditStart++;
	mEditEnd++;
	Refresh();
}

void		WED_Sign_Editor::selection_changed()
{
	Refresh();

	vector<sign_token>& side(mEditSide ? mData.back : mData.front);
	if(mEditStart < 0) mEditStart = 0;
	if(mEditEnd < 0) mEditEnd = 0;
	if(mEditStart > side.size()) mEditStart = side.size();
	if(mEditEnd > side.size()) mEditEnd = side.size();
	
	for(int i = max(mEditStart,mEditEnd)-1; i >= 0; --i)
	{
		if(side[i].color != sign_color_independent)
		{
			mColor = side[i].color;
			return;
		}
	}
}

bool		WED_Sign_Editor::SetSignText(const string& inDesc)
{
	if(!mData.from_code(inDesc))
		return false;
	selection_changed();	
	return true;	
}


void		WED_Sign_Editor::GetSignText(string& outDesc)
{
	outDesc = mData.to_code();
}



void RenderSign(GUI_GraphState * state, int x, int y, const string& sign_text, float scale, int font_id, const float color[4])
{
	sign_data sign;
	if(sign.from_code(sign_text))
	{
		y -= 4.0 * scale;
		for(int s = 0; s < 2; ++s)
		{
			vector<sign_token>& side(s ? sign.back : sign.front);
			for(int t = 0; t < side.size(); ++t)
			{
				x += plot_token(side[t], x, y, scale, state);
			}
			x += 5;
		}		
	}
	else
	{
		GUI_FontDraw(state, font_id, color, x, y, sign_text.c_str());
	}
}

