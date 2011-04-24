/*
 * Copyright (c) 2007, Laminar Research.
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

#include <limits.h>

#include "GreedyMesh.h"
#include "MeshDefs.h"
#include "DEMGrid.h"
#include "DEMDefs.h"
#include "PerfUtils.h"
#include "MathUtils.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "PolyRasterUtils.h"

static		 CDT *		sCurrentMesh = NULL;
static const DEMGeo *	sCurrentDEM = NULL;
static		 DEMMask *	sUsedDEM = NULL;
static const DEMGrid *	sGridUsed = NULL;

static FaceQueue	sBestChoices;

struct	eval_face {
bool operator()(const CDT::Face_handle f1, const CDT::Face_handle f2) const {
	return f1->info().insert_err < f2->info().insert_err; }
};

inline CDT::Face_handle CDT_Recover_Handle(CDT::Face *the_face)
{
	CDT::Face_handle n = the_face->neighbor(0);
	CDT::Vertex_handle c = the_face->vertex(CDT::cw(0));
	int mirror_i = n->index(c);
	CDT::Face_handle self = n->neighbor(CDT::cw(mirror_i));
	DebugAssert(&*self == the_face);
	return self;
}


// Calc plane eq of one tri
bool	InitOneTri(CDT::Face_handle face)
{
	if (!sCurrentMesh->is_infinite(face))
	{
		Point3	p1(sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(0)->point().x())),
				   sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(0)->point().y())),
				   face->vertex(0)->info().height);
		Point3	p2(sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(1)->point().x())),
				   sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(1)->point().y())),
				   face->vertex(1)->info().height);
		Point3	p3(sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(2)->point().x())),
				   sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(2)->point().y())),
				   face->vertex(2)->info().height);

		Vector3	v1(p1, p2);
		Vector3	v2(p1, p3);
		Plane3	plane(p1, v1.cross(v2));

		face->info().plane_a = -plane.n.dx / plane.n.dz;
		face->info().plane_b = -plane.n.dy / plane.n.dz;
		face->info().plane_c = plane.ndotp / plane.n.dz;
	}

	bool	first_time = !face->info().flag;
	if (first_time)
		face->info().self = sBestChoices.end();
	face->info().flag = true;
	return first_time;
}

// The rasterization of triangles is done in floating point, but this can lead to subtle errors.  This code goes back
// and checks the final point (converted back to precise CGAL coordinates) against the original triangle.  We don't include
// the point if (1) it is outside the triangle bounds or (2) it duplicates a corner (since corners are already exact).
bool really_ok_point(const DEMGeo * dem, const DEMGrid * g, int x, int y, const CDT::Point& v1, const CDT::Point& v2, const CDT::Point& v3)
{
	CDT::Point p(g->xy_to_pt(x,y));
	return p != v1 && p != v2 && p != v3 &&
		!Triangle_2(v1,v2,v3).has_on_unbounded_side(p);
}

inline float ScanlineMaxError(
					const DEMGeo *	inDEMSrc,
					const DEMMask *	inDEMUsed,
					const DEMGrid *	inGrid,
					int				y,
					double			x1,
					double			x2,
					float			worst,
					int *			worst_x,
					int *			worst_y,
					double			a,
					double			b,
					double			c,
					const CDT::Point&		v1,
					const CDT::Point&		v2,
					const CDT::Point&		v3)
{
	float * row = inDEMSrc->mData + y * inDEMSrc->mWidth;
	vector<bool>::const_iterator used = inDEMUsed->mData.begin() + y * inDEMUsed->mWidth;
//	DebugAssert(x1 < x2);
	DebugAssert(y >= 0);
	DebugAssert(y < inDEMSrc->mHeight);

	int ix1 = max(0,(int)round(min(x1,x2)));
	int ix2 = min(inDEMSrc->mWidth-1,(int)round(max(x1,x2)));
	DebugAssert(ix1 >= 0);
	DebugAssert(ix2 < inDEMSrc->mWidth);

	row += ix1;
	used += ix1;
	float partial = b * y + c;

	for (int x = ix1; x <= ix2; ++x, ++row, ++used)
	{
//		gMeshPoints.push_back(pair<Point2, Point3>(Point2(inDEM->x_to_lon(x), inDEM->y_to_lat(y)), Point3(0, 1, 0.5)));
		float want = *row;
		if (want != DEM_NO_DATA && (*used) == false)
		{
			float got = a * x + partial;
			float diff = want - got;
			if (diff < 0.0) diff = -diff;
			if (diff > worst)
			if (really_ok_point(inDEMSrc,inGrid, x,y,v1,v2,v3))
			{
				worst = diff;
				*worst_x = x;
				*worst_y = y;
			}
		}
	}
	return worst;
}


// Find err of one tri
void	CalcOneTriError(CDT::Face_handle face, double size_lim)
{
	if (sCurrentMesh->is_infinite(face))
	{
		face->info().insert_err = 0.0;
		return;
	}
	Point2	p0( sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(0)->point().x())),
			    sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(0)->point().y())));
	Point2	p1( sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(1)->point().x())),
			    sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(1)->point().y())));
	Point2	p2( sCurrentDEM->lon_to_x(CGAL::to_double(face->vertex(2)->point().x())),
			    sCurrentDEM->lat_to_y(CGAL::to_double(face->vertex(2)->point().y())));

	if (p0.x() < 0 || p0.x() > sCurrentDEM->mWidth ||
		p0.y() < 0 || p0.y() > sCurrentDEM->mHeight ||
		p1.x() < 0 || p1.x() > sCurrentDEM->mWidth ||
		p1.y() < 0 || p1.y() > sCurrentDEM->mHeight ||
		p2.x() < 0 || p2.x() > sCurrentDEM->mWidth ||
		p2.y() < 0 || p2.y() > sCurrentDEM->mHeight)
	{
		fprintf(stderr, "%lf %lf, %lf %lf, %lf %lf\n",
				CGAL::to_double(face->vertex(0)->point().x()), CGAL::to_double(face->vertex(0)->point().y()),
				CGAL::to_double(face->vertex(1)->point().x()), CGAL::to_double(face->vertex(1)->point().y()),
				CGAL::to_double(face->vertex(2)->point().x()), CGAL::to_double(face->vertex(2)->point().y()));
		face->info().insert_err = 0.0;
		return;
	}



	if (size_lim != 0.0)
	{
		double xmin = min(min(CGAL::to_double(face->vertex(0)->point().x()),CGAL::to_double(face->vertex(1)->point().x())),CGAL::to_double(face->vertex(2)->point().x()));
		double xmax = max(max(CGAL::to_double(face->vertex(0)->point().x()),CGAL::to_double(face->vertex(1)->point().x())),CGAL::to_double(face->vertex(2)->point().x()));
		double ymin = min(min(CGAL::to_double(face->vertex(0)->point().y()),CGAL::to_double(face->vertex(1)->point().y())),CGAL::to_double(face->vertex(2)->point().y()));
		double ymax = max(max(CGAL::to_double(face->vertex(0)->point().y()),CGAL::to_double(face->vertex(1)->point().y())),CGAL::to_double(face->vertex(2)->point().y()));

		double xs = xmax - xmin;
		double ys = ymax - ymin;

		if (xs < size_lim && ys < size_lim)
		{
			face->info().insert_err = 0.0;
			return;
		}
	}

//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(0)->point().x(),face->vertex(0)->point().y()), Point3(1,0,0)));
//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(1)->point().x(),face->vertex(1)->point().y()), Point3(1,0,0)));
//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(1)->point().x(),face->vertex(1)->point().y()), Point3(1,0,0)));
//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(2)->point().x(),face->vertex(2)->point().y()), Point3(1,0,0)));
//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(2)->point().x(),face->vertex(2)->point().y()), Point3(1,0,0)));
//	gMeshLines.push_back(pair<Point2,Point3>(Point2(face->vertex(0)->point().x(),face->vertex(0)->point().y()), Point3(1,0,0)));

#if 1

	if (p2.y() < p1.y()) swap(p1, p2);
	if (p1.y() < p0.y()) swap(p1, p0);
	if (p2.y() < p1.y()) swap(p1, p2);
	DebugAssert(p0.y() <= p1.y() && p1.y() <= p2.y());
	
	if(p0.y() == p2.y())
	{
		// WTF?  Well, maybe the vector data has a micr-sliver, and the floating point equivalent is so damned thin...bail out.
		face->info().insert_err = 0.0;
		return;
	}

	float err = 0;

	int y;
	
	int y0 = max(0,int(round(p0.y())));
	int y1 = min(sCurrentDEM->mHeight-1,int(round(p2.y())));
	int yc = max(y0,(int)floor(p1.y()));
	
	DebugAssert(yc >= y0);
	DebugAssert(y1 >= yc);

	double a = face->info().plane_a;
	double b = face->info().plane_b;
	double c = face->info().plane_c;

/*	
	if(p0.y() == p2.y())
	{		
		debug_mesh_point(cgal2ben(face->vertex(0)->point()), 1,1,1);
		debug_mesh_point(cgal2ben(face->vertex(1)->point()), 1,0,1);
		debug_mesh_point(cgal2ben(face->vertex(2)->point()), 1,1,0);
	}
*/
	DebugAssert(p0.y() != p2.y());

	int 	worst_x = 0, worst_y = 0;

	CDT::Point v1(face->vertex(0)->point());
	CDT::Point v2(face->vertex(1)->point());
	CDT::Point v3(face->vertex(2)->point());

	for (y = y0; y < yc; ++y)
	{
		double x1 = double_interp(p0.y(),p0.x(),p1.y(),p1.x(),y);
		double x2 = double_interp(p0.y(),p0.x(),p2.y(),p2.x(),y);
		err = ScanlineMaxError(sCurrentDEM, sUsedDEM, sGridUsed, y, x1, x2, err, &worst_x, &worst_y, a, b, c, v1, v2, v3);
	}

	for (y = yc; y < y1; ++y)
	{
		double x1 = double_interp(p0.y(),p0.x(),p2.y(),p2.x(),y);
		double x2 = double_interp(p1.y(),p1.x(),p2.y(),p2.x(),y);
		err = ScanlineMaxError(sCurrentDEM, sUsedDEM, sGridUsed, y, x1, x2, err, &worst_x, &worst_y, a, b, c, v1, v2, v3);
	}

