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
#include "DSFBuilder.h"
#include "DSFLib.h"
#include "DSFDefs.h"
#include "EnumSystem.h"
#include "NetTables.h"
#include "ForestTables.h"
#include "NetPlacement.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "AssertUtils.h"
#include "MathUtils.h"
#include "PerfUtils.h"
#include "GISTool_Globals.h"
#include "MobileAutogenAlgs.h"

/*
	TODO:
		make names that are written out to the definition manifest be what we want!

 */

#define PROFILE_PERFORMANCE 1
#if PROFILE_PERFORMANCE
#define TIMER(x)	StElapsedTime	__PerfTimer##x(#x);
#else
#define TIMER(x)
#endif

DSFBuildPrefs_t	gDSFBuildPrefs = { 1 };

// Ben syas: 32x32 is definitely a good bucket size - when we go 16x16 our vertex count goes way up and fps tank.
// And increasing bucket size does NOT improve stripification al ot.
static inline int get_patch_dim_hi() { return gMobile ? 32 : 16; }
static inline int get_patch_dim_lo() { return gMobile ? 32 : 16; }
#define		DSF_DIVISIONS	8

#define		TERRAIN_NEAR_LOD			 0.0
#define		TERRAIN_FAR_LOD				-1.0
#define		TERRAIN_NEAR_BORDER_LOD 	 0.0
static inline int get_terrain_far_border_lod() { return gMobile ? 10000 : 40000; } // Ben says - 5000 was too small - it popped - ugly!

#define		ORTHO_NEAR_LOD			100000.0
#define		ORTHO_FAR_LOD			-1.0
#define		MAX_TRIS_PER_PATCH		85

// Disable ortho-mesh
#define		NO_ORTHO 1

// Don't output borders
#define		NO_BORDERS 0

// Enable reduction of bezier curves.
#define		CAN_OPTIMIZE_BEZIERS 1

// Set this 1 to only optimize the longer ramps in the DSF - useful when step-debugging
// bezier optimization because you only get the really interesting cases.
#define		ONLY_OPTIMIZE_RAMPS 0

// Set to 1 to visualize beters on screen.
#define		SHOW_BEZIERS 0

// These macros set the height and normal in the mesh to the new-style modes.
#define USE_DEM_H(x,w,m,v)	((CategorizeVertex(m,v,terrain_Water) <= 0) ? (x) : -32768.0)
#define USE_DEM_N(x)		0.0f

#if 0
static void sub_heights(CDT& mesh, const DEMGeo& sl)
{
	CDT::Finite_faces_iterator ffi;
	for(ffi = mesh.finite_faces_begin(); ffi != mesh.finite_faces_end(); ++ffi)
	if(ffi->info().terrain == terrain_Water)
		ffi->info().flag = 0;

	for(ffi = mesh.finite_faces_begin(); ffi != mesh.finite_faces_end(); ++ffi)
	if(ffi->info().terrain == terrain_Water)
	if(ffi->info().flag == 0)
	{
		list<CDT::Face_handle>	water_body;
		list<CDT::Face_handle>	to_visit;
		to_visit.push_back(ffi);
		ffi->info().flag = 1;
		while(!to_visit.empty())
		{
			CDT::Face_handle f = to_visit.front();
			to_visit.pop_front();
			water_body.push_back(f);
			for(int n = 0; n < 3; ++n)
			{
				CDT::Face_handle nf = f->neighbor(n);
				if(!mesh.is_infinite(nf) && nf->info().terrain == terrain_Water && nf->info().flag == 0)
				{
					nf->info().flag = 1;
					to_visit.push_back(nf);
				}
			}
		}
		
		Bbox2	this_lake;
		set<CDT::Vertex_handle> mv;
		for(list<CDT::Face_handle>::iterator i = water_body.begin(); i != water_body.end(); ++i)
		for(int n = 0; n < 3; ++n)
		{
			this_lake += cgal2ben((*i)->vertex(n)->point());
			mv.insert((*i)->vertex(n));
		}
		
		bool hosed = false;
		for(set<CDT::Vertex_handle>::iterator v = mv.begin(); v != mv.end(); ++v)
		{
			double sh = sl.xy_nearest(CGAL::to_double((*v)->point().x()),CGAL::to_double((*v)->point().y()));
			if(fabs(sh - (*v)->info().height) > 3.0)
			{
				debug_mesh_point(cgal2ben((*v)->point()),1,0,0);
			
				hosed = true;
				break;
			}
		}
		if(!hosed)
		{
			for(set<CDT::Vertex_handle>::iterator v = mv.begin(); v != mv.end(); ++v)
			{
				int c = CategorizeVertex(mesh, *v, terrain_Water);
				(*v)->info().height = c + 1;				
			}
		}
		else
		{	
			debug_mesh_line(
				Point2(this_lake.xmin(),this_lake.ymin()),
				Point2(this_lake.xmax(),this_lake.ymin()), 0, 0, 1, 0, 0, 1);
			debug_mesh_line(
				Point2(this_lake.xmin(),this_lake.ymax()),
				Point2(this_lake.xmax(),this_lake.ymax()), 0, 0, 1, 0, 0, 1);

			debug_mesh_line(
				Point2(this_lake.xmin(),this_lake.ymin()),
				Point2(this_lake.xmin(),this_lake.ymax()), 0, 0, 1, 0, 0, 1);
			debug_mesh_line(
				Point2(this_lake.xmax(),this_lake.ymin()),
				Point2(this_lake.xmax(),this_lake.ymax()), 0, 0, 1, 0, 0, 1);
			
			for(set<CDT::Vertex_handle>::iterator v = mv.begin(); v != mv.end(); ++v)
			{
			}
		}
	}
	
}
#endif

class	deferred_pool : public list<void *> {
public:
	~deferred_pool()
	{
		for(list<void*>::iterator i = begin(); i != end(); ++i)
			free(*i);
	}
};

template<typename T>
T * ConvertDEMTo(const DEMGeo& d, DSFRasterHeader_t& h, int fmt, float s, float o)
{
	T * mem = (T *) malloc(sizeof(T) * d.mWidth * d.mHeight);
	T * p = mem;

	h.version = dsf_RasterVersion;
	h.bytes_per_pixel = sizeof(T);
	h.flags = fmt;
	if(d.mPost) h.flags |= dsf_Raster_Post;
	h.width = d.mWidth;
	h.height = d.mHeight;
	h.offset = o;
	h.scale = s;

	int pc = d.mWidth * d.mHeight;
	DEMGeo::const_iterator i = d.begin();

	float sp = 1.0 / s;
	float op = -sp * o;

	while(pc--)
	{
		*p++ = (*i++) * sp + op;
	}

	return mem;
}

#if SHOW_BEZIERS
void visualize_bezier(list<Point2c>& bez, bool want_straight_segs, float nt, float r)
{
	list<Point2c>::iterator start(bez.begin()), stop(bez.begin()), last(bez.end());
	--last;
	DebugAssert(!last->c);

	while(start != last)
	{
		debug_mesh_point(*start,nt,nt,nt);
		DebugAssert(!start->c);
		stop = start;
		++stop;
		while(stop->c) ++stop;

		int d = distance(start,stop);
//		printf("Debugging %d\n", d);
		switch(d) {
		case 1:
			if(want_straight_segs)
				debug_mesh_line(*start, *stop, 0,1,0, 0, 1, 0);
			break;
		case 2:
			debug_mesh_bezier(*start, *nth_from(start,1), *nth_from(start,2), r,0,1,r,0,1);
			debug_mesh_point(*nth_from(start,1), r*nt, 0, nt);
			break;
		case 3:
			debug_mesh_bezier(*start, *nth_from(start,1), *nth_from(start,2),*nth_from(start,3), r,1,1,r,1,1);
			debug_mesh_point(*nth_from(start,1), r*nt, 0,nt);
			debug_mesh_point(*nth_from(start,2), r*nt, 0,nt);
			break;
		}
		start = stop;
	}
	debug_mesh_point(*last,nt,nt,nt);

}
#endif


struct	road_coords_checker {
	double last[3];
	void * ptr;
	char lm;
	road_coords_checker(void * p, double c[3], char m) {ptr = p;  last[0] = c[0]; last[1] = c[1]; last[2] = c[2]; lm = m; }

//	#define epsi 0.00001
	#define epsi 0.0000001

	void check(double c[3], char m) {

		if(fabs(c[0] - last[0]) < epsi &&
		   fabs(c[1] - last[1]) < epsi &&
		   (
			(lm == 'B' && c[2] == 0.0) ||
			(m == 'E' && last[2] == 0.0) ||
			(lm == 'B' && m == 'E') ||
			(c[2] == 0.0 && last[2] == 0.0)
		   )
		)
		{
//			debug_mesh_point(Point2(c[0],c[1]),1,1,1);
//			debug_mesh_point(Point2(last[0],last[1]),1,0,0);
			fprintf(stderr, "ERROR: double point: %c %lf, %lf (%lf) to %c %lf, %lf (%lf) (%p)\n", lm, last[0],last[1], last[2], m, c[0], c[1], c[2], ptr);
			exit(1);
		}
		last[0] = c[0]; last[1] = c[1];
	}
};

// We have to transform generic/specific int pair land uses into one numbering system and back!

// Edge-wrapper...turns out CDT::Edge is so deeply templated that stuffing it in a map crashes CW8.
// So we build a dummy wrapper around an edge to prevent a template with a huge expanded name.
#if 0
struct	edge_wrapper {
	edge_wrapper() : edge(CDT::Edge()) { };
	edge_wrapper(const CDT::Edge e) : edge(e) { };
	edge_wrapper(const edge_wrapper& x) : edge(x.edge) { }
	edge_wrapper(CDT::Face_handle f, int i) : edge(CDT::Edge(f, i)) { }
	edge_wrapper& operator=(const edge_wrapper& x) { edge = x.edge; return *this; }
	bool operator==(const edge_wrapper& x) const { return edge == x.edge; }
	bool operator!=(const edge_wrapper& x) const { return edge != x.edge; }
	bool operator<(const edge_wrapper& x) const { return edge < x.edge; }

	const Face_handle orig_face(void) const { return edge.first->info().orig_face; }

	CDT::Edge	edge;
};
#endif

HASH_MAP_NAMESPACE_START
#if MSC
template<> inline
size_t hash_value<edge_wrapper>(const edge_wrapper& key)
{
	return (size_t) &*key.edge.first + (size_t) key.edge.second;
}
#else
struct hash_edge {
	typedef CDT::Edge		KeyType;
	// Trick: we think most ptrs are 4-byte aligned - reuse lower 2 bits.
	size_t operator()(const KeyType& key) const { return (size_t) &*key.first + (size_t) key.second; }
};
#endif
HASH_MAP_NAMESPACE_END


