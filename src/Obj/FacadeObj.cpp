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
#include "hl_types.h"
#include "FacadeObj.h"
#include "MemFileUtils.h"
#include "CompGeomUtils.h"
#if LIN
#  include <GL/glu.h>
#else
#  include <glu.h>
#endif

/* BAS TODO

	Smarter choice of facades?
	Randomize choice of facades
	Randomize choice of roof textures

*/

#if APL
#define LSEP "\n"
#elif IBM
#define LSEP "\r\n"
#elif LIN
#define LSEP "\n"
#endif

#define	MAX_FACADE_PTS	256

ExtrudeFunc_f gExtrudeTessFunc;
void * gExtrudeTessRef;
float gExtrudeTessLODNear;
float gExtrudeTessLODFar;
float gExtrudeTessS1;
float gExtrudeTessT1;
float gExtrudeTessS2;
float gExtrudeTessT2;
float gExtrudeX1;
float gExtrudeZ1;
float gExtrudeX2;
float gExtrudeZ2;
int gExtrudeMode;
float	gExtrudeXYZ[MAX_FACADE_PTS * 3];
float	gExtrudeST[MAX_FACADE_PTS * 2];
int		gExtrudePtCount = 0;

#if IBM
	#define OGL_CALL __stdcall
#else
	#define OGL_CALL
#endif

FacadeWall_t::FacadeWall_t() :
	min_width(0.0),
	max_width(0.0),
	x_scale(0.0),
	y_scale(0.0),
	roof_slope(0.0),
	left(0),
	center(0),
	right(0),
	bottom(0),
	middle(0),
	top(0)
{
}

FacadeLOD_t::FacadeLOD_t() :
	lod_near(0.0),
	lod_far(0.0)
{
}

FacadeObj_t::FacadeObj_t() :
	is_ring(false),
	two_sided(false)
{
}

void OGL_CALL ExtrudeTessBegin(GLenum mode)
{
	switch(mode) {
	case GL_TRIANGLES:			gExtrudeMode = ext_Poly_Tri;		break;
	case GL_TRIANGLE_STRIP:		gExtrudeMode = ext_Poly_TriStrip;	break;
	case GL_TRIANGLE_FAN:		gExtrudeMode = ext_Poly_TriFan;		break;
	case GL_QUADS:				gExtrudeMode = ext_Poly_Quad;		break;
	case GL_QUAD_STRIP:			gExtrudeMode = ext_Poly_QuadStrip;	break;
	}
	gExtrudePtCount = 0;
}
void OGL_CALL ExtrudeTessEnd(void)
{
	gExtrudeTessFunc(gExtrudeMode, gExtrudePtCount, gExtrudeXYZ, gExtrudeST, gExtrudeTessLODNear, gExtrudeTessLODFar, gExtrudeTessRef);
}

void OGL_CALL ExtrudeTessVertex(float * pt)
{
	gExtrudeXYZ[gExtrudePtCount * 3  ] = pt[0];
	gExtrudeXYZ[gExtrudePtCount * 3+1] = pt[1];
	gExtrudeXYZ[gExtrudePtCount * 3+2] = pt[2];

	gExtrudeST[gExtrudePtCount * 2  ] = gExtrudeTessS1 + gExtrudeTessS2 * (pt[0] - gExtrudeX1) / gExtrudeX2;
	gExtrudeST[gExtrudePtCount * 2+1] = gExtrudeTessT1 + gExtrudeTessT2 * (pt[2] - gExtrudeZ1) / gExtrudeZ2;
	gExtrudePtCount++;
	if (gExtrudePtCount >= MAX_FACADE_PTS)
		MACIBM_alert(0, "Ran out of facade points.", "", "", "", t_exit);
}

bool TokenizeVector(const char * a, const char * b, void * r)
{
	vector<string> * v = (vector<string> *) r;
	v->push_back(string(a,b));
	return true;	// keep going
}

