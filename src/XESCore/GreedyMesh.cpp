#include "GreedyMesh.h"
#include "MeshDefs.h"
#include "DEMDefs.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "PolyRasterUtils.h"
//#include "WED_Globals.h"

static CDT *	sCurrentMesh = NULL;
static DEMGeo *	sCurrentDEM = NULL;

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
		Point3	p1(sCurrentDEM->lon_to_x(face->vertex(0)->point().x()),
				   sCurrentDEM->lat_to_y(face->vertex(0)->point().y()),
				   face->vertex(0)->info().height);
		Point3	p2(sCurrentDEM->lon_to_x(face->vertex(1)->point().x()),
				   sCurrentDEM->lat_to_y(face->vertex(1)->point().y()),
				   face->vertex(1)->info().height);
		Point3	p3(sCurrentDEM->lon_to_x(face->vertex(2)->point().x()),
				   sCurrentDEM->lat_to_y(face->vertex(2)->point().y()),
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

inline float ScanlineMaxError(
					const DEMGeo *	inDEM,
					int				y,
					double			x1,
					double			x2,
					float			worst,
					int *			worst_x,
					int *			worst_y,
					double			a,
					double			b,
					double			c)
{
	float * row = inDEM->mData + y * inDEM->mWidth;
//	DebugAssert(x1 < x2);
	DebugAssert(y >= 0);
	DebugAssert(y < inDEM->mHeight);
	
	int ix1 = ceil(min(x1,x2));
	int ix2 = floor(max(x1,x2));
	DebugAssert(ix1 >= 0);
	DebugAssert(ix2 < inDEM->mWidth);
	
	row += ix1;
	float partial = b * y + c;
	
	for (int x = ix1; x <= ix2; ++x, ++row)
	{
//		gMeshPoints.push_back(pair<Point2, Point3>(Point2(inDEM->x_to_lon(x), inDEM->y_to_lat(y)), Point3(0, 1, 0.5)));
		float want = *row;
		if (want != NO_DATA)
		{
			float got = a * x + partial;
			float diff = want - got;
			if (diff < 0.0) diff = -diff;
			if (diff > worst)
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
//	gMeshLines.clear();
//	gMeshPoints.clear();

	Point2	p0(sCurrentDEM->lon_to_x(face->vertex(0)->point().x()),
			   sCurrentDEM->lat_to_y(face->vertex(0)->point().y()));
	Point2	p1(sCurrentDEM->lon_to_x(face->vertex(1)->point().x()),
			   sCurrentDEM->lat_to_y(face->vertex(1)->point().y()));
	Point2	p2(sCurrentDEM->lon_to_x(face->vertex(2)->point().x()),
			   sCurrentDEM->lat_to_y(face->vertex(2)->point().y()));
			   
	if (size_lim != 0.0)
	{
		double xmin = min(min(face->vertex(0)->point().x(),face->vertex(1)->point().x()),face->vertex(2)->point().x());
		double xmax = max(max(face->vertex(0)->point().x(),face->vertex(1)->point().x()),face->vertex(2)->point().x());
		double ymin = min(min(face->vertex(0)->point().y(),face->vertex(1)->point().y()),face->vertex(2)->point().y());
		double ymax = max(max(face->vertex(0)->point().y(),face->vertex(1)->point().y()),face->vertex(2)->point().y());
		
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
	
	if (p2.y < p1.y) swap(p1, p2);
	if (p1.y < p0.y) swap(p1, p0);
	if (p2.y < p1.y) swap(p1, p2);
	DebugAssert(p0.y <= p1.y && p1.y <= p2.y);

	float err = 0;
	
	double	p0yc = ceil(p0.y);
	double	p1yc = ceil(p1.y);
	double	p2yc = ceil(p2.y);
	int y0 = p0yc;
	int y1 = p1yc;
	int y2 = p2yc;
	int y;
	
	double dx1, dx2, x1, x2;
	
	double a = face->info().plane_a;
	double b = face->info().plane_b;
	double c = face->info().plane_c;

	x1 = x2 = p0.x;

	DebugAssert(p0.y != p2.y);

	if (p0.y != p2.y)
		dx2 = (p2.x - p0.x) / (p2.y - p0.y);		

	int 	worst_x = 0, worst_y = 0;

	double partial = p0yc-p0.y;
	x2 += dx2 * partial;

	// SPECIAL CASE: if p1 and p2 are horizontal, there is no section 2 of the tri - it has a flat top.  Do NOT miss that top scanline!
	// Basically use floor + 1 to INCLDE the top scanline if we have a perfect match.
	if (p1.y == p2.y)
		y1 = floor(p1.y)+1;

	if (p0.y != p1.y)
	{
		dx1 = (p1.x - p0.x) / (p1.y - p0.y);
		x1 += dx1 * partial;
		for (y = y0; y < y1; ++y)
		{
//			gMeshPoints.push_back(pair<Point2,Point3>(Point2(sCurrentDEM->x_to_lon_double(x1), sCurrentDEM->y_to_lat_double(y)),Point3(0,0,1)));
//			gMeshPoints.push_back(pair<Point2,Point3>(Point2(sCurrentDEM->x_to_lon_double(x2), sCurrentDEM->y_to_lat_double(y)),Point3(0,0,1)));
			err = ScanlineMaxError(sCurrentDEM, y, x1, x2, err, &worst_x, &worst_y, a, b, c);
			x1 += dx1;
			x2 += dx2;
		}
	}
	
	if (p1.y != p2.y)
	{
		dx1 = (p2.x - p1.x) / (p2.y - p1.y);
		x1 = p1.x;
		partial = p1yc-p1.y;
		x1 += dx1 * partial;
		
		for (y = y1; y < y2; ++y)
		{
//			gMeshPoints.push_back(pair<Point2,Point3>(Point2(sCurrentDEM->x_to_lon_double(x1), sCurrentDEM->y_to_lat_double(y)),Point3(0,0,1)));
//			gMeshPoints.push_back(pair<Point2,Point3>(Point2(sCurrentDEM->x_to_lon_double(x2), sCurrentDEM->y_to_lat_double(y)),Point3(0,0,1)));
			err = ScanlineMaxError(sCurrentDEM, y, x1, x2, err, &worst_x, &worst_y, a, b, c);
			x1 += dx1;
			x2 += dx2;
		}
	}
	
	face->info().insert_err = err;
	if (err > 0)
	{
		face->info().insert_x = worst_x;
		face->info().insert_y = worst_y;

//		double lon = sCurrentDEM->x_to_lon(worst_x);
//		double lat = sCurrentDEM->y_to_lat(worst_y);
//		Point2 t(lon, lat);
//		gMeshPoints.push_back(pair<Point2,Point3>(t, Point3(1, 0, 0))); 
/*
		Point2 t0(face->vertex(0)->point().x(),face->vertex(0)->point().y());
		Point2 t1(face->vertex(1)->point().x(),face->vertex(1)->point().y());
		Point2 t2(face->vertex(2)->point().x(),face->vertex(2)->point().y());

		if(Segment2(t0,t1).on_right_side(t)) { AssertPrintf("ERROR OB"); }
		if(Segment2(t1,t2).on_right_side(t)) { AssertPrintf("ERROR OB"); }
		if(Segment2(t2,t0).on_right_side(t)) { AssertPrintf("ERROR OB"); }
*/
	}
}

// Init the whole mesh - all tris, calc errs, queue
void	InitMesh(CDT& inCDT, DEMGeo& inDem, double err_cutoff, double size_lim)
{
	sBestChoices.clear();
	sCurrentDEM = &inDem;
	sCurrentMesh = &inCDT;
	
	for (CDT::Face_iterator face = inCDT.faces_begin(); face != inCDT.faces_end(); ++face)
	{
		face->info().flag = 0;
		InitOneTri(face);
		CalcOneTriError(face, size_lim);
		if (face->info().insert_err > err_cutoff)
		{
			face->info().self = sBestChoices.insert(FaceQueue::value_type(face->info().insert_err, &*face));
		}
	}
}

// Cleanup
void	DoneMesh(void)
{
	sBestChoices.clear();
	sCurrentDEM = NULL;
	sCurrentMesh = NULL;
}

void	GreedyMeshBuild(CDT& inCDT, DEMGeo& inAvail, DEMGeo& outUsed, double err_lim, double size_lim, int max_num, ProgressFunc func)
{
	PROGRESS_START(func, 0, 1, "Building Mesh")
	InitMesh(inCDT, inAvail, err_lim, size_lim);

	if (max_num == 0) max_num = INT_MAX;
	int cnt_insert = 0, cnt_new = 0, cnt_recalc = 0;
		
	for (int n = 0; n < max_num; ++n)
	{
		if (sBestChoices.empty()) break;
		PROGRESS_CHECK(func, 0, 1, "Building mesh", n, max_num, max_num / 200)
		++cnt_insert;
		CDT::Face * the_face = (CDT::Face *) sBestChoices.begin()->second;
		
		
		CDT::Face_handle	face_handle(CDT_Recover_Handle(the_face));
		
		DebugAssert(!inCDT.is_infinite(face_handle));
		
		CDT::Point p(inAvail.x_to_lon(the_face->info().insert_x),
					  inAvail.y_to_lat(the_face->info().insert_y));
					  
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(0)->point().x(),the_face->vertex(0)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(1)->point().x(),the_face->vertex(1)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(1)->point().x(),the_face->vertex(1)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(2)->point().x(),the_face->vertex(2)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(2)->point().x(),the_face->vertex(2)->point().y()), Point3(1,0,1)));
//		gMeshLines.push_back(pair<Point2,Point3>(Point2(the_face->vertex(0)->point().x(),the_face->vertex(0)->point().y()), Point3(1,0,1)));
//		gMeshPoints.push_back(pair<Point2,Point3>(Point2(p.x(), p.y()), Point3(1,1,1)));
					  
		double h = inAvail.get(the_face->info().insert_x, the_face->info().insert_y);
//		printf("Inserting: 0x%08lx, %d,%d - %lf, %lf, h = %lf\n",&*the_face, the_face->info().insert_x,the_face->info().insert_y, p.x(), p.y(), h);
		DebugAssert(h != NO_DATA);
		outUsed(the_face->info().insert_x, the_face->info().insert_y) = h;
		inAvail(the_face->info().insert_x, the_face->info().insert_y) = NO_DATA;

/*
		CDT::Locate_type lt;
		int li;
		CDT::Face_handle test = inCDT.locate(p, lt, li, the_face);
		printf("  LOCATE TYPE = %d\n", lt);
		if (lt == CDT::FACE)		
		{
			DebugAssert(test == the_face);
		} else if (lt == CDT::EDGE) {
			DebugAssert(test == the_face || test->neighbor(li) == the_face);
		} else if (lt == CDT::VERTEX) {
			CDT::Vertex_handle v = test->vertex(li);
			DebugAssert(v == the_face->vertex(0) || v == the_face->vertex(1) || v == the_face->vertex(2));
		} else
			AssertPrintf("Bad Locate.");
*/
		
		CDT::Vertex_handle new_v = inCDT.insert(p, face_handle);
		new_v->info().height = h;
		
		CDT::Face_circulator circ, stop;
		circ = stop = inCDT.incident_faces(new_v);
		do {
//			printf("   Realc 0x%08lx\n", &*circ);
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
				circ->info().self = sBestChoices.insert(FaceQueue::value_type(circ->info().insert_err, &*circ));
			}
			++the_face;
			++circ;
		} while (circ != stop);	
		
	}
	
	DoneMesh();
	PROGRESS_DONE(func, 0, 1, "Building Mesh")	
	
	printf("Greedy insert: %d pts, %d recalcs, %d new faces\n", cnt_insert, cnt_recalc, cnt_new);
}