// Given a beach edge, fetch the beach-type coords.  last means use the target rather than src pt.
static void BeachPtGrab(const CDT::Edge& edge, bool last, const CDT& inMesh, double coords[3], int kind)
{
	CDT::Face_circulator stop, circ;
//	int	lterrain = NO_VALUE;

//	GISVertex * pm_vs = pm_edge->source();
//	GISVertex * pm_vt = pm_edge->target();

//	loc = inMesh.locate(CDT::Point(pm_vs->point().x, pm_vs->point().y), lt, i, loc);
//	Assert(lt == CDT::VERTEX);
	CDT::Vertex_handle v_s = edge.first->vertex(CDT::ccw(edge.second));
	DebugAssert(!inMesh.is_infinite(v_s));
//	loc = inMesh.locate(CDT::Point(pm_vt->point().x, pm_vt->point().y), lt, i, loc);
//	Assert(lt == CDT::VERTEX);
	CDT::Vertex_handle v_t = edge.first->vertex(CDT::cw(edge.second));
	DebugAssert(!inMesh.is_infinite(v_t));

	if (last)
	{
		coords[0] = CGAL::to_double(v_t->point().x());
		coords[1] = CGAL::to_double(v_t->point().y());
//		coords[2] = v_t->info().height;
	} else {
		coords[0] = CGAL::to_double(v_s->point().x());
		coords[1] = CGAL::to_double(v_s->point().y());
//		coords[2] = v_s->info().height;
	}
#if 0
	Vector3 nrml_s(0.0, 0.0, 0.0);
	Vector3 nrml_t(0.0, 0.0, 0.0);

	stop = circ = inMesh.incident_faces(v_s);
	do {
		if (!inMesh.is_infinite(circ) && circ->info().terrain != terrain_Water)
		{
//			if (lterrain == NO_VALUE)								lterrain = circ->info().terrain_specific;
//			else if (lterrain != circ->info().terrain_specific)		lterrain = NO_DATA;
			nrml_s.dx += circ->info().normal[0];
			nrml_s.dy += circ->info().normal[1];
			nrml_s.dz += circ->info().normal[2];
		}
		++circ;
	} while (stop != circ);

	stop = circ = inMesh.incident_faces(v_t);
	do {
		if (!inMesh.is_infinite(circ) && circ->info().terrain != terrain_Water)
		{
			nrml_t.dx += circ->info().normal[0];
			nrml_t.dy += circ->info().normal[1];
			nrml_t.dz += circ->info().normal[2];
		}
		++circ;
	} while (stop != circ);

	nrml_s.normalize();
	nrml_t.normalize();

	if (last)
	{
		coords[3] = nrml_t.dx;
		coords[4] =-nrml_t.dy;
	} else {
		coords[3] = nrml_s.dx;
		coords[4] =-nrml_s.dy;
	}

	DebugAssert(pythag(coords[3],coords[4]) <= 1.01);
#endif
	coords[2] = kind;
	//printf("Beach: %lf,%lf,%lf,%lf,%lf,%lf\n",coords[0],coords[1],coords[2],coords[3],coords[4],coords[5]);
}

float GetParamConst(const Face_handle face, int e)
{
	GISParamMap::const_iterator i = face->data().mParams.find(e);
	if (i == face->data().mParams.end()) return 0.0;
	return i->second;
}

#define	INLAND_BLEND_DIST 5.0

inline bool IsCustomOverWaterHard(int n)
{
	if (n == terrain_Water)	return false;
	if (n == terrain_VisualWater)	return false;
	return gNaturalTerrainInfo[n].custom_ter == tex_custom_hard_water;
}

inline bool IsCustomOverWaterSoft(int n)
{
	if (n == terrain_Water)	return false;
	if (n == terrain_VisualWater)	return false;
	return gNaturalTerrainInfo[n].custom_ter == tex_custom_soft_water;
}

inline bool IsCustomOverWaterAny(int n)
{
	if (n == terrain_Water)	return false;
	if (n == terrain_VisualWater)	return false;
	return gNaturalTerrainInfo[n].custom_ter == tex_custom_hard_water ||
			gNaturalTerrainInfo[n].custom_ter == tex_custom_soft_water;
}

inline bool IsCustomOrtho(int n)
{
	return n != terrain_Water && gNaturalTerrainInfo[n].custom_ter == tex_custom_pseudo_ortho;
}

inline bool IsCustom(int n)
{
	if (n == terrain_Water)	return false;
	return gNaturalTerrainInfo[n].custom_ter != tex_not_custom;
}


inline double tri_area(const Point2& p1, const Point2& p2, const Point2& p3)
{
	double v1_dx = p2.x() - p1.x();
	double v1_dy = p2.y() - p1.y();
	double v2_dx = p3.x() - p2.x();
	double v2_dy = p3.y() - p2.y();
	return (v1_dx * v2_dy - v1_dy * v2_dx) * 0.5;
}

static void ProjectTex(double lon, double lat, double& s, double& t, const tex_proj_info * info)
{
	Point2 p(lon, lat);
	double total1 = tri_area(info->corners[0],info->corners[1],info->corners[2]);
	double total2 = tri_area(info->corners[0],info->corners[2],info->corners[3]);

	double a1_0 = tri_area(info->corners[1],info->corners[2],p);
	double a1_1 = tri_area(info->corners[2],info->corners[0],p);
	double a1_2 = tri_area(info->corners[0],info->corners[1],p);

	double a2_0 = tri_area(info->corners[2],info->corners[3],p);
	double a2_2 = tri_area(info->corners[3],info->corners[0],p);
	double a2_3 = tri_area(info->corners[0],info->corners[2],p);

	double most_neg_1 = min(min(a1_0,a1_1),a1_2);
	double most_neg_2 = min(min(a2_0,a2_2),a2_3);

	if (most_neg_1 < most_neg_2)
	{
		// use 2
		double r0 = a2_0 / total2;
		double r2 = a2_2 / total2;
		double r3 = a2_3 / total2;
		s = info->ST[0].x() * r0 + info->ST[2].x() * r2 + info->ST[3].x() * r3;
		t = info->ST[0].y() * r0 + info->ST[2].y() * r2 + info->ST[3].y() * r3;
	}
	else
	{
		// use 2
		double r0 = a1_0 / total1;
		double r1 = a1_1 / total1;
		double r2 = a1_2 / total1;
		s = info->ST[0].x() * r0 + info->ST[1].x() * r1 + info->ST[2].x() * r2;
		t = info->ST[0].y() * r0 + info->ST[1].y() * r1 + info->ST[2].y() * r2;
	}
	if(s > -0.001 & s < 0.0) s = 0.0;
	if(t > -0.001 & t < 0.0) t = 0.0;
	if(s <  1.001 & s > 1.0) s = 1.0;
	if(t <  1.001 & t > 1.0) t = 1.0;
}



static double GetWaterBlend(CDT::Vertex_handle v_han, const DEMGeo& dem_land, const DEMGeo& dem_water)
{
	double lon, lat;
	lon = doblim(CGAL::to_double(v_han->point().x()),dem_land.mWest ,dem_land.mEast );
	lat = doblim(CGAL::to_double(v_han->point().y()),dem_land.mSouth,dem_land.mNorth);

	float land_ele  = dem_land.value_linear(lon, lat);
	float water_ele = dem_water.value_linear(lon, lat);

	float ret = interp(0, 0, 50, 1, land_ele - water_ele);

	v_han->info().wave_height = ret;

	if (ret > 1.0)
		printf("Over.\n");
	if (ret < 0.0)
		printf("Under.\n");
	return ret;
}

// Tightness - given a vertex on a face and a certain terrain border we're putting down on that face,
// what "tightness" shouldd the transition have - that's basically the T coord of the dither control mask.
static double GetTightnessBlend(CDT& inMesh, CDT::Face_handle f_han, CDT::Vertex_handle v_han, int terrain)
{
	// First check for projetion problems.  Take a vector of the angle this iterrain will proj at and
	// the tri normal.  If they are 'shear' by more than 45 degrees, the projection is going to look like
	// ass.  In that case automatically tighten up the border via a cos^2 power curve, for the tightest
	// border at totally shear angle.
	Vector3	tproj(0,0,1);
	int proj = gNaturalTerrainInfo[terrain].proj_angle;
	if (proj == proj_EastWest)	tproj = Vector3(1,0,0);
	if (proj == proj_NorthSouth)	tproj = Vector3(0,1,0);

	Vector3	tri(f_han->info().normal[0],
				f_han->info().normal[1],
				f_han->info().normal[2]);

	double proj_err_dot = fabs(tri.dot(tproj));
	if (proj_err_dot < 0.7)
	{
		return 1.0 - proj_err_dot * proj_err_dot;
	}

	// Okay we don't have proj problems...basically find the biggest angle change (smallest
	// dot product of normals) between the tri we are doing now and any of the incident neighbors
	// who share the terrain.  Translate that into an angle from 0 (planar) to 90 (right turn),
	// and that is indexed into the T coord.

	double smallest_dot = 1.0;
	CDT::Face_circulator stop, circ; // , last;
	stop = circ = inMesh.incident_faces(v_han);
	Vector3	up(0,0,1);
	do {
//		last = circ;
		++circ;
		if (!inMesh.is_infinite(circ))
		if (circ->info().terrain == terrain || circ->info().terrain_border.count(terrain))					// We know we'll hit this iat least once, because circ must equal f_han once.
		{
			Vector3	v1(circ->info().normal[0],circ->info().normal[1],circ->info().normal[2]);
			Vector3	v2(f_han->info().normal[0],f_han->info().normal[1],f_han->info().normal[2]);
			smallest_dot = min(smallest_dot, v1.dot(v2));
			smallest_dot = min(smallest_dot, v1.dot(up));
		}

	} while (stop != circ);
	smallest_dot = max( 0.0, smallest_dot);				// must be non-negative!
	smallest_dot = acos(smallest_dot) / (PI / 2.0);
	return smallest_dot;
}

// Given an edge, finds the next edge clockwise from the source vertex
// of this edge.  (Pmwx equivalent is twin->next
CDT::Edge edge_twin_next(const CDT::Edge& e)
{
	CDT::Edge new_e;
	int center_index = CDT::ccw(e.second);
	CDT::Vertex_handle center = e.first->vertex(center_index);

	new_e.first = e.first->neighbor(e.second);
	new_e.second = CDT::cw(new_e.first->index(center));
	return new_e;
}

// Given an edge, find the next edge in a clockwise circulation
// around its target vertex.  (Pmwx equivalent is next->twin)
CDT::Edge edge_next_twin(const CDT::Edge& e)
{
	CDT::Edge new_e;

	new_e.first = e.first->neighbor(CDT::ccw(e.second));
	new_e.second = CDT::cw(new_e.first->index(e.first->vertex(e.second)));
	return new_e;
}


// Given an edge, find the leftmost turn connected to us.  (pmwx equivalent is next)
CDT::Edge edge_next(const CDT::Edge& e)
{
	CDT::Edge new_e(e);
	new_e.second = CDT::ccw(new_e.second);
	return new_e;
}

// Find the edge in opposite direction (pmwx version is twin)
CDT::Edge edge_twin(const CDT::Edge& e)
{
	CDT::Edge new_e(e);
	CDT::Vertex_handle v = e.first->vertex(CDT::ccw(e.second));
	new_e.first = e.first->neighbor(e.second);
	new_e.second = CDT::ccw(new_e.first->index(v));
	return new_e;
}

