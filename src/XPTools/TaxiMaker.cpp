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
#include "XObjDefs.h"
#include "XUtils.h"
#include "PlatformUtils.h"
#include "XGrinderApp.h"
#include "XObjReadWrite.h"
#include "ObjUtils.h"

float	gMasterScale = 0.6;		// Size of one square panel
float	gPoleHeight = 0.3;		// Height off the ground
float	gPoleRadius = 0.04;		// Radius of poles
float	gWidthTop = 0.04;		// Thickness at sign top
float	gWidthBottom = 0.08;	// Thickness at sign bottom
int		gUseMaterials = 0;		// Do we use materials to make taxiways light up?
string	gTextureName = "taxiways:taxi2";

/* TODO:

	Need to add space to end caps
	Need to generally fix how length is managed!

*/

#define	SYM_SPACE -1

enum {
	mode_None = -1,
	mode_Any = 0,
	mode_Yellow = 1,
	mode_Black = 2,
	mode_Red = 3,
	mode_White = 4
};

enum {
	sym_None,
	sym_Middle,
	sym_Leftcap,
	sym_Rightcap,
	sym_Bordered
};

struct	taxi_element_t {
	const char *			name;		// Symbol of the token
	int						s;			// Tex coordinates of the token
	int						t;
	int						w;
	int						h;
	int						match_mode;	// What mode must we be in to use this??
	int						end_mode;	// Mode mode are we in after this?
	int						category;	// What kind of element are we?

};


#define RED_YELLOW_LETTER(c, s1, s2)	\
	{ c, s1, 64, (s2 - s1), 32,		mode_Red,		mode_Red,	sym_Middle },	\
	{ c, s1, 96, (s2 - s1), 32,		mode_Yellow,	mode_Yellow,sym_Middle },

#define BORDERED_LETTER1(c, s, t)			\
	{ c, s * 32, t * 32, 32, 32, mode_Any, mode_None, sym_Bordered },

#define BLACK_BORDERED_LETTER(c, s, t) \
	{ "[" c "]"	, s * 32	, t * 32, 32, 32, mode_Any, mode_None, sym_Bordered }, \
	{ "[" c		, s * 32	, t * 32, 26, 32, mode_Any, mode_Black, sym_Leftcap }, \
	{ c "]"		, s * 32 + 6, t * 32, 26, 32, mode_Black, mode_None, sym_Rightcap }, \
	{ c			, s * 32 + 6, t * 32, 20, 32, mode_Black, mode_Black, sym_Middle },


#define BORDERED_LETTER2(c, s, t)			\
	{ c, s * 32, t * 32, 64, 32, mode_Any, mode_None, sym_Bordered },


