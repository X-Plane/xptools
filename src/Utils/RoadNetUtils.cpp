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
#include "RoadNetUtils.h"
#include <stdio.h>
#include "XUtils.h"
#include "CompGeomDefs3.h"
#include "MatrixUtils.h"
#include "CompGeomUtils.h"

#define 	MAX_JUNC_PTS 256

/*

	ROAD NET INSTANTIATION - THEORY
	
	We can break up road instantiation into three distinct phases:
	
	
 */

typedef	vector<Point3>			QuadStrip_t;
typedef	vector<QuadStrip_t>		QuadStripVector_t;
typedef	vector<Point3>			Polygon_t;
typedef	vector<Polygon_t>		PolygonVector_t;

 

inline	double fract_part(double x) { return x - ((double) ((int) x)); }

struct	CapRecord_t {
	string			front_name;
	string			back_name;
	vector<int>		junction_types;
};

typedef	multimap<int, CapRecord_t>	CapRecordMap_t;

bool	LoadNetworkDefs(
						const char *			inFileName,
						NetworkDef_t&			outDefs,
						vector<string> *		outDesiredObjs)						
{
	string			s;
	int				n, count, our_type;
	vector<string>	v;

	map<int, string>				segname_table;
	vector<CapRecordMap_t>			cap_records;

	map<string, NetworkSegment_t>	segment_table;
	set<string>						objects_needed;

	outDefs.segments.clear();

	NetworkSegment_t *	currentSegment = NULL;
	NetworkSegmentLOD_t *	currentSegmentLOD = NULL;

	StTextFileScanner	scanner(inFileName, true);
	if (scanner.done()) return false;
	
	float		lod_near = 0, lod_far = -1;
	
	while (GetNextNoComments(scanner, s))
	{
		BreakString(s, v);
		if (!v.empty())
		{
			// BITMAP <filename>
			if (v[0] == "BITMAP")
			{
				if (v.size() < 2) return false;
				outDefs.texture = v[1];
			}
			// TYPE <number> <main segment name>
			else if (v[0] == "TYPE")
			{
				if (v.size() < 3) return false;
				segname_table.insert(map<int,string>::value_type(
					atoi(v[1].c_str()), v[2].c_str()));
			}
			// SEGMENT <name>
			else if (v[0] == "SEGMENT")
			{
				if(v.size() < 2) return false;

				NetworkSegment_t	seg;
				segment_table.insert(pair<string, NetworkSegment_t>(v[1], seg));
				currentSegment = &segment_table[v[1]];
			} 
			// LOD <near> <far>  <lon length> <v offset> [<t1> <t2> [<chop point>]]
			else if (v[0] == "LOD")
			{
				if (v.size() < 5) return false;
				NetworkSegmentLOD_t	seg;
				seg.lod_near = atof(v[1].c_str());
				seg.lod_far = atof(v[2].c_str());
				seg.width = 0.0;
				seg.scale_lon = atof(v[3].c_str());
				seg.vert_offset = atof(v[4].c_str());
				if (v.size() > 5)
					seg.start_pixel_t = atof(v[5].c_str()) / 128.0;
				else
					seg.start_pixel_t = 0.0;
				if (v.size() > 6)
					seg.end_pixel_t = atof(v[6].c_str()) / 128.0;
				else
					seg.end_pixel_t = 1.0;
				if (v.size() > 7)
					seg.chop_point_percent = atof(v[7].c_str());
				else
					seg.chop_point_percent = -1.0;
				if (seg.scale_lon == 0.0) seg.scale_lon = 1.0;
				
				currentSegment->lod.push_back(seg);
				currentSegmentLOD = &currentSegment->lod.back();

			}
			// QUAD <s1> <s2> <delta h> <delta v>
			else if (v[0] == "QUAD") 
			{
				if (v.size() < 5) return false;
				if (!currentSegmentLOD) return false;

				NetworkSegmentItem_t	item;
				item.s1 = atof(v[1].c_str()) / 512.0;
				item.s2 = atof(v[2].c_str()) / 512.0;
				item.delta_lat = atof(v[3].c_str());
				item.delta_vert = atof(v[4].c_str());
				if (lod_near == 0.0)
					currentSegmentLOD->width += item.delta_lat;
				currentSegmentLOD->items.push_back(item);
			}
			// OBJ <name> <s> <t> <rotation> <on road>	
			else if (v[0] == "OBJ")
			{
				if (v.size() < 6) return false;
				if (!currentSegmentLOD) return false;
				NetworkObjectItem_t	obj;
				obj.object_name = v[1];
				obj.lat_offset = atof(v[2].c_str());
				obj.long_fraction = atof(v[3].c_str());
				obj.rotation = atof(v[4].c_str());
				obj.on_road = atoi(v[5].c_str()) != 0;
				currentSegmentLOD->objects.push_back(obj);
				objects_needed.insert(obj.object_name);
			}
			// ENDCAP <front name> <back name> <valence> <types...>
			else if (v[0] == "ENDCAP") 
			{
				if (v.size() < 5) return false;
				count = atoi(v[3].c_str());
				if (v.size() < (4 + count)) return false;
				CapRecord_t	cap;
				cap.front_name = v[1];
				cap.back_name = v[2];
				for (n = 0; n < count; ++n)
					cap.junction_types.push_back(atoi(v[n+4].c_str()));
				
				our_type = atoi(v[4].c_str());
				
				if (cap_records.size() <= our_type)
					cap_records.resize(our_type + 1);
				
				CapRecordMap_t& my_map = cap_records[our_type];
				
				my_map.insert(CapRecordMap_t::value_type(cap.junction_types.size(), cap));
			} else if (v[0] == "JUNCTION")
			// JUNCTION <near> <far> <count> <types...> <s&ts>
			{
				// We must have at least 3 S&Ts and one type, plus two LODs, for 11 params.
				if (v.size() < 11) return false;
				count = atoi(v[3].c_str());
				if (v.size() < (3 * count) + 4) return false;
				
				JunctionRule_t	rule;
				rule.lod_near = atof(v[1].c_str());
				rule.lod_far = atof(v[2].c_str());
				for (int n = 0; n < count; ++n)
				{
					rule.junction_types.push_back(atoi(v[n+4].c_str()));
					rule.s_coord.push_back(atof(v[n*2+count+4].c_str()) / 512.0);
					rule.t_coord.push_back(atof(v[n*2+count+5].c_str()) / 128.0);
				}
				if (count == 1)
				for (int n = 1; n < 3; ++n)
				{
					rule.s_coord.push_back(atof(v[n*2+count+4].c_str()) / 512.0);
					rule.t_coord.push_back(atof(v[n*2+count+5].c_str()) / 128.0);
				}
				
				outDefs.junctions.push_back(rule);
			}
		}
	}
	
	map<string, NetworkSegment_t>::iterator segIter;
	map<int, string>::iterator 				nameIter;
	
	// Now go in and hack all of the widths - we have to do this 
	// once the whole file is read.
	for (segIter = segment_table.begin();
			segIter != segment_table.end(); ++segIter)
	{
		for (int l = 0; l < segIter->second.lod.size(); ++l)
		if (segIter->second.lod[l].width != 0.0)
			for (n = 0; n < segIter->second.lod[l].items.size(); ++n)
				segIter->second.lod[l].items[n].delta_lat /= segIter->second.lod[l].width;
	}				
	
	// Put together the whole thing.

	NetworkSegment_t	nullCap;

	for (nameIter = segname_table.begin(); nameIter != segname_table.end(); ++nameIter)
	{
		segIter = segment_table.find(nameIter->second);
		if (segIter == segment_table.end()) 
			return false;
		else {
			if (outDefs.segments.size() <= nameIter->first)
				outDefs.segments.resize(nameIter->first+1);
			outDefs.segments[nameIter->first] = segIter->second;
		}
	}
		
	// Caps...
	outDefs.caps.resize(outDefs.segments.size());
	for (n = 0; n < cap_records.size(); ++n)
	{
		NetworkCapRules_t rulez;
		for (multimap<int,CapRecord_t>::reverse_iterator riter =
			cap_records[n].rbegin(); riter != cap_records[n].rend(); ++riter)
		{
			NetworkCapRule_t	rule;
			rule.junction_types = riter->second.junction_types;
			
			segIter = segment_table.find(riter->second.front_name);
			if (segIter == segment_table.end())
				rule.front_cap = nullCap;
			else
				rule.front_cap = segIter->second;
				
			segIter = segment_table.find(riter->second.back_name);
			if (segIter == segment_table.end())
				rule.back_cap = nullCap;
			else
				rule.back_cap = segIter->second;
			
			rulez.push_back(rule);
		}
		outDefs.caps[n] = rulez;
	}
	
	if (outDesiredObjs)
	{
		outDesiredObjs->clear();
		for (set<string>::iterator i = objects_needed.begin(); i != objects_needed.end(); ++i)
			outDesiredObjs->push_back(*i);
	}
	
	return true;
}