int is_coast(const CDT::Edge& inEdge, const CDT& inMesh)
{
	if (inMesh.is_infinite(inEdge.first)) return false;
	if (inMesh.is_infinite(inEdge.first->neighbor(inEdge.second))) return false;

	if (inEdge.first->info().terrain != terrain_Water) return false;
	if (inEdge.first->neighbor(inEdge.second)->info().terrain == terrain_Water) return false;
	return true;
}

double edge_len(const CDT::Edge& e)
{
	CDT::Vertex_handle	v_s = e.first->vertex(CDT::ccw(e.second));
	CDT::Vertex_handle	v_t = e.first->vertex(CDT:: cw(e.second));
	return LonLatDistMeters(CGAL::to_double(v_s->point().x()),CGAL::to_double(v_s->point().y()),
							CGAL::to_double(v_t->point().x()),CGAL::to_double(v_t->point().y()));
}

bool edge_convex(const CDT::Edge& e1, const CDT::Edge& e2)
{
	CDT::Vertex_handle	e1s = e1.first->vertex(CDT::ccw(e1.second));
	CDT::Vertex_handle	e1t = e1.first->vertex(CDT:: cw(e1.second));

	CDT::Vertex_handle	e2s = e2.first->vertex(CDT::ccw(e2.second));
	CDT::Vertex_handle	e2t = e2.first->vertex(CDT:: cw(e2.second));

	DebugAssert(e1t == e2s);

	Point2	p1(CGAL::to_double(e1s->point().x()),CGAL::to_double(e1s->point().y()));
	Point2	p2(CGAL::to_double(e1t->point().x()),CGAL::to_double(e1t->point().y()));
	Point2	p3(CGAL::to_double(e2t->point().x()),CGAL::to_double(e2t->point().y()));

	Vector2	v1(p1,p2);
	Vector2 v2(p2,p3);

	return v1.left_turn(v2);
}

double edge_angle(const CDT::Edge& e1, const CDT::Edge& e2)
{
	CDT::Vertex_handle	e1s = e1.first->vertex(CDT::ccw(e1.second));
	CDT::Vertex_handle	e1t = e1.first->vertex(CDT:: cw(e1.second));

	CDT::Vertex_handle	e2s = e2.first->vertex(CDT::ccw(e2.second));
	CDT::Vertex_handle	e2t = e2.first->vertex(CDT:: cw(e2.second));

	DebugAssert(e1t == e2s);

	Point2	p1(CGAL::to_double(e1s->point().x()),CGAL::to_double(e1s->point().y()));
	Point2	p2(CGAL::to_double(e1t->point().x()),CGAL::to_double(e1t->point().y()));
	Point2	p3(CGAL::to_double(e2t->point().x()),CGAL::to_double(e2t->point().y()));

	Vector2	v1(p1,p2);
	Vector2 v2(p2,p3);
	double scale = cos(p2.y() * DEG_TO_RAD);
	v1.dx *= scale;
	v2.dx *= scale;
	v1.normalize();
	v2.normalize();

	return v1.dot(v2);
}


int	has_beach(const CDT::Edge& inEdge, const CDT& inMesh, int& kind)
{
	if(gMobile)
	{
		return false;
	}
	
	if (!is_coast(inEdge, inMesh))	return false;

	CDT::Face_handle tri = inEdge.first;

	DebugAssert(tri->info().terrain == terrain_Water);
	if (tri->info().terrain == terrain_Water)
		tri = inEdge.first->neighbor(inEdge.second);

	int lterrain = tri->info().terrain;
	int is_apt = IsAirportTerrain(lterrain);
	int i;

	if(IsCustom(lterrain)) return false;

	CDT::Vertex_handle v_s = inEdge.first->vertex(CDT::ccw(inEdge.second));
	CDT::Vertex_handle v_t = inEdge.first->vertex(CDT::cw(inEdge.second));

	const Face_handle orig_face = inEdge.first->info().orig_face;
	//Assert(orig_face != NULL);

	double	prev_ang = 1.0, next_ang = 1.0;
	bool	prev_convex = true, next_convex =true;
	double prev_len = 0.0;
	double next_len = 0.0;

	// Find our outgoing (next) angle
	for (CDT::Edge iter = edge_next(inEdge); iter != edge_twin(inEdge); iter = edge_twin_next(iter))
	if (is_coast(iter, inMesh))
	{
		next_ang = edge_angle(inEdge, iter);
		next_convex = edge_convex(inEdge, iter);
		next_len = edge_len(iter);
		break;
	}

	// Find our incoming (previous) angle
	for (CDT::Edge iter = edge_next_twin(edge_twin(inEdge)); iter != edge_twin(inEdge); iter = edge_next_twin(iter))
	if (is_coast(iter, inMesh))
	{
		prev_ang = edge_angle(iter, inEdge);
		prev_convex = edge_convex(iter, inEdge);
		prev_len = edge_len(iter);
	}

	double		wave = (v_s->info().wave_height + v_t->info().wave_height) * 0.5;
	double		len = LonLatDistMeters(CGAL::to_double(v_s->point().x()),CGAL::to_double(v_s->point().y()),
									   CGAL::to_double(v_t->point().x()),CGAL::to_double(v_t->point().y())) + prev_len + next_len;

	double slope = tri->info().normal[2];

	for (i = 0; i < gBeachInfoTable.size(); ++i)
	{
		if (is_apt == gBeachInfoTable[i].require_airport &&
			slope >= gBeachInfoTable[i].min_slope &&
			slope <= gBeachInfoTable[i].max_slope &&
			gBeachInfoTable[i].min_sea <= wave &&
			wave <= gBeachInfoTable[i].max_sea &&
			prev_ang >= (prev_convex ? gBeachInfoTable[i].max_turn_convex : gBeachInfoTable[i].max_turn_concave) &&
			next_ang >= (next_convex ? gBeachInfoTable[i].max_turn_convex : gBeachInfoTable[i].max_turn_concave) &&
			fabs(CGAL::to_double(tri->vertex(0)->point().y())) >= gBeachInfoTable[i].min_lat &&
			fabs(CGAL::to_double(tri->vertex(0)->point().y())) <= gBeachInfoTable[i].max_lat &&
			tri->info().mesh_temp >= gBeachInfoTable[i].min_temp &&
			tri->info().mesh_temp <= gBeachInfoTable[i].max_temp &&
			tri->info().mesh_rain >= gBeachInfoTable[i].min_rain &&
			tri->info().mesh_rain <= gBeachInfoTable[i].max_rain &&
//			len >= gBeachInfoTable[i].min_len &&
			gBeachInfoTable[i].min_area < GetParamConst(orig_face, af_WaterArea) &&
			(gBeachInfoTable[i].require_open == 0 || GetParamConst(orig_face,af_WaterOpen) != 0.0))

		{
			kind = gBeachInfoTable[i].x_beach_type;
			break;
		}
	}

	if (i == gBeachInfoTable.size())
	{
		return false;
	}
	return true;
}

#if MSC
typedef hash_map<CDT::Edge,CDT::Edge>	edge_hash_map;
typedef hash_map<CDT::Edge, int>			edge_info_map;
#else
typedef hash_map<CDT::Edge, CDT::Edge, hash_edge> edge_hash_map;
typedef hash_map<CDT::Edge, int, hash_edge> edge_info_map;
#endif

void FixBeachContinuity(
						edge_hash_map&								linkNext,
						const CDT::Edge&							this_start,
						edge_info_map&								typedata)
{
	CDT::Edge circ, discon, stop, iter;
	bool retry;

	for (int lim = 0; lim < gBeachInfoTable.size(); ++lim)
	{
		do {
			retry = false;
			circ = stop = this_start;

			// Main circulator group on each beac htype
			do {
				discon = circ;

				// Keep trying until our beach meets requirements
				// Calculate contiguous type-length
				double len = 0;
				double req_len = gBeachInfoTable[gBeachIndex[typedata[circ]]].min_len;
				do
				{
					len += edge_len(discon);
					discon = (linkNext.count(discon) == 0) ? CDT::Edge() : linkNext[discon];
					// incr disocn!
				} while (discon != CDT::Edge() && discon != stop && typedata[discon] == typedata[circ]);

				// If we failed - go back and retry, otherwise advance forward and break out
				if (len < req_len && gBeachIndex[typedata[circ]] < lim)
				{
					retry = true;
					int new_type = gBeachInfoTable[gBeachIndex[typedata[circ]]].x_backup;
					iter = circ;
					do {
						typedata[iter] = new_type;
						iter = (linkNext.count(iter) == 0) ? CDT::Edge() : linkNext[iter];
					} while (iter != discon);

				}
				circ = discon;

			} while (circ != stop && circ != CDT::Edge());
		} while (retry);
	}
}
bool StripSoft(string& n)
{
	if(n.size() > 5 && n.substr(n.size()-5) == "_soft")
	{
		n.erase(n.size()-5,5);
		return true;
	}
	if(n.size() > 5 && n.substr(n.size()-5) == "_hard")
	{
		n.erase(n.size()-5,5);
		return true;
	}
	return false;
}


string		get_terrain_name(int composite)
{
	if (composite == terrain_Water)
	{
		return FetchTokenString(composite);
	}
	else if (gNaturalTerrainInfo.count(composite) > 0)
	{
		if(IsCustom(composite) && !IsCustomOrtho(composite))
		{
			if(IsCustomOverWaterAny(composite))
			{
				string n=FetchTokenString(composite);
				StripSoft(n);
				return n;
			}
			return FetchTokenString(composite);
		}
		else if(IsCustomOrtho(composite))
		{
			const string prefix = "lib/mobile/autogen/";
			return prefix + string(FetchTokenString(composite)) + ".ter";
		}
		else
		{
			const string prefix = gMobile ? "lib/g8/" : "lib/g10/";
			return prefix + string(FetchTokenString(composite)) + ".ter";
		}
	}

//	DebugAssert(!"bad terrain.");
	AssertPrintf("WARNING: no name for terrain %d (token=%s\n", composite, FetchTokenString(composite));
	return "UNKNOWN TERRAIN!";
}

struct SortByLULayer {
	bool operator()(const int& lhs, const int& rhs) const {
		if (lhs >= terrain_Natural && rhs >= terrain_Natural)
		{
			bool custom_lhs = IsCustom(lhs) && !IsCustomOrtho(lhs);
			bool custom_rhs = IsCustom(rhs) && !IsCustomOrtho(rhs);
			if (!custom_lhs && !custom_rhs)
				return LowerPriorityNaturalTerrain(lhs,rhs);
		}
		return lhs < rhs;
	}
};

struct	StNukeWriter {
	StNukeWriter(void * writer) : writer_(writer) { }
	~StNukeWriter() { if (writer_) DSFDestroyWriter(writer_); }
	void * writer_;
};




void 	CHECK_TRI(CDT::Vertex_handle a, CDT::Vertex_handle b, CDT::Vertex_handle c)
{
	if (a->point().x() == b->point().x() && a->point().y() == b->point().y())
	{
		if (a == b)	fprintf(stderr, "Dupe point same handle"); else fprintf(stderr, "Dupe point, diff handle");
		return;
	}
	if (a->point().x() == c->point().x() && a->point().y() == c->point().y())
	{
		if (a == c)	fprintf(stderr, "Dupe point same handle"); else fprintf(stderr, "Dupe point, diff handle");
		return;
	}
	if (b->point().x() == c->point().x() && b->point().y() == c->point().y())
	{
		if (b == c)	fprintf(stderr, "Dupe point same handle"); else fprintf(stderr, "Dupe point, diff handle");
		return;
	}
}

