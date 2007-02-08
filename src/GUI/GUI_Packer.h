#ifndef GUI_PACKER_H
#define GUI_PACKER_H

#include "GUI_Pane.h"

enum GUI_Packer_Side {
	gui_Pack_Left,
	gui_Pack_Right,
	gui_Pack_Bottom,
	gui_Pack_Top,
	gui_Pack_Center
};
	

class	GUI_Packer : public GUI_Pane {
public:

			 GUI_Packer();
	virtual	~GUI_Packer();
	
			void		PackPane(GUI_Pane * child, GUI_Packer_Side);

	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

private:

	int		mPackArea[4];

};


#endif /* GUI_PACKER_H */