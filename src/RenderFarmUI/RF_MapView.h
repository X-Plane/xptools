/*
 * Copyright (c) 2004, Laminar Research.
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
#ifndef RF_MAPVIEW_H
#define RF_MAPVIEW_H

#include "GUI_Timer.h"

#include "CompGeomDefs2.h"
#include "GUI_Pane.h"
#include "GUI_Commander.h"
#include "GUI_Listener.h"
#include "RF_Notify.h"
#include "GUI_GraphState.h"
//#include "AssertUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

class	RF_MapZoomer;
class	RF_MapTool;
struct	ImageInfo;

extern	int			sShowMap;
extern	int			sShowMeshTrisHi;
extern	int			sShowMeshAlphas;
extern  int			sShowAirports;
extern	int			sShowShading;
extern  float		sShadingAzi;
extern	float		sShadingDecl;

#define	MESH_BUCKET_SIZE 4

class	RF_MapView : public GUI_Pane, public GUI_Commander, public GUI_Listener, public RF_Notifiable, public GUI_Timer {
public:

					 RF_MapView(GUI_Commander * cmdr);
	virtual			~RF_MapView();

	// GUI_Pane
	virtual void	SetBounds(int inBounds[4]);	
	virtual	void	Draw(GUI_GraphState * state);
	virtual	int		MouseMove(int x, int y			  );
	virtual	int		MouseDown(int x, int y, int button);
	virtual	void	MouseDrag(int x, int y, int button);
	virtual	void	MouseUp  (int x, int y, int button);
	virtual	int		ScrollWheel(int x, int y, int dist, int axis);

	// GUI_Commander
	virtual	int		AcceptTakeFocus(void) { return 1; }
	virtual	int		HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int		HandleCommand(int command);
	virtual	int		CanHandleCommand(int command, string& ioName, int& ioCheck);

	// RF_Notifiable
	virtual	void	HandleNotification(int catagory, int message, void * param);

	// GUI_Listener
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam);

	// GUI_Timer
	virtual	void		TimerFired(void);

			void	SetFlowImage(
							ImageInfo&				image,
							double					bounds[4]);

			void	MakeMenus(void);

private:

			bool	RecalcDEM(bool do_relief);

			RF_MapTool *	CurTool(void);
			char *	MonitorCaption(void);

	RF_MapZoomer *			mZoomer;
	int						mCurTool;
	vector<RF_MapTool *>	mTools;
	
	int						mOldX, mOldY;
	

	vector<GUI_Pane *>		mToolFuncBtns;		// Buttons that do tool-specific cmds
	vector<GUI_Pane *>		mToolProperties;	// Editing of tool properties
	vector<GUI_Pane *>		mSelModeBtns;		// Selection-mode buttons
	vector<GUI_Pane *>		mToolBarBtns;		// Tools themselves

	int						mToolBtnsOffset;
//	int						mToolStatusOffset;

	GLuint					mTexID;
	GLuint					mReliefID;
	bool					mHasTex;
	bool					mHasRelief;
	float					mTexS;
	float					mTexT;
	float					mReliefS;
	float					mReliefT;
	double					mDEMBounds[4];

	GLuint					mFlowID;
	bool					mHasFlow;
	float					mFlowS;
	float					mFlowT;
	double					mFlowBounds[4];

	bool					mNeedRecalcMapFull;
	bool					mNeedRecalcMapMeta;
	bool					mNeedRecalcDEM;
	bool					mNeedRecalcRelief;

	bool					mNeedRecalcMeshHi;
	bool					mNeedRecalcMeshHiAlpha;

	Bbox2					mDLBuckets[MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1];
	int						mDLMeshLine;
	int						mDLMeshFill;
	int						mTris[MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1];
	int						mEdges[MESH_BUCKET_SIZE * MESH_BUCKET_SIZE + 1];


	//TODO: make these private static!
	friend	void	RF_MapView_HandleMenuCommand(void *, void *);
	friend	void	RF_MapView_HandleDEMMenuCommand(void *, void *);
	friend	void	RF_MapView_UpdateCommandStatus(void);
};

extern	RF_MapView *		gMapView;

#endif