static taxi_element_t	kNormal[] = {

RED_YELLOW_LETTER("0",	4,	19 )
RED_YELLOW_LETTER("1",	23, 35 )
RED_YELLOW_LETTER("2",	39, 54 )
RED_YELLOW_LETTER("3",	58,	73 )
RED_YELLOW_LETTER("4",	77,	92 )
RED_YELLOW_LETTER("5",	96, 111)
RED_YELLOW_LETTER("6",	115,130)
RED_YELLOW_LETTER("7",	134,149)
RED_YELLOW_LETTER("8",	153,168)
RED_YELLOW_LETTER("9",	172,187)

RED_YELLOW_LETTER("A",	191,208)
RED_YELLOW_LETTER("B",	212,227)
RED_YELLOW_LETTER("C",	231,246)
RED_YELLOW_LETTER("D",	250,265)
RED_YELLOW_LETTER("E",	269,284)
RED_YELLOW_LETTER("F",	288,303)
RED_YELLOW_LETTER("G",	307,322)
RED_YELLOW_LETTER("H",	326,341)
RED_YELLOW_LETTER("I",	345,354)
RED_YELLOW_LETTER("J",	358,373)
RED_YELLOW_LETTER("K",	377,392)
RED_YELLOW_LETTER("L",	396,411)
RED_YELLOW_LETTER("M",	415,433)
RED_YELLOW_LETTER("N",	437,455)
RED_YELLOW_LETTER("O",	459,477)
RED_YELLOW_LETTER("P",	481,496)
RED_YELLOW_LETTER("Q",	500,518)
RED_YELLOW_LETTER("R",	522,537)
RED_YELLOW_LETTER("S",	541,556)
RED_YELLOW_LETTER("T",	560,575)
RED_YELLOW_LETTER("U",	579,596)
RED_YELLOW_LETTER("V",	600,615)
RED_YELLOW_LETTER("W",	619,637)
RED_YELLOW_LETTER("X",	641,656)
RED_YELLOW_LETTER("Y",	660,675)
RED_YELLOW_LETTER("Z",	679,694)

RED_YELLOW_LETTER("-",	698,708)
RED_YELLOW_LETTER("*",	712,722)
RED_YELLOW_LETTER("<",	726,741)
RED_YELLOW_LETTER(">",	745,760)
RED_YELLOW_LETTER("^",	764,779)
RED_YELLOW_LETTER("v",	783,798)
RED_YELLOW_LETTER("|/",	802,817)
RED_YELLOW_LETTER("\\|",821,836)
RED_YELLOW_LETTER("|\\",840,855)
RED_YELLOW_LETTER("/|",	859,874)
RED_YELLOW_LETTER(".",	878,885)
RED_YELLOW_LETTER(",",	889,896)
RED_YELLOW_LETTER("/",	900,915)
RED_YELLOW_LETTER("i",	919,930)

BLACK_BORDERED_LETTER("A", 0, 1)
BLACK_BORDERED_LETTER("B", 1, 1)
BLACK_BORDERED_LETTER("C", 2, 1)
BLACK_BORDERED_LETTER("D", 3, 1)
BLACK_BORDERED_LETTER("E", 4, 1)
BLACK_BORDERED_LETTER("F", 5, 1)
BLACK_BORDERED_LETTER("G", 6, 1)
BLACK_BORDERED_LETTER("H", 7, 1)
BLACK_BORDERED_LETTER("I", 8, 1)
BLACK_BORDERED_LETTER("J", 9, 1)
BLACK_BORDERED_LETTER("K", 10, 1)
BLACK_BORDERED_LETTER("L", 11, 1)
BLACK_BORDERED_LETTER("M", 12, 1)
BLACK_BORDERED_LETTER("N", 13, 1)
BLACK_BORDERED_LETTER("O", 14, 1)
BLACK_BORDERED_LETTER("P", 15, 1)
BLACK_BORDERED_LETTER("Q", 16, 1)
BLACK_BORDERED_LETTER("R", 17, 1)
BLACK_BORDERED_LETTER("S", 18, 1)
BLACK_BORDERED_LETTER("T", 19, 1)
BLACK_BORDERED_LETTER("U", 20, 1)
BLACK_BORDERED_LETTER("V", 21, 1)
BLACK_BORDERED_LETTER("W", 22, 1)
BLACK_BORDERED_LETTER("X", 23, 1)
BLACK_BORDERED_LETTER("Y", 24, 1)
BLACK_BORDERED_LETTER("Z", 25, 1)
BLACK_BORDERED_LETTER("0", 26, 1)
BLACK_BORDERED_LETTER("1", 27, 1)
BLACK_BORDERED_LETTER("2", 28, 1)
BLACK_BORDERED_LETTER("3", 29, 1)
BLACK_BORDERED_LETTER("4", 30, 1)
BLACK_BORDERED_LETTER("5", 31, 1)

BLACK_BORDERED_LETTER("6", 0, 0)
BLACK_BORDERED_LETTER("7", 1, 0)
BLACK_BORDERED_LETTER("8", 2, 0)
BLACK_BORDERED_LETTER("9", 3, 0)
BORDERED_LETTER1("(1)", 4, 0)
BORDERED_LETTER1("(2)", 5, 0)
BORDERED_LETTER1("(3)", 6, 0)
BORDERED_LETTER1("(4)", 7, 0)
BORDERED_LETTER2("///", 8, 0)
BORDERED_LETTER2("===", 10, 0)
BORDERED_LETTER2("+++", 12, 0)
BORDERED_LETTER1("@", 14, 0)

	{ "|", 1016, 96, 8, 32, mode_Yellow, mode_None, sym_Rightcap },
	{ "|", 960 , 96, 8, 32, mode_Any,    mode_Yellow, sym_Leftcap  },
	{ "}", 1016, 64, 8, 32, mode_Red,    mode_None, sym_Rightcap },
	{ "{", 960 , 64, 8, 32, mode_Any,    mode_Red,    sym_Leftcap  },

	{	0, 0, 0, 0, 0, 0, 0 }
};

