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
#include "MapDefs.h"
#include "MeshAlgs.h"
//#include "WED_Globals.h"
#include "ParamDefs.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "CompGeomUtils.h"
#include "PolyRasterUtils.h"
#include "AssertUtils.h"
#include "PerfUtils.h"
#include "MapAlgs.h"
#include "DEMTables.h"
#include "GISUtils.h"

/* Used to disable triangulation for fast preview of points. */
#define NO_TRIANGULATE 0

/* This defines the ratio of skip to ok points for vector rivers.  It's
   a real pain to do a perfect computation of vital river points, and 
   frankly it doesn't matter...every river point is DEM-wise useful and
   for the most part the data is organized to keep adjacent rivers nearby
   in the map.  So skipping a few produces surprisingly pleasing results 
   for like, no work. */
#define RIVER_SKIP 6
#define LOW_RES_WATER_INTERVAL 40

#define PROFILE_PERFORMANCE 1

#define FAR_TEX_DIVISIONS 2

// About 80 degrees
#define ROCK_SLOPE_CUTOFF	0.17364	

#if PROFILE_PERFORMANCE
#define TIMER(x)	StElapsedTime	__PerfTimer##x(#x);
#else
#define TIMER(x)	
#endif

MeshPrefs_t gMeshPrefs = { 
				25000, 
				100.0,
				10.00,
				0,
				1,
				2000.0
};

int	kFarTextures[FAR_TEX_DIVISIONS*FAR_TEX_DIVISIONS] = {
	terrain_VirtualOrtho00,		terrain_VirtualOrtho10,
	terrain_VirtualOrtho01,		terrain_VirtualOrtho11
};

/* Conststraint markers - here's the deal: the way we set the water body
 * triangles to water (precisely) is we remember the pairs of vertices that
 * make up their constrained edges.  These vertices are directed and form
 * a CCB, so we know that the left side of this pair is a triangle that is
 * wet.  This lets us seed the water-finding process. */
 
typedef pair<CDT::Vertex_handle, CDT::Vertex_handle>	ConstraintMarker_t;
typedef pair<int, int>									LandusePair_t;			// "Left" and "right" side
typedef pair<ConstraintMarker_t, LandusePair_t>			LanduseConstraint_t;

static	void	SlightClampToDEM(Point2& ioPoint, const DEMGeo& ioDEM);

inline bool	PersistentFindEdge(CDT& ioMesh, CDT::Vertex_handle a, CDT::Vertex_handle b, CDT::Face_handle& h, int& vnum)
{
	if (ioMesh.is_edge(a, b, h, vnum))
	{
		DebugAssert(ioMesh.is_constrained(CDT::Edge(h, vnum)));
		return true;
	}
	
	Vector2	along(Point2(a->point().x(), a->point().y()),Point2(b->point().x(), b->point().y()));
	
	CDT::Vertex_handle v = a;
	do {
		CDT::Vertex_circulator	circ, stop;
		CDT::Vertex_handle best = CDT::Vertex_handle();
		circ = stop = ioMesh.incident_vertices(v);
		do {
			Vector2	step(Point2(v->point().x(), v->point().y()),Point2(circ->point().x(), circ->point().y()));
			if (along.dot(step) > 0.0 && (along.dx * step.dy == along.dy * step.dx))
			{
				best = circ;
				break;
			}
			++circ;
		} while (circ != stop);
		
		if (best == CDT::Vertex_handle()) return false;
		if (!ioMesh.is_edge(v, best, h, vnum)) return false;		
		DebugAssert(ioMesh.is_constrained(CDT::Edge(h, vnum)));
		
		v = best;
	} while (v != b);
	
	return true;
}

/************************************************************************************************************************
 * BORDER MATCHING
 ************************************************************************************************************************
 *
 * These data structures hold a single border...points and the edges between them, going either west to east along the top
 * or north to south along the right side.
 * 
 */

struct	mesh_match_vertex_t {
	Point2					loc;
	double					height;
	hash_map<int, float>	blending;
	CDT::Vertex_handle		buddy;
};

struct	mesh_match_edge_t {
	int						base;
	set<int>				borders;
	CDT::Face_handle		buddy;
};

struct	mesh_match_t {
	vector<mesh_match_vertex_t>	vertices;
	vector<mesh_match_edge_t>	edges;
};

inline bool MATCH(const char * big, const char * msmall)
{
	return strncmp(big, msmall, strlen(msmall)) == 0;
}

static mesh_match_t gMatchBottom, gMatchLeft;

inline bool is_border(const CDT& inMesh, CDT::Face_handle f)
{
	for (int n = 0; n < 3; ++n)
	{
		if (f->neighbor(n)->has_vertex(inMesh.infinite_vertex()))
			return true;
	}
	return false;
}

inline void FindNextEast(CDT& ioMesh, CDT::Face_handle& ioFace, int& index)
{
	CDT::Vertex_handle sv = ioFace->vertex(index);
	CDT::Point p = sv->point();
	CDT::Vertex_circulator stop, now;
	stop = now = ioMesh.incident_vertices(sv);
		
//	printf("Starting with: %lf, %lf\n", CGAL::to_double(sv->point().x()), CGAL::to_double(sv->point().y()));
	
	CDT::Geom_traits::Compare_y_2 cy;
	CDT::Geom_traits::Compare_x_2 cx;
	do {
//		printf("Checking: %lf, %lf\n", CGAL::to_double(now->point().x()), CGAL::to_double(now->point().y()));
		if (now != ioMesh.infinite_vertex())
		if (cy(now->point(), p) == CGAL::EQUAL)
		if (cx(now->point(), p) == CGAL::LARGER)
		{
			CDT::Face_handle	a_face;
			CDT::Vertex_circulator next = now;
			--next;
			Assert(ioMesh.is_face(sv, now, next, a_face));
			ioFace = a_face;
			index = ioFace->index(now);
			return;
		}
		++now;
	} while (stop != now);
	AssertPrintf("Next mesh point not found.");
}

inline void FindNextSouth(CDT& ioMesh, CDT::Face_handle& ioFace, int& index)
{
	CDT::Vertex_handle sv = ioFace->vertex(index);
	CDT::Point p = sv->point();
	CDT::Vertex_circulator stop, now;
	stop = now = ioMesh.incident_vertices(sv);
		
//	printf("Starting with: %lf, %lf\n", CGAL::to_double(sv->point().x()), CGAL::to_double(sv->point().y()));
	
	CDT::Geom_traits::Compare_y_2 cy;
	CDT::Geom_traits::Compare_x_2 cx;
	do {
//		printf("Checking: %lf, %lf\n", CGAL::to_double(now->point().x()), CGAL::to_double(now->point().y()));
		if (now != ioMesh.infinite_vertex())
		if (cx(now->point(), p) == CGAL::EQUAL)
		if (cy(now->point(), p) == CGAL::SMALLER)
		{
			CDT::Face_handle	a_face;
			CDT::Vertex_circulator next = now;
			--next;
			Assert(ioMesh.is_face(sv, now, next, a_face));
			ioFace = a_face;
			index = ioFace->index(now);
			return;
		}
		++now;
	} while (stop != now);
	Assert(!"Next pt not found.");
}

static void border_find_edge_tris(CDT& ioMesh, mesh_match_t& ioBorder)
{
	DebugAssert(ioBorder.vertices.size() == (ioBorder.edges.size()+1));
	for (int n = 0; n < ioBorder.edges.size(); ++n)
	{
#if DEV
		CDT::Point	p1 = ioBorder.vertices[n  ].buddy->point();
		CDT::Point	p2 = ioBorder.vertices[n+1].buddy->point();
#endif		
		Assert(ioMesh.is_face(ioBorder.vertices[n].buddy, ioBorder.vertices[n+1].buddy, ioMesh.infinite_vertex(), ioBorder.edges[n].buddy));
		int idx = ioBorder.edges[n].buddy->index(ioMesh.infinite_vertex());
		ioBorder.edges[n].buddy = ioBorder.edges[n].buddy->neighbor(idx);
	}
}

inline void AddZeroMixIfNeeded(CDT::Face_handle f, int layer)
{
	for (int i = 0; i < 3; ++i)
	{
		CDT::Vertex_handle vv = f->vertex(i);
		if (vv->info().border_blend.count(layer) == 0)
			vv->info().border_blend[layer] = 0.0;
	}
}

inline void ZapBorders(CDT::Vertex_handle v)
{
	for (hash_map<int, float>::iterator i = v->info().border_blend.begin(); i != v->info().border_blend.end(); ++i)
		i->second = 0.0;
}

static bool	load_match_file(const char * path, mesh_match_t& outTop, mesh_match_t& outRight)
{
	outTop.vertices.clear();
	outTop.edges.clear();
	outRight.vertices.clear();
	outRight.edges.clear();
	
	FILE * fi = fopen(path, "r");
	if (fi == NULL) return false;
	char buf[80];
	bool go = true;
	int count;
	float mix;
	char ter[80];
	
	while (go)
	{
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		if (MATCH(buf, "VT"))
		{
			outTop.vertices.push_back(mesh_match_vertex_t());
			sscanf(buf, "VT %lf, %lf, %lf", &outTop.vertices.back().loc.x, &outTop.vertices.back().loc.y, &outTop.vertices.back().height);			
			outTop.vertices.back().buddy = NULL;
		}
		if (MATCH(buf, "VC"))
		{
			go = false;
			outTop.vertices.push_back(mesh_match_vertex_t());
			sscanf(buf, "VC %lf, %lf, %lf", &outTop.vertices.back().loc.x, &outTop.vertices.back().loc.y, &outTop.vertices.back().height);			
			outTop.vertices.back().buddy = NULL;			
		}
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		sscanf(buf, "VBC %d", &count);
		while (count--)
		{
			if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
			sscanf(buf, "VB %f %s", &mix, ter);
			outTop.vertices.back().blending[LookupToken(ter)] = mix;
		}
		if (go)
		{
			if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
			sscanf(buf, "TERRAIN %s", ter);
			outTop.edges.push_back(mesh_match_edge_t());
			outTop.edges.back().base = LookupToken(ter);
			if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
			sscanf(buf, "BORDER_C %d", &count);
			while (count--)
			{
				if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
				sscanf(buf, "BORDER_T %s", ter);
				outTop.edges.back().borders.insert( LookupToken(ter));				
			}			
		}
	}
	
	outRight.vertices.push_back(outTop.vertices.back());
	go = true;
	while (go)
	{
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		if (MATCH(buf, "END"))
		{
			fclose(fi);
			return true;
		}
		sscanf(buf, "TERRAIN %s", ter);
		outRight.edges.push_back(mesh_match_edge_t());
		outRight.edges.back().base = LookupToken(ter);
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		sscanf(buf, "BORDER_C %d", &count);
		while (count--)
		{
			if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
			sscanf(buf, "BORDER_T %s", ter);
			outRight.edges.back().borders.insert( LookupToken(ter));				
		}			
	
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		if (MATCH(buf, "VR"))
		{
			outRight.vertices.push_back(mesh_match_vertex_t());
			sscanf(buf, "VR %lf, %lf, %lf", &outRight.vertices.back().loc.x, &outRight.vertices.back().loc.y, &outRight.vertices.back().height);			
			outRight.vertices.back().buddy = NULL;			
		}
		if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
		sscanf(buf, "VBC %d", &count);
		while (count--)
		{
			if (fgets(buf, sizeof(buf), fi) == NULL) goto bail;
			sscanf(buf, "VB %f %s", &mix, ter);
			outRight.vertices.back().blending[LookupToken(ter)] = mix;
		}
	}
	
	
bail:	
	outTop.vertices.clear();
	outTop.edges.clear();
	outRight.vertices.clear();
	outRight.edges.clear();
	fclose(fi);
	return false;	
}

// Given a point on the left edge of the top border or top edge of the right border, this fetches all border
// points in order of distance from that origin.
void	fetch_border(CDT& ioMesh, const Point2& origin, map<double, CDT::Vertex_handle>& outPts, bool isRight)
{
	CDT::Vertex_handle sv = ioMesh.infinite_vertex();
	CDT::Vertex_circulator stop, now;
	stop = now = ioMesh.incident_vertices(sv);
		
	CDT::Point	pt(origin.x, origin.y);
	
	outPts.clear();	
		
	CDT::Geom_traits::Compare_y_2 cy;
	CDT::Geom_traits::Compare_x_2 cx;
	do {
		double dist;
		if (isRight && cx(now->point(), pt) == CGAL::EQUAL)
		{
			dist = CGAL::to_double(now->point().y()) - origin.y;
			DebugAssert(outPts.count(dist)==0);
			outPts[dist] = now;
		}
		if (!isRight && cy(now->point(), pt) == CGAL::EQUAL)
		{
			dist = CGAL::to_double(now->point().x()) - origin.x;
			DebugAssert(outPts.count(dist)==0);
			outPts[dist] = now;
		}
		
		++now;
	} while (stop != now);
}

