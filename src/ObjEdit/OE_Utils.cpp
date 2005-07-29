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
#include "OE_Utils.h"
#include "XObjDefs.h"
#include "GeoUtils.h"
#include "MatrixUtils.h"
#include "CompGeomUtils.h"

void	OE_FindDupes(
				const XObj& 		inObj,
				const set<int> * 	inSearch,	// Can be NULL
				set<int>&			outFound);

void	OE_MergeObject(
				const ObjectTable&	inObjects,
				const LODTable&		inLOD,
				XObj&				outObj)
{
	outObj.cmds.clear();
	outObj.texture = inObjects.empty() ? string() : inObjects[0].texture;
	
	for (int n = 0; n < inObjects.size(); ++n)
	{
		
		XObjCmd	lodCmd;
		lodCmd.cmdType = type_Attr;
		lodCmd.cmdID = attr_LOD;
		lodCmd.attributes.push_back(inLOD[n].first);
		lodCmd.attributes.push_back(inLOD[n].second);
		if (inLOD[n].first != -1.0 || inLOD[n].second != -1.0)
			outObj.cmds.push_back(lodCmd);
		outObj.cmds.insert(outObj.cmds.end(), inObjects[n].cmds.begin(), inObjects[n].cmds.end());
	}
}
				
void	OE_SplitObj(
				const XObj&			inObj,
				ObjectTable&		outObjects,
				LODTable&			outLOD)
{
	outLOD.clear();
	outObjects.clear();
	
	for (int n = 0; n < inObj.cmds.size(); ++n)
	{
		if (inObj.cmds[n].cmdID == attr_LOD)
		{
			if (inObj.cmds[n].attributes.size() > 1)
			{
				XObj	newObj;
				newObj.texture = inObj.texture;
				outObjects.push_back(newObj);
				outLOD.push_back(LODRange(
					inObj.cmds[n].attributes[0],
					inObj.cmds[n].attributes[1]));
			}		
		} else {
			if (outObjects.empty())
			{
				XObj	newObj;
				newObj.texture = inObj.texture;
				outObjects.push_back(newObj);
				outLOD.push_back(LODRange(-1.0, -1.0));
			}
			outObjects.back().cmds.push_back(inObj.cmds[n]);;
		}
	}
}

int		OE_MaxSelected(void)
{
	if (gSelection.empty())
		return -1;
	set<int>::iterator i = gSelection.end();
	--i;
	return *i;
}

int		OE_NextPrevUntextured(int direction)
{
	if (gObjects.empty())	return 0;
	int	cmdCount = gObjects[gLevelOfDetail].cmds.size();
	if (cmdCount < 1)	return 0;
	
	int	start = 0;
	
	if (!gSelection.empty())
	{	
		if (direction > 0)
			start = OE_MaxSelected();
		else
			start = *gSelection.begin();
	}
	
	int	loop = start;
	do {
		loop += direction;
		if (loop < 0)
			loop = cmdCount - 1;
		if (loop >= cmdCount)
			loop = 0;
		if (!gObjects[gLevelOfDetail].cmds[loop].st.empty())
		if (OE_IsCleared(gObjects[gLevelOfDetail].cmds[loop]))
			return loop;
	} while (loop != start);
	return start;
}


void	OE_SelectByPixels(
				const XObj&				inObj,
				const vector<VisibleVector>&	inVisible,				
				double					inX1,
				double					inY1,
				double					inX2,
				double					inY2,
				set<int>&				outSel)
{
	outSel.clear();
	double	pt2[2], pt3[3];
	int n = 0;
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd, ++n)
	{
		bool	allIn = !cmd->st.empty() || !cmd->rgb.empty();
		for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			pt3[0] = st->v[0];
			pt3[1] = st->v[1];
			pt3[2] = st->v[2];
			ModelToScreenPt(pt3,pt2);
			if (pt2[0] < inX1 || pt2[0] > inX2 ||
				pt2[1] < inY1 || pt2[1] > inY2)
			{
				allIn = false;
			}
		}
		for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			pt3[0] = rgb->v[0];
			pt3[1] = rgb->v[1];
			pt3[2] = rgb->v[2];
			ModelToScreenPt(pt3,pt2);
			if (pt2[0] < inX1 || pt2[0] > inX2 ||
				pt2[1] < inY1 || pt2[1] > inY2)
			{
				allIn = false;
			}
		}
		if (allIn && !inVisible.empty())
		{
			bool	isVisible = false;
			for (VisibleVector::const_iterator i = inVisible[n].begin();
				i != inVisible[n].end(); ++i)
			{
				if (*i) 
					isVisible = true;
			}
			if (!isVisible)		
				allIn = false;
		}
		if (allIn)
			outSel.insert(n);

	}
}

