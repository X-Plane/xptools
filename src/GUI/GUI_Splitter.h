#ifndef GUI_SPLITTER_H
#define GUI_SPLITTER_H

#include "GUI_Pane.h"

enum {
	gui_Split_Horizontal,
	gui_Split_Vertical
};

class GUI_Splitter : public GUI_Pane {
public:

						 GUI_Splitter(int direction);
	virtual				~GUI_Splitter();
	
			void		AlignContents();
			void		AlignContentsAt(int split);
			void		SetImage(const char * image_res);
	
	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);
	virtual	int			GetCursor(int x, int y);

private:

			int			GetSplitSize(void);

	int		mDirection;
	int		mSlop;		// mouse - boundary = slop
	int		mClick;
	
	string	mImage;
	
};

#endif