void	InterpPt3(const Point3& p1, const Point3& p2, float i, Point3& o)
{
	float mi = 1.0 - i;
	o.x = p1.x * mi + p2.x * i;
	o.y = p1.y * mi + p2.y * i;
	o.z = p1.z * mi + p2.z * i;	
}

static	double	CalcJointAngleDeg(const Point3& p1, const Point3& p2, const Point3& p3)
{
	Vector3 v1(p1, p2);
	Vector3 v2(p3, p2);
	v1.normalize();
	v2.normalize();
	return acos(v1.dot(v2)) * 180.0 / M_PI;
}					

static	double	CalcShapedJointAngle(const Shape_Point_t& p1, const Shape_Point_t& p2, const Shape_Point_t& p3)
{
	/* HACK WARNING -- this routine is an approximation, and a shitty one at that.  /BAS
		
		The problem is that we're supposed to be figuring out the angle of this joint.
		Theoretically if there is a shape point then the angle at this joint is straight
		because the curve must be normal.  This is probably not true, but this is a 
		different pinch case that we don't yet care about.  The real problem is that
		if our ends have a curve but the center doesn't, the angle may be less or more
		than if no curves were there due to the bending of the curve.  I have _no_
		idea how to calculate the tangent of the flat side of a half-bezier curve...
		it seems like it would be undefined.  So...for now we wing it.
	*/
	
	Vector3	v1(p1.location, p2.location);
	Vector3 v2(p3.location, p2.location);
	
	v1.normalize();
	v2.normalize();
	
	return acos(v1.dot(v2));
}

static	double	CalcShapedJointAngleWithVector(const Shape_Point_t& p1, const Shape_Point_t& p2, const Vector3& v)
{
	/* HACK WARNING -- this routine is an approximation, and a shitty one at that.  /BAS

		This one is slightly better than above in that if we start with a curve, we use
		our tangent, which is _close_ to being safe, but a little risky.  (Our safety margin 
		HACK_SAFETY_AMOUNT protects us from the slight curve we ignore here.)  But if our
		far point has a curve, hell's if I know what the tangent is at the point without 
		a control.

	*/
	
	Vector3	v1(p1.location, p2.location);
	if (p1.has_curve)
		v1 = Vector3(p1.location, p1.curve);
	Vector3 v2(v);
	
	v1.normalize();
	v2.normalize();
	
	return acos(v1.dot(v2));
}