#endif

#if 0
	double ymin = min(min(CGAL::to_double(face->vertex(0)->point().y()),CGAL::to_double(face->vertex(1)->point().y())),CGAL::to_double(face->vertex(2)->point().y()));
	double ymax = max(max(CGAL::to_double(face->vertex(0)->point().y()),CGAL::to_double(face->vertex(1)->point().y())),CGAL::to_double(face->vertex(2)->point().y()));
	double xmin = min(min(CGAL::to_double(face->vertex(0)->point().x()),CGAL::to_double(face->vertex(1)->point().x())),CGAL::to_double(face->vertex(2)->point().x()));
	double xmax = max(max(CGAL::to_double(face->vertex(0)->point().x()),CGAL::to_double(face->vertex(1)->point().x())),CGAL::to_double(face->vertex(2)->point().x()));

	int x1 = max(0,(int) floor(sCurrentDEM->lon_to_x(xmin)-0.5));
	int y1 = max(0,(int) floor(sCurrentDEM->lat_to_y(ymin)-0.5));
	int x2 = min(sCurrentDEM->mWidth -1,(int) ceil(sCurrentDEM->lon_to_x(xmax)+0.5));
	int y2 = min(sCurrentDEM->mHeight-1,(int) ceil(sCurrentDEM->lat_to_y(ymax)+0.5));

	float	err = 0;
	int 	worst_x = 0, worst_y = 0;

	double a = face->info().plane_a;
	double b = face->info().plane_b;
	double c = face->info().plane_c;

	CDT::Point v1(face->vertex(0)->point());
	CDT::Point v2(face->vertex(1)->point());
	CDT::Point v3(face->vertex(2)->point());
	
	for(int y = y1; y <= y2; ++y)
	{
		err = ScanlineMaxError(sCurrentDEM, sUsedDEM, sGridUsed, y, x1, x2, err, &worst_x, &worst_y, a, b, c, v1, v2, v3);		
	}
