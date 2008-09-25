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

/*
	Ben's TODO list:
		Alias Wavefront OBJects
		DOF support (uh what's DOF?)
		auto merge of textures????
*/

#include "XGrinderApp.h"
#include "PlatformUtils.h"
#include "XObjReadWrite.h"
#include "XObjDefs.h"
#include "ObjUtils.h"
#include "ObjConvert.h"
#include "XUtils.h"

#include "ConvertObjDXF.h"
#include "ConvertObj3DS.h"
//#include "ConvertObjVRML.h"

#include "XWin.h"

#define kFeetToMeters			0.3048
#define	kInchesToMeters			(0.3048 / 12.0)
#define kMetersToFeet (1.0 / kFeetToMeters)
#define kMetersToInches (1.0 / kInchesToMeters)

static xmenu	gSettingsM;


/*
	NOTE: in Y = up,
		x = x
		y = y
		z = z
	in Z = up
		x = -x
		z = y
		y = z

*/

const char *	kSettingsItems[] = {
	"Inches",
	"Feet",
	"Meters",
	"-",
	"Center Object Horizontally",
	"-",
	"Flip X",
	"Flip Y",
	"Flip Z",
	"-",
	"Polygons are CW",
	"Polygons are CCW",
	"-",
	"Y axis is up",
	"Z axis is up",
	"-",
	"Save as OBJ7",
	"Save as OBJ8",
	"Save as 3DS",
	"Save as DXF",
	0
};

enum {
	unit_Inches = 0,
	unit_Feet = 1,
	unit_Meters = 2,
	do_Center = 4,
	flip_X = 6,
	flip_Y = 7,
	flip_Z = 8,
	poly_CW = 10,
	poly_CCW = 11,
	axis_Y = 13,
	axis_Z = 14,
	save_OBJ7 = 16,
	save_OBJ8 = 17,
	save_3DS = 18,
	save_DXF = 19
};

static	int	gUnits = unit_Meters;
static	int	gCenterH = 0;
static	int	gFlipX = 0;
static	int	gFlipY = 0;
static	int	gFlipZ = 0;
static	int	gPoly = poly_CCW;
static	int	gAxis = axis_Z;
static	int	gSave = save_OBJ7;

void	ConformCheckItems(void)
{
	switch(gSave) {
	case save_OBJ7:		XGrinder_ShowMessage("Drag a file into this window to convert it to OBJ7.");	break;
	case save_OBJ8:		XGrinder_ShowMessage("Drag a file into this window to convert it to OBJ8.");	break;
	case save_3DS:		XGrinder_ShowMessage("Drag a file into this window to convert it to 3DS.");	break;
	case save_DXF:		XGrinder_ShowMessage("Drag a file into this window to convert it to DXF.");	break;
	}
	XWin::CheckMenuItem(gSettingsM, unit_Inches, 	gUnits == unit_Inches);
	XWin::CheckMenuItem(gSettingsM, unit_Feet, 		gUnits == unit_Feet);
	XWin::CheckMenuItem(gSettingsM, unit_Meters, 	gUnits == unit_Meters);
	XWin::CheckMenuItem(gSettingsM, do_Center,		gCenterH == do_Center);
	XWin::CheckMenuItem(gSettingsM, flip_X,			gFlipX == flip_X);
	XWin::CheckMenuItem(gSettingsM, flip_Y,			gFlipY == flip_Y);
	XWin::CheckMenuItem(gSettingsM, flip_Z,			gFlipZ == flip_Z);
	XWin::CheckMenuItem(gSettingsM, poly_CW,		gPoly == poly_CW);
	XWin::CheckMenuItem(gSettingsM, poly_CCW,		gPoly == poly_CCW);
	XWin::CheckMenuItem(gSettingsM, axis_Y,			gAxis == axis_Y);
	XWin::CheckMenuItem(gSettingsM, axis_Z,			gAxis == axis_Z);
	XWin::CheckMenuItem(gSettingsM, save_OBJ7,		gSave == save_OBJ7);
	XWin::CheckMenuItem(gSettingsM, save_OBJ8,		gSave == save_OBJ8);
	XWin::CheckMenuItem(gSettingsM, save_3DS,		gSave == save_3DS);
	XWin::CheckMenuItem(gSettingsM, save_DXF,		gSave == save_DXF);
}

void	PostProcessVertex(float v[3], bool inReverse)
{
	float	nv[3];

	if (gAxis == axis_Y)
	{
		nv[0] = v[0];
		nv[1] = v[1];
		nv[2] = v[2];
	} else {
		nv[0] = -v[0];
		nv[1] =  v[2];
		nv[2] =  v[1];
	}

	if (gFlipX)	nv[0] = -nv[0];
	if (gFlipY)	nv[1] = -nv[1];
	if (gFlipZ)	nv[2] = -nv[2];

	if (gUnits == unit_Feet)
	{
		nv[0] *= (inReverse ? kMetersToFeet : kFeetToMeters);
		nv[1] *= (inReverse ? kMetersToFeet : kFeetToMeters);
		nv[2] *= (inReverse ? kMetersToFeet : kFeetToMeters);
	}
	if (gUnits == unit_Inches)
	{
		nv[0] *= (inReverse ? kMetersToInches : kInchesToMeters);
		nv[1] *= (inReverse ? kMetersToInches : kInchesToMeters);
		nv[2] *= (inReverse ? kMetersToInches : kInchesToMeters);
	}

	v[0] = nv[0];
	v[1] = nv[1];
	v[2] = nv[2];
}

