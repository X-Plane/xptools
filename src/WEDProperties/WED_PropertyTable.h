#ifndef WED_PROPERTYTABLE_H
#define WED_PROPERTYTABLE_H

#include "GUI_TextTable.h"
#include "GUI_SimpleTableGeometry.h"

class	WED_Thing;
class	WED_Archive;


class	WED_PropertyTable : public GUI_TextTableProvider, public GUI_SimpleTableGeometryRowProvider {
public:

					 WED_PropertyTable(
									WED_Thing *				root,
									const char **			col_names,
									int *					def_col_widths);
	virtual			~WED_PropertyTable();

			GUI_TableGeometry *	GetGeometry(void) { return &mGeometry; }

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

	virtual	int		CountRows(void);


private:

			WED_Thing *		FetchNth(int row);
			WED_Thing *		FetchNthRecursive(WED_Thing * thing, int& row);
			int				CountRowsRecursive(WED_Thing * thing);
			int				GetThingDepth(WED_Thing * d);

	vector<string>				mColNames;

	GUI_SimpleTableGeometry		mGeometry;
	WED_Archive *				mArchive;
	int							mEntity;	
	
	hash_map<int,int>			mOpen;
	
};


//----------------------------------------------------------------------------------------------------------------

class	WED_PropertyTableHeader : public GUI_TextTableHeaderProvider {
public:

					 WED_PropertyTableHeader(
									const char **			col_names,
									int *					def_col_widths);
	virtual			~WED_PropertyTableHeader();

	virtual void	GetHeaderContent(
						int							cell_x, 
						GUI_HeaderContent&			the_content);	

private:

	vector<string>				mColNames;

};



#endif /* WED_PROPERTYTABLE_H */