bool	SaveFacadeObjFile(const char * inPath, const FacadeObj_t& inObj)
{
	FILE * fi = fopen(inPath, "w");
	if (!fi) return false;
	fprintf(fi, "A" LSEP "800" LSEP "FACADE" LSEP LSEP);
	fprintf(fi, "TEXTURE %s" LSEP, inObj.texture.c_str());
	if (!inObj.texture_lit.empty())
		fprintf(fi, "TEXTURE_LIT %s" LSEP, inObj.texture.c_str());
	fprintf(fi, "RING %d" LSEP, inObj.is_ring);
	fprintf(fi, "TWO_SIDED %d" LSEP, inObj.two_sided);
	for (int L = 0; L < inObj.lods.size(); ++L)
	{
		fprintf(fi,"LOD %f %f" LSEP, inObj.lods[L].lod_near, inObj.lods[L].lod_far);
		for (int R = 0; R < inObj.lods[L].roof_s.size(); ++R)
			fprintf(fi,"  ROOF %lf %lf" LSEP, inObj.lods[L].roof_s[R], inObj.lods[L].roof_t[R]);
		for (int F = 0; F < inObj.lods[L].walls.size(); ++F)
		{
			fprintf(fi,"  WALL %lf %lf" LSEP, inObj.lods[L].walls[F].min_width, inObj.lods[L].walls[F].max_width);
			fprintf(fi,"    SCALE %lf %lf" LSEP, inObj.lods[L].walls[F].x_scale, inObj.lods[L].walls[F].y_scale);
			fprintf(fi,"    ROOF_SLOPE %lf" LSEP, inObj.lods[L].walls[F].roof_slope);

			int S = 0, T = 0, i;
			for (i = 0; i < inObj.lods[L].walls[F].left; ++i, ++S)
				fprintf(fi, "    LEFT %f %f" LSEP, inObj.lods[L].walls[F].s_panels[S].first,inObj.lods[L].walls[F].s_panels[S].second);
			for (i = 0; i < inObj.lods[L].walls[F].center; ++i, ++S)
				fprintf(fi, "    CENTER %f %f" LSEP, inObj.lods[L].walls[F].s_panels[S].first,inObj.lods[L].walls[F].s_panels[S].second);
			for (i = 0; i < inObj.lods[L].walls[F].right; ++i, ++S)
				fprintf(fi, "    RIGHT %f %f" LSEP, inObj.lods[L].walls[F].s_panels[S].first,inObj.lods[L].walls[F].s_panels[S].second);

			for (i = 0; i < inObj.lods[L].walls[F].bottom; ++i, ++T)
				fprintf(fi, "    BOTTOM %f %f" LSEP, inObj.lods[L].walls[F].t_floors[T].first,inObj.lods[L].walls[F].t_floors[T].second);
			for (i = 0; i < inObj.lods[L].walls[F].middle; ++i, ++T)
				fprintf(fi, "    MIDDLE %f %f" LSEP, inObj.lods[L].walls[F].t_floors[T].first,inObj.lods[L].walls[F].t_floors[T].second);
			for (i = 0; i < inObj.lods[L].walls[F].top; ++i, ++T)
				fprintf(fi, "    TOP %f %f" LSEP, inObj.lods[L].walls[F].t_floors[T].first,inObj.lods[L].walls[F].t_floors[T].second);
		}
	}
	fclose(fi);
	return true;
}