void	match_border(CDT& ioMesh, mesh_match_t& ioBorder, bool isRight)
{
	map<double, CDT::Vertex_handle>	slaves;
	Point2	origin = ioBorder.vertices.front().loc;
	fetch_border(ioMesh, origin, slaves, isRight);
	
	while (!slaves.empty())	
	{
		multimap<double, pair<double, mesh_match_vertex_t *> >	nearest;
		for (vector<mesh_match_vertex_t>::iterator pts = ioBorder.vertices.begin(); pts != ioBorder.vertices.end(); ++pts)
		if (pts->buddy == NULL)
		{
			for (map<double, CDT::Vertex_handle>::iterator sl = slaves.begin(); sl != slaves.end(); ++sl)
			{
				double myDist = isRight ? (pts->loc.y - CGAL::to_double(sl->second->point().y())) : (pts->loc.x - CGAL::to_double(sl->second->point().x()));
				if (myDist < 0.0) myDist = -myDist;
				nearest.insert(multimap<double, pair<double, mesh_match_vertex_t *> >::value_type(myDist, pair<double, mesh_match_vertex_t *>(sl->first, &*pts)));
			}
		}
		
		Assert(!nearest.empty());
		pair<double, mesh_match_vertex_t *> best_match = nearest.begin()->second;
		DebugAssert(slaves.count(best_match.first) > 0);
		best_match.second->buddy = slaves[best_match.first];
		slaves.erase(best_match.first);
	}
	
	CDT::Face_handle	nearf = NULL;
	for (vector<mesh_match_vertex_t>::iterator pts = ioBorder.vertices.begin(); pts != ioBorder.vertices.end(); ++pts)
	if (pts->buddy == NULL)
	{
		pts->buddy = ioMesh.insert(CDT::Point(pts->loc.x, pts->loc.y), nearf);
		nearf = pts->buddy->face();
		pts->buddy->info().height = pts->height;
	}
}

static void RebaseTriangle(CDT& ioMesh, CDT::Face_handle tri, int new_base, CDT::Vertex_handle v1, CDT::Vertex_handle v2)
{
	int old_base = tri->info().terrain_specific;
	DebugAssert(old_base != terrain_Water);
	tri->info().terrain_specific = new_base;	
	if (new_base != terrain_Water)
	{
		tri->info().terrain_border.insert(old_base);

		for (int i = 0; i < 3; ++i)
		{
			CDT::Vertex_handle v = tri->vertex(i);
			if (v == v1 || v == v2)
				v->info().border_blend[old_base] = 0.0;
			else
				v->info().border_blend[old_base] = 1.0;
		}
		
		if (v1 != CDT::Vertex_handle() && v2 != CDT::Vertex_handle())
		{
			CDT::Face_circulator stop, iter;
			
			CDT::Face_handle	foo;
			int i;
			Assert(ioMesh.is_edge(v2, v1, foo, i));
			DebugAssert(foo == tri);
			CDT::Vertex_handle v3 = tri->vertex(i);
			
			stop = iter = ioMesh.incident_faces(v3);
			do {
				if (!ioMesh.is_infinite(iter))
				if (!is_border(ioMesh, iter))
				if (LowerPriorityNaturalTerrain(new_base, iter->info().terrain_specific))
				{
					DebugAssert(iter != tri);
					RebaseTriangle(ioMesh, iter, new_base, 
						(iter->has_vertex(v1)) ? v1 : CDT::Vertex_handle(),
						(iter->has_vertex(v2)) ? v2 : CDT::Vertex_handle());				
				}
				++iter;
			} while (stop != iter);
		}	
	}
}

inline int MAJORITY_RULES(int a, int b, int c, int d)
{
	int la = 1, lb = 1, lc = 1, ld = 1;
	if (a == b) ++la, ++lb;
	if (a == c) ++la, ++lc;
	if (a == d) ++la, ++ld;
	if (b == c) ++lb, ++lc;
	if (b == d) ++lb, ++ld;
	if (c == d) ++lc, ++ld;
	
	if (la >= lb && la >= lc && la >= ld) return a;
	if (lb >= la && lb >= lc && lb >= ld) return b;
	if (lc >= la && lc >= lb && lc >= ld) return c;
	if (ld >= la && ld >= lb && ld >= lc) return d;
	return a;
}

inline float SAFE_AVERAGE(float a, float b, float c)
{
	int i = 0;
	float t = 0.0;
	if (a != NO_DATA) t += a, ++i;
	if (b != NO_DATA) t += b, ++i;
	if (c != NO_DATA) t += c, ++i;
	if (i == 0) return NO_DATA;
	return t / i;
}

inline float SAFE_MAX(float a, float b, float c)
{
	return max(a, max(b, c));
}

inline double GetXonDist(int layer1, int layer2, double y_normal)
{
	double dist_1 = gNaturalTerrainTable[gNaturalTerrainIndex[layer1]].xon_dist;
	double dist_2 = gNaturalTerrainTable[gNaturalTerrainIndex[layer2]].xon_dist;
	return min(dist_1, dist_2); // * y_normal
}



inline double	DistPtToTri(CDT::Vertex_handle v, CDT::Face_handle f)
{
	// Find the closest a triangle comes to a point.  Inputs are in lat/lon, otuput is in meters!
	Point2	vp(v->point().x(), v->point().y());
	Point2	tp1(f->vertex(0)->point().x(), f->vertex(0)->point().y());
	Point2	tp2(f->vertex(1)->point().x(), f->vertex(1)->point().y());
	Point2	tp3(f->vertex(2)->point().x(), f->vertex(2)->point().y());
	Vector2	vpv(vp);
	tp1 -= vpv;
	tp2 -= vpv;
	tp3 -= vpv;
	double	DEG_TO_NM_LON = DEG_TO_NM_LAT * cos(vp.y * DEG_TO_RAD);
	tp1.x *= (DEG_TO_NM_LON * NM_TO_MTR);
	tp1.y *= (DEG_TO_NM_LAT * NM_TO_MTR);
	tp2.x *= (DEG_TO_NM_LON * NM_TO_MTR);
	tp2.y *= (DEG_TO_NM_LAT * NM_TO_MTR);
	tp3.x *= (DEG_TO_NM_LON * NM_TO_MTR);
	tp3.y *= (DEG_TO_NM_LAT * NM_TO_MTR);
	
	Segment2	s1(tp1, tp2);
	Segment2	s2(tp2, tp3);
	Segment2	s3(tp3, tp1);
	Point2	origin(0.0, 0.0);
	
//	double d1 = s1.squared_distance(origin);
//	double d2 = s2.squared_distance(origin);
//	double d3 = s3.squared_distance(origin);
	double d4 = tp1.squared_distance(origin);
	double d5 = tp2.squared_distance(origin);
	double d6 = tp3.squared_distance(origin);
	
//	double	nearest = min(min(d1, d2), min(min(d3, d4), min(d5, d6)));
	double	nearest = min(min(d4, d5), d6);
	return sqrt(nearest);
}








/***************************************************************************
 * ALGORITHMS TO FIND VALUABLE POINTS IN A DEM *****************************
 ***************************************************************************
 *
 * These routines take a fully populated DEM and copy points of interest into
 * an empty DEM to build up a small number of points we can use to triangulate.
 * 'orig' is always the main DEM and 'deriv' the sparse one.  The goal is to
 * get about 20,000-30,000 points that provide good coverage and capture the
 * terrain morphology. 
 */


/*
 * CopyWetPoints
 *
 * This routine copies the points that are inside water bodies and copies
 * them, modifying their altitude to be at sea level.  It also copies 
 * to another DEM (if desired) points that are near the edges of the water bodies.
 * this can be a useful reference.
 *
 */
void CopyWetPoints(
				const DEMGeo& 			orig, 		// The original DEM
				DEMGeo& 				deriv, 		// All water points, flattened, are added to this DEM
				DEMGeo * 				corners, 	// Near-vertices are added to this DEM
				const Pmwx& 			map)		// The map we get the water bodies from
{
	// BEN NOTE ON CLAMPING: I think we do NOT care if an edge is microscopically outside the DEM
	// in this case...xy_nearest could care less...and the polygon rasterizer doesn't care much
	// either.  We do not generate any coastline edges here.

//	FILE * fi = fopen("dump.txt", "a");
	PolyRasterizer	rasterizer;
	SetupWaterRasterizer(map, orig, rasterizer);

	for (Pmwx::Halfedge_const_iterator i = map.halfedges_begin(); i != map.halfedges_end(); ++i)
	{
		if (i->mDominant)
		{
			bool	iWet = i->face()->IsWater() && !i->face()->is_unbounded();
			bool	oWet = i->twin()->face()->IsWater() && !i->twin()->face()->is_unbounded();
			
			if (iWet != oWet)
			{				
				if (corners)
				{
					int xp, yp;
					float e;
					e = orig.xy_nearest(i->source()->point().x,i->source()->point().y, xp, yp);
					if (e != NO_DATA)
						(*corners)(xp,yp) = e;
					
					e = orig.xy_nearest(i->target()->point().x,i->target()->point().y, xp, yp);
					if (e != NO_DATA)
						(*corners)(xp,yp) = e;
				}				
			}
		}
	}


	int y = 0;
	rasterizer.StartScanline(y);
	while (!rasterizer.DoneScan())
	{
		int x1, x2;
		while (rasterizer.GetRange(x1, x2))
		{
			for (int x = x1; x < x2; ++x)
			{
				int e = orig.get_lowest(x,y,5);
				if (e != NO_DATA)
				{
					deriv(x,y) = e;
//					gMeshPoints.push_back(Point2(orig.x_to_lon(x), orig.y_to_lat(y)));
				}
			}
		}
		++y;
		if (y >= orig.mHeight) break;
		rasterizer.AdvanceScanline(y);
	}
}

/*
 * BuildSparseWaterMesh
 * 
 * Given a DEM that contains all wet points and a DEM that contains approximations (snapped to the DEM) of 
 * all coastline points, this routine adds to a final DEM a sparse subset of points from the wet DEM
 * that are every X points and at least Y points from any coastlines.  This builds a sparse mesh for 
 * water bodies to keep fogging working properly.  It also erases the rest of the water body. 
 *
 */
void	BuildSparseWaterMesh(
					const DEMGeo& inWet, 		// A mesh that contains all water points, dropped to water level
					const DEMGeo& inEdges, 		// The vertices of the water bodies
					DEMGeo& deriv, 				// A few water points are added to this DEM
					int skip, 					// The skip interval - add a water point once every N DEM poionts
					int search)					// Search range for coast vertices - search this far for a nearby  coast point.
{
	int x, y, dx, dy;
	float h;
	for (y = 0; y < inWet.mHeight; y++)
	for (x = 0; x < inWet.mWidth; x++)
	{
		h = inWet.get(x,y);
		if (h != NO_DATA)
		{
			if ((x % skip) == 0 && (y % skip) == 0)
			{
				for (dy = y-search; dy <= y+search; ++dy)
				for (dx = x-search; dx <= x+search; ++dx)
				{
					if (inEdges.get(dx,dy) != NO_DATA)
						goto foundone;
				}
				
				deriv(x,y) = h;
				continue;
foundone:	
				deriv(x,y) = NO_DATA;
			} else
				deriv(x,y) = NO_DATA;
		}
	}
}

/*
 * AddRiverPoints
 *
 * Given a map with marked rivers, a master DEM and a derived DEM of
 * important points, this routine will copy a few of the river points
 * into the DEM using nearest neighbor (so all DEM points falll on the grid)
 * to help get valleys into the mesh. 
 *
 */
int	AddRiverPoints(
			const DEMGeo& 		orig, 		// The original DEM
			DEMGeo& 			deriv, 		// A few river points are added to this one
			const Pmwx& 		map)		// A vector map with the rivers
{	
	// BAS - we do not care about being slightly outside the DEM here...points are only
	// processed via xy_nearsest and clamped onto the DEM, and we skip a lot of river points anyway.
	int added = 0;
	int k = 0;
	for (Pmwx::Halfedge_const_iterator i = map.halfedges_begin(); i != map.halfedges_end(); ++i)
	{
		if (i->mDominant &&
			i->mParams.find(he_IsRiver) != i->mParams.end() &&
			!i->face()->IsWater() &&
			!i->twin()->face()->IsWater())
		{
			int x, y;
			float h;
			h = orig.xy_nearest(i->source()->point().x, i->source()->point().y, x, y);
			if (h != NO_DATA)
			{
				if (deriv(x,y) == NO_DATA && ((k++)%RIVER_SKIP)==0) 
				{   
					++added;
					deriv(x,y) = h;
				}
			}

			h = orig.xy_nearest(i->target()->point().x, i->target()->point().y, x, y);
			if (h != NO_DATA)
			{
				if (deriv(x,y) == NO_DATA && ((k++)%RIVER_SKIP)==0) 
				{   
					++added;
					deriv(x,y) = h;
				}
			}

		}
	}
	return added;
}

/*
 * AddEdgePoints
 *
 * This function adds the edges to the DEMs, at the interval specified.
 *
 */