static	double	CalcSafetyLength(double angle, double width_us, double width_them)
{
	double	s = sin(angle);
	double	t = cos(angle);
	
	if (s == 0.0) return 0.0;	// These are continuous - no safety needed!
	return width_us * t / s + width_them / s;
}

static	void	BuildOneRoadSegment(
					const Point3&			inStartLeft,
					const Point3&			inStartRight,
					const Point3&			inEndLeft,
					const Point3&			inEndRight,
					float					inLeftT1,
					float					inRightT1,
					float					inLeftT2,
					float					inRightT2,
					bool					inAngleT1,		// Scale texture T coords based on 
					bool					inAngleT2,		// irregularities in the quad!
					const NetworkSegmentLOD_t&	inDef,
					ExtrudeFunc_f			extrudeFunc,
					ReceiveObj_f			objReceiveFunc,
					void *					inRef1,
					void *					inRef2)
{
	// Regenerate up vectors...
	Vector3	start_lat(inStartLeft, inStartRight);
	Vector3	end_lat(inEndLeft, inEndRight);
	Vector3 longitude(Segment3(inStartLeft, inStartRight).midpoint(), Segment3(inEndLeft, inEndRight).midpoint());
	
	Vector3	inUpStart = start_lat.cross(longitude);
	Vector3	inUpEnd = end_lat.cross(longitude);
	inUpStart.normalize();
	inUpEnd.normalize();

	Point3	middleStart, middleEnd;
	InterpPt3(inStartLeft, inStartRight, 0.5, middleStart);	
	InterpPt3(inEndLeft  , inEndRight  , 0.5, middleEnd  );
	double	norm_dist  = sqrt(Segment3(middleStart , middleEnd ).squared_length());
	double	left_dist  = sqrt(Segment3(inStartLeft , inEndLeft ).squared_length());
	double	right_dist = sqrt(Segment3(inStartRight, inEndRight).squared_length());

	// If we are asked to angle either t1 or t2 (the two ends), basically what's
	// happening is we have a funny shaped quad and are being asked to treat it as
	// having the texture projected over it (e.g. use more texture if we stick out)
	// rather than stretch the texture.  We split the difference between the two
	// ends if we are being angled on both (but the code almost never asks for this).
	
	double	left_percent = left_dist / norm_dist;
	double	right_percent = right_dist / norm_dist;
	double	t_change = right_percent - left_percent;
	double	half_t_change = t_change * 0.5;
	// These numbers are what percentage of the T-space we travel is
	// different _from_ the left _to_ the right side.  For example, if this is .6
	// it means that we went from .7 to 1.3 percent of the normal length.
	// We split the difference between the two ends if necessary.
	double	t1_change = inAngleT1 ? (inAngleT2 ? half_t_change : t_change) : 0.0;
	double	t2_change = inAngleT2 ? (inAngleT1 ? half_t_change : t_change) : 0.0;

	// Now we go in and scale the T1s and T2s that we were actually passed in
	// (for all we know, the texturing space we are being passed on left and right
	// doesn't match up at all!!!) and apply this scaling.
	double	left_t_dist = inLeftT2 - inLeftT1;
	double	right_t_dist = inRightT2 - inRightT1;
	inLeftT1  += left_t_dist * 0.5 * t1_change;		// Math note: positive t_change means left is
	inRightT1 -= right_t_dist * 0.5 * t1_change;	// shorter than right.  So add to left T1, subtract
	inLeftT2  -= left_t_dist * 0.5 * t2_change;		// from right T1 to shorten.  Subtract from left T2
	inRightT2 += right_t_dist * 0.5 * t2_change;	// and add to right T2 to shorten.

	float	cur_lat = 0.0;
	float	cur_height = inDef.vert_offset;
	for (int s = 0; s < inDef.items.size(); ++s)
	{
		Point3	sl,el,er,sr;
		InterpPt3(inStartLeft, inStartRight, cur_lat, sl);
		InterpPt3(inEndLeft,   inEndRight, 	 cur_lat, el);
		double	left_t1 = inLeftT1 * (1.0 - cur_lat) + inRightT1 * cur_lat;
		double	left_t2 = inLeftT2 * (1.0 - cur_lat) + inRightT2 * cur_lat;
		
		cur_lat += inDef.items[s].delta_lat;
		InterpPt3(inStartLeft, inStartRight, cur_lat, sr);
		InterpPt3(inEndLeft,   inEndRight, 	 cur_lat, er);
		double	right_t1 = inLeftT1 * (1.0 - cur_lat) + inRightT1 * cur_lat;
		double	right_t2 = inLeftT2 * (1.0 - cur_lat) + inRightT2 * cur_lat;
		
		sl += inUpStart * cur_height;		// sl.y += cur_height;
		el += inUpEnd * cur_height;		// el.y += cur_height;		
		cur_height += inDef.items[s].delta_vert;		
		er += inUpEnd * cur_height;			// er.y += cur_height;
		sr += inUpStart * cur_height;			// sr.y += cur_height;
		
		float pts[12];
		float sts[ 8];
		
		pts[0] = sl.x; pts[1] = sl.y; pts[2] = sl.z;
		sts[0] = inDef.items[s].s1;
		sts[1] = inDef.start_pixel_t + (inDef.end_pixel_t - inDef.start_pixel_t) * left_t1;

		pts[3] = el.x; pts[4] = el.y; pts[5] = el.z;
		sts[2] = inDef.items[s].s1;
		sts[3] = inDef.start_pixel_t + (inDef.end_pixel_t - inDef.start_pixel_t) * left_t2;

		pts[6] = er.x; pts[7] = er.y; pts[8] = er.z;
		sts[4] = inDef.items[s].s2;
		sts[5] = inDef.start_pixel_t + (inDef.end_pixel_t - inDef.start_pixel_t) * right_t2;

		pts[9] = sr.x; pts[10] = sr.y; pts[11] = sr.z;
		sts[6] = inDef.items[s].s2;
		sts[7] = inDef.start_pixel_t + (inDef.end_pixel_t - inDef.start_pixel_t) * right_t1;

		extrudeFunc(ext_Poly_Quad, 4, pts,sts, inDef.lod_near, inDef.lod_far, inRef1);
	}	
	for (int o = 0; o < inDef.objects.size(); ++o)
	{	
		Point3	objPosFront, objPosBack, pt;
		InterpPt3(inStartLeft, inStartRight, inDef.objects[o].lat_offset, objPosFront);
		InterpPt3(inEndLeft, inEndRight, inDef.objects[o].lat_offset, objPosBack);
		Vector3	v(objPosFront, objPosBack);
		v.normalize();
		double	angle = atan2(v.dx, -v.dz) * 180.0 / M_PI;
		
		double	our_t1 = inLeftT1 * (1.0 - inDef.objects[o].lat_offset) + inRightT1 * inDef.objects[o].lat_offset;
		double	our_t2 = inLeftT2 * (1.0 - inDef.objects[o].lat_offset) + inRightT2 * inDef.objects[o].lat_offset;
		
		double	elapsed_t = our_t2 - our_t1;
		double	initial_offset = inDef.objects[o].long_fraction - fract_part(our_t1);
		if (initial_offset < 0.0)
			initial_offset += 1.0;
		int number_to_place = ceil(elapsed_t - initial_offset);
		
		for (int n = 0; n < number_to_place; ++n)
		{
			double	this_t = our_t1 + initial_offset + (1.0 * (double) n);
			double	long_displace = (this_t - our_t1) / (our_t2 - our_t1);

			InterpPt3(objPosFront, objPosBack, long_displace, pt);

			if (!inDef.objects[o].on_road)
				pt.y = 0.0;
			if (objReceiveFunc)
				objReceiveFunc(pt.x, pt.y, pt.z, inDef.objects[o].rotation+ angle, !inDef.objects[o].on_road, inDef.objects[o].object_name.c_str(), inRef2);			
		}		
	}
}