struct	ObjPrio {

	bool operator()(const int& lhs, const int& rhs) const
	{
		bool lfeat = IsFeatureObject(lhs);
		bool rfeat = IsFeatureObject(rhs);
		if (lfeat && rfeat) return lhs < rhs;
		if (lfeat)			return false;
		if (rfeat)			return true;
							return lhs < rhs;
	}
};


static int IsAliased(int lu)
{
	if (lu == terrain_VisualWater)
		return terrain_Water;
	if (IsCustomOverWaterSoft(lu))
	{
		string  rn(FetchTokenString(lu));
		if(!StripSoft(rn)) return NO_VALUE;

		int lup = LookupToken(rn.c_str());
		return (lup == -1) ? 0 : lup;
	}
	return 0;
}

tex_proj_info project_ortho(const CDT::Face_handle &face, const DEMGeo &dsf_bounds, int rotation_deg)
{
	const Bbox2 ortho_grid_square = get_ortho_grid_square_bounds(face, Bbox2(dsf_bounds.mWest, dsf_bounds.mSouth, dsf_bounds.mEast, dsf_bounds.mNorth));

	tex_proj_info ortho_projection = {};
	ortho_projection.corners[0] = ortho_grid_square.bottom_left();
	ortho_projection.corners[1] = ortho_grid_square.bottom_right();
	ortho_projection.corners[2] = ortho_grid_square.top_right();
	ortho_projection.corners[3] = ortho_grid_square.top_left();
	ortho_projection.ST[0] = Point2(0, 0);
	ortho_projection.ST[1] = Point2(1, 0);
	ortho_projection.ST[2] = Point2(1, 1);
	ortho_projection.ST[3] = Point2(0, 1);

	DebugAssert(rotation_deg % 90 == 0);
	while(rotation_deg > 0)
	{
		const Point2 old_bl = ortho_projection.ST[0];
		ortho_projection.ST[0] = ortho_projection.ST[1];
		ortho_projection.ST[1] = ortho_projection.ST[2];
		ortho_projection.ST[2] = ortho_projection.ST[3];
		ortho_projection.ST[3] = old_bl;
		rotation_deg -= 90;
	}

	return ortho_projection;
}

bool has_some_border(const vector<CDT::Face_handle> &hi_res_tris, int land_use)
{
	if(hi_res_tris.empty())
		return false;

	for(vector<CDT::Face_handle>::const_iterator face = hi_res_tris.begin(); face != hi_res_tris.end(); ++face)
	{
		if((*face)->info().terrain_border.count(land_use))
		{
			return true;
		}
	}
	return false;
}

