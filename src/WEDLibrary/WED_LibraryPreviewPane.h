/*
 * Copyright (c) 2012, Laminar Research.
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

#ifndef WED_LibraryPreviewPane_H
#define WED_LibraryPreviewPane_H

class	WED_ResourceMgr;
class	ITexMgr;

#include "GUI_Pane.h"
#include "GUI_Button.h"
#include "GUI_Listener.h"
#include "GUI_Commander.h"

class WED_LibraryPreviewPane : public GUI_Pane, public GUI_Listener, public GUI_Commander, public GUI_Broadcaster {
public:

	WED_LibraryPreviewPane(GUI_Commander *cmdr, WED_ResourceMgr * res_mgr, ITexMgr * tex_mgr);

	void				Draw(GUI_GraphState * state) override;
	void				SetResource(const string& r, int res_type, int variants);
	void				ClearResource(void);

	void				ReceiveMessage(GUI_Broadcaster * inSrc, intptr_t inMsg, intptr_t inParam) override;

	int					MouseDown(int x, int y, int button) override;
	void				MouseDrag(int x, int y, int button) override;
	int					MouseMove(int x, int y) override;
	void				MouseUp(int x, int y, int button) override;
	int					ScrollWheel(int x, int y, int dist, int axis) override;

	/* DnD from GUI_Pane */

	GUI_DragOperation   DragEnter	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) override;
	GUI_DragOperation   DragOver	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) override;
	void                DragScroll	(int x, int y) override { }
	void                DragLeave	(void) override { }
	GUI_DragOperation   Drop        (int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) override;

private:
	void				DrawOneItem(int type, const string& res, int b[4], GUI_GraphState * g, const char * label = nullptr);
	void 				UpdateFacadePreview(void);
	void 				begin3d(const int *b, double radius_m);
	void				end3d(const int *b);

	int					mX, mY;
	float				mPsi,mThe;
	float				mPsiOrig,mTheOrig;
	int					mHgt,mHgtOrig;
	int					mWalls;          // maximum number of walls for Facade preview
	float				mWid,mWidOrig;

	float				mZoom;
	WED_ResourceMgr *	mResMgr;
	ITexMgr *			mTexMgr;
	string				mRes;
	int					mType;
	int					mNumVariants;  // number of variants provided by object
	int					mVariant;      // variant we want to show
	GUI_Button *		mInfoButton;   // Button to advance to next Variant, 2D/3D etc

	int					mMSAA;
	unsigned int		mFBO;			// for MSAA in preview window
	unsigned int		mColBuf, mDepthBuf;

	bool				mLightBackground;
	vector<pair<string, int> > mRess;
};

#endif