static	void	ExtrudeRoadChain(
					const vector<Point3>&		inQuadStrip,
					const vector<Vector3>&		up,
					const NetworkSegmentLOD_t&	inMain,
					const NetworkSegmentLOD_t&	inStartCap,
					const NetworkSegmentLOD_t& 	inEndCap,					
					ExtrudeFunc_f				extrudeFunc,
					ReceiveObj_f				objReceiveFunc,
					void *						inRef1,
					void *						inRef2)
{
	extrudeFunc(ext_Start_Obj, 0, NULL, NULL, 0, 0, inRef1);
	
	int 	n, last;
	Point3	sl, sr, el, er;	
	float	t1 = 0.0, t1l, t1r, t2l, t2r;

	vector<Point3>	quadStrip(inQuadStrip), frontCap, backCap, chain3d;
	if (inStartCap.scale_lon > 0.0)
	{
		if (inStartCap.chop_point_percent != -1.0)
			RemoveFromQuadStripFront(quadStrip, inStartCap.scale_lon * inStartCap.chop_point_percent, frontCap, true);
		else
			RemoveFromQuadStripFront(quadStrip, inStartCap.scale_lon, frontCap, false);
	}
	if (inEndCap.scale_lon > 0.0)
	{
		if (inEndCap.chop_point_percent != -1.0)
			RemoveFromQuadStripBack(quadStrip, inEndCap.scale_lon * inEndCap.chop_point_percent, backCap, true);
		else
			RemoveFromQuadStripBack(quadStrip, inEndCap.scale_lon, backCap, false);
	}
	
	ReverseQuadStrip(frontCap);
	ReverseQuadStrip(backCap);
	
	for (n = 0; n < quadStrip.size(); n += 2)
	{
		chain3d.push_back(Segment3(quadStrip[n], quadStrip[n+1]).midpoint());
	}
	
	double	total_front_left = 0.0, total_front_right = 0.0, total_back_left = 0.0, total_back_right = 0.0;
	
	if (inStartCap.chop_point_percent != -1.0)
		total_front_left = total_front_right = inStartCap.scale_lon;
	else for (n = 2; n < frontCap.size(); n += 2)
	{
		sr = frontCap[n-2];
		sl = frontCap[n-1];
		er = frontCap[n  ];
		el = frontCap[n+1];
		
		float	lside_len = sqrt(Segment3(sl,el).squared_length());
		float	rside_len = sqrt(Segment3(sr,er).squared_length());
		total_front_left += lside_len;
		total_front_right += rside_len;
	}
	
	if (inEndCap.chop_point_percent != -1.0)
		total_back_left = total_back_right = inEndCap.scale_lon;
	else for (n = 2; n < backCap.size(); n += 2)
	{
		sr = backCap[n-2];
		sl = backCap[n-1];
		er = backCap[n  ];
		el = backCap[n+1];
		
		float	lside_len = sqrt(Segment3(sl,el).squared_length());
		float	rside_len = sqrt(Segment3(sr,er).squared_length());
		
		total_back_left += lside_len;
		total_back_right += rside_len;
	}	
	
	t1l = 0.0;
	t1r = 0.0;		
	for (n = 2; n < frontCap.size(); n += 2)
	{
		sr = frontCap[n-2];
		sl = frontCap[n-1];
		er = frontCap[n  ];
		el = frontCap[n+1];
		
		float	lside_len = sqrt(Segment3(sl,el).squared_length());
		float	rside_len = sqrt(Segment3(sr,er).squared_length());
		
		t2l = t1l + (lside_len / total_front_left);
		t2r = t1r + (rside_len / total_front_right);
		
		BuildOneRoadSegment(sl, sr, el, er, 
			t1l, t1r, t2l, t2r,
			false, false, inStartCap, extrudeFunc, objReceiveFunc, inRef1, inRef2);
		
		t1l = t2l;
		t1r = t2r;		
	}
	
	t1l = 0.0;
	t1r = 0.0;	
	for (n = 2; n < backCap.size(); n += 2)
	{
		sr = backCap[n-2];
		sl = backCap[n-1];
		er = backCap[n  ];
		el = backCap[n+1];
		
		float	lside_len = sqrt(Segment3(sl,el).squared_length());
		float	rside_len = sqrt(Segment3(sr,er).squared_length());
		
		t2l = t1l + (lside_len / total_back_left);
		t2r = t1r + (rside_len / total_back_right);
		
		BuildOneRoadSegment(sl, sr, el, er, 
			t1l, t1r, t2l, t2r,
			false, false, inEndCap, extrudeFunc, objReceiveFunc, inRef1, inRef2);
		
		t1l = t2l;
		t1r = t2r;		
	}		
	
	last = chain3d.size() - 1;	
	for (n = 2; n < quadStrip.size(); n += 2)
	{
		sr = quadStrip[n-2];
		sl = quadStrip[n-1];
		er = quadStrip[n  ];
		el = quadStrip[n+1];
		
		int	chain_pt_start = n/2-1;
		int	chain_pt_end = n/2;
		
		float	dist = sqrt(Segment3(chain3d[chain_pt_start], chain3d[chain_pt_end]).squared_length());
		float	t2 = t1 + (dist / inMain.scale_lon);
		
		bool	start_tight = false;
		bool	end_tight = false;
		
		if (chain_pt_start > 0)
			start_tight = (fabs(CalcJointAngleDeg(chain3d[chain_pt_start-1],chain3d[chain_pt_start], chain3d[chain_pt_start+1])) > 20.0);
		else 
			start_tight = true;
		
		if (chain_pt_end < last)
			end_tight = (fabs(CalcJointAngleDeg(chain3d[chain_pt_end-1],chain3d[chain_pt_end], chain3d[chain_pt_end+1])) > 20.0);
		else 
			chain_pt_end = true;
		
		BuildOneRoadSegment(sl, sr, el, er, 
//			up[n/2-1], up[n/2],
			t1, t1, t2, t2,
			start_tight, end_tight, inMain, extrudeFunc, objReceiveFunc, inRef1, inRef2);
		
		t1 = t2 - (float) ((int) t2);
		
	}
	
	extrudeFunc(ext_Stop_Obj, 0, NULL, NULL, 0, 0, inRef1);	
}

