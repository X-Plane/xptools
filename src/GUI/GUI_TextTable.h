#ifndef GUI_TEXTTABLE_H
#define GUI_TEXTTABLE_H

#include "GUI_Listener.h"
#include "GUI_Table.h"

// Cell content as known by a text table - we have a few different kinds of cell
// displays...see comments for which fields they use.

enum GUI_CellContentType {
	
	gui_Cell_None,
	gui_Cell_EditText,			// string val
	gui_Cell_CheckBox,			// int val
	gui_Cell_Integer,			// int val
	gui_Cell_Double,			// double val
	gui_Cell_Enum,				// string val and int val
	gui_Cell_EnumSet			// int set
};

struct	GUI_CellContent {
	GUI_CellContentType		content_type;
	int						can_edit;
	int						can_disclose;
	int						is_disclosed;

	string					text_val;		// Only one of these is used - which one depends on the cell content type!
	int						int_val;
	double					double_val;
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
	
};	


#endif /* GUI_TEXTTABLE_H */