bool	ReadFacadeObjFile(const char * inPath, FacadeObj_t& outObj)
{
	outObj.lods.clear();
	outObj.is_ring = 1;
	outObj.two_sided = 0;
	outObj.texture_lit.clear();
	MFMemFile *	f = MemFile_Open(inPath);
	if (!f) return false;
	MFTextScanner * s = TextScanner_Open(f);
	if (!s) { MemFile_Close(f); return false; }

	vector<string>	header;
	while (!TextScanner_IsDone(s) && header.empty())
		{ TextScanner_TokenizeLine(s, " \t", "\r\n#", -1, TokenizeVector, &header); TextScanner_Next(s); }
	if (TextScanner_IsDone(s)) return false;
	if (header.size() != 1) return false;
	if (header[0] != "I" && header[0] != "A") return false;
	header.clear();
	while (!TextScanner_IsDone(s) && header.empty())
		{ TextScanner_TokenizeLine(s, " \t", "\r\n#", -1, TokenizeVector, &header); TextScanner_Next(s); }
	if (TextScanner_IsDone(s)) return false;
	if (header.empty()) return false;
	if (header[0] != "800") return false;
	header.clear();
	while (!TextScanner_IsDone(s) && header.empty())
		{ TextScanner_TokenizeLine(s, " \t", "\r\n#", -1, TokenizeVector, &header); TextScanner_Next(s); }
	if (header.empty()) return false;
	if (header[0] != "FACADE") return false;

	while (!TextScanner_IsDone(s))
	{
		vector<string>	tokens;
		TextScanner_TokenizeLine(s, " \t", "\r\n#", -1, TokenizeVector, &tokens);

		if (!tokens.empty())
		{
			// TEXTURE <file name>
			if (tokens[0] == "TEXTURE" && tokens.size() == 2)
				outObj.texture = tokens[1];

			// TEXTURE_LIT <filename>
			if (tokens[0] == "TEXTURE_LIT" && tokens.size() == 2)
				outObj.texture_lit = tokens[1];

			// LOD <near> <far>
			if (tokens[0] == "LOD" && tokens.size() == 3)
			{
				outObj.lods.push_back(FacadeLOD_t());
				outObj.lods.back().lod_near = atof(tokens[1].c_str());
				outObj.lods.back().lod_far  = atof(tokens[2].c_str());
			}

			// WALL <min width> <max width>
			else if (tokens[0] == "WALL" && tokens.size() == 3 && !outObj.lods.empty())
			{
				outObj.lods.back().walls.push_back(FacadeWall_t());
				outObj.lods.back().walls.back().min_width = atof(tokens[1].c_str());
				outObj.lods.back().walls.back().max_width = atof(tokens[2].c_str());
				outObj.lods.back().walls.back().roof_slope = 0.0;
				outObj.lods.back().walls.back().left = 0;
				outObj.lods.back().walls.back().center = 0;
				outObj.lods.back().walls.back().right = 0;
				outObj.lods.back().walls.back().bottom = 0;
				outObj.lods.back().walls.back().middle = 0;
				outObj.lods.back().walls.back().top = 0;
			}

			// SCALE <x scale y scale>
			else if (tokens[0] == "SCALE" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty())
			{
				outObj.lods.back().walls.back().x_scale = atof(tokens[1].c_str());
				outObj.lods.back().walls.back().y_scale = atof(tokens[2].c_str());
			}
			// ROOF_SLOPE <slope>
			else if (tokens[0] == "ROOF_SLOPE" && tokens.size() == 2 && !outObj.lods.empty()&& !outObj.lods.back().walls.empty())
				outObj.lods.back().walls.back().roof_slope = atof(tokens[1].c_str());

			// BOTTOM <t bottom> <t top>
			else if (tokens[0] == "BOTTOM" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().t_floors.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().bottom; }
			// MIDDLE <t bottom> <t top>
			else if (tokens[0] == "MIDDLE" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().t_floors.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().middle; }
			// TOP <t bottom> <t top>
			else if (tokens[0] == "TOP" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().t_floors.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().top; }

			// LEFT <s left> <s right>
			else if (tokens[0] == "LEFT" && tokens.size() == 3 && !outObj.lods.empty()&& !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().s_panels.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().left; }
			// CENTER <s left> <s right>
			else if (tokens[0] == "CENTER" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().s_panels.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().center; }
			// RIGHT <<s left> <s right>
			else if (tokens[0] == "RIGHT" && tokens.size() == 3 && !outObj.lods.empty() && !outObj.lods.back().walls.empty()) {
				outObj.lods.back().walls.back().s_panels.push_back(pair<float, float>(atof(tokens[1].c_str()),atof(tokens[2].c_str())));
				++outObj.lods.back().walls.back().right; }




			// ROOF <s> <t>
			else if (tokens[0] == "ROOF" && tokens.size() == 3 && !outObj.lods.empty())
			{
				outObj.lods.back().roof_s.push_back(atof(tokens[1].c_str()));
				outObj.lods.back().roof_t.push_back(atof(tokens[2].c_str()));
			}

			// RING <0 or 1>
			else if (tokens[0] == "RING" && tokens.size() == 2)
				outObj.is_ring = atoi(tokens[1].c_str());
			// TWO_SIDED <0 or 1>
			else if (tokens[0] == "TWO_SIDED" && tokens.size() == 2)
				outObj.two_sided = atoi(tokens[1].c_str());
		}

		TextScanner_Next(s);
	}
	TextScanner_Close(s);
	MemFile_Close(f);
	return true;
}