static	bool	MatchMode(int cur, int possible)
{
	switch(cur) {
	case mode_None: 	return (possible == mode_Any);
	case mode_Any:		return true;
	case mode_Yellow:
	case mode_Black:
	case mode_Red:
	case mode_White:	return (possible == mode_Any) || (possible == cur);
	default:			return (possible == mode_Any);
	}
}

static	bool	IsBorderPair(int leftSym, int rightSym)
{
	return ((leftSym == sym_Rightcap || leftSym == sym_Bordered) &&
			(rightSym == sym_Leftcap || rightSym == sym_Bordered));
}

void	StringToElements(const string& desc, vector<int>& outElements)
{
	int	curMode = mode_None;
	int	nextMode;

	for (string::size_type p = 0; p < desc.length(); )
	{
		int i = 0;
		int		matched = 0;
		int		best_index = 0;
		while (kNormal[i].name)
		{
			if (desc[p] == '_')
			{
				matched = 1;
				best_index = SYM_SPACE;
				nextMode = curMode;
			}
			if ((desc.find(kNormal[i].name, p) == p) && MatchMode(curMode, kNormal[i].match_mode))
			{
				if (strlen(kNormal[i].name) > matched)
				{
					best_index = i;
					matched = strlen(kNormal[i].name);
					nextMode = kNormal[i].end_mode;
				}
			}
			++i;
		}
		if (!matched)
			++p;
		else {
			p += matched;
			outElements.push_back(best_index);
			curMode = nextMode;
		}

	}
}

void	PushST(XObjCmd&	cmd, double	x, double y, double z, double s, double t)
{
	vec_tex v;
	v.v[0] = x; v.v[1] = y; v.v[2] = z;
	v.st[0] = s; v.st[1] = t;
	cmd.st.push_back(v);
}

