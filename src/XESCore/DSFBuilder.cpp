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
#include "NetPlacement.h"
#include "MeshAlgs.h"
#include "TriFan.h"
#include "DEMAlgs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "AssertUtils.h"

#include "PerfUtils.h"

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


#define		PATCH_DIM_HI	32
#define		PATCH_DIM_LO	32
#define		TERRAIN_NEAR_LOD 		0.0
#define		TERRAIN_FAR_LOD			-1.0
#define		ORTHO_NEAR_LOD			100000.0
#define		ORTHO_FAR_LOD			-1.0
#define		MAX_TRIS_PER_PATCH		85

#define		NO_ORTHO 1
#define		NO_BORDERS 0
#define 	TEST_FORESTS 0

// We have to transform generic/specific int pair land uses into one numbering system and back!

// Edge-wrapper...turns out CDT::Edge is so deeply templated that stuffing it in a map crashes CW8.
// So we build a dummy wrapper around an edge to prevent a template with a huge expanded name.
struct	edge_wrapper {
	edge_wrapper() : edge(NULL, 0) { };
	edge_wrapper(const CDT::Edge e) : edge(e) { };
	edge_wrapper(const edge_wrapper& x) : edge(x.edge) { }
	edge_wrapper(CDT::Face_handle f, int i) : edge(CDT::Edge(f, i)) { }
	edge_wrapper& operator=(const edge_wrapper& x) { edge = x.edge; return *this; }
	bool operator==(const edge_wrapper& x) const { return edge == x.edge; }
	bool operator!=(const edge_wrapper& x) const { return edge != x.edge; }
	bool operator<(const edge_wrapper& x) const { return edge < x.edge; } 

	CDT::Edge	edge;
};

// Given a beach edge, fetch the beach-type coords.  last means use the target rather than src pt.
static void BeachPtGrab(const edge_wrapper& edge, bool last, const CDT& inMesh, double coords[6], int kind)
{
	CDT::Face_circulator stop, circ;
//	int	lterrain = NO_VALUE;
	
//	GISVertex * pm_vs = pm_edge->source();
//	GISVertex * pm_vt = pm_edge->target();
	
//	loc = inMesh.locate(CDT::Point(pm_vs->point().x, pm_vs->point().y), lt, i, loc);
//	Assert(lt == CDT::VERTEX);
	CDT::Vertex_handle v_s = edge.edge.first->vertex(CDT::ccw(edge.edge.second));
//	loc = inMesh.locate(CDT::Point(pm_vt->point().x, pm_vt->point().y), lt, i, loc);
//	Assert(lt == CDT::VERTEX);
	CDT::Vertex_handle v_t = edge.edge.first->vertex(CDT::cw(edge.edge.second));

	if (last)
	{
		coords[0] = v_t->point().x();
		coords[1] = v_t->point().y();
		coords[2] = v_t->info().height;
	} else {
		coords[0] = v_s->point().x();
		coords[1] = v_s->point().y();
		coords[2] = v_s->info().height;
	}
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
	coords[5] = kind;
}

#define	INLAND_BLEND_DIST 5.0

static double GetWaterBlend(CDT::Vertex_handle v_han, const DEMGeo& dem)
{
	double lon, lat;
	lon = CGAL::to_double(v_han->point().x());
	lat = CGAL::to_double(v_han->point().y());
	
	
	float ret = dem.value_linear(lon, lat);
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
	int proj = gNaturalTerrainTable[gNaturalTerrainIndex[terrain]].proj_angle;
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
	smallest_dot = max(0.0, smallest_dot);				// must be non-negative!
	smallest_dot = acos(smallest_dot) / (PI / 2.0);
	return smallest_dot;	
}

// Given an edge, finds the next edge clockwise from the source vertex
// of this edge.  (Pmwx equivalent is twin->next
edge_wrapper edge_twin_next(const edge_wrapper& e)
{
	edge_wrapper new_e;
	int center_index = CDT::ccw(e.edge.second);
	CDT::Vertex_handle center = e.edge.first->vertex(center_index);

	new_e.edge.first = e.edge.first->neighbor(e.edge.second);
	new_e.edge.second = CDT::cw(new_e.edge.first->index(center));
	return new_e;
}

// Given an edge, find the next edge in a clockwise circulation
// around its target vertex.  (Pmwx equivalent is next->twin)
edge_wrapper edge_next_twin(const edge_wrapper& e)
{
	edge_wrapper new_e;

	new_e.edge.first = e.edge.first->neighbor(CDT::ccw(e.edge.second));
	new_e.edge.second = CDT::cw(new_e.edge.first->index(e.edge.first->vertex(e.edge.second)));
	return new_e;
}


// Given an edge, find the leftmost turn connected to us.  (pmwx equivalent is next)
edge_wrapper edge_next(const edge_wrapper& e)
{
	edge_wrapper new_e(e);
	new_e.edge.second = CDT::ccw(new_e.edge.second);
	return new_e;
}

// Find the edge in opposite direction (pmwx version is twin)
edge_wrapper edge_twin(const edge_wrapper& e)
{
	edge_wrapper new_e(e);
	CDT::Vertex_handle v = e.edge.first->vertex(CDT::ccw(e.edge.second));
	new_e.edge.first = e.edge.first->neighbor(e.edge.second);
	new_e.edge.second = CDT::ccw(new_e.edge.first->index(v));
	return new_e;
}

int is_coast(const edge_wrapper& inEdge, const CDT& inMesh)
{
	if (inMesh.is_infinite(inEdge.edge.first)) return false;
	if (inMesh.is_infinite(inEdge.edge.first->neighbor(inEdge.edge.second))) return false;

	if (inEdge.edge.first->info().terrain != terrain_Water) return false;
	if (inEdge.edge.first->neighbor(inEdge.edge.second)->info().terrain == terrain_Water) return false;	
	return true;
}