static	int		PanelsForLength( const FacadeWall_t& fac, double len)
{
	double	d = 0.0;
	for (int n = 0; n < fac.s_panels.size(); ++n)
	{
		d += ((fac.s_panels[n].second - fac.s_panels[n].first) * fac.x_scale);
	}
	d /= (double) fac.s_panels.size();
	int g = (len / d) + 0.5;
	return (g > 0) ? g : 1;
}

static	void	BuildOnePanel(
						const FacadeWall_t& fac,
						const Segment3& inBase,
						const Segment3& inRoof,
						const Vector3&	inUp,
						int				left,		// Panel indices
						int				bottom,		// And floor indices
						int				right,
						int				top,
						double			h_start,	// Ratios
						double			v_start,	// Meters
						double			h_end,
						double			v_end,
						bool			use_roof,
						ExtrudeFunc_f	func,
						double			lod_near,
						double			lod_far,
						void *			ref)
{
	float			coords[12];
	float			texes[8];
	Point3			p;
	p = inBase.midpoint(h_start) + inUp * (v_start * fac.y_scale);
	coords[0] = p.x;
	coords[1] = p.y;
	coords[2] = p.z;
	p = inBase.midpoint(h_end  ) + inUp * (v_start * fac.y_scale);
	coords[ 9] = p.x;
	coords[10] = p.y;
	coords[11] = p.z;

	if (use_roof) {
		p = inRoof.midpoint(h_start) + inUp * (v_end * fac.y_scale);
		coords[3] = p.x;
		coords[4] = p.y;
		coords[5] = p.z;
		p = inRoof.midpoint(h_end  ) + inUp * (v_end * fac.y_scale);
		coords[6] = p.x;
		coords[7] = p.y;
		coords[8] = p.z;
	} else {
		p = inBase.midpoint(h_start) + inUp * (v_end * fac.y_scale);
		coords[3] = p.x;
		coords[4] = p.y;
		coords[5] = p.z;
		p = inBase.midpoint(h_end  ) + inUp * (v_end * fac.y_scale);
		coords[6] = p.x;
		coords[7] = p.y;
		coords[8] = p.z;
	}
	texes[0] = fac.s_panels[left    ].first;
	texes[1] = fac.t_floors[bottom  ].first;
	texes[2] = fac.s_panels[left    ].first;
	texes[3] = fac.t_floors[top   -1].second;
	texes[4] = fac.s_panels[right -1].second;
	texes[5] = fac.t_floors[top   -1].second;
	texes[6] = fac.s_panels[right -1].second;
	texes[7] = fac.t_floors[bottom  ].first;

	func(ext_Poly_Quad, 4, coords, texes, lod_near, lod_far, ref);
}

