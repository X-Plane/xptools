/*
 * Copyright (c) 2017, Laminar Research.
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

#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "XESConstants.h"
#include "MathUtils.h"

#include "ITexMgr.h"
#include "TexUtils.h"
#include "GUI_GraphState.h"

#include "WED_ResourceMgr.h"
#include "WED_FacadePreview.h"
#include "WED_PreviewLayer.h"

static bool cull_obj(const XObj8 * o, double ppm)                   // cut off if laterally smaller than 5 pixels
{
	double pix = ppm * max(o->xyz_max[0] - o->xyz_min[0], o->xyz_max[2] - o->xyz_min[2]);
//	printf("pix %.1lf\n", sqrt(pix));
	return pix < MIN_PIXELS_PREVIEW;
}

static bool closer_to(double a, double b, double x)
{
	return fabs(x-a) > fabs(x-b);
}

static int int_seedrand(int low, int high, int seed)
{
	return (low+high)/2;
}

void	UTL_pick_spelling(
						const vector<UTL_spelling_t>& 	spellings,
						xflt							len,
						UTL_spelling_t&					out_choice,
						xint							seed)
{
	dev_assert(!spellings.empty());

	while(1)
	{
		if(out_choice.total + spellings.back().total < len)
		{
			xint low_choice = intlim(spellings.size() / 4, 0, spellings.size()-1);
			xint high_choice = intlim(spellings.size() * 3 / 4, 0, spellings.size()-1);
			out_choice += spellings[int_seedrand(low_choice,high_choice,seed++)];
		}
		else
		{
			xint best = -1;
			for(xint n = spellings.size() - 1; n >= 0; --n)
			if(closer_to(out_choice.total,out_choice.total + spellings[n].total, len))
			if(best == -1 || closer_to(out_choice.total + spellings[best].total,out_choice.total + spellings[n].total, len))
			{
				best = n;
				if((out_choice.total + spellings[n].total) < len)
					break;
			}
			if(best == -1)
				break;
			out_choice += spellings[best];
			break;
		}
	}
	
	if(out_choice.empty())
		out_choice = spellings[0];	

	dev_assert(out_choice.widths.size() == out_choice.indices.size());
		
}

//void print_wall(const REN_facade_wall_t& w)
//{
//	printf("%f %f %f %f\n", w.min_width,w.max_width, w.min_heading,w.max_heading);
//	for(vector<UTL_spelling_t>::const_iterator s = w.spellings.begin(); s != w.spellings.end(); ++s)
//	{
//		printf("%zd ",distance(w.spellings.begin(),s));
//		for(vector<xbyt>::const_iterator b = s->indices.begin(); b != s->indices.end(); ++b)
//			printf(" %d", *b);
//		printf("\n");
//	}	
//}
inline bool in_heading_in_range(xflt h, xflt h_min, xflt h_max)
{
	if (h_min == h_max) return true;
	
	h = fltwrap(h,0,360);
	h_min = fltwrap(h_min,0,360);
	h_max = fltwrap(h_max,0,360);
	
	if (h_min < h_max) return (h_min <= h && h < h_max);
					   return (h_min <= h || h < h_max);
}

bool	REN_facade_wall_filter_t::is_ok(xflt len, xflt rel_hdg) const
{
	return fltrange(len,min_width, max_width) && in_heading_in_range(rel_hdg, min_heading, max_heading);		
}

bool	REN_facade_wall_filters_t::is_ok(xflt len, xflt rel_hdg) const
{
	for(vector<REN_facade_wall_filter_t>::const_iterator i = filters.begin(); i != filters.end(); ++i)
		if(i->is_ok(len,rel_hdg))
			return true;
	return false;
}

FacadeWall_t::FacadeWall_t() :
	x_scale(0.0),y_scale(0.0),
	roof_slope(0.0),
	left(0), center(0), right(0),	
	bottom(0), middle(0), top(0),
	basement(0.0)
{}

static void vec3f_diff(float * r, float * a, float * b)
{
	for(int i = 0; i<3; ++i)
		r[i] = b[i] - a[i];
}

static float vec3f_dot(float * a, float * b)
{
	xflt res = 0.0;
	for(int i = 0; i<3; ++i)
		res += a[i] * b[i];
	return res;
}

static void expand_pair(xflt& v1, xflt& v2, xflt s)
{
	dev_assert(!isinf(s));
	dev_assert(!isnan(s));
	dev_assert(v1 < v2);
	dev_assert(s >= 1.0);
	if(v1 <= 0.0 && v2 >= 0.0)
	{
		v1 *= s;
		v2 *= s;
	}
	else
	{
		xflt d = (v2-v1) * (s-1.0) * 0.5;
		v1 -= d;
		v2 += d;
	}
	dev_assert(v1 < v2);
}

#if !IBM
#define CALLBACK
#endif

static void CALLBACK TessVertex(const Point2 * p, double * h){ glTexCoord2f((p+1)->x(), (p+1)->y()); glVertex3f(p->x(), *h, p->y());	}

void glPolygon2h(const Point2 * pts, double height, int n)
{
	GLUtesselator * tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))glBegin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))glEnd);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA,	(void (CALLBACK *)(void))TessVertex);

	gluTessBeginPolygon(tess,(void *) &height);
	gluTessBeginContour(tess);
	while(n--)
	{
		double	xyz[3] = { pts->x_, height, pts->y_ };
		gluTessVertex(tess, xyz, (void*) pts);
		pts += 2;
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);
}


typedef void (*RenderQuadFunc) (float *, float *);

static	float	BuildOnePanel(      // return width of this section
						const FacadeWall_t& fac,
						const Segment3& inBase,    // segment of facade polygon
						const Segment3& inRoof,    // same, but 
						const Vector3&	inUp,       // up vector
						int				left,		   // Panel indices
						int				bottom,		// And floor indices
						int				right,
						int				top,
						double			h_start,	// Ratios
						double			v_start,	// Meters
						double			h_end,
						double			v_end,
						bool				use_roof,
						RenderQuadFunc DrawQuad,   // redenering function to draw the quad from coordinate lists
						xint				two_sided,
						xint				doubled)
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

	if(use_roof)
	{
		// If we have a roof, we may have stretching and deforming of the UV horizontal coordinate.
		// Use the bottom of the panel as a 'basis' vector.  Reproject all coordinates along it and
		// reinterp the U coords to span it.  This will 'pull in' the bottom or top, depending on whether
		// the vertex pulls out or in.
		xflt u_basis[3];
		vec3f_diff(u_basis,coords,coords+9);
		xflt u[4] = {
			vec3f_dot(u_basis, coords),
			vec3f_dot(u_basis, coords+3),
			vec3f_dot(u_basis, coords+6),
			vec3f_dot(u_basis, coords+9) };
		xflt u_min = fltmin4(u[0],u[1],u[2],u[3]);
		xflt u_max = fltmax4(u[0],u[1],u[2],u[3]);
		
		texes[0] = interp(u_min,fac.s_panels[left    ].first,u_max,fac.s_panels[right -1].second,u[0]);
		texes[2] = interp(u_min,fac.s_panels[left    ].first,u_max,fac.s_panels[right -1].second,u[1]);
		texes[4] = interp(u_min,fac.s_panels[left    ].first,u_max,fac.s_panels[right -1].second,u[2]);
		texes[6] = interp(u_min,fac.s_panels[left    ].first,u_max,fac.s_panels[right -1].second,u[3]);
			
	}
	DrawQuad(coords, texes);
	if(doubled)
	{
		swap(coords[0],coords[9 ]);
		swap(coords[1],coords[10]);
		swap(coords[2],coords[11]);
		swap(coords[3],coords[6 ]);
		swap(coords[4],coords[7 ]);
		swap(coords[5],coords[8 ]);

		swap(texes[0],texes[6]);
		swap(texes[1],texes[7]);
		swap(texes[2],texes[4]);
		swap(texes[3],texes[5]);
		DrawQuad(coords, texes);
	}
	
	return fac.s_panels[right -1].second - fac.s_panels[left].first;
}


static double BuildOneFacade(                    // that is one wall for one segment
						const FacadeWall_t& fac,
						const Segment3& inBase,
						const Segment3& inRoof,
						int			inFloors,
						int			inPanels,
						const Vector3&	inUp,
						bool			inDoRoofAngle,
						xint			two_sided,
						xint			doubled,
						bool			tex_correct_slope,
					   RenderQuadFunc inFunc)
{
	if (inFloors == 0.0) return 0.0;
	
	// STEP 1: compute exactly how many floors we'll be doing.	
	int left_c, center_c, right_c, bottom_c, middle_c, top_c, ang_c;
	int center_r, middle_r;
	int n, i, j;
	
	if (inDoRoofAngle && fac.top > 0) 
	{
		ang_c = 1; --inFloors;
	} 
	else
		ang_c = 0;
		
	dev_assert(ang_c <= fac.top);
	
	if ((inFloors+ang_c) == (fac.bottom + fac.middle + fac.top))		// user request is EXACT to the facade, agnostic of whether we are angled on top.
	{
		// This optimizes: when we have the EXACT number of floors that the facade has, we don't need ANY cuts.  So we
		// declare the "bottom" to run the whole range.
		// Note that in the angular case inFloors is decremented, and ang_c is 1, so this is STILL correct.
		bottom_c = inFloors; middle_c = 0; top_c = 0;
	} else {
		middle_c = inFloors + ang_c - (fac.bottom + 2 * fac.middle + fac.top);			// This is how many floors we need AFTER we use all of the bottom extended through middle and top extended through middle
		if (middle_c < 0) middle_c = 0;

		bottom_c = min(fac.bottom+fac.middle,(inFloors - middle_c) / 2);				// Divide out the first few extra floors to bottom and top.  TOP has priority!
		top_c = min(fac.top+fac.middle-ang_c,inFloors - middle_c - bottom_c);
		bottom_c = inFloors - middle_c - top_c;
	}
	//printf("  floors = %d, split is %d,%d,%d,%d\n", inFloors,bottom_c,middle_c,top_c,ang_c);
	
	if (inPanels == (fac.left + fac.center + fac.right))
	{
		// Same optimization but...horizontal.  
		left_c = inPanels; right_c = 0; center_c = 0;
	} else {
		center_c = inPanels - (fac.left + 2 * fac.center + fac.right);
		if (center_c < 0) center_c = 0;
		left_c = min(fac.left+fac.center,(inPanels - center_c) / 2);
		right_c = min(fac.center+fac.right,inPanels - center_c - left_c);
		left_c = inPanels - right_c - center_c;
	}

	//printf("   (%d,%d,%d) %d panels: %d,%d,%d\n", fac.left, fac.center, fac.right, inPanels,left_c,center_c,right_c);
		
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
	act_floor_t.push_back(-fac.basement);
	double	total_floor_height = -fac.basement;
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
		// Cosine of angle sets the vertical component, so we can go down with cos > 90, etc.
		if(tex_correct_slope)
			act_floor_t.push_back(total_floor_height + (fac.t_floors[fac.t_floors.size() - 1].second - fac.t_floors[fac.t_floors.size() - 1].first) * cos(fac.roof_slope * DEG_TO_RAD));
		else
			act_floor_t.push_back(total_floor_height + (fac.t_floors[fac.t_floors.size() - 1].second - fac.t_floors[fac.t_floors.size() - 1].first) * 1.0);
		total_floor_height = act_floor_t.back(); }
	// STEP 3: Now is the time on sprockets when we extrude.  Note that we build _one polygon_ for left, right, etc.
	
	int	l, r, t, b, h_count, v_count;

	// EARLY EXIT IF JUST FIGURING OUT THE REAL HEIGHT
	if(inFunc == NULL)
		return total_floor_height * fac.y_scale;

	xflt rat_len = 0.0f;

	if (bottom_c)
	{
		if (left_c)
			rat_len += BuildOnePanel(fac, inBase, inRoof, inUp, 0, 0, left_c, bottom_c, 
							0.0, act_floor_t[0], act_panel_s[left_c], act_floor_t[bottom_c], false, inFunc, two_sided, doubled);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			rat_len += BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, 0, fac.left+h_count, bottom_c, 
							act_panel_s[left_c + l], act_floor_t[0], act_panel_s[left_c + r], act_floor_t[bottom_c], false, inFunc, two_sided, doubled);
		}
		if (right_c)
			rat_len += BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, 0, fac.s_panels.size(), bottom_c,
							act_panel_s[left_c + center_c], act_floor_t[0], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c], false, inFunc, two_sided, doubled);
	}

//	xflt metric_len = fac.x_scale * rat_len;
//	xflt act_len = sqrt(inBase.squared_length());
//	if(!fltrange(metric_len/act_len,0.8,1.25))
//		printf("Scale drift: %f (%f,%f)\n", metric_len/act_len,metric_len,act_len);
	
	for (j = 0; j < middle_r; ++j)
	{
		b = j * fac.middle;
		t = b + fac.middle;
		if (t > middle_c) t = middle_c;
		v_count = t - b;
		
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.bottom, left_c, fac.bottom + v_count, 
							0.0, act_floor_t[bottom_c + b], act_panel_s[left_c], act_floor_t[bottom_c + t], false, inFunc, two_sided, doubled);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = l + fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.bottom, fac.left+h_count, fac.bottom + v_count, 
							act_panel_s[left_c + l], act_floor_t[bottom_c + b], act_panel_s[left_c + r], act_floor_t[bottom_c + t], false, inFunc, two_sided, doubled);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.bottom, fac.s_panels.size(), fac.bottom + v_count,
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + b], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + t], false, inFunc, two_sided, doubled);
	}
	
	if (top_c)
	{
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.t_floors.size() - top_c - ang_c, left_c, fac.t_floors.size() - ang_c, 
							0.0, act_floor_t[bottom_c + middle_c], act_panel_s[left_c], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, two_sided, doubled);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.t_floors.size() - top_c - ang_c, fac.left+h_count, fac.t_floors.size() - ang_c, 
							act_panel_s[left_c + l], act_floor_t[bottom_c + middle_c], act_panel_s[left_c + r], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, two_sided, doubled);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.t_floors.size() - top_c - ang_c, fac.s_panels.size(), fac.t_floors.size() - ang_c,
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + middle_c], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + middle_c + top_c], false, inFunc, two_sided, doubled);
	}
	
	if (ang_c)
	{
		if (left_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, 0, fac.t_floors.size() - ang_c, left_c, fac.t_floors.size(), 
							0.0, act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, two_sided, doubled);
		for (i = 0; i < center_r; ++i)
		{
			l = i * fac.center;
			r = (i+1) * fac.center;
			if (r > center_c) r = center_c;
			h_count = r - l;
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.left, fac.t_floors.size() - ang_c, fac.left+h_count, fac.t_floors.size(), 
							act_panel_s[left_c + l], act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c + r], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, two_sided, doubled);
		}
		if (right_c)
			BuildOnePanel(fac, inBase, inRoof, inUp, fac.s_panels.size() - right_c, fac.t_floors.size() - ang_c, fac.s_panels.size(), fac.t_floors.size(),
							act_panel_s[left_c + center_c], act_floor_t[bottom_c + middle_c + top_c], act_panel_s[left_c + center_c + right_c], act_floor_t[bottom_c + middle_c + top_c + ang_c], true, inFunc, two_sided, doubled);
	}
	return total_floor_height * fac.y_scale;
}


static int PanelsForLength(const FacadeWall_t& wall, double len)
{

//	printf("LEn: %f ", len);
//	double	d = 0.0;
//	for (int n = 0; n < wall.s_panels.size(); ++n)
//	{
//		d += ((wall.s_panels[n].second - wall.s_panels[n].first) * wall.x_scale);		
//	}
//	d /= (double) wall.s_panels.size();
//	int g = (int)((len / d) + 0.5);
//	printf("Estimate: %d ",g);
//	return (g > 0) ? g : 1;

	if(wall.s_panels.empty()) return 0;

	// e.g. if scale is 100 then whole tex is 100 meters tall.
	// So if we need 100 meters we need "1" ST coords worth of stuff.
	float needed_pixels = (len) / wall.x_scale;

	int count = 0;
	float dist = 0.0f;
	//printf("want %f meters (%f pixels), start at %f\n", len, needed_pixels, dist);
	int left = 0;
	int right = 0;
	int left_max = wall.left + wall.center;
	int right_max = wall.right + wall.center;
	
	int i_left = 0;
	int i_right = wall.s_panels.size() - 1;

	float last_dist = dist;	
	while(left < left_max || right < right_max)
	{
		last_dist = dist;
		//printf(" Loop iteration, so far %f, want %f, left=%d,right=%d\n", last_dist, needed_pixels, left, right);
		if((left < left_max && left < right) ||
			right == right_max)
		{
			// insert a left panel
			dist += (wall.s_panels[i_left].second - wall.s_panels[i_left].first);
			//printf(" try left panel %d, takes us to %f\n", i_left, dist);
			if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
			{
				//printf("  we were better at %f, bail with %d\n", last_dist, count);
				return intmax2(count,1);
			}
			++i_left;
			++left;
			++count;
		}
		else
		{
			dist += (wall.s_panels[i_right].second - wall.s_panels[i_right].first);
			//printf(" try right panel %d, takes us to %f\n", i_right, dist);
			if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
			{
				//printf("  we were better at %f, bail with %d\n", last_dist, count);
				return intmax2(count,1);
			}
			--i_right;
			++right;
			++count;
		}
	}
	
	//printf(" So far we have %d, using all left and right, at %f, want %f\n", count, dist, needed_pixels);
	float mid_t_total = 0.0;
	int mid_t_count = 0;
	int end_mid = wall.s_panels.size() - wall.right;
	for(xint i = wall.left; i < end_mid; ++i)
	{
		mid_t_total += (wall.s_panels[i].second - wall.s_panels[i].first);
		++mid_t_count;
	}
		
	if(mid_t_total == 0.0f)
	{
		//printf("  No midel panels at all, bail with %d\n",count);
		return intmax2(count,1);
	}
	float total_mid_reps = floor((needed_pixels - dist) / mid_t_total);
	dist += (total_mid_reps * mid_t_total);
	count += (total_mid_reps * mid_t_count);
	//printf(" Add in %f reps of ALL center for %f.\n", total_mid_reps, dist);
	last_dist = dist;
	for(xint i = wall.left; i < end_mid; ++i)
	{
		last_dist = dist;
		dist += (wall.s_panels[i].second - wall.s_panels[i].first);
		//printf(" Add in center panel %d for %f\n", i, dist);
		if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
		{
			//printf("  we were better at %f, bail with %d\n", last_dist, count);
			return intmax2(count,1);
		}
		++count;		
	}
	//printf(" Ran out of center tiles, end with %d\n", count);
	return count;
}

static void DrawQuad(float * coords, float * texes)
{
	for (int i = 0; i < 4; ++i)
	{
		glTexCoord2fv(&texes[2*i]); glVertex3fv(&coords[3*i]);
	}
}

void height_desc_for_facade(const fac_info_t& info, string& h_decription)
{
	char c[64];
	if(info.is_new)
	{
		vector<int> heights;
		for(auto& f : info.floors)
			if(f.roofs.size())
			{
				heights.push_back(f.roofs.back().roof_height);
			}

		if(heights.size()  > 1)
		{
			sort(heights.begin(), heights.end());

			int last_height = -99;
			bool is_range = false;

			h_decription = "h=";
			for(auto h : heights)
			{
				int this_height = h;
				{
					if(this_height == last_height + 1)
					{
						if(!is_range)
						{
							h_decription += "-";
							is_range = true;
						}
					}
					else
					{
						if(is_range) h_decription += to_string(last_height);
						if(last_height >= 0)	h_decription += ", ";
						h_decription += to_string(this_height);
						is_range = false;
					}
				}
				last_height = this_height;
			}
			if(is_range) h_decription += to_string(last_height);
			 h_decription += "m";
		}
		else if(info.floors.size() && info.floors.front().templates.size())
		{
			snprintf(c,63,"h=%.1fm (fixed)", info.floors.back().templates.front().bounds[1]);
			h_decription = c;
		}
	}
	else if(!info.walls.empty())
	{
		Segment3 dummy_seg;
		double h_min, h_max;
		
		h_min = BuildOneFacade(info.walls[0], dummy_seg, dummy_seg, info.min_floors, 1, Vector3(0,1,0),
							true, xfals, xfals, info.tex_correct_slope, NULL);
		h_max = BuildOneFacade(info.walls[0], dummy_seg, dummy_seg, info.max_floors, 1, Vector3(0,1,0),
							true, xfals, xfals, info.tex_correct_slope, NULL);

		if (info.walls.back().middle && info.min_floors != info.max_floors)
		{
			if(info.max_floors >=999)
				snprintf(c,63,"h=%.0f-∞m", h_min);
			else
				snprintf(c,63,"h=%.0f-%.0fm", h_min, h_max);
		}
		else
			snprintf(c,63,"h=%.1fm (fixed)",(info.walls[0].t_floors.back().second - info.walls[0].t_floors.front().first) * info.walls[0].y_scale);
		h_decription = c;
	}
	
	if(info.scrapers.size())
	{
		snprintf(c,63," +scraper%s %.0f-%.0fm", info.scrapers.size() ? "s" : "", info.scrapers.back().min_agl, info.scrapers.front().max_agl);
		h_decription += c;
	}

}


static int get_floors_for_height(const fac_info_t& fac, double height)
{
	if(fac.min_floors == fac.max_floors) return fac.min_floors;
	
	const FacadeWall_t& wall(fac.walls.front());

	// e.g. if scale is 100 then whole tex is 100 meters tall.
	// So if we need 100 meters we need "1" ST coords worth of stuff.
	float needed_pixels = (height) / wall.y_scale;

	int count = 0;
	float dist = -wall.basement;
	//printf("%s: want %f meters (%f pixels), start at %f\n", fac.debug_rpath(), height, needed_pixels, dist);
	int bot = 0;
	int top = 0;
	int bot_max = wall.bottom + wall.middle;
	int top_max = wall.top + wall.middle;
	
	int i_bot = 0;
	int i_top = wall.t_floors.size() - 1;

	float last_dist = dist;	
	while(bot < bot_max || top < top_max)
	{
		last_dist = dist;
		//printf(" Loop iteration, so far %f, want %f, bot=%d,top=%d\n", last_dist, needed_pixels, bot, top);
		if((bot < bot_max && bot < top) ||
			top == top_max)
		{
			// insert a bottom panel
			dist += (wall.t_floors[i_bot].second - wall.t_floors[i_bot].first);
			//printf(" try bottom panel %d, takes us to %f\n", i_bot, dist);
			if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
			{
				//printf("  we were better at %f, bail with %d\n", last_dist, count);
				return intlim(count,fac.min_floors,fac.max_floors);
			}
			++i_bot;
			++bot;
			++count;
		}
		else
		{
			// insert a top panel
			float scale_factor = 1.0f;
			if(i_top == wall.t_floors.size() - 1)
				scale_factor = cosf(wall.roof_slope * DEG_TO_RAD);
			float panel_height = wall.t_floors[i_top].second - wall.t_floors[i_top].first;
			panel_height *= scale_factor;
			
			if(wall.roof_slope != 0.0f && (i_top == wall.t_floors.size() - 1))
			{
				//printf(" try top panel %d, it is folded over, takes us to %f/%d\n", i_top, dist + panel_height, count+1);
				++count;
				--top_max;
				--i_top;
				dist += panel_height;
			}
			else
			{
				dist += panel_height;
				//printf(" try top panel %d, takes us to %f\n", i_top, dist);
				if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
				{
					//printf("  we were better at %f, bail with %d\n", last_dist, count);
					return intlim(count,fac.min_floors,fac.max_floors);
				}
				--i_top;
				++top;
				++count;
			}
		}
	}
	
	//printf(" So far we have %d, using all bottom and top, at %f, want %f\n", count, dist, needed_pixels);
	float mid_t_total = 0.0;
	int mid_t_count = 0;
	int end_mid = wall.t_floors.size() - wall.top;
	for(xint i = wall.bottom; i < end_mid; ++i)
	{
		mid_t_total += (wall.t_floors[i].second - wall.t_floors[i].first);
		++mid_t_count;
	}
		
	if(mid_t_total == 0.0f)
	{
		//printf("  No midel panels at all, bail with %d\n",count);
		return intlim(count,fac.min_floors,fac.max_floors);
	}
	float total_mid_reps = floor((needed_pixels - dist) / mid_t_total);	
	count += total_mid_reps * mid_t_count;
	dist += (total_mid_reps * mid_t_total);
	//printf(" Add in %f reps of ALL middle for %f.\n", total_mid_reps, dist);
	last_dist = dist;
	for(xint i = wall.bottom; i < end_mid; ++i)
	{
		last_dist = dist;
		dist += (wall.t_floors[i].second - wall.t_floors[i].first);
		//printf(" Add in middle panel %d for %f\n", i, dist);
		if(!closer_to(last_dist,dist,needed_pixels) && count > 0)
		{
			//printf("  we were better at %f, bail with %d\n", last_dist, count);
			return intlim(count,fac.min_floors,fac.max_floors);
		}
		++count;		
	}
	//printf(" Ran out of middle tiles, end with %d\n", count);
	return count;
		
}

struct obj {
	int 	idx;                 // index to type 2 objects
	float	x,y,z,r;
};


void draw_facade(ITexMgr * tman, WED_ResourceMgr * rman, const string& vpath, const fac_info_t& info, const Polygon2& footprint, const vector<int>& choices,
	double fac_height, GUI_GraphState * g, bool want_thinWalls, double ppm_for_culling)
{
	for(auto f : info.scrapers)
	{
		if(fltrange(fac_height,f.min_agl,f.max_agl))
		{
			fac_height = (fac_height - f.min_agl) / f.step_agl;
			double hgt = fac_height * f.step_agl;
			string scp_base(f.choices[0].base_obj);
			if(!scp_base.empty())
			{
				const XObj8 * oo;
				if(rman->GetObjRelative(scp_base, vpath, oo))
				{
					draw_obj_at_xyz(tman, oo,
						f.choices[0].base_xzr[0], hgt, f.choices[0].base_xzr[1],
						f.choices[0].base_xzr[2]-90, g);
				} 
			}
			string scp_twr(f.choices[0].towr_obj);
			if(!scp_twr.empty())
			{
				const XObj8 * oo;
				if(rman->GetObjRelative(scp_twr, vpath, oo))
				{
					draw_obj_at_xyz(tman, oo,
						f.choices[0].towr_xzr[0], hgt, f.choices[0].towr_xzr[1],
						f.choices[0].towr_xzr[2]-90, g);
				} 
			}
			break;
		}
	}

	TexRef	tRef = tman->LookupTexture(info.wall_tex.c_str() ,true, tex_Compress_Ok);
	g->SetTexUnits(1);
	g->BindTex(tRef  ? tman->GetTexID(tRef) : 0, 0);
	
	const REN_facade_floor_t * bestFloor = nullptr;
	vector<obj>		obj_locs;
	
	Polygon2			roof_pts;
	double			roof_height;
	Bbox2				roof_extent;
	
	if(info.two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
		
	int n_wall = info.is_ring ? footprint.size() : footprint.size()-1;

	if(!info.is_new)  // type1 facades
	{
		if(info.walls.empty())	return;
		
		int floors = get_floors_for_height(info, fac_height);
		vector<float> insets;
		insets.reserve(footprint.size());
		
		if(want_thinWalls)                            // Todo: Still render sloped roof segments
		for(int n = 0; n < footprint.size(); ++n)
		{
			const FacadeWall_t& me = info.walls[intmin2(info.walls.size()-1,choices[n])];
			if(fabs(me.roof_slope) > 1.0)
				insets.push_back(info.tex_correct_slope ?
				sin(me.roof_slope * DEG_TO_RAD) * ((me.t_floors.back().second - me.t_floors.back().first) * me.y_scale) :
				tan(me.roof_slope * DEG_TO_RAD) * ((me.t_floors.back().second - me.t_floors.back().first) * me.y_scale));
		}
		
		if(insets.size())
		{
			Segment2 prevSeg(footprint.side(n_wall-1));
			Vector2 prevDir(prevSeg.p1, prevSeg.p2);
			prevDir.normalize();
			prevDir = prevDir.perpendicular_cw();
			prevSeg += prevDir * insets[n_wall-1];
			
			Point2 pt;
			for (int w = 0; w < footprint.size(); ++w)
			{
				Segment2 thisSeg(footprint.side(w));
				Vector2 thisDir(thisSeg.p1, thisSeg.p2);
				thisDir.normalize();
				thisDir = thisDir.perpendicular_cw();
				thisSeg += thisDir * insets[w];
				prevSeg.intersect(thisSeg, pt);
				roof_pts.push_back(pt);
				roof_extent += pt;
				prevSeg = thisSeg;
			}
		}
		else
			roof_pts = footprint;
		
		if(want_thinWalls)
		{
			glBegin(GL_QUADS);
			for(int w = 0; w < n_wall; ++w)
			{
				const FacadeWall_t * me = &info.walls[intmin2(info.walls.size()-1,choices[w])];

				Segment2 inBase(footprint.side(w));
				double seg_length = sqrt(inBase.squared_length());
				Segment2 inRoof(roof_pts.side(w));

				int wall_panels = PanelsForLength(*me, seg_length);
				Segment3 inBase3(Point3(inBase.p1.x(),0,inBase.p1.y()), Point3(inBase.p2.x(),0,inBase.p2.y()));
				Segment3 inRoof3(Point3(inRoof.p1.x(),0,inRoof.p1.y()), Point3(inRoof.p2.x(),0,inRoof.p2.y()));

				roof_height = BuildOneFacade(*me, inBase3, inRoof3, floors, wall_panels, Vector3(0,1,0),
							info.has_roof, info.two_sided, info.doubled, info.tex_correct_slope, DrawQuad);
			}
			glEnd();
		}
		else
		{
			Segment3 dummy_seg;
			roof_height = BuildOneFacade(info.walls[0], dummy_seg, dummy_seg, floors, 1, Vector3(0,1,0), 
							true, xfals, xfals, info.tex_correct_slope, NULL);
		}
	}
	else // type 2 facades
	{
		Vector2 miter;
		roof_height =  -9.9e9;

		for(auto& f : info.floors)
		{
			double h = f.max_roof_height();
			if(closer_to(roof_height, h, fac_height))
			{
				roof_height = h;
				bestFloor = &f;
			}
		}
		if(!bestFloor || bestFloor->walls.empty()) return;

		glBegin(GL_TRIANGLES);
		for(int w = 0; w < n_wall; ++w)
		{
			Segment2 inBase(footprint.side(w));
			Vector2 segDir(inBase.p1, inBase.p2);
			double seg_length = segDir.normalize();
			Vector2 perpDir(segDir.perpendicular_cw());

			const REN_facade_wall_t * bestWall = &bestFloor->walls[intmin2(bestFloor->walls.size()-1, choices[w])];
			UTL_spelling_t our_choice;
			UTL_pick_spelling(bestWall->spellings, seg_length, our_choice, 0);
			seg_length /= our_choice.total;

			Point2 thisPt = footprint[w];
			roof_pts.push_back(thisPt);
			roof_extent += thisPt;

			Vector2 corrDir_first,corrDir_last;
			{
				Vector2 tangentDir;
				if(w == 0)
					if(info.is_ring)
					{
						inBase = footprint.side(n_wall-1);
						tangentDir = Vector2(inBase.p1, inBase.p2);
						tangentDir.normalize();
						tangentDir += segDir;
						tangentDir.normalize();
						miter = tangentDir.perpendicular_ccw();
						miter /= miter.dot(perpDir);
					}
					else
						miter =perpDir;
				corrDir_first = perpDir - miter;
				if (info.is_ring || w < n_wall -1) 
				{
					inBase = footprint.side(w < n_wall-1 ? w+1 : 0);
					tangentDir = Vector2(inBase.p1, inBase.p2);
					tangentDir.normalize();
					tangentDir += segDir;
					tangentDir.normalize();
					miter = tangentDir.perpendicular_ccw();
					miter /= miter.dot(perpDir);
					corrDir_last = perpDir - miter;
				}
				else
					corrDir_last = Vector2();
			}

			int first = 1;
//	printf("start\n");
			for(auto ch : our_choice.indices)
			{
				const REN_facade_template_t& t = bestFloor->templates[ch];
				double segMult = seg_length * t.bounds[2];

//	printf("%.1f %.1f %.1f\n",t.bounds[0], t.bounds[1], t.bounds[2]);
				if(!info.nowallmesh && (want_thinWalls || (info.has_roof && t.bounds[1] > 0.5) || (!info.is_ring && t.bounds[0] > 0.5)))
				for(auto m : t.meshes) // all meshes == maximum LOD detail
				{
					for(auto ind : m.idx)
					{
						Point2 xy = thisPt - perpDir * m.xyz[3*ind] - segDir * m.xyz[3*ind+2] * segMult;
						if(first == 1)
							xy += corrDir_first * (m.xyz[3*ind+2]+1) * m.xyz[3*ind];
						if(first == our_choice.indices.size())
							xy -=  corrDir_last * (m.xyz[3*ind+2]+0) * m.xyz[3*ind];
						glTexCoord2fv(&m.uv[2*ind]); glVertex3f(xy.x(), m.xyz[3*ind+1], xy.y());
					}
				}
				first++;
				for(auto o: t.objs)
				{
					struct obj obj_ref;
					obj_ref.idx = o.idx;
					Point2 xy = thisPt - perpDir * o.xyzr[0] - segDir * o.xyzr[2]  * seg_length;

					obj_ref.x = xy.x();
					obj_ref.y = o.xyzr[1];
					obj_ref.z = xy.y();
					obj_ref.r = o.xyzr[3] + atan2(perpDir.y(), perpDir.x()) * RAD_TO_DEG - 180.0;
					obj_locs.push_back(obj_ref);
				}
				thisPt += segDir * segMult;
			}
			corrDir_first = corrDir_last;
		}
		glEnd();
		if(info.has_roof && !info.is_ring)
			roof_pts.push_back(footprint.back());    // we didn't process the last wall - but still need a complete roof. E.g. Fenced Parking Facades.
	}

	if (info.has_roof) // && want_roof
	{
		tRef = tman->LookupTexture(info.roof_tex.c_str() ,true, tex_Wrap|tex_Compress_Ok);
		g->BindTex(tRef ? tman->GetTexID(tRef) : 0, 0);

		// all facdes are drawn cw (!)
		glCullFace(GL_FRONT); 

		if(!info.roof_s.empty() && roof_pts.size() < 5)
		{
			glBegin(GL_POLYGON);
			for (int n = 0; n < roof_pts.size(); ++n)
			{
				glTexCoord2f(info.roof_s[n % info.roof_s.size()],
								 info.roof_t[n % info.roof_t.size()]);
				glVertex3f(roof_pts[n].x(), roof_height, roof_pts[n].y());
			}
			glEnd();
		}
		else if(!info.roof_s.empty())
		{
			glBegin(GL_POLYGON);
			for (int n = 0; n < roof_pts.size(); ++n)
			{
				glTexCoord2f(interp(roof_extent.xmin(), info.roof_s[0], roof_extent.xmax(), info.roof_s[2 % info.roof_s.size()], roof_pts[n].x()),
								 interp(roof_extent.ymin(), info.roof_t[0], roof_extent.ymax(), info.roof_t[2 % info.roof_t.size()], roof_pts[n].y()));
				glVertex3f(roof_pts[n].x(), roof_height, roof_pts[n].y());
			}
			glEnd();
		}
		else
		{
			// establish directions in facade-aligned meter space
			Vector2 dirVec(footprint[0], footprint[1]);
			dirVec.normalize();
			Vector2 perpVec(dirVec.perpendicular_cw());
			
			Vector2 xz_centroid(footprint.side(0).midpoint());
			double dirDot = -dirVec.dot(xz_centroid);
			double perpDot = -perpVec.dot(xz_centroid);
			
			if(!info.is_new)  // type 1 facades, new UV mapping algo for Alex
			{
				// This effectively finds the bounds of the polygon in facade-aligned meter space.
				Bbox2 ab;
				for(auto fp : footprint)
				{
					ab += Point2(dirVec.dot(Vector2(fp)) + dirDot,
									 dirVec.dot(Vector2(fp)) + perpDot);
				}

				xflt ab_use[4] = { (xflt) ab.xmin(), (xflt) ab.ymin(), (xflt) ab.xmax(), (xflt) ab.ymax() };

				if(!ab.is_empty())		// safety check for degenerate single-point facade.
				{
					xflt ab_ideal = (info.roof_ab[2] - info.roof_ab[0]) / (info.roof_ab[3] - info.roof_ab[1]);
					xflt ab_now = (ab_use[2] - ab_use[0]) / (ab_use[3] - ab_use[1]);
					if(ab_ideal > ab_now)
					{
						// We were wide but now we are deep.  Get wider.
						expand_pair(ab_use[0],ab_use[2], ab_ideal / ab_now);
					}
					else if (ab_ideal < ab_now)
					{
						// We were deep but now are wide.  Get deeper.
						expand_pair(ab_use[1],ab_use[3], ab_now / ab_ideal);
					}
				}

				if((ab_use[2] - ab_use[0]) < (info.roof_ab[2] - info.roof_ab[0]))
				if(ab_use[2] != ab_use[0])	// safety check for degenerate single-point facade.
				if(ab_use[3] != ab_use[1])	// f--- if I know why they would happen but let's not NaN out.
				{
					xflt scale = (info.roof_ab[2] - info.roof_ab[0]) / (ab_use[2] - ab_use[0]);
					expand_pair(ab_use[0],ab_use[2], scale);
					expand_pair(ab_use[1],ab_use[3], scale);
				}

				xflt x = ab.xmin();               // this is bizarre: the DevAssert macro itself creates a segmentation violation ...
				dev_assert(x >= ab_use[0]);
//				dev_assert(ab.xmin() >= ab_use[0]);  // ab coordinates to actually use are larger than bounding box
				x = ab.ymin();
				dev_assert(x >= ab_use[1]);
//				dev_assert(ab.ymin() >= ab_use[1]);
				x = ab.xmax();
				dev_assert(x <= ab_use[2]);
//				dev_assert(ab.xmax() <= ab_use[2]);
				x = ab.ymax();	
				dev_assert(x <= ab_use[3]);	
//				dev_assert(ab.ymax() <= ab_use[3]);	

				vector<Point2> new_pts; new_pts.reserve(roof_pts.size()*2);
				for(auto p : roof_pts)
				{
					new_pts.push_back(p);
					new_pts.push_back(Point2(interp(ab_use[0], info.roof_st[0], ab_use[2], info.roof_st[2], dirVec.dot(Vector2(p))  + dirDot ),
					                         interp(ab_use[1], info.roof_st[1], ab_use[3], info.roof_st[3], perpVec.dot(Vector2(p)) + perpDot)));
				}
				glPolygon2h(new_pts.data(), roof_height, roof_pts.size());
			}
			else if(!info.noroofmesh)  // type 2 facades
			{
				if(want_thinWalls && roof_pts.size() <= 5) // add roof objects, but only for preview pane. Its slow when taking the exact shape of the roof into account.
				{
					for(auto ro : bestFloor->roofs.back().roof_objs)
					{
						Point2 loc0 = footprint.side(0).midpoint();
						Point2 loc_uv = loc0 + dirVec * ro.str[0] * info.roof_scale_s + perpVec * ro.str[1] * info.roof_scale_t;

						for(int s = (footprint[0].x() - loc0.x()) / info.roof_scale_s; s < ((footprint[1].x() - loc0.x()) / info.roof_scale_s) - 1; ++s)
							for(int t = ((-footprint[0].y() - loc0.y()) / info.roof_scale_t) + 1; t < ((-footprint[2].y() - loc0.y()) / info.roof_scale_t) + 1; ++t)
							{
								struct obj obj_ref;
								Point2 xy = loc_uv + dirVec * s * info.roof_scale_s - perpVec * t * info.roof_scale_t;
								obj_ref.x = xy.x();
								obj_ref.y = roof_height;
								obj_ref.z = xy.y();
								obj_ref.r = ro.str[2] + atan2(perpVec.y(), perpVec.x()) * RAD_TO_DEG + 90.0;
								obj_ref.idx = ro.obj;
								obj_locs.push_back(obj_ref);
							}
					}
				}
				int xtra_roofs = 0;
				if (want_thinWalls && bestFloor->roofs.size() > 1) xtra_roofs = bestFloor->roofs.size() - 1;
				do
				{
					vector<Point2> new_pts; new_pts.reserve(roof_pts.size()*2);
					for(auto p: roof_pts)
					{
						new_pts.push_back(p);
						new_pts.push_back(Point2( (dirVec.dot(Vector2(p))   + dirDot)  / info.roof_scale_s,
						                           (perpVec.dot(Vector2(p)) + perpDot) / info.roof_scale_t));
					}
					glPolygon2h(new_pts.data(), roof_height, roof_pts.size());
					
					xtra_roofs--;
					if(xtra_roofs >= 0)
						roof_height = bestFloor->roofs[xtra_roofs].roof_height;
				}
				while (xtra_roofs >=0);
			}
		}
		glCullFace(GL_BACK);
	}

	for(auto l : obj_locs)
	{
		const XObj8 * oo(info.xobjs[l.idx]);
		if(oo && !cull_obj(oo, ppm_for_culling))
		{
			// facade is aligned so midpoint of first wall is origin
			draw_obj_at_xyz(tman, oo,
				l.x, l.y, l.z,
				l.r, g);
		} 
	}
	
}