int		OE_SelectByPoint(
				int						inBounds[4],
				const XObj&				inObj,
				const vector<VisibleVector>&	inVisible,
				double					inX,
				double					inY)
{
	vector<Polygon3Vector>	polygons;
	OE_DerivePolygons(inObj, polygons);
	
	int	found = -1;
	int	cur = 0;
	double	plane[4];
	double	hit_pt_model[4];
	double	hit_pt_eye[4];
	double	nearest = 0.0;
	GLdouble	model_view[16];

	gZoomer.SetupMatrices(inBounds);
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);	
	gZoomer.ResetMatrices();
	
	for (vector<Polygon3Vector>::iterator cmd = polygons.begin();
		cmd != polygons.end(); ++cmd, ++cur)
	{
		int polyNum = 0;
		for (Polygon3Vector::iterator poly = cmd->begin();
			poly != cmd->end(); ++poly, ++polyNum)
		{
			if (inVisible.empty() || inVisible[cur][polyNum])
			{
				Vector3	v1 = Vector3((*poly)[0], (*poly)[1]);
				Vector3	v2 = Vector3((*poly)[2], (*poly)[1]);
				Vector3	normal = v1.cross(v2);
				normal.normalize();
				plane[0] = normal.dx;
				plane[1] = normal.dy;
				plane[2] = normal.dz;
				plane[3] = -(normal.dot(Vector3((*poly)[0])));
				
				gZoomer.FindPointOnPlane(inBounds, plane, inX, inY, hit_pt_model);
				hit_pt_model[3] = 1.0;
				
				multMatrixVec(hit_pt_eye, model_view, hit_pt_model);
				
				if (PointInPolygon3(*poly, Point3(hit_pt_model[0], hit_pt_model[1],hit_pt_model[2])))
				{
					if (found == -1 || hit_pt_eye[2] > nearest)
					{
						found = cur;
						nearest = hit_pt_eye[2];
					}
				}
			}
		}
	}
	
	return found;
}				


#pragma mark -

void	OE_DerivePolygons(
				const XObj&					inObj,
				vector<Polygon3Vector>&		outPolyVectors)
{
	outPolyVectors.clear();
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin();
		cmd != inObj.cmds.end(); ++cmd)
	{
		if (cmd->cmdType == type_Poly)
		{
			Polygon3Vector		polys;
			Polygon3			aPoly;
			Point3				p1,p2,p3,p4;
			
			switch(cmd->cmdID) {
			case obj_Quad_Strip:
				for (int n = 2; n < cmd->st.size(); n += 2)
				{
					aPoly.clear();
					p1 = Point3(cmd->st[n-2].v[0], cmd->st[n-2].v[1], cmd->st[n-2].v[2]);
					p2 = Point3(cmd->st[n-1].v[0], cmd->st[n-1].v[1], cmd->st[n-1].v[2]);
					p3 = Point3(cmd->st[n+1].v[0], cmd->st[n+1].v[1], cmd->st[n+1].v[2]);
					p4 = Point3(cmd->st[n  ].v[0], cmd->st[n  ].v[1], cmd->st[n  ].v[2]);
					aPoly.push_back(p1);
					aPoly.push_back(p2);
					aPoly.push_back(p3);
					aPoly.push_back(p4);
					polys.push_back(aPoly);
				}
				break;
			case obj_Tri_Strip:
				for (int n = 2; n < cmd->st.size(); ++n)
				{
					aPoly.clear();
					p1 = Point3(cmd->st[n-2].v[0], cmd->st[n-2].v[1], cmd->st[n-2].v[2]);
					p2 = Point3(cmd->st[n-1].v[0], cmd->st[n-1].v[1], cmd->st[n-1].v[2]);
					p3 = Point3(cmd->st[n  ].v[0], cmd->st[n  ].v[1], cmd->st[n  ].v[2]);
					if (n % 2)
					{
						// Every other poly is reversed...
						aPoly.push_back(p1);
						aPoly.push_back(p3);
						aPoly.push_back(p2);
					} else {
						// Normal order
						aPoly.push_back(p1);
						aPoly.push_back(p2);
						aPoly.push_back(p3);
					}
					polys.push_back(aPoly);
				}					
				break;
			case obj_Tri_Fan:
				for (int n = 2; n < cmd->st.size(); ++n)
				{
					aPoly.clear();
					p1 = Point3(cmd->st[0  ].v[0], cmd->st[0  ].v[1], cmd->st[0  ].v[2]);
					p2 = Point3(cmd->st[n-1].v[0], cmd->st[n-1].v[1], cmd->st[n-1].v[2]);
					p3 = Point3(cmd->st[n  ].v[0], cmd->st[n  ].v[1], cmd->st[n  ].v[2]);
					aPoly.push_back(p1);
					aPoly.push_back(p2);
					aPoly.push_back(p3);
					polys.push_back(aPoly);					
				}
				break;;			
			default:		
				for (vector<vec_tex>::const_iterator st = cmd->st.begin();
					st != cmd->st.end();++st)
				{
					Point3	pt(st->v[0], st->v[1], st->v[2]);
					aPoly.push_back(pt);
				}
				polys.push_back(aPoly);
				break;
			}
			
			outPolyVectors.push_back(polys);
		} else {
			outPolyVectors.push_back(Polygon3Vector());
		}
	}
}				

