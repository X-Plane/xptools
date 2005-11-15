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
#include "ObjUtils.h"
#include "XUtils.h"
#include "XObjReadWrite.h"
#include "CompGeomUtils.h"
#include "MatrixUtils.h"

enum {
	xyz = 0,
	Xyz,
	xYz,
	XYz,
	xyZ,
	XyZ,
	xYZ,
	XYZ	
};	

#if 0


inline void Rescale2f(float	vec[2], float inset)
{
	float	dist = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	float	factor = inset / dist;
	vec[0] *= factor;
	vec[1] *= factor;
}

static	void	CalcRepeats(const vector<int>	objRepeatPattern,
							int					desiredTotal,
							vector<int>&		usePattern)
{
	int		total_fixed = 0;
	int		total_repeat = 0;
	
	for (vector<int>::const_iterator i = objRepeatPattern.begin(); i != objRepeatPattern.end(); ++i)
	{
		if ((*i) == 0)
			total_fixed++;
		else
			total_repeat += *i;
	}
	
	int		variable_len = desiredTotal - total_fixed;
	
	int		clean_repeats = variable_len / total_repeat;
	
	int		slop = (variable_len - (total_repeat * clean_repeats));
	
	for (vector<int>::const_iterator i = objRepeatPattern.begin(); i != objRepeatPattern.end(); ++i)
	{
		if ((*i) == 0)
		{
			usePattern.push_back(1);
		} else {
			if (slop > 0)
			{
				usePattern.push_back((*i) * clean_repeats + 1);
				slop--;
			} else
				usePattern.push_back((*i) * clean_repeats);
		}
	}	
}							
							
static	void	CalcRepeatsWeighted(const vector<pair<int, float> >&	objRepeatPattern,
							float				desiredTotal,
							vector<int>&			usePattern)
{
	float		total_fixed = 0.0;
	float		total_repeat = 0.0;
	
	for (vector<pair<int, float> >::const_iterator i = objRepeatPattern.begin(); i != objRepeatPattern.end(); ++i)
	{
		if ((i->first) == 0)
			total_fixed += i->second;
		else
			total_repeat += ((float) i->first * i->second);
	}
	
	float		variable_len = desiredTotal - total_fixed;
	
	int			clean_repeats = variable_len / total_repeat;
	
	float		slop = (variable_len - ((float) total_repeat * clean_repeats));
	
	for (vector<pair<int, float> >::const_iterator i = objRepeatPattern.begin(); i != objRepeatPattern.end(); ++i)
	{
		if ((i->first) == 0)
		{
			usePattern.push_back(1);
		} else {
			if (slop > (0.5 * i->second))
			{
				usePattern.push_back((i->first) * clean_repeats + 1);
				slop -= (i->second);
			} else
				usePattern.push_back((i->first) * clean_repeats);
		}
	}	
}

static	void	ApplySegment(
							const ProtoSeg_t&					inSegment,
							float								inStartBot[3],
							float								inEndTop[3],
							XObj&								outObject)
{
	float	objMin[3], objMax[3];
	float	theBox[8][3];
	
	GetObjDimensions(inSegment.obj, objMin, objMax);
	
	// Important: we set our dimensions in the Y to be the same as X.  This makes our bounding box look square
	// from the top.  Since our destination box is like this, we must have this property!
	float	range = objMax[0] - objMin[0];
	float	center = (objMax[2] + objMin[2]) * 0.5;	
	center = 0.0;	// LOCK to Z axis for now!  This is necessary for asymetric weighting
	objMin[2] = center - range;
	objMax[2] = center + range;

	// Build a destination box (square in size, specified height) to plop it in
	ExtrudeBoxZ(inStartBot, inEndTop, theBox);

	XObj	changed = inSegment.obj;

	// And....splat.
	ConformObjectToBox(changed, objMin, objMax, theBox);

	outObject.texture = changed.texture;
	outObject.cmds.insert(outObject.cmds.end(), changed.cmds.begin(), changed.cmds.end());
}							