void ExtrudeSpecializedJunction(
						const Polygon_t& 		inPolygon, 
						const Point3&			inCentroid,
						const JunctionRule_t&	inRule,
						ExtrudeFunc_f			extrudeFunc,
						ReceiveObj_f			objReceiveFunc,
						void *					inRef1,
						void *					inRef2)
{	
	if (inPolygon.size() < 3)	return;
	float	pts[3 * MAX_JUNC_PTS];
	float	sts[2 * MAX_JUNC_PTS];

	if (inPolygon.size() > MAX_JUNC_PTS)
		MACIBM_alert(0, "Too many polygon pts in specialized junction", "", "", "", t_exit);
		
	extrudeFunc(ext_Start_Obj, 0, NULL, NULL, 0, 0, inRef1);
		
	for (int n = 0; n < inPolygon.size(); ++n)
	{
		pts[n*3  ] = inPolygon[n].x;
		pts[n*3+1] = inPolygon[n].y;
		pts[n*3+2] = inPolygon[n].z;
		sts[n*2  ] = inRule.s_coord[n];
		sts[n*2+1] = inRule.t_coord[n];
	}
	
	extrudeFunc(ext_Poly_Poly, inPolygon.size(), pts, sts, inRule.lod_near, inRule.lod_far, inRef1);

	extrudeFunc(ext_Stop_Obj, 0, NULL, NULL, 0, 0, inRef1);
}