#endif	

	face->info().insert_err = err;
	if (err > 0)
	{
		face->info().insert_x = worst_x;
		face->info().insert_y = worst_y;
	}
}

// Init the whole mesh - all tris, calc errs, queue
void	InitMesh(CDT& inCDT, const DEMGeo& inDem, DEMMask& inUsed, const DEMGrid& inGrid, double err_cutoff, double size_lim)
{
	sBestChoices.clear();
	sCurrentDEM = &inDem;
	sUsedDEM = &inUsed;
	sGridUsed = &inGrid,
	sCurrentMesh = &inCDT;

	for (CDT::All_faces_iterator face = inCDT.all_faces_begin(); face != inCDT.all_faces_end(); ++face)
	{
		if (!sCurrentMesh->is_infinite(face)) {
			face->info().flag = 0;
			InitOneTri(face);
			CalcOneTriError(face, size_lim);
			if (face->info().insert_err > err_cutoff)
			{
//				printf("Initing 0x%08x because err is %f at %d,%d\n", &*face, face->info().insert_err,face->info().insert_x,face->info().insert_y);
			
				face->info().self = sBestChoices.insert(FaceQueue::value_type(face->info().insert_err, &*face));
			}
		}
	}
}

// Cleanup
void	DoneMesh(void)
{
	sBestChoices.clear();
	sCurrentDEM = NULL;
	sUsedDEM = NULL;
	sGridUsed = NULL;
	sCurrentMesh = NULL;
}