static	void	BuildWall(
							const ProtoWall_t&					inWall,
							float								inStartBot[3],
							float								inEndTop[3],
							XObj&								outObject)
{
	vector<float>				seg_lengths;
	vector<pair<int,float> >	repeatPattern;
	vector<int>					usagePattern;

	float						objMin[3], objMax[3];
	
	float						wall_vec[2];
	wall_vec[0] = inEndTop[0] - inStartBot[0];
	wall_vec[1] = inEndTop[2] - inStartBot[2];
	float						start_pt[02];
	start_pt[0] = inStartBot[0];
	start_pt[1] = inStartBot[2];
		
	float	total_wall_len = sqrt((wall_vec[0])*(wall_vec[0]) + 
								  (wall_vec[1])*(wall_vec[1]));
		
	for (SegVector_t::const_iterator seg = inWall.segments.begin(); 
		seg != inWall.segments.end(); ++seg)
	{
		GetObjDimensions(seg->obj, objMin, objMax);
		seg_lengths.push_back(objMax[0] - objMin[0]);
		
		repeatPattern.push_back(pair<int, float>(seg->repeats, objMax[0] - objMin[0]));
	}
	
	CalcRepeatsWeighted(repeatPattern, total_wall_len, usagePattern);

	float		ideal_wall_len = 0.0;
	for (int n = 0; n < inWall.segments.size(); ++n)
	{
		ideal_wall_len += ((float) usagePattern[n] * seg_lengths[n]);
	}
	
	float		scaleFactor = total_wall_len / ideal_wall_len;
	
	for (int n = 0; n < inWall.segments.size(); ++n)
	{
		for (int c = 0; c < usagePattern[n]; ++c)
		{
			float	segLen = seg_lengths[n];
			segLen *= scaleFactor;
			Rescale2f(wall_vec, segLen);
			
			float pt1[3], pt2[3];
			pt1[0] = start_pt[0];
			pt1[1] = inStartBot[1];
			pt1[2] = start_pt[1];
			pt2[0] = start_pt[0] + wall_vec[0];
			pt2[1] = inEndTop[1];
			pt2[2] = start_pt[1] + wall_vec[1];

			ApplySegment(inWall.segments[n], pt1, pt2, outObject);
			
			start_pt[0] += wall_vec[0];
			start_pt[1] += wall_vec[1];
			
		}
	}
}							

static	void	BuildLayer(
							const ProtoLayer_t&					inLayer,
					   		const Polygon2& 					inPoints,
					   		bool								inClosed,
					   		float								inHeight,
						   XObj&							    outObject)
{
	if (inLayer.layer_type == layer_FinalRoof)
	{
		Polygon2		pts;
		
		InsetPolygon2(inPoints, NULL, inLayer.bottom_inset, inClosed, pts);
		
		XObjCmd	cmd;
		cmd.cmdType = type_Poly;
		cmd.cmdID = obj_Polygon;
		if (pts.size() == 3)
			cmd.cmdID = obj_Tri;
		if (pts.size() == 4)
			cmd.cmdID = obj_Quad;
			
		vec_tex	v;
		v.st[0] = 0.0; v.st[1] = 0.0;
		
		for (int n = 0; n < pts.size(); ++n)
		{
			v.v[0] = pts[n].x;
			v.v[1] = inHeight;
			v.v[2] = pts[n].y;		
			cmd.st.push_back(v);
		}
		outObject.cmds.push_back(cmd);
	}

	if (inLayer.layer_type == layer_Roof)
	{
		Polygon2		bottomPts, topPts;
		
		InsetPolygon2(inPoints, NULL, inLayer.bottom_inset, inClosed, bottomPts);
		InsetPolygon2(inPoints, NULL, inLayer.top_inset, inClosed, topPts);
		
		// If the top is inside the bottom, it's a roof.  Otherwise it's a floor.
		
		XObjCmd	cmd;
		cmd.cmdType = type_Poly;
		cmd.cmdID = obj_Quad_Strip;

			vec_tex	v;
		v.st[0] = 0.0; v.st[1] = 0.0;
		
		for (int n = 0; n < bottomPts.size(); ++n)
		{			
			v.v[0] = topPts[n].x;
			v.v[1] = inHeight + inLayer.height;
			v.v[2] = topPts[n].y;		
			cmd.st.push_back(v);

			v.v[0] = bottomPts[n].x;
			v.v[1] = inHeight;
			v.v[2] = bottomPts[n].y;
			cmd.st.push_back(v);			
		}
		
		if (inClosed)
		{
			v.v[0] = topPts[0].x;
			v.v[1] = inHeight + inLayer.height;
			v.v[2] = topPts[0].y;			
			cmd.st.push_back(v);

			v.v[0] = bottomPts[0].x;
			v.v[1] = inHeight;
			v.v[2] = bottomPts[0].y;
			cmd.st.push_back(v);		
		}	
		
		outObject.cmds.push_back(cmd);

	}

	if (inLayer.layer_type == layer_Facade)
	{
		int	sides = inPoints.size() - (inClosed ? 0 : 1);
		Polygon2		pts;		
		InsetPolygon2(inPoints, NULL, inLayer.bottom_inset, inClosed, pts);
				
		for (int n = 0; n < sides; ++n)
		{
			int next = n+1;
			if (next == inPoints.size())
				next = 0;
				
			float	startPt[3], endPt[3];
			startPt[0] = pts[n].x;
			startPt[1] = inHeight;
			startPt[2] = pts[n].y;

			endPt[0] = pts[next].x;
			endPt[1] = inHeight + inLayer.height;
			endPt[2] = pts[next].y;
		
			BuildWall(inLayer.walls[n % inLayer.walls.size()],
						startPt, 
						endPt, 
						outObject);			
		}
	}
}						   