void ExtrudeGeneralJunction(
						const Polygon_t& 				inPolygon, 
						const Point3&					inCentroid,
						const vector<JunctionRule_t>&	inRules,
						ExtrudeFunc_f					extrudeFunc,
						ReceiveObj_f					objReceiveFunc,
						void *							inRef1,
						void *							inRef2)
{
	if (inPolygon.size() < 3)	return;

	float pts[9];
	float sts[6];
	extrudeFunc(ext_Start_Obj, 0, NULL, NULL, 0, 0, inRef1);
	for (int n = 0; n < inPolygon.size(); ++n)
	{
		int prev = (n-1);
		if (prev < 0) prev += inPolygon.size();
		pts[0] = inPolygon[prev].x;
		pts[1] = inPolygon[prev].y;
		pts[2] = inPolygon[prev].z;
		sts[0] = inRules[n].s_coord[0];
		sts[1] = inRules[n].t_coord[0];

		pts[3] = inPolygon[n].x;
		pts[4] = inPolygon[n].y;
		pts[5] = inPolygon[n].z;
		sts[2] = inRules[n].s_coord[1];
		sts[3] = inRules[n].t_coord[1];

		pts[6] = inCentroid.x;
		pts[7] = inCentroid.y;
		pts[8] = inCentroid.z;
		sts[4] = inRules[n].s_coord[2];
		sts[5] = inRules[n].t_coord[2];
		extrudeFunc(ext_Poly_Tri, 3, pts, sts, inRules[n].lod_near, inRules[n].lod_far, inRef1);
	}
	extrudeFunc(ext_Stop_Obj, 0, NULL, NULL, 0, 0, inRef1);
	
}

// NOTE: since our estimation of the intersect angle is at best a hack,
// we pad ourselves 2x on the straightaway for now.  Is this enough?!?
#define 	HACK_SAFETY_AMOUNT 1.3

/* 
 * ShapedCurveToCurve
 * 
 * This routine takes one shaped chain and breaks it down into many line segments.  There are
 * a number of parameters controlling this routine:
 *
 * inPoints					This is the original set of shaped points that are to be expanded.
 * outPoints				The end resulting chain of points.
 *
 * inWidth					This is how wide our road will eventually be.  It is used to keep points
 *							from becoming so dense that the curves pinch themselves.
 *
 * inLeftStartFriendDir		These are the headings (away from the intersection we're in) and angles of the neigbhoring
 * inLeftStartFriendWidth	roads at our end junctions.  The width is 0 if there are no neighbors.  These are used
 * inRightStartFriendDir	to straighten the ends of our segments so that we don't have tiny little segments in the
 * inRightStartFriendWidth	middle of an intersection.
 * inLeftEndFriendDir
 * inLeftEndFriendWidth
 * inRightEndFriendDir
 * inRightEndFriendWidth
 *
 */
static	void	ShapedCurveToCurve(
					const vector<Shape_Point_t>&	inPoints,
					vector<Point3>&					outPoints,
					double							inWidth,
					const Vector3&					inLeftStartFriendDir,
					double							inLeftStartFriendWidth,
					const Vector3&					inRightStartFriendDir,
					double							inRightStartFriendWidth,
					const Vector3&					inLeftEndFriendDir,
					double							inLeftEndFriendWidth,
					const Vector3&					inRightEndFriendDir,
					double							inRightEndFriendWidth)
{
	outPoints.clear();
	outPoints.push_back(inPoints[0].location);
	int last = inPoints.size() - 1;
	for (int n = 1; n <= last; ++n)
	{
		vector<Point3>	pts;
		
		double	front_angle = (n > 1)   ? (CalcShapedJointAngle(inPoints[n-2], inPoints[n-1], inPoints[n])) : 0.0;	
		double	back_angle = (n < last) ? (CalcShapedJointAngle(inPoints[n-1], inPoints[n], inPoints[n+1])) : 0.0;

		double	front_safety = (n > 1) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(front_angle, inWidth * 0.5, inWidth * 0.5)) : 0.0;
		double	back_safety = (n < last) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(back_angle, inWidth * 0.5, inWidth * 0.5)) : 0.0;
				
		// We have to special case our start and end!
		if (n == 1)
		{
			double	front_left_angle = (inLeftStartFriendWidth > 0.0) ? CalcShapedJointAngleWithVector(inPoints[0], inPoints[1], inLeftStartFriendDir) : 0.0;
			double	front_right_angle = (inRightStartFriendWidth > 0.0) ? CalcShapedJointAngleWithVector(inPoints[0], inPoints[1], inRightStartFriendDir) : 0.0;
			double	front_left_safety = (inLeftStartFriendWidth > 0.0) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(front_left_angle, inWidth * 0.5, inLeftStartFriendWidth * 0.5)) : 0.0;
			double	front_right_safety = (inRightStartFriendWidth > 0.0) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(front_right_angle, inWidth * 0.5, inRightStartFriendWidth * 0.5)) : 0.0;

			front_safety = (front_left_safety > front_right_safety) ? front_left_safety : front_right_safety;
		}
		if (n == last)
		{
			double	back_left_angle = (inLeftEndFriendWidth > 0.0) ? CalcShapedJointAngleWithVector(inPoints[last], inPoints[last-1],inLeftEndFriendDir) : 0.0;
			double	back_right_angle = (inRightEndFriendWidth > 0.0) ? CalcShapedJointAngleWithVector(inPoints[last], inPoints[last-1], inRightEndFriendDir) : 0.0;
			double	back_left_safety = (inLeftEndFriendWidth > 0.0) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(back_left_angle, inWidth * 0.5, inLeftEndFriendWidth * 0.5)) : 0.0;
			double	back_right_safety = (inRightEndFriendWidth > 0.0) ? (HACK_SAFETY_AMOUNT * CalcSafetyLength(back_right_angle, inWidth * 0.5, inRightEndFriendWidth * 0.5)) : 0.0;

			back_safety = (back_left_safety > back_right_safety) ? back_left_safety : back_right_safety;
		}
				
		BezierCurve(inPoints[n-1].location,
					inPoints[n].location,
					inPoints[n-1].has_curve,
					inPoints[n].has_curve,
					inPoints[n-1].curve,					
					inPoints[n].location - Vector3(inPoints[n].location, inPoints[n].curve),
					50,
					front_safety,
					back_safety,
					pts);
		pts.erase(pts.begin());
		outPoints.insert(outPoints.end(), pts.begin(), pts.end());
	}
}

