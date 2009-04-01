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
int							gSelectionMode = wed_Select_Face;

set<Pmwx::Face_handle>		gFaceSelection;
set<Pmwx::Halfedge_handle>	gEdgeSelection;
set<Pmwx::Vertex_handle>	gVertexSelection;
set<PointFeatureSelection>	gPointFeatureSelection;

void	WED_SetSelectionMode(int mode)
{
	if (mode == gSelectionMode) return;
	gSelectionMode = mode;
	int	cleared = 0;
	if (mode != wed_Select_Vertex && !gVertexSelection.empty()) { gVertexSelection.clear(); cleared = 1; }
	if (mode != wed_Select_Edge   && !  gEdgeSelection.empty()) {   gEdgeSelection.clear(); cleared = 1; }
	if (mode != wed_Select_Face   && !  gFaceSelection.empty()) {   gFaceSelection.clear(); cleared = 1; }
	if (mode != wed_Select_PointFeatures && !gPointFeatureSelection.empty()) { gPointFeatureSelection.clear(); cleared = 1; }

	WED_Notifiable::Notify(wed_Cat_Selection, wed_Msg_SelectionModeChanged, (void *) cleared);
}


