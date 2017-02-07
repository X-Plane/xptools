#include "WED_Taxi_Sign.h"
#include "AssertUtils.h"

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
