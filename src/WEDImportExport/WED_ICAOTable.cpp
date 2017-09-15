/* 
 * Copyright (c) 2014, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#include "WED_ICAOTable.h"
#include "GUI_Messages.h"
#include "STLUtils.h"

static int kDefCols[] = { 100, 100 };

WED_ICAOTable::WED_ICAOTable(
						const AptVector *			apts) :
	GUI_SimpleTableGeometry(2,kDefCols,20),	
	mApts(apts),
	mSortColumn(1),
	mInvertSort(1)
{
}	


WED_ICAOTable::~WED_ICAOTable()
{
}

	
void	WED_ICAOTable::SetFilter(
						const string&				new_filter)
{
	mFilter = new_filter;
	resort();
}
			
void	WED_ICAOTable::AptVectorChanged(void)
{
	resort();
}
			
void	WED_ICAOTable::GetSelection(
						set<int>&					out_selection)
{
	out_selection = mSelected;
}

void	WED_ICAOTable::SelectHeaderCell(
						int							cell_x)
{
	if(cell_x == mSortColumn)
		mInvertSort = !mInvertSort;
	else
	{
		mSortColumn = cell_x;
		mInvertSort = 1;
	}
	resort();
}

void	WED_ICAOTable::GetHeaderContent(
						int							cell_x,
						GUI_HeaderContent&			the_content)
{
	the_content.is_selected = (cell_x == mSortColumn);
	the_content.can_resize = 1;
	the_content.can_select = 1;
	switch(cell_x) {
	case 0:
		the_content.title = "Airport ID";
		break;
	case 1:
		the_content.title = "Name";
		break;
	}		
}

int		WED_ICAOTable::GetColCount(void)
{
	return 2;
}

int		WED_ICAOTable::GetRowCount(void)
{
	return mSorted.size();
}


void	WED_ICAOTable::GetCellContent(
					int							cell_x,
					int							cell_y,
					GUI_CellContent&			the_content)
{
	the_content.content_type = gui_Cell_EditText;
	the_content.can_delete = false;
	the_content.can_edit = 0;
	the_content.can_disclose = 0;
	the_content.can_select = 1;
	the_content.can_drag = 0;

	int apt_id = mSorted[cell_y];

	the_content.is_disclosed = 0;
	the_content.is_selected = mSelected.count(apt_id);
	the_content.indent_level = 0;


	switch(cell_x) {
	case 0:		
		the_content.text_val = mApts->at(apt_id).icao;
		break;
	case 1:		
		the_content.text_val = mApts->at(apt_id).name;
		break;
	}
	the_content.string_is_resource = 0;
}

void	WED_ICAOTable::SelectionStart(
					int							clear)
{
	if(clear)
		mSelected.clear();
}

int		WED_ICAOTable::SelectGetExtent(
					int&						low_x,
					int&						low_y,
					int&						high_x,
					int&						high_y)
{
	if(mSorted.empty())	
		return 0;
	low_x = 0;
	high_x = 0;
	low_y = mSorted.size();
	high_y = 0;
	for(int i = 0; i < mSorted.size(); ++i)
	if(mSelected.count(mSorted[i]))
	{
		low_y = min(low_y,i);
		high_y = max(high_y,i);
	}
	if(low_y <= high_y)
		return 1;
	else
		return 0;
}

int		WED_ICAOTable::SelectGetLimits(
					int&						low_x,
					int&						low_y,
					int&						high_x,
					int&						high_y)
{
	if(mSorted.empty())	return 0;
	low_x = 0;
	low_y = 0;
	high_x = 0;
	high_y = mSorted.size()-1;
	return 1;
}

void	WED_ICAOTable::SelectRange(
					int							start_x,
					int							start_y,
					int							end_x,
					int							end_y,
					int							is_toggle)
{
	mSelected.clear();

	int apt_id = mSorted[start_y];
	mSelected.insert(apt_id);
	
	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
}

void	WED_ICAOTable::SelectionEnd(void)
{
}

int		WED_ICAOTable::SelectDisclose(
					int							open_it,
					int							all)
{
	return 0;
}

int		WED_ICAOTable::TabAdvance(
					int&						io_x,
					int&						io_y,
					int							reverse,
					GUI_CellContent&			the_content)
{
	return 0;
}

int		WED_ICAOTable::DoubleClickCell(
					int							cell_x,
					int							cell_y)
{
	return 0;
}
	
inline void toupper(string& io_string)
{
	for(int i = 0; i < io_string.size(); ++i)
		io_string[i] = toupper(io_string[i]);
}

struct sort_by_apt {
	sort_by_apt(const AptVector * apts, int sort_column, int invert_sort) : apts_(apts), sort_column_(sort_column), invert_sort_(invert_sort) { }

	bool operator()(int x, int y) const {
		string xs(sort_column_ ? apts_->at(x).name : apts_->at(x).icao);
		string ys(sort_column_ ? apts_->at(y).name : apts_->at(y).icao);
		toupper(xs);
		toupper(ys);
		if(invert_sort_)
			return ys < xs;
		else
			return xs < ys;
	}

	int sort_column_;
	int invert_sort_;
	const AptVector * apts_;
};


void		WED_ICAOTable::resort(void)
{
	set<int>	old_sorted;
	swap(old_sorted,mSelected);
	vector<string>	filters;
	tokenize_string_func(mFilter.begin(),mFilter.end(),back_inserter(filters),::isspace);

	mSorted.clear();
	for(int i = 0; i < mApts->size(); ++i)
	{
		if (filters.empty() ||
			filter_match(mApts->at(i).icao, filters.begin(),filters.end()) ||
			filter_match(mApts->at(i).name, filters.begin(),filters.end()))
		{
			if(old_sorted.count(i))
				mSelected.insert(i);
			mSorted.push_back(i);
		}
	}
	sort(mSorted.begin(),mSorted.end(), sort_by_apt(mApts, mSortColumn,mInvertSort));
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}