void AddEdgePoints(
			const DEMGeo& 		orig, 			// The original DEM
			DEMGeo& 			deriv, 			// Edge points are added to this
			int 				interval,		// The interval - add an edge point once every N points.
			int					divisions,		// Number of divisions - 1 means 1 big, "2" means 4 parts, etc.
			bool				has_left,		// True if the left and bottom edges are provided to us.  This is
			bool				has_bottom)		// Useful in making sure our borders match up.
{
	int	div_skip_x = (deriv.mWidth-1) / divisions;
	int	div_skip_y = (deriv.mHeight-1) / divisions;
	int x, y, dx, dy;
	for (y = (has_bottom ? div_skip_y : 0); y < deriv.mHeight; y += div_skip_y)
	for (x = (has_left ? div_skip_x : 0); x < deriv.mWidth; x += div_skip_x)
	{
		for (dy = 0; dy < deriv.mHeight; dy += interval)
		for (dx = 0; dx < deriv.mWidth; dx += interval)
		{
			deriv(x,dx) = orig(x,dx);
			deriv(dx,y) = orig(dx,y);
		}
		if (orig(x,y) == NO_DATA)
			printf("WARNING: mesh point %d,%d lacks data for cutting and edging!\n", x, y);
		deriv(x,y) = orig(x,y);
		
	}
//	deriv(0				,0				) = orig(0			  , 0			  );
//	deriv(0				,deriv.mHeight-1) = orig(0			  , orig.mHeight-1);
//	deriv(deriv.mWidth-1,deriv.mHeight-1) = orig(orig.mWidth-1, orig.mHeight-1);
//	deriv(deriv.mWidth-1,0				) = orig(orig.mWidth-1, 0			  );
}

/* Given a DEM, simply pick out any point that has a north/south/east/west
 * neighbor of more than a certain rise.  This is useful for always getting
 * topographically interesting points.  The gap should be big, otherwise
 * we just select every single hill, which is silly. */
int	AddExtremeVerticalPoints(const DEMGeo& orig, DEMGeo& deriv, float gap)
{
	int x, y, e, e1, e2, e3, e4;
	int	total = 0, added = 0;
	for (y = 0; y < deriv.mHeight; ++y)
	for (x = 0; x < deriv.mWidth; ++x)
	{
		e = orig(x,y);
		if (e != NO_DATA)
		{
			e1 = orig.get(x-1,y);
			e2 = orig.get(x+1,y);
			e3 = orig.get(x,y-1);
			e4 = orig.get(x,y+1);
			if ((e1 != NO_DATA && fabs(e - e1) > gap) ||
				(e2 != NO_DATA && fabs(e - e2) > gap) ||
				(e3 != NO_DATA && fabs(e - e3) > gap) ||
				(e4 != NO_DATA && fabs(e - e4) > gap))
			{
				if (deriv(x,y) == NO_DATA)
					added++;
				total++;
				deriv(x,y) = e;
			}
		}
	}
	return added;	
}

/* This is the damned weirdest point-selection routine of all.  If there
 * is only one change in angle across a point (e.g. from flat to vertical),
 * it's on an "edge" of a cliff or mountain.  But if there are two changes 
 * in angle, it is on a "corner" of a cliff or mountain and is very important
 * for the mesh.  This routine tries to measure such a phenomenon.
 *
 * The code is weird because previously it averaged a change in angle over
 * potentially a significant range of points.  It turns out it works best when
 * we look at only one grid point to our left, right, top and bottom.  (This
 * is with 90 meter DEMs).  Then it turns out that if the product of the slope
 * to our top and right is different enough from our bottom-left, then we need
 * this point.
 *
 * WHY does this work?  Well, generally a change in just the X or Y axis 
 * means we've got a reasonably flat gradient.  (As the one-fold change rotates
 * around, the X and Y multiplication work out to be vaguely constant.  A dot
 * product is probably more appropriate).  BUT if we have a fold in two dimensions,
 * the X-angle change and Y-angle change both get huge and our multiplier crosses
 * a threshhold and we know we have the right point.
 *
 */
int	AddAngularDifferencePoints(
					const DEMGeo& 		orig, 	// Original mesh	
					DEMGeo& 			deriv, 	// Interesting points are added here
					double 				level)	// Heuristic cutoff level
{
	int total = 0, added = 0;
	int x, y;
	for (y = 0; y < deriv.mHeight; ++y)
	for (x = 0; x < deriv.mWidth; ++x)
	{
#define SAMPLE_RANGE 2
		float h = orig(x,y);
		if (h != NO_DATA)
		{
			float	height_left[SAMPLE_RANGE];
			float	height_right[SAMPLE_RANGE];
			float	height_top[SAMPLE_RANGE];
			float	height_bottom[SAMPLE_RANGE];
			height_left[0] = height_right[0] = height_top[0] = height_bottom[0] = 0.0;
			float ct_left = 0.0, ct_right = 0.0, ct_top = 0.0, ct_bottom = 0.0;
			for (int range = 1; range < SAMPLE_RANGE; ++range)
			{
				float	x_dist = orig.x_dist_to_m(range);
				float	y_dist = orig.y_dist_to_m(range);
				height_left[range] = orig.get(x-range,y);
				height_right[range] = orig.get(x+range,y);
				height_bottom[range] = orig.get(x,y-range);
				height_top[range] = orig.get(x,y+range);
				
				if (height_left[range] != NO_DATA)
				{
					height_left[range] = h - height_left[range];
					height_left[range] /= x_dist;
					height_left[0] += height_left[range];
					ct_left += 1.0;
				}
				if (height_right[range] != NO_DATA)
				{
					height_right[range] = height_right[range] - h;
					height_right[range] /= x_dist;
					height_right[0] += height_right[range];
					ct_right += 1.0;
				}
				if (height_bottom[range] != NO_DATA)
				{
					height_bottom[range] = h - height_bottom[range];
					height_bottom[range] /= y_dist;
					height_bottom[0] += height_bottom[range];
					ct_bottom += 1.0;
				}
				if (height_top[range] != NO_DATA)
				{
					height_top[range] = height_top[range] - h;
					height_top[range] /= y_dist;
					height_top[0] += height_top[range];
					ct_top += 1.0;
				}
			}
			if (ct_left != 0.0) height_left[0] /= ct_left;
			if (ct_right != 0.0) height_right[0] /= ct_right;
			if (ct_bottom != 0.0) height_bottom[0] /= ct_bottom;
			if (ct_top != 0.0) height_top[0] /= ct_top;

// VALUES: 0.4 = dolomites, 0.05 = NY

			// This is a measure of some kind of cumulative gradient change, more or less.  It works ok.
			if (fabs(height_top[0] * height_right[0] - height_bottom[0] * height_left[0]) > level)

			// This is an attempt to combine the change on both axes and prioritize a change in both.
//			if ((fabs(height_top[0] - height_bottom[0]) * fabs(height_right[0] - height_left[0]) > 0.02))
			{
				if (deriv(x,y) == NO_DATA)
					added++;
				total++;
				deriv(x,y) = orig(x,y);
			}
		}
	}
	return added;
}

/*
 * FowlerLittle
 *
 * http://www.geog.ubc.ca/courses/klink/gis.notes/ncgia/u39.html#SEC39.1.1
 *
 */
void FowlerLittle(const DEMGeo& orig, DEMGeo& deriv)
{
	DEMGeo	passes(orig.mWidth, orig.mHeight);
	DEMGeo	lowest(orig.mWidth, orig.mHeight);
	DEMGeo	highest(orig.mWidth, orig.mHeight);
	passes = NO_DATA;
	int x, y;
	for (y = 1; y < (orig.mHeight-1); ++y)
	for (x = 1; x < (orig.mWidth-1); ++x)
	{
		float e = orig.get(x,y);
		bool dif[8];
		dif[0] = orig.get(x  ,y+1) > e;
		dif[1] = orig.get(x+1,y+1) > e;
		dif[2] = orig.get(x+1,y  ) > e;
		dif[3] = orig.get(x+1,y-1) > e;
		dif[4] = orig.get(x  ,y-1) > e;
		dif[5] = orig.get(x-1,y-1) > e;
		dif[6] = orig.get(x-1,y  ) > e;
		dif[7] = orig.get(x-1,y+1) > e;
		
		int cycles = 0;
		for (int n = 0; n < 8; ++n)
		if (dif[n] != dif[(n+7)%8])
			++cycles;
		
		if (cycles == 0)
			deriv(x,y) = orig(x,y);
		if (cycles > 2)
			passes(x,y) = 1.0;
	}
	for (y = 1; y < (orig.mHeight); ++y)
	for (x = 1; x < (orig.mWidth); ++x)
	{
		float e[4];
		e[0] = orig.get(x-1,y-1);
		e[1] = orig.get(x  ,y-1);
		e[2] = orig.get(x-1,y  );
		e[3] = orig.get(x  ,y  );

		if (e[0] < e[1] &&
			e[0] < e[2] &&
			e[0] < e[3])	lowest(x-1,y-1) = 1;
		if (e[0] > e[1] &&
			e[0] > e[2] &&
			e[0] > e[3])	highest(x-1,y-1) = 1;

		if (e[1] < e[0] &&
			e[1] < e[2] &&
			e[1] < e[3])	lowest(x  ,y-1) = 1;
		if (e[1] > e[0] &&
			e[1] > e[2] &&
			e[1] > e[3])	highest(x  ,y-1) = 1;

		if (e[2] < e[0] &&
			e[2] < e[1] &&
			e[2] < e[3])	lowest(x-1,y  ) = 1;
		if (e[2] > e[0] &&
			e[2] > e[1] &&
			e[2] > e[3])	highest(x-1,y  ) = 1;

		if (e[3] < e[0] &&
			e[3] < e[1] &&
			e[3] < e[2])	lowest(x  ,y  ) = 1;
		if (e[3] > e[0] &&
			e[3] > e[1] &&
			e[3] > e[2])	highest(x  ,y  ) = 1;		
	}

	for (y = 0; y < (orig.mHeight); ++y)
	for (x = 0; x < (orig.mWidth); ++x)
	{
		if (passes(x,y) != 0.0)
		if (lowest(x,y) == 0.0 || highest(x,y) == 0.0)
			deriv(x,y) = orig(x,y);
	}	
}

static GISHalfedge * ExtendLanduseEdge(GISHalfedge * start)
{
	GISVertex * target;
	Pmwx::Halfedge_around_vertex_circulator	circ, stop;
	GISHalfedge * next;
	
	start->mMark = true;
	start->twin()->mMark = true;
	Vector2 dir_v(start->source()->point(), start->target()->point());
	dir_v.normalize();
	
	while (1)
	{
		target = start->target();
		circ = stop = target->incident_halfedges();
		next = NULL;
		do {
			if (start != circ)
			{
				if (circ->face()->mTerrainType != circ->twin()->face()->mTerrainType)
				{
					Vector2 d(circ->target()->point(), circ->source()->point());
					d.normalize();
					if (dir_v.dot(d) > 0.999847695156 &&
						!circ->mMark &&
						circ->face()->mTerrainType == start->twin()->face()->mTerrainType &&
						circ->twin()->face()->mTerrainType == start->face()->mTerrainType)
					{
						DebugAssert(next == NULL);
						next = circ->twin();
					} else 
						return start;
				}
			}
			++circ;
		} while (circ != stop);

		if (next == NULL)
			return start;
		else {
			start = next;
			next->mMark = true;
			next->twin()->mMark = true;
		}
	}
} 

void CollectPointsAlongLine(const Point2& p1, const Point2& p2, vector<Point2>& outPts, DEMGeo& ioDem)
{
	outPts.push_back(p1);
	
	int x1 = floor(ioDem.lon_to_x(p1.x));
	int y1 = floor(ioDem.lat_to_y(p1.y));
	int x2 = ceil(ioDem.lon_to_x(p2.x));
	int y2 = ceil(ioDem.lat_to_y(p2.y));
	if (x1 > x2) swap(x1, x2);
	if (y1 > y2) swap(y1, y2);
	
	double	DEG_TO_MTR_LON = DEG_TO_MTR_LAT * cos(DEG_TO_RAD * (p1.y + p2.y) * 0.5);

	Segment2	seg(p1, p2);
	Vector2		dir(p1, p2);
	dir.normalize();
	multimap<double, Point2>	added_pts;
	
	for (int y = y1; y <= y2; ++y)
	for (int x = x1; x <= x2; ++x)
	{
		float e = ioDem.get(x,y);
		if (e != NO_DATA)
		{
			Point2 test = Point2(ioDem.x_to_lon(x), ioDem.y_to_lat(y));
			Point2 proj = seg.projection(test);
			if (seg.collinear_has_on(proj) && proj != p1 && proj != p2)
			{			
				double alen = Vector2(p1, proj).squared_length();
				double llen = Vector2(proj, test).squared_length();
				
				if ((llen*16.0) < alen)
				{
//					gMeshPoints.push_back(test);
					added_pts.insert(multimap<double, Point2>::value_type(alen, proj));
					
					if (LonLatDistMetersWithScale(test.x, test.y, proj.x, proj.y, DEG_TO_MTR_LON, DEG_TO_MTR_LAT) < 10.0)
					if (x != 0 && x != (ioDem.mWidth-1) && y != 0 && y != (ioDem.mHeight-1))
						ioDem(x,y) = NO_DATA;
				}			
			}
		}
	}
	
	for (multimap<double, Point2>::iterator i = added_pts.begin(); i != added_pts.end(); ++i)
	if (LonLatDistMetersWithScale(outPts.back().x, outPts.back().y, i->second.x, i->second.y, DEG_TO_MTR_LON, DEG_TO_MTR_LAT) > 30.0)	
	if (LonLatDistMetersWithScale(p2.x, p2.y, i->second.x, i->second.y, DEG_TO_MTR_LON, DEG_TO_MTR_LAT) > 30.0)	
		outPts.push_back(i->second);
	
	outPts.push_back(p2);
	
}

/*
 * AddWaterMeshPoints
 *
 * Given a water map, this point adds the vertices (based on the rough height of the master DEM) into
 * the triangualtion and clears any nearby DEM points in the slave DEM.  If it is hires, constraints
 * are added to enforce coastlines.
 *
 */