void	ApplyPrototype(const Prototype_t& 					inPrototype,
					   const Polygon2& 						inPoints,
					   int									inFloors,
					   XObj&							    outObject)
{
	outObject.cmds.clear();
	
	vector<int>	repeatPattern, usagePattern;

	for (LayerVector_t::const_iterator layer = inPrototype.layers.begin(); 
		layer != inPrototype.layers.end(); ++layer)
	{
		repeatPattern.push_back(layer->repeats);
	}
	
	CalcRepeats(repeatPattern, inFloors, usagePattern);

	double	curHeight = 0.0;
	
	for (int n = 0; n < inPrototype.layers.size(); ++n)
	{
		for (int c = 0; c < usagePattern[n]; ++c)
		{
			BuildLayer(inPrototype.layers[n],
						inPoints,
						inPrototype.closed,
						curHeight,
						outObject);
						
			curHeight += inPrototype.layers[n].height;			
		}
	}
}
#endif					   

#pragma mark -

void	GetObjBoundingSphere(const XObj& inObj, float outSphere[4])
{
	vector<Point3>	pts;
	
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			pts.push_back(Point3(st->v[0], st->v[1], st->v[2]));
		}
		
		for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			pts.push_back(Point3(rgb->v[0], rgb->v[1], rgb->v[2]));
		}
	}

	Sphere3	sphere;
	FastBoundingSphere(pts, sphere);
	outSphere[0] = sphere.c.x;
	outSphere[1] = sphere.c.y;
	outSphere[2] = sphere.c.z;
	outSphere[3] = sphere.radius_squared;
}

void	GetObjBoundingSphere8(const XObj8& inObj, float outSphere[4])
{
	vector<Point3>	pts;
	
	int 	n;
	const  float * d;
	for (n = 0; n < inObj.geo_tri.count(); ++n)
	{
		d = inObj.geo_tri.get(n);
		pts.push_back(Point3(d[0], d[1], d[2]));
	}
	for (n = 0; n < inObj.geo_lines.count(); ++n)
	{
		d = inObj.geo_lines.get(n);
		pts.push_back(Point3(d[0], d[1], d[2]));
	}
	for (n = 0; n < inObj.geo_lights.count(); ++n)
	{
		d = inObj.geo_lights.get(n);
		pts.push_back(Point3(d[0], d[1], d[2]));
	}
	
	Sphere3	sphere;
	FastBoundingSphere(pts, sphere);
	outSphere[0] = sphere.c.x;
	outSphere[1] = sphere.c.y;
	outSphere[2] = sphere.c.z;
	outSphere[3] = sphere.radius_squared;
}


void OffsetObject(XObj& ioObj, double x, double y, double z)
{
	for (vector<XObjCmd>::iterator cmd = ioObj.cmds.begin(); cmd != ioObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			st->v[0] += x;
			st->v[1] += y;
			st->v[2] += z;
		}
		
		for (vector<vec_rgb>::iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			rgb->v[0] += x;
			rgb->v[1] += y;
			rgb->v[2] += z;
		}
	}
}

