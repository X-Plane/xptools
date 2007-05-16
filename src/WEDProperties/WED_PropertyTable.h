#ifndef WED_PROPERTYTABLE_H
#define WED_PROPERTYTABLE_H

#include "GUI_TextTable.h"
#include "GUI_Listener.h"
#include "GUI_SimpleTableGeometry.h"

class	IUnknown;
class	ISelection;
class	IResolver;
class	WED_Thing;
class	WED_Archive;
class	WED_Select;


class	WED_PropertyTable : public GUI_TextTableProvider, public GUI_SimpleTableGeometry, public GUI_Listener, public GUI_TextTableHeaderProvider, public GUI_Broadcaster {
public:

					 WED_PropertyTable(
									IResolver *				resolver,
									const char **			col_names,
									int *					def_col_widths,
									int						vertical,
									int						dynamic_cols,
									int						sel_only,
									const char **			filter);
	virtual			~WED_PropertyTable();

	virtual void	GetCellContent(
						int							cell_x, 
						int							cell_y, 
						GUI_CellContent&			the_content);	
	virtual	void	GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						map<int, string>&			out_dictionary);
	virtual	void	AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content);
	virtual	void	ToggleDisclose(
						int							cell_x,
						int							cell_y);
	virtual	void	DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							bounds[4]);
	virtual void	SelectionStart(
						int							clear);
	virtual	int		SelectGetExtent(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y);
	virtual	int		SelectGetLimits(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y);
	virtual	void	SelectRange(
						int							start_x,
						int							start_y,
						int							end_x,
						int							end_y,
						int							is_toggle);
	virtual	void	SelectionEnd(void);

	virtual	int		TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content);

	virtual	void					GetLegalDropOperations(
											int&						allow_between_col,
											int&						allow_between_row,
											int&						allow_into_cell);
	virtual	GUI_DragOperation		CanDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended,
											int&						whole_col,
											int&						whole_row);
	virtual	GUI_DragOperation		CanDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		CanDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);


	virtual	GUI_DragOperation		DoDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		DoDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		DoDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);


	virtual	int		GetColCount(void);
	virtual	int		GetRowCount(void);
	
	virtual void	GetHeaderContent(
						int							cell_x, 
						GUI_HeaderContent&			the_content);		

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:

			WED_Thing *		FetchNth(int row);
			WED_Thing *		FetchNthRecursive(WED_Thing * thing, int& row, ISelection * sel);
			int				CountRowsRecursive(WED_Thing * thing, ISelection * sel);
			int				GetThingDepth(WED_Thing * d);

			bool			GetOpen(int id);
			void			ToggleOpen(int id);
			void			GetFilterStatus(WED_Thing * what, ISelection * sel, 
									int&	visible, 
									int&	recurse_children,
									int&	can_disclose,
									int&	is_disclose);

	vector<string>				mColNames;

//	WED_Archive *				mArchive;
//	int							mEntity;	
//	int							mSelect;
	IResolver *					mResolver;

	
	hash_map<int,int>			mOpen;
	
	int							mVertical;
	int							mDynamicCols;
	int							mSelOnly;
	set<string>					mFilter;
	
	vector<IUnknown *>			mSelSave;
};


//----------------------------------------------------------------------------------------------------------------

class	WED_PropertyTableHeader : public GUI_TextTableHeaderProvider {
public:

					 WED_PropertyTableHeader(
									const char **			col_names,
									int *					def_col_widths);
	virtual			~WED_PropertyTableHeader();


private:

	vector<string>				mColNames;

};


#endif /* WED_PROPERTYTABLE_H */