double	BuildElements(XObj& obj, const vector<int>& elements, bool isBack, bool isUnmarked, double add_extra, double width)
{
	double caps = 0.0;
	vector<int>::const_iterator	ii, last, next;
	for (ii = elements.begin(); ii != elements.end(); ++ii)
	{
		if (*ii != SYM_SPACE)
			if ((kNormal[*ii].category == sym_Leftcap) ||
				(kNormal[*ii].category == sym_Rightcap))
			{
				caps += 1.0;
			}
	}

	if (caps > 0.0)
		add_extra = add_extra / caps;
	else
		add_extra = 0.0;

	double	hwidth = width * 0.5;
	double	pos = (isBack) ? hwidth : -hwidth;

	double	accum_width = 0.0;

	if (isUnmarked)
	{
		double	s1 = 608.0 / 1024.0;
		double	s2 = 640.0 / 1024.0;
		double	t1 = 0.0;
		double	t2 = 32.0 / 128.0;
		if ((width > 45) && (width < 110))
		{
			s1 = 640.0 / 1024.0;
			s2 = 704.0 / 1024.0;
		}

		double	x1 = pos / 32.0;
		double	x2 = -pos / 32.0;
		double	y1 = 0.0;
		double	y2 = 32.0 / 32.0;

		XObjCmd		quadCmd;

		quadCmd.cmdType = type_Poly;
		quadCmd.cmdID = obj_Quad;

		// LB
		PushST(quadCmd, x1 * gMasterScale, y1 * gMasterScale + gPoleHeight,
			isBack ? -gWidthBottom : gWidthBottom, s1, t1);
		// LT
		PushST(quadCmd, x1 * gMasterScale, y2 * gMasterScale + gPoleHeight,
			isBack ? -gWidthTop : gWidthTop, s1, t2);
		// RT
		PushST(quadCmd, x2 * gMasterScale, y2 * gMasterScale + gPoleHeight,
			isBack ? -gWidthTop : gWidthTop, s2, t2);
		// RB
		PushST(quadCmd, x2 * gMasterScale, y1 * gMasterScale + gPoleHeight,
			isBack ? -gWidthBottom : gWidthBottom, s2, t1);
		obj.cmds.push_back(quadCmd);

	} else {

		last = elements.end();
		for (ii = elements.begin(); ii != elements.end(); ++ii)
		{
			if (*ii == SYM_SPACE)
			{
				last = ii;
				continue;
			}

			next = ii;
			++next;

			double	w = kNormal[*ii].w;
			double	o = 0.0;
			if (last != elements.end())
			{
				if (*last == SYM_SPACE)
				{
					if (kNormal[*ii].category == sym_Middle)
					{
						w += 3.0;
						o -= 3.0;
					}
				} else if (IsBorderPair(kNormal[*last].category, kNormal[*ii].category))
				{
					w -= 1.0;
				}
			}

			if (next != elements.end())
			{
				if (*next == SYM_SPACE)
				{
					if (kNormal[*ii].category == sym_Middle)
					{
						w += 3.0;
					}
				} else if (IsBorderPair(kNormal[*ii].category, kNormal[*next].category))
				{
					w -= 1.0;
				}
			}

			if (kNormal[*ii].category == sym_Leftcap)
			{
				w += add_extra;
			}

			if (kNormal[*ii].category == sym_Rightcap)
			{
				w += add_extra;
				o -= add_extra;
			}

			double	x1 = pos / 32.0;
			double	x2 = (isBack ? (pos - w) : (pos + w) ) / 32.0;
			double	y1 = 0.0;
			double	y2 = kNormal[*ii].h / 32.0;

			double	s1 = (o + kNormal[*ii].s) / 1024.0;
			double	s2 = (o + kNormal[*ii].s + w) / 1024.0;
			double	t1 = kNormal[*ii].t / 128.0;
			double	t2 = (kNormal[*ii].t + kNormal[*ii].h) / 128.0;

			XObjCmd		quadCmd;

			quadCmd.cmdType = type_Poly;
			quadCmd.cmdID = obj_Quad;

			// LB
			PushST(quadCmd, x1 * gMasterScale, y1 * gMasterScale + gPoleHeight,
				isBack ? -gWidthBottom : gWidthBottom, s1, t1);
			// LT
			PushST(quadCmd, x1 * gMasterScale, y2 * gMasterScale + gPoleHeight,
				isBack ? -gWidthTop : gWidthTop, s1, t2);
			// RT
			PushST(quadCmd, x2 * gMasterScale, y2 * gMasterScale + gPoleHeight,
				isBack ? -gWidthTop : gWidthTop, s2, t2);
			// RB
			PushST(quadCmd, x2 * gMasterScale, y1 * gMasterScale + gPoleHeight,
				isBack ? -gWidthBottom : gWidthBottom, s2, t1);
			obj.cmds.push_back(quadCmd);

			if (isBack)
				pos -= w;
			else
				pos += w;

			accum_width += w;

			last = ii;
		}
	}
	return accum_width;
}