double edge_angle(const edge_wrapper& e1, const edge_wrapper& e2)
{
	CDT::Vertex_handle	e1s = e1.edge.first->vertex(CDT::ccw(e1.edge.second));
	CDT::Vertex_handle	e1t = e1.edge.first->vertex(CDT:: cw(e1.edge.second));

	CDT::Vertex_handle	e2s = e2.edge.first->vertex(CDT::ccw(e2.edge.second));
	CDT::Vertex_handle	e2t = e2.edge.first->vertex(CDT:: cw(e2.edge.second));
	
	DebugAssert(e1t == e2s);
	
	Point2	p1(e1s->point().x(),e1s->point().y());
	Point2	p2(e1t->point().x(),e1t->point().y());
	Point2	p3(e2t->point().x(),e2t->point().y());
	
	Vector2	v1(p1,p2);
	Vector2 v2(p2,p3);
	v1.normalize();
	v2.normalize();
	return v1.dot(v2);
}


int	has_beach(const edge_wrapper& inEdge, const CDT& inMesh, int& kind)
{
	if (!is_coast(inEdge, inMesh))	return false;

	CDT::Face_handle tri = inEdge.edge.first;
	if (tri->info().terrain == terrain_Water)
		tri = inEdge.edge.first->neighbor(inEdge.edge.second);

	int lterrain = tri->info().terrain;
	int i;	
	
	CDT::Vertex_handle v_s = inEdge.edge.first->vertex(CDT::ccw(inEdge.edge.second));
	CDT::Vertex_handle v_t = inEdge.edge.first->vertex(CDT::cw(inEdge.edge.second));
	
	double	prev_ang = 1.0, next_ang = 1.0;
	
	// Find our outgoing (next) angle
	for (edge_wrapper iter = edge_next(inEdge); iter != edge_twin(inEdge); iter = edge_twin_next(iter))
	if (is_coast(iter, inMesh))
	{
		next_ang = edge_angle(inEdge, iter);
		break;
	}

	// Find our incoming (previous) angle	
	for (edge_wrapper iter = edge_next_twin(edge_twin(inEdge)); iter != edge_twin(inEdge); iter = edge_next_twin(iter))
	if (is_coast(iter, inMesh))
	{
		prev_ang = edge_angle(iter, inEdge);
	}

	double		wave = (v_s->info().wave_height + v_t->info().wave_height) * 0.5;
	double		len = LonLatDistMeters(v_s->point().x(),v_s->point().y(),
									   v_t->point().x(),v_t->point().y());
									   
	double slope = tri->info().normal[2];
	
	for (i = 0; i < gBeachInfoTable.size(); ++i)
	{
		if (slope >= gBeachInfoTable[i].min_slope && 
			slope <= gBeachInfoTable[i].max_slope &&
			wave >= gBeachInfoTable[i].min_sea &&
			wave <= gBeachInfoTable[i].max_sea &&
			prev_ang >= gBeachInfoTable[i].max_turn &&
			next_ang >= gBeachInfoTable[i].max_turn)
			
			// TODO 
//			min_len
//			min_lat/max_lat
//			min_rain/max_rain
//			min_temp/max_temp
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

string		get_terrain_name(int composite) 
{
	if (composite == terrain_Water)
		return FetchTokenString(composite);
	else if (gNaturalTerrainIndex.count(composite) > 0)
	{
		return string("lib/g8/") + FetchTokenString(composite) + ".ter";
	}

//	DebugAssert(!"bad terrain.");
	AssertPrintf("WARNING: no name for terrain %d (token=%s\n", composite, FetchTokenString(composite));
	return "UNKNOWN TERRAIN!";
}

struct SortByLULayer {
	bool operator()(const int& lhs, const int& rhs) const {
		if (lhs >= terrain_Natural && rhs >= terrain_Natural)
			return LowerPriorityNaturalTerrain(lhs,rhs);
		return lhs < rhs;
	}
};

struct	StNukeWriter {
	StNukeWriter(void * writer) : writer_(writer) { }
	~StNukeWriter() { if (writer_) DSFDestroyWriter(writer_); }
	void * writer_;
};




struct hash_edge {
	typedef edge_wrapper		KeyType;
	// Trick: we think most ptrs are 4-byte aligned - reuse lower 2 bits.
	size_t operator()(const KeyType& key) const { return (size_t) &*key.edge.first + (size_t) key.edge.second; }
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



void	BuildDSF(
			const char *	inFileName,
			const DEMGeo&	inLanduse,
//			const DEMGeo&	inVegeDem,
			CDT&			inHiresMesh,
//			CDT&			inLoresMesh,
			Pmwx&			inVectorMap,
			ProgressFunc	inProgress)
{
vector<CDT::Face_handle>	sHiResTris[PATCH_DIM_HI * PATCH_DIM_HI];
vector<CDT::Face_handle>	sLoResTris[PATCH_DIM_LO * PATCH_DIM_LO];
set<int>					sHiResLU[PATCH_DIM_HI * PATCH_DIM_HI];
set<int>					sHiResBO[PATCH_DIM_HI * PATCH_DIM_HI];
set<int>					sLoResLU[PATCH_DIM_LO * PATCH_DIM_LO];


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
		double	coords2[2];
		double	coords6[6];
		double	coords8[8];
		double							x, y, v;
		CDT::Finite_faces_iterator		fi;
		CDT::Face_handle				f;
		CDT::Vertex_handle				avert;
		CDT::Finite_vertices_iterator	vert;
		map<int, int, SortByLULayer>::iterator 		lu_ranked;
		map<int, int>::iterator 		lu;
		map<int, int>::iterator 		obdef;
		set<int>::iterator				border_lu;
		bool							is_water;
		list<CDT::Face_handle>::iterator nf;

		Pmwx::Face_iterator						pf;
		GISObjPlacementVector::iterator			pointObj;
		GISPolyObjPlacementVector::iterator		polyObj;
		Polygon2::iterator						polyPt;
		vector<Polygon2>::iterator				polyRing;

		void *			writer;
		DSFCallbacks_t	cbs;
		
		Net_JunctionInfoSet				junctions;
		Net_ChainInfoSet				chains;
		Net_JunctionInfoSet::iterator 	ji;		
		Net_ChainInfoSet::iterator 		ci;
		vector<Point3>::iterator		shapePoint;
		
		map<int, int, SortByLULayer>landuses;
		map<int, int>				landuses_reversed;	
		map<int, int>	objects,	objects_reversed;
		map<int, int>	facades,	facades_reversed;

		char	prop_buf[256];

	/***********************************************************************************************
	 * PICK UP 2-D VALUES
	 ***********************************************************************************************/	
	DEMGeo	waterType(inLanduse);
	for (y = 0; y < waterType.mHeight;++y)
	for (x = 0; x < waterType.mWidth; ++x)
	{
		if (waterType(x,y) != lu_usgs_SEA_WATER && waterType(x,y) != lu_usgs_INLAND_WATER)
			waterType(x,y) = NO_VALUE;
	}
	
	{
		DEMGeo temp(waterType);
		for (y = 0; y < waterType.mHeight;++y)
		for (x = 0; x < waterType.mWidth; ++x)
		{
			float e = temp.get_radial(x,y,2, NO_VALUE);
			waterType(x,y) = (e == lu_usgs_SEA_WATER) ? 1.0 : 0.0;
			
		}
	}

	// Ben sez: this is about the minimum smeaer we can do without seeing spiky triangular marks
	// in the water depth info...this is because we have to blend enough that a tiny triangle
	// never goes from full to min blend fast.
	#define SMEAR_SIZE_WATER 3

	float	smear[SMEAR_SIZE_WATER*SMEAR_SIZE_WATER];
	CalculateFilter(SMEAR_SIZE_WATER,smear,demFilter_Spread,true);
	waterType.filter_self(SMEAR_SIZE_WATER,smear);	
	for (y = 0; y < waterType.mHeight;++y)
	for (x = 0; x < waterType.mWidth; ++x)
	{
		waterType(x,y) = min(max(waterType(x,y), 0.0f), 1.0f);
	}

//	for (vert = inHiresMesh.finite_vertices_begin(); vert != inHiresMesh.finite_vertices_end(); ++vert)
//	{
//	 	vert->info().vege_density = inVegeDem.value_linear(vert->point().x(), vert->point().y());
//	 	if (vert->info().vege_density > 1.0) vert->info().vege_density = 1.0;
//	 	if (vert->info().vege_density < 0.0) vert->info().vege_density = 0.0;
//	}
		
	/****************************************************************
	 * SETUP
	 ****************************************************************/	
	 
	writer = DSFCreateWriter(inLanduse.mWest, inLanduse.mSouth, inLanduse.mEast, inLanduse.mNorth, 8);
	StNukeWriter	dontLeakWriter(writer);
	DSFGetWriterCallbacks(&cbs);
	 
	/****************************************************************
	 * MESH GENERATION
	 ****************************************************************/

	// First assign IDs to each triangle to differentiate patches.
	// Also work out land uses.
	
	if (inProgress && inProgress(0, 5, "Compiling Mesh", 0.0)) return;	

	for (fi = inHiresMesh.finite_faces_begin(); fi != inHiresMesh.finite_faces_end(); ++fi)
	{
		fi->info().flag = 0;
		
		if (fi->vertex(0)->point().y() >= inLanduse.mNorth &&
			fi->vertex(1)->point().y() >= inLanduse.mNorth &&		
			fi->vertex(2)->point().y() >= inLanduse.mNorth)			continue;
			
		if (fi->vertex(0)->point().y() <= inLanduse.mSouth &&
			fi->vertex(1)->point().y() <= inLanduse.mSouth &&		
			fi->vertex(2)->point().y() <= inLanduse.mSouth)			continue;

		if (fi->vertex(0)->point().x() >= inLanduse.mEast &&
			fi->vertex(1)->point().x() >= inLanduse.mEast &&		
			fi->vertex(2)->point().x() >= inLanduse.mEast)			continue;
			
		if (fi->vertex(0)->point().x() <= inLanduse.mWest &&
			fi->vertex(1)->point().x() <= inLanduse.mWest &&		
			fi->vertex(2)->point().x() <= inLanduse.mWest)			continue;
			
			
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
		
		x = (fi->vertex(0)->point().x() + fi->vertex(1)->point().x() + fi->vertex(2)->point().x()) / 3.0;
		y = (fi->vertex(0)->point().y() + fi->vertex(1)->point().y() + fi->vertex(2)->point().y()) / 3.0;
		
		x = (x - inLanduse.mWest) / (inLanduse.mEast - inLanduse.mWest);
		y = (y - inLanduse.mSouth) / (inLanduse.mNorth - inLanduse.mSouth);
		
		x = floor(x*PATCH_DIM_HI);
		y = floor(y*PATCH_DIM_HI);
		
		if (x == PATCH_DIM_HI) x = PATCH_DIM_HI-1;
		if (y == PATCH_DIM_HI) y = PATCH_DIM_HI-1;
		if (x < 0 || y < 0 || x > PATCH_DIM_HI || y > PATCH_DIM_HI)
			fprintf(stderr, "Hires Triangle out of range, patch %lf,%lf, coords are %lf,%lf %lf,%lf %lf,%lf\n", x, y, 
				fi->vertex(0)->point().x(),fi->vertex(0)->point().y(),
				fi->vertex(1)->point().x(),fi->vertex(1)->point().y(),
				fi->vertex(2)->point().x(),fi->vertex(2)->point().y());
		
		// Accumulate the various texes into the various layers.  This means marking what land uses we have per each patch
		// and also any borders we need.
		sHiResTris[(int) x + (int) y * PATCH_DIM_HI].push_back(fi);
		landuses.insert(map<int, int, SortByLULayer>::value_type(fi->info().terrain,0));
		sHiResLU[(int) x + (int) y * PATCH_DIM_HI].insert(fi->info().terrain);
		for (border_lu = fi->info().terrain_border.begin(); border_lu != fi->info().terrain_border.end(); ++border_lu)
			sHiResBO[(int) x + (int) y * PATCH_DIM_HI].insert(*border_lu);
	}

	if (inProgress && inProgress(0, 5, "Compiling Mesh", 0.5)) return;	

#if !NO_ORTHO
	for (fi = inLoresMesh.finite_faces_begin(); fi != inLoresMesh.finite_faces_end(); ++fi)
	{
		if (fi->vertex(0)->point().y() >= inLanduse.mNorth &&
			fi->vertex(1)->point().y() >= inLanduse.mNorth &&		
			fi->vertex(2)->point().y() >= inLanduse.mNorth)			continue;
			
		if (fi->vertex(0)->point().y() <= inLanduse.mSouth &&
			fi->vertex(1)->point().y() <= inLanduse.mSouth &&		
			fi->vertex(2)->point().y() <= inLanduse.mSouth)			continue;

		if (fi->vertex(0)->point().x() >= inLanduse.mEast &&
			fi->vertex(1)->point().x() >= inLanduse.mEast &&		
			fi->vertex(2)->point().x() >= inLanduse.mEast)			continue;
			
		if (fi->vertex(0)->point().x() <= inLanduse.mWest &&
			fi->vertex(1)->point().x() <= inLanduse.mWest &&		
			fi->vertex(2)->point().x() <= inLanduse.mWest)			continue;

		x = (fi->vertex(0)->point().x() + fi->vertex(1)->point().x() + fi->vertex(2)->point().x()) / 3.0;
		y = (fi->vertex(0)->point().y() + fi->vertex(1)->point().y() + fi->vertex(2)->point().y()) / 3.0;
		
		x = (x - inLanduse.mWest) / (inLanduse.mEast - inLanduse.mWest);
		y = (y - inLanduse.mSouth) / (inLanduse.mNorth - inLanduse.mSouth);
		
		x = floor(x*PATCH_DIM_LO);
		y = floor(y*PATCH_DIM_LO);
		
		if (x < 0 || y < 0 || x >= PATCH_DIM_LO || y >= PATCH_DIM_LO)
			fprintf(stderr, "Lores Triangle out of range, patch %lf,%lf, coords are %lf,%lf %lf,%lf %lf,%lf\n", x, y, 
				fi->vertex(0)->point().x(),fi->vertex(0)->point().y(),
				fi->vertex(1)->point().x(),fi->vertex(1)->point().y(),
				fi->vertex(2)->point().x(),fi->vertex(2)->point().y());
		
		sLoResTris[(int) x + (int) y * PATCH_DIM_LO].push_back(fi);
		landuses.insert(map<int, int, SortByLULayer>::value_type(fi->info().terrain,0));
		sLoResLU[(int) x + (int) y * PATCH_DIM_LO].insert(fi->info().terrain);
	}
#endif
	
	// Now that we have our land uses, we can go back and calculate
	// the DSF-file-relative indices.

	cur_id = 0;
	for (lu_ranked = landuses.begin(); lu_ranked != landuses.end(); ++lu_ranked, ++cur_id)
	{
		lu_ranked->second = cur_id;
		landuses_reversed[cur_id] = lu_ranked->first;
	}

	if (inProgress && inProgress(0, 5, "Compiling Mesh", 1.0)) return;	

	for (prog_c = 0.0, lu_ranked = landuses.begin(); lu_ranked != landuses.end(); ++lu_ranked, prog_c += 1.0)
	{
		if (inProgress && inProgress(1, 5, "Sorting Mesh", prog_c / (float) landuses.size())) return;	

		/***************************************************************************************************************************************
		 * WRITE OUT LOW RES ORTHOPHOTO PATCHES
		 ***************************************************************************************************************************************/	

		is_water = lu_ranked->first == terrain_Water;
#if !NO_ORTHO		
		for (cur_id = 0; cur_id < (PATCH_DIM_LO*PATCH_DIM_LO); ++cur_id)
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
			TriFan_t * fan;
			cbs.BeginPatch_f(lu_ranked->second, ORTHO_NEAR_LOD, ORTHO_FAR_LOD, 0, 5, writer);
			while ((fan = fan_builder.GetNextFan()) != NULL)
			{
				++total_tri_fans;
				total_tri_fan_pts += fan->faces.size();
				cbs.BeginPrimitive_f(dsf_TriFan, writer);
				coords8[0] = fan->center->point().x();
				coords8[1] = fan->center->point().y();
				coords8[2] = fan->center->info().height;
				coords8[3] = fan->center->info().normal[0];
				coords8[4] =-fan->center->info().normal[1];						
				DebugAssert(coords8[3] >= -1.0);
				DebugAssert(coords8[3] <=  1.0);
				DebugAssert(coords8[4] >= -1.0);
				DebugAssert(coords8[4] <=  1.0);
				cbs.AddPatchVertex_f(coords8, writer);						
				avert = (*fan->faces.begin())->vertex(CDT::cw((*fan->faces.begin())->index(fan->center)));
				coords8[0] = avert->point().x();
				coords8[1] = avert->point().y();
				coords8[2] = avert->info().height;
				coords8[3] = avert->info().normal[0];
				coords8[4] =-avert->info().normal[1];						
				DebugAssert(coords8[3] >= -1.0);
				DebugAssert(coords8[3] <=  1.0);
				DebugAssert(coords8[4] >= -1.0);
				DebugAssert(coords8[4] <=  1.0);
				cbs.AddPatchVertex_f(coords8, writer);						
				for (nf = fan->faces.begin(); nf != fan->faces.end(); ++nf)
				{
					avert = (*nf)->vertex(CDT::ccw((*nf)->index(fan->center)));
					coords8[0] = avert->point().x();
					coords8[1] = avert->point().y();
					coords8[2] = avert->info().height;
					coords8[3] = avert->info().normal[0];
					coords8[4] =-avert->info().normal[1];						
					DebugAssert(coords8[3] >= -1.0);
					DebugAssert(coords8[3] <=  1.0);
					DebugAssert(coords8[4] >= -1.0);
					DebugAssert(coords8[4] <=  1.0);
					cbs.AddPatchVertex_f(coords8, writer);						
				}
				cbs.EndPrimitive_f(writer);
				fan_builder.DoneWithFan(fan);
			}
			if (!fan_builder.Done())
			{
				cbs.BeginPrimitive_f(dsf_Tri, writer);
				tris_this_patch = 0;
				while (!fan_builder.Done())
				{
					if (tris_this_patch >= MAX_TRIS_PER_PATCH)
					{
						cbs.EndPrimitive_f(writer);
						cbs.BeginPrimitive_f(dsf_Tri, writer);				
						tris_this_patch = 0;
					}				
					++total_tris;
					f = fan_builder.GetNextRemainingTriangle();
					for (v = 2; v >= 0; --v)
					{
						coords8[0] = f->vertex(v)->point().x();
						coords8[1] = f->vertex(v)->point().y();
						coords8[2] = f->vertex(v)->info().height;
						coords8[3] = f->vertex(v)->info().normal[0];
						coords8[4] =-f->vertex(v)->info().normal[1];
						DebugAssert(coords8[3] >= -1.0);
						DebugAssert(coords8[3] <=  1.0);
						DebugAssert(coords8[4] >= -1.0);
						DebugAssert(coords8[4] <=  1.0);
						cbs.AddPatchVertex_f(coords8, writer);
					}
//					cbs.EndPrimitive_f(writer);
					++tris_this_patch;
				}
				cbs.EndPrimitive_f(writer);
			}
			cbs.EndPatch_f(writer);
			++total_patches;
		}
#endif		

		/***************************************************************************************************************************************
		 * WRITE OUT HI RES BASE PATCHES
		 ***************************************************************************************************************************************/	
		for (cur_id = 0; cur_id < (PATCH_DIM_HI*PATCH_DIM_HI); ++cur_id)
		if (sHiResLU[cur_id].count(lu_ranked->first))
		{
			TriFanBuilder	fan_builder(&inHiresMesh);
			for (tri = 0; tri < sHiResTris[cur_id].size(); ++tri)
			{
				f = sHiResTris[cur_id][tri];
				if (f->info().terrain == lu_ranked->first)
				{
					CHECK_TRI(f->vertex(0),f->vertex(1),f->vertex(2));
					fan_builder.AddTriToFanPool(f);

					++debug_add_tri_fan;
				}
			}
			fan_builder.CalcFans();
			TriFan_t * fan;
			cbs.BeginPatch_f(lu_ranked->second, TERRAIN_NEAR_LOD, TERRAIN_FAR_LOD, dsf_Flag_Physical, is_water ? 6 : 5, writer);			
			while ((fan = fan_builder.GetNextFan()) != NULL)
			{
				++total_tri_fans;
				total_tri_fan_pts += fan->faces.size();
				cbs.BeginPrimitive_f(dsf_TriFan, writer);
				coords8[0] = fan->center->point().x();
				coords8[1] = fan->center->point().y();
				coords8[2] = fan->center->info().height;
				coords8[3] = fan->center->info().normal[0];
				coords8[4] =-fan->center->info().normal[1];						
				if (is_water)
					coords8[5] = GetWaterBlend(fan->center, waterType);
				DebugAssert(coords8[3] >= -1.0);
				DebugAssert(coords8[3] <=  1.0);
				DebugAssert(coords8[4] >= -1.0);
				DebugAssert(coords8[4] <=  1.0);
				cbs.AddPatchVertex_f(coords8, writer);						
				avert = (*fan->faces.begin())->vertex(CDT::cw((*fan->faces.begin())->index(fan->center)));
				coords8[0] = avert->point().x();
				coords8[1] = avert->point().y();
				coords8[2] = avert->info().height;
				coords8[3] = avert->info().normal[0];
				coords8[4] =-avert->info().normal[1];						
				if (is_water)
					coords8[5] = GetWaterBlend(avert, waterType);
				DebugAssert(coords8[3] >= -1.0);
				DebugAssert(coords8[3] <=  1.0);
				DebugAssert(coords8[4] >= -1.0);
				DebugAssert(coords8[4] <=  1.0);
				cbs.AddPatchVertex_f(coords8, writer);						
				for (nf = fan->faces.begin(); nf != fan->faces.end(); ++nf)
				{
					avert = (*nf)->vertex(CDT::ccw((*nf)->index(fan->center)));
					coords8[0] = avert->point().x();
					coords8[1] = avert->point().y();
					coords8[2] = avert->info().height;
					coords8[3] = avert->info().normal[0];
					coords8[4] =-avert->info().normal[1];						
					if (is_water)
						coords8[5] = GetWaterBlend(avert, waterType);
					DebugAssert(coords8[3] >= -1.0);
					DebugAssert(coords8[3] <=  1.0);
					DebugAssert(coords8[4] >= -1.0);
					DebugAssert(coords8[4] <=  1.0);
					cbs.AddPatchVertex_f(coords8, writer);						
					++debug_sub_tri_fan;
				}
				cbs.EndPrimitive_f(writer);
				fan_builder.DoneWithFan(fan);
			}
			if (!fan_builder.Done())
			{
				cbs.BeginPrimitive_f(dsf_Tri, writer);				
				tris_this_patch = 0;
				while (!fan_builder.Done())
				{
					if (tris_this_patch >= MAX_TRIS_PER_PATCH)
					{
						cbs.EndPrimitive_f(writer);
						cbs.BeginPrimitive_f(dsf_Tri, writer);				
						tris_this_patch = 0;
					}				
					++total_tris;
					f = fan_builder.GetNextRemainingTriangle();
					CHECK_TRI(f->vertex(2),f->vertex(1),f->vertex(0));
					for (v = 2; v >= 0; --v)
					{
						coords8[0] = f->vertex(v)->point().x();
						coords8[1] = f->vertex(v)->point().y();
						coords8[2] = f->vertex(v)->info().height;
						coords8[3] = f->vertex(v)->info().normal[0];
						coords8[4] =-f->vertex(v)->info().normal[1];						
						if (is_water)
							coords8[5] = GetWaterBlend(f->vertex(v), waterType);
						DebugAssert(coords8[3] >= -1.0);
						DebugAssert(coords8[3] <=  1.0);
						DebugAssert(coords8[4] >= -1.0);
						DebugAssert(coords8[4] <=  1.0);
						cbs.AddPatchVertex_f(coords8, writer);						
					}
					++tris_this_patch;
					++debug_sub_tri_fan;					
				}
				cbs.EndPrimitive_f(writer);
			}
			cbs.EndPatch_f(writer);
			++total_patches;
		}		

		/***************************************************************************************************************************************
		 * WRITE OUT HI RES BORDER PATCHES
		 ***************************************************************************************************************************************/	

#if !NO_BORDERS
		for (cur_id = 0; cur_id < (PATCH_DIM_HI*PATCH_DIM_HI); ++cur_id)		// For each triangle in this patch
		if (lu_ranked->first >= terrain_Natural)
		if (sHiResBO[cur_id].count(lu_ranked->first))							// Quick check: do we have ANY border tris in this layer in this patch?
		{
			cbs.BeginPatch_f(lu_ranked->second, TERRAIN_NEAR_LOD, TERRAIN_FAR_LOD, dsf_Flag_Overlay, /*is_composite ? 8 :*/ 7, writer);
			cbs.BeginPrimitive_f(dsf_Tri, writer);				
			tris_this_patch = 0;
			for (tri = 0; tri < sHiResTris[cur_id].size(); ++tri)				// For each tri
			{
				f = sHiResTris[cur_id][tri];
				if (f->info().terrain_border.count(lu_ranked->first))			// If it has this border...
				{
					float	bblend[3];
					int vi;
					for (vi = 0; vi < 3; ++vi)
						bblend[vi] = f->vertex(vi)->info().border_blend[lu_ranked->first];
					
					int ts = -1, te = 0;
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
							cbs.EndPrimitive_f(writer);
							cbs.BeginPrimitive_f(dsf_Tri, writer);				
							tris_this_patch = 0;
						}
					
						for (vi = 2; vi >= 0 ; --vi)
						{
							coords8[0] = f->vertex(vi)->point().x();
							coords8[1] = f->vertex(vi)->point().y();
							coords8[2] = f->vertex(vi)->info().height;
							coords8[3] = f->vertex(vi)->info().normal[0];
							coords8[4] =-f->vertex(vi)->info().normal[1];
//							coords8[5] = f->vertex(vi)->info().border_blend[lu_ranked->first];
							coords8[5] = vi == border_pass ? 0.0 : bblend[vi];
							coords8[6] = GetTightnessBlend(inHiresMesh, f, f->vertex(vi), lu_ranked->first);
							DebugAssert(!is_water);
	//						if (is_composite)
	//							coords8[7] = is_water ? GetWaterBlend(f->vertex(vi), waterType) : f->vertex(vi)->info().vege_density;
							DebugAssert(coords8[3] >= -1.0);
							DebugAssert(coords8[3] <=  1.0);
							DebugAssert(coords8[4] >= -1.0);
							DebugAssert(coords8[4] <=  1.0);
							cbs.AddPatchVertex_f(coords8, writer);
						}
						++total_tris;
						++border_tris;
						++tris_this_patch;						
					}										
				}
			}
			cbs.EndPrimitive_f(writer);
			cbs.EndPatch_f(writer);
			++total_patches;
		}
#endif		
	}
	
	for (lu = landuses_reversed.begin(); lu != landuses_reversed.end(); ++lu)
	{
		string def = get_terrain_name(lu->second);
		cbs.AcceptTerrainDef_f(def.c_str(), writer);
	}
	
	if (inProgress && inProgress(1, 5, "Sorting Mesh", 1.0)) return;	

	/****************************************************************
	 * BEACH EXPORT
	 ****************************************************************/
	
	// Beach export - we are going to export polygon rings/chains out of
	// every homogenous continous coastline type.  Two issues:
	// When a beach is not a ring, we need to find the start link
	// We also need to identify rings somehow.

	typedef hash_map<edge_wrapper, edge_wrapper, hash_edge>						LinkMap;
	typedef set<edge_wrapper>													LinkSet;
	typedef hash_map<edge_wrapper, int, hash_edge>								LinkInfo;
	
	LinkMap			linkNext;	// A hash map from each halfedge to the next with matching beach.  Uses CCW traversal to handle screw cases.
	LinkSet			nonStart;	// Set of all halfedges that are pointed to by another.
	LinkInfo		all;		// Ones we haven't exported.
	LinkSet			starts;		// Ones that are not pointed to by a HE
	edge_wrapper	beach, last_beach;
	int				beachKind;

	// Go through and build up the link map, e.g. for each edge, who's next.
	// Also record each edge that's pointed to by another - these are NOT
	// the starts of non-ring beaches.
	for (fi = inHiresMesh.finite_faces_begin(); fi != inHiresMesh.finite_faces_end(); ++fi)
	for (v = 0; v < 3; ++v)
	{
		edge_wrapper edge;
		edge.edge.first = fi;
		edge.edge.second = v;
		if (has_beach(edge, inHiresMesh, beachKind))
		{
			all[edge] = beachKind;
			starts.insert(edge);
			// Go through each he coming out of our target starting with the one to the clockwise of us, going clockwise. 
			// We're searching for the next beach seg but skipping bogus in-water stuff like brides.
			for (edge_wrapper iter = edge_next(edge); iter != edge_twin(edge); iter = edge_twin_next(iter))
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
				if (iter.edge.first->info().terrain != terrain_Water)
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
	
		cbs.BeginPolygon_f(0, 0, 6, writer);
		cbs.BeginPolygonWinding_f(writer);
		
		for (beach = *a_start; beach.edge.first != NULL; beach = ((linkNext.count(beach)) ? (linkNext[beach]) : edge_wrapper(NULL, 0)))
		{
			last_beach = beach;
			DebugAssert(all.count(beach) != 0);
			beachKind = all[beach];
			BeachPtGrab(beach, false, inHiresMesh, coords6, beachKind);
			cbs.AddPolygonPoint_f(coords6, writer);
			all.erase(beach);
		}
		DebugAssert(all.count(*a_start) == 0);

		BeachPtGrab(last_beach, true, inHiresMesh, coords6, beachKind);
		cbs.AddPolygonPoint_f(coords6, writer);	
		
		cbs.EndPolygonWinding_f(writer);
		cbs.EndPolygon_f(writer);
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
		edge_wrapper this_start = all.begin()->first;
		cbs.BeginPolygon_f(0, 1, 6, writer);
		cbs.BeginPolygonWinding_f(writer);
	
		beach = this_start;
		do {
			DebugAssert(all.count(beach) != 0);
			DebugAssert(linkNext.count(beach) != 0);
			beachKind = all.begin()->second;
			BeachPtGrab(beach, false, inHiresMesh, coords6, beachKind);
			cbs.AddPolygonPoint_f(coords6, writer);			
			all.erase(beach);
			beach = linkNext[beach];
		} while (beach != this_start);		
		cbs.EndPolygonWinding_f(writer);
		cbs.EndPolygon_f(writer);
		
	}
	cbs.AcceptPolygonDef_f("lib/g8/beaches.bch", writer);
	
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
		for (pointObj = pf->mObjs.begin(); pointObj != pf->mObjs.end(); ++pointObj)
			objects.insert(map<int, int>::value_type(pointObj->mRepType, 0));		
		for (polyObj = pf->mPolyObjs.begin(); polyObj != pf->mPolyObjs.end(); ++polyObj)
			facades.insert(map<int, int>::value_type(polyObj->mRepType, 0));
	}

	// Farm out object IDs.
	cur_id = 0;
	for (obdef = objects.begin(); obdef != objects.end(); ++obdef, ++cur_id)
	{
		obdef->second = cur_id;
		objects_reversed[cur_id] = obdef->first;
	}
	cur_id = 1;
	for (obdef = facades.begin(); obdef != facades.end(); ++obdef, ++cur_id)
	{
		obdef->second = cur_id;
		facades_reversed[cur_id] = obdef->first;
	}
	
	// Now go through and emit the objects.  Note: there is no point to 
	// sorting them - the DSF lib is good about cleaning up the object
	// data you give it.
	
	for (pf = inVectorMap.faces_begin(); pf != inVectorMap.faces_end(); ++pf)
	if (!pf->is_unbounded())
	{
		for (pointObj = pf->mObjs.begin(); pointObj != pf->mObjs.end(); ++pointObj)
		{
			coords2[0] = pointObj->mLocation.x;
			coords2[1] = pointObj->mLocation.y;
			cbs.AddObject_f(
				objects[pointObj->mRepType],
				coords2,
				(pointObj->mHeading < 0.0) ? (pointObj->mHeading + 360.0) : pointObj->mHeading,
				writer);
			++total_objs;
		}
		
		for (polyObj = pf->mPolyObjs.begin(); polyObj != pf->mPolyObjs.end(); ++polyObj)
		{
			bool	 broken = false;
			for (polyRing = polyObj->mShape.begin(); polyRing != polyObj->mShape.end(); ++polyRing)
			for (polyPt = polyRing->begin(); polyPt != polyRing->end(); ++polyPt)
			{
				if (polyPt->x < inLanduse.mWest ||
					polyPt->x > inLanduse.mEast ||
					polyPt->y < inLanduse.mSouth ||
					polyPt->y > inLanduse.mNorth)
				{
					printf("Pt %lf %lf is out of DEM.\n", polyPt->x, polyPt->y);
					broken = true;
				}
			}
			
			if (broken) 
				continue;
				
			cbs.BeginPolygon_f(
					facades[polyObj->mRepType],
					polyObj->mHeight, 2,
					writer);
			for (polyRing = polyObj->mShape.begin(); polyRing != polyObj->mShape.end(); ++polyRing)
			{
				cbs.BeginPolygonWinding_f(writer);					
				for (polyPt = polyRing->begin(); polyPt != polyRing->end(); ++polyPt)
				{
					coords2[0] = polyPt->x;
					coords2[1] = polyPt->y;
					cbs.AddPolygonPoint_f(coords2, writer);
					
				}
				cbs.EndPolygonWinding_f(writer);
			}
			cbs.EndPolygon_f(writer);
			++total_polys;
		}
	}

	// Write out definition names too.	
	for (obdef = objects_reversed.begin(); obdef != objects_reversed.end(); ++obdef)
	{
		string objName = FetchTokenString(obdef->second);
		objName += ".obj";
		cbs.AcceptObjectDef_f(objName.c_str(), writer);
	}

	for (obdef = facades_reversed.begin(); obdef != facades_reversed.end(); ++obdef)
	{
		string facName = FetchTokenString(obdef->second);
		if (IsForestType(obdef->second))
		{
			facName = "lib/g8/"+facName+".for";
		} else
			facName += ".fac";
		cbs.AcceptPolygonDef_f(facName.c_str(), writer);
	}

	if (inProgress && inProgress(2, 5, "Compiling Objects", 1.0)) return;	

	/***************************************************************
	 * HACKED TREE CODE 
	 ****************************************************************/

#if TEST_FORESTS
	{	 
		int for_def = facades_reversed.size() + 1;
		cbs.BeginPolygon_f(for_def, 255, 2, writer);
		cbs.BeginPolygonWinding_f(writer);
		coords2[0] = -117.1;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.1;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.12;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.12;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		cbs.EndPolygonWinding_f(writer);
		cbs.EndPolygon_f(writer);
		////////////////////////////////////////////////////////
		cbs.BeginPolygon_f(for_def, 150, 2, writer);
		cbs.BeginPolygonWinding_f(writer);
		coords2[0] = -117.2;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.2;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.22;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.22;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		cbs.EndPolygonWinding_f(writer);
		cbs.EndPolygon_f(writer);
		////////////////////////////////////////////////////////
		cbs.BeginPolygon_f(for_def, 40, 2, writer);
		cbs.BeginPolygonWinding_f(writer);
		coords2[0] = -117.3;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.3;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.32;
		coords2[1] = 32.92;
		cbs.AddPolygonPoint_f(coords2, writer);

		coords2[0] = -117.32;
		coords2[1] = 32.9;
		cbs.AddPolygonPoint_f(coords2, writer);

		cbs.EndPolygonWinding_f(writer);
		cbs.EndPolygon_f(writer);

		cbs.AcceptPolygonDef_f("test.for", writer);
	}
#endif	 

	/****************************************************************
	 * VECTOR EXPORT
	 ****************************************************************/

	static int vec_export_hint_id = CDT::gen_cache_key();

	if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.0)) return;	

	if (gDSFBuildPrefs.export_roads)
	{

		if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.3)) return;	

		{
			TIMER(BuildNetworkTopology)
			BuildNetworkTopology(inVectorMap, junctions, chains);
		}
		{
			TIMER(DrapeRoads)
			if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.6)) return;	
			DrapeRoads(junctions, chains, inHiresMesh);
		}
		{	
			TIMER(PromoteShapePoints)
			PromoteShapePoints(junctions, chains);
		}
		{
			TIMER(VerticalPartitionRoads)		
			VerticalPartitionRoads(junctions, chains);
		}
		{
			TIMER(VerticalBuildBridges)
			VerticalBuildBridges(junctions, chains);
		}
		{
			TIMER(InterpolateRoadHeights)
			InterpolateRoadHeights(junctions, chains);
		}
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
		{
			TIMER(SpacePowerlines)
			SpacePowerlines(junctions, chains, 1000.0, 10.0);
		}
		if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.7)) return;	

		cur_id = 1;
		for (ji = junctions.begin(); ji != junctions.end(); ++ji)
			(*ji)->index = cur_id++;

			vector<Point3>	intermediates;
				
		for (ci = chains.begin(); ci != chains.end(); ++ci)
		{
			coords3[0] = (*ci)->start_junction->location.x;
			coords3[1] = (*ci)->start_junction->location.y;
			coords3[2] = (*ci)->start_junction->location.z;
			if (coords3[0] < inLanduse.mWest  || coords3[0] > inLanduse.mEast || coords3[1] < inLanduse.mSouth || coords3[1] > inLanduse.mNorth)
				printf("WARNING: coordinate out of range.\n");
				
			cbs.BeginSegment_f(
							0, 
							(*ci)->export_type,
							(*ci)->start_junction->index,
							coords3,
							false,
							writer);
			++total_chains;
			
			for (shapePoint = (*ci)->shape.begin(); shapePoint != (*ci)->shape.end(); ++shapePoint)
			{
				coords3[0] = shapePoint->x;
				coords3[1] = shapePoint->y;
				coords3[2] = shapePoint->z;
				if (coords3[0] < inLanduse.mWest  || coords3[0] > inLanduse.mEast || coords3[1] < inLanduse.mSouth || coords3[1] > inLanduse.mNorth)
					printf("WARNING: coordinate out of range.\n");

				cbs.AddSegmentShapePoint_f(coords3, false, writer);
				++total_shapes;
			}

			coords3[0] = (*ci)->end_junction->location.x;
			coords3[1] = (*ci)->end_junction->location.y;
			coords3[2] = (*ci)->end_junction->location.z;
			if (coords3[0] < inLanduse.mWest  || coords3[0] > inLanduse.mEast || coords3[1] < inLanduse.mSouth || coords3[1] > inLanduse.mNorth)
				printf("WARNING: coordinate out of range.\n");		
			cbs.EndSegment_f(
					(*ci)->end_junction->index,
					coords3,
					false,
					writer);
		}
		if (inProgress && inProgress(3, 5, "Compiling Vectors", 0.9)) return;	

		CleanupNetworkTopology(junctions, chains);
		if (inProgress && inProgress(3, 5, "Compiling Vectors", 1.0)) return;	
		cbs.AcceptNetworkDef_f("lib/g8/roads.net", writer);
	}
	
	/****************************************************************
	 * MANIFEST
	 ****************************************************************/

	sprintf(prop_buf, "%d", (int) inLanduse.mWest);			cbs.AcceptProperty_f("sim/west", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) inLanduse.mEast);			cbs.AcceptProperty_f("sim/east", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) inLanduse.mNorth);		cbs.AcceptProperty_f("sim/north", prop_buf, writer);
	sprintf(prop_buf, "%d", (int) inLanduse.mSouth);		cbs.AcceptProperty_f("sim/south", prop_buf, writer);
	cbs.AcceptProperty_f("sim/planet", "earth", writer);
	cbs.AcceptProperty_f("sim/creation_agent", "X-Plane Scenery Creator 1.0", writer);
	cbs.AcceptProperty_f("laminar/internal_revision", "1", writer);
	
	/****************************************************************
	 * WRITEOUT
	 ****************************************************************/
	if (inProgress && inProgress(4, 5, "Writing DSF file", 0.0)) return;	
	DSFWriteToFile(inFileName, writer);
	if (inProgress && inProgress(4, 5, "Writing DSF file", 1.0)) return;	
	
	printf("Patches: %d, Free Tris: %d, Tri Fans: %d, Tris in Fans: %d, Border Tris: %d, Avg Per Patch: %f, avg per fan: %f\n",
		total_patches, total_tris, total_tri_fans, total_tri_fan_pts, border_tris, 
		(float) (total_tri_fan_pts + total_tris) / (float) total_patches,
		(total_tri_fans == 0) ? 0.0 : ((float) (total_tri_fan_pts) / (float) total_tri_fans));
	printf("Objects: %d, Polys: %d\n", total_objs, total_polys);
	printf("LU: %d, Objdef: %d, PolyDef: %d\n", landuses.size(), objects.size(), facades.size());
	printf("Chains: %d, Shapes: %d\n", total_chains, total_shapes);
	printf("Submitted to tri fan builder: %d.  Removed from builder: %d.\n", debug_add_tri_fan, debug_sub_tri_fan);

}
