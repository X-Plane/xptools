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

#pragma mark -

void	GetObjBoundingSphere(const XObj& inObj, float outSphere[4])
{
	float minxyz[3] = {  9.9e9, 9.9e9, 9.9e9 };
	float maxxyz[3] = { -9.9e9,-9.9e9,-9.9e9 };

	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			minxyz[0] = min(minxyz[0], st->v[0]);
			minxyz[1] = min(minxyz[1], st->v[1]);
			minxyz[2] = min(minxyz[2], st->v[2]);
			maxxyz[0] = max(maxxyz[0], st->v[0]);
			maxxyz[1] = max(maxxyz[1], st->v[1]);
			maxxyz[2] = max(maxxyz[2], st->v[2]);
		}

		for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			minxyz[0] = min(minxyz[0], rgb->v[0]);
			minxyz[1] = min(minxyz[1], rgb->v[1]);
			minxyz[2] = min(minxyz[2], rgb->v[2]);
			maxxyz[0] = max(maxxyz[0], rgb->v[0]);
			maxxyz[1] = max(maxxyz[1], rgb->v[1]);
			maxxyz[2] = max(maxxyz[2], rgb->v[2]);
		}
	}

	outSphere[0] = (minxyz[0]+maxxyz[0]) * 0.5;
	outSphere[1] = (minxyz[1]+maxxyz[1]) * 0.5;
	outSphere[2] = (minxyz[2]+maxxyz[2]) * 0.5;
	outSphere[3] = (minxyz[0]-outSphere[0])*(minxyz[0]-outSphere[0])+
				   (minxyz[1]-outSphere[1])*(minxyz[1]-outSphere[1])+
				   (minxyz[2]-outSphere[2])*(minxyz[2]-outSphere[2]);
}
/*

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
*/

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