static	void	FetchStripSegments(
						const QuadStrip_t& 			inQuads,
						const NetworkData_t&		inData,
						int							inChainIndex,
						int							inJunctionIndex,
						bool						inWantsRightSide,
						bool						inWantsBack,
						Segment3&					outSegment)
{
	int	n = inQuads.size();
	
	if (inWantsBack)
	{
		if (inWantsRightSide)
		{
			outSegment = Segment3(inQuads[n-4], inQuads[n-2]);
		} else {
			outSegment = Segment3(inQuads[n-3], inQuads[n-1]);
		}
	} else {
		if (inWantsRightSide)
		{
			outSegment = Segment3(inQuads[3], inQuads[1]);			
		} else {
			outSegment = Segment3(inQuads[2], inQuads[0]);			
		}
	}
}					

static	void	ReduceStripSegments(
						QuadStrip_t&	 			ioQuads,
						const NetworkData_t&		inData,
						int							inChainIndex,
						int							inJunctionIndex,
						bool						inWantsRightSide,
						bool						inWantsBack,
						const Point3&				inPt)
{
	int	n = ioQuads.size();
	
	if (inWantsBack)
	{
		if (inWantsRightSide)
		{
			ioQuads[n-2] = inPt;
		} else {
			ioQuads[n-1] = inPt;
		}
	} else {
		if (inWantsRightSide)
		{
			ioQuads[1] = inPt;
		} else {
			ioQuads[0] = inPt;
		}
	}
}		