double	BuildOneFacade(
						const FacadeWall_t&	fac,
						const Segment3& inBase,
						const Segment3& inRoof,
						int				inFloors,
						int				inPanels,
						const Vector3&	inUp,
						bool			inDoRoofAngle,
						double			lod_near,
						double			lod_far,
					    ExtrudeFunc_f	inFunc,
					    void *			inRef)
{
	if (inFloors == 0.0) return 0.0;

	// STEP 1: compute exactly how many floors we'll be doing.
	int	left_c, center_c, right_c, bottom_c, middle_c, top_c, ang_c;
	int center_r, middle_r;
	int n, i, j;

	if (inDoRoofAngle) {
		ang_c = 1; --inFloors;
	} else
		ang_c = 0;

	if (inFloors == (fac.bottom + fac.middle + fac.top))
	{
		bottom_c = inFloors; middle_c = 0; top_c = 0;
	} else {
		middle_c = inFloors - (fac.bottom + 2 * fac.middle + fac.top);
		if (middle_c < 0) middle_c = 0;
		bottom_c = fac.bottom + (inFloors - middle_c  - fac.bottom - fac.top) / 2;
		top_c = (inFloors - middle_c - bottom_c);
	}

	if (inPanels == (fac.left + fac.center + fac.right))
	{
		left_c = inPanels; right_c = 0; center_c = 0;
	} else {
		center_c = inPanels - (fac.left + 2 * fac.center + fac.right);
		if (center_c < 0) center_c = 0;
		left_c = fac.left + (inPanels - center_c - fac.left - fac.right) / 2;
		right_c = (inPanels - center_c - left_c);
	}
	// Also figure out how many times we're going to repeat the center section.
	center_r = fac.center ? ((center_c + fac.center - 1) / fac.center) : 0;
	middle_r = fac.middle ? ((middle_c + fac.middle - 1) / fac.middle) : 0;
	if (center_r == 0) center_c = 0;
	if (middle_r == 0) middle_c = 0;

	// STEP 2: figure out the spacing along the facade as fractions of the segment.
	// sum up the "length" of the panel in pixels.
	double	total_panel_width = 0.0;
	vector<double>	act_panel_s;		// This is the s coord of the right side of the panel as we render
	act_panel_s.push_back(0.0);
	for (n = 0; n < left_c; ++n) {
		act_panel_s.push_back(total_panel_width + fac.s_panels[n].second - fac.s_panels[n].first);
		total_panel_width = act_panel_s.back(); }
	for (n = 0; n < center_c; ++n) {
		act_panel_s.push_back(total_panel_width + fac.s_panels[fac.left + (n%fac.center)].second - fac.s_panels[fac.left + (n%fac.center)].first);
		total_panel_width = act_panel_s.back(); }
	for (n = 0; n < right_c; ++n) {
		act_panel_s.push_back(total_panel_width + fac.s_panels[fac.s_panels.size() - right_c + n].second - fac.s_panels[fac.s_panels.size() - right_c + n].first);
		total_panel_width = act_panel_s.back(); }
	if (total_panel_width == 0.0) return 0.0;
	// Normalize our widths, now we have the right-side S coordinate per panel.
	total_panel_width = 1.0 / total_panel_width;
	for (n = 0; n < act_panel_s.size(); ++n)
		act_panel_s[n] *= total_panel_width;
	act_panel_s[act_panel_s.size()-1] = 1.0;	// Hack - make sure right edge doesn't lose a tiny bit...that way we'll line up right.

	// STEP 3: figure out the heights of each part of the building
	vector<double>	act_floor_t;
	act_floor_t.push_back(0.0);
	double	total_floor_height = 0.0;
	for (n = 0; n < bottom_c; ++n) {
		act_floor_t.push_back(total_floor_height + fac.t_floors[n].second - fac.t_floors[n].first);
		total_floor_height = act_floor_t.back(); }
	for (n = 0; n < middle_c; ++n) {
		act_floor_t.push_back(total_floor_height + fac.t_floors[fac.bottom + (n%fac.middle)].second - fac.t_floors[fac.bottom + (n%fac.middle)].first);
		total_floor_height = act_floor_t.back(); }
	for (n = 0; n < top_c; ++n) {
		act_floor_t.push_back(total_floor_height + fac.t_floors[fac.t_floors.size() - top_c - ang_c + n].second - fac.t_floors[fac.t_floors.size() - top_c - ang_c + n].first);
		total_floor_height = act_floor_t.back(); }
	for (n = 0; n < ang_c; ++n) {
		act_floor_t.push_back(total_floor_height + fac.t_floors[fac.t_floors.size() - 1].second - fac.t_floors[fac.t_floors.size() - 1].first);
		total_floor_height = act_floor_t.back(); }
	// STEP 3: Now is the time on sprockets when we extrude.  Note that we build _one polygon_ for left, right, etc.

	int	l, r, t, b, h_count, v_count;

	if (bottom_c)
	{
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, 0, left_c, bottom_c,
							0.0, 0.0, act_panel_s[left_c], act_floor_t[bottom_c], false, inFunc, lod_near, lod_far, inRef);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, 0, fac.left+h_count, bottom_c,
							act_panel_s[left_c + l], 0.0, act_panel_s[left_c + r], act_floor_t[bottom_c], false, inFunc, lod_near, lod_far, inRef);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, 0, fac.s_panels.size(), bottom_c,
							act_panel_s[left_c + center_c], 0.0, act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c], false, inFunc, lod_near, lod_far, inRef);
	}

	for (j = 0; j < middle_r; ++j)
	{
		b = j * fac.middle;
		t = b + fac.middle;
		if (t > middle_c) t = middle_c;
		v_count = t - b;

		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.bottom, left_c, fac.bottom + v_count,
							0.0, act_floor_t[bottom_c + b], act_panel_s[left_c], act_floor_t[bottom_c + t], false, inFunc, lod_near, lod_far, inRef);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = l + fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.bottom, fac.left+h_count, fac.bottom + v_count,
							act_panel_s[left_c + l], act_floor_t[bottom_c + b], act_panel_s[left_c + r], act_floor_t[bottom_c + t], false, inFunc, lod_near, lod_far, inRef);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.bottom, fac.s_panels.size(), fac.bottom + v_count,
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + b], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + t], false, inFunc, lod_near, lod_far, inRef);
	}

	if (top_c)
	{
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.t_floors.size() - top_c - ang_c, left_c, fac.t_floors.size() - ang_c,
							0.0, act_floor_t[bottom_c + middle_c], act_panel_s[left_c], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, lod_near, lod_far, inRef);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.t_floors.size() - top_c - ang_c, fac.left+h_count, fac.t_floors.size() - ang_c,
							act_panel_s[left_c + l], act_floor_t[bottom_c + middle_c], act_panel_s[left_c + r], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, lod_near, lod_far, inRef);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.t_floors.size() - top_c - ang_c, fac.s_panels.size(), fac.t_floors.size() - ang_c,
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + middle_c], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, lod_near, lod_far, inRef);
	}

	if (ang_c)
	{
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.t_floors.size() - ang_c, left_c, fac.t_floors.size(),
							0.0, act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, lod_near, lod_far, inRef);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.t_floors.size() - ang_c, fac.left+h_count, fac.t_floors.size(),
							act_panel_s[left_c + l], act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c + r], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, lod_near, lod_far, inRef);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.t_floors.size() - ang_c, fac.s_panels.size(), fac.t_floors.size(),
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, lod_near, lod_far, inRef);
	}
	return total_floor_height * fac.y_scale;
}

