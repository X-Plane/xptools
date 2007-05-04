#ifndef WED_PROPERTYTABLE_H
#define WED_PROPERTYTABLE_H

#include "GUI_TextTable.h"
#include "GUI_Listener.h"
#include "GUI_SimpleTableGeometry.h"

class	ISelection;
class	WED_Thing;
class	WED_Archive;
class	WED_Select;


class	WED_PropertyTable : public GUI_TextTableProvider, public GUI_SimpleTableGeometry, public GUI_Listener, public GUI_TextTableHeaderProvider, public GUI_Broadcaster {
public:

					 WED_PropertyTable(
									WED_Thing *				root,
									WED_Select *			selection,
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
	virtual	void	SelectCell(
						int							cell_x,
						int							cell_y);
	virtual	void	SelectCellToggle(
						int							cell_x,
						int							cell_y);
	virtual	void	SelectCellExtend(
						int							cell_x,
						int							cell_y);

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

	vector<string>				mColNames;

	WED_Archive *				mArchive;
	int							mEntity;	
	int							mSelect;
	
	hash_map<int,int>			mOpen;
	
	int							mVertical;
	int							mDynamicCols;
	int							mSelOnly;
	set<string>					mFilter;
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