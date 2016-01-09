//
//  WED_SignEditor.hpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/31/15.
//
//

#ifndef WED_Sign_Editor_h
#define WED_Sign_Editor_h

#include "WED_Sign_Parser.h"
#include "GUI_Pane.h"
#include "GUI_Commander.h"

class GUI_GraphState;

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

	void	from_code(const string& code);
	string	to_code() const;
	void	recalc_borders();	// sets borders of each glyph as needed

	int		calc_width(int side); // width of whole sign in pixels
	int		left_offset(int side, int token);	// pixel of left side
	int		right_offset(int side, int token);	// pixel of left side
	int		find_token(int side, int offset);	// which token does a pixel fall in - tie goes to pixel on right
	
	void	insert_glyph(int side, int position, const sign_token& glyph);
	void	delete_range(int side, int start, int end);		

};
	
string optimize_signs(const string& input);	// cleans it up for copy-paste?
	
void plot_token(const sign_token& sign, int x, int y, GUI_GraphState * g);


	
class WED_Sign_Editor : public GUI_Pane, public GUI_Commander {
public:

	virtual	void		Draw(GUI_GraphState * state);
	virtual	int			MouseMove(int x, int y			  );
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

private:

	void	delete_selection();


	int		mEditSide;
	int		mEditStart;
	int		mEditEnd;
	
	
	sign_data		mData;
	
	
};







#endif /* WED_Sign_Editor_h */
