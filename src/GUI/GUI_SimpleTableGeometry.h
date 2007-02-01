#ifndef GUI_SIMPLETABLEGEOMETRY_H
#define GUI_SIMPLETABLEGEOMETRY_H

#include "GUI_Table.h"

class	GUI_SimpleTableGeometryRowProvider {
public:

	virtual	int			CountRows(void)=0;

};

class	GUI_SimpleTableGeometry : public GUI_TableGeometry {
public:

						 GUI_SimpleTableGeometry(
										int		num_cols,
										GUI_SimpleTableGeometryRowProvider * row_provider,
										int *	default_col_widths,
										int		row_height);
										
	virtual				~GUI_SimpleTableGeometry();

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

		GUI_SimpleTableGeometryRowProvider *		mProvider;
		
			int			mRowHeight;
			vector<int>	mCols;
			
			
	
};

#endif
