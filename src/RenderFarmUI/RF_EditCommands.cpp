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

#include "RF_EditCommands.h"
#include "XPLMMenus.h"
#include "RF_Progress.h"
#include "PlatformUtils.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "MapTopology.h"
#include "RF_Globals.h"
#include "RF_Selection.h"
#include "RF_Assert.h"
#include "MapAlgs.h"
#include "MemFileUtils.h"
#include "Hydro.h"
#include "XESIO.h"
#include "GISTool_Globals.h"
enum {
//	editCmd_SimplifyWater,
	editCmd_SimplifyMap,
	editCmd_ClearArea,
	editCmd_InsertMap,
	editCmd_MakeWet,
	editCmd_Count
};

const char *	kEditCmdNames[] = {
//	"simplify water",
	"simplify map",
	"clear area",
	"insert map...",
	"make faces wet",
	0
};

static	XPLMMenuID	sEditMenu = NULL;

static	void	RF_HandleEditMenuCmd(void *, void * i);
static	void	RF_NotifyEditMenus(int catagory, int message, void * param);
static	void	RF_RecalcEditMenus(void);



void	RegisterEditCommands(void)
{
	int n;
	sEditMenu = XPLMCreateMenu("Edit", NULL, 0, RF_HandleEditMenuCmd, NULL);
	n = 0;
	while (kEditCmdNames[n])
	{
		XPLMAppendMenuItem(sEditMenu, kEditCmdNames[n], (void *) n, 1);
		++n;
	}
	RF_RegisterNotifyFunc(RF_NotifyEditMenus);
	RF_RecalcEditMenus();
}

static	void	RF_HandleEditMenuCmd(void *, void * i)
{
	try {
		int cmd = (int) i;
		switch(cmd) {
//		case editCmd_SimplifyWater:
//			for (set<Face_handle>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
//			{
//				SimplifyCoastlineFace(gMap, *fsel);
//			}
//			RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
//			break;
		case editCmd_SimplifyMap:
			SimplifyMap(gMap, true, RF_ProgressFunc);
			gEdgeSelection.clear();
			gFaceSelection.clear();
			gVertexSelection.clear();
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			break;
		case editCmd_ClearArea:
			{
				set<Face_handle> kill_f;
				int ctr;

				PROGRESS_START(RF_ProgressFunc, 0, 3, "Accumulating Faces")
				ctr = 0;
				for (set<Face_handle>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel, ++ctr)
				{
					PROGRESS_CHECK(RF_ProgressFunc, 0, 3, "Accumulating Faces", ctr, gFaceSelection.size(), gFaceSelection.size() / 200)
					kill_f.insert(*fsel);
				}
				PROGRESS_DONE(RF_ProgressFunc, 0, 3, "Accumulating Faces")

				PROGRESS_START(RF_ProgressFunc, 1, 3, "Accumulating Edges")
				set<Halfedge_handle> kill_e;			
				ctr = 0;
				for (Pmwx::Halfedge_iterator e = gMap.halfedges_begin(); e != gMap.halfedges_end(); ++e, ++ctr)
				if (e->data().mDominant)
				{
					PROGRESS_CHECK(RF_ProgressFunc, 1, 3, "Accumulating Edges", ctr, gMap.number_of_halfedges(), gMap.number_of_halfedges() / 200)
					if (kill_f.count(e->face()) &&
						kill_f.count(e->twin()->face()))
					kill_e.insert(e);
				}
				PROGRESS_DONE(RF_ProgressFunc, 1, 3, "Accumulating Edges")

				ctr = 0;
				PROGRESS_START(RF_ProgressFunc, 2, 3, "Deleting Edges")
				for (set<Halfedge_handle>::iterator kill = kill_e.begin(); kill != kill_e.end(); ++kill, ++ctr)
				{
					PROGRESS_CHECK(RF_ProgressFunc, 2, 3, "Deleting Edges", ctr, kill_e.size(), kill_e.size() / 200)
					gMap.remove_edge(*kill);
				}
				PROGRESS_DONE(RF_ProgressFunc, 2, 3, "Deleting Edges")
			}
			gEdgeSelection.clear();
			gFaceSelection.clear();
			gVertexSelection.clear();
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			break;
		case editCmd_InsertMap:
			{
/*			
				char	path[1024];
				path[0] = 0;
				if (gFaceSelection.size() == 1)
				if (GetFilePathFromUser(getFile_Open, "Please pick an .XES file to open for roads", "Open", 8, path, sizeof(path)))
				{
					MFMemFile * fi = MemFile_Open(path);
					if (fi)
					{
						Pmwx		overMap;
						ReadXESFile(fi, &overMap, NULL, NULL, NULL, RF_ProgressFunc);

						Point2 master1, master2, slave1, slave2;
						CalcBoundingBox(gMap, master1, master2);
						CalcBoundingBox(overMap, slave1, slave2);

						Vector2 delta(slave1, master1);

						for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
							overMap.UnindexVertex(i);

						for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
							i->point() += delta;

						for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
							overMap.ReindexVertex(i);

						SwapFace(gMap, overMap, *gFaceSelection.begin(), RF_ProgressFunc);

						RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
						MemFile_Close(fi);
					}
				}
*/				
			}			
			gEdgeSelection.clear();
			gFaceSelection.clear();
			gVertexSelection.clear();
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			break;
		case editCmd_MakeWet:
			for (set<Face_handle>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
			{
				(*f)->data().mTerrainType = terrain_Water;
	//			(*f)->mAreaFeature.mFeatType = feat_Park;
			}
			for (set<Halfedge_handle>::iterator e = gEdgeSelection.begin(); e != gEdgeSelection.end(); ++e)
			{
				(*e)->data().mSegments.clear();
			}
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			break;
		}
	} catch (rf_assert_fail_exception& e) {
	} catch (exception& e) {
		DoUserAlert(e.what());
	} catch (...) {
		DoUserAlert("The operation was aborted due to an unexpected internal error.");
	}
}

void	RF_NotifyEditMenus(int catagory, int message, void * param)
{
	switch(catagory) {
	case rf_Cat_Selection:
		RF_RecalcEditMenus();
		break;
	}
}

void	RF_RecalcEditMenus(void)
{
//	XPLMSetMenuItemName(sFileMenu, fileCmd_Save, gFilePath.empty() ? "Save..." : "Save", 1);
//	XPLMEnableMenuItem(sFileMenu,fileCmd_Revert, gDirty);
}