void	PostProcessObj(XObj& ioObj, bool inReverse)
{
	for (vector<XObjCmd>::iterator cmd = ioObj.cmds.begin(); cmd != ioObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			PostProcessVertex(st->v, inReverse);
		}
		for (vector<vec_rgb>::iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			PostProcessVertex(rgb->v, inReverse);
		}
	}

	if (gCenterH)
	{
		float sphere[4];
		GetObjBoundingSphere(ioObj, sphere);
		OffsetObject(ioObj, -sphere[0], 0.0, -sphere[2]);	// DO NOT center on Y!!
	}
}

void	XGrindInit(string& t)
{
	gSettingsM = XGrinder_AddMenu("Settings", kSettingsItems);
	ConformCheckItems();

	t = "Convert Objects";
	XGrinder_ShowMessage("Drag a file into this window to convert it to OBJ7.");
}

int	XGrinderMenuPick(xmenu menu, int item)
{
	if (menu == gSettingsM)
	{
		switch(item) {
		case unit_Inches:
		case unit_Feet:
		case unit_Meters:
			gUnits = item;
			break;
		case do_Center:		gCenterH = do_Center - gCenterH;	break;
		case flip_X:		gFlipX = flip_X - gFlipX;	break;
		case flip_Y:		gFlipY = flip_Y - gFlipY;	break;
		case flip_Z:		gFlipZ = flip_Z - gFlipZ;	break;
		case poly_CW:
		case poly_CCW:
			gPoly = item;
			break;
		case axis_Y:
		case axis_Z:
			gAxis = item;
			break;
		case save_OBJ7:
		case save_OBJ8:
		case save_3DS:
		case save_DXF:
			gSave = item;
			break;
		}
		ConformCheckItems();
		return 1;
	}
	return 0;
}


void	XGrindFile(const char * inFileName);

void	XGrindFiles(const vector<string>& files)
{
	for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		XGrindFile(i->c_str());
	}
}

void	XGrindFile(const char * inFileName)
{
	string	fullpath(inFileName);
	string	path, fname(inFileName);
	string::size_type sep = fname.rfind(DIR_CHAR);
	if (sep != fname.npos)
	{
		fname = fullpath.substr(sep+1);
		path = fullpath.substr(0, sep+1);
	}

	string	noext = fname;
	string	fname_new;
	bool	is_dxf = HasExtNoCase(fname, ".dxf");
	bool	is_3ds = HasExtNoCase(fname, ".3ds");
//	bool	is_wrl = HasExtNoCase(fname, ".wrl");

	if (HasExtNoCase(fname, ".obj") || is_dxf || is_3ds)
	{
		noext = fname.substr(0, fname.length() - 4);
	}

	switch(gSave) {
	case save_OBJ7:
	case save_OBJ8:		fname_new = noext + "_new.obj";		break;
	case save_3DS : 	fname_new = noext + "_new.3ds";		break;
	case save_DXF : 	fname_new = noext + "_new.dxf";		break;
	}

	string	new_full = path + fname_new;

	XGrinder_ShowMessage("Converting %s...",fname.c_str());

	XObj	obj;
	XObj8	obj8;

	bool success = false;

	if (is_3ds)
	{
		// 3DS files are normally clockwise polygon orientation...if we want CCW we
		// need to tell it to reverse.
		success = ReadObj3DS(inFileName, obj, gPoly == poly_CCW);
		if (success)
			PostProcessObj(obj, false);
	} else if (is_dxf)
	{
		// DXF files are normally clockwise polygon orientation...if we want CCW we
		// need to tell it to reverse.
		success = ReadObjDXF(inFileName, obj, gPoly == poly_CCW);
		if (success)
			PostProcessObj(obj, false);

	} else {
		success = XObj8Read(inFileName, obj8);
		if (success)
		{
			Obj8ToObj7(obj8, obj);
		} else
			success = XObjRead(inFileName, obj);
	}

	if (success)
	{
		if (gSave != save_OBJ7 && gSave != save_OBJ8)
			PostProcessObj(obj, true);
		switch(gSave) {
		case save_OBJ7:							success = XObjWrite		(new_full.c_str(), obj					);			break;
		case save_OBJ8:	Obj7ToObj8(obj, obj8);	success = XObj8Write	(new_full.c_str(), obj8					);			break;
		case save_3DS :							success = WriteObj3DS	(new_full.c_str(), obj,gPoly == poly_CCW);			break;
		case save_DXF :							success = WriteObjDXF	(new_full.c_str(), obj,gPoly == poly_CCW);			break;
		default: success = false;
		}
		if (!success)
			XGrinder_ShowMessage("Could not write %s as %s.",fname.c_str(), fname_new.c_str());
		else
			XGrinder_ShowMessage("Converted %s to %s.",fname.c_str(), fname_new.c_str());

	} else {
		XGrinder_ShowMessage("Could not open and parse %s.",fname.c_str());
	}
}