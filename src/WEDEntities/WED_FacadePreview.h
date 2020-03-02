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

#ifndef WED_FacadePreview_H
#define WED_FacadePreview_H

/******** from REN_facade.h *******/

struct  XObj8;
struct fac_info_t;

typedef int xint;
typedef float xflt;
typedef unsigned char 	xbyt;
#define xtrue			true
#define xfals			false

#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#define dev_assert(x) DebugAssert(x)
#include "AssertUtils.h"
#include "MathUtils.h"

/****************************************************************************************************************
 * SPELLING
 ****************************************************************************************************************/

// Given a bunch of tiles (up to 255), each of a different width...a spelling is a set of tile indices.  This lets
// us rearrange art assets.  Our spelling structure has:
// 1. A vector of tile indices.
// 2. A vector of the width of the pick for each tile. (if indices[a] == indices[b] then widths[a] == widths[b])
// 3. a total for the entire spelling, which is the sum of all elements in widths
// The += operator concatenates in a spelling and updates widths.
 
struct UTL_spelling_t {
	vector<xbyt>					indices;
	vector<xflt>					widths ;
	xflt							total  ;
	UTL_spelling_t() { total = 0.0f; }
	
	void clear() { indices.clear(); widths.clear(); total = 0.0f; }
	bool empty() const { return indices.empty(); }
	bool operator<(const UTL_spelling_t& rhs) const { 
		dev_assert(indices.size() == widths.size());
		dev_assert(rhs.indices.size() == rhs.widths.size());
		return total < rhs.total; 
	}
	
	UTL_spelling_t& operator+=(const UTL_spelling_t& rhs) {
		dev_assert(indices.size() == widths.size());
		dev_assert(rhs.indices.size() == rhs.widths.size());
		indices.insert(indices.end(),rhs.indices.begin(),rhs.indices.end());
		widths.insert(widths.end(),rhs.widths.begin(),rhs.widths.end());
		dev_assert(indices.size() == widths.size());
		total += rhs.total;
		return *this;
	}
};

class ITexMgr;
class WED_ResourceMgr;
class GUI_GraphState;

/****************************************************************************************************
 * NEW V2 FACADE GUNK
 ****************************************************************************************************/

// Wall template - this is our minimum "pourable" entity.  The object is stored in "normalized" coordinates - that is, the
// length of the wall goes from z=0 to z=-1, and the height from y=0 to y=1.  The wall's X is x=0 is on the wall, x=1 is outside
// and x=-1 is inside.  The units for X match Z - that is, if the wall's "natural" length is 20 meters, X = 20 is a full 20 meters
// outside the wall.  
// 
// To extrude, we "pour" the object into 2 or more rectilinear frames.  The format of the frame is four vertices: lower wall, lower outside,
// upper outside, upper wall, where the width of the frames matches the length of the overall segment (path-wise).  
//
// When we pour, a positive tangent means to advance the OUTSIDE elements closer to the TARGET of the wall - 1.0 means advance as much
// (along -z) as we have X.  So a square with beveled edges would have each wall have a tangent value of -1, 1 respectively.
//
// Normal vectors are calculated during pouring by passing in the original linear/bezier path for the floor, and we always of course pass "up"
// to give an extrusion direction.  

struct REN_facade_wall_filter_t {
	REN_facade_wall_filter_t() : min_width(), max_width(0), min_heading(0), max_heading(0) { }
	xflt							min_width;
	xflt							max_width;
	double							min_heading;
	double							max_heading;
	bool	is_ok(xflt len, xflt rel_hdg) const;
};

struct REN_facade_wall_filters_t {
	vector<REN_facade_wall_filter_t>	filters;
	bool	is_ok(xflt len, xflt rel_hdg) const;
};
	
struct REN_facade_template_t {
	struct obj {
		xint		idx;
		xflt		xyzr[4];
//		asset_freq	freq;		
	};
	struct mesh {
		vector<xflt> 	xyz;
		vector<xflt> 	uv;      // 5 floats per vertex, skipping normals
		vector<xint>	idx;
	};

	vector<obj>			objs;
	vector<mesh>		meshes;		// Each mesh within this template.
	xflt					bounds[3];
};


struct REN_facade_wall_t : public REN_facade_wall_filters_t {
	vector<UTL_spelling_t>			spellings;
};

struct REN_facade_roof_t {
	xflt							roof_height;
	xint							two_sided;
	struct robj {
		xflt		str[3];
		xint		obj;
//		asset_freq	freq;
	};
	vector<robj>			roof_objs;
	REN_facade_roof_t(xflt h=0.0) : roof_height(h), two_sided(xfals) { }
};	

struct REN_facade_floor_t {
	string							name;          // not really usedfor anything
	vector<REN_facade_template_t>	templates;
//	vector<REN_facade_template_t>	templates_curved;
//	vector<xint>					groups;		// sorted list of group IDs used by ANY piece of ANY wall.  Used to plan iteration around the floor multiple times.
	xint							roof_surface;
	vector<REN_facade_wall_t>		walls;
	vector<REN_facade_roof_t>		roofs;
	inline xflt						max_roof_height(void) const { return roofs.empty() ? 0.0 : roofs.back().roof_height; }
};

/****************************************************************************************************
 * OLD SCHOOL V1 FACADE PIECES
 ****************************************************************************************************/

struct	FacadeWall_t { // : public REN_facade_wall_filters_t {

	FacadeWall_t();

	double			x_scale;	// From tex to meters
	double			y_scale;
	float				basement;
	double			roof_slope;
	
	vector<pair<float, float> >		s_panels;
	int								left;
	int								center;
	int								right;
	vector<pair<float, float> >		t_floors;
	int								bottom;
	int								middle;
	int								top;
};	

// Facade scraper - defines how to combine an OBJ + facade into a two-part sky-scraper.
// Note: we will encode scraper floors as (1+scraper IDX) * 256 + facade floors
// If we have a scraper idx > 0 (e.g. floors > 256) then floors is the floors of the BIG 
// building in steps up from min AGL.

struct REN_facade_scraper_t {

struct tower_t 
	{
		tower_t() 
		{
			for(int i = 0; i < 3; ++i)
				base_xzr[i] = towr_xzr[i] = 0.0;
		}
		string			base_obj;			// OBJ to be used
		string			towr_obj;			// tower object - can be empty range if base-only scraper -- it happens!
		xflt				base_xzr[3];
		xflt				towr_xzr[3];
	};

//	vector<obj_ref>				assets;
	vector<tower_t>		choices;
	xflt						min_agl;			// range of AGL where this scraper rule applies
	xflt						max_agl;
	xflt						step_agl;		// Step from min AGL up for height
	xint						floors;			// number of floors to use for facade base
};

struct	REN_FacadeLOD_t {

	bool					tex_correct_slope;
	vector<FacadeWall_t>	walls;
	vector<double>			roof_s;
	vector<double>			roof_t;
	float					roof_st[4];
	float					roof_ab[4];
	bool					has_roof;
	bool					doubled;
	int					min_floors;	      // new in 10.20 - for floor count determination, this clamps the floor range.
	int					max_floors;

};

/**********************/


void draw_facade(ITexMgr * tman, WED_ResourceMgr * rman, const string& vpath, const fac_info_t& info, const Polygon2& footprint, const vector<int>& choices, 
	double fac_height, GUI_GraphState * g, bool want_thinWalls, double ppm_for_culling = 100);
	
void height_desc_for_facade(const fac_info_t& info, string& h_decription);

#endif