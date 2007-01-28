#ifndef WED_PROPERTYTABLE_H
#define WED_PROPERTYTABLE_H

#include "GUI_TextTable.h"
#include <sqlite3.h>


struct	WED_ColumnDesc {
	string					column_name;
	GUI_CellContentType		content_type;
	GUI_EnumDictionary		enum_dict;
	int						width;
};




class	WED_PropertyTable : public GUI_TextTableProvider, public GUI_TableGeometry {
public:

					 WED_PropertyTable(
						sqlite3 *							db,
						const string&						table_name,
						const vector<WED_ColumnDesc>&		columns);
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

	virtual	int			GetColCount(void);
	virtual	int			GetRowCount(void);
	
	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);

	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);
	
	// Index
	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);
	
	// Setting geometry
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);

private:

	vector<WED_ColumnDesc>		mColumns;
	sqlite3 *					mDB;
	string						mTable;
};


#endif /* WED_PROPERTYTABLE_H */