void	AddWaterMeshPoints(	
				Pmwx& 								inMap, 		// Vec Map of waterbodies
				const DEMGeo& 						master, 	// Master DEM with elevations
				const DEMGeo& 						water, 		// Water bodies lowered
				DEMGeo& 							slave, 		// This DEM has mesh points erased where vertices are added
				CDT& 								outMesh, 	// Vertices and constraints added to this mesh
				vector<LanduseConstraint_t>&		outCons,	// The constraints we add for water are added here for later use
				bool 								hires)		// True if we are hires and need constraints.
{
	/*******************************************************************************************
	 * FIND POLYGON GROUPS THAT CONTAIN LAND USE DIFFERENCES
	 *******************************************************************************************/
	 
	// We are going to go through the whole map and find every halfedge that represents a real land use
	// change.
	
		CDT::Face_handle	local = NULL;	// For cache coherency
		CDT::Vertex_handle	v1, v2;
		float				e1, e2;
		
		Pmwx::Halfedge_iterator he;

	for (he = inMap.halfedges_begin(); he != inMap.halfedges_end(); ++he)
		he->mMark = false;
		
	for (he = inMap.halfedges_begin(); he != inMap.halfedges_end(); ++he)
	if (he->mDominant)
	if (!he->mMark)
	{
		Pmwx::Face_const_handle	f1 = he->face();
		Pmwx::Face_const_handle	f2 = he->twin()->face();
		
		if (!f1->is_unbounded() && !f2->is_unbounded() &&
			f1->mTerrainType != f2->mTerrainType)
		{
			GISHalfedge * extended1 = ExtendLanduseEdge(he);
			GISHalfedge * extended2 = ExtendLanduseEdge(he->twin());
			Point2	p1(extended2->target()->point().x, extended2->target()->point().y);
			Point2	p2(extended1->target()->point().x, extended1->target()->point().y);
			SlightClampToDEM(p1, master);
			SlightClampToDEM(p2, master);
			
			vector<Point2>	pts;
			CollectPointsAlongLine(p1, p2, pts, slave);
			
			for (int n = 1; n < pts.size(); ++n)
			{				
				e1 = NO_DATA;
				e2 = NO_DATA;
				if (f1->mTerrainType == terrain_Water || f2->mTerrainType == terrain_Water)
				{
					e1 = water.xy_nearest(pts[n-1].x, pts[n-1].y);
					e2 = water.xy_nearest(pts[n].x, pts[n].y);
					if (e1 == NO_DATA) 
						e1 = water.search_nearest(pts[n-1].x, pts[n-1].y);
					if (e2 == NO_DATA) 
						e2 = water.search_nearest(pts[n].x,pts[n].y);
					if (e1 == NO_DATA || e2 == NO_DATA)
						printf("WARNING: FOUND NO FLAT WATER DATA NEARBY\n");
				}
				if (e1 == NO_DATA)					e1 = master.value_linear(pts[n-1].x, pts[n-1].y);
				if (e2 == NO_DATA)					e2 = master.value_linear(pts[n].x, pts[n].y);
				if (e1 == NO_DATA)					e1 = master.xy_nearest(pts[n-1].x, pts[n-1].y);
				if (e2 == NO_DATA)					e2 = master.xy_nearest(pts[n].x, pts[n].y);
//				slave.zap_linear(pts[n-1].x, pts[n-1].y);
//				slave.zap_linear(pts[n].x, pts[n].y);
				
				if (e1 == NO_DATA || e2 == NO_DATA) printf("ERROR: missing elevation data for constraint.\n");
				v1 = outMesh.insert(CDT::Point(pts[n-1].x, pts[n-1].y), local);
				v1->info().height = e1;
				local = v1->face();
				v2 = outMesh.insert(CDT::Point(pts[n].x, pts[n].y), local);
				v2->info().height = e2;
				local = v2->face();
				
				outMesh.insert_constraint(v1, v2);
				outCons.push_back(LanduseConstraint_t(ConstraintMarker_t(v1,v2),LandusePair_t(f1->mTerrainType, f2->mTerrainType)));
			}
		}
	}
}

/* 
 * FindMinMaxPointsOnMesh
 *
 * This routine selects points based on local minima and maxima.  We use a sliding window (sliding
 * half a window at a time) and we keep just the minimum and maximum.  We only take these if the
 * total rise over the area is more than 5 meters.  Also, when our window is a full 30 DEM points
 * (a large window) we will allow the edges of the window to be treated as local minimums and maximums.
 * This means that if the entire window is sloped evenly, we'll take the edges and add them.  This
 * means that even on a flat hill we get points every now and then, which is desirable.
 *
 */
int	FindMinMaxPointsOnMesh(
			const DEMGeo& 		orig, 		// Original DEM
			DEMGeo& 			deriv, 		// min max points are added into this
			bool 				hires)		// True if we are hires
{
	int x, y, added = 0, total = 0;
	vector<DEMGeo>	mincache, maxcache;
	DEMGeo_BuildMinMax(orig, mincache, maxcache, 4);
			
	for (int window = (hires ? 10 : 30); window < 40; window += 10)
	{
		for (y = 0; y < (deriv.mHeight - window); y += (window / 2))
		for (x = 0; x < (deriv.mWidth - window); x += (window / 2))
		{
			int minx, miny, maxx, maxy;
			float minh, maxh;
			float rise = DEMGeo_LocalMinMaxWithCache(orig, mincache, maxcache, x,y,x+window,y+window, minx, miny, minh, maxx, maxy, maxh,
				window >= 30.0);
			if (rise != NO_DATA)
			{
				float dist = orig.y_dist_to_m(window);
//				if ((rise / dist) > kRatioTable[window])
				if (rise > 5.0)
				{
					if (minh != NO_DATA)
					{
						++total;
						if (deriv(minx, miny) == NO_DATA) ++added;
						deriv(minx, miny) = minh;
					}
					if (maxh != NO_DATA)
					{
						deriv(maxx, maxy) = maxh;
						++total;
						if (deriv(maxx, maxy) == NO_DATA) ++added;
					}
				}
			}
		}
	}	
	return added;
}

void	SetWaterBodiesToWet(CDT& ioMesh, vector<LanduseConstraint_t>& inCoastlines)
{
	set<CDT::Face_handle>		wet_faces;
	set<CDT::Face_handle>		visited;

	// Quick pass - set everyone to natural.   This is needed because if there are no polys,
	// then the outside of those polys won't make natural terrain.
	
	for (CDT::Finite_faces_iterator ffi = ioMesh.finite_faces_begin(); ffi != ioMesh.finite_faces_end(); ++ffi)
	{
		ffi->info().terrain_general = terrain_Natural;
		ffi->info().terrain_specific = NO_VALUE;
	} 

	// Next mark every point on a tri that's just inside as hot unless it's also an edge point.	
	// Also mark these tris as wet.
	for (vector<LanduseConstraint_t>::iterator c = inCoastlines.begin(); c != inCoastlines.end(); ++c)
	{
		CDT::Face_handle	face_h;
		int					vnum;
		// Dig up the face that includes our edge.  is_edge gives us the right-hand side triangle, but we want
		// the left since this is a counter clockwise boundary, so go backward on the constraint.
		
		if (!PersistentFindEdge(ioMesh, c->first.second, c->first.first, face_h, vnum))
		{
			AssertPrintf("ASSERTION FAILURE: constraint not an edge.\n");
		} else {
			face_h->info().terrain_general = c->second.first;
			wet_faces.insert(face_h);
		}

		if (!PersistentFindEdge(ioMesh, c->first.first, c->first.second, face_h, vnum))
		{
			AssertPrintf("ASSERTION FAILURE: constraint not an edge.\n");
		} else {
			face_h->info().terrain_general = c->second.second;
			wet_faces.insert(face_h);
		}
	}

	while (!wet_faces.empty())
	{
		CDT::Face_handle f = *wet_faces.begin();
		wet_faces.erase(f);
		visited.insert(f);
		
		int tg = f->info().terrain_general;		
		f->info().flag = 0;
		f->info().terrain_specific = NO_VALUE;
		CDT::Face_handle	fn;
		if (!ioMesh.is_constrained(CDT::Edge(f,0)))
		{
			fn = f->neighbor(0);
			if (!ioMesh.is_infinite(fn))
			if (visited.find(fn) == visited.end())
			{
				if (fn->info().terrain_general != terrain_Natural && fn->info().terrain_general != tg)
					AssertPrintf("Error: conflicting terrain assignment between %s and %s, near %lf, %lf\n",
							FetchTokenString(fn->info().terrain_general), FetchTokenString(tg),
							CGAL::to_double(f->vertex(0)->point().x()), CGAL::to_double(f->vertex(0)->point().y()));
				fn->info().terrain_general = tg;
				wet_faces.insert(fn);
			}
		}

		if (!ioMesh.is_constrained(CDT::Edge(f,1)))
		{
			fn = f->neighbor(1);
			if (!ioMesh.is_infinite(fn))
			if (visited.find(fn) == visited.end())
			{
				if (fn->info().terrain_general != terrain_Natural && fn->info().terrain_general != tg)
					AssertPrintf("Error: conflicting terrain assignment between %s and %s, near %lf, %lf\n",
							FetchTokenString(fn->info().terrain_general), FetchTokenString(tg),
							CGAL::to_double(f->vertex((1))->point().x()), CGAL::to_double(f->vertex((1))->point().y()));
				fn->info().terrain_general = tg;
				wet_faces.insert(fn);
			}
		}

		if (!ioMesh.is_constrained(CDT::Edge(f,2)))
		{
			fn = f->neighbor(2);
			if (!ioMesh.is_infinite(fn))
			if (visited.find(fn) == visited.end())
			{
				if (fn->info().terrain_general != terrain_Natural && fn->info().terrain_general != tg)
					AssertPrintf("Error: conflicting terrain assignment between %s and %s, near %lf, %lf\n",
							FetchTokenString(fn->info().terrain_general), FetchTokenString(tg),
							CGAL::to_double(f->vertex((2))->point().x()), CGAL::to_double(f->vertex((2))->point().y()));
				fn->info().terrain_general = tg;
				wet_faces.insert(fn);
			}
		}
	}
}

/*
 * BuildCutLinesInDEM
 *
 * This routine builds horizontal and vertical lines in a DEM via constraints.
 * Useful for cutting over on a rectangularly mapped texture.
 *
 * The DEM points that are used are removed from the mesh to keep them from getting hit multiple times.
 *
 */
void	BuildCutLinesInDEM(
				DEMGeo&					ioDem,
				CDT&					outMesh,
				int						segments)	// Number of cuts per dim, 1 means no action taken!
{
	CDT::Face_handle	local;

	int x_interval = (ioDem.mWidth-1) / segments;
	int y_interval = (ioDem.mHeight-1) / segments;
	vector<CDT::Vertex_handle>	junctions;
	junctions.resize((segments+1)*(segments+1));
	
	// First, there will be some crossing points - add every one of them to the triangulation.
	int x, y, dx, dy;
	for (y = 0; y < ioDem.mHeight; y += y_interval)
	for (x = 0; x < ioDem.mWidth; x += x_interval)
	{
		float h = ioDem(x,y);
		if (h != NO_DATA)
		{			
//			gMeshPoints.push_back(Point_2(ioDem.x_to_lon(x),ioDem.y_to_lat(y)));
#if !NO_TRIANGULATE
			CDT::Vertex_handle vv = outMesh.insert(CDT::Point(ioDem.x_to_lon(x),ioDem.y_to_lat(y)), local);
			vv->info().height = h;
			local = vv->face();
#endif
			junctions[(x / x_interval) + (y / y_interval) * (segments+1)] = vv;
		} else
			AssertPrintf("Needed DEM point AWOL - %d,%d.\n",x,y);
	}
	
	// Next, add the vertical segments.  Run through each vertical stripe except the edges,
	// for every horizontal one except the top.  This is each vertical band we must add.
	for (y = y_interval; y < ioDem.mHeight; y += y_interval)
	for (x = x_interval; x < (ioDem.mWidth-x_interval); x += x_interval)
	{
		CDT::Vertex_handle	v1, v2;
		v1 = junctions[(x / x_interval) + ((y-y_interval) / y_interval) * (segments+1)];
		for (dy = y - y_interval + 1; dy < y; ++dy)
		{
			float h = ioDem(x,dy);
			if (h != NO_DATA)
			{
//				gMeshPoints.push_back(Point_2(ioDem.x_to_lon(x),ioDem.y_to_lat(dy)));
	#if !NO_TRIANGULATE
				v2 = outMesh.insert(CDT::Point(ioDem.x_to_lon(x),ioDem.y_to_lat(dy)), local);
				v2->info().height = h;
				local = v2->face();
				outMesh.insert_constraint(v1, v2);
				v2 = v1;
	#endif				
			} 			
		}
		v2 = junctions[(x / x_interval) + (y / y_interval) * (segments+1)];
		outMesh.insert_constraint(v1, v2);
		
	}
	
	// Same thing but horizontal-like.
	for (y = y_interval; y < (ioDem.mHeight-y_interval); y += y_interval)
	for (x = x_interval; x < ioDem.mWidth; x += x_interval)
	{
		CDT::Vertex_handle	v1, v2;
		v1 = junctions[((x-x_interval) / x_interval) + (y / y_interval) * (segments+1)];
		for (dx = x - x_interval + 1; dx < x; ++dx)
		{
			float h = ioDem(dx,y);
			if (h != NO_DATA)
			{				
//				gMeshPoints.push_back(Point_2(ioDem.x_to_lon(dx),ioDem.y_to_lat(y)));
	#if !NO_TRIANGULATE
				v2 = outMesh.insert(CDT::Point(ioDem.x_to_lon(dx),ioDem.y_to_lat(y)), local);
				v2->info().height = h;
				local = v2->face();
				outMesh.insert_constraint(v1, v2);
				v2 = v1;
	#endif				
			} 			
		}		
		v2 = junctions[(x / x_interval) + (y / y_interval) * (segments+1)];
		outMesh.insert_constraint(v1, v2);		
	}
}
				

