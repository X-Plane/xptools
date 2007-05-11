#ifndef WED_TOOLINFOADAPTER_H
#define WED_TOOLINFOADAPTER_H

#include "GUI_TextTable.h"

class	WED_MapToolNew;

class	WED_ToolInfoAdapter : public GUI_TextTableProvider, public GUI_TableGeometry, public GUI_Broadcaster {
public:

					 WED_ToolInfoAdapter();
	virtual			~WED_ToolInfoAdapter();

			void	SetTool(WED_MapToolNew * tool);

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

	virtual	int			GetColCount(void);
	virtual	int			GetRowCount(void);
	
	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);

	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);
	
	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);
	
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);

private:

	WED_MapToolNew *	mTool;
	
};

#endif