void	BuildEndCaps(XObj& obj, double width)
{
	XObjCmd	cmd, s1, s2;

	width *= 0.5;

	if (gWidthBottom > 0.0)
	{
		if (gWidthTop > 0.0)
		{
			cmd.cmdID = obj_Quad_Strip;
			cmd.cmdType = type_Poly;

			PushST(cmd, -width, gPoleHeight, 				  gWidthBottom,608.0 / 1024.0, 0.0 / 128.0);
			PushST(cmd, -width, gPoleHeight, 				 -gWidthBottom,640.0 / 1024.0, 0.0 / 128.0);

			PushST(cmd, -width, gPoleHeight + gMasterScale,  gWidthTop,608.0 / 1024.0, 8.0 / 128.0);
			PushST(cmd, -width, gPoleHeight + gMasterScale, -gWidthTop,640.0 / 1024.0, 8.0 / 128.0);

			PushST(cmd,  width, gPoleHeight + gMasterScale,  gWidthTop,608.0 / 1024.0, 24.0 / 128.0);
			PushST(cmd,  width, gPoleHeight + gMasterScale, -gWidthTop,640.0 / 1024.0, 24.0 / 128.0);

			PushST(cmd,  width, gPoleHeight, 				  gWidthBottom,608.0 / 1024.0, 32.0 / 128.0);
			PushST(cmd,  width, gPoleHeight, 				 -gWidthBottom,640.0 / 1024.0, 32.0 / 128.0);

			obj.cmds.push_back(cmd);

		} else {

			s1.cmdID = s2.cmdID = obj_Tri;
			s1.cmdType = s2.cmdType = type_Poly;

			PushST(s1, -width, gPoleHeight, 				  gWidthBottom,608.0 / 1024.0, 0.0 / 128.0);
			PushST(s1, -width, gPoleHeight, 				 -gWidthBottom,640.0 / 1024.0, 0.0 / 128.0);
			PushST(s1, -width, gPoleHeight + gMasterScale,  0.0,			608.0 / 1024.0, 8.0 / 128.0);

			PushST(s2,  width, gPoleHeight, 				 -gWidthBottom,640.0 / 1024.0, 32.0 / 128.0);
			PushST(s2,  width, gPoleHeight, 				  gWidthBottom,608.0 / 1024.0, 32.0 / 128.0);
			PushST(s2,  width, gPoleHeight + gMasterScale, 0.0,				640.0 / 1024.0, 24.0 / 128.0);

			obj.cmds.push_back(s1);
			obj.cmds.push_back(s2);

		}
	}
}

void	BuildPoles(XObj& obj, double width)
{
	if (gPoleHeight > 0.0 && gPoleRadius > 0.0)
	{
		width *= 0.7;

		XObjCmd	pole;
		pole.cmdType = type_Poly;
		pole.cmdID = obj_Quad_Strip;

		PushST(pole,  gPoleRadius, 0, 			-gPoleRadius, 	704.0 / 1024.0,  0.0 / 128.0);
		PushST(pole,  gPoleRadius, gPoleHeight, -gPoleRadius, 	704.0 / 1024.0, 32.0 / 128.0);

		PushST(pole, -gPoleRadius, 0, 			-gPoleRadius, 	712.0 / 1024.0,  0.0 / 128.0);
		PushST(pole, -gPoleRadius, gPoleHeight, -gPoleRadius, 	712.0 / 1024.0, 32.0 / 128.0);

		PushST(pole, -gPoleRadius, 0, 			 gPoleRadius, 	720.0 / 1024.0,  0.0 / 512.0);
		PushST(pole, -gPoleRadius, gPoleHeight,  gPoleRadius, 	720.0 / 1024.0, 32.0 / 128.0);

		PushST(pole,  gPoleRadius, 0, 			 gPoleRadius, 	728.0 / 1024.0,  0.0 / 512.0);
		PushST(pole,  gPoleRadius, gPoleHeight,  gPoleRadius, 	728.0 / 1024.0, 32.0 / 128.0);

		PushST(pole,  gPoleRadius, 0, 			-gPoleRadius, 	736.0 / 1024.0,  0.0 / 128.0);
		PushST(pole,  gPoleRadius, gPoleHeight, -gPoleRadius, 	736.0 / 1024.0, 32.0 / 128.0);

		vector<vec_tex>::iterator i;
		for (i = pole.st.begin(); i != pole.st.end(); ++i)
		{
			i->v[0] -= (width * 0.5);
		}
		obj.cmds.push_back(pole);

		for (i = pole.st.begin(); i != pole.st.end(); ++i)
		{
			i->v[0] += width;
		}
		obj.cmds.push_back(pole);
	}
}

void	XGrindInit(string& outTitle)
{
	outTitle = "XTaxiMaker";
	XGrinder_ShowMessage("Drag a .taxi file here to build objects.");
}

