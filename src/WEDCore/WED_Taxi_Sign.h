/**
 *
 *
 *
 */

#ifndef __wed_taxi_sign_h__
#define __wed_taxi_sign_h__

#include "WED_Sign_Parser.h"

struct sign_token {
	parser_glyph_t	glyph;
	parser_color_t	color;
	int				has_left_border;
	int				has_right_border;

	int				calc_width() const;	// returns width in screen pixels
};

struct sign_data {
	vector<sign_token>	front;
	vector<sign_token>	back;

	bool	from_code(const string& code);
	string	to_code() const;
	void	recalc_borders();	// sets borders of each glyph as needed

	int		calc_width(int side); // width of whole sign in pixels
	int		left_offset(int side, int token);	// pixel of left side
	int		right_offset(int side, int token);	// pixel of left side
	int		find_token(int side, int offset);	// which token does a pixel fall in - tie goes to pixel on right
	int		insert_point(int side, int offset);

	void	insert_glyph(int side, int position, const sign_token& glyph);
	void	delete_range(int side, int start, int end);

};


#endif // defined(__wed_taxi_sign_h__)
