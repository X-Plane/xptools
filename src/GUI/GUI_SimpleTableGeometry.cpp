#include "GUI_SimpleTableGeometry.h"
#include "GUI_Messages.h"

GUI_SimpleTableGeometry::GUI_SimpleTableGeometry(
			int		num_cols,
			GUI_SimpleTableGeometryRowProvider * row_provider,
			int *	default_col_widths,
			int		row_height)
{
	mProvider = row_provider;
	mRowHeight = row_height;
	mCols.resize(num_cols);
	int p = 0;
	for (int n = 0; n < num_cols; ++n)
	{
		p += default_col_widths[n];
		mCols[n] = p;
	}
}
			
GUI_SimpleTableGeometry::~GUI_SimpleTableGeometry()
{
}

int			GUI_SimpleTableGeometry::GetColCount(void)
{
	return mCols.size();
}

int			GUI_SimpleTableGeometry::GetRowCount(void)
{
	return mProvider->CountRows();
}
	
int			GUI_SimpleTableGeometry::GetCellLeft (int n)
{
	return (n == 0) ? 0 : mCols[n-1];
}

int			GUI_SimpleTableGeometry::GetCellRight(int n)
{
	return mCols[n];
}

int			GUI_SimpleTableGeometry::GetCellWidth(int n)
{
	return (n == 0) ? mCols[0] : (mCols[n] - mCols[n-1]);
}

int			GUI_SimpleTableGeometry::GetCellBottom(int n)
{
	return n * mRowHeight;
}

int			GUI_SimpleTableGeometry::GetCellTop	 (int n)
{
	return (n+1)*mRowHeight;
}

int			GUI_SimpleTableGeometry::GetCellHeight(int n)
{
	return mRowHeight;
}
	
int			GUI_SimpleTableGeometry::ColForX(int n)
{
	vector<int>::iterator i = lower_bound(mCols.begin(),mCols.end(),n);
	if (i == mCols.end()) return mCols.size();
	int p = distance(mCols.begin(),i);
	if (n == *i) ++p;
	if (p == mCols.size()) return -1;
	return p;
}

int			GUI_SimpleTableGeometry::RowForY(int n)
{
	return n / mRowHeight;
}
	
void		GUI_SimpleTableGeometry::SetCellWidth (int n, int w)
{
	int delta = w - GetCellWidth(n);
	for (int i = n; i < mCols.size(); ++i)	
		mCols[i] += delta;
	BroadcastMessage(GUI_TABLE_SHAPE_RESIZED, 0);
}

void		GUI_SimpleTableGeometry::SetCellHeight(int n, int h)
{
}