#pragma mark -

void	GetObjDimensions(const XObj& inObj,
						float	minCoords[3],
						float	maxCoords[3])
{	
	minCoords[0] = minCoords[1] = minCoords[2] =  0.0;
	maxCoords[0] = maxCoords[1] = maxCoords[2] =  0.0;

	bool assigned = false;
	
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			if (!assigned)
			{
				for (int n = 0; n < 3; ++n)
				{
					minCoords[n] = st->v[n];
					maxCoords[n] = st->v[n];
				}
				assigned = true;
			}
			
			for (int n = 0; n < 3; ++n)
			{
				minCoords[n] = min(minCoords[n], st->v[n]);
				maxCoords[n] = max(maxCoords[n], st->v[n]);
			}
		}
		
		for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			if (!assigned)
			{
				for (int n = 0; n < 3; ++n)
				{
					minCoords[n] = rgb->v[n];
					maxCoords[n] = rgb->v[n];
				}
				assigned = true;
			}
		
			for (int n = 0; n < 3; ++n)
			{
				minCoords[n] = min(minCoords[n], rgb->v[n]);
				maxCoords[n] = max(maxCoords[n], rgb->v[n]);
			}
		}
	}
}

void	GetObjDimensions8(const XObj8& inObj,
						float	minCoords[3],
						float	maxCoords[3])
{	
	float	mintemp[3], maxtemp[3];
	
	bool has = false;
	
	if (inObj.geo_tri.count() > 0)
	{
		inObj.geo_tri.get_minmax(minCoords, maxCoords);
		has = true;
	} else {
		minCoords[0] = minCoords[1] = minCoords[2] = maxCoords[0] = maxCoords[1] = maxCoords[2] = 0;
	}

	if (inObj.geo_lines.count() > 0)
	{
		if (has) {
			inObj.geo_lines.get_minmax(mintemp, maxtemp);
			minCoords[0] = min(minCoords[0], mintemp[0]);	maxCoords[0] = max(maxCoords[0], maxtemp[0]);
			minCoords[1] = min(minCoords[1], mintemp[1]);	maxCoords[0] = max(maxCoords[1], maxtemp[1]);
			minCoords[2] = min(minCoords[2], mintemp[2]);	maxCoords[0] = max(maxCoords[2], maxtemp[2]);
		} else 
			inObj.geo_lines.get_minmax(minCoords, maxCoords);
		has = true;
	}

	if (inObj.geo_lights.count() > 0)
	{
		if (has) {
			inObj.geo_lights.get_minmax(mintemp, maxtemp);
			minCoords[0] = min(minCoords[0], mintemp[0]);	maxCoords[0] = max(maxCoords[0], maxtemp[0]);
			minCoords[1] = min(minCoords[1], mintemp[1]);	maxCoords[0] = max(maxCoords[1], maxtemp[1]);
			minCoords[2] = min(minCoords[2], mintemp[2]);	maxCoords[0] = max(maxCoords[2], maxtemp[2]);
		} else 
			inObj.geo_lights.get_minmax(minCoords, maxCoords);
		has = true;
	}
}




