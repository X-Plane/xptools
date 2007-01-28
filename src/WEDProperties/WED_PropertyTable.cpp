#include "WED_PropertyTable.h"
#include "SQLUtils.h"
#if !DEV
	this is the worst code ever - trivial sql crap to just get us up and looking
#endif

WED_PropertyTable::WED_PropertyTable(
						sqlite3 *							db,
						const string&						table_name,
						const vector<WED_ColumnDesc>&		columns)
 : mDB(db), mColumns(columns), mTable(table_name)
{
}

WED_PropertyTable::~WED_PropertyTable()
{
}

void	WED_PropertyTable::GetCellContent(
						int							cell_x, 
						int							cell_y, 
						GUI_CellContent&			the_content)
{
	char query[1024];
	sprintf(query,"SELECT %s FROM %s LIMIT 1 OFFSET %d;",
		mColumns[cell_x].column_name.c_str(),
		mTable.c_str(),
		cell_y);
		
	int err = sql_do_hack(mDB, query, &the_content.text_val);
	the_content.can_edit = 0;
	the_content.content_type = mColumns[cell_x].content_type;
}

void	WED_PropertyTable::GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						map<int, string>&			out_dictionary)
{
	out_dictionary = mColumns[cell_x].enum_dict;
}

void	WED_PropertyTable::AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content)
{
}

int			WED_PropertyTable::GetColCount(void)
{
	return mColumns.size();
}

int			WED_PropertyTable::GetRowCount(void)
{
	char query[1024];
	sprintf(query,"SELECT COUNT(*) FROM %s;",
		mTable.c_str());
		
	string foo;	
	int err = sql_do_hack(mDB, query, &foo);
	if (err != SQLITE_OK) return 0;
	int r = atoi(foo.c_str());
	return r;
}
	
	
int			WED_PropertyTable::GetCellLeft (int n)
{
	return n * 100;
}

int			WED_PropertyTable::GetCellRight(int n)
{
	return n * 100 + 100;
}

int			WED_PropertyTable::GetCellWidth(int n)
{
	return 100;
}

int			WED_PropertyTable::GetCellBottom(int n)
{
	return n * 20;
}

int			WED_PropertyTable::GetCellTop	 (int n)
{
	return n * 20 + 20;
}

int			WED_PropertyTable::GetCellHeight(int n)
{
	return 20;
}
	
int			WED_PropertyTable::ColForX(int n)
{
	return n / 100;
}

int			WED_PropertyTable::RowForY(int n)
{
	return n / 20;
}
	
void		WED_PropertyTable::SetCellWidth (int n, int w)
{
}

void		WED_PropertyTable::SetCellHeight(int n, int h)
{
}