void	OE_DeriveNormals(
				const vector<Polygon3Vector>&	outPolyVectors,
				vector<NormalVector>&			outNormals)
{
	outNormals.clear();
	for (vector<Polygon3Vector>::const_iterator cmd = outPolyVectors.begin();
		cmd != outPolyVectors.end(); ++cmd)
	{
		NormalVector	normals;
		for (Polygon3Vector::const_iterator poly = cmd->begin(); 
			poly != cmd->end(); ++poly)
		{
			Vector3	v1 = Vector3((*poly)[0], (*poly)[1]);
			Vector3	v2 = Vector3((*poly)[2], (*poly)[1]);
			Vector3	normal = v1.cross(v2);
			normals.push_back(normal);
		}
		outNormals.push_back(normals);
	}
}				

void	OE_DeriveVisible(
				const vector<NormalVector>&		inNormals,
				vector<VisibleVector>&			outVisible)
{
	// Important note: we can't just transform the deltas of a normal
	// vector from model view to eye coordinates.  Why not?  Well, 
	// there may be translations in the model view matrix that will just
	// add big quantities to the vector.  Vectors shouldn't be added to, only
	// points.  Our solution: we also transform the origin from model view to
	// eye coordinates and subtract it out, thus refinding the normal vector 
	// in eye space from two properly transformed pts.  We actually only subtract
	// out the Z coordinate cause that's all we care about for figuring out if the 
	// polygon is facing us or not.
	//
	// NOTE: I think we could probably just have set the translation params in the
	// model view matrix to zero, since we don't care about distortion of the size
	// of the normal vector (we don't even normalize it, so to speak).  OGL takes
	// the inverse of the transposed matrix or something, which I suspect does the same
	// although lord knows what it does to the W coordinate.

	outVisible.clear();
	GLdouble	model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	GLdouble	pt_zero_mv[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLdouble	pt_zero_ey[4];
	multMatrixVec(pt_zero_ey, model_view, pt_zero_mv);

	for (vector<NormalVector>::const_iterator cmd = inNormals.begin();
		cmd != inNormals.end(); ++cmd)
	{
		VisibleVector	vis;
		for (NormalVector::const_iterator normal = cmd->begin();
			normal != cmd->end(); ++normal)
		{
			GLdouble	ptMV[4] = { normal->dx, normal->dy, normal->dz, 1.0 };
			GLdouble	ptEye[4];
			multMatrixVec(ptEye, model_view, ptMV);
			vis.push_back((ptEye[2] - pt_zero_ey[2]) > 0.0);
		}
		outVisible.push_back(vis);
	}
}

#pragma mark -



void	OE_ResetST(
				XObjCmd&						ioCmd,
				float							s1,
				float							s2,
				float							t1,
				float							t2)
{
	if (ioCmd.st.size() < 3)
		return;
	
	// Ok, here comes the _whack shit_ we're going to do:
	// This is a lot harder than it looks.  We need to rotate
	// the polygon so that we have it in the XZ plane.  That way we have
	// some notion of how to apply 2d ST coordinates to a 3-d object.  Here we go...
	
	Point3	p1(ioCmd.st[0].v[0],ioCmd.st[0].v[1],ioCmd.st[0].v[2]);
	Point3	p2(ioCmd.st[1].v[0],ioCmd.st[1].v[1],ioCmd.st[1].v[2]);
	Point3	p3(ioCmd.st[2].v[0],ioCmd.st[2].v[1],ioCmd.st[2].v[2]);
	
	Vector3	side1(p1,p2);
	Vector3	side2(p3,p2);
	Vector3	cur_normal(side1.cross(side2));
	cur_normal.normalize();
	Vector3	wanted_normal(0.0, 1.0, 0.0);	
	Vector3	rotate_around(cur_normal.cross(wanted_normal));	
	double	rotate_amount = acos(cur_normal.dot(wanted_normal)) * 180.0 / M_PI;
	
	GLdouble	rotation_matrix[16];
	buildRotation( rotation_matrix, rotate_amount, 
		rotate_around.dx,rotate_around.dy,rotate_around.dz);

	GLdouble	vec[4];

	double	xMax,zMax, xMin,zMin;

	vector<Point3>	newPts;
	for (vector<vec_tex>::iterator st = ioCmd.st.begin(); st != ioCmd.st.end(); ++st)
	{
		vec[0] = st->v[0];
		vec[1] = st->v[1];
		vec[2] = st->v[2];
		vec[3] = 1.0;
		applyMatrixVec(vec, rotation_matrix);
		newPts.push_back(Point3(vec[0], vec[1], vec[2]));
		
		if (st == ioCmd.st.begin())
		{
			xMax = xMin = vec[0];
			zMax = zMin = vec[2];
		} else {
			xMax = std::max(xMax, vec[0]);
			xMin = std::min(xMin, vec[0]);
			zMax = std::max(zMax, vec[2]);
			zMin = std::min(zMin, vec[2]);
		}
	}
	
	double	xDist = xMax - xMin;
	double	zDist = zMax - zMin;
	
	for (int n = 0; n < ioCmd.st.size(); ++n)
	{
		ioCmd.st[n].st[0] = (xDist == 0.0) ? 0.0 : s1 + ((newPts[n].x - xMin) * (s2 - s1) / xDist);
		ioCmd.st[n].st[1] = (zDist == 0.0) ? 0.0 : t1 + ((newPts[n].z - zMin) * (t2 - t1) / zDist);
	}	
}				

void	OE_ClearST(
				XObjCmd&						ioCmd)
{
	for (vector<vec_tex>::iterator st = ioCmd.st.begin(); st != ioCmd.st.end(); ++st)
	{
		st->st[0] = st->st[1] = 0.0;
	}
}				

bool	OE_IsCleared(
				const XObjCmd&					ioCmd)
{
	for (vector<vec_tex>::const_iterator st = ioCmd.st.begin(); st != ioCmd.st.end(); ++st)
	{
		if (st->st[0] != 0.0 || st->st[1] != 0.0)
			return false;
	}
	return true;	
}


void	OE_RotateST(
				XObjCmd&						ioCmd,
				bool							inCCW)
{
	if (ioCmd.st.empty())	return;
	
	if (inCCW)
	{
		float	s = ioCmd.st[0].st[0];
		float	t = ioCmd.st[0].st[1];
		for (int n = 1; n < ioCmd.st.size(); ++n)
		{
			ioCmd.st[n-1].st[0] = ioCmd.st[n].st[0];
			ioCmd.st[n-1].st[1] = ioCmd.st[n].st[1];			
		}
		ioCmd.st[ioCmd.st.size()-1].st[0] = s;
		ioCmd.st[ioCmd.st.size()-1].st[1] = t;
		
	} else {	
		float	s = ioCmd.st[ioCmd.st.size()-1].st[0];
		float	t = ioCmd.st[ioCmd.st.size()-1].st[1];
		for (int n = ioCmd.st.size() - 1; n > 0; --n)
		{
			ioCmd.st[n].st[0] = ioCmd.st[n-1].st[0];
			ioCmd.st[n].st[1] = ioCmd.st[n-1].st[1];
		}
		ioCmd.st[0].st[0] = s;
		ioCmd.st[0].st[1] = t;
	}
}				

void	OE_FlipST(
				XObjCmd&						ioCmd)
{
	if (ioCmd.st.empty())	return;

	int all = ioCmd.st.size();
	int	half = all / 2;
	
	for (int n = 0; n < half; ++n)
	{
		swap(ioCmd.st[n].st[0], ioCmd.st[all-1-n].st[0]);
		swap(ioCmd.st[n].st[1], ioCmd.st[all-1-n].st[1]);
	}
}				

void	OE_ConstrainDrag(
				int								inX1,
				int								inY1,
				int&							inX2,
				int&							inY2)
{
	int	xdif = abs(inX1 - inX2);
	int ydif = abs(inY1 - inY2);
	if (xdif > ydif)
		inY2 = inY1;
	else
		inX2 = inX1;
}