#if 0
// Given two points that will be the minimum and max X
// locations for a given object at the min and max Y locations
// this routine extrudes them in an axis opposite the wall 
// line to make sure X-Z coordinates are square.
void	ExtrudeBoxZ(float minCorner[3], float maxCorner[3],
					float outNewCoords[8][3])
{
	float		main_line[3];
	float		half_line[3];
	float		half_right[3];
	float		half_left[3];
	
	main_line[0] = maxCorner[0] - minCorner[0];
	main_line[2] = maxCorner[2] - minCorner[2];
	half_line[0] = main_line[0] * 0.5;
	half_line[2] = main_line[2] * 0.5;
	
	half_right[0] = -half_line[2];
	half_right[2] = half_line[0];	
	half_left[0] = half_line[2];
	half_left[2] = -half_line[0];
	
	outNewCoords[xyz][0] = minCorner[0] + half_left[0];
	outNewCoords[xyz][1] = minCorner[1];
	outNewCoords[xyz][2] = minCorner[2] + half_left[2];

	outNewCoords[xyZ][0] = minCorner[0] + half_right[0];
	outNewCoords[xyZ][1] = minCorner[1];
	outNewCoords[xyZ][2] = minCorner[2] + half_right[2];

	outNewCoords[Xyz][0] = maxCorner[0] + half_left[0];
	outNewCoords[Xyz][1] = minCorner[1];
	outNewCoords[Xyz][2] = maxCorner[2] + half_left[2];

	outNewCoords[XyZ][0] = maxCorner[0] + half_right[0];
	outNewCoords[XyZ][1] = minCorner[1];
	outNewCoords[XyZ][2] = maxCorner[2] + half_right[2];

	outNewCoords[xYz][0] = minCorner[0] + half_left[0];
	outNewCoords[xYz][1] = maxCorner[1];
	outNewCoords[xYz][2] = minCorner[2] + half_left[2];

	outNewCoords[xYZ][0] = minCorner[0] + half_right[0];
	outNewCoords[xYZ][1] = maxCorner[1];
	outNewCoords[xYZ][2] = minCorner[2] + half_right[2];

	outNewCoords[XYz][0] = maxCorner[0] + half_left[0];
	outNewCoords[XYz][1] = maxCorner[1];
	outNewCoords[XYz][2] = maxCorner[2] + half_left[2];

	outNewCoords[XYZ][0] = maxCorner[0] + half_right[0];
	outNewCoords[XYZ][1] = maxCorner[1];
	outNewCoords[XYZ][2] = maxCorner[2] + half_right[2];	
}					

#endif

static	inline	float	Interp2d(
							float		val,
							float		inMin,
							float		inMax,
							float		outMin,
							float		outMax)
{
	return outMin + ((val - inMin) * (outMax - outMin) / (inMax - inMin));
}							

// Given a point in an old cartesian bounding box and a new
// bounding box, calc its position
static	void	Interp3d(
							float		inMinCoords[3],
							float		inMaxCoords[3],
							float		inNewCoords[8][3],
							float		inOldPt[3],
							float		outNewPt[3])
{
	// X Coordinate
	
	float	xBotBack =  Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], inNewCoords[xyz][0], inNewCoords[Xyz][0]);
	float	xBotFront = Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], inNewCoords[xyZ][0], inNewCoords[XyZ][0]);
	float	xTopBack =  Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], inNewCoords[xYz][0], inNewCoords[XYz][0]);
	float	xTopFront = Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], inNewCoords[xYZ][0], inNewCoords[XYZ][0]);
	
	float	xBack  = Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], xBotBack,  xTopBack);
	float	xFront = Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], xBotFront, xTopFront);
	
	outNewPt[0] = Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], xBack, xFront);
	
	// Y Coordinate
	float	yLeftBack =   Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], inNewCoords[xyz][1], inNewCoords[xYz][1]);
	float	yRightBack =  Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], inNewCoords[Xyz][1], inNewCoords[XYz][1]);
	float	yLeftFront =  Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], inNewCoords[xyZ][1], inNewCoords[xYZ][1]);
	float	yRightFront = Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], inNewCoords[XyZ][1], inNewCoords[XYZ][1]);
	
	float	yBack =  Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], yLeftBack,  yRightBack);
	float	yFront = Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], yLeftFront, yRightFront);
	
	outNewPt[1] = Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], yBack, yFront);
	
	// Z coordinate
	float	zLeftBottom =  Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], inNewCoords[xyz][2], inNewCoords[xyZ][2]);
	float	zRightBottom = Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], inNewCoords[Xyz][2], inNewCoords[XyZ][2]);
	float	zLeftTop =     Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], inNewCoords[xYz][2], inNewCoords[xYZ][2]);
	float	zRightTop =    Interp2d(inOldPt[2], inMinCoords[2], inMaxCoords[2], inNewCoords[XYz][2], inNewCoords[XYZ][2]);
	
	float	zBottom = Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], zLeftBottom, zRightBottom);
	float	zTop = 	  Interp2d(inOldPt[0], inMinCoords[0], inMaxCoords[0], zLeftTop,    zRightTop);
	
	outNewPt[2] = Interp2d(inOldPt[1], inMinCoords[1], inMaxCoords[1], zBottom, zTop);	
}							

