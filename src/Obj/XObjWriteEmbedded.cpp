/*
 *  XObjWriteEmbedded.cp
 *  SceneryTools
 *
 *  Created by bsupnik on 8/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XObjWriteEmbedded.h"
#include "AssertUtils.h"
#include <math.h>
#include <algorithm>
#include "MathUtils.h"
#include "MatrixUtils.h"
using std::max;

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// BOUNDING SPHERES
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// This routine calculates a bounding sphere from point data.  The bounding sphere isn't the SMALLEST one containing all the points, but it is very close, and still calculates
// fast.  Since we use  bounding spheres to cull, having small bounding spheres is good.  (This bounding sphere is better than doing a pythag on the bounding cube.  In other words,
// this bounding sphere of points is smaller than the bounding sphere of the bounding cube of the points.)
//
// pts_start	pointer to the floating point data.  Pointer to the first X value!
// pts_count	number of distinct points.
// pts_stride	number of floats in each point.  XYZ must be first, so this must be at least 3.  This lets us skip normals and ST coordinates.
// out_min_xyz	these get filled out with the min and max X, Y and Z values in the entire data aset
// out_max_xyz
// out_sphere	This sphere gets filled out.  It is in the form x, y, z (center), radius
inline void	bounding_sphere(
				const float *	pts_start,
				int			pts_count,
				int			pts_stride,
				float			out_min_xyz[3],
				float			out_max_xyz[3],
				float			out_sphere[4])
{
	int		n;

	const float *	p;

	out_min_xyz[0] = out_max_xyz[0] = pts_start[0];
	out_min_xyz[1] = out_max_xyz[1] = pts_start[1];
	out_min_xyz[2] = out_max_xyz[2] = pts_start[2];

	p = pts_start;
	for(n = 0; n < pts_count; ++n, p += pts_stride)
	{
		out_min_xyz[0] = min(out_min_xyz[0],p[0]);
		out_min_xyz[1] = min(out_min_xyz[1],p[1]);
		out_min_xyz[2] = min(out_min_xyz[2],p[2]);

		out_max_xyz[0] = max(out_max_xyz[0],p[0]);
		out_max_xyz[1] = max(out_max_xyz[1],p[1]);
		out_max_xyz[2] = max(out_max_xyz[2],p[2]);
	}

	out_sphere[0] = (out_min_xyz[0] + out_max_xyz[0]) * 0.5f;
	out_sphere[1] = (out_min_xyz[1] + out_max_xyz[1]) * 0.5f;
	out_sphere[2] = (out_min_xyz[2] + out_max_xyz[2]) * 0.5f;
	out_sphere[3] = fltmax3(
						out_max_xyz[0]-out_min_xyz[0],
						out_max_xyz[1]-out_min_xyz[1],
						out_max_xyz[2]-out_min_xyz[2]) * 0.5;

	p = pts_start;
	for(n = 0; n < pts_count; ++n, p += pts_stride)
	{
		float	dV[3] = {
						p[0] - out_sphere[0],
						p[1] - out_sphere[1],
						p[2] - out_sphere[2] };
		float	dist2 = pythag_sqr(dV[0],dV[1], dV[2]);

		if(dist2 > sqr(out_sphere[3]))
		{
			float dist = sqrt(dist2);
			out_sphere[3] = (out_sphere[3] + dist) * 0.5f;
			float scale = (dist - out_sphere[3]) / dist;
			out_sphere[0] += (dV[0] * scale);
			out_sphere[1] += (dV[1] * scale);
			out_sphere[2] += (dV[2] * scale);
		}
	}
}

// This routine takes two spheres.  Cur is the sphere we want to grow, and add is the sphere that we are adding to  it.
// cur will become bigger such that add is fully contained in cur.  We try to grow cur as little as possible.
// NOTE: a sphere with negative radius is treated as the "empty" sphere...so if cur has negative radius, then cur becomes
// add.  Both spheres are in the form x,y,z (center), r.
inline void grow_sphere(float cur[4], const float add[4])
{
	Assert	(add[3]>=0.0);			// Quick exit: new sphere is gone.
	if			(add[3]< 0.0)return;	// if the size is 0, we cannot add it.

	// Quick exit: old sphere is gone -- use new.
	if (cur[3] < 0.0) {
		cur[0] = add[0];
		cur[1] = add[1];
		cur[2] = add[2];
		cur[3] = add[3];
		return;
	}

	// Quick exit: new sphere is fully inside old (difference in radii is larger than the distance
	// from the old center to the new.  OR vice versa!
	if (sqr(cur[3] - add[3]) >= pythag_sqr(cur[0]-add[0] , cur[1]-add[1] , cur[2]-add[2]))
	{
		if (cur[3] >= add[3]) return;	// Old sphere is bigger - we win
		cur[0] = add[0]; 				// Use the new sphere and bail
		cur[1] = add[1];
		cur[2] = add[2];
		cur[3] = add[3];
	 	return;
	}

	// Vector from old to new
	double	to_new[3] = { add[0] - cur[0], add[1] - cur[1], add[2] - cur[2] };
	vec3_normalize(to_new);

	double	old_r[3] = { to_new[0] * -cur[3], to_new[1] * -cur[3], to_new[2] * -cur[3] };
	double	new_r[3] = { to_new[0] *  add[3], to_new[1] *  add[3], to_new[2] *  add[3] };

	double	p1[3] = { add[0] + new_r[0], add[1] + new_r[1], add[2] + new_r[2] };
	double	p2[3] = { cur[0] + old_r[0], cur[1] + old_r[1], cur[2] + old_r[2] };

	cur[0] = (p1[0]+p2[0])*0.5;
	cur[1] = (p1[1]+p2[1])*0.5;
	cur[2] = (p1[2]+p2[2])*0.5;
	cur[3] = pythag(p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2])*0.5 + 0.1;	// add 0.1 to counter any roundoff error that would give us a dev-assert below! this will give us enough round-off error buffer to handle numbers along the lines of 800,000 m from the origin, or 2.4 million feet up
	// What the heck is this?!?  +0.1  Here's the problem: enough rounding happens that the "Exact" new large sphere that
	// merges the two inputs might not be big enough to really contain it..it might be 0.000000001 to small. :-(  So - we fudge and
	// grow the sphere by an extra 0.1 to hedge rounding error.  Also do a quick check to see if we're definitely going to fail
	// containment tests later... that would mean our fudge factor isn't large enough.
	Assert(sqr(cur[3] - add[3]) >= pythag_sqr(cur[0]-add[0] , cur[1]-add[1] , cur[2]-add[2]) && cur[3] >= add[3]);
}

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// LIGHT HANDLING
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

struct named_light_info_t {
	const char *	name;
	int				custom;
	const char *	dataref;
};

static named_light_info_t k_light_info[] = {
	"airplane_landing",			1,		"sim/graphics/animation/lights/airplane_landing_light",
	"airplane_nav_l",			1,		"sim/graphics/animation/lights/airplane_nav_light_l",
	"airplane_nav_r",			1,		"sim/graphics/animation/lights/airplane_nav_light_r",
	"airplane_strobe",			1,		"sim/graphics/animation/lights/airplane_strobe_light",
	"airplane_beacon",			1,		"sim/graphics/animation/lights/airplane_beacon_light",

	"rwy_ww",					0,		"sim/graphics/animation/lights/runway_ww",
	"rwy_wy",					0,		"sim/graphics/animation/lights/runway_wy",
	"rwy_yw",					0,		"sim/graphics/animation/lights/runway_yw",
	"rwy_yy",					0,		"sim/graphics/animation/lights/runway_yy",
	0,0,0
};

int light_from_name(const char * name)
{
	int n = 0;
	while(k_light_info[n].name)
	{
		if(strcmp(k_light_info[n].name,name)==0) return n;
		++n;
	}
	printf("ERROR: unknown light %s\n", name);
	exit(1);
	return -1;
}

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// MEMORY BLOCK UTILITY
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// A crude memory accumulator - fixed size - we can predict that an OBJe command list won't be that big, and that we have a LOT of memory on the converter machine.

struct	mem_block {

	unsigned char *		begin;
	unsigned char *		end;
	unsigned char *		lim;

 	 mem_block(int len) { begin = (unsigned char *) malloc(len); end = begin; lim = end + len; }
	~mem_block() { free(begin); }

	int len() { return end - begin; }
	void *	accum_mem(void * mem, int len)
	{
		if((lim - end) < len)	Assert(!"Out of mem");
		memcpy(end,mem,len);
		void * p = end;
		end += len;
		return p;
	}

	template <class T>
	T *		accum(T v) { return (T *) accum_mem(&v,sizeof(T)); }
};

int accum_str(vector<string>& strs, const string& ns)
{
	for(int n = 0; n < strs.size(); ++n)
	{
		if(strs[n] == ns) return n;
	}
	strs.push_back(ns);
	return strs.size()-1;
}

//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// OBJe structures
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
// This MUST be kept in sync with the dest app.

enum {
	setup_tex=0,
	setup_tris=1,
	draw_tris=2,		// unsigned short offset, unsigned short count
	setup_lines=3,
	setup_no_tex=4,
	draw_lines=5,		// unsigned short offset, unsigned short count

	attr_lod=6,			// float near, float far, ushort: bytes to skip

	attr_shiny=7,		// float ratio
	attr_emissive=8,	// float r, g, b

	attr_cull=9,
	attr_no_cull=10,
	attr_blend=11,
	attr_no_blend=12,	// float cutoff
	attr_poly_offset=13,// char offset
	attr_reset=14,

	attr_begin=15,
	attr_end=16,
	attr_translate_static=17,	// float x, y, z
	attr_translate=18,	// float x1, y1, z1, x2, y2, z2, v1, v2, dref
	attr_rotate=19,		// float ax, ay, az, r1, r2, v1, v2, dref
	attr_show=20,		// float v1, v2, dref
	attr_hide=21,		// float v1, v2, dref

	attr_light_named=22,	// uchar light idx float x, y, z

	attr_light_bulk=23,		// ucahr light idx ushort count, float [xyz] x count

	cmd_stop=24

};



struct	embed_props_t {
	volatile int	ref_count;
	int				layer_group;
	int				tex_day;		// string offset becomes obj
	int				tex_lit;		// string offset becoems obj
	float			cull_xyzr[4];
	float			max_lod;
	float			scale_vert;		// scale for XYZ
	float			scale_tex;		// scale for XYZ
	unsigned short	geo_type;
	unsigned short	geo_size;
	int				line_off;		// byte offset to line data
	int				hard_verts;		// count of hard verticies
	unsigned int	vbo_geo;
	unsigned int	vbo_idx;
	void *			light_info;
	void *			dref_info;
};

struct master_header_t {
	char	magic[4];
	int		prp_off;
	int		prp_len;
	int		geo_off;
	int		geo_len;
	int		idx_off;
	int		idx_len;
	int		str_off;
	int		str_len;
};


//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••OBJECT COMPILATION••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

bool	XObjWriteEmbedded(const char * inFile, const XObj8& inObj, int USE_SHORT)
{
	// find scale from points
	vector<string>			str;

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// SCALING CALCS
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	float max_c = 0;
	float max_t = 0;

	for(int n = 0; n < inObj.geo_tri.count(); ++n)
	{
		const float * xyz_st = inObj.geo_tri.get(n);
		max_c = max(max_c,fabsf(xyz_st[0]));
		max_c = max(max_c,fabsf(xyz_st[1]));
		max_c = max(max_c,fabsf(xyz_st[2]));
		max_t = max(max_t,fabsf(xyz_st[6]));
		max_t = max(max_t,fabsf(xyz_st[7]));
	}

	for(int n = 0; n < inObj.geo_lines.count(); ++n)
	{
		const float * xyz_rgb = inObj.geo_lines.get(n);
		max_c = max(max_c,fabsf(xyz_rgb[0]));
		max_c = max(max_c,fabsf(xyz_rgb[1]));
		max_c = max(max_c,fabsf(xyz_rgb[2]));
	}

	for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
	for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
	if(C->cmd == obj8_LightNamed)
	{
		float xyz[3] = { C->params[0],C->params[1],C->params[2] };
		max_c = max(max_c,fabsf(xyz[0]));
		max_c = max(max_c,fabsf(xyz[1]));
		max_c = max(max_c,fabsf(xyz[2]));
	}

	max_t = ceil(max_t);
	float scale_up_vert = 32766.0 / max_c;	// off by one to make sure round-up doesn't exceed max!
	float scale_up_tex = 512.0;
	float scale_up_nrm = 16384.0;
	if(!USE_SHORT)
	{
		scale_up_vert = 1.0;	// off by one to make sure round-up doesn't exceed max!
		scale_up_tex = 1.0;
		scale_up_nrm = 1.0;
	}

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// MAIN PROPERTIES
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	embed_props_t	embed_props;
	embed_props.layer_group = 1950;
	embed_props.ref_count = 0;
	embed_props.tex_day = accum_str(str,inObj.texture);
	if(inObj.texture_lit.empty())
		embed_props.tex_lit = 0;
	else
		{embed_props.tex_lit = accum_str(str,inObj.texture_lit);}

	embed_props.cull_xyzr[0] =
	embed_props.cull_xyzr[1] =
	embed_props.cull_xyzr[2] =
	embed_props.cull_xyzr[3] = 0;

	float min_xyz[3], max_xyz[3];

	Assert(inObj.geo_tri.count() > 0);

	bounding_sphere(inObj.geo_tri.get(0),inObj.geo_tri.count(),8,min_xyz,max_xyz,embed_props.cull_xyzr);
	if(inObj.geo_lines.count() > 0)
	{
		float minp[3], maxp[3];
		float	rgb_bounds[4];
		bounding_sphere(inObj.geo_lines.get(0),inObj.geo_lines.count(),6,minp,maxp,rgb_bounds);
		for(int n = 0; n < 3; ++n)
		{
			min_xyz[n] = fltmin2(min_xyz[n],minp[n]);
			max_xyz[n] = fltmax2(max_xyz[n],maxp[n]);
		}
		grow_sphere(embed_props.cull_xyzr,rgb_bounds);
	}

	for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
	for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
	if(C->cmd == obj8_LightNamed)
	{
		float xyzr[4] = { C->params[0],C->params[1],C->params[2], 0.0 };
		grow_sphere(embed_props.cull_xyzr,xyzr);
	}

	embed_props.max_lod = inObj.lods.back().lod_far;

	if(embed_props.max_lod <= 0)
	{
		float diff_x=max_xyz[0]-min_xyz[0];
		float diff_y=max_xyz[1]-min_xyz[1];
		float diff_z=max_xyz[2]-min_xyz[2];

		if (diff_x <= 0 && diff_y <= 0 && diff_z <= 0)
		{
			embed_props.max_lod = 0;
		} else {

			// From each side, what's the smallest dimension...this is the one that will disappear and
			// is used to calculate the lesser radius.
			float lesser_front =fltmin2(diff_y, diff_z);
			float lesser_top   =fltmin2(diff_x, diff_z)/7.0;	// Reduce apparent size from top for buildings, etc.
			float lesser_side  =fltmin2(diff_x, diff_y);

			// Take the biggest of the lesser radii, that's the one we need to worry about.
			float radius=0.5*fltmax3(lesser_front,lesser_top,lesser_side);
			float tan_semi_width=tan(45.0*0.5*DEG2RAD);

			// BEN SAYS: we used to have the current FOV put in here but this is WRONG.  Remember that objs with the LOD attribute
			// contain a hard-coded LOD - that LOD dist doesn't change with FOV.  So the renderer has to compensate, and that would
			// happen in 1_terrain.  So we should generate a DEFALT LOD based on a "typical" 45 degree FOV here, not use the ren
			// settings.

			// What's this calc?  Well, at a 90 degree FOV, at 512 meters (half a screen width) from the object,
			// 1 meter is equivalent to 1 pixel. So at "radius" times that distance, the whole object is one pixel.
			// We divide by the tangent of the FOV to do this for any FOV.
			float LOD_dis=480*0.5*radius/tan_semi_width*1.5;	// throw on a 50% fudge-factor there... i can see things popping if i dont.. half a pixels still aliases between pixels
//			if (!new_obj->lites.empty())
//				LOD_dis = fltmax2(LOD_dis, 16000);
				embed_props.max_lod=LOD_dis;
		}
	}

	embed_props.scale_vert= 1.0 / scale_up_vert;
	embed_props.scale_tex = 1.0 / scale_up_tex;
	embed_props.line_off = 0;
	embed_props.hard_verts = 0;
	embed_props.vbo_geo = 0;
	embed_props.vbo_idx = 0;
	embed_props.light_info = NULL;
	embed_props.dref_info = NULL;
	if(USE_SHORT)
	{
		embed_props.geo_type = 0x1402;
		embed_props.geo_size = 2;
	} else {
		embed_props.geo_type = 0x1406;
		embed_props.geo_size = 4;
	}

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// BUILD VBOS
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	// build index list
	vector<unsigned short>	idx;
	for(vector<int>::const_iterator I = inObj.indices.begin(); I != inObj.indices.end(); ++I)
	{
		Assert(*I <= 65535);
		idx.push_back(*I);
	}

	// build geo list
	vector<short>			geo_short;
	vector<float>			geo_float;

	if(USE_SHORT)
	{
		for(int n = 0; n < inObj.geo_tri.count(); ++n)
		{
			const float * xyz_st = inObj.geo_tri.get(n);
			geo_short.push_back(xyz_st[0] * scale_up_vert);
			geo_short.push_back(xyz_st[1] * scale_up_vert);
			geo_short.push_back(xyz_st[2] * scale_up_vert);
			geo_short.push_back(xyz_st[3] * scale_up_nrm);
			geo_short.push_back(xyz_st[4] * scale_up_nrm);
			geo_short.push_back(xyz_st[5] * scale_up_nrm);
			geo_short.push_back(xyz_st[6] * scale_up_tex);
			geo_short.push_back(xyz_st[7] * scale_up_tex);
		}
		embed_props.line_off = geo_short.size() * 2;
	} else {
		for(int n = 0; n < inObj.geo_tri.count(); ++n)
		{
			const float * xyz_st = inObj.geo_tri.get(n);
			geo_float.push_back(xyz_st[0] * scale_up_vert);
			geo_float.push_back(xyz_st[1] * scale_up_vert);
			geo_float.push_back(xyz_st[2] * scale_up_vert);
			geo_float.push_back(xyz_st[3] * scale_up_nrm);
			geo_float.push_back(xyz_st[4] * scale_up_nrm);
			geo_float.push_back(xyz_st[5] * scale_up_nrm);
			geo_float.push_back(xyz_st[6] * scale_up_tex);
			geo_float.push_back(xyz_st[7] * scale_up_tex);
		}
		embed_props.line_off = geo_float.size() * 4;
	}

	for(int n = 0; n < inObj.geo_lines.count(); ++n)
	{
		const float * xyz_rgb = inObj.geo_lines.get(n);
		if(USE_SHORT)
		{
			geo_short.push_back(xyz_rgb[0] * scale_up_vert);
			geo_short.push_back(xyz_rgb[1] * scale_up_vert);
			geo_short.push_back(xyz_rgb[2] * scale_up_vert);
		} else {
			geo_float.push_back(xyz_rgb[0] * scale_up_vert);
			geo_float.push_back(xyz_rgb[1] * scale_up_vert);
			geo_float.push_back(xyz_rgb[2] * scale_up_vert);
		}

		unsigned char rgba[4] = {
				xyz_rgb[0] * 255.0,
				xyz_rgb[1] * 255.0,
				xyz_rgb[2] * 255.0,
							 255    };

		// color data - always 4-byte RGBA, no matter geo format!  Pack it into two shorts or one float.
		if(USE_SHORT)
		{
			geo_short.push_back(rgba[0] | (rgba[1] << 8));
			geo_short.push_back(rgba[2] | (rgba[3] << 8));
		} else {
			float rgbaf = *((float *) &rgba[0]);
			geo_float.push_back(rgbaf);
		}
	}

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// TRANSLATE COMMAND LIST
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	unsigned char * last_lod = 0;
	unsigned short * patch_lod;
	mem_block	cmds(1024*1024*4);	// 4 MB - if we have more than that on iphone, I will stab myself!

	enum { no_mode, line_mode, light_mode, tri_mode };
	int vbo_mode = no_mode;
	int tex_mode = tri_mode;

	int hard_start = 0;
	int hard_stop = 0;
	int is_hard = false;

	int has_poly_os = 0;
	int has_no_blend = 0;
	int has_no_cull = 0;
	int has_emissive = 0;
	int has_shiny = 0;

	for(vector<XObjLOD8>::const_iterator L = inObj.lods.begin(); L != inObj.lods.end(); ++L)
	{
		is_hard = false;
		unsigned char * this_lod = cmds.accum<unsigned char>(attr_lod);

		if(patch_lod && last_lod)
			*patch_lod = this_lod - last_lod;
		last_lod = this_lod;

		cmds.accum<float>(L->lod_near);
		if(L->lod_far <= 0)
		{
			Assert(inObj.lods.size() == 1);
			cmds.accum<float>(embed_props.max_lod);
		}
		else
			cmds.accum<float>(L->lod_far);
		patch_lod = cmds.accum<unsigned short>(0);

		for(vector<XObjCmd8>::const_iterator C = L->cmds.begin(); C != L->cmds.end(); ++C)
		switch(C->cmd) {
		case attr_Shade_Flat:
		attr_Shade_Smooth:
			Assert(!"Flat shading is not supported.\n");
			break;
		case attr_Ambient_RGB:
		case attr_Diffuse_RGB:
		case attr_Specular_RGB:
			Assert(!"Legacy lighting materials are not supported.\n");
			break;
		case attr_Emission_RGB:
			cmds.accum<unsigned char>(attr_emissive);
			cmds.accum<float>(C->params[0]);
			cmds.accum<float>(C->params[1]);
			cmds.accum<float>(C->params[2]);
			has_emissive= (C->params[0] > 0.0 || C->params[1] > 0.0 || C->params[2] > 0.0) ? 1 : 0;
			break;
		case attr_Shiny_Rat:
			cmds.accum<unsigned char>(attr_shiny);
			cmds.accum<float>(C->params[0]);
			has_shiny= (C->params[0] > 0) ? 1 : 0;
			break;
		case attr_No_Depth:
		case attr_Depth:
			Assert(!"Depth-write disable is not supported.\n");
			break;
		case attr_LOD:
			Assert(!"Unexpected LOD command.\n");
			break;
		case attr_Reset:
			if(has_shiny)
			{
				cmds.accum<unsigned char>(attr_shiny);
				cmds.accum<float>(0.0f);
				has_shiny = 0;
			}
			if(has_emissive)
			{
				cmds.accum<unsigned char>(attr_emissive);
				cmds.accum<float>(0);
				cmds.accum<float>(0);
				cmds.accum<float>(0);
				has_emissive = 0;
			}
			if(has_poly_os)
			{
				cmds.accum<unsigned char>(attr_poly_offset);
				cmds.accum<unsigned char>(0);
				has_poly_os = 0;
			}
			if(has_no_blend)
			{
				cmds.accum<unsigned char>(attr_blend);
				has_no_blend = 0;
			}
			if(has_no_cull)
			{
				cmds.accum<unsigned char>(attr_cull);
				has_no_cull=0;
			}
			break;
		case attr_Cull:
			cmds.accum<unsigned char>(attr_cull);
			has_no_cull=1;
			break;
		case attr_NoCull:
			cmds.accum<unsigned char>(attr_no_cull);
			has_no_cull=0;
			break;
		case attr_Offset:
			cmds.accum<unsigned char>(attr_poly_offset);
			Assert(C->params[0] >= 0);
			cmds.accum<unsigned char>(C->params[0]);
			has_poly_os = (C->params[0] > 0) ? 1 : 0;
			break;
		case obj_Smoke_Black:
		case obj_Smoke_White:
			Assert(!"Smoke puffs not supported.\n");
			break;
		case obj8_Tris:
			if(tex_mode != tri_mode) cmds.accum<unsigned char>(setup_tex);
			   tex_mode  = tri_mode;
			if(vbo_mode != tri_mode) cmds.accum<unsigned char>(setup_tris);
			   vbo_mode  = tri_mode;
			cmds.accum<unsigned char>(draw_tris);
			cmds.accum<unsigned short>(C->idx_offset);
			cmds.accum<unsigned short>(C->idx_count);
			Assert(C->idx_offset <= 65535);
			Assert(C->idx_count <= 65535);
			if(is_hard)
			{
				int s = C->idx_offset;
				int e = C->idx_offset + C->idx_count;
				if(s > hard_stop || e < hard_start)
					Assert(!"ERROR: hard start and stop regions are discontiguous!");
				hard_start = min(s,hard_start);
				hard_stop = max(e,hard_stop);
			}
			break;
		case obj8_Lines:
			if(tex_mode != line_mode) cmds.accum<unsigned char>(setup_no_tex);
			   tex_mode  = line_mode;
			if(vbo_mode != line_mode) cmds.accum<unsigned char>(setup_lines);
			   vbo_mode  = line_mode;
			cmds.accum<unsigned char>(draw_lines);
			cmds.accum<unsigned short>(C->idx_offset);
			cmds.accum<unsigned short>(C->idx_count);
			Assert(C->idx_offset <= 65535);
			Assert(C->idx_count <= 65535);
			break;
		case obj8_Lights:
			Assert(!"Old RGB lights are not supported.\n");
			break;
		case attr_Tex_Normal:
		case attr_Tex_Cockpit:
			Assert(!"Cockpit texture is not supported.\n");
			break;
		case attr_No_Blend:
			cmds.accum<unsigned char>(attr_no_blend);
			cmds.accum<float>(C->params[0]);
			has_no_blend = 1;
			break;
		case attr_Blend:
			cmds.accum<unsigned char>(attr_blend);
			has_no_blend = 0;
			break;
		case attr_Hard:
			is_hard=true;
			break;
		case attr_Hard_Deck:
			is_hard=true;
			break;
		case attr_No_Hard:
			is_hard=false;
			break;
		case anim_Begin:
			cmds.accum<unsigned char>(attr_begin);
			break;
		case anim_End:
			cmds.accum<unsigned char>(attr_end);
			break;
		case anim_Rotate:
			cmds.accum<unsigned char>(attr_rotate);
			cmds.accum<float>(inObj.animation[C->idx_offset].axis[0]);
			cmds.accum<float>(inObj.animation[C->idx_offset].axis[1]);
			cmds.accum<float>(inObj.animation[C->idx_offset].axis[2]);
			cmds.accum<unsigned char>(inObj.animation[C->idx_offset].keyframes.size());
			for(vector<XObjKey>::const_iterator i = inObj.animation[C->idx_offset].keyframes.begin(); i != inObj.animation[C->idx_offset].keyframes.end(); ++i)
			{
				cmds.accum<float>(i->key);
				cmds.accum<float>(i->v[0]);
			}
			cmds.accum<unsigned char>(accum_str(str,inObj.animation[C->idx_offset].dataref));
			Assert(str.size() < 256);
			break;
		case anim_Translate:
			if (inObj.animation[C->idx_offset].keyframes.size() == 2 &&
				inObj.animation[C->idx_offset].keyframes[0].v[0] == inObj.animation[C->idx_offset].keyframes[1].v[0] &&
				inObj.animation[C->idx_offset].keyframes[0].v[1] == inObj.animation[C->idx_offset].keyframes[1].v[1] &&
				inObj.animation[C->idx_offset].keyframes[0].v[2] == inObj.animation[C->idx_offset].keyframes[1].v[2])
			{
				cmds.accum<unsigned char>(attr_translate_static);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[0] * scale_up_vert);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[1] * scale_up_vert);
				cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].v[2] * scale_up_vert);
			} else {
				cmds.accum<unsigned char>(attr_translate);
				cmds.accum<unsigned char>(inObj.animation[C->idx_offset].keyframes.size());
				for(vector<XObjKey>::const_iterator i = inObj.animation[C->idx_offset].keyframes.begin(); i != inObj.animation[C->idx_offset].keyframes.end(); ++i)
				{
					cmds.accum<float>(i->key);
					cmds.accum<float>(i->v[0] * scale_up_vert);
					cmds.accum<float>(i->v[1] * scale_up_vert);
					cmds.accum<float>(i->v[2] * scale_up_vert);
				}
				cmds.accum<unsigned char>(accum_str(str,inObj.animation[C->idx_offset].dataref));
				Assert(str.size() < 256);
			}
			break;
		case anim_Hide:
			Assert(inObj.animation[C->idx_offset].keyframes.size() == 2);
			cmds.accum<unsigned char>(attr_hide);
			cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].key);
			cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[1].key);
			cmds.accum<unsigned char>(accum_str(str,inObj.animation[C->idx_offset].dataref));
			Assert(str.size() < 256);
			break;
		case anim_Show:
			Assert(inObj.animation[C->idx_offset].keyframes.size() == 2);
			cmds.accum<unsigned char>(attr_show);
			cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[0].key);
			cmds.accum<float>(inObj.animation[C->idx_offset].keyframes[1].key);
			cmds.accum<unsigned char>(accum_str(str,inObj.animation[C->idx_offset].dataref));
			Assert(str.size() < 256);
			break;
		case obj8_LightCustom:			// all in name??  param is pos?
			Assert(!"No custom lights.\n");
			break;
		case obj8_LightNamed:
			{
				bool custom = k_light_info[light_from_name(C->name.c_str())].custom;
				vector<XObjCmd8>::const_iterator E = C;
				while(E != L->cmds.end() && E->cmd == obj8_LightNamed && C->name == E->name)
				{
					++E;
					if(custom)
						break;
				}
				if(custom)
				{
					cmds.accum<unsigned char>(attr_light_named);
					cmds.accum<unsigned char>(accum_str(str,k_light_info[light_from_name(C->name.c_str())].dataref));
					Assert(str.size() < 256);
					cmds.accum<float>(C->params[0] * scale_up_vert);
					cmds.accum<float>(C->params[1] * scale_up_vert);
					cmds.accum<float>(C->params[2] * scale_up_vert);
				}
				else
				{
					cmds.accum<unsigned char>(attr_light_bulk);
					cmds.accum<unsigned char>(accum_str(str,k_light_info[light_from_name(C->name.c_str())].dataref));
					Assert(str.size() < 256);
					cmds.accum<unsigned short>(E - C);
					for(vector<XObjCmd8>::const_iterator l = C; l != E; ++l)
					{
						cmds.accum<float>(l->params[0] * scale_up_vert);
						cmds.accum<float>(l->params[1] * scale_up_vert);
						cmds.accum<float>(l->params[2] * scale_up_vert);
					}
					C = E;
					--C;
				}
				vbo_mode = no_mode;
				tex_mode = no_mode;
			}
			break;
		case attr_Layer_Group:
			if(C->name == "terrain"						 )	embed_props.layer_group = 5 + C->params[0];
			if(C->name == "beaches"						 )	embed_props.layer_group = 25 + C->params[0];
			if(C->name == "shoulders" && C->params[0] < 0)	embed_props.layer_group = 70 + C->params[0];
			if(C->name == "shoulders" && C->params[0] >=0)	embed_props.layer_group = 90 + C->params[0];
			if(C->name == "taxiways" && C->params[0] < 0 )	embed_props.layer_group = 100 + C->params[0];
			if(C->name == "taxiways" && C->params[0] >=0 )	embed_props.layer_group = 1000 + C->params[0];
			if(C->name == "runways" && C->params[0] < 0  )	embed_props.layer_group = 1100 + C->params[0];
			if(C->name == "runways" && C->params[0] >=0	 )	embed_props.layer_group = 1900 + C->params[0];
			if(C->name == "markings"					 )	embed_props.layer_group = 1920 + C->params[0];
			if(C->name == "airports" && C->params[0] < 0 )	embed_props.layer_group = 60 + C->params[0];
			if(C->name == "airports" && C->params[0] >=0 )	embed_props.layer_group = 1930 + C->params[0];
			if(C->name == "roads"						 )	embed_props.layer_group = 1940 + C->params[0];
			if(C->name == "objects"						 )	embed_props.layer_group = 1950 + C->params[0];
			if(C->name == "light_objects"				 )	embed_props.layer_group = 1955 + C->params[0];
			if(C->name == "cars"						 )	embed_props.layer_group = 1960 + C->params[0];
			break;
		case attr_Tex_Cockpit_Subregion:
			Assert(!"Cockpit textures are not supported..\n");
			break;
		default:
			Assert(!"Command not supported!\n");
		}


		if(has_shiny)
		{
			cmds.accum<unsigned char>(attr_shiny);
			cmds.accum<float>(0.0f);
			has_shiny = 0;
		}
		if(has_emissive)
		{
			cmds.accum<unsigned char>(attr_emissive);
			cmds.accum<float>(0);
			cmds.accum<float>(0);
			cmds.accum<float>(0);
			has_emissive = 0;
		}
		if(has_poly_os)
		{
			cmds.accum<unsigned char>(attr_poly_offset);
			cmds.accum<unsigned char>(0);
			has_poly_os = 0;
		}
		if(has_no_blend)
		{
			cmds.accum<unsigned char>(attr_blend);
			has_no_blend = 0;
		}
		if(has_no_cull)
		{
			cmds.accum<unsigned char>(attr_cull);
			has_no_cull=0;
		}

	}

	embed_props.hard_verts = hard_stop;

	if(tex_mode != tri_mode) cmds.accum<unsigned char>(setup_tex);

	unsigned char * end_cmd = cmds.accum<unsigned char>(cmd_stop);
	if(patch_lod && last_lod)
		*patch_lod = end_cmd - last_lod;

	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
	// WRITE OUT
	//••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••

	// Master header calc
	master_header_t	mheader;
	mheader.magic[0] = 'O';
	mheader.magic[1] = 'B';
	mheader.magic[2] = 'J';
	mheader.magic[3] = 'e';

	mheader.prp_off = sizeof(mheader);
	mheader.prp_len = sizeof(embed_props_t) + cmds.len();
	mheader.geo_off = mheader.prp_off + mheader.prp_len;
	if(USE_SHORT)
		mheader.geo_len = geo_short.size() * sizeof(geo_short[0]);
	else
		mheader.geo_len = geo_float.size() * sizeof(geo_float[0]);
	mheader.idx_off = mheader.geo_off + mheader.geo_len;
	mheader.idx_len = idx.size() * 2;
	mheader.str_off = mheader.idx_off + mheader.idx_len;
	mheader.str_len = str.size();

	for(vector<string>::iterator s = str.begin(); s != str.end(); ++s)
		mheader.str_len += s->length();


	// Write out file!

	FILE * fi = fopen(inFile, "wb");
	if(fi)
	{
		fwrite(&mheader,1,sizeof(mheader),fi);

		fwrite(&embed_props,1,sizeof(embed_props),fi);

		fwrite(cmds.begin, 1, cmds.len(), fi);

		if(USE_SHORT)
			fwrite(&*geo_short.begin(),sizeof(geo_short[0]),geo_short.size(),fi);
		else
			fwrite(&*geo_float.begin(),sizeof(geo_float[0]),geo_float.size(),fi);

		fwrite(&*idx.begin(),2,idx.size(),fi);

		for(vector<string>::iterator s = str.begin(); s != str.end(); ++s)
			fwrite(s->c_str(),1,s->length()+1,fi);

		fclose(fi);
		return 1;
	}
	return 0;
}