int		FindFloorsForHeight(const FacadeObj_t& inObj, float height)
{
	if (inObj.lods.empty() || inObj.lods.front().walls.empty()) return 0;
	const FacadeWall_t& wall(inObj.lods.front().walls.front());
	// e.g. if scale is 100 then whole tex is 100 meters tall.
	// So if we need 100 meters we need "1" ST coords worth of stuff.
	float needed_pixels = height / wall.y_scale;
	float bot = 0.0;
	float mid = 0.0;
	float top = 0.0;
	for (int n = 0; n < wall.bottom; ++n)
		bot += wall.t_floors[n].second - wall.t_floors[n].first;
	for (int n = wall.bottom; n < (wall.t_floors.size() - wall.top); ++n)
		mid += wall.t_floors[n].second - wall.t_floors[n].first;
	for (int n = wall.t_floors.size() - wall.top; n < wall.t_floors.size(); ++n)
		top += wall.t_floors[n].second - wall.t_floors[n].first;
	if (needed_pixels < bot) return wall.bottom;
	if (needed_pixels < (bot + top)) return wall.bottom + wall.top;
	needed_pixels -= bot;
	needed_pixels -= top;
	needed_pixels /= mid;
	return wall.bottom + wall.top + needed_pixels * ((wall.t_floors.size() - wall.bottom - wall.top));
}