// New coords go xyz Xyz xYz XYz xyZ Xyz xYZ XYZ
void	ConformObjectToBox( XObj&		obj,
							float		inMinCoords[3],
							float		inMaxCoords[3],
							float		inNewCoords[8][3])
{
	// Do an interp3d on every S, T, RGB pt
			float	p[3];
	
	for (vector<XObjCmd>::iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			Interp3d(inMinCoords, inMaxCoords, inNewCoords, st->v, p);
			st->v[0] = p[0];
			st->v[1] = p[1];
			st->v[2] = p[2];
		}
		
		for (vector<vec_rgb>::iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			Interp3d(inMinCoords, inMaxCoords, inNewCoords, rgb->v, p);
			rgb->v[0] = p[0];
			rgb->v[1] = p[1];
			rgb->v[2] = p[2];
		}
	}
}

#if 0
bool	LoadPrototype(const char * inFileName, Prototype_t& outProto)
{
	outProto.layers.clear();
	bool ok = false;
	
	StTextFileScanner	scanner(inFileName, true);
	string	s;
	while (GetNextNoComments(scanner, s))
	{
		vector<string>	v;
		BreakString(s, v);
		if (!v.empty())
		{
			if (v[0] == "PROTOTYPE")
			{
				if (v.size() > 1)
				{
					outProto.closed = (v[1] == "CLOSED");
					ok = true;
				}
			}
			if (v[0] == "LAYER")
			{
				if (v.size() > 1)
				{
					ProtoLayer_t	layer;
					
					if (v[1] == "FACADE" && (v.size() > 4))
					{
						layer.layer_type = layer_Facade;
						layer.bottom_inset = atof(v[2].c_str());
						layer.height = atof(v[3].c_str());
						layer.repeats = atoi(v[4].c_str());						
					}
					if (v[1] == "ROOF" && (v.size() > 5))
					{
						layer.layer_type = layer_Roof;
						layer.bottom_inset = atof(v[2].c_str());
						layer.top_inset = atof(v[3].c_str());
						layer.height = atof(v[4].c_str());
						layer.repeats = atoi(v[5].c_str());						
					}
					if (v[1] == "FINALROOF" && (v.size() > 2))
					{
						layer.layer_type = layer_FinalRoof;
						layer.bottom_inset = atof(v[2].c_str());
						layer.repeats = 0;
					}
					outProto.layers.push_back(layer);
				} else
					ok = false;
			}
			if (v[0] == "WALL")
			{
				if (!outProto.layers.empty())
				{
					ProtoLayer_t&	l = outProto.layers[outProto.layers.size() - 1];
					l.walls.push_back(ProtoWall_t());
				} else ok = false;
			}
			if (v[0] == "SEGMENT" && v.size() > 2)
			{
				ProtoSeg_t seg;
				seg.repeats = atoi(v[2].c_str());
				string	fpath = inFileName;
				string	fname = fpath;
				StripPath(fname);
				fpath = fpath.substr(0, fpath.size() - fname.size());
				fpath += v[1];
				XObjRead(fpath.c_str(), seg.obj);
				seg.name = v[1];
				
				if (!outProto.layers.empty())
				{
					ProtoLayer_t&	l = outProto.layers[outProto.layers.size() - 1];
					if (!l.walls.empty())
					{
						ProtoWall_t& w = l.walls[l.walls.size() - 1];
						w.segments.push_back(seg);
					} else ok = false;					
				} else ok = false;
			}
		}
	}
	return ok;
}

bool	SavePrototype(const char * inFileName, const Prototype_t& outProto)
{
	FILE * fi = fopen(inFileName, "w");
	if (!fi) return false;
	fprintf(fi,"PROTOTYPE %s" CRLF,outProto.closed ? "CLOSED" : "OPEN");
	for (LayerVector_t::const_iterator layer = outProto.layers.begin();
		layer != outProto.layers.end(); ++layer)
	{
		switch(layer->layer_type) {
		case layer_Facade:
			fprintf(fi,"   LAYER FACADE %f %f %d" CRLF, layer->bottom_inset, layer->height, layer->repeats);
			for (WallVector_t::const_iterator wall = layer->walls.begin(); wall != layer->walls.end(); ++wall)
			{
				fprintf(fi, "      WALL" CRLF);
				for (SegVector_t::const_iterator seg = wall->segments.begin(); seg != wall->segments.end(); ++seg)
				{
					fprintf(fi,"         SEGMENT %s %d" CRLF, seg->name.c_str(), seg->repeats);
				}
			}
			break;
		case layer_Roof:
			fprintf(fi,"   LAYER ROOF %f %f %f %d" CRLF, layer->bottom_inset, layer->top_inset, layer->height, layer->repeats);
			break;
		case layer_FinalRoof:
			fprintf(fi,"   LAYER FINALROOF %f" CRLF, layer->bottom_inset);
			break;
		}
	}

	fprintf(fi,"END" CRLF);	
	fclose(fi);
	return true;
}
#endif

