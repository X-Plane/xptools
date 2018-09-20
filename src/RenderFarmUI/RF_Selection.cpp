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
#include "RF_Selection.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "GUI_Defs.h"
#include "GUI_Application.h"

int							gSelectionMode = rf_Select_Face;

set<Pmwx::Face_handle>		gFaceSelection;
set<Pmwx::Halfedge_handle>	gEdgeSelection;
set<Pmwx::Vertex_handle>	gVertexSelection;
set<PointFeatureSelection>	gPointFeatureSelection;


GUI_MenuItem_t	kSelectionItems[] = {
{	"Vertex",			'0',	gui_ControlFlag + gui_OptionAltFlag + gui_ShiftFlag,	0,	selCmd_Vertex	},
{	"Edge",				'1',	gui_ControlFlag + gui_OptionAltFlag + gui_ShiftFlag,	0,	selCmd_Edge		},
{	"Face",				'2',	gui_ControlFlag + gui_OptionAltFlag + gui_ShiftFlag,	0,	selCmd_Face		},
{	"Point Features",	'3',	gui_ControlFlag + gui_OptionAltFlag + gui_ShiftFlag,	0,	selCmd_Point	},
{	0,					0,		0,														0,	0				}};


void RF_RegisterSelectionCommands()
{
	GUI_Menu special_menu = gApplication->CreateMenu("Selection", kSelectionItems, gApplication->GetMenuBar(), 0);
}

int command_to_selection_mode(int cmd)
{
	switch(cmd) {
		case selCmd_Vertex:	return rf_Select_Vertex;
		case selCmd_Edge:	return rf_Select_Edge;
		case selCmd_Face:	return rf_Select_Face;
		case selCmd_Point:	return rf_Select_PointFeatures;
	}
	return -1;
}

void RF_HandleSelectionCommand(int cmd)
{
	RF_SetSelectionMode(command_to_selection_mode(cmd));
}

int RF_CanHandleSelectionCommand(int command, string& ioName, int &ioCheck)
{
	const int sel_mode = command_to_selection_mode(command);
	ioCheck = sel_mode == gSelectionMode;
	return 1;
}

void	RF_SetSelectionMode(int mode)
{
	if (mode == gSelectionMode) return;
	gSelectionMode = mode;
	int	cleared = 0;
	if (mode != rf_Select_Vertex && !gVertexSelection.empty()) { gVertexSelection.clear(); cleared = 1; }
	if (mode != rf_Select_Edge   && !  gEdgeSelection.empty()) {   gEdgeSelection.clear(); cleared = 1; }
	if (mode != rf_Select_Face   && !  gFaceSelection.empty()) {   gFaceSelection.clear(); cleared = 1; }
	if (mode != rf_Select_PointFeatures && !gPointFeatureSelection.empty()) { gPointFeatureSelection.clear(); cleared = 1; }

	RF_Notifiable::Notify(rf_Cat_Selection, rf_Msg_SelectionModeChanged, (void *) cleared);
}