void	BuildDSF(
			const char *	inFileName1,
			const char *	inFileName2,
			const DEMGeo&	inElevation,
			const DEMGeo&	inBathymetry,
			const DEMGeo&	inUrbanDensity,
//			const DEMGeo&	inVegeDem,
			CDT&			inHiresMesh,
//			CDT&			inLoresMesh,
			Pmwx&			inVectorMap,
			ProgressFunc	inProgress)
{


vector<CDT::Face_handle>	sHiResTris[get_patch_dim_hi() * get_patch_dim_hi()];
vector<CDT::Face_handle>	sLoResTris[get_patch_dim_lo() * get_patch_dim_hi()];
set<int>					sHiResLU[get_patch_dim_hi() * get_patch_dim_hi()];
set<int>					sHiResBO[get_patch_dim_hi() * get_patch_dim_hi()];
set<int>					sLoResLU[get_patch_dim_lo() * get_patch_dim_lo()];


		double	prog_c = 0.0;
		int		debug_add_tri_fan = 0;
		int		debug_sub_tri_fan = 0;
		int		total_tris = 0;
		int		total_tri_fans = 0;
		int		total_tri_fan_pts = 0;
		int		border_tris = 0;
		int		total_patches = 0;
		int		total_objs = 0;
		int		total_polys = 0;
		int		total_chains = 0;
		int		total_shapes = 0;


		int		cur_id = 0, tri, tris_this_patch;
		double	coords3[3];
		double	coords4[4];
		double	coords2[2];
		//double	coords6[6];
		double	coords8[9];
		double							x, y, v;
		CDT::Finite_faces_iterator		fi;
		CDT::Vertex_handle				avert;
		CDT::Finite_vertices_iterator	vert;
		map<int, int, SortByLULayer>::iterator 		lu_ranked;
		map<int, int>::iterator 		lu;
		map<int, int>::iterator 		obdef;
		map<int, int, ObjPrio>::iterator obdef_prio;
		set<int>::iterator				border_lu;
		bool							is_water, is_overlay;
		list<CDT::Face_handle>::iterator nf;

		Pmwx::Face_iterator						pf;
		GISObjPlacementVector::iterator			pointObj;
		GISPolyObjPlacementVector::iterator		polyObj;
		Polygon2::iterator						polyPt;
		vector<Polygon2>::iterator				polyHole;

		void *			writer1, * writer2;
		DSFCallbacks_t	cbs;

		Net_JunctionInfoSet				junctions;
		Net_ChainInfoSet				chains;
		Net_JunctionInfoSet::iterator 	ji;
		Net_ChainInfoSet::iterator 		ci;
		vector<Point2>::iterator		shapePoint;

		map<int, int, SortByLULayer>landuses;			// This is a map from DSF to layer, used to start a patch and generally get organized.  Sorting is specialized to be by LU layering from config file.
		map<int, int>				landuses_reversed;	// This is a map from DSF layer to land-use, used to write out DSF layers in order.
		map<int, int>				objects_reversed;
		map<int, int>	facades,	facades_reversed;
		map<int, int, ObjPrio>		objects;

		char	prop_buf[256];

		deferred_pool	must_dealloc;

	/****************************************************************
	 * SETUP
	 ****************************************************************/

	double hmin = 9.9e9, hmax = -9.9e9;
	for(vert = inHiresMesh.finite_vertices_begin(); vert != inHiresMesh.finite_vertices_end(); ++vert)
	{
		hmin = min(hmin, vert->info().height);
		hmax = max(hmax, vert->info().height);
	}
	int emin = floor(hmin);
	int emax = ceil(hmax);
	int erange = emax - emin;
	int erange2 = 1;
	while(erange2 <= erange) erange2 *= 2;
	erange2--;
	int extra = erange2 - erange;
	int use_min = emin - extra/2;
	int use_max = use_min + erange2;
	printf("Real span: %lf to %lf.  Using: %d to %d\n", hmin, hmax, use_min, use_max);
	// Andrew: change divisions to 16
	writer1 = inFileName1 ? DSFCreateWriter(inElevation.mWest, inElevation.mSouth, inElevation.mEast, inElevation.mNorth, -32768, 32767, DSF_DIVISIONS) : NULL;
	writer2 = inFileName2 ? ((inFileName1 && strcmp(inFileName1,inFileName2)==0) ? writer1 : DSFCreateWriter(inElevation.mWest, inElevation.mSouth, inElevation.mEast, inElevation.mNorth,use_min, use_max, DSF_DIVISIONS)) : NULL;
	StNukeWriter	dontLeakWriter1(writer1);
	StNukeWriter	dontLeakWriter2(writer2==writer1 ? NULL : writer2);
 	DSFGetWriterCallbacks(&cbs);

	/****************************************************************
	 * MESH GENERATION
	 ****************************************************************/

	// First assign IDs to each triangle to differentiate patches.
	// Also work out land uses.

	if (inProgress && inProgress(0, 5, "Compiling Mesh", 0.0)) return;

	#if DEV
	for (fi = inHiresMesh.finite_faces_begin(); fi != inHiresMesh.finite_faces_end(); ++fi)
	for(int n = 0; n < 3; ++n)
	{
		double x = CGAL::to_double(fi->vertex(n)->point().x());
		double y = CGAL::to_double(fi->vertex(n)->point().y());

		if(x < inElevation.mWest)
			DebugAssert(fi->vertex(n)->point().x() == inElevation.mWest);
		if(x > inElevation.mEast)
			DebugAssert(fi->vertex(n)->point().x() == inElevation.mEast);

		if(y < inElevation.mSouth)
			DebugAssert(fi->vertex(n)->point().y() == inElevation.mSouth);
		if(y > inElevation.mNorth)
			DebugAssert(fi->vertex(n)->point().y() == inElevation.mNorth);

		if(fi->vertex(n)->point().y() > inElevation.mNorth)
			printf("WARNING: out of bounds pt: %lf\n", CGAL::to_double(fi->vertex(n)->point().y()));
		if(fi->vertex(n)->point().y() < inElevation.mSouth)
			printf("WARNING: out of bounds pt: %lf\n", CGAL::to_double(fi->vertex(n)->point().y()));

		if(fi->vertex(n)->point().x() > inElevation.mEast)
			printf("WARNING: out of bounds pt: %lf\n", CGAL::to_double(fi->vertex(n)->point().x()));
		if(fi->vertex(n)->point().x() < inElevation.mWest)
			printf("WARNING: out of bounds pt: %lf\n", CGAL::to_double(fi->vertex(n)->point().x()));
	}
	#endif

	if(writer1)
	for (fi = inHiresMesh.finite_faces_begin(); fi != inHiresMesh.finite_faces_end(); ++fi)
	{
		fi->info().flag = 0;

		if (fi->vertex(0)->point().y() >= inElevation.mNorth &&
			fi->vertex(1)->point().y() >= inElevation.mNorth &&
			fi->vertex(2)->point().y() >= inElevation.mNorth)
		{
			printf("WARNING: skipping colinear out of bounds triange.\n");
			continue;
		}

		if (fi->vertex(0)->point().y() <= inElevation.mSouth &&
			fi->vertex(1)->point().y() <= inElevation.mSouth &&
			fi->vertex(2)->point().y() <= inElevation.mSouth)
		{
			printf("WARNING: skipping colinear out of bounds triange.\n");
			continue;
		}


		if (fi->vertex(0)->point().x() >= inElevation.mEast &&
			fi->vertex(1)->point().x() >= inElevation.mEast &&
			fi->vertex(2)->point().x() >= inElevation.mEast)
		{
			printf("WARNING: skipping colinear out of bounds triange.\n");
			continue;
		}


		if (fi->vertex(0)->point().x() <= inElevation.mWest &&
			fi->vertex(1)->point().x() <= inElevation.mWest &&
			fi->vertex(2)->point().x() <= inElevation.mWest)
		{
			printf("WARNING: skipping colinear out of bounds triange.\n");
			continue;
		}

		if (fi->vertex(0)->point().y() == fi->vertex(1)->point().y() &&
			fi->vertex(0)->point().y() == fi->vertex(2)->point().y())
		{
			printf("WARNING: Y-colinear triangle. skipping.\n");
			continue;
		}
		if (fi->vertex(0)->point().x() == fi->vertex(1)->point().x() &&
			fi->vertex(0)->point().x() == fi->vertex(2)->point().x())
		{
			printf("WARNING: X-colinear triangle. skipping.\n");
			continue;
		}

		x = (CGAL::to_double(fi->vertex(0)->point().x() + fi->vertex(1)->point().x() + fi->vertex(2)->point().x())) / 3.0;
		y = (CGAL::to_double(fi->vertex(0)->point().y() + fi->vertex(1)->point().y() + fi->vertex(2)->point().y())) / 3.0;

		x = (x - inElevation.mWest) / (inElevation.mEast - inElevation.mWest);
		y = (y - inElevation.mSouth) / (inElevation.mNorth - inElevation.mSouth);

		x = floor(x*get_patch_dim_hi());
		y = floor(y*get_patch_dim_hi());

		if (x == get_patch_dim_hi()) x = get_patch_dim_hi()-1;
		if (y == get_patch_dim_hi()) y = get_patch_dim_hi()-1;
		if (x < 0 || y < 0 || x > get_patch_dim_hi() || y > get_patch_dim_hi())
			fprintf(stderr, "Hires Triangle out of range, patch %lf,%lf, coords are %lf,%lf %lf,%lf %lf,%lf\n", x, y,
				CGAL::to_double(fi->vertex(0)->point().x()),CGAL::to_double(fi->vertex(0)->point().y()),
				CGAL::to_double(fi->vertex(1)->point().x()),CGAL::to_double(fi->vertex(1)->point().y()),
				CGAL::to_double(fi->vertex(2)->point().x()),CGAL::to_double(fi->vertex(2)->point().y()));

		// Accumulate the various texes into the various layers.  This means marking what land uses we have per each patch
		// and also any borders we need.
		sHiResTris[(int) x + (int) y * get_patch_dim_hi()].push_back(fi);
		DebugAssert(fi->info().terrain != -1);
		landuses.insert(map<int, int, SortByLULayer>::value_type(fi->info().terrain,0));
		// special case: maybe the hard variant is never used?  In that case, make sure to accum it here or we'll never export that land use.
		if(IsAliased(fi->info().terrain))
			landuses.insert(map<int, int, SortByLULayer>::value_type(IsAliased(fi->info().terrain),0));
		sHiResLU[(int) x + (int) y * get_patch_dim_hi()].insert(fi->info().terrain);


		if(IsCustomOverWaterHard(fi->info().terrain))
		{
			// Over water, but maintain hard physics.  So we need to put ourselves in the visual layer, and make sure there is water for aliasing.
			landuses.insert(map<int, int, SortByLULayer>::value_type(terrain_Water,0));
			landuses.insert(map<int, int, SortByLULayer>::value_type(terrain_VisualWater,0));
			sHiResLU[(int) x + (int) y * get_patch_dim_hi()].insert(terrain_VisualWater);
		}
		if(IsCustomOverWaterSoft(fi->info().terrain))
		{
			// Over water soft - put us in the water layer.
			landuses.insert(map<int, int, SortByLULayer>::value_type(terrain_Water,0));
			sHiResLU[(int) x + (int) y * get_patch_dim_hi()].insert(terrain_Water);
		}

		for (border_lu = fi->info().terrain_border.begin(); border_lu != fi->info().terrain_border.end(); ++border_lu)
		{
			sHiResBO[(int) x + (int) y * get_patch_dim_hi()].insert(*border_lu);
			landuses.insert(map<int, int, SortByLULayer>::value_type(*border_lu,0));
			DebugAssert(*border_lu != -1);
		}
	}

	if (inProgress && inProgress(0, 5, "Compiling Mesh", 0.5)) return;

#if !NO_ORTHO
	if(writer1)
	for (fi = inLoresMesh.finite_faces_begin(); fi != inLoresMesh.finite_faces_end(); ++fi)
	{
		if (fi->vertex(0)->point().y() >= inElevation.mNorth &&
			fi->vertex(1)->point().y() >= inElevation.mNorth &&
			fi->vertex(2)->point().y() >= inElevation.mNorth)			continue;

		if (fi->vertex(0)->point().y() <= inElevation.mSouth &&
			fi->vertex(1)->point().y() <= inElevation.mSouth &&
			fi->vertex(2)->point().y() <= inElevation.mSouth)			continue;

		if (fi->vertex(0)->point().x() >= inElevation.mEast &&
			fi->vertex(1)->point().x() >= inElevation.mEast &&
			fi->vertex(2)->point().x() >= inElevation.mEast)			continue;

		if (fi->vertex(0)->point().x() <= inElevation.mWest &&
			fi->vertex(1)->point().x() <= inElevation.mWest &&
			fi->vertex(2)->point().x() <= inElevation.mWest)			continue;

		x = (fi->vertex(0)->point().x() + fi->vertex(1)->point().x() + fi->vertex(2)->point().x()) / 3.0;
		y = (fi->vertex(0)->point().y() + fi->vertex(1)->point().y() + fi->vertex(2)->point().y()) / 3.0;

		x = (x - inElevation.mWest) / (inElevation.mEast - inElevation.mWest);
		y = (y - inElevation.mSouth) / (inElevation.mNorth - inElevation.mSouth);

		x = floor(x*get_patch_dim_lo());
		y = floor(y*get_patch_dim_lo());

		if (x < 0 || y < 0 || x >= get_patch_dim_lo() || y >= get_patch_dim_lo())
			fprintf(stderr, "Lores Triangle out of range, patch %lf,%lf, coords are %lf,%lf %lf,%lf %lf,%lf\n", x, y,
				fi->vertex(0)->point().x(),fi->vertex(0)->point().y(),
				fi->vertex(1)->point().x(),fi->vertex(1)->point().y(),
				fi->vertex(2)->point().x(),fi->vertex(2)->point().y());

		sLoResTris[(int) x + (int) y * get_patch_dim_lo()].push_back(fi);
		landuses.insert(map<int, int, SortByLULayer>::value_type(fi->info().terrain,0));
		DebugAssert(fi->info().terrain != -1);
		sLoResLU[(int) x + (int) y * get_patch_dim_lo()].insert(fi->info().terrain);
	}
#endif

	// Now that we have our land uses, we can go back and calculate
	// the DSF-file-relative indices.

	cur_id = 0;
	if(writer1)
	for (lu_ranked = landuses.begin(); lu_ranked != landuses.end(); ++lu_ranked)
	if (!IsAliased(lu_ranked->first))
	{
		lu_ranked->second = cur_id;
		landuses_reversed[cur_id] = lu_ranked->first;
		++cur_id;
	}

	for (lu_ranked = landuses.begin(); lu_ranked != landuses.end(); ++lu_ranked)
	if(IsAliased(lu_ranked->first))
		lu_ranked->second = landuses[IsAliased(lu_ranked->first)];

	if (inProgress && inProgress(0, 5, "Compiling Mesh", 1.0)) return;

	if(writer1)
	for (prog_c = 0.0, lu_ranked = landuses.begin(); lu_ranked != landuses.end(); ++lu_ranked, prog_c += 1.0)
	{
		if (inProgress && inProgress(1, 5, "Sorting Mesh", prog_c / (float) landuses.size())) return;

		/***************************************************************************************************************************************
		 * WRITE OUT LOW RES ORTHOPHOTO PATCHES
		 ***************************************************************************************************************************************/

		is_water		= lu_ranked->first == terrain_VisualWater || lu_ranked->first == terrain_Water;
		is_overlay		= IsCustomOverWaterAny(lu_ranked->first);		// This layer is an overlay to water, so be sure to set the flags!

#if !NO_ORTHO
		for (cur_id = 0; cur_id < (get_patch_dim_lo()*get_patch_dim_lo()); ++cur_id)
		if (sLoResLU[cur_id].count(lu_ranked->first))
		{
			TriFanBuilder	fan_builder(&inLoresMesh);
			for (tri = 0; tri < sLoResTris[cur_id].size(); ++tri)
			{
				f = sLoResTris[cur_id][tri];
				if (f->info().terrain == lu_ranked->first)
				{
					CHECK_TRI(f->vertex(0),f->vertex(1),f->vertex(2));
					fan_builder.AddTriToFanPool(f);
				}
			}
			fan_builder.CalcFans();
			cbs.BeginPatch_f(lu_ranked->second, ORTHO_NEAR_LOD, ORTHO_FAR_LOD, 0, 5, writer1);
			list<CDT::Vertex_handle>				primv;
			list<CDT::Vertex_handle>::iterator		vert;
			int										primt;
			while(1)
			{
				primt = fan_builder.GetNextPrimitive(primv);
				if(primv.empty()) break;
				if(primt != dsf_Tri)
				{
					++total_tri_fans;
					total_tris += (primv.size() - 2);
				} else {
					total_tris += (primv.size() / 3);
					tris_this_patch += (primv.size() / 3);
				}
				if
				cbs.BeginPrimitive_f(primt, writer1);
				for(vert = primv.begin(); vert != primv.end(); ++vert)
				{
					coords8[0] = (*vert)->point().x();
					coords8[1] = (*vert)->point().y();
					coords8[2] =USE_DEM_H( (*vert)->info().height, false,inHiresMesh,(*vert));
					coords8[3] =USE_DEM_N( (*vert)->info().normal[0]);
					coords8[4] =USE_DEM_N(-(*vert)->info().normal[1]);
					DebugAssert(coords8[3] >= -1.0);
					DebugAssert(coords8[3] <=  1.0);
					DebugAssert(coords8[4] >= -1.0);
					DebugAssert(coords8[4] <=  1.0);
					cbs.AddPatchVertex_f(coords8, writer1);
				}
				cbs.EndPrimitive_f(writer1);
			}
			cbs.EndPatch_f(writer1);
			++total_patches;
		}
#endif

		/***************************************************************************************************************************************
		 * WRITE OUT HI RES BASE PATCHES
		 ***************************************************************************************************************************************/
		for (cur_id = 0; cur_id < (get_patch_dim_hi()*get_patch_dim_hi()); ++cur_id)
		if (sHiResLU[cur_id].count(lu_ranked->first))
		{
			vector<CDT::Face_handle> &hiResTris = sHiResTris[cur_id];
			int prev_ter_enum = -1;
			int prev_ter_rotation = 0;
			const int prev_patch_tris = tris_this_patch;
			for(tri = 0; tri < hiResTris.size(); ++tri)
			{
				CDT::Face_handle f = hiResTris[tri];
				const int ter_enum = f->info().terrain;
				const int rotation = f->info().orig_face->data().mRotationDeg;
				if (ter_enum == lu_ranked->first ||
					(IsCustomOverWaterHard(ter_enum) && lu_ranked->first == terrain_VisualWater) ||		// Take hard cus tris when doing vis water
					(IsCustomOverWaterSoft(ter_enum) && lu_ranked->first == terrain_Water))				// Take soft cus tris when doing real water
				{
					CHECK_TRI(f->vertex(0),f->vertex(1),f->vertex(2));

					++total_tris;
					++tris_this_patch;

					map<int, NaturalTerrainInfo_t>::const_iterator terrain = gNaturalTerrainInfo.find(ter_enum);
					const bool is_mobile_ortho = terrain != gNaturalTerrainInfo.end() && terrain->second.custom_ter == tex_custom_pseudo_ortho;

					tex_proj_info * pinfo = (gTexProj.count(lu_ranked->first)) ? &gTexProj[lu_ranked->first] : NULL;

					int flags = 0;
					if(is_overlay)  flags |= dsf_Flag_Overlay;
					if(lu_ranked->first != terrain_VisualWater &&			// Every patch is physical EXCEPT: visual water, obviously just for looks!
						!IsCustomOverWaterSoft(lu_ranked->first))			// custom over soft water - we get physics from who is underneath
						flags |= dsf_Flag_Physical;

					if(prev_ter_enum != ter_enum)
					{
						if(prev_ter_enum >= 0)
						{
							cbs.EndPatch_f(writer1);
						}
						cbs.BeginPatch_f(lu_ranked->second, TERRAIN_NEAR_LOD, TERRAIN_FAR_LOD, flags, (is_water || pinfo || is_mobile_ortho ? 7 : 5), writer1);
						++total_patches;
						prev_ter_enum = ter_enum;
						prev_ter_rotation = rotation;
					}

					cbs.BeginPrimitive_f(dsf_Tri, writer1);
					for(int vert_idx = 2; vert_idx >= 0; --vert_idx)
					{
						CDT::Vertex_handle vert = f->vertex(vert_idx);

						// Ben says: the use of doblim warrants some explanation: CGAL provides EXACT arithmetic, but it does not give exact
						// conversion back to float EVEN when that is possible!!  So the edge of our tile is guaranteed to be exactly on the DSF
						// border but is not guaranteed to be within the DSF border once rounded.
						// Because of this, we have to clamp our output to the double-precision bounds after conversion, since DSFLib is sensitive
						// to out-of-boundary conditions!
						DebugAssert(vert->point().x() >= inElevation.mWest  && vert->point().x() <= inElevation.mEast );
						DebugAssert(vert->point().y() >= inElevation.mSouth && vert->point().y() <= inElevation.mNorth);
						coords8[0] = doblim(CGAL::to_double(vert->point().x()),inElevation.mWest ,inElevation.mEast );
						coords8[1] = doblim(CGAL::to_double(vert->point().y()),inElevation.mSouth,inElevation.mNorth);
						DebugAssert(coords8[0] >= inElevation.mWest  && coords8[0] <= inElevation.mEast );
						DebugAssert(coords8[1] >= inElevation.mSouth && coords8[1] <= inElevation.mNorth);
						coords8[2] =USE_DEM_H( vert->info().height, is_water,inHiresMesh,vert);
						coords8[3] =USE_DEM_N( vert->info().normal[0]);
						coords8[4] =USE_DEM_N(-vert->info().normal[1]);
						if (is_water)
						{
							coords8[5] = GetWaterBlend(vert, inElevation, inBathymetry);
							coords8[6] = CategorizeVertex(inHiresMesh,vert,terrain_Water) >= 0 ? 0.0 : 1.0;
							DebugAssert(coords8[5] >= 0.0);
							DebugAssert(coords8[5] <= 1.0);
						}
						else if (pinfo)	{
							ProjectTex(coords8[0],coords8[1],coords8[5],coords8[6],pinfo);
							DebugAssert(coords8[5] >= 0.0);
							DebugAssert(coords8[5] <= 1.0);
							DebugAssert(coords8[6] >= 0.0);
							DebugAssert(coords8[6] <= 1.0);
						}
						else if(is_mobile_ortho)
						{
							// COPY PASTA warning - see same use of project_ortho() for border tris but with more UV coordinates.
							tex_proj_info ortho_projection = project_ortho(f, inElevation, rotation);
							ProjectTex(coords8[0], coords8[1], coords8[5], coords8[6], &ortho_projection);
							DebugAssert(coords8[5] >= 0.0); // s
							DebugAssert(coords8[5] <= 1.0);
							DebugAssert(coords8[6] >= 0.0); // t
							DebugAssert(coords8[6] <= 1.0);
						}
						DebugAssert(coords8[3] >= -1.0);
						DebugAssert(coords8[3] <=  1.0);
						DebugAssert(coords8[4] >= -1.0);
						DebugAssert(coords8[4] <=  1.0);
						cbs.AddPatchVertex_f(coords8, writer1);
					}
					cbs.EndPrimitive_f(writer1);
				}

				if(tris_this_patch > prev_patch_tris)
				{
					cbs.EndPatch_f(writer1);
				}
			}
		}

		/***************************************************************************************************************************************
		 * WRITE OUT HI RES BORDER PATCHES
		 ***************************************************************************************************************************************/

#if !NO_BORDERS
		for (cur_id = 0; cur_id < (get_patch_dim_hi()*get_patch_dim_hi()); ++cur_id)		// For each triangle in this patch
		if (lu_ranked->first >= terrain_Natural)
		if (sHiResBO[cur_id].count(lu_ranked->first) &&					// Quick check: do we have ANY border tris in this layer in this patch?
			has_some_border(sHiResTris[cur_id], lu_ranked->first))		// This ensures we don't accidentally make an empty patch
		{
			map<int, NaturalTerrainInfo_t>::const_iterator terrain = gNaturalTerrainInfo.find(lu_ranked->first);
			const bool is_mobile_ortho = terrain != gNaturalTerrainInfo.end() && terrain->second.custom_ter == tex_custom_pseudo_ortho;

			cbs.BeginPatch_f(lu_ranked->second, TERRAIN_NEAR_BORDER_LOD, get_terrain_far_border_lod(), dsf_Flag_Overlay, /*is_composite ? 8 :*/ is_mobile_ortho ? 9 : 7, writer1);
			cbs.BeginPrimitive_f(dsf_Tri, writer1);
			tris_this_patch = 0;
			for (tri = 0; tri < sHiResTris[cur_id].size(); ++tri)				// For each tri
			{
				CDT::Face_handle f = sHiResTris[cur_id][tri];
				if (f->info().terrain_border.count(lu_ranked->first))			// If it has this border...
				{
					float	bblend[3];
					int vi;
					for (vi = 0; vi < 3; ++vi)
						bblend[vi] = f->vertex(vi)->info().border_blend[lu_ranked->first];

					// Ben says: normally we would like to draw one DSF overdrawn tri for each border tri.  But there is an exception case:
					// if ALL of our border blends are 100% but our border is NOT a variant (e.g. this is a meaningful border change) then
					// we really need to make 3 border tris that all fade out...this allows the CENTER of our tri to show the base terrain
					// while the borders show the neighboring tris.  (Without this, a single tri of cliff will be COMPLETELY covered by
					// the non-cliff terrain surrouding on 3 sides.)  In this case we make THREE passes and force one vertex to 0% blend for
					// each pass.
					int ts = -1, te = 0;
//					if (!AreVariants(lu_ranked->first, f->info().terrain))
					if (bblend[0] == bblend[1] &&
						bblend[1] == bblend[2] &&
						bblend[0] == 1.0)
					{
						ts = 0; te = 3;
					}

					for (int border_pass = ts; border_pass < te; ++border_pass)
					{

						if (tris_this_patch >= MAX_TRIS_PER_PATCH)
						{
							cbs.EndPrimitive_f(writer1);
							cbs.BeginPrimitive_f(dsf_Tri, writer1);
							tris_this_patch = 0;
						}

						for (vi = 2; vi >= 0 ; --vi)
						{
							DebugAssert(f->vertex(vi)->point().x() >= inElevation.mWest  && f->vertex(vi)->point().x() <= inElevation.mEast );
							DebugAssert(f->vertex(vi)->point().y() >= inElevation.mSouth && f->vertex(vi)->point().y() <= inElevation.mNorth);
							coords8[0] = doblim(CGAL::to_double(f->vertex(vi)->point().x()),inElevation.mWest ,inElevation.mEast );
							coords8[1] = doblim(CGAL::to_double(f->vertex(vi)->point().y()),inElevation.mSouth,inElevation.mNorth);
							DebugAssert(coords8[0] >= inElevation.mWest  && coords8[0] <= inElevation.mEast );
							DebugAssert(coords8[1] >= inElevation.mSouth && coords8[1] <= inElevation.mNorth);

							coords8[2] =USE_DEM_H( f->vertex(vi)->info().height , is_water, inHiresMesh,f->vertex(vi));
							coords8[3] =USE_DEM_N( f->vertex(vi)->info().normal[0]);
							coords8[4] =USE_DEM_N(-f->vertex(vi)->info().normal[1]);
//							coords8[5] = f->vertex(vi)->info().border_blend[lu_ranked->first];

							if(is_mobile_ortho)
							{
								const int rotation = f->info().orig_face->data().mRotationDeg;
								// COPY PASTA WARNING - this is stolen from the base mesh case.
								tex_proj_info ortho_projection = project_ortho(f, inElevation, rotation);
								ProjectTex(coords8[0], coords8[1], coords8[5], coords8[6], &ortho_projection);
								// DSF sucks!  We have to replicate the UV coordinates of the base texture into the mask
								// to get a correct border triangle.  Maybe someday we can optimize this out.
								coords8[7] = coords8[5];
								coords8[8] = coords8[6];
							}
							else
							{
								coords8[5] = vi == border_pass ? 0.0 : bblend[vi];
								coords8[6] = GetTightnessBlend(inHiresMesh, f, f->vertex(vi), lu_ranked->first);
							}
							DebugAssert(coords8[5] >= 0.0);
							DebugAssert(coords8[5] <= 1.0);
							DebugAssert(coords8[6] >= 0.0);
							DebugAssert(coords8[6] <= 1.0);
							DebugAssert(!is_water);
	//						if (is_composite)
	//							coords8[7] = is_water ? GetWaterBlend(f->vertex(vi), waterType) : f->vertex(vi)->info().vege_density;
							DebugAssert(coords8[3] >= -1.0);
							DebugAssert(coords8[3] <=  1.0);
							DebugAssert(coords8[4] >= -1.0);
							DebugAssert(coords8[4] <=  1.0);
							cbs.AddPatchVertex_f(coords8, writer1);
						}
						++total_tris;
						++border_tris;
						++tris_this_patch;
					}
				}
			}
			cbs.EndPrimitive_f(writer1);
			cbs.EndPatch_f(writer1);
			++total_patches;
		}
#endif
	}

	if(writer1)
	for (lu = landuses_reversed.begin(); lu != landuses_reversed.end(); ++lu)
	{
		string def = get_terrain_name(lu->second);
		cbs.AcceptTerrainDef_f(def.c_str(), writer1);
	}

	if (inProgress && inProgress(1, 5, "Sorting Mesh", 1.0)) return;


	if(writer1)
	{
		cbs.AcceptRasterDef_f("elevation",writer1);
		cbs.AcceptRasterDef_f("sea_level",writer1);
		cbs.AcceptRasterDef_f("bathymetry",writer1);

		DSFRasterHeader_t	header;
		short * data = ConvertDEMTo<short>(inElevation,header, dsf_Raster_Format_Int,1.0,0.0);
		must_dealloc.push_back(data);
		cbs.AddRasterData_f(&header,data,writer1);

//		data = ConvertDEMTo<short>(inSeaLevel,header, dsf_Raster_Format_Int,1.0,0.0);
//		must_dealloc.push_back(data);
//		cbs.AddRasterData_f(&header,data,writer1);

		data = ConvertDEMTo<short>(inBathymetry,header, dsf_Raster_Format_Int,1.0,0.0);
		must_dealloc.push_back(data);
		cbs.AddRasterData_f(&header,data,writer1);
	}

	/****************************************************************
	 * BEACH EXPORT
	 ****************************************************************/
	if(!gMobile && writer1)
	{
		// Beach export - we are going to export polygon rings/chains out of
		// every homogenous continous coastline type.  Two issues:
		// When a beach is not a ring, we need to find the start link
		// We also need to identify rings somehow.

		typedef edge_hash_map														LinkMap;
		typedef set<CDT::Edge>													LinkSet;
		typedef edge_info_map														LinkInfo;

		LinkMap			linkNext;	// A hash map from each halfedge to the next with matching beach.  Uses CCW traversal to handle screw cases.
		LinkSet			nonStart;	// Set of all halfedges that are pointed to by another.
		LinkInfo		all;		// Ones we haven't exported.
		LinkSet			starts;		// Ones that are not pointed to by a HE
		CDT::Edge	beach, last_beach;
		int				beachKind;

		// Go through and build up the link map, e.g. for each edge, who's next.
		// Also record each edge that's pointed to by another - these are NOT
		// the starts of non-ring beaches.
		for (fi = inHiresMesh.finite_faces_begin(); fi != inHiresMesh.finite_faces_end(); ++fi)
		for (v = 0; v < 3; ++v)
		{
			CDT::Edge edge;
			edge.first = fi;
			edge.second = v;
			if (has_beach(edge, inHiresMesh, beachKind))
			{
				all[edge] = beachKind;
				starts.insert(edge);
				// Go through each he coming out of our target starting with the one to the clockwise of us, going clockwise.
				// We're searching for the next beach seg but skipping bogus in-water stuff like brides.
				for (CDT::Edge iter = edge_next(edge); iter != edge_twin(edge); iter = edge_twin_next(iter))
				{
					if (has_beach(iter, inHiresMesh, beachKind))
					{
	//					DebugAssert(iter->twin() != he);
						DebugAssert(linkNext.count(edge) == 0);
						linkNext[edge] = iter;
						DebugAssert(nonStart.count(iter) == 0);
						nonStart.insert(iter);
						break;
					}
					// If we hit something that isn't bounding water, we've gone out of our land into the next
					// water out of this vertex.  Stop now before we link to a non-connected water body!!
					if (iter.first->info().terrain != terrain_Water)
						break;
				}
			}
		}

		for (LinkSet::iterator i = nonStart.begin(); i != nonStart.end(); ++i)
		{
			starts.erase(*i);
		}

		// Export non-ring beaches.  For each link that's not pointed to by someone else
		// export the chain.

		for (LinkSet::iterator a_start = starts.begin(); a_start != starts.end(); ++a_start)
		{
			FixBeachContinuity(linkNext, *a_start, all);

			cbs.BeginPolygon_f(0, 0, 3, writer1);
			cbs.BeginPolygonWinding_f(writer1);

			for (beach = *a_start; beach != CDT::Edge(); beach = ((linkNext.count(beach)) ? (linkNext[beach]) : CDT::Edge()))
			{
	//			printf("output non-circ beach type = %d, len = %lf\n", all[beach], edge_len(beach));
				last_beach = beach;
				DebugAssert(all.count(beach) != 0);
				beachKind = all[beach];
				BeachPtGrab(beach, false, inHiresMesh, coords3, beachKind);
				coords3[0] = doblim(coords3[0],inElevation.mWest,  inElevation.mEast);
				coords3[1] = doblim(coords3[1],inElevation.mSouth, inElevation.mNorth);
				cbs.AddPolygonPoint_f(coords3, writer1);
				all.erase(beach);
			}
			DebugAssert(all.count(*a_start) == 0);

			BeachPtGrab(last_beach, true, inHiresMesh, coords3, beachKind);
			coords3[0] = doblim(coords3[0],inElevation.mWest,  inElevation.mEast);
			coords3[1] = doblim(coords3[1],inElevation.mSouth, inElevation.mNorth);
			cbs.AddPolygonPoint_f(coords3, writer1);

			cbs.EndPolygonWinding_f(writer1);
			cbs.EndPolygon_f(writer1);
			//printf("end non-circular.\n");
		}

	#if DEV
		for (LinkInfo::iterator test = all.begin(); test != all.end(); ++test)
		{
			DebugAssert(linkNext.count(test->first) != 0);
		}
	#endif

		// Now just pick an edge and export in a circulator - we should only have rings!
		while (!all.empty())
		{
			CDT::Edge this_start = all.begin()->first;
			FixBeachContinuity(linkNext, this_start, all);
			cbs.BeginPolygon_f(0, 1, 3, writer1);
			cbs.BeginPolygonWinding_f(writer1);

			beach = this_start;
			do {
	//			printf("output circ beach type = %d, len = %lf\n", all[beach], edge_len(beach));
				DebugAssert(all.count(beach) != 0);
				DebugAssert(linkNext.count(beach) != 0);
				beachKind = all.begin()->second;
				BeachPtGrab(beach, false, inHiresMesh, coords3, beachKind);
				coords3[0] = doblim(coords3[0],inElevation.mWest,  inElevation.mEast);
				coords3[1] = doblim(coords3[1],inElevation.mSouth, inElevation.mNorth);
				cbs.AddPolygonPoint_f(coords3, writer1);
				all.erase(beach);
				beach = linkNext[beach];
			} while (beach != this_start);
			cbs.EndPolygonWinding_f(writer1);
			cbs.EndPolygon_f(writer1);

		}
		cbs.AcceptPolygonDef_f("lib/g8/beaches.bch", writer1);
	}

	/****************************************************************
	 * OBJECT EXPORT/FACADE/FOREST WRITEOUT
	 ****************************************************************/

	if (inProgress && inProgress(2, 5, "Compiling Objects", 0.0)) return;

	// First go through and accumulate our object and facade types.
	// We need this in advance so we can figure out the DSF-relative
	// IDs.

	for (pf = inVectorMap.faces_begin(); pf != inVectorMap.faces_end(); ++pf)
	if (!pf->is_unbounded())
	{
		for (pointObj = pf->data().mObjs.begin(); pointObj != pf->data().mObjs.end(); ++pointObj)
			objects.insert(map<int, int, ObjPrio>::value_type(pointObj->mRepType, 0));
		for (polyObj = pf->data().mPolyObjs.begin(); polyObj != pf->data().mPolyObjs.end(); ++polyObj)
			facades.insert(map<int, int>::value_type(polyObj->mRepType, 0));
	}

	int lowest_required = objects.size();

	// Farm out object IDs.
	cur_id = 0;
	for (obdef_prio = objects.begin(); obdef_prio != objects.end(); ++obdef_prio, ++cur_id)
	{
		obdef_prio->second = cur_id;
		objects_reversed[cur_id] = obdef_prio->first;
		if (IsFeatureObject(obdef_prio->first))
			lowest_required = min(lowest_required, cur_id);
	}

	if(writer2)
	if (lowest_required != objects.size())
	{
		char buf[256];
		sprintf(buf,"1/%d", lowest_required);
		cbs.AcceptProperty_f("sim/require_object", buf, writer2);
	}

	cur_id = (writer2 == writer1 ? 1 : 0);
	for (obdef = facades.begin(); obdef != facades.end(); ++obdef, ++cur_id)
	{
		obdef->second = cur_id;
		facades_reversed[cur_id] = obdef->first;
	}

	// Now go through and emit the objects.  Note: there is no point to
	// sorting them - the DSF lib is good about cleaning up the object
	// data you give it.

	if(writer2)
	for (pf = inVectorMap.faces_begin(); pf != inVectorMap.faces_end(); ++pf)
	if (!pf->is_unbounded())
	{
		for (pointObj = pf->data().mObjs.begin(); pointObj != pf->data().mObjs.end(); ++pointObj)
		{
			coords2[0] = CGAL::to_double(pointObj->mLocation.x());
			coords2[1] = CGAL::to_double(pointObj->mLocation.y());
			cbs.AddObject_f(
				objects[pointObj->mRepType],
				coords2,
				(pointObj->mHeading < 0.0) ? (pointObj->mHeading + 360.0) : pointObj->mHeading,
				writer2);
			++total_objs;
		}

		for (polyObj = pf->data().mPolyObjs.begin(); polyObj != pf->data().mPolyObjs.end(); ++polyObj)
		{
			bool	 broken = false;
			for (polyPt = polyObj->mShape.front().begin(); polyPt != polyObj->mShape.front().end(); ++polyPt)
			{
				if (polyPt->x() < inElevation.mWest ||
					polyPt->x() > inElevation.mEast ||
					polyPt->y() < inElevation.mSouth ||
					polyPt->y() > inElevation.mNorth)
				{
					printf("Pt %lf %lf is out of DEM.\n", CGAL::to_double(polyPt->x()), CGAL::to_double(polyPt->y()));
					broken = true;
				}
			}

			if (broken)
				continue;

//			if(polyObj->mShape.size() > 254)
//				continue;

			cbs.BeginPolygon_f(
						facades[polyObj->mRepType],
						polyObj->mParam, 2,
						writer2);
			// boundary

			for (polyHole = polyObj->mShape.begin(); polyHole != polyObj->mShape.end(); ++ polyHole)
			{
				cbs.BeginPolygonWinding_f(writer2);
				for (polyPt = polyHole->begin(); polyPt != polyHole->end(); ++polyPt)
				{
					coords2[0] = polyPt->x();
					coords2[1] = polyPt->y();
					cbs.AddPolygonPoint_f(coords2, writer2);
				}
				cbs.EndPolygonWinding_f(writer2);
			}
			cbs.EndPolygon_f(writer2);
			++total_polys;
		}
	}

	// Write out definition names too.
	if(writer2)
	for (obdef = objects_reversed.begin(); obdef != objects_reversed.end(); ++obdef)
	{
		Assert(obdef->second != NO_VALUE);
		Assert(obdef->second != DEM_NO_DATA);
		string objName = gObjLibPrefix + FetchTokenString(obdef->second);
		objName += ".obj";
		cbs.AcceptObjectDef_f(objName.c_str(), writer2);
	}

	if(writer2)
	for (obdef = facades_reversed.begin(); obdef != facades_reversed.end(); ++obdef)
	{
		Assert(obdef->second != NO_VALUE);
		Assert(obdef->second != DEM_NO_DATA);
		string facName = FetchTokenString(obdef->second);

		if(facName.find('.') == facName.npos)
		{
			if (IsForestType(obdef->second))
			{
				facName = "lib/g8/"+facName+".for";
			} else
				facName = gObjLibPrefix + facName + ".fac";
		}
		cbs.AcceptPolygonDef_f(facName.c_str(), writer2);
	}

	if (inProgress && inProgress(2, 5, "Compiling Objects", 1.0)) return;

	/****************************************************************
	 * VECTOR EXPORT
	 ****************************************************************/

	static int vec_export_hint_id = CDT::gen_cache_key();

	if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.0)) return;

	if(writer2)
	if (gDSFBuildPrefs.export_roads)
	{

		if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.3)) return;

		{
			TIMER(BuildNetworkTopology)
			BuildNetworkTopology(inVectorMap, inHiresMesh, junctions, chains);
		}

		{
			TIMER(RemoveSmall)
			MergeNearJunctions(junctions, chains, 0.00002);
		}