/*
 * AddBulkPointsToMesh
 *
 * Given a DEM with points that need to be in the mesh, this routine blasts them in.
 *
 */
void	AddBulkPointsToMesh(
				DEMGeo& 				ioDEM, 		// DEM with data only where we want mesh points, points are nuked
				CDT& 					outMesh,
				ProgressFunc			inFunc)	// mesh to receive points.
{
	if (inFunc) inFunc(1, 3, "Building Triangle Mesh", 0.0);

	int x, y, step, total = 0;
	CDT::Face_handle	local;
	for (step = 1024; step > 0; step /= 4)
	{
		for (y = 0; y < ioDEM.mHeight; y+=step)
		for (x = 0; x < ioDEM.mWidth; x+=step)
		{
			if (inFunc && x == 0 && (y % 20 == 0)) inFunc(1, 3, "Building Triangle Mesh", (double) y / (double) ioDEM.mHeight);
			float h = ioDEM(x,y);
			if (h != NO_DATA)
			{			
	//			gMeshPoints.push_back(Point_2(ioDEM.x_to_lon(x),ioDEM.y_to_lat(y)));
	#if !NO_TRIANGULATE
				CDT::Point	p(ioDEM.x_to_lon(x),ioDEM.y_to_lat(y));
				CDT::Locate_type tp;
				int vnum;
				local = outMesh.locate(p, tp, vnum, local);
	//			if (tp != CDT::EDGE)
				{
					CDT::Vertex_handle vv = outMesh.insert(p, local);
					vv->info().height = h;
					local = vv->face();	
					++total;
				}
	#endif			
				ioDEM(x,y) = NO_DATA;
			}
		}
//		if (step == 1) break;
	}
//	printf("Inserted %d points.\n", total);
	if (inFunc) inFunc(1, 3, "Building Triangle Mesh", 1.0);
}

/*
 * CalculateMeshNormals
 *
 * This routine calcs the normals per vertex.
 *
 */
void CalculateMeshNormals(CDT& ioMesh)
{
	for (CDT::Finite_vertices_iterator i = ioMesh.finite_vertices_begin(); i != ioMesh.finite_vertices_end(); ++i)
	{
		Vector3	total(0.0, 0.0, 0.0);
		CDT::Vertex_circulator last = ioMesh.incident_vertices(i);
		CDT::Vertex_circulator nowi = last, stop = last;
		do {
			last = nowi;
			++nowi;
			Point3	lastP(last->point().x(), last->point().y(), last->info().height);
			Point3	nowiP(nowi->point().x(), nowi->point().y(), nowi->info().height);
			Point3	selfP(   i->point().x(),    i->point().y(),    i->info().height);
			Vector3	v1(selfP, lastP);
			Vector3	v2(selfP, nowiP);
			v1.dx *= (DEG_TO_MTR_LAT * cos(selfP.y * DEG_TO_RAD));
			v2.dx *= (DEG_TO_MTR_LAT * cos(selfP.y * DEG_TO_RAD));
			v1.dy *= (DEG_TO_MTR_LAT);
			v2.dy *= (DEG_TO_MTR_LAT);
			v1.normalize();
			v2.normalize();
			Vector3	normal(v1.cross(v2));
			normal.normalize();
			CDT::Face_handle	a_face;
			if (ioMesh.is_face(i, last, nowi, a_face))
			{
				a_face->info().normal[0] = normal.dx;
				a_face->info().normal[1] = normal.dy;
				a_face->info().normal[2] = normal.dz;
			}
			total += normal;			
		} while (nowi != stop);
		total.normalize();
		i->info().normal[0] = total.dx;
		i->info().normal[1] = total.dy;
		i->info().normal[2] = total.dz;
	}
}					

/*******************************************************************************************
 *******************************************************************************************
 ** GENERATION OF A MESH MASTER ROUTINE ****************************************************
 *******************************************************************************************
 *******************************************************************************************/







void	TriangulateMesh(Pmwx& inMap, CDT& outMesh, DEMGeoMap& inDEMs, ProgressFunc prog)
{
	TIMER(Total)
//	if (prog && prog(0, 3, "Differencing", 0.0)) return;		

//	Pmwx	aMap(inMap);
//	ReduceToWaterBodies(aMap);
	outMesh.clear();
//	gMeshPoints.clear();
//	gMeshLines.clear();
	
	int		x, y;
	DEMGeo&	orig(inDEMs[dem_Elevation]);
	
	Assert(orig.get(0			 ,0				) != NO_DATA);
	Assert(orig.get(orig.mWidth-1,orig.mHeight-1) != NO_DATA);
	Assert(orig.get(0			 ,orig.mHeight-1) != NO_DATA);
	Assert(orig.get(orig.mWidth-1,orig.mHeight-1) != NO_DATA);
	
	DEMGeo	deriv(orig.mWidth, orig.mHeight);
	deriv.copy_geo_from(orig);
	deriv = NO_DATA;

	DEMGeo	outline(deriv);
	DEMGeo	water(deriv);
	
	int basePoints;
	int angularPoints;
	
	if (prog) prog(0, 3, "Calculating Mesh Points", 0.0);

	if (!gMeshPrefs.fowler_little)
	{
		TIMER(minmax)
		basePoints = FindMinMaxPointsOnMesh(orig, deriv, true);
	}

	if (prog) prog(0, 3, "Calculating Mesh Points", 0.1);
	
	if (!gMeshPrefs.fowler_little)
	{
		TIMER(angdif)
		DEMGeo	save(deriv);
		double ramp = 0.05;
		do {
			angularPoints = AddAngularDifferencePoints(orig, deriv, ramp);
			printf("Added angular points: %d (ramp: %lf)\n", angularPoints, ramp);
			if (angularPoints > gMeshPrefs.max_mountain_points)
			{
				deriv = save;
				ramp *= 2.0;
			}
		} while (angularPoints > gMeshPrefs.max_mountain_points);
	}

	if (prog) prog(0, 3, "Calculating Mesh Points", 0.15);
	
	if (!gMeshPrefs.fowler_little)
	{
		TIMER(extreme_v)
		AddExtremeVerticalPoints(orig, deriv, gMeshPrefs.cliff_height);
	}
	
	if (prog) prog(0, 1, "Calculating Triangle Mesh", 0.2);	
	
	if (!gMeshPrefs.fowler_little)
	{
		TIMER(River)
		AddRiverPoints(orig, deriv, inMap);
	}

	if (gMeshPrefs.fowler_little)
	{
		TIMER(fowler_little)
		FowlerLittle(orig, deriv);
	}	
	
	if (prog) prog(0, 3, "Calculating Mesh Points", 0.2);

	{
		TIMER(linear_remove)
		deriv.remove_linear(2, gMeshPrefs.max_error);
	}

	if (prog) prog(0, 3, "Calculating Mesh Points", 0.25);

	// MAKE SURE the corners are in the DEM!  
//	if (deriv.get(0				,0				) == NO_DATA)		deriv(0				,0				) = orig.get(0			   ,0			   );
//	if (deriv.get(0				,deriv.mHeight-1) == NO_DATA)		deriv(0				,deriv.mHeight-1) = orig.get(0			   ,deriv.mHeight-1);
//	if (deriv.get(deriv.mWidth-1,0				) == NO_DATA)		deriv(deriv.mWidth-1,0				) = orig.get(deriv.mWidth-1,0			   );
//	if (deriv.get(deriv.mWidth-1,deriv.mHeight-1) == NO_DATA)		deriv(deriv.mWidth-1,deriv.mHeight-1) = orig.get(deriv.mWidth-1,deriv.mHeight-1);
		


	if (prog) prog(0, 3, "Calculating Mesh Points", 0.3);

	{
		TIMER(build_wet_map)
		CopyWetPoints(orig, water, &outline, inMap);
	}
	
	if (prog) prog(0, 3, "Calculating Mesh Points", 0.4);
	
	{
		TIMER(sparsify_wet_map)
		BuildSparseWaterMesh(water, outline, deriv, LOW_RES_WATER_INTERVAL, LOW_RES_WATER_INTERVAL/2);
	}
	
	
	int	ct = 0;
	for (y = 0; y < deriv.mHeight; ++y)
	for (x = 0; x < deriv.mWidth; ++x)
	{
		float h = deriv(x,y);
		if (h != NO_DATA)
			++ct;
	}
	
	char	buf[100];
	sprintf(buf,"%d triangles", ct);

	if (prog) prog(0, 3, "Calculating Mesh Points", 0.6);

	// Order matters - it is better to do coastlines first and the DEM second.
	// A regular type-writer style insert means that all points are inserted
	// outside the hull, which is real slow - the hull has a huge number of 
	// vertices.

	vector<LanduseConstraint_t>	coastlines_markers;

#if !NO_TRIANGULATE			
	
	{
		TIMER(edges);
		
		char	fname_left[32];
		char	fname_bot[32];
		sprintf(fname_left,"%+03d%+04d.border.txt", (int) (deriv.mSouth), (int) (deriv.mWest - 1));
		sprintf(fname_bot ,"%+03d%+04d.border.txt", (int) (deriv.mSouth - 1), (int) (deriv.mWest));
		
		mesh_match_t bot_junk, left_junk;
		bool	has_left = load_match_file(fname_left, left_junk, gMatchLeft);
		bool	has_bot  = load_match_file(fname_bot , gMatchBottom, bot_junk);
		
		AddEdgePoints(orig, deriv, 20, 1, has_left, has_bot);
	}

	/// Moved AddWaterMeshPoints down here - so we can correctly desliver the edges of the mesh!
	{
		TIMER(Triangulate_Coastlines)
		AddWaterMeshPoints(inMap, orig, water, deriv, outMesh, coastlines_markers, true);
	}
	
	if (prog) prog(0, 3, "Calculating Mesh Points", 0.8);


#endif	

	{
		if (!gMatchLeft.vertices.empty())
		for (y = 0; y < deriv.mHeight; ++y)
			deriv(0, y) = NO_DATA;
		if (!gMatchBottom.vertices.empty())
		for (x = 0; x < deriv.mWidth; ++x)
			deriv(x, 0) = NO_DATA;
	}

	if (prog) prog(0, 3, "Calculating Mesh Points", 1.0);
		
	{
		TIMER(Triangulate_Elevation)
		
		AddBulkPointsToMesh(deriv, outMesh, prog);
	}	
	
	if (!gMatchLeft.vertices.empty())
		match_border(outMesh, gMatchLeft, true);
	if (!gMatchBottom.vertices.empty())
		match_border(outMesh, gMatchBottom, false);

	if (prog) prog(2, 3, "Calculating Wet Areas", 0.2);

	{
		SetWaterBodiesToWet(outMesh, coastlines_markers);	
	}
	
	if (prog) prog(2, 3, "Calculating Wet Areas", 0.5);
	
	CalculateMeshNormals(outMesh);

	if (prog) prog(2, 3, "Calculating Wet Areas", 1.0);
	
//	orig.swap(water);
}


/*******************************************************************************************
 *******************************************************************************************
 ** MESH LANDUSE ASSIGNMENT ****************************************************************
 *******************************************************************************************
 *******************************************************************************************/


