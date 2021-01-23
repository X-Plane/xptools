/*
 * Copyright (c) 2021, Laminar Research.
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

#ifndef WED_Road_Selector_h
#define WED_Road_Selector_h

#include "GUI_Pane.h"
#include "GUI_TextTable.h"
#include "GUI_Commander.h"

class GUI_GraphState;

struct road_entry {
	string	name;
	bool	checked;
	int		enu;
	road_entry(const char * c = "") : name(c), checked(false), enu(-1) {}
};

struct road_choices {
	struct  sfx_t {
		int idx;
		int enu;
		sfx_t(int i, int e) : idx(i), enu(e) {};
		};
	string prefix;
	vector<sfx_t> combis;
	road_choices(string s) : prefix(s) {};
};

class WED_Road_Selector : public GUI_EditorInsert {
public:
			WED_Road_Selector(GUI_Commander * parent, const GUI_EnumDictionary& dict);

	void	Draw(GUI_GraphState * state) override;

	int		MouseDown(int x, int y, int button) override;
	int		MouseMove(int x, int y) override;

	int		GetCursor(int x, int y) override { return gui_Cursor_Arrow; } // prevents cursor being affected by elements in underlyig windows

	bool	SetData(const GUI_CellContent& c) override;
	void	GetData(GUI_CellContent& c) override;
	void	GetSizeHint(int * w, int * h) override;

protected:

	int		AcceptTakeFocus() override { return 1; }
	int		AcceptLoseFocus(int force) override { return 1; }
	int		HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags) override;

private:

	vector<road_choices> mRd_prefix;   // prefix plus pairs of valid suffixes and emums
	vector<string>	mRd_suffix;
	int				mPfx;              // current selections
	int				mSfx;
	int				mChoice;           // original choice when tarted, used in case of abort

	int				mColWidth[2];
	int				mR, mC;
};

#endif /* WED_Road_Selector_h */
