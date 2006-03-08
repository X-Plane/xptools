#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include "GUI_GraphState.h"
#include "GUI_Pane.h"
#include "XWinGL.h"

class	GUI_Window : public GUI_Pane, public XWinGL {
public:
	
							GUI_Window(const char * inTitle, int inBounds[4]);
	virtual					~GUI_Window();
	
			void			SetClearSpecs(bool inDoClearColor, bool inDoClearDepth, float inClearColor[4]);

	// From GUI_Pane
	virtual void			Refresh(void);	
	virtual void			Show(void);
	virtual void			Hide(void);
	virtual void			SetBounds(int x1, int y1, int x2, int y2);
	virtual void			SetBounds(int inBounds[4]);
	virtual	void			SetDescriptor(const string& inDesc);
	virtual	int				InternalSetFocus(GUI_Pane * who);
	virtual	int				AcceptTakeFocus(void);
	virtual	GUI_Pane *		GetFocus(void);

	// From XWinGL
	virtual	void			Timer(void) { }
	virtual	bool			Closed(void) { return true; }

	virtual	void			ClickDown(int inX, int inY, int inButton);
	virtual	void			ClickUp(int inX, int inY, int inButton);
	virtual	void			ClickDrag(int inX, int inY, int inButton);
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis);

	virtual	int				KeyPressed(char inKey, long msg, long p1, long p2);

	// TODO - we need non-stub impl of these!
	virtual	void			DragEnter(int inX, int inY) { }
	virtual	void			DragOver(int inX, int inY) { }
	virtual	void			DragLeave(void) { }
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int inX, int inY) { }
	
	
	virtual	void			GLReshaped(int inWidth, int inHeight);
	virtual	void			GLDraw(void);

private:

	GUI_GraphState	mState;
	float			mClearColorRGBA[4];
	bool			mClearDepth;
	bool			mClearColor;
	GUI_Pane *		mMouseFocusPane;
	
	GUI_Pane *		mKeyFocus;

};



#endif
