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

#include "WED_AptTable.h"
#include "GUI_Messages.h"
#include "STLUtils.h"

static int kDefCols[] = { 90, 330, 120, 130, 100 };

WED_AptTable::WED_AptTable(
						const AptVector * apts,
						const char * hdr3, 
						const char * hdr4) : GUI_SimpleTableGeometry(2+ (hdr3 != nullptr) + (hdr4 != nullptr), kDefCols),
	mApts(apts), 
	mSortColumn(1), 
	mInvertSort(1)
{
	mColumns = 2 + (hdr3 != nullptr);
	if(hdr3) { mColumns++; mHeaderTitle3 = hdr3; }
	if(hdr4) { mColumns++; mHeaderTitle4 = hdr4; }
}	


WED_AptTable::~WED_AptTable()
{
}

	
void	WED_AptTable::SetFilter(
						const string&				new_filter)
{
	mFilter = new_filter;
	resort();
}
			
void	WED_AptTable::AptVectorChanged(void)
{
	resort();
}
			
void	WED_AptTable::GetSelection(
						set<int>&					out_selection)
{
	out_selection = mSelected;
}

void	WED_AptTable::SelectHeaderCell(
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

void	WED_AptTable::GetHeaderContent(
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
		the_content.title = "Airport Name";
		break;
	case 2:
		the_content.title = "ICAO/Local"; 
		break;
	case 3:
		the_content.title = mHeaderTitle3;
		break;
	case 4:
		the_content.title = mHeaderTitle4;
		break;
	}		
}

int		WED_AptTable::GetColCount(void)
{
	return mColumns;
}

int		WED_AptTable::GetRowCount(void)
{
	return mSorted.size();
}


void	WED_AptTable::GetCellContent(
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

	switch(cell_x)
	{
		case 0:	the_content.text_val = mApts->at(apt_id).icao; break;
		case 1: the_content.text_val = mApts->at(apt_id).name; break;
		case 2: the_content.text_val = mApts->at(apt_id).meta_data.front().second; break;
		case 3: if(mApts->at(apt_id).meta_data.size() > 1)
					the_content.text_val = mApts->at(apt_id).meta_data.back().first; break;
		case 4: if(mApts->at(apt_id).meta_data.size() > 1)
					the_content.text_val = mApts->at(apt_id).meta_data.back().second; break;
	}
	the_content.string_is_resource = 0;
}

void	WED_AptTable::SelectionStart(
					int							clear)
{
	if(clear)
		mSelected.clear();
	mSelectedOrig = mSelected;
}

int		WED_AptTable::SelectGetExtent(
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

int		WED_AptTable::SelectGetLimits(
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

void	WED_AptTable::SelectRange(
					int							start_x,
					int							start_y,
					int							end_x,
					int							end_y,
					int							is_toggle)
{
	mSelected = mSelectedOrig;

	for(int x = start_y; x <= end_y; ++x)
	{
		int apt_id = mSorted[x];
		if(is_toggle && mSelected.count(apt_id))		mSelected.erase (apt_id);
		else											mSelected.insert(apt_id);		
	}
	BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);
}

void	WED_AptTable::SelectionEnd(void)
{
}

int		WED_AptTable::SelectDisclose(
					int							open_it,
					int							all)
{
	return 0;
}

int		WED_AptTable::TabAdvance(
					int&						io_x,
					int&						io_y,
					int							reverse,
					GUI_CellContent&			the_content)
{
	return 0;
}

int		WED_AptTable::DoubleClickCell(
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
		string xs;
		string ys;
		switch (sort_column_)
		{ 
			case 0: xs = apts_->at(x).icao;  ys = apts_->at(y).icao; break;
			case 1: xs = apts_->at(x).name;  ys = apts_->at(y).name; break;
			case 2: xs = apts_->at(x).meta_data.front().second;  
					ys = apts_->at(y).meta_data.front().second; break;
			case 3: if (apts_->at(x).meta_data.size() > 1) xs = apts_->at(x).meta_data.back().first;  
					if (apts_->at(y).meta_data.size() > 1) ys = apts_->at(y).meta_data.back().first; break;
			case 4: if (apts_->at(x).meta_data.size() > 1) xs = apts_->at(x).meta_data.back().second;  
					if (apts_->at(y).meta_data.size() > 1) ys = apts_->at(y).meta_data.back().second; break;
		}
		toupper(xs);
		toupper(ys);
		
		if(invert_sort_)
		{
			if(xs.empty()) xs = "\xFF";
			if(ys.empty()) ys = "\xFF";
			return ys < xs;
		}
		else
			return xs < ys;
	}

	int sort_column_;
	int invert_sort_;
	const AptVector * apts_;
};


void		WED_AptTable::resort(void)
{
	vector<string>	filters;
	tokenize_string_func(mFilter.begin(),mFilter.end(),back_inserter(filters),::isspace);

	mSorted.clear();
	for(int i = 0; i < mApts->size(); ++i)
	{
		if (filters.empty() ||
			filter_match(mApts->at(i).icao, filters.begin(),filters.end()) ||
			filter_match(mApts->at(i).name, filters.begin(),filters.end()) ||
			(mColumns <= 2 ? 0 : filter_match(mApts->at(i).meta_data.front().second, filters.begin(),filters.end())))
		{
			mSorted.push_back(i);
		}
	}
	sort(mSorted.begin(),mSorted.end(), sort_by_apt(mApts, mSortColumn,mInvertSort));
	BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);
}