int	XGrinderMenuPick(xmenu menu, int item)
{
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
		string	path(inFileName);
		string	fname(inFileName);
		string	dir;
		string::size_type sep = path.rfind(DIR_CHAR);

	if (sep != path.npos)
	{
		fname = path.substr(sep+1);
		dir = path.substr(0, sep+1);
	}

	if (HasExtNoCase(fname, ".taxi"))
	{
		XGrinder_ShowMessage("Processing file %s.",fname.c_str());

		StTextFileScanner	fs(inFileName, true);

		if (!fs.done())
		{
			string	ln;
			vector<string>	args;
			while (GetNextNoComments(fs, ln))
			{
				if (ln == "HALT_HALT_HALT")	break;
				BreakString(ln, args);

				if (args.size() > 1)
				{
					if (args[0] == "MATERIALS")
					{
						gUseMaterials = atoi(args[1].c_str());
					} else if (args[0] == "SCALE")
					{
						gMasterScale = atof(args[1].c_str());
					} else if (args[0] == "HEIGHT")
					{
						gPoleHeight = atof(args[1].c_str());
					} else if (args[0] == "RADIUS")
					{
						gPoleRadius = atof(args[1].c_str());
					} else if (args[0] == "TOP")
					{
						gWidthTop = atof(args[1].c_str());
					} else if (args[0] == "BOTTOM")
					{
						gWidthBottom = atof(args[1].c_str());
					} else if (args[0] == "TEXTURE")
					{
						gTextureName = args[1];
					} else {

						vector<int>	front, back;

						StringToElements(args[1], front);

						if ((args.size() > 2) && (!args[2].empty()))
							StringToElements(args[2], back);

						XObj	obj;
						obj.texture = gTextureName;

						double	front_width = BuildElements(obj, front, false, false, 0.0, 0.0);
						double	back_width = 0.0;
						if ((args.size() > 2) && (!args[2].empty()))
							back_width = BuildElements(obj, back, true, false, 0.0, 0.0);

						double 	width = (front_width > back_width) ? front_width : back_width;
						double	add_front = width - front_width;
						double	add_back = width - back_width;

						obj.cmds.clear();

						XObjCmd	emissive;
						emissive.cmdType = type_Attr;
						emissive.cmdID = attr_Emission_RGB;
						emissive.attributes.push_back(1.0);
						emissive.attributes.push_back(1.0);
						emissive.attributes.push_back(1.0);
						if (gUseMaterials)
							obj.cmds.push_back(emissive);
						emissive.cmdID = attr_Reset;
						emissive.attributes.clear();

						BuildElements(obj, front, false/*front*/, false/*for real*/, add_front, width);
						if ((args.size() > 2) && (!args[2].empty())) {
							back_width = BuildElements(obj, back, true/*back*/, false/*for real*/, add_back, width);
							if (gUseMaterials)
								obj.cmds.push_back(emissive);
						} else {
							if (gUseMaterials)
								obj.cmds.push_back(emissive);
							BuildElements(obj, front, true/*back*/, true/*just blanks*/, 0.0, width);
						}

						BuildEndCaps(obj, width * gMasterScale / 32.0);

						BuildPoles(obj, width * gMasterScale / 32.0);

						string	fullnewpath = dir + args[0];

						double	radius = GetObjRadius(obj);

						XObjCmd	lodCmd;
						lodCmd.cmdType = type_Attr;
						lodCmd.cmdID = attr_LOD;
						lodCmd.attributes.push_back(0.0);
						// Rational: we want a 4 meter sign to be visible from about
						// 2 miles away.  A 4 meter sign's radius is around 2 meters, because
						// signs are always centered.  so for each meter of radius, we want
						// a mile of vis.  About 1900 m per nm.
						lodCmd.attributes.push_back(3000);
						obj.cmds.insert(obj.cmds.begin(), lodCmd);

						XGrinder_ShowMessage("Outputing file %s.",fullnewpath.c_str());

						XObjWrite(fullnewpath.c_str(), obj);
					}
				}
			}
		} else {
			XGrinder_ShowMessage("Unable to process file %s.",fname.c_str());
		}
	}
}