void	ExtrudeFuncToObj(int polyType, int count, float * pts, float * sts, float LOD_near, float LOD_far, void * inRef)
{
	XObj * obj = (XObj *) inRef;
	float	last_near = -1;
	float	last_far = -1;
	for (vector<XObjCmd>::reverse_iterator riter = obj->cmds.rbegin(); riter != obj->cmds.rend(); ++riter)
	{
		if (riter->cmdType == type_Attr && riter->cmdID == attr_LOD)
		{
			last_near = riter->attributes[0];
			last_far = riter->attributes[1];
			break;
		}
	}
	
	if (last_near != LOD_near || last_far != LOD_far)
	{
		XObjCmd	lod;
		lod.cmdType = type_Attr;
		lod.cmdID = attr_LOD;
		lod.attributes.push_back(LOD_near);
		lod.attributes.push_back(LOD_far);
		obj->cmds.push_back(lod);
	}
	
	XObjCmd	cmd;
	cmd.cmdType = type_Poly;
	switch(polyType) {
	case ext_Poly_Tri:			cmd.cmdID = obj_Tri;			break;
	case ext_Poly_TriStrip:		cmd.cmdID = obj_Tri_Strip;		break;
	case ext_Poly_TriFan:		cmd.cmdID = obj_Tri_Fan;		break;
	case ext_Poly_Quad:			cmd.cmdID = obj_Quad;			break;
	case ext_Poly_QuadStrip:	cmd.cmdID = obj_Tri_Strip;		break;
	}
	for (int i = 0; i < count; ++i)
	{
		vec_tex	v;
		v.v[0] = pts[i*3  ];
		v.v[1] = pts[i*3+1];
		v.v[2] = pts[i*3+2];
		v.st[0] = sts[i*2  ];
		v.st[1] = sts[i*2+1];
		cmd.st.push_back(v);
	}
	obj->cmds.push_back(cmd);
}