void	BuildFacadeObjLOD(
						int					L,
						const FacadeObj_t&	inObj,
					   const Polygon3& 	 	inPolygon,
					   int					inFloors,
					   const Vector3&		inUp,
					   ExtrudeFunc_f		inFunc,
					   void *				inRef)
{
	int				mod_seed = 0;			// Offset to keep from always using same walls
	vector<double>	lengths;				// Lengths of sides in meters
	vector<double>	insets;					// Inset distance for roof
	vector<int>		facade_types;			// Assignment of each facade for the polygon
	vector<int>		num_panels;				// For each facade, how many panels do we need?
	float			roof_st[MAX_FACADE_PTS * 2];// STs for the roof
	Polygon3		inset;					// The actual squished polygon
	bool			has_roof = false;		// Do we need a roof at all?
	int				n, d;

	for (n = 0; n < inPolygon.size(); ++n)
	{
		vector<int>	okDefs;
		lengths.push_back(sqrt(inPolygon.side(n).squared_length()));
		for (d = 0; d < inObj.lods[L].walls.size(); ++d)
		if (lengths.back() >= inObj.lods[L].walls[d].min_width &&
			lengths.back() < inObj.lods[L].walls[d].max_width)
				okDefs.push_back(d);
		if (okDefs.empty())
			facade_types.push_back(0);	// Shit, nothing fits?!??
		else
			facade_types.push_back(okDefs[0]);
	}

	double	roof = 0.0;

	if (!inObj.lods[L].walls.empty())
	{
	for (n = 0; n < inPolygon.size(); ++n)
	{
		num_panels.push_back(PanelsForLength(inObj.lods[L].walls[facade_types[n]], lengths[n]));
	}

	if (inFloors > 0)
	for (n = 0; n < inPolygon.size(); ++n)
	if (inObj.lods[L].walls[facade_types[n]].roof_slope > 0.0) {
		has_roof = true; break;
	}
	}

	if (has_roof)
	{
		for (n = 0; n < inPolygon.size(); ++n)
		{
			const FacadeWall_t& me = inObj.lods[L].walls[facade_types[n]];
			insets.push_back(tan(me.roof_slope * 3.14159265358979323846 / 180.0) * ((me.t_floors.back().second - me.t_floors.back().first) * me.y_scale));
		}
		InsetPolygon3(inPolygon, &*insets.begin(), 1.0, inObj.is_ring, inUp, inset);
	} else
		inset = inPolygon;

	if (!inObj.lods[L].walls.empty())
	{
		for (n = 0; n < inPolygon.size() - (inObj.is_ring ? 0 : 1); ++n)
		roof = BuildOneFacade(inObj.lods[L].walls[facade_types[n]], inPolygon.side(n), inset.side(n), inFloors, num_panels[n], inUp, has_roof, inObj.lods[L].lod_near, inObj.lods[L].lod_far, inFunc, inRef);
	}

	if (inObj.is_ring && !inObj.lods[L].roof_s.empty())
	{
		float roof_xyz[MAX_FACADE_PTS * 3];
		if (inPolygon.size() > MAX_FACADE_PTS)
			MACIBM_alert(0, "Too many pts in roof polygon", "", "", "", t_exit);

	for (n = 0; n < inPolygon.size(); ++n)
		{
			roof_st[n*2  ] = inObj.lods[L].roof_s[n % inObj.lods[L].roof_s.size()];
			roof_st[n*2+1] = inObj.lods[L].roof_t[n % inObj.lods[L].roof_t.size()];
			Point3 p = inset[inset.size()-n-1] + (inUp * roof);
			roof_xyz[n*3  ] = p.x;
			roof_xyz[n*3+1] = p.y;
			roof_xyz[n*3+2] = p.z;
		}

	switch(inPolygon.size()) {
	case 3:
			inFunc(ext_Poly_Tri, 3, roof_xyz, roof_st, inObj.lods[L].lod_near, inObj.lods[L].lod_far,inRef);
		break;
	case 4:
			inFunc(ext_Poly_Quad, 4, roof_xyz, roof_st, inObj.lods[L].lod_near, inObj.lods[L].lod_far,inRef);
		break;
	default:
			{
				GLUtesselator * tess = gluNewTess();
				gluTessCallback (tess, GLU_TESS_BEGIN, 		(void (OGL_CALL *)()) ExtrudeTessBegin);
				gluTessCallback (tess, GLU_TESS_END, 		(void (OGL_CALL *)()) ExtrudeTessEnd);
				gluTessCallback (tess, GLU_TESS_VERTEX, 	(void (OGL_CALL *)()) ExtrudeTessVertex);
				gExtrudeTessFunc = inFunc;
				gExtrudeTessRef = inRef;
				gExtrudeTessLODNear = inObj.lods[L].lod_near;
				gExtrudeTessLODFar = inObj.lods[L].lod_far;
				gExtrudeTessS1 = inObj.lods[L].roof_s[0];
				gExtrudeTessT1 = inObj.lods[L].roof_t[0];
				gExtrudeTessS2 = inObj.lods[L].roof_s[2 % inObj.lods[L].roof_s.size()] - gExtrudeTessS1;
				gExtrudeTessT2 = inObj.lods[L].roof_t[2 % inObj.lods[L].roof_t.size()] - gExtrudeTessT1;
				gExtrudeX1 = roof_xyz[0  ];
				gExtrudeZ1 = roof_xyz[2  ];
				gExtrudeX2 = roof_xyz[0  ];
				gExtrudeZ2 = roof_xyz[2  ];
				for (n = 1; n < inPolygon.size(); ++n)
				{
					gExtrudeX1 = min(gExtrudeX1,roof_xyz[n*3  ]);
					gExtrudeZ1 = min(gExtrudeZ1,roof_xyz[n*3+2]);
					gExtrudeX2 = max(gExtrudeX2,roof_xyz[n*3  ]);
					gExtrudeZ2 = max(gExtrudeZ2,roof_xyz[n*3+2]);
				}
				gExtrudeX2 -= gExtrudeX1;
				gExtrudeZ2 -= gExtrudeZ1;
				gluBeginPolygon(tess);
				for (n = 0; n < inPolygon.size(); ++n)
				{
					GLdouble	loc[3];
					loc[0] = roof_xyz[n*3  ];
					loc[1] = roof_xyz[n*3+1];
					loc[2] = roof_xyz[n*3+2];
					gluTessVertex(tess, loc, roof_xyz+n*3);
				}
				gluEndPolygon(tess);
				gluDeleteTess(tess);
			}
		break;
	}
}
}

void	BuildFacadeObj(
						const FacadeObj_t&	inObj,
					   const Polygon3& 	 	inPolygon,
					   int					inFloors,
					   const Vector3&		inUp,
					   ExtrudeFunc_f		inFunc,
					   void *				inRef)
{
	inFunc(ext_Start_Obj, 0, NULL, NULL, 0, 0, inRef);

	for (int L = 0; L < inObj.lods.size(); ++L)
	{
		BuildFacadeObjLOD(L, inObj, inPolygon, inFloors, inUp, inFunc, inRef);
	}
	inFunc(ext_Stop_Obj, 0, NULL, NULL, 0, 0, inRef);
}