//		{
//			TIMER(DrapeRoads)
//			if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.6)) return;
//			DrapeRoads(junctions, chains, inHiresMesh, false);
//			DrapeRoads(junctions, chains, inHiresMesh, true);
//		}

//		{
//			TIMER(PromoteShapePoints)
//			PromoteShapePoints(junctions, chains);
//		}

//		{
//			TIMER(VerticalPartitionRoads)
//			VerticalPartitionRoads(junctions, chains);
//			VerticalPartitionOnlyPower(junctions, chains);
//		}

//		{
//			TIMER(VerticalBuildBridges)
//			VerticalBuildBridges(junctions, chains);
//		}

//		{
//			TIMER(InterpolateRoadHeights)
//			InterpolateRoadHeights(junctions, chains);
//		}
		{
			TIMER(AssignExportTypes)
			AssignExportTypes(junctions, chains);
		}
		{
			TIMER(DeleteBlankChains)
			DeleteBlankChains(junctions, chains);
		}

		{
			TIMER(OptimizeNetwork)
			OptimizeNetwork(junctions, chains, false);
		}

//		{
//			TIMER(SpacePowerlines)
//			SpacePowerlines(junctions, chains, 1000.0, 10.0);
//		}
		if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.7)) return;

		{
			int orig_shape_count = 0;
			int reduced_shape_count = 0;
			TIMER(OptimizePush)

			cur_id = 1;
			for (ji = junctions.begin(); ji != junctions.end(); ++ji)
				(*ji)->index = cur_id++;

			for (ci = chains.begin(); ci != chains.end(); ++ci)
			{
				coords4[0] = (*ci)->start_junction->location.x();
				coords4[1] = (*ci)->start_junction->location.y();
				coords4[2] = (*ci)->start_junction->GetLayerForChain(*ci);
				coords4[3] = (*ci)->start_junction->index;

				if (coords4[0] < inElevation.mWest  || coords4[0] > inElevation.mEast || coords4[1] < inElevation.mSouth || coords4[1] > inElevation.mNorth)
					printf("WARNING: coordinate out of range.\n");

				DebugAssert(junctions.count((*ci)->start_junction));
				DebugAssert(((*ci)->start_junction->index) != 0xDEADBEEF);
				DebugAssert(junctions.count((*ci)->end_junction));
				DebugAssert(((*ci)->end_junction->index) != 0xDEADBEEF);

				road_coords_checker	checker((*ci), coords3, 'B');
				//printf("Bgn: %lf, %lf, %lf (%d)\n", coords3[0],coords3[1],coords3[2], (*ci)->start_junction->index);
				cbs.BeginSegment_f(
								0,
								(*ci)->export_type,
								coords4,
								false,
								writer2);
				++total_chains;
				//debug_mesh_point(Point2(coords3[0],coords3[1]),1,0,0);


				NetRepInfo * info = &gNetReps[(*ci)->rep_type];

				list<Point2c>	pts;

				pts.push_back(Point2c((*ci)->start_junction->location,false));

				for(int n = 0; n < (*ci)->shape.size(); ++n)
				{
					if((*ci)->shape.size() == 1)
					{
						generate_bezier((*ci)->start_junction->location,
										(*ci)->shape[0  ],
										(*ci)->end_junction->location,
										info->min_defl_deg_mtr, info->crease_angle_cos,
										pts);
					}
					else if(n == 0)
					{
						generate_bezier((*ci)->start_junction->location,
										(*ci)->shape[n  ],
										(*ci)->shape[n+1],
										info->min_defl_deg_mtr, info->crease_angle_cos,
										pts);
					}
					else if (n == (*ci)->shape.size()-1)
					{
						generate_bezier((*ci)->shape[n-1],
										(*ci)->shape[n  ],
										(*ci)->end_junction->location,
										info->min_defl_deg_mtr, info->crease_angle_cos,
										pts);
					}
					else
					{
						generate_bezier((*ci)->shape[n-1],
										(*ci)->shape[n  ],
										(*ci)->shape[n+1],
										info->min_defl_deg_mtr, info->crease_angle_cos,
										pts);
					}
				}

				pts.push_back(Point2c((*ci)->end_junction->location,false));
				DebugAssert(pts.size() >= 2);

				list<Point2c>::iterator last(pts.end()), start(pts.begin());
				--last;
				DebugAssert(last->c == 0);
				#if CAN_OPTIMIZE_BEZIERS
				if(gNetReps[(*ci)->rep_type].max_err > 0.0)
				#if ONLY_OPTIMIZE_RAMPS
				if(gNetReps[(*ci)->rep_type].use_mode == use_Ramp && pts.size() > 20)
				#endif
				{
					#if SHOW_BEZIERS
						visualize_bezier(pts,true,0.1f, 0.0f);
					#endif
					orig_shape_count += pts.size();
					bezier_multi_simplify_straight_ok(pts, MTR_TO_DEG_LAT * gNetReps[(*ci)->rep_type].max_err, 0.00005);// * MTR_TO_DEG_LAT * 5.0 * 5.0);
					reduced_shape_count += pts.size();
					#if SHOW_BEZIERS
						visualize_bezier(pts,false,1.0f,1.0f);
					#endif
				}
				#endif
				DebugAssert(pts.size() >= 2);
				pts.pop_back();
				pts.pop_front();
				for(list<Point2c>::iterator p = pts.begin(); p != pts.end(); ++p)
				{
					coords3[0] = doblim(p->x(),inElevation.mWest,inElevation.mEast);
					coords3[1] = doblim(p->y(),inElevation.mSouth,inElevation.mNorth);
					coords3[2] = p->c ? 1 : 0;

					if (coords3[0] < inElevation.mWest  || coords3[0] > inElevation.mEast || coords3[1] < inElevation.mSouth || coords3[1] > inElevation.mNorth)
					{
						printf("WARNING: coordinate out of range.\n");
	//					debug_mesh_point(Point2(coords3[0],coords3[1]),1,0,coords3[2]);
					}
					checker.check(coords3,'S');
					//printf("Shp: %lf, %lf, %lf\n", coords3[0],coords3[1],coords3[2]);
					cbs.AddSegmentShapePoint_f(coords3, false, writer2);
					++total_shapes;
					//debug_mesh_point(Point2(coords3[0],coords3[1]),1,1,coords3[2]);
				}

				coords4[0] = (*ci)->end_junction->location.x();
				coords4[1] = (*ci)->end_junction->location.y();
				coords4[2] = (*ci)->end_junction->GetLayerForChain(*ci);
				coords4[3] = (*ci)->end_junction->index;

				if (coords4[0] < inElevation.mWest  || coords4[0] > inElevation.mEast || coords4[1] < inElevation.mSouth || coords4[1] > inElevation.mNorth)
					printf("WARNING: coordinate out of range.\n");

				checker.check(coords4,'E');

				cbs.EndSegment_f(
						coords4,
						false,
						writer2);
				//debug_mesh_point(Point2(coords3[0],coords3[1]),0,1,0);
			}
			if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.9)) return;

			CleanupNetworkTopology(junctions, chains);
			if (inProgress && inProgress(3, 5, "Compiling Vectors", 1.0)) return;
			if(gRegion == rf_eu)
				cbs.AcceptNetworkDef_f("lib/g10/roads_EU.net", writer2);
			else
				cbs.AcceptNetworkDef_f("lib/g10/roads.net", writer2);			

			printf("Shape points: %d to %d.\n", orig_shape_count, reduced_shape_count);
		}
	}

	/****************************************************************
	 * MANIFEST
	 ****************************************************************/

	if(writer1)
	{
		sprintf(prop_buf, "%d", (int) inElevation.mWest);			cbs.AcceptProperty_f("sim/west", prop_buf, writer1);
		sprintf(prop_buf, "%d", (int) inElevation.mEast);			cbs.AcceptProperty_f("sim/east", prop_buf, writer1);
		sprintf(prop_buf, "%d", (int) inElevation.mNorth);		cbs.AcceptProperty_f("sim/north", prop_buf, writer1);
		sprintf(prop_buf, "%d", (int) inElevation.mSouth);		cbs.AcceptProperty_f("sim/south", prop_buf, writer1);
		cbs.AcceptProperty_f("sim/planet", "earth", writer1);
		cbs.AcceptProperty_f("sim/creation_agent", "X-Plane Scenery Creator 0.9a", writer1);
		cbs.AcceptProperty_f("laminar/internal_revision", "1", writer1);
	}

	if (writer2 && writer2 != writer1)
	{
		sprintf(prop_buf, "%d", (int) inElevation.mWest);			cbs.AcceptProperty_f("sim/west", prop_buf, writer2);
		sprintf(prop_buf, "%d", (int) inElevation.mEast);			cbs.AcceptProperty_f("sim/east", prop_buf, writer2);
		sprintf(prop_buf, "%d", (int) inElevation.mNorth);		cbs.AcceptProperty_f("sim/north", prop_buf, writer2);
		sprintf(prop_buf, "%d", (int) inElevation.mSouth);		cbs.AcceptProperty_f("sim/south", prop_buf, writer2);
		cbs.AcceptProperty_f("sim/planet", "earth", writer2);
		cbs.AcceptProperty_f("sim/creation_agent", "X-Plane Scenery Creator 0.9a", writer2);
		cbs.AcceptProperty_f("laminar/internal_revision", "1", writer2);
		cbs.AcceptProperty_f("sim/overlay", "1", writer2);
	}

	/****************************************************************
	 * WRITEOUT
	 ****************************************************************/
	if (inProgress && inProgress(4, 5, "Writing DSF file", 0.0)) return;
	if (writer1) DSFWriteToFile(inFileName1, writer1);
	if (inProgress && inProgress(4, 5, "Writing DSF file", 0.5)) return;
																																																																																												if (writer2 && writer2 != writer1) DSFWriteToFile(inFileName2, writer2);
	if (inProgress && inProgress(4, 5, "Writing DSF file", 1.0)) return;

//	printf("Patches: %d, Free Tris: %d, Tri Fans: %d, Tris in Fans: %d, Border Tris: %d, Avg Per Patch: %f, avg per fan: %f\n",
//		total_patches, total_tris, total_tri_fans, total_tri_fan_pts, border_tris,
//		(float) (total_tri_fan_pts + total_tris) / (float) total_patches,
//		(total_tri_fans == 0) ? 0.0 : ((float) (total_tri_fan_pts) / (float) total_tri_fans));
	printf("Objects: %d, Polys: %d\n", total_objs, total_polys);
	printf("LU: %llu, Objdef: %llu, PolyDef: %llu\n", (unsigned long long)landuses.size(), (unsigned long long)objects.size(), (unsigned long long)facades.size());
	printf("Chains: %d, Shapes: %d\n", total_chains, total_shapes);
//	printf("Submitted to tri fan builder: %d.  Removed from builder: %d.\n", debug_add_tri_fan, debug_sub_tri_fan);

}
