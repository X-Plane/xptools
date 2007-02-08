#ifndef GUI_TEXTTABLE_H
#define GUI_TEXTTABLE_H

#include "GUI_Listener.h"
#include "GUI_Table.h"

// Cell content as known by a text table - we have a few different kinds of cell
// displays...see comments for which fields they use.

enum GUI_CellContentType {
	
	gui_Cell_None,
	gui_Cell_Disclose,			// none
	gui_Cell_EditText,			// string val
	gui_Cell_CheckBox,			// int val
	gui_Cell_Integer,			// int val
	gui_Cell_Double,			// double val
	gui_Cell_Enum,				// string val and int val
	gui_Cell_EnumSet			// string val and int set val
};

struct	GUI_CellContent {
	GUI_CellContentType		content_type;
	int						can_edit;
	int						can_disclose;
	int						can_select;
	
	int						is_disclosed;
	int						is_selected;
	int						indent_level;

	string					text_val;		// Only one of these is used - which one depends on the cell content type!
	int						int_val;
	double					double_val;
	set<int>				int_set_val;
};

struct GUI_HeaderContent {
	string					title;
	int						is_selected;
	
	int						can_resize;
	int						can_select;
};

typedef	map<int, string>		GUI_EnumDictionary;

class	GUI_TextTableProvider : public GUI_Broadcaster {
public:

	virtual void	GetCellContent(
						int							cell_x, 
						int							cell_y, 
						GUI_CellContent&			the_content)=0;	
	virtual	void	GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						GUI_EnumDictionary&			out_dictionary)=0;
	virtual	void	AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content)=0;
	virtual	void	ToggleDisclose(
						int							cell_x,
						int							cell_y)=0;

};

class	GUI_TextTableHeaderProvider : public GUI_Broadcaster { 
public:

	virtual	void	GetHeaderContent(
						int							cell_x,
						GUI_HeaderContent&			the_content)=0;
						
};

// A text table is table content - that is, the drawing brains of a table.
// It in turn queries the "provider" for the actual content.  It allows you to specify a table as strings (easy)
// instead of drawing calls (PITA).

class	GUI_TextTable : public GUI_TableContent, public GUI_Listener {
public:

						 GUI_TextTable();
	virtual				~GUI_TextTable();
	
			void		SetProvider(GUI_TextTableProvider * content);

	virtual	void		CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  );
	virtual	int			CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);
	virtual	void		CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);
	virtual	void		CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);
	virtual	void		Deactivate(void);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:

	GUI_TextTableProvider * mContent;
	int		mClickCellX;
	int		mClickCellY;
	int		mEditType;
	int		mInBounds;
	int		mTrackLeft;
	int		mTrackRight;
	
};	



class	GUI_TextTableHeader : public GUI_TableHeader, public GUI_Listener {
public:
						 GUI_TextTableHeader();
	virtual				~GUI_TextTableHeader();
	
			void		SetProvider(GUI_TextTableHeaderProvider * content);
			void		SetGeometry(GUI_TableGeometry * geometry);

	virtual	void		HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState			  );
	virtual	int			HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button);
	virtual	void		HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button);
	virtual	void		HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:
	GUI_TextTableHeaderProvider *	mContent;
	GUI_TableGeometry *				mGeometry;
	int								mCellResize;
	int								mLastX;

};


#endif /* GUI_TEXTTABLE_H */