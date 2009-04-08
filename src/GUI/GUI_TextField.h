/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef GUI_TEXTFIELD_H
#define GUI_TEXTFIELD_H

/*

	TODO: occlusion - tell OGLE if we don't need to draw the entire visible bounds??

*/

#include "ogle.h"
#include "GUI_Pane.h"
#include "GUI_ScrollerPane.h"
#include "GUI_Commander.h"
#include "GUI_Timer.h"

class	GUI_GraphState;

class	GUI_TextField : public GUI_Pane,
						public GUI_Commander,
						public GUI_ScrollerPaneContent,
						public GUI_Timer,
						public OGLE {
public:

						 GUI_TextField(int h_scroll, GUI_Commander * parent);
	virtual				~GUI_TextField();

			void		SetFont(int font);
			void		SetColors(float text_color[4],
								  float hilite_color[4],
								  float bkgnd_color[4],
								  float box_color[4]);

			void		SetWidth(float width);
			void		SetKeyAllowed(char key, bool allowed);
			void		SetVKAllowed(int vk, bool allowed);
			void		SetMargins(float l, float b, float r, float t);

	// GUI_Pane
	virtual	void		Draw(GUI_GraphState * state);
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);
	virtual	int			ScrollWheel(int x, int y, int dist, int axis);
	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

	// GUI_Commander
	virtual	int			HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int			HandleCommand(int command);
	virtual	int			CanHandleCommand(int command, string& ioName, int& ioCheck);
	virtual	int			AcceptTakeFocus(void);
	virtual int			AcceptLoseFocus(int inForce);
	virtual	int			AcceptFocusChain(void);

	// GUI_ScrollerPaneContent
	virtual	void		GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4]);
	virtual	void		ScrollH(float xOffset);
	virtual	void		ScrollV(float yOffset);

	// GUI_Timer
	virtual	void		TimerFired(void);

protected:

	// OGLE
	virtual	void			GetVisibleBounds(
								float			bounds[4]);
	virtual	void			GetLogicalBounds(
								float			bounds[4]);
	virtual	void			SetLogicalHeight(
								float 			height);
	virtual	void			ScrollTo(
								float			where[4]);
	virtual	void			GetText(
								const char **	start_p,
								const char **	end_p);
	virtual	void			ReplaceText(
								int				offset1,
								int				offset2,
								const char *	t1,
								const char *	t2);
	virtual	float			GetLineHeight(void);
	virtual	float			MeasureString(
								const char * 	tStart,
								const char * 	tEnd);
	virtual	int				FitStringFwd(
								const char * 	tStart,
								const char * 	tEnd,
								float 			space);
	virtual	int				FitStringRev(
								const char * 	tStart,
								const char * 	tEnd,
								float 			space);
	virtual	void			DrawString(
								const char *	tStart,
								const char *	tEnd,
								float			x,
								float			y);
	virtual	void			DrawSelection(
								float			bounds[4]);
	virtual	const char *	WordBreak(
								const char *	t1,
								const char *	t2);

private:

			void			ConstrainLogicalBounds(void);

		int					mFont;
		int					mCaret;
		int					mScrollH;
		float				mLogicalBounds[4];
		GUI_GraphState * 	mState;
		string				mText;
		bool				mAllowed[256];
		bool				mAllowedVK[256];
		float				mMargins[4];

		float				mColorText[4];
		float				mColorHilite[4];
		float				mColorBkgnd[4];
		float				mColorBox[4];

};

#endif /* GUI_TEXTFIELD_H */

