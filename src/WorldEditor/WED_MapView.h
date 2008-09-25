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
#ifndef WED_MAPVIEW_H
#define WED_MAPVIEW_H

#include "CompGeomDefs2.h"
#include "WED_Pane.h"
#include "WED_Notify.h"

class	WED_MapZoomer;
class	WED_MapTool;
struct	ImageInfo;

extern	int			sShowMap;
extern	int			sShowMeshTrisHi;
extern	int			sShowMeshAlphas;
extern  int			sShowAirports;
extern	int			sShowShading;
extern  float		sShadingAzi;
extern	float		sShadingDecl;

#define	MESH_BUCKET_SIZE 4

class	WED_MapView : public WED_Pane, public WED_Notifiable {
public:

					WED_MapView(
                                   int                  inLeft,
                                   int                  inTop,
                                   int                  inRight,
                                   int                  inBottom,
                                   int                  inVisible,
                                   WED_Pane *			inSuper);
	virtual			~WED_MapView();

	virtual	void	DrawSelf(void);
	virtual	int		HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	int		HandleKey(char key, XPLMKeyFlags flags, char vkey);
	virtual	int		HandleMouseWheel(int x, int y, int direction);

	virtual	void	HandleNotification(int catagory, int message, void * param);

	virtual	int		MessageFunc(
                                   XPWidgetMessage      inMessage,
                                   long                 inParam1,
                                   long                 inParam2);

			void	SetFlowImage(
							ImageInfo&				image,
							double					bounds[4]);

private:

			bool	RecalcDEM(bool do_relief);
			void	SetupForTool(void);
			void	UpdateForTool(void);

			WED_MapTool *	CurTool(void);
			char *	MonitorCaption(void);

	WED_MapZoomer *			mZoomer;
	int						mCurTool;
	vector<WED_MapTool *>	mTools;

	vector<XPWidgetID>		mToolFuncBtns;		// Buttons that do tool-specific cmds
	vector<XPWidgetID>		mToolProperties;	// Editing of tool properties
	vector<XPWidgetID>		mSelModeBtns;		// Selection-mode buttons
	vector<XPWidgetID>		mToolBarBtns;		// Tools themselves

	int						mToolBtnsOffset;
	int						mToolStatusOffset;

	int						mTexID;
	int						mReliefID;
	bool					mHasTex;
	bool					mHasRelief;
	float					mTexS;
	float					mTexT;
	float					mReliefS;
	float					mReliefT;
	double					mDEMBounds[4];

	int						mFlowID;
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
	friend	void	WED_MapView_HandleMenuCommand(void *, void *);
	friend	void	WED_MapView_HandleDEMMenuCommand(void *, void *);
	friend	void	WED_MapView_UpdateCommandStatus(void);
};

extern	WED_MapView *		gMapView;

#endif