void	GreedyMeshBuild(CDT& inCDT, const DEMGeo& inAvail, DEMMask& ioUsed, const DEMGrid& gridlines, double err_lim, double size_lim, int max_num, ProgressFunc func)
{
	StElapsedTime gr("greedymesh");
//	fprintf(stderr,"Building Mesh err=%lf size=%lf max=%d\n", err_lim, size_lim, max_num);
	PROGRESS_START(func, 0, 1, "Building Mesh")
	InitMesh(inCDT, inAvail, ioUsed, gridlines, err_lim, size_lim);

	if (max_num == 0) max_num = INT_MAX;
	int cnt_insert = 0, cnt_new = 0, cnt_recalc = 0;

//	if(!sBestChoices.empty())
//		printf("GD start, worst err is: %f\n", sBestChoices.begin()->first);

	for (int n = 0; n < max_num; ++n)
	{
		if (sBestChoices.empty()) 
		{
//			printf("Done with greedy mesh - we met our criteria.\n");
			break;
		}
		PROGRESS_CHECK(func, 0, 1, "Building mesh", n, max_num, max_num / 200)
		++cnt_insert;
		CDT::Face * the_face = (CDT::Face *) sBestChoices.begin()->second;


		CDT::Face_handle	face_handle(CDT_Recover_Handle(the_face));

		DebugAssert(!inCDT.is_infinite(face_handle));

		CDT::Point p(gridlines.xy_to_pt(the_face->info().insert_x,the_face->info().insert_y));

//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(0)->point().x(),the_face->vertex(0)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(1)->point().x(),the_face->vertex(1)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(1)->point().x(),the_face->vertex(1)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(2)->point().x(),the_face->vertex(2)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(2)->point().x(),the_face->vertex(2)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(0)->point().x(),the_face->vertex(0)->point().y()), Point3(1,0,1)));
//		gMeshPoints.push_back(pair<Point2,Point3>(Point2(p.x(), p.y()), Point3(1,1,1)));

		double h = inAvail.get(the_face->info().insert_x, the_face->info().insert_y);
		#if DEV
		
		bool hh = ioUsed.get(the_face->info().insert_x, the_face->info().insert_y);
		if(hh)
		{
			printf("ERROR: we want to do this.\n");
			printf("Inserting: 0x%08lx, %d,%d, err was %f\n",&*the_face, the_face->info().insert_x,the_face->info().insert_y, the_face->info().insert_err);
			printf("But the point is not available for insert.\n");
		}
		DebugAssert(!hh);
		#endif
//		printf("Inserting: 0x%08lx, %d,%d, err was %f\n",&*the_face, the_face->info().insert_x,the_face->info().insert_y, the_face->info().insert_err);
		DebugAssert(h != DEM_NO_DATA);
		ioUsed.set(the_face->info().insert_x, the_face->info().insert_y,true);

		set<CDT::Face_handle>	affected;
		CDT::Vertex_handle new_v = inCDT.insert_collect_flips(p,face_handle, affected);
		new_v->info().height = h;

		for(set<CDT::Face_handle>::iterator a = affected.begin(); a != affected.end(); ++a)
		{
			CDT::Face_handle circ(*a);
			
			if (InitOneTri(circ))
			{
				++cnt_new;
			}
			if (circ->info().self != sBestChoices.end())
			{
				sBestChoices.erase(circ->info().self);
				circ->info().self = sBestChoices.end();
			}
			CalcOneTriError(circ, size_lim);
			if (circ->info().insert_err > err_lim)
			{
//				printf("Reinserting 0x%08x because err is %f at %d,%d\n", &*circ, circ->info().insert_err,circ->info().insert_x,circ->info().insert_y);
				circ->info().self = sBestChoices.insert(FaceQueue::value_type(circ->info().insert_err, &*circ));
			}
		} 

	}

	DoneMesh();
	PROGRESS_DONE(func, 0, 1, "Building Mesh")

	printf("Greedy insert: %d pts, %d recalcs, %d new faces\n", cnt_insert, cnt_recalc, cnt_new);
}