void	DecomposeObjCmd(const XObjCmd& inCmd, vector<XObjCmd>& outCmds, int maxValence)
{
	XObjCmd	c;
	c.cmdType = type_Poly;
	c.cmdID = obj_Tri;
	switch(inCmd.cmdID) {
	case obj_Tri:
		// Triangles never need breaking down.
		outCmds.push_back(inCmd);
		break;
	case obj_Quad:
	case obj_Quad_Hard:
	case obj_Smoke_Black:
	case obj_Smoke_White:
	case obj_Movie:
		// Quads - split into triangles if necessary.
		if (maxValence > 3) {
			outCmds.push_back(inCmd);
			outCmds.back().cmdID = obj_Quad;
		} else {
			outCmds.push_back(inCmd);
			outCmds.back().cmdID = obj_Tri;
			outCmds.back().st.erase(outCmds.back().st.begin()+3);
			outCmds.push_back(inCmd);			
			outCmds.back().cmdID = obj_Tri;
			outCmds.back().st.erase(outCmds.back().st.begin()+1);
		}
		break;
	case obj_Polygon:
		// Polygons might be ok.  But if we have to break them down,
		// we generate N-2 triangles in a fan configuration.
		if (maxValence < inCmd.st.size())
		{
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			for (int n = 2; n < inCmd.st.size(); ++n)
			{
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		} else 
			outCmds.push_back(inCmd);
		break;
	case obj_Tri_Strip:
		// Triangle strips - every other triangle's vertices
		// are backward!
		c.st.push_back(inCmd.st[0]);
		c.st.push_back(inCmd.st[1]);
		c.st.push_back(inCmd.st[2]);
		for (int n = 2; n < inCmd.st.size(); ++n)
		{
			if (n%2)
			{
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n  ];
				c.st[2] = inCmd.st[n-1];
				outCmds.push_back(c);
			} else {
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		}
		break;
	case obj_Tri_Fan:
		// Tri fan - run around the triangle fan emitting triangles.
		c.st.push_back(inCmd.st[0]);
		c.st.push_back(inCmd.st[1]);
		c.st.push_back(inCmd.st[2]);
		for (int n = 2; n < inCmd.st.size(); ++n)
		{
			c.st[1] = inCmd.st[n-1];
			c.st[2] = inCmd.st[n  ];
			outCmds.push_back(c);
		}
		break;
	case obj_Quad_Strip:
		// Quad strips can become either quads or triangles!!
		if (maxValence > 3)
		{
			c.cmdID = obj_Quad;
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			c.st.push_back(inCmd.st[3]);
			for (int n = 2; n < inCmd.st.size(); n += 2)
			{
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n+1];
				c.st[3] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		} else {
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			for (int n = 2; n < inCmd.st.size(); ++n)
			{
				if (n%2)
				{
					c.st[0] = inCmd.st[n-2];
					c.st[1] = inCmd.st[n  ];
					c.st[2] = inCmd.st[n-1];
					outCmds.push_back(c);
				} else {
					c.st[0] = inCmd.st[n-2];
					c.st[1] = inCmd.st[n-1];
					c.st[2] = inCmd.st[n  ];
					outCmds.push_back(c);
				}
			}
		}
		break;
	default:
		outCmds.push_back(inCmd);
	}
}

void	DecomposeObj(const XObj& inObj, XObj& outObj, int maxValence)
{
	outObj.cmds.clear();
	outObj.texture = inObj.texture;
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); 
		cmd != inObj.cmds.end(); ++cmd)
	{
		vector<XObjCmd>		newCmds;
		DecomposeObjCmd(*cmd, newCmds, maxValence);
		outObj.cmds.insert(outObj.cmds.end(), newCmds.begin(), newCmds.end());
	}
}

void	ChangePolyCmdCW(XObjCmd& ioCmd)
{
	vector<vec_tex>	v;
	for (vector<vec_tex>::reverse_iterator riter = ioCmd.st.rbegin();
		riter != ioCmd.st.rend(); ++riter)
	{
		v.push_back(*riter);
	}
	ioCmd.st = v;
}

double	GetObjRadius(const XObj& inObj)
{
	double	dist = 0, d;
	for (vector<XObjCmd>::const_iterator c = inObj.cmds.begin();
		c != inObj.cmds.end(); ++c)	
	{
		for (vector<vec_tex>::const_iterator v = c->st.begin();
			v != c->st.end(); ++v)
		{
			d = sqrt(v->v[0] * v->v[0] +
					 v->v[1] * v->v[1] +
					 v->v[2] * v->v[2]);
			if (d > dist) dist = d;
		}
		
		for (vector<vec_rgb>::const_iterator p = c->rgb.begin();
			p != c->rgb.end(); ++p)
		{
			d = sqrt(p->v[0] * p->v[0] +
					 p->v[1] * p->v[1] +
					 p->v[2] * p->v[2]);
			if (d > dist) dist = d;
		}
	}
	return dist;
}

double	GetObjRadius8(const XObj8& inObj)
{
	double	dist = 0, d;
	int		n;
	const float * p;
	for (n = 0; n < inObj.geo_tri.count(); ++n)
	{
		p = inObj.geo_tri.get(n);
		d = sqrt(p[0] * p[0] +
				 p[1] * p[1] +
				 p[2] * p[2]);
		if (d > dist) dist = d;
	}

	for (n = 0; n < inObj.geo_lines.count(); ++n)
	{
		p = inObj.geo_lines.get(n);
		d = sqrt(p[0] * p[0] +
				 p[1] * p[1] +
				 p[2] * p[2]);
		if (d > dist) dist = d;
	}

	for (n = 0; n < inObj.geo_lights.count(); ++n)
	{
		p = inObj.geo_lights.get(n);
		d = sqrt(p[0] * p[0] +
				 p[1] * p[1] +
				 p[2] * p[2]);
		if (d > dist) dist = d;
	}

	return dist;
}