void	AssignLandusesToMesh(	DEMGeoMap& inDEMs, 
								CDT& ioMesh, 
								ProgressFunc	inProg)
{


		CDT::Finite_faces_iterator tri;
		CDT::Finite_vertices_iterator vert;

		int	rock_enum = LookupToken("rock_gray.ter");

	if (inProg) inProg(0, 1, "Assigning Landuses", 0.0);

	DEMGeo&	inClimate(inDEMs[dem_Climate]);
	DEMGeo&	inElevation(inDEMs[dem_Elevation]);
	DEMGeo&	inSlope(inDEMs[dem_Slope]);
	DEMGeo&	inSlopeHeading(inDEMs[dem_SlopeHeading]);
	DEMGeo&	inRelElev(inDEMs[dem_RelativeElevation]);
	DEMGeo&	inRelElevRange(inDEMs[dem_ElevationRange]);
	DEMGeo&	inTemp(inDEMs[dem_Temperature]);
	DEMGeo&	inTempRng(inDEMs[dem_TemperatureRange]);
	DEMGeo&	inRain(inDEMs[dem_Rainfall]);
	DEMGeo& inUrbanDensity(inDEMs[dem_UrbanDensity]);
	DEMGeo& inUrbanRadial(inDEMs[dem_UrbanRadial]);
	DEMGeo& inUrbanTransport(inDEMs[dem_UrbanTransport]);
	DEMGeo& usquare(inDEMs[dem_UrbanSquare]);

	DEMGeo	landuse(inDEMs[dem_LandUse]);

	for (int y = 0; y < landuse.mHeight;++y)
	for (int x = 0; x < landuse.mWidth; ++x)
	{
		float e = landuse(x,y);
		if (e == NO_VALUE || 
			e == lu_usgs_INTERRUPTED_AREAS || 
//			e == lu_usgs_URBAN_SQUARE || 
//			e == lu_usgs_URBAN_IRREGULAR || 
			e == lu_usgs_INLAND_WATER || 
			e == lu_usgs_SEA_WATER || 
			e == lu_usgs_NO_DATA)
			landuse(x,y) = NO_DATA;			
	}
	landuse.fill_nearest();	

	/***********************************************************************************************
	 * ASSIGN BASIC LAND USES TO MESH
	 ***********************************************************************************************/

	if (inProg) inProg(0, 1, "Assigning Landuses", 0.1);
	for (tri = ioMesh.finite_faces_begin(); tri != ioMesh.finite_faces_end(); ++tri)
	{
		// First assign a basic land use type.
		{
			tri->info().flag = 0;
			// Hires - take from DEM if we don't have one.
			if (tri->info().terrain_general != terrain_Water)
			{
				double	center_x = (tri->vertex(0)->point().x() + tri->vertex(1)->point().x() + tri->vertex(2)->point().x()) / 3.0;
				double	center_y = (tri->vertex(0)->point().y() + tri->vertex(1)->point().y() + tri->vertex(2)->point().y()) / 3.0;
				
				float lu  = landuse.search_nearest(center_x, center_y);
				float lu1 = landuse.search_nearest(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float lu2 = landuse.search_nearest(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float lu3 = landuse.search_nearest(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());

				float cl  = inClimate.search_nearest(center_x, center_y);
				float cl1 = inClimate.search_nearest(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float cl2 = inClimate.search_nearest(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float cl3 = inClimate.search_nearest(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());

				if (lu == NO_DATA)
					fprintf(stderr, "NO data anywhere near %f, %f\n", center_x, center_y);
				lu = MAJORITY_RULES(lu,lu1,lu2, lu3);
				cl = MAJORITY_RULES(cl, cl1, cl2, cl3);

				float	el1 = inElevation.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	el2 = inElevation.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	el3 = inElevation.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	el = SAFE_AVERAGE(el1, el2, el3);
				
				float	sl1 = inSlope.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	sl2 = inSlope.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	sl3 = inSlope.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	sl = SAFE_MAX	 (sl1, sl2, sl3);	// Could be safe max.

				float	tm1 = inTemp.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	tm2 = inTemp.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	tm3 = inTemp.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	tm = SAFE_AVERAGE(tm1, tm2, tm3);	// Could be safe max.

				float	tmr1 = inTempRng.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	tmr2 = inTempRng.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	tmr3 = inTempRng.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	tmr = SAFE_AVERAGE(tmr1, tmr2, tmr3);	// Could be safe max.

				float	rn1 = inRain.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	rn2 = inRain.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	rn3 = inRain.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	rn = SAFE_AVERAGE(rn1, rn2, rn3);	// Could be safe max.

				float	sh1 = inSlopeHeading.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	sh2 = inSlopeHeading.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	sh3 = inSlopeHeading.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	sh = SAFE_AVERAGE(sh1, sh2, sh3);	// Could be safe max.

				float	re1 = inRelElev.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	re2 = inRelElev.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	re3 = inRelElev.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	re = SAFE_AVERAGE(re1, re2, re3);	// Could be safe max.

				float	er1 = inRelElevRange.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	er2 = inRelElevRange.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	er3 = inRelElevRange.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	er = SAFE_AVERAGE(er1, er2, er3);	// Could be safe max.
				
				int		near_water =(tri->neighbor(0)->info().terrain_general == terrain_Water && !ioMesh.is_infinite(tri->neighbor(0))) ||
									(tri->neighbor(1)->info().terrain_general == terrain_Water && !ioMesh.is_infinite(tri->neighbor(1))) ||
									(tri->neighbor(2)->info().terrain_general == terrain_Water && !ioMesh.is_infinite(tri->neighbor(2)));

				float	uden1 = inUrbanDensity.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	uden2 = inUrbanDensity.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	uden3 = inUrbanDensity.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	uden = SAFE_AVERAGE(uden1, uden2, uden3);	// Could be safe max.

				float	urad1 = inUrbanRadial.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	urad2 = inUrbanRadial.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	urad3 = inUrbanRadial.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	urad = SAFE_AVERAGE(urad1, urad2, urad3);	// Could be safe max.

				float	utrn1 = inUrbanTransport.value_linear(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float	utrn2 = inUrbanTransport.value_linear(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float	utrn3 = inUrbanTransport.value_linear(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				float	utrn = SAFE_AVERAGE(utrn1, utrn2, utrn3);	// Could be safe max.

				float usq  = usquare.search_nearest(center_x, center_y);
				float usq1 = usquare.search_nearest(tri->vertex(0)->point().x(),tri->vertex(0)->point().y());
				float usq2 = usquare.search_nearest(tri->vertex(1)->point().x(),tri->vertex(1)->point().y());
				float usq3 = usquare.search_nearest(tri->vertex(2)->point().x(),tri->vertex(2)->point().y());
				usq = MAJORITY_RULES(usq, usq1, usq2, usq3);

//				float	el1 = tri->vertex(0)->info().height;
//				float	el2 = tri->vertex(1)->info().height;
//				float	el3 = tri->vertex(2)->info().height;
//				float	el_tri = (el1 + el2 + el3) / 3.0;
				
				float	sl_tri = 1.0 - tri->info().normal[2];

				float	patches = (gMeshPrefs.rep_switch_m == 0.0) ? 100.0 : (60.0 * NM_TO_MTR / gMeshPrefs.rep_switch_m);
				int x_variant = fabs(center_x /*+ RandRange(-0.03, 0.03)*/) * patches; // 25.0;
				int y_variant = fabs(center_y /*+ RandRange(-0.03, 0.03)*/) * patches; // 25.0;
				int variant = ((x_variant + y_variant * 2) % 4) + 1;
				int terrain = FindNaturalTerrain(tri->info().terrain_general, lu, cl, el, sl, sl_tri, tm, tmr, rn, near_water, sh, re, er, uden, urad, utrn, usq, center_y, variant);
				if (terrain == -1)
					AssertPrintf("Cannot find terrain for: %s, %s, %f, %f\n", FetchTokenString(lu), FetchTokenString(cl), el, sl);
				if (terrain == gNaturalTerrainTable.back().name)
				{
					printf("Hit any rule. lu=%s, msl=%f, slope=%f, trislope=%f, temp=%f, temprange=%f, rain=%f, water=%d, heading=%f, lat=%f\n",
						FetchTokenString(lu), el, acos(1-sl)*RAD_TO_DEG, acos(1-sl_tri)*RAD_TO_DEG, tm, tmr, rn, near_water, sh, center_y);
				}
				
				tri->info().terrain_specific = terrain;

			} else {
				// Water case!
				tri->info().terrain_specific = tri->info().terrain_general;
			}
			
		}
#if 0
	OLD LOW RES MESH USE CASE		
		{
			// Lores - assign a texture for orthophotos.
			double	center_x = (tri->vertex(0)->point().x() + tri->vertex(1)->point().x() + tri->vertex(2)->point().x()) / 3.0;
			double	center_y = (tri->vertex(0)->point().y() + tri->vertex(1)->point().y() + tri->vertex(2)->point().y()) / 3.0;
			center_x -= landuse.mWest;
			center_y -= landuse.mSouth;
			center_x /= (landuse.mEast - landuse.mWest);
			center_y /= (landuse.mNorth - landuse.mSouth);
			center_x *= (double) FAR_TEX_DIVISIONS;
			center_y *= (double) FAR_TEX_DIVISIONS;
			tri->info().terrain_general = tri->info().terrain_specific = kFarTextures[((int) center_x) + ((int) center_y) * FAR_TEX_DIVISIONS];
			tri->info().flag = 0;
		}
#endif		
	}


	/***********************************************************************************************
	 * DEAL WITH INTRUSION FROM OUR MASTER SIDE
	 ***********************************************************************************************/

	// This must be POST optmize - we can go OUT OF ORDER on the borders because must have left-master/right-slave.
	// So the optmizer will NUKE this stuff. :-(

	if (!gMatchLeft.vertices.empty())	border_find_edge_tris(ioMesh, gMatchLeft);
	if (!gMatchBottom.vertices.empty())	border_find_edge_tris(ioMesh, gMatchBottom);
	int n;
	int lowest;
	
	// Now we have to "rebase" our edges.  Basically it is possible that we are getting intruded from the left
	// by a lower priority texture.  If we just use borders, that low prio tex will end up UNDER our base, and we'll
	// never see it.  So we need to take the tex on our right side and reduce it.
	for (n = 0; n < gMatchLeft.edges.size(); ++n)
	{
		lowest = gMatchLeft.edges[n].buddy->info().terrain_specific;
		if (LowerPriorityNaturalTerrain(gMatchLeft.edges[n].base, lowest))
			lowest = gMatchLeft.edges[n].base;
		for (set<int>::iterator bl = gMatchLeft.edges[n].borders.begin(); bl != gMatchLeft.edges[n].borders.end(); ++bl)
		{
			if (LowerPriorityNaturalTerrain(*bl, lowest))
				lowest = *bl;
		}
		
		if (lowest != gMatchLeft.edges[n].buddy->info().terrain_specific)
			RebaseTriangle(ioMesh, gMatchLeft.edges[n].buddy, lowest, gMatchLeft.vertices[n].buddy, gMatchLeft.vertices[n+1].buddy);
	}

	for (n = 0; n < gMatchBottom.edges.size(); ++n)
	{
		lowest = gMatchBottom.edges[n].buddy->info().terrain_specific;
		if (LowerPriorityNaturalTerrain(gMatchBottom.edges[n].base, lowest))
			lowest = gMatchBottom.edges[n].base;
		for (set<int>::iterator bl = gMatchBottom.edges[n].borders.begin(); bl != gMatchBottom.edges[n].borders.end(); ++bl)
		{
			if (LowerPriorityNaturalTerrain(*bl, lowest))
				lowest = *bl;
		}
		
		if (lowest != gMatchBottom.edges[n].buddy->info().terrain_specific)
			RebaseTriangle(ioMesh, gMatchBottom.edges[n].buddy, lowest, gMatchBottom.vertices[n].buddy, gMatchBottom.vertices[n+1].buddy);
	}
	

	/***********************************************************************************************
	 * CALCULATE BORDERS
	 ***********************************************************************************************/

	if (inProg) inProg(0, 1, "Assigning Landuses", 0.5);

	/* 	Here's the idea:
		We are going to go through each triangle, which now has a land use, and figure ouet which
		ones have borders.  A triangle that has a border will get: 
		(1) the land use of the border triangle in its set of "border landuses", so it 
		 	can easily be identified in that mesh, and
		(2) for each of its vertices, a hash map entry with the alpha level for the border at that
			point, so we can figure out how  the border fades.
		
		To do this we say: for each triangle, we do a "spreading" type algorithm, e.g. we collect
		non-visited neighbors that meet our criteria in a set and go outward.  We only take neighbors
		that have a lower natural land use and haven't been visited.  We calc our distance to the
		corners to get the blend, and if we're not all faded out, keep going.
	*/

	int		visited = 0;	// flag value - by using a rolling flag, we don't have to reset 
							// this all of the time.
	int		tri_total = 0, tri_border = 0, tri_check = 0, tri_opt = 0;
	for (tri = ioMesh.finite_faces_begin(); tri != ioMesh.finite_faces_end(); ++tri)
	if (tri->info().terrain_general != terrain_Water)
	{
		++visited;
		set<CDT::Face_handle>	to_visit;
		to_visit.insert(tri);
		bool					spread;
		int						layer = tri->info().terrain_specific;
		tri->info().flag = visited;
		
		while (!to_visit.empty())
		{
			CDT::Face_handle	border = *to_visit.begin();
			to_visit.erase(border);
			spread = false;
			if (&*border != &*tri)
			{
				// Calculation phase - figure out alphas of 
				// the corners.
				CDT::Vertex_handle v1 = border->vertex(0);
				CDT::Vertex_handle v2 = border->vertex(1);
				CDT::Vertex_handle v3 = border->vertex(2);
				double	dist1 = DistPtToTri(v1, tri);
				double	dist2 = DistPtToTri(v2, tri);
				double	dist3 = DistPtToTri(v3, tri);
				double	dist_max = GetXonDist(layer, border->info().terrain_specific, border->info().normal[2]);
				
				dist1 = max(0.0, min((dist_max-dist1)/dist_max,1.0));
				dist2 = max(0.0, min((dist_max-dist2)/dist_max,1.0));
				dist3 = max(0.0, min((dist_max-dist3)/dist_max,1.0));
				
				double	odist1 = v1->info().border_blend[layer];
				double	odist2 = v2->info().border_blend[layer];
				double	odist3 = v3->info().border_blend[layer];
				
				++tri_check;
				if (dist1 > 0.0 || dist2 > 0.0 || dist3 > 0.0) 
				{
					// If we're not faded out totally, record an increase.  ONLY keep 	
					// searching if we are increasing one of the vertices.  Otherwise
					// someone else has been over this territory who is already closer
					// and we're just wasting our time.
					if (dist1 > odist1) { spread = true; v1->info().border_blend[layer] = dist1; }
					if (dist2 > odist2) { spread = true; v2->info().border_blend[layer] = dist2; }
					if (dist3 > odist3) { spread = true; v3->info().border_blend[layer] = dist3; }

					// HACK - does always extending the borders fix a bug?
					border->info().terrain_border.insert(layer);
					spread = true;
				}
			} else
				spread = true;
				
			border->info().flag = visited;
			
			// Spreading case: check our neighbors to make sure we haven't seen them and it makes
			// sense to check them.
			if (spread)
			{
				CDT::Face_handle b1 = border->neighbor(0);
				CDT::Face_handle b2 = border->neighbor(1);
				CDT::Face_handle b3 = border->neighbor(2);
				
				if (b1->info().flag != visited && !ioMesh.is_infinite(b1) && b1->info().terrain_general != terrain_Water && LowerPriorityNaturalTerrain(b1->info().terrain_specific, layer))	to_visit.insert(b1);
				if (b2->info().flag != visited && !ioMesh.is_infinite(b2) && b2->info().terrain_general != terrain_Water && LowerPriorityNaturalTerrain(b2->info().terrain_specific, layer))	to_visit.insert(b2);
				if (b3->info().flag != visited && !ioMesh.is_infinite(b3) && b3->info().terrain_general != terrain_Water && LowerPriorityNaturalTerrain(b3->info().terrain_specific, layer))	to_visit.insert(b3);
			}
		}
	}

	/***********************************************************************************************
	 * DEAL WITH INTRUSION FROM OUR MASTER SIDE
	 ***********************************************************************************************/

	for (n = 0; n < gMatchLeft.vertices.size(); ++n)
	for (hash_map<int, float>::iterator blev = gMatchLeft.vertices[n].buddy->info().border_blend.begin(); blev != gMatchLeft.vertices[n].buddy->info().border_blend.end(); ++blev)
		blev->second = 0.0;

	for (n = 0; n < gMatchBottom.vertices.size(); ++n)
	for (hash_map<int, float>::iterator blev = gMatchBottom.vertices[n].buddy->info().border_blend.begin(); blev != gMatchBottom.vertices[n].buddy->info().border_blend.end(); ++blev)
		blev->second = 0.0;

	for (n = 0; n < gMatchLeft.edges.size(); ++n)
	if (gMatchLeft.edges[n].buddy->info().terrain_specific != terrain_Water)
	{
		// Handle the base terrain
		if (gMatchLeft.edges[n].buddy->info().terrain_specific != gMatchLeft.edges[n].base)
		{
			gMatchLeft.edges[n].buddy->info().terrain_border.insert(gMatchLeft.edges[n].base);
			AddZeroMixIfNeeded(gMatchLeft.edges[n].buddy, gMatchLeft.edges[n].base);
			gMatchLeft.vertices[n].buddy->info().border_blend[gMatchLeft.edges[n].base] = 1.0;
			gMatchLeft.vertices[n+1].buddy->info().border_blend[gMatchLeft.edges[n].base] = 1.0;
		}
		
		// Handle any overlay layers...
		for (set<int>::iterator bl = gMatchLeft.edges[n].borders.begin(); bl != gMatchLeft.edges[n].borders.end(); ++bl)
		{
			if (gMatchLeft.edges[n].buddy->info().terrain_specific != *bl)
			{
				gMatchLeft.edges[n].buddy->info().terrain_border.insert(*bl);
				AddZeroMixIfNeeded(gMatchLeft.edges[n].buddy, *bl);
				gMatchLeft.vertices[n].buddy->info().border_blend[*bl] = gMatchLeft.vertices[n].blending[*bl];
				gMatchLeft.vertices[n+1].buddy->info().border_blend[*bl] = gMatchLeft.vertices[n+1].blending[*bl];
			}
		}
	}
	
	for (n = 0; n < gMatchBottom.edges.size(); ++n)
	if (gMatchBottom.edges[n].buddy->info().terrain_specific != terrain_Water)
	{
		// Handle the base terrain
		if (gMatchBottom.edges[n].buddy->info().terrain_specific != gMatchBottom.edges[n].base)
		{
			gMatchBottom.edges[n].buddy->info().terrain_border.insert(gMatchBottom.edges[n].base);
			AddZeroMixIfNeeded(gMatchBottom.edges[n].buddy, gMatchBottom.edges[n].base);
			gMatchBottom.vertices[n].buddy->info().border_blend[gMatchBottom.edges[n].base] = 1.0;
			gMatchBottom.vertices[n+1].buddy->info().border_blend[gMatchBottom.edges[n].base] = 1.0;
		}
		
		// Handle any overlay layers...
		for (set<int>::iterator bl = gMatchBottom.edges[n].borders.begin(); bl != gMatchBottom.edges[n].borders.end(); ++bl)
		{
			if (gMatchBottom.edges[n].buddy->info().terrain_specific != *bl)
			{
				gMatchBottom.edges[n].buddy->info().terrain_border.insert(*bl);
				AddZeroMixIfNeeded(gMatchBottom.edges[n].buddy, *bl);
				gMatchBottom.vertices[n].buddy->info().border_blend[*bl] = gMatchBottom.vertices[n].blending[*bl];
				gMatchBottom.vertices[n+1].buddy->info().border_blend[*bl] = gMatchBottom.vertices[n+1].blending[*bl];
			}
		}
	
	}

	/***********************************************************************************************
	 * OPTIMIZE BORDERS!
	 ***********************************************************************************************/
	if (inProg) inProg(0, 1, "Assigning Landuses", 0.75);

	{
		for (tri = ioMesh.finite_faces_begin(); tri != ioMesh.finite_faces_end(); ++tri)
		if (tri->info().terrain_general != terrain_Water)
		{
			bool need_optimize = false;
			for (set<int>::iterator blayer = tri->info().terrain_border.begin();
				blayer != tri->info().terrain_border.end(); ++blayer)
			{
				if (tri->vertex(0)->info().border_blend[*blayer] == 1.0 &&
					tri->vertex(1)->info().border_blend[*blayer] == 1.0 &&
					tri->vertex(2)->info().border_blend[*blayer] == 1.0)
				{
					if (LowerPriorityNaturalTerrain(tri->info().terrain_specific, *blayer))
					{
						tri->info().terrain_specific = *blayer;
						need_optimize = true;
					}
				}
			}
			if (need_optimize)
			{
				set<int>	nuke;
				for (set<int>::iterator blayer = tri->info().terrain_border.begin();
					blayer != tri->info().terrain_border.end(); ++blayer)
				{
					if (!LowerPriorityNaturalTerrain(tri->info().terrain_specific, *blayer))
						nuke.insert(*blayer);
				}
				for (set<int>::iterator nlayer = nuke.begin(); nlayer != nuke.end(); ++nlayer)
				{
					tri->info().terrain_border.erase(*nlayer);
					// DO NOT eliminate these - maybe our neighbor is using them!!
//					tri->vertex(0)->info().border_blend.erase(*nlayer);
//					tri->vertex(1)->info().border_blend.erase(*nlayer);
//					tri->vertex(2)->info().border_blend.erase(*nlayer);
					++tri_opt;
				}				
			}
		}
	}

	{
		for (tri = ioMesh.finite_faces_begin(); tri != ioMesh.finite_faces_end(); ++tri)
		if (tri->info().terrain_general != terrain_Water)
		{
			tri_total++;
			tri_border += (tri->info().terrain_border.size());
		} else if (!tri->info().terrain_border.empty())
			AssertPrintf("BORDER ON NON-NATURAL LAND USE!");
		printf("Total: %d - border: %d - check: %d - opt: %d\n", tri_total, tri_border, tri_check, tri_opt);
	}

	
	
	/***********************************************************************************************
	 * WRITE OUT MESH
	 ***********************************************************************************************/
	
	// We need to write out an edge file for our next guy in line.

	if (gMeshPrefs.border_match)
	{
		double	west = inElevation.mWest;
		double	east = inElevation.mEast;
		double	south = inElevation.mSouth;
		double	north = inElevation.mNorth;
		char	fname[32];
		sprintf(fname, "%+03d%+04d.border.txt", (int) south, (int) west);
		
		FILE * border = fopen(fname, "w");
		
		CDT::Point	cur(west,north), stop(east, north);
	
		CDT::Face_handle	f;
		int					i;
		CDT::Locate_type	lt;
		f = ioMesh.locate(cur, lt, i);
		Assert(lt == CDT::VERTEX);
		
		do {
			fprintf(border, "VT %.12lf, %.12lf, %lf\n", 
				CGAL::to_double(f->vertex(i)->point().x()),
				CGAL::to_double(f->vertex(i)->point().y()),
				CGAL::to_double(f->vertex(i)->info().height));
			fprintf(border, "VBC %d\n", f->vertex(i)->info().border_blend.size());
			for (hash_map<int, float>::iterator hfi = f->vertex(i)->info().border_blend.begin(); hfi != f->vertex(i)->info().border_blend.end(); ++hfi)
				fprintf(border, "VB %f %s\n", hfi->second, FetchTokenString(hfi->first));

			FindNextEast(ioMesh, f, i);
			DebugAssert(!ioMesh.is_infinite(f));

			fprintf(border, "TERRAIN %s\n", FetchTokenString(f->info().terrain_specific));
			fprintf(border, "BORDER_C %d\n", f->info().terrain_border.size());
			for (set<int>::iterator si = f->info().terrain_border.begin(); si != f->info().terrain_border.end(); ++si)
				fprintf(border, "BORDER_T %s\n", FetchTokenString(*si));
			
		} while (f->vertex(i)->point() != stop);
		
		fprintf(border, "VC %.12lf, %.12lf, %lf\n", 
				CGAL::to_double(f->vertex(i)->point().x()),
				CGAL::to_double(f->vertex(i)->point().y()),
				CGAL::to_double(f->vertex(i)->info().height));
		fprintf(border, "VBC %d\n", f->vertex(i)->info().border_blend.size());
		for (hash_map<int, float>::iterator hfi = f->vertex(i)->info().border_blend.begin(); hfi != f->vertex(i)->info().border_blend.end(); ++hfi)
			fprintf(border, "VB %f %s\n", hfi->second, FetchTokenString(hfi->first));

		
		stop = CDT::Point(east, south);

		do {
			FindNextSouth(ioMesh, f, i);
			DebugAssert(!ioMesh.is_infinite(f));
			fprintf(border, "TERRAIN %s\n", FetchTokenString(f->info().terrain_specific));
			fprintf(border, "BORDER_C %d\n", f->info().terrain_border.size());
			for (set<int>::iterator si = f->info().terrain_border.begin(); si != f->info().terrain_border.end(); ++si)
				fprintf(border, "BORDER_T %s\n", FetchTokenString(*si));
		
			fprintf(border, "VR %.12lf, %.12lf, %lf\n", 
				CGAL::to_double(f->vertex(i)->point().x()),
				CGAL::to_double(f->vertex(i)->point().y()),
				CGAL::to_double(f->vertex(i)->info().height));
			fprintf(border, "VBC %d\n", f->vertex(i)->info().border_blend.size());
			for (hash_map<int, float>::iterator hfi = f->vertex(i)->info().border_blend.begin(); hfi != f->vertex(i)->info().border_blend.end(); ++hfi)
				fprintf(border, "VB %f %s\n", hfi->second, FetchTokenString(hfi->first));

		} while (f->vertex(i)->point() != stop);

		fprintf(border, "END\n");
		fclose(border);

	}

	if (inProg) inProg(0, 1, "Assigning Landuses", 1.0);
	
	
	
}
		


/*******************************************************************************************
 *	UTILITY ROUTINES
 *******************************************************************************************/
void SetupWaterRasterizer(const Pmwx& map, const DEMGeo& orig, PolyRasterizer& rasterizer)
{
	for (Pmwx::Halfedge_const_iterator i = map.halfedges_begin(); i != map.halfedges_end(); ++i)
	{
		if (i->mDominant)
		{
			bool	iWet = i->face()->IsWater() && !i->face()->is_unbounded();
			bool	oWet = i->twin()->face()->IsWater() && !i->twin()->face()->is_unbounded();
			
			if (iWet != oWet)
			{
				double x1 = orig.lon_to_x(i->source()->point().x);
				double y1 = orig.lat_to_y(i->source()->point().y);
				double x2 = orig.lon_to_x(i->target()->point().x);
				double y2 = orig.lat_to_y(i->target()->point().y);
									
//				gMeshLines.push_back(i->source()->point());
//				gMeshLines.push_back(i->target()->point());				
				
//				fprintf(fi,"%lf,%lf    %lf,%lf   %s\n", x1,y1,x2,y2, ((y1 == 0.0 || y2 == 0.0) && y1 != y2) ? "****" : "");
				
				if (y1 != y2)
				{
					if (y1 < y2)
						rasterizer.masters.push_back(PolyRasterSeg_t(x1,y1,x2,y2));
					else
						rasterizer.masters.push_back(PolyRasterSeg_t(x2,y2,x1,y1));
				} 
			}
		}
	}
//	fclose(fi);
	
	rasterizer.SortMasters();
}

void	Calc2ndDerivative(DEMGeo& deriv)
{
	int x, y;
	float 	h, ha, hr, hb, hl;
	for (y = 0; y < (deriv.mHeight-1); ++y)
	for (x = 0; x < (deriv.mWidth-1); ++x)
	{
		h  = deriv(x,y);
		ha = deriv(x,y+1);
		hr = deriv(x+1,y);
		
		if (h == NO_DATA || ha == NO_DATA || hr == NO_DATA)
			deriv(x,y) = NO_DATA;
		else
			deriv(x,y) = (ha - h) + (hr - h);
	}

	for (y = (deriv.mHeight-2); y >= 1; --y)
	for (x = (deriv.mWidth-2);  x >= 1; --x)
	{
		h  = deriv(x,y);
		hb = deriv(x,y-1);
		hl = deriv(x-1,y);
		
		if (h == NO_DATA || hb == NO_DATA || hl == NO_DATA)
			deriv(x,y) = NO_DATA;
		else
			deriv(x,y) = (h - hl) + (h - hb);
	}
	
	for (x = 0; x < deriv.mWidth; ++x)
	{
		deriv(x, 0) = NO_DATA;
		deriv(x, deriv.mHeight-1) = NO_DATA;
	}
	for (x = y; y < deriv.mHeight; ++y)
	{
		deriv(0, y) = NO_DATA;
		deriv(deriv.mWidth-1, y) = NO_DATA;
	}
}

double	HeightWithinTri(CDT::Face_handle f, double inLon, double inLat)
{
	Point3	p1(CGAL::to_double(f->vertex(0)->point().x()),
			   CGAL::to_double(f->vertex(0)->point().y()),
				 			   f->vertex(0)->info().height);

	Point3	p2(CGAL::to_double(f->vertex(1)->point().x()),
			   CGAL::to_double(f->vertex(1)->point().y()),
				 			   f->vertex(1)->info().height);

	Point3	p3(CGAL::to_double(f->vertex(2)->point().x()),
			   CGAL::to_double(f->vertex(2)->point().y()),
				 			   f->vertex(2)->info().height);

	Vector3	s1(p2, p3);
	Vector3	s2(p2, p1);
	Vector3	n = s1.cross(s2);
	Plane3	plane;
	plane.n = n;
	plane.ndotp = n.dot(Vector3(p1));
	return	(-(n.dx * inLon + n.dy * inLat) + plane.ndotp) / n.dz;

}


double	MeshHeightAtPoint(CDT& inMesh, double inLon, double inLat, int hint_id)
{
	if (inMesh.number_of_faces() < 1) return NO_DATA;
	CDT::Face_handle	f = NULL;
	int	n;
	CDT::Locate_type lt;
	f = inMesh.locate_cache(CDT::Point(inLon, inLat), lt, n, hint_id);
	if (lt == CDT::VERTEX)
	{
		return f->vertex(n)->info().height;
	}
	if (lt == CDT::EDGE && inMesh.is_infinite(f))
	{
		f = f->neighbor(n);
	}
	
	if (!inMesh.is_infinite(f))
	{
		return HeightWithinTri(f, inLon, inLat);
	} else {
		printf("Requested point was off mesh: %lf, %lf\n", inLon, inLat);
		return NO_DATA;
	}
}




#define 	CLAMP_EPSILON	0.001

static void SlightClampToDEM(Point2& ioPoint, const DEMGeo& ioDEM)
{
	double	wicked_north = ioDEM.mNorth + CLAMP_EPSILON;
	double	wicked_south = ioDEM.mSouth - CLAMP_EPSILON;
	double	wicked_east = ioDEM.mEast + CLAMP_EPSILON;
	double	wicked_west = ioDEM.mWest - CLAMP_EPSILON;
	if (ioPoint.y > wicked_north ||
		ioPoint.y < wicked_south ||
		ioPoint.x > wicked_east ||
		ioPoint.x < wicked_west)
	{
		printf("WARNING: Point is way outside DEM.  Will probably cause a leak.\n");
	} else {
		if (ioPoint.x > ioDEM.mEast)	ioPoint.x = ioDEM.mEast;
		if (ioPoint.x < ioDEM.mWest)	ioPoint.x = ioDEM.mWest;
		if (ioPoint.y > ioDEM.mNorth)	ioPoint.y = ioDEM.mNorth;
		if (ioPoint.y < ioDEM.mSouth)	ioPoint.y = ioDEM.mSouth;
	}
}

#undef CLAMP_EPSILON

int	CalcMeshError(CDT& mesh, DEMGeo& elev, map<float, int>& err, ProgressFunc inFunc)
{
	if (inFunc) inFunc(0, 1, "Calculating Error", 0.0);
	int hint = CDT::gen_cache_key();
	err.clear();
	int ctr = 0;
	for (int y = 0; y < elev.mHeight; ++y)
	{
		if (inFunc && (y % 20) == 0) inFunc(0, 1, "Calculating Error", (float) y / (float) elev.mHeight);

		for (int x = 0; x < elev.mWidth ; ++x)
		{
			float ideal = elev.get(x,y);
			if (ideal != NO_DATA)
			{
				float real = MeshHeightAtPoint(mesh, elev.x_to_lon(x), elev.y_to_lat(y), hint);
				if (real != NO_DATA)
				{
					float derr = real - ideal;
					err[derr]++;
					++ctr;
				}
			}
		}
	}
	if (inFunc) inFunc(0, 1, "Calculating Error", 1.0);
	return ctr;
}

static bool RayInTri(CDT::Face_handle tri, CDT::Vertex_handle v, const CDT::Point& goal)
{
	CDT::Orientation_2 pred;
	
	CDT::Vertex_handle	v_cw =  tri->vertex(CDT::cw (tri->index(v)));
	CDT::Vertex_handle	v_ccw = tri->vertex(CDT::ccw(tri->index(v)));
	
	if (pred(v->point(),  v_cw->point(), goal) == CGAL::LEFT_TURN ) return false;
	if (pred(v->point(), v_ccw->point(), goal) == CGAL::RIGHT_TURN) return false;
																	 return true;	
}

bool common_vertex(CDT::Face_handle t1, CDT::Face_handle t2, int& index)
{
	if (t2->has_vertex(t1->vertex(0))) { index = 0; return true; }
	if (t2->has_vertex(t1->vertex(1))) { index = 1; return true; }
	if (t2->has_vertex(t1->vertex(2))) { index = 2; return true; }
	return false;
}



CDT_MarchOverTerrain_t::CDT_MarchOverTerrain_t() 
{
	locate_face = NULL;
}
	
void MarchHeightStart(CDT& inMesh, const CDT::Point& loc, CDT_MarchOverTerrain_t& info)
{
	CDT::Locate_type	locate_type;
	int					locate_index;

	info.locate_face = inMesh.locate(loc, locate_type, locate_index, info.locate_face);
	info.locate_pt = loc;
	info.locate_height = HeightWithinTri(info.locate_face, loc.x(), loc.y());
	
	// Special case: under some conditions we'll get the infinite-face edge.  This actually depends
	// on what our seed locate was.  Either way it is unacceptable - passing in an infinite face
	// generally makes the locate algorithm a little bonkers.  Reverse it here.
	if (inMesh.is_infinite(info.locate_face) && locate_type == CDT::EDGE)
		info.locate_face = info.locate_face->neighbor(locate_index);
}

void  MarchHeightGo(CDT& inMesh, const CDT::Point& goal, CDT_MarchOverTerrain_t& march_info, vector<Point3>& intermediates)
{	
	static int level = 0;
	Assert(level < 2);
	
	// Makse sure our input makes some sense!
	DebugAssert(!inMesh.is_infinite(march_info.locate_face));
	DebugAssert(inMesh.triangle(march_info.locate_face).bounded_side(march_info.locate_pt) != CGAL::ON_UNBOUNDED_SIDE);
	
	intermediates.clear();

	
	CDT::Line_face_circulator circ(inMesh.line_walk(march_info.locate_pt, goal, march_info.locate_face));
	CDT::Line_face_circulator stop(circ);

	if (circ == CDT::Face_handle(NULL))
	{
		CDT::Locate_type	goal_type;
		int					goal_index;
		CDT::Face_handle	goal_face;
		CDT::Point			rev_goal = march_info.locate_pt;
		goal_face = inMesh.locate(goal, goal_type, goal_index, march_info.locate_face);
		if (inMesh.is_infinite(goal_face) && goal_type == CDT::EDGE)
			goal_face = goal_face->neighbor(goal_index);

		double				goal_height = HeightWithinTri(goal_face, goal.x(), goal.y());

		march_info.locate_pt = goal;
		march_info.locate_face = goal_face;
		march_info.locate_height = goal_height;
		
		++level;
		MarchHeightGo(inMesh, rev_goal, march_info, intermediates);
		--level;

		march_info.locate_pt = goal;
		march_info.locate_face = goal_face;
		march_info.locate_height = goal_height;
		
		int s = intermediates.size() / 2;
		for (int n = 0; n < s; ++n)
		{
			swap(intermediates[n], intermediates[intermediates.size() - n - 1]);
		}
		DebugAssert(!inMesh.is_infinite(march_info.locate_face));
		DebugAssert(inMesh.triangle(march_info.locate_face).bounded_side(march_info.locate_pt) != CGAL::ON_UNBOUNDED_SIDE);
		return;
	}

	intermediates.push_back(Point3(march_info.locate_pt.x(), march_info.locate_pt.y(), march_info.locate_height));

	CDT::Segment	ray(march_info.locate_pt, goal);
	int				cross_side;

	CDT::Geom_traits::Orientation_2 pred;
		
	while (1)
	{
		CDT::Point	last_pt;
		double		last_ht;
	
		CDT::Face_handle now = circ;
		++circ;
		CDT::Face_handle next = circ;
		
		if (!inMesh.is_infinite(now) && inMesh.triangle(now).bounded_side(goal) != CGAL::ON_UNBOUNDED_SIDE)
		{
			march_info.locate_pt = last_pt = goal;
			march_info.locate_height = last_ht = HeightWithinTri(now, goal.x(), goal.y());
			march_info.locate_face = now;
			intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));		
			DebugAssert(!inMesh.is_infinite(march_info.locate_face));
			DebugAssert(inMesh.triangle(march_info.locate_face).bounded_side(march_info.locate_pt) != CGAL::ON_UNBOUNDED_SIDE);	
			break;
		}
		
		if (now->has_neighbor(next))
		{
			cross_side = now->index(next);
			CDT::Segment crossed_seg = inMesh.segment(CDT::Edge(now, cross_side));

			CGAL::Orientation o1 = pred(ray.source(), ray.target(), crossed_seg.source());
			CGAL::Orientation o2 = pred(ray.source(), ray.target(), crossed_seg.target());

			// We can't both be any one value - that means the common side is on both tris - 
			// one tri shouldn't be in the iteration!
			DebugAssert(o1 != o2);
			
			if (o1 == CGAL::COLLINEAR)
			{
				last_pt = now->vertex(CDT::ccw(cross_side))->point();
				last_ht = now->vertex(CDT::ccw(cross_side))->info().height;
				intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));			
			} else if (o2 == CGAL::COLLINEAR)
			{
				last_pt = now->vertex(CDT::cw(cross_side))->point();
				last_ht = now->vertex(CDT::cw(cross_side))->info().height;
				intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));
			
			} else {		
				CGAL::Object o = CGAL::intersection(ray, crossed_seg);
				if (CGAL::assign(last_pt, o))
				{
					last_ht = HeightWithinTri(now, last_pt.x(), last_pt.y());
					intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));
				} else {
#if DEV				
					printf("Ray: %lf,%lf->%lf,%lf\nSide: %lf,%lf->%lf,%lf\n",
						ray.source().x(), ray.source().y(), 
						ray.target().x(), ray.target().y(), 
						crossed_seg.source().x(), crossed_seg.source().y(), 
						crossed_seg.target().x(), crossed_seg.target().y());
#endif						
					AssertPrintf("Intersection failed.");
				}
			}
		} 
		else if (common_vertex(now, next, cross_side))
		{
			last_pt = now->vertex(cross_side)->point();
			last_ht = now->vertex(cross_side)->info().height;
			intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));
		} else
			AssertPrintf("Cannot determine relationship between triangles!");
		
		// If we hit our goal dead-on, great!
		if (last_pt == goal)
		{
			march_info.locate_pt = last_pt;
			march_info.locate_height = last_ht;
			march_info.locate_face = next;
			DebugAssert(!inMesh.is_infinite(march_info.locate_face));
			DebugAssert(inMesh.triangle(march_info.locate_face).bounded_side(march_info.locate_pt) != CGAL::ON_UNBOUNDED_SIDE);	
			break;
		}
		
/*		
		// VERY STRANGE: given a simple horizontal line case, collinear_has_on is returning CRAP results.  
		if (!ray.collinear_has_on(last_pt))
		{
			intermediates.pop_back();
			march_info.locate_pt = last_pt = goal;
			march_info.locate_height = last_ht = HeightWithinTri(now, goal.x(), goal.y());
			march_info.locate_face = now;
			intermediates.push_back(Point3(last_pt.x(), last_pt.y(), last_ht));
			DebugAssert(!inMesh.is_infinite(march_info.locate_face));
			DebugAssert(inMesh.triangle(march_info.locate_face).bounded_side(march_info.locate_pt) != CGAL::ON_UNBOUNDED_SIDE);	
			break;
		}
*/		
		DebugAssert(circ != stop);
	}
}
