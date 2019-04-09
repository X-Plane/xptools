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

static bool closer_to(double x, double a, double b)
{
	return abs(x-a) < abs(x-b);
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


static double	BuildOneFacade(                    // that is one wall for one segment
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

#include "WED_PreviewLayer.h"

struct obj {
	int 	idx;                 // index to type 2 objects
	float	x,y,z,r;
};

void draw_facade(ITexMgr * tman, WED_ResourceMgr * rman, const string& vpath, const fac_info_t& info, const Polygon2& footprint, const vector<int>& choices, double fac_height, GUI_GraphState * g)
{
	TexRef	tRef = tman->LookupTexture(info.wall_tex.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);			
	g->SetTexUnits(1);
	g->BindTex(tRef  ? tman->GetTexID(tRef) : 0, 0);
	
	const REN_facade_floor_t * bestFloor;
	vector<Point2>   roof_pts;
	double           roof_height;
	vector<obj>      obj_locs;
	
	if(info.two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
		
	int n_wall = info.is_ring ? footprint.size() : footprint.size()-1;

	if(!info.is_new)
	{
		if (info.walls.empty())	return;
		fac_height=intlim(fac_height,info.min_floors,info.max_floors);
		double insets[footprint.size()];
		for (int n = 0; n < footprint.size(); ++n)
		{
			const FacadeWall_t& me = info.walls[intmin2(info.walls.size()-1,choices[n])];
			insets[n] = info.tex_correct_slope ?
				sin(me.roof_slope * DEG_TO_RAD) * ((me.t_floors.back().second - me.t_floors.back().first) * me.y_scale) :
				tan(me.roof_slope * DEG_TO_RAD) * ((me.t_floors.back().second - me.t_floors.back().first) * me.y_scale) ;
		}
		glBegin(GL_QUADS);
		for (int w = 0; w < n_wall; ++w)
		{
			const FacadeWall_t * me = &info.walls[intmin2(info.walls.size()-1,choices[w])];
			
			Segment2 inBase(footprint.side(w));
			Vector2 seg_dir(inBase.p1, inBase.p2);
			double seg_length = sqrt(seg_dir.squared_length());
			seg_dir.normalize();

			Segment2 inRoof(inBase);
			inRoof += seg_dir.perpendicular_cw() * insets[w];
			inRoof.p1 += seg_dir * insets[w==0 ? footprint.size()-1: w-1];  // only valid if walls are at right angles
			inRoof.p2 -= seg_dir * insets[w==footprint.size()-1 ? 0: w+1];
			
			int wall_panels = PanelsForLength(*me, seg_length);
			roof_pts.push_back(inRoof.p1);

			Segment3 inBase3(Point3(inBase.p1.x(),0,inBase.p1.y()),Point3(inBase.p2.x(),0,inBase.p2.y()));
			Segment3 inRoof3(Point3(inRoof.p1.x(),0,inRoof.p1.y()),Point3(inRoof.p2.x(),0,inRoof.p2.y()));

			roof_height = BuildOneFacade(*me, inBase3, inRoof3, fac_height, wall_panels, Vector3(0,1,0),
						info.has_roof, info.two_sided, info.doubled, info.tex_correct_slope, DrawQuad);
		}
		glEnd();
	}
	else
	{
		if (info.floors.empty() || info.floors.front().walls.empty()) return;
		bestFloor = &info.floors.front();
		for(auto& f : info.floors)
			if(f.max_roof_height() < fac_height)
				bestFloor = &f;
		roof_height = bestFloor->max_roof_height();
		
		glBegin(GL_TRIANGLES);
		for (int w = 0; w < n_wall; ++w)
		{
			Segment2 inBase(footprint.side(w));
			Vector2 seg_dir(inBase.p1, inBase.p2);
			double seg_length = sqrt(seg_dir.squared_length());
			seg_dir.normalize();
			Vector2 dir_z(seg_dir.perpendicular_cw());
			
			const REN_facade_wall_t * bestWall = &bestFloor->walls[intmin2(bestFloor->walls.size()-1, choices[w])];
			UTL_spelling_t our_choice;
			UTL_pick_spelling(bestWall->spellings, seg_length, our_choice, 0);
			seg_dir *= seg_length / our_choice.total;
			
			Point2 thisPt = footprint[w];
			roof_pts.push_back(thisPt);
			
	//		glPushMatrix();
	//		glTranslatef(thisPt.x(),0,thisPt.y());
	//		glRotate(,0,1,0);
	//		glScalef(1,1,seg_length / our_choice.total);

			for(int i = 0; i < our_choice.indices.size(); i++)
			{
	
				const REN_facade_template_t& t = bestFloor->templates[our_choice.indices[i]];
				for(auto m : t.meshes) // all meshes == maximum LOD detail
				{
/*					glVertexPointer(3, GL_FLOAT, 0, m.xyz.data());
					glTexCoordPointer(2, GL_FLOAT, 0, m.uv.data());
					
					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glDisableClientState(GL_COLOR_ARRAY);
					glDisableClientState(GL_NORMAL_ARRAY);

					glDrawElements(GL_TRIANGLES, m.idx.size(), GL_UNSIGNED_INT, m.idx.data());		*/
					
					for(auto ind : m.idx)
					{
						Point2 xy = thisPt - dir_z * m.xyz[3*ind] - seg_dir * m.xyz[3*ind+2];
						glTexCoord2fv(&m.uv[2*ind]); glVertex3f(xy.x(), m.xyz[3*ind+1], xy.y());
					}
				}
				for(auto o: t.objs)
				{
					struct obj obj_ref;
					obj_ref.idx = o.idx;
					Point2 xy = thisPt - dir_z * o.xyzr[0] - seg_dir * o.xyzr[2];

					obj_ref.x = xy.x();
					obj_ref.y = o.xyzr[1];
					obj_ref.z = xy.y();
					obj_ref.r = o.xyzr[3] + atan2(dir_z.y(), dir_z.x()) * RAD_TO_DEG - 180.0;
					obj_locs.push_back(obj_ref);
				}
				thisPt += seg_dir * t.bounds[2];
			}
	//		glPopMatrix();
		}
		glEnd();
	}
	
	if (info.has_roof)
	{
		double fac_width = sqrt(Vector2(roof_pts[0], roof_pts[1]).squared_length());
		
		tRef = tman->LookupTexture(info.roof_tex.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);
		g->BindTex(tRef  ? tman->GetTexID(tRef) : 0, 0);
		
		float s_roof[2] = { 0.0, 1.0 };
		float t_roof[2] = { 0.0, 1.0 };
		
		if(!info.is_new)
		{
			if(info.roof_s.size() == 4)
			{
				s_roof[0] = info.roof_s[0];	t_roof[0] = info.roof_t[0];
				s_roof[1] = info.roof_s[2];	t_roof[1] = info.roof_t[2];
			}
			else
			{
				s_roof[0] = info.roof_st[0];
				t_roof[0] = info.roof_st[1];
				s_roof[1] = info.roof_st[0] + (info.roof_st[2] - info.roof_st[0]) * min(1.0,fac_width / (info.roof_ab[2] - info.roof_ab[0]));
				t_roof[1] = info.roof_st[1] + (info.roof_st[3] - info.roof_st[1]) * min(1.0,fac_width / (info.roof_ab[2] - info.roof_ab[0]));
			}
		}
		else
		{
			s_roof[1] = fac_width / info.roof_scale_s;
			t_roof[1] = fac_width / info.roof_scale_t;
		}
		
		int xtra_roofs = 0;
		if (info.is_new && bestFloor->roofs.size() > 1) xtra_roofs = bestFloor->roofs.size() - 1;
		
		glBegin(GL_QUADS);
		if(roof_pts.size() == 4)   // need2draw propper polygon
		do
		{
			float pts[12];
			float tex[8];
			for (int n = 0; n < roof_pts.size(); ++n)
			{
				int x = n == 0 || n == 3;
				int z = n < 2;
				
				pts[3*n  ] = roof_pts[3-n].x();
				pts[3*n+1] = roof_height;
				pts[3*n+2] = roof_pts[3-n].y();
				
				tex[2*n  ] = s_roof[x];
				tex[2*n+1] = t_roof[z];
			}
			DrawQuad(pts, tex);
			
			xtra_roofs--;
			if(xtra_roofs >= 0)
				roof_height = bestFloor->roofs[xtra_roofs].roof_height;
		}
		while (xtra_roofs >=0);
		glEnd();
	}

	for(auto l : obj_locs)
	{
		const XObj8 * oo;
		if(rman->GetObjRelative(info.objs[l.idx].c_str(), vpath, oo))
		{
			// facade is aligned so midpoint of first wall is origin
			draw_obj_at_xyz(tman, oo,
				l.x, l.y, l.z,
				l.r, g);
		} 
	}
	
	for(auto f : info.scrapers)
	{
		if(fltrange(fac_height,f.min_agl,f.max_agl))
		{
			int floors = (fac_height - f.min_agl) / f.step_agl;
			double hgt = floors * f.step_agl;
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
}