void	ExtrudeNetwork(
					const NetworkDef_t&		inDefs,
					const NetworkData_t&	inRoads,
					ExtrudeFunc_f			extrudeFunc,
					ReceiveObj_f			objReceiveFunc,
					void *					inRef1,
					void *					inRef2)
{					

		int	chain, junction;
		NetworkJunction_t::spur		me_start, me_end, start_left_spur, start_right_spur, end_left_spur, end_right_spur;

	PolygonVector_t			polyjunctions;
	QuadStripVector_t		quadchains;
	QuadStripVector_t		curvedchains;
	vector<vector<Vector3> >	chainups;
	
	//	1. Conversion of "shaped curve" (a set of linked bezier curves) road
	// 	   centerlines into a simple set of line segments with many small
	// 	   segments approximating curves.
	//	   ** PROBLEM ** How do we make sure to not pinch at an intersection?!?
	   
	for (chain = 0; chain < inRoads.chains.size(); ++chain)
	{
		vector<Shape_Point_t>	shapes = inRoads.chains[chain].shape_points;
		Shape_Point_t			start, stop;
		start.location = inRoads.junctions[inRoads.chains[chain].start_junction].location;
		start.has_curve = inRoads.chains[chain].has_start_curve;
		start.curve = inRoads.chains[chain].start_curve;
		
		stop.location = inRoads.junctions[inRoads.chains[chain].end_junction].location;
		stop.has_curve = inRoads.chains[chain].has_end_curve;
		stop.curve = inRoads.chains[chain].end_curve;
		
		shapes.insert(shapes.begin(), start);
		shapes.insert(shapes.end(), stop);
		
		vector<Point3>	curved;
		
		me_start = NetworkJunction_t::spur(chain, false);
		me_end = NetworkJunction_t::spur(chain, true);
		start_left_spur  = inRoads.previous_chain(me_start);
		start_right_spur = inRoads.next_chain    (me_start);
		end_left_spur    = inRoads.previous_chain(me_end  );
		end_right_spur   = inRoads.next_chain    (me_end  );


		ShapedCurveToCurve(shapes, curved, inDefs.segments[inRoads.chains[chain].chain_type].lod[0].width,
			inRoads.junction_get_spur_heading(start_left_spur),
			(start_left_spur == me_start) ? 0.0 : 
			inDefs.segments[inRoads.chains[start_left_spur.first].chain_type].lod[0].width,

			inRoads.junction_get_spur_heading(start_right_spur),
			(start_right_spur == me_start) ? 0.0 : 
			inDefs.segments[inRoads.chains[start_right_spur.first].chain_type].lod[0].width,

			inRoads.junction_get_spur_heading(end_left_spur),
			(end_left_spur == me_end) ? 0.0 : 
			inDefs.segments[inRoads.chains[end_left_spur.first].chain_type].lod[0].width,

			inRoads.junction_get_spur_heading(end_right_spur),
			(end_right_spur == me_end) ? 0.0 : 
			inDefs.segments[inRoads.chains[end_right_spur.first].chain_type].lod[0].width);
					
		curvedchains.push_back(curved);
		
	}

	//	2. Conversion of these shaped curves into quad strips.  These quad strips
	//	   will have the pavement built onto them later.

	for (chain = 0; chain < inRoads.chains.size(); ++chain)
	{
			vector<Point3>	chain3d;
			vector<Point3>	quadStrip;
			vector<double>	widths;
			vector<Vector3>	up;
	
		for (int n = 0; n < curvedchains[chain].size(); ++n)
		{
			widths.push_back(inDefs.segments[inRoads.chains[chain].chain_type].lod[0].width);
	//		Vector3	upv(-(float) n / (float) inChain.size(), 1.0, (float) n / (float) inChain.size());
			Vector3	upv(0.0, 1.0, 0.0);
	//		Vector3	upv(1.0, 1.0, -1.0);
			upv.normalize();
			up.push_back(upv);
			chain3d.push_back(Point3(curvedchains[chain][n].x,curvedchains[chain][n].y,curvedchains[chain][n].z));
		}		

		ChainToQuadStrip(chain3d, up, widths, quadStrip);
		quadchains.push_back(quadStrip);		
		chainups.push_back(up);
	}
	

	//	3. Build-up of junction polygons from the quad strips.  The quad strips are
	//	   trimmed back to match the polygons.
	for (junction = 0; junction < inRoads.junctions.size(); ++junction)
	{
		Polygon_t	junctionShape;
		// We only have to build a junction if this junction isn't the end of one road.
		if (inRoads.junctions[junction].spurs.size() > 1)
		for (int spur = 0; spur < inRoads.junctions[junction].spurs.size(); ++spur)
		{
			// The Nth vertex of the polygon is the intersection of the left side of the Nth
			// spur with the right side of the Nth + 1 spur.
			
			int	next_spur = (spur + 1) % inRoads.junctions[junction].spurs.size();
			Segment3	our_left, following_right;
			
			FetchStripSegments(quadchains[inRoads.junctions[junction].spurs[spur].first],inRoads,
							inRoads.junctions[junction].spurs[spur].first, junction,
							false, inRoads.junctions[junction].spurs[spur].second, our_left);

			FetchStripSegments(quadchains[inRoads.junctions[junction].spurs[next_spur].first],inRoads,
							inRoads.junctions[junction].spurs[next_spur].first, junction,
							true, inRoads.junctions[junction].spurs[next_spur].second, following_right);

			Point3	intersect_pt;
			if (IntersectLinesAroundJunction(Line3(our_left), Line3(following_right), inRoads.junctions[junction].location, intersect_pt))
			{
				junctionShape.push_back(intersect_pt);
				ReduceStripSegments(quadchains[inRoads.junctions[junction].spurs[spur].first],inRoads,
							inRoads.junctions[junction].spurs[spur].first, junction,
							false, inRoads.junctions[junction].spurs[spur].second, intersect_pt);

				ReduceStripSegments(quadchains[inRoads.junctions[junction].spurs[next_spur].first],inRoads,
							inRoads.junctions[junction].spurs[next_spur].first, junction,
							true, inRoads.junctions[junction].spurs[next_spur].second, intersect_pt);
				
			} else {
				junctionShape.push_back(our_left.p2);
				if (our_left.p2 != following_right.p2)
					junctionShape.push_back(following_right.p2);
			}			
		}
		if (junctionShape.size() < 3)
			junctionShape.clear();
		polyjunctions.push_back(junctionShape);
	}	

	//	4. Extrusion of the straight road onto the quad strips

	for (chain = 0; chain < inRoads.chains.size(); ++chain)
	{
		// At this point we should really go back and retroactively hack curvedchains
		// to make sure our middle point matches where we've been reduced to.
		
		int	qcl = quadchains[chain].size() - 1;
		int ccl = curvedchains[chain].size() - 1;
		
		InterpPt3(quadchains[chain][0], quadchains[chain][1], 0.5, curvedchains[chain][0]);
		InterpPt3(quadchains[chain][qcl], quadchains[chain][qcl-1], 0.5, curvedchains[chain][ccl]);

		vector<int>	frontJunction, backJunction;

		me_start = NetworkJunction_t::spur(chain, false);
		me_end = NetworkJunction_t::spur(chain, true);
		
		inRoads.get_junction_types(me_start, frontJunction);
		inRoads.get_junction_types(me_end, backJunction);
		
		for (int L = 0; L < inDefs.segments[inRoads.chains[chain].chain_type].lod.size(); ++L)
		{
			NetworkSegmentLOD_t	frontCapDef, backCapDef;
			
			inDefs.find_caps(frontJunction, backJunction, 
							inDefs.segments[inRoads.chains[chain].chain_type].lod[L].lod_near,
							inDefs.segments[inRoads.chains[chain].chain_type].lod[L].lod_far,
							frontCapDef, backCapDef);

			ExtrudeRoadChain(quadchains[chain], chainups[chain],
						inDefs.segments[inRoads.chains[chain].chain_type].lod[L],
						frontCapDef, backCapDef,
					extrudeFunc, objReceiveFunc, inRef1, inRef2);
		}
	}

	//	5. Extrusion of the intersections onto the polygons
	for (junction = 0; junction < inRoads.junctions.size(); ++junction)
	{
		vector<int>	ourJunction;
		inRoads.get_junction_types(junction, ourJunction);
		JunctionRule_t	rule;
		
		Point3	centroid = inRoads.junctions[junction].location;
		
		if (inDefs.find_junction(ourJunction, rule))
		{
			ExtrudeSpecializedJunction(polyjunctions[junction], centroid, rule, extrudeFunc, objReceiveFunc, inRef1, inRef2);
		} else {
			vector<JunctionRule_t>	rules;
			for (int n = 0; n < ourJunction.size(); ++n)
			{
				if (inDefs.find_junction_part(ourJunction[n], rule))
					rules.push_back(rule);
			}
			if (rules.size() == ourJunction.size())
				ExtrudeGeneralJunction(polyjunctions[junction], centroid, rules, extrudeFunc, objReceiveFunc, inRef1, inRef2);
		}
	}
}

