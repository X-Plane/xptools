/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_CreateBoxTool.h"
#include "IResolver.h"
#include "WED_ToolUtils.h"
#include "WED_ExclusionZone.h"
#include "ISelection.h"
#include "WED_EnumSystem.h"
#include "WED_SimpleBoundaryNode.h"

static const char * kCreateCmds[] = {
	"Exclusion Zone"
};

WED_CreateBoxTool::WED_CreateBoxTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateBox_t			tool) :
	WED_CreateToolBase(tool_name,host, zoomer, resolver,archive,
	1,								// min pts
	1,								// max pts
	1,								// curve allowed
	1,								// curve required?
	0,								// close allowed
	0),								// close required?
	mType(tool),
		mExclusions(this,"Exclusions", XML_Name("",""), ExclusionTypes, 0)
{
}

WED_CreateBoxTool::~WED_CreateBoxTool()
{
}

void	WED_CreateBoxTool::AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)
{
		char buf[256];

	sprintf(buf, "Create %s",kCreateCmds[mType]);

	GetArchive()->StartCommand(buf);

	int idx;
	WED_Thing * host = WED_GetCreateHost(GetResolver(), false, true, idx);
	WED_ExclusionZone * exc;
	WED_GISBoundingBox * obj;

	switch(mType) {
	case create_Exclusion:
		obj = exc = WED_ExclusionZone::CreateTyped(GetArchive());
		exc->SetExclusions(mExclusions.value);
		break;
	}

	WED_SimpleBoundaryNode * n1 = WED_SimpleBoundaryNode::CreateTyped(GetArchive());
	WED_SimpleBoundaryNode * n2 = WED_SimpleBoundaryNode::CreateTyped(GetArchive());

	n1->SetParent(obj,0);
	n2->SetParent(obj,1);
	n1->SetName("Start");
	n2->SetName("End");

	obj->GetMin()->SetLocation(gis_Geo,pts[0]);
	obj->GetMax()->SetLocation(gis_Geo,dirs_hi[0]);
	static int n = 0;
	++n;
	obj->SetParent(host, idx);

	sprintf(buf,"Exclusion Zone %d", n);
	obj->SetName(buf);

	ISelection * sel = WED_GetSelect(GetResolver());
	sel->Clear();
	sel->Select(obj);

	GetArchive()->CommitCommand();

}


const char *	WED_CreateBoxTool::GetStatusText(void)
{
	return NULL;
}

bool		WED_CreateBoxTool::CanCreateNow(void)
{
	return true;
}

