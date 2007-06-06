#ifndef GUI_SIMPLETABLEGEOMETRY_H
#define GUI_SIMPLETABLEGEOMETRY_H

#include "GUI_Table.h"

class	GUI_SimpleTableGeometry : public GUI_TableGeometry {
public:

						 GUI_SimpleTableGeometry(
										int		num_cols,
										int *	default_col_widths,
										int		row_height);
										
	virtual				~GUI_SimpleTableGeometry();

//	virtual	int			GetColCount(void)=0;
//	virtual	int			GetRowCount(void)=0;
	
	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);

	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);
	
	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);
	
	virtual	bool		CanSetCellWidth (void) const;
	virtual	bool		CanSetCellHeight(void) const;
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);

private:

			void		ExtendTo(int x);

			int			mRowHeight;
			vector<int>	mCols;
};

#endif
