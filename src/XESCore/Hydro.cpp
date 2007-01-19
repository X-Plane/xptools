#include "Hydro.h"
#include "ParamDefs.h"
#include "MapDefs.h"
#include "DEMDefs.h"
#include "DEMAlgs.h"
#include <ShapeFil.h>
#include "MapAlgs.h"
#include "GISUtils.h"
#include "WED_Globals.h"
#include "AssertUtils.h"
#include "SimpleIO.h"
#include "MemFileUtils.h"
#include "MeshAlgs.h"
#include "PolyRasterUtils.h"
#include "PerfUtils.h"
#include "DEMToVector.h"
#include "CompGeomUtils.h"
#include "PlatformUtils.h"
#if APL && !defined(__MACH__)
#define __DEBUGGING__
#include "XUtils.h"
#endif
#define HYDRO_BORDER	"Hydro"

// This is how high we can raise the waterlevel of a river (turning it into a lake) before we give up
// and say 'heck, we have no idea what's going on'.  This prevents water from flowing massively uphill
// out of a pit in the DEM.
#define MAX_FLOOD 200.0

// This is how many DEM points we can merge trying to build a river before we give up and say 'this is standing water, it can
// just sit here'.
#define MAX_AREA 4000.0

// This is how far off we think the SRTM can be...we use this to not get too jiggy with it.
#define SRTM_VERTICAL_SLOP 8

// Required flow to actually build the river.
#define REQUIRED_FLOW 24000.0		// was 1500, 6000 is better
// Max slope over which a river can go.  This huge value basically turns off slope checking.
#define REQUIRED_SLOPE 100.0

// Number of DEM squares to look through to find river exit.
#define DEM_EXIT_SEARCH_RANGE 10

// Use 4-way flow instead of 8-way
#define DIRS_FOUR 0

// How far do we look around to see if this is hilly terrain?  Use this to tell VMAP to chill out
#define VMAP_RELIEF_RANGE 3
// If our steepness is beyond this, shouldn't be using VMAP!
#define VMAP_TOO_STEEP 24


// Size in DEM posts of the blocks to decide on what we want
#define SRTM_CHOICE_BLOCK_SIZE 10
// If less than this is wet by SRTM - hrm - maybe VMAP is on to something?
#define SRTM_TRUSTED_WETNESS 0.05

#if DIRS_FOUR
#define	DIRS_COUNT 4
static int dirs_x[DIRS_COUNT] = { 0, 1, 0, -1 };
static int dirs_y[DIRS_COUNT] = { 1, 0, -1, 0 };
#else
#define DIRS_COUNT 8
static int dirs_x[DIRS_COUNT] = { 0, 1, 1, 1, 0, -1, -1, -1 };
static int dirs_y[DIRS_COUNT] = { 1, 1, 0, -1, -1, -1, 0, 1 };
#endif


/******************************************************************************************************************************
 * DEM PT SETS
 ******************************************************************************************************************************/


struct DemPt {
	DemPt(int ix, int iy) : x(ix), y(iy) { }
	DemPt() : x(0), y(0) { }
	DemPt(const DemPt& rhs) : x(rhs.x), y(rhs.y) { }
	DemPt& operator=(const DemPt& rhs) { x = rhs.x; y = rhs.y; return *this; }
	int x;
	int y;
	bool operator<(const DemPt& rhs) const { 
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		return x < rhs.x;
	}
	bool operator==(const DemPt& rhs) const {
		return x == rhs.x && y == rhs.y; 
	}
	bool operator!=(const DemPt& rhs) const {
		return x != rhs.x || y != rhs.y; 
	}
};

HASH_MAP_NAMESPACE_START
template <>
struct hash<DemPt>
	HASH_PARENT(DemPt, std::size_t)
{
	std::size_t operator()(const DemPt& key) const { return key.x + (key.y << 16); }
};
HASH_MAP_NAMESPACE_END


typedef set<DemPt>				DemPtSet;
typedef multimap<float, DemPt>	DemPtMap;
typedef vector<DemPt>			DemPtVector;


inline void GetNeighbors(const DemPtSet& pts, DemPtSet& neighbors, DemPtMap& borderMap, int width, int height, const DEMGeo& elev)
{
	neighbors.clear();
	for (DemPtSet::const_iterator p = pts.begin(); p != pts.end(); ++p)
	{
		for (int n = 0; n < DIRS_COUNT; ++n)
		{
			DemPt np;
			np.x = p->x + dirs_x[n];
			np.y = p->y + dirs_y[n];
			if (np.x >= 0 && np.y >= 0 && np.x < width && np.y < height)
			if (pts.count(np) == 0)
			{
				neighbors.insert(np);
				float e = elev.get(np.x, np.y);
				borderMap.insert(DemPtMap::value_type(e, np));
				
			}
		}
	}	
}

inline void AddPtToSet(DemPtSet& area, DemPtSet& border, DemPtMap& borderMap, const DemPtMap::iterator it, int width, int height, const DEMGeo& elev)
{
	DemPt	p = it->second;
	DebugAssert(border.count(p) == 1);
	
	area.insert(p);
	border.erase(p);
	borderMap.erase(it);
	for (int n = 0; n < DIRS_COUNT; ++n)
	{
		DemPt np;
		np.x = p.x + dirs_x[n];
		np.y = p.y + dirs_y[n];
		if (np.x >= 0 && np.y >= 0 && np.x < width && np.y < height)
		if (area.count(np) == 0)
		if (border.count(np) == 0)
		{
			float e=  elev.get(np.x, np.y);
			border.insert(np);
			borderMap.insert(DemPtMap::value_type(e, np));
		}
	}
}

/******************************************************************************************************************************
 * RIVER DETECTION
 ******************************************************************************************************************************/

inline int	GetFlowDir(float e[DIRS_COUNT+1])
{
	float dif[DIRS_COUNT];
	int n;
	for (n = 0; n < DIRS_COUNT; ++n)
	{
		dif[n] = e[DIRS_COUNT] - e[n];
		if (e[n] == NO_DATA) dif[n] = NO_DATA;
	}
	
	float best = dif[0];
	int dir = drain_Dir0;
	for (n = 1; n < DIRS_COUNT; ++n)
		if (dif[n] > best) dir = drain_Dir0 + n, best = dif[n];
	if (best <= 0.0) dir = sink_Unresolved;
	return dir;	
}

inline bool GetNext(int& x, int& y, const DEMGeo& dem)
{
	int code = dem(x,y);
	if (code < drain_Dir0) return false;	
	DebugAssert(code < (drain_Dir0+DIRS_COUNT));
	x += dirs_x[code - drain_Dir0];
	y += dirs_y[code - drain_Dir0];
	return true;
}

inline float LowestInRange(const DEMGeo& inDEM, int x1, int y1, int x2, int y2, int& outX, int& outY)
{
	float e = NO_DATA;
	for (int y = y1; y < y2; ++y)
	for (int x = x1; x < x2; ++x)
	{
		float ee = inDEM.get(x,y);
		if (ee != NO_DATA)
		if (e == NO_DATA || ee < e)
		{
			e = ee;
			outX = x;
			outY = y;
		}
	}
	return e;
}

static void BurnLowestNearDrainPt(const DEMGeo& elev, DEMGeo& hydro_dir, double lon, double lat)
{
	int x, y, xo, yo;
	if (lon == elev.mWest)
	{
		y = elev.lat_to_y(lat);
		if (LowestInRange(elev, 0, y - DEM_EXIT_SEARCH_RANGE, 1, y + DEM_EXIT_SEARCH_RANGE + 1, xo, yo) != NO_DATA)
		{
			hydro_dir(xo, yo) = sink_Known;
		}
	}

	if (lon == elev.mEast)
	{
		y = elev.lat_to_y(lat);
		if (LowestInRange(elev, elev.mWidth-1, y - DEM_EXIT_SEARCH_RANGE, elev.mWidth, y + DEM_EXIT_SEARCH_RANGE + 1, xo, yo) != NO_DATA)
		{
			hydro_dir(xo, yo) = sink_Known;
		}
	}

	if (lat == elev.mSouth)
	{
		x = elev.lon_to_x(lon);
		if (LowestInRange(elev, x - DEM_EXIT_SEARCH_RANGE, 0, x + DEM_EXIT_SEARCH_RANGE + 1, 1, xo, yo) != NO_DATA)
		{
			hydro_dir(xo, yo) = sink_Known;
		}
	}

	if (lat == elev.mNorth)
	{
		x = elev.lon_to_x(lon);
		if (LowestInRange(elev, x - DEM_EXIT_SEARCH_RANGE, elev.mHeight-1, x + DEM_EXIT_SEARCH_RANGE + 1, elev.mHeight, xo, yo) != NO_DATA)
		{
			hydro_dir(xo, yo) = sink_Known;
		}
	}

}

int FixSink(int x, int y, DEMGeo& elev, DEMGeo& hydro_dir)
{	
	DemPt		orig; orig.x = x; orig.y = y;
	DemPtSet	sink; sink.insert(orig);
	float		low_elev;
	DemPt		drainPt;
	float		orig_elev = elev(x,y);
	int			hosed_fill = sink_Invalid;
	int			ctr = 0;
	DemPtSet	last_border(sink);
	DemPtMap	border_elev;
	GetNeighbors(sink, last_border, border_elev, elev.mWidth, elev.mHeight, elev);
	while (1)
	{
		if (last_border.empty()) 
			goto hosed;
		DebugAssert(!border_elev.empty());
	
		DemPtMap::iterator i = border_elev.begin();
		low_elev = i->first;

		++ctr;
		if (low_elev < elev(x,y) || hydro_dir(i->second.x, i->second.y) == sink_Known)
		{
			drainPt = i->second;
#if DEV
			DemPt foo(drainPt);
			if (GetNext(foo.x, foo.y, hydro_dir))
				DebugAssert(sink.count(foo) == 0);
#endif			
			goto found;				
		}
			
		hydro_dir(i->second.x, i->second.y) = sink_Unresolved;			
		AddPtToSet(sink, last_border, border_elev, i, elev.mWidth, elev.mHeight, elev);

		elev(x, y) = low_elev;
			
		DebugAssert(low_elev >= orig_elev);
		if ((low_elev - orig_elev) > MAX_FLOOD)
			goto hosed;
		if (sink.size() > MAX_AREA)
		{
			// BEN SEZ:: at this point our lakes are so @#$@#ing huge we probably do NOT want to fill them in - 
			// if there was a lake there, it'd be on VMAP0.
			hosed_fill = sink_Invalid;	// sink_Lake;
			goto hosed;
		}		
	}
hosed:

	for (DemPtSet::iterator j = sink.begin(); j != sink.end(); ++j)
		elev(j->x, j->y) = low_elev;

	for (DemPtSet::iterator j = sink.begin(); j != sink.end(); ++j)
		hydro_dir(j->x, j->y) = hosed_fill;
	return ctr;
		
found:

	for (DemPtSet::iterator j = sink.begin(); j != sink.end(); ++j)
		elev(j->x, j->y) = low_elev;

	DemPtSet	ok, working;
	working.insert(drainPt);

	while (!working.empty())
	{
		DemPt tr = *working.begin();
		working.erase(working.begin());
		ok.insert(tr);
		
		for (int n = 0; n < DIRS_COUNT; ++n)
		{
			DemPt	tr_n(tr);
			tr_n.x -= dirs_x[n];
			tr_n.y -= dirs_y[n];
			if (sink.count(tr_n) > 0)
			{
				hydro_dir(tr_n.x, tr_n.y) = n+drain_Dir0;
				sink.erase(tr_n);
				if (ok.count(tr_n) == 0)
					working.insert(tr_n);
			}
		}
	}
	DebugAssert(sink.empty());
	return ctr;
}

inline float MinSlopeNear(const DEMGeo& dem, int x, int y)
{
	float e = dem.get(x,y);
	for (int r = 1; r <= 2; ++r)
	for (int n = 0; n < DIRS_COUNT; ++n)
	{
		float ee = dem.get(x+r*dirs_x[n], y+r*dirs_y[n]);
		e = MIN_NODATA(e, ee);
	}
	return e;
}

inline int HydroFlowToPt(int x, int y, DEMGeo * elev, DEMGeo * dirs, DEMGeo * flows, DEMGeo * slope, int * ctr)
{
	(*ctr)++;
	float sum = 1.0;
	float slp = NO_DATA;
	float me_elev = elev->get(x,y);
	for (int n = 0; n < DIRS_COUNT; ++n)
	{
		if (dirs->get(x-dirs_x[n],y-dirs_y[n]) == (n+drain_Dir0))
		{			
			sum += HydroFlowToPt(x-dirs_x[n],y-dirs_y[n], elev, dirs, flows, slope, ctr);
			if (me_elev != NO_DATA)
			{
				float other_elev = elev->get(x-dirs_x[n],y-dirs_y[n]);
				if (other_elev != NO_DATA)
				{	
					float grad = other_elev - me_elev;
					if (grad >= 0.0)
						slp = MIN_NODATA(grad, slp);
				}
			}
		}
	}
	(*flows)(x,y) = sum;
	if (slp == NO_DATA) slp = 0.0;
	(*slope)(x,y) = slp;
	return sum;
}

static void BurnRiver(DEMGeo& dem, const Point2& p1, const Point2& p2, float v)
{
	double	x1 = dem.lon_to_x(p1.x);
	double	x2 = dem.lon_to_x(p2.x);
	double	y1 = dem.lat_to_y(p1.y);
	double	y2 = dem.lat_to_y(p2.y);
	
	Segment2	seg(Point2(x1, y1), Point2(x2, y2));
	double len = sqrt(seg.squared_length());
	if (len == 0.0) len = 1.0;
	len = 1.0 / len;
	
	for (double t = 0.0; t <= 1.0; t += len)
	{
		Point2 p = seg.midpoint(t);
		int ix = round(p.x);
		int iy = round(p.y);
		dem.set(ix-1,iy-1,v);
		dem.set(ix  ,iy-1,v);
		dem.set(ix+1,iy-1,v);
		dem.set(ix-1,iy  ,v);
		dem.set(ix  ,iy  ,v);
		dem.set(ix+1,iy  ,v);
		dem.set(ix-1,iy+1,v);
		dem.set(ix  ,iy+1,v);
		dem.set(ix+1,iy+1,v);
	}
}

bool	RiverPtsConnected(int x1, int y1, int x2, int y2, const DEMGeo& hydro_dir, const DEMGeo& hydro_flw, const DEMGeo& hydro_elev, const DEMGeo& is_river)
{
	if (hydro_elev.get(x1,y1) == NO_DATA) return false;
	if (hydro_elev.get(x2,y2) == NO_DATA) return false;
	
	int flow1 = hydro_dir.get(x1,y1);
	if (flow1 >= drain_Dir0 && 
		flow1 <= drain_Dir7 &&
		x1 + dirs_x[flow1 - drain_Dir0] == x2 &&
		y1 + dirs_y[flow1 - drain_Dir0] == y2)			return true;

	int flow2 = hydro_dir.get(x2,y2);
	if (flow2 >= drain_Dir0 && 
		flow2 <= drain_Dir7 &&
		x2 + dirs_x[flow2 - drain_Dir0] == x1 &&
		y2 + dirs_y[flow2 - drain_Dir0] == y1)			return true;
		
	return false;
}

int		RiverPtsConnectedAngle(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	for (int n = drain_Dir0; n <= drain_Dir7; ++n)
	{
		if (dirs_x[n - drain_Dir0] == dx &&
			dirs_y[n - drain_Dir0] == dy)	return n;
	}
	DebugAssert(!"No drain direction!");
	return sink_Invalid;
}

inline int	DrainDir_CW(int drain_dir)  { return drain_Dir0 + ((drain_dir - drain_Dir0 + 1) % 8); }
inline int	DrainDir_CCW(int drain_dir) { return drain_Dir0 + ((drain_dir - drain_Dir0 + 7) % 8); }

void	FindNextRiverSegment(int old_x, int old_y, int cur_x, int cur_y, int& new_x, int& new_y, const DEMGeo& hydro_dir, const DEMGeo& hydro_flw, const DEMGeo& hydro_elev, const DEMGeo& is_river)
{
	int	stop, angle = RiverPtsConnectedAngle(cur_x, cur_y, old_x, old_y);
	stop = angle;
	
	do {
		angle = DrainDir_CW(angle);
		new_x = cur_x + dirs_x[angle - drain_Dir0];
		new_y = cur_y + dirs_y[angle - drain_Dir0];
		if (RiverPtsConnected(cur_x, cur_y, new_x, new_y, hydro_dir, hydro_flw, hydro_elev, is_river))	return;
	} while (stop != angle);
	DebugAssert(!"No link found.");
}


void	BuildRiverPolygon(int x, int y, const DEMGeo& hydro_dir, const DEMGeo& hydro_flw, const DEMGeo& hydro_elev, const DEMGeo& is_river, Polygon2& poly, vector<double>& height)
{
	poly.clear();
	height.clear();
	
		int a;
		
	for (a = 0; a < 8; ++a)
	if (RiverPtsConnected(x, y, x + dirs_x[a], y + dirs_y[a], hydro_dir, hydro_flw, hydro_elev, is_river))
		break;
		
	if (a == 8) return;
	
	int last_x = x, last_y = y;
	int cur_x = x + dirs_x[a], cur_y = y + dirs_y[a];
	int new_x, new_y;
	int start_last_x = last_x, start_last_y = last_y;
	int start_cur_x = cur_x, start_cur_y = cur_y;
	
	do {
		poly.push_back(Point2(hydro_elev.x_to_lon(cur_x), hydro_elev.y_to_lat(cur_y)));
		
		FindNextRiverSegment(last_x, last_y, cur_x, cur_y, new_x, new_y, hydro_dir, hydro_flw, hydro_elev, is_river);

		double h_diff = 
			(fabs(hydro_elev.get(cur_x, cur_y) - hydro_elev.get(last_x, last_y)) +
			 fabs(hydro_elev.get(cur_x, cur_y) - hydro_elev.get(new_x, new_y)));
	
		double dist = sqrt(hydro_elev.x_dist_to_m(cur_x-last_x) * hydro_elev.x_dist_to_m(cur_x-last_x) +
						   hydro_elev.y_dist_to_m(cur_y-last_y) * hydro_elev.y_dist_to_m(cur_y-last_y)) + 
					  sqrt(hydro_elev.x_dist_to_m(cur_x-new_x ) * hydro_elev.x_dist_to_m(cur_x-new_x ) +
						   hydro_elev.y_dist_to_m(cur_y-new_y ) * hydro_elev.y_dist_to_m(cur_y-new_y ));

		dist /= 1000.0;

		double width = (h_diff == 0.0) ? 125.0 : (125.0 * dist / h_diff);
		width *= 0.5;
		
		if (width < 12.0) width = 12.0;
		if (width > 30.0) width = 30.0;
		
		height.push_back(width);
		
		last_x = cur_x;	last_y = cur_y;
		cur_x = new_x;	cur_y = new_y;
		
	} while (last_x != start_last_x || last_y != start_last_y || cur_x != start_cur_x || cur_y != start_cur_y);
}


void	BuildRivers(const Pmwx& inMap, DEMGeoMap& ioDEMs, ProgressFunc inProg)
{
	if (inProg) inProg(0, 4, "Preparing elevation maps", 0.0);
	int x, y, n, max_hydro;

	gMeshPoints.clear();
	gMeshLines.clear();
	DEMGeo  elev(ioDEMs[dem_Elevation]);
	DEMGeo&	hydro_elev(ioDEMs[dem_HydroElevation]);
	DEMGeo	is_river(61, 61);
	
	DEMGeo	hydro_dir(elev.mWidth, elev.mHeight);
	DEMGeo	hydro_flw(elev.mWidth, elev.mHeight);
	DEMGeo	hydro_slp(elev.mWidth, elev.mHeight);
	hydro_dir.copy_geo_from(elev);
	hydro_flw.copy_geo_from(elev);
	hydro_slp.copy_geo_from(elev);
	is_river.copy_geo_from(elev);
	
	hydro_dir = sink_Unresolved;
	is_river = 0;
	
	for (Pmwx::Halfedge_const_iterator he = inMap.halfedges_begin(); he != inMap.halfedges_end(); ++he)
	if (he->mDominant)
	if (he->mParams.count(he_IsRiver))
	{
		BurnRiver(is_river, he->source()->point(), he->target()->point(), 1);
	}
	
	if (inProg) inProg(0, 4, "Preparing elevation maps", 1.0);

	for (y = 0; y < hydro_elev.mHeight;++y)
		hydro_dir(0, y) = sink_Invalid;
	for (x = 0; x < hydro_elev.mWidth; ++x)
		hydro_dir(x, 0) = sink_Invalid;
	
	for (y = 0; y < hydro_elev.mHeight;++y)
	for (x = 0; x < hydro_elev.mWidth; ++x)
	if (hydro_elev.get(x,y) != NO_DATA)
		hydro_dir(x,y) = sink_Known;

	for (Pmwx::Halfedge_const_iterator he = inMap.halfedges_begin(); he != inMap.halfedges_end(); ++he)
	if (he->mDominant)	
	if (he->mParams.count(he_IsRiver) > 0)
	{
		Point2	sp = he->source()->point();
		if (sp.x == hydro_elev.mWest ||
			sp.x == hydro_elev.mEast ||
			sp.y == hydro_elev.mSouth ||
			sp.y == hydro_elev.mNorth)
		{
			BurnLowestNearDrainPt(elev, hydro_dir, sp.x, sp.y);
		}
		Point2	tp = he->target()->point();
		if (tp.x == hydro_elev.mWest ||
			tp.x == hydro_elev.mEast ||
			tp.y == hydro_elev.mSouth ||
			tp.y == hydro_elev.mNorth)
		{
			BurnLowestNearDrainPt(elev, hydro_dir, tp.x, tp.y);
		}
	}

	max_hydro = elev.mWidth * elev.mHeight;
	float e[DIRS_COUNT+1];
	if (inProg) inProg(1, 4, "Calculating drainage...", 0.0);	
	for (x = 0; x < hydro_dir.mWidth; ++x)
	{
		if (inProg && (x % 20) == 0) inProg(1, 4, "Calculating drainage...", (float) x / (float) hydro_dir.mWidth);

		for (y = 0; y < hydro_dir.mHeight; ++y)
		{
			if (hydro_dir(x,y) == sink_Known || hydro_dir(x,y) == sink_Invalid) 
				continue;		
			for (n = 0; n < DIRS_COUNT; ++n)
				e[n] = elev.get(x+dirs_x[n], y + dirs_y[n]);	
			e[DIRS_COUNT] = elev.get(x  ,y  );
			hydro_dir(x,y) = GetFlowDir(e);
		}
	}
	if (inProg) inProg(1, 4, "Calculating drainage...", 1.0);
	
	
	if (inProg) inProg(2, 4, "Removing sinks...", 0.0);	
	int total_sink_pts = 0;
//	map<int, int>	histo;
	for (x = 0; x < hydro_dir.mWidth; ++x)
	{
		if (inProg && (x % 20) == 0) inProg(2, 4, "Removing sinks...", (float) x / (float) hydro_dir.mWidth);	
		for (y = 0; y < hydro_dir.mHeight; ++y)
		{
			if (hydro_dir(x,y) == sink_Unresolved)
			{
				int worked = FixSink(x, y, elev, hydro_dir);
				total_sink_pts += worked;
				worked -= (worked % 100);
//				histo[worked]++;
			}
		}
	}
//	printf("HISTO:\n");
//	for (map<int, int>::iterator i = histo.begin(); i != histo.end(); ++i)
//		printf("%10d %10d\n", i->first, i->second);
	if (inProg) inProg(2, 4, "Removing sinks...", 1.0);	

	if (inProg) inProg(3, 4, "Calculating Flow...", 0.0);	
	int ctr = 0;
	for (y = 0; y < hydro_dir.mHeight; ++y)
	{
		if (inProg && (y % 20) == 0) inProg(3, 4, "Calculating Flow...", (float) y / (float) hydro_dir.mHeight);	
		for (x = 0; x < hydro_dir.mWidth; ++x)
		if (hydro_dir(x,y) < drain_Dir0)
			HydroFlowToPt(x, y, &elev, &hydro_dir, &hydro_flw, &hydro_slp, &ctr);
	}

	for (y = 0; y < hydro_dir.mHeight; ++y)
	for (x = 0; x < hydro_dir.mWidth; ++x)
	if (is_river.xy_nearest(hydro_dir.x_to_lon(x),hydro_dir.y_to_lat(y)))
	{
		if (hydro_flw(x,y) > REQUIRED_FLOW)
		if (MinSlopeNear(hydro_slp,x,y) < REQUIRED_SLOPE)
			hydro_elev(x,y) = elev(x,y);
		if (hydro_dir(x,y) == sink_Lake)
			hydro_elev(x,y) = elev(x,y);
	}
	if (inProg) inProg(3, 4, "Calculating Flow...", 1.0);	

#if 0		
	for (y = 0; y < hydro_dir.mHeight; ++y)
	for (x = 0; x < hydro_dir.mWidth; ++x)
	if (hydro_dir(x,y) == sink_Known)
	{
		Polygon2	foo;
		vector<double>	widths;
		BuildRiverPolygon(x, y, hydro_dir, hydro_flw, hydro_elev, is_river, foo, widths);
		if (!foo.empty())
		{		
			for (int k = 0; k < foo.size(); ++k)
			{
				if (k != 0)	gMeshLines.push_back(foo[k]);
							gMeshLines.push_back(foo[k]);
			}
			gMeshLines.push_back(foo[0]);

			MidpointSimplifyPolygon(foo);
			if (!foo.empty())
			{

				Polygon2 foo2;
				InsetPolygon2(foo, &*widths.begin(), 1.0 * MTR_TO_NM * NM_TO_DEG_LAT, true, foo2, NULL, NULL);
				for (int k = 0; k < foo2.size(); ++k)
				{
					if (k != 0)	gMeshLines.push_back(foo2[k]);
								gMeshLines.push_back(foo2[k]);
				}
				gMeshLines.push_back(foo2[0]);
			}
		}
	}
#endif	
			
	ioDEMs[dem_HydroDirection].swap(hydro_dir);
	ioDEMs[dem_HydroQuantity].swap(hydro_flw);
	
	printf("Total sink points: %d\n", total_sink_pts);
}

#pragma mark -
/******************************************************************************************************************************
 * OCEAN CORRECTION
 ******************************************************************************************************************************/

inline bool	IsCoastal(const DEMGeo& dem, int x, int y)
{
	if (x == 0 || y == 0 ||
		x == (dem.mWidth-1) ||
		y == (dem.mHeight-1))			return false;
	if (dem.get(x-1,y  ) == NO_DATA)	return true;
	if (dem.get(x+1,y  ) == NO_DATA)	return true;
	if (dem.get(x  ,y-1) == NO_DATA)	return true;
	if (dem.get(x  ,y+1) == NO_DATA)	return true;
	
// Hrm...diagonals let the algorithm hop all over the place and trace
// out discontinuous stuff.  Not good to do.	

//	if (dem.get(x-1,y-1) == NO_DATA)	return true;
//	if (dem.get(x+1,y-1) == NO_DATA)	return true;
//	if (dem.get(x-1,y+1) == NO_DATA)	return true;
//	if (dem.get(x+1,y+1) == NO_DATA)	return true;
	return false;
}

/*
 * origElev - the original unmodified elevation DEM.
 * wetElev - destination DEM - we copy the wet points into this DEM, with area reduced and water body lowered.
 * wetFaces - a set of all of the connected wet faces to analyze.
 *
 */
void	BuildCorrectedWaterBody(const DEMGeo& origElev, DEMGeo& wetElev, const set<GISFace *> wetFaces)
{
	DEMGeo					workingElev(origElev.mWidth, origElev.mHeight);
	PolyRasterizer			raster;
	set<GISHalfedge *>		bounds;
	int						x1, x2, y1, y2;	// This is the working bounds of the water body for faster future DEM processing.
	int						rx1, rx2, x, y;
	int						count = 0;
	hash_map<float, int>	histogram;
	float					e;
	float 					minv_lim = 9.9e9;
	float 					maxv_lim =-9.9e9;
	
	/* STEP 1. BURN THE DEM INTO THE WORKING MAP AND DETERMINE SEALEVEL. */

	workingElev.copy_geo_from(origElev);
	workingElev = NO_DATA;	
	FindEdgesForFaceSet(wetFaces, bounds);

	y = SetupRasterizerForDEM(bounds, origElev, raster);
	x1 = origElev.mWidth;
	x2 = 0;
	y1 = y;
	y2 = y;

	raster.StartScanline(y);
	while (!raster.DoneScan())
	{
		while (raster.GetRange(rx1, rx2))
		{
			x2 = max(x2, rx2);
			x1 = min(x1, rx1);
			for (x = rx1; x < rx2; ++x)
			{
				e = origElev.get(x,y);
				DebugAssert(e != NO_DATA);	// We expect the DEM to be filled in.
				if (e != NO_DATA)
					count++, histogram[e]++;
				workingElev(x,y) = e;
			}
		}
		++y;
		if (y >= origElev.mHeight) 
			break;
		raster.AdvanceScanline(y);		
	}
	y2 = y;
	
	// If we have no area, quick bail!
	if (count == 0) return;
	
	if (x2 == (workingElev.mWidth-1))
	{
		for (y = 0; y < workingElev.mHeight; ++y)
			workingElev(x2, y) = workingElev.get(x2-1, y);
		++x2;
	}
	if (y2 == (workingElev.mHeight-1))
	{
		for (x = 0; x < workingElev.mWidth; ++x)
			workingElev(x, y2) = workingElev.get(x, y2-1);
		++y2;
	}
	
	{
		multimap<int, float, greater<int> >	freqtable;
		for (hash_map<float, int>::iterator iter = histogram.begin(); iter != histogram.end(); ++iter)
			freqtable.insert(multimap<int,float>::value_type(iter->second, iter->first));
		
		float sum = 0.0;
		int ctr = 0;
		int needed = 0.5 * (float) count;
		for (multimap<int, float, greater<int> >::iterator i = freqtable.begin(); i != freqtable.end(); ++i)
		{
			minv_lim = min(minv_lim, i->second);
			maxv_lim = max(maxv_lim, i->second);
			sum += (i->second * (float) i->first);
			ctr += i->first;
			if (ctr > needed)
				break;
		}
	}
	
	minv_lim -= SRTM_VERTICAL_SLOP;
	maxv_lim += SRTM_VERTICAL_SLOP;

	DemPtVector	coastal;
	
	DEMGeo	hit(workingElev.mWidth, workingElev.mHeight);
	
//	StElapsedTime	processOne("checking coastal pts.");
	int check = 0, total = 0;
	
	for (y = y1; y < y2; ++y)
	for (x = x1; x < x2; ++x)
	{
		e = workingElev.get(x,y);
		if (e != NO_DATA)
		if (IsCoastal(workingElev, x, y))
		{
			hit(x,y) = 1;
			coastal.push_back(DemPt(x,y));
		}
	}
	do {
		DemPtVector	newer;
		for (DemPtVector::iterator pt = coastal.begin(); pt != coastal.end(); ++pt)
		{
			++check;
			e = workingElev.get(pt->x, pt->y);
			if (e < minv_lim || e > maxv_lim)
			{
				++total;
				workingElev(pt->x,pt->y) = NO_DATA;
				e = workingElev.get(pt->x-1,pt->y  ); if (e != NO_DATA && IsCoastal(workingElev, pt->x-1,pt->y  ) && hit.get(pt->x-1,pt->y  )==0) { newer.push_back(DemPt(pt->x-1,pt->y  )); hit(pt->x-1,pt->y  )=1; }
				e = workingElev.get(pt->x+1,pt->y  ); if (e != NO_DATA && IsCoastal(workingElev, pt->x+1,pt->y  ) && hit.get(pt->x+1,pt->y  )==0) { newer.push_back(DemPt(pt->x+1,pt->y  )); hit(pt->x+1,pt->y  )=1; }
				e = workingElev.get(pt->x  ,pt->y-1); if (e != NO_DATA && IsCoastal(workingElev, pt->x  ,pt->y-1) && hit.get(pt->x  ,pt->y-1)==0) { newer.push_back(DemPt(pt->x  ,pt->y-1)); hit(pt->x  ,pt->y-1)=1; }
				e = workingElev.get(pt->x  ,pt->y+1); if (e != NO_DATA && IsCoastal(workingElev, pt->x  ,pt->y+1) && hit.get(pt->x  ,pt->y+1)==0) { newer.push_back(DemPt(pt->x  ,pt->y+1)); hit(pt->x  ,pt->y+1)=1; }
			}
		}
		coastal.swap(newer);
	} while (!coastal.empty());

	for (y = y1; y < y2; ++y)
	for (x = x1; x < x2; ++x)
	{
		e = workingElev.get(x,y);
		if (e != NO_DATA)
			wetElev(x,y) = e;
	}	
}

void	CorrectWaterBodies(Pmwx& inMap, DEMGeoMap& dems, ProgressFunc inProg)
{
	set<GISFace *>	water;
	for (Pmwx::Face_iterator f = inMap.faces_begin(); f != inMap.faces_end(); ++f)
	if (!f->is_unbounded())
	if (f->IsWater())
		water.insert(f);
		
	const DEMGeo& elev(dems[dem_Elevation]);

	DEMGeo	new_wet(elev.mWidth, elev.mHeight);
	new_wet.copy_geo_from(elev);
	new_wet = NO_DATA;
	
	int ctr = 0;
//	StElapsedTime	oceans("oceans");
	
	int total = water.size();
	PROGRESS_START(inProg, 0, 1, "Editing Coastlines...")
	
	while (!water.empty())
	{
		PROGRESS_SHOW(inProg, 0, 1, "Editing Coastlines...", (total - water.size()), total)
		set<GISFace *> group;
		FindConnectedWetFaces(*water.begin(), group);
		++ctr;
		BuildCorrectedWaterBody(elev, new_wet, group);
		for (set<GISFace *>::iterator g = group.begin(); g != group.end(); ++g)
			water.erase(*g);
	}	
	
	new_wet.swap(dems[dem_HydroElevation]);
	PROGRESS_DONE(inProg, 0, 1, "Editing Coastlines...")
}

/******************************************************************************************************************************
 * SHAPE-FILE BASED CORRECTION
 ******************************************************************************************************************************/

void	UpdateWaterWithMaskFile(Pmwx& inMap, DEMGeoMap& dems, const char * maskFile, ProgressFunc inProg)
{
	const DEMGeo& elev(dems[dem_Elevation]);

	DEMGeo	new_wet(elev.mWidth, elev.mHeight);
	new_wet.copy_geo_from(elev);
	new_wet = NO_DATA;

	DEMGeo	old_wet(elev.mWidth, elev.mHeight);
	old_wet.copy_geo_from(elev);
	old_wet = NO_DATA;

	DEMGeo	wetness(elev.mWidth, elev.mHeight);
	wetness.copy_geo_from(elev);

//	DEMGeo	relief(elev.mWidth, elev.mHeight);
//	relief.copy_geo_from(elev);

	/************************************************************************************
	 * RASTERIZE MASK FILE
	 ************************************************************************************/

	int x,y;
	MFMemFile * mem_file = MemFile_Open(maskFile);
	if (mem_file)
	{
		const char * p = MemFile_GetBegin(mem_file);
		const char * e = MemFile_GetEnd(mem_file);
		Assert((e - p) == (new_wet.mWidth * new_wet.mHeight));
		for (y = 0; y < new_wet.mHeight; ++y)
		for (x = 0; x < new_wet.mWidth ; ++x)
		if (*p++)
			new_wet(x,y) = elev.get(x,y);
		MemFile_Close(mem_file);
	}

	/************************************************************************************
	 * RASTERIZE OLD VMAP0 DATA, BUT CORRECT WATER BODIES TO NOT CLIMB UP MOUNTAINS!
	 ************************************************************************************/


	set<GISFace *>	water;
	for (Pmwx::Face_iterator f = inMap.faces_begin(); f != inMap.faces_end(); ++f)
	if (!f->is_unbounded())
	if (f->IsWater())
		water.insert(f);
		
	while (!water.empty())
	{
		set<GISFace *> group;
		FindConnectedWetFaces(*water.begin(), group);
		BuildCorrectedWaterBody(elev, old_wet, group);
		for (set<GISFace *>::iterator g = group.begin(); g != group.end(); ++g)
			water.erase(*g);
	}	
	
	/************************************************************************************
	 * OR JUST A STRAIGHT RENDER OF VMAP0?
	 ************************************************************************************/

#if 0
	PolyRasterizer	raster_old;
	SetupRasterizerForDEM(edges, old_wet, raster_old);	

	y = 0;
	raster_old.StartScanline(y);
	while (!raster_old.DoneScan())
	{
		while (raster_old.GetRange(rx1, rx2))
		{
			for (x = rx1; x < rx2; ++x)
			{
				e = elev.get(x,y);
				DebugAssert(e != NO_DATA);	// We expect the DEM to be filled in.
				old_wet(x,y) = e;
			}
		}
		++y;
		if (y >= elev.mHeight) 
			break;
		raster_old.AdvanceScanline(y);		
	}
	
#endif	

	/************************************************************************************
	 * COMBINE RESULTS
	 ************************************************************************************/
	
	int dx, dy;
	float e;
	for (y = 0; y < elev.mHeight ; y++)
	for (x = 0; x < elev.mWidth  ; x++)
	if (new_wet.get(x,y) == NO_DATA && 
		old_wet.get(x,y) != NO_DATA)
	{
		double	bmin, bmax;
		bmin = bmax = elev.get(x,y);
		for (dy = y-VMAP_RELIEF_RANGE; dy <= y+VMAP_RELIEF_RANGE; ++dy)
		for (dx = x-VMAP_RELIEF_RANGE; dx <= x+VMAP_RELIEF_RANGE; ++dx)
		{
			e = elev.get(dx,dy);
			bmin = MIN_NODATA(bmin, e);
			
			// We only consider points LOWER than us.  We are trying to find the span of water bodies, which will generally be the lowest point.  So only consider
			// for a point P that its surroundings go down.  (If P has a lot of stuff above it, so what - it can still be water - it is surrounded by cliffs!)
//			bmax = MAX_NODATA(bmax, e);
		}
		if ((bmax-bmin) > VMAP_TOO_STEEP && old_wet.get(x,y) != NO_DATA)
		{
			old_wet(x,y) = NO_DATA;
			wetness(x,y) = 1.0;
			continue;
		}
//		relief(x,y) = bmax-bmin;
	
		int	c_old = 0, c_new = 0;
		for (dy = -SRTM_CHOICE_BLOCK_SIZE; dy <= SRTM_CHOICE_BLOCK_SIZE; ++dy)
		for (dx = -SRTM_CHOICE_BLOCK_SIZE; dx <= SRTM_CHOICE_BLOCK_SIZE; ++dx)
		{
			if (old_wet.get(x+dx,y+dy) != NO_DATA) ++c_old;
			if (new_wet.get(x+dx,y+dy) != NO_DATA) ++c_new;
		}
		double rat = (double) c_new   / (double) ((SRTM_CHOICE_BLOCK_SIZE*2+1) * (SRTM_CHOICE_BLOCK_SIZE*2+1));
		wetness(x,y) = rat;
	} else
		wetness(x,y) = 1.0;

	for (y = 0; y < elev.mHeight ; ++y)
	for (x = 0; x < elev.mWidth  ; ++x)
	if (wetness(x,y) < SRTM_TRUSTED_WETNESS && new_wet.get(x,y) == NO_DATA)
	{
		new_wet(x,y) = old_wet(x,y);
	}
	
	new_wet.swap(dems[dem_HydroElevation]);	
	wetness.swap(dems[dem_Wizard]);
}


/******************************************************************************************************************************
 * MASTER HYDRO RECONSTRUCTION FOR GLOBAL DATA
 ******************************************************************************************************************************/

#pragma mark -

void	ConformWater(DEMGeoMap& dems, bool inWrite)
{
	DEMGeo&	water_elev(dems[dem_HydroElevation]);
	char	fname_left[1024], fname_bot[1024], fname_self[1024];

	string border_loc = HYDRO_BORDER;
#if APL && !defined(__MACH__)
	string	appP;
	AppPath(appP);
	string::size_type b = appP.rfind(':');
	appP.erase(b+1);
	border_loc = appP + border_loc;
#endif

	sprintf(fname_self,"%s%s%+03d%+04d.hydro.txt", border_loc.c_str(), DIR_STR, (int) (water_elev.mSouth), (int) (water_elev.mWest));
	sprintf(fname_left,"%s%s%+03d%+04d.hydro.txt", border_loc.c_str(), DIR_STR, (int) (water_elev.mSouth), (int) (water_elev.mWest - 1));
	sprintf(fname_bot ,"%s%s%+03d%+04d.hydro.txt", border_loc.c_str(), DIR_STR, (int) (water_elev.mSouth - 1), (int) (water_elev.mWest));
	FILE * fi;
	int n, w, h;
	float e;
	if (inWrite)
	{
		fi = fopen(fname_self, "w");
		if (fi)
		{
			fprintf(fi,"%d,%d\n", water_elev.mWidth, water_elev.mHeight);
			for (n = 0; n < water_elev.mWidth; ++n)
			{
				e = water_elev.get(n, water_elev.mHeight-1);
				fprintf(fi, "%f\n", e);
			}
			for (n = 0; n < water_elev.mHeight; ++n)
			{
				e = water_elev.get(water_elev.mWidth-1, n);
				fprintf(fi, "%f\n", e);
			}
			fclose(fi);
		} else
			AssertPrintf("Unable to open file %s for write.", fname_self);
	} else {
		fi = fopen(fname_bot, "r");
		if (fi)
		{	
			if (fscanf(fi, "%d,%d", &w, &h) == 2)
			{
				if (w == water_elev.mWidth)
				{
					for (n = 0; n < water_elev.mWidth; ++n)
					{
						if (fscanf(fi, "%f", &e) == 1)
							water_elev(n, 0) = e;
					}
				}
			}
			fclose(fi);
		}
		
		fi = fopen(fname_left, "r");
		if (fi)
		{	
			if (fscanf(fi, "%d,%d", &w, &h) == 2)
			{
				if (h == water_elev.mHeight)
				{
					for (n = 0; n < water_elev.mWidth; ++n)
					{
						fscanf(fi, "%f", &e);							
					}
					for (n = 0; n < water_elev.mHeight; ++n)
					{
						if (fscanf(fi, "%f", &e) == 1)
							water_elev(0, n) = e;
					}					
				}
			}
			fclose(fi);
		}
	}
}

void	HydroReconstruct(Pmwx& ioMap, DEMGeoMap& ioDem, const char * mask_file, ProgressFunc inFunc)
{
	UpdateWaterWithMaskFile(ioMap, ioDem, mask_file, inFunc);
	ConformWater(ioDem, false);
	BuildRivers		  (ioMap, ioDem, inFunc);
	DEMGeo foo(ioDem[dem_HydroElevation]), bar;
	InterpDoubleDEM(foo, bar);
	ReduceToBorder(bar, foo);

	Pmwx	water;
	water.unbounded_face()->mTerrainType = terrain_Natural;
	DemToVector(foo, water, false, terrain_Water, inFunc);
	
	water.insert_edge(Point2(foo.mWest, foo.mSouth), Point2(foo.mEast, foo.mSouth), NULL, NULL);
	water.insert_edge(Point2(foo.mWest, foo.mNorth), Point2(foo.mEast, foo.mNorth), NULL, NULL);
	water.insert_edge(Point2(foo.mWest, foo.mSouth), Point2(foo.mWest, foo.mNorth), NULL, NULL);
	water.insert_edge(Point2(foo.mEast, foo.mSouth), Point2(foo.mEast, foo.mNorth), NULL, NULL);
	water.unbounded_face()->mTerrainType = terrain_Natural;

	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	if (!f->is_unbounded())
	if (f->IsWater())
	{
		f->mTerrainType = terrain_Natural;
	}
	
	SimplifyMap(ioMap, true);
	TopoIntegrateMaps(&ioMap, &water);
	MergeMaps(water, ioMap, true, NULL, true, inFunc);
	ioMap.swap(water);
	
	ConformWater(ioDem, true);	
}

/******************************************************************************************************************************
 * COASTLINE SIMPLIFICATION FOR US XES
 ******************************************************************************************************************************/

#pragma mark -

#define SLIVER_PROTECTION	0.984807753012

// UTILITY:
// Returns true if there is no edge adjacent to (and pointing to) V that
// is nearly incident with vec (vec is revsersed by the bool param).
inline bool NoHalfedgeInDir(GISVertex * v, const Vector2& vec, bool reverse)
{
	double mul = reverse ? -1.0 : 1.0;
	Pmwx::Halfedge_around_vertex_circulator stop, iter;
	iter = stop = v->incident_halfedges();
	do {
		Vector2		iv(iter->source()->point(), iter->target()->point());
		iv.normalize();
		if ((mul * vec.dot(iv)) > SLIVER_PROTECTION) return false;
		++iter;
	} while (iter != stop);
	return true;
}


// Utility - tracks the edge we added.
void insert_add_one(GISHalfedge * oh, GISHalfedge * nh, void * ref)
{
	GISHalfedge ** p = (GISHalfedge **) ref;
	DebugAssert(nh == NULL || oh == NULL);
	DebugAssert(*p == NULL);
	if (nh != NULL)
		*p = nh;
}

// This constant controls the area of a polygon we might simplify.
#define MIN_FACE_SIZE	(500 * 500)

// The goal of this function is to go in and remove sharp triangular stuff
// from fairly big polygons.
void	OLD_SimplifyCoastlines(Pmwx& ioMap, double max_annex_area, ProgressFunc func)
{
	SimplifyMap(ioMap, false);
	Pmwx::Halfedge_iterator he;
	GISHalfedge * next;
	bool	did_work;
	int nuke = 0;
	int total = ioMap.number_of_halfedges();
	
	PROGRESS_START(func, 0, 1, "Simplifying coastlines...")
	for (Pmwx::Face_iterator face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face)
	if (!face->is_unbounded())
	{
		face->mTemp1 = GetMapFaceAreaMeters(face);
	}	

	do {
		// One cycle through: try to identify and cut off unneeded points.
	
		set <GISHalfedge *>	nuke_he;
		did_work = false;	
		int ctr = 0;
		
		// Start by finding any halfedge that's water on the left, land on the right, not border.
		for (he = ioMap.halfedges_begin(); he != ioMap.halfedges_end(); ++he, ++ctr)
		if (!he->face()->is_unbounded() && !he->twin()->face()->is_unbounded())		
		if (he->face()->mTerrainType == terrain_Water && he->twin()->face()->mTerrainType != terrain_Water)
		{
			// Find the next segment (follow the water curve - this implies we're simplified.)  We must terrain match.
			next = he->next();
			if (next->face()->mTerrainType == he->face()->mTerrainType && next->twin()->face()->mTerrainType == he->twin()->face()->mTerrainType)
			{
				// Calculate the "area" this curve cuts out...if it's small enoguh and our face is big enough, maybe we can simplify.
				DebugAssert(he->twin() != next);
				DebugAssert(he != next->twin());
				Vector2	v1(he->source()->point(), he->target()->point());
				Vector2	v2(next->source()->point(), next->target()->point());
				double sa = v1.signed_area(v2);
				if (sa > -max_annex_area && sa < max_annex_area)
				if ((sa > 0 && he->face()->mTemp1 		  > MIN_FACE_SIZE) ||
					(sa < 0 && he->twin()->face()->mTemp1 > MIN_FACE_SIZE))
				{
					// Look at our angle - are we a sharp enough turn to warrant cutting this edge off?
					v1.normalize();
					v2.normalize();
					if (v1.dot(v2) < -0.5)
					{
						// This was a check to avoid slivering ,but, um, it's gone.
						Vector2	new_dir(he->source()->point(), next->target()->point());
						new_dir.normalize();
//							if (NoHalfedgeInDir(he->source(), new_dir, true))
//							if (NoHalfedgeInDir(next->target(), new_dir, false))
						{
						
							// Ray test - if we can't fire a ray, this isn't a clear curve to cut off!
							Pmwx::Locate_type	loc;
							Point2				pt;
							GISHalfedge * rs = ioMap.ray_shoot(he->source()->point(), Pmwx::locate_Vertex, he->twin(), next->target()->point(), pt, loc);
							if (loc == Pmwx::locate_Vertex && rs->target() == next->target())
							{
								// Do the actual insertion and see if there wasn't an edge there
								DebugAssert(pt == next->target()->point());
								GISHalfedge * ne = NULL;
								ioMap.insert_edge(he->source()->point(), next->target()->point(), he->twin(), Pmwx::locate_Vertex, insert_add_one, &ne);
								if (ne != NULL)
								{
									// If we did add an edge match terrain and mark the segment for deletion.
									if (sa > 0.0)
									{
										ne->twin()->face()->mTerrainType = he->twin()->face()->mTerrainType;
										ne->twin()->face()->mAreaFeature = he->twin()->face()->mAreaFeature;
									} else {
										ne->face()->mTerrainType = he->face()->mTerrainType;
										ne->face()->mAreaFeature = he->face()->mAreaFeature;
									}
									
									if (next->mSegments.empty())
										nuke_he.insert(next->mDominant ? next : next->twin());

									if (he->mSegments.empty())
										nuke_he.insert(he->mDominant ? he : he->twin());
										
									PROGRESS_SHOW(func, 0, 1, "Simplifying coastlines...", ctr, total)
										
									did_work = true;
									++nuke;
								}
							}
						}
					}
				}
			}
		}
		
		// Now nuke anyone we got rid of and recalc face sizes.		
		for (set<GISHalfedge *>::iterator nuke_e = nuke_he.begin(); nuke_e != nuke_he.end(); ++nuke_e)
		{
//			gMeshLines.push_back((*nuke_e)->source()->point());
//			gMeshLines.push_back((*nuke_e)->target()->point());
			
			GISFace * a = (*nuke_e)->face();
			GISFace * b = (*nuke_e)->twin()->face();
			
			GISFace * d = ioMap.remove_edge(*nuke_e);

			if (a != d)	a->mTemp1 = GetMapFaceAreaMeters(a);
			if (b != d)	b->mTemp1 = GetMapFaceAreaMeters(b);
			
		}
			
	} while (did_work);
	
	// clean up any additional map crap??
	SimplifyMap(ioMap, false);
	PROGRESS_DONE(func, 0, 1, "Simplifying coastlines...")
	printf("End result: %d simplifies, %d before, %d after.\n", nuke, total, ioMap.number_of_halfedges());
}

inline void SubBox(const Bbox2& src, float x1, float y1, float x2, float y2, Bbox2& dst)
{
	Vector2 sz(src.p1,src.p2);
	dst.p1.x = src.p1.x + sz.dx * x1;
	dst.p1.y = src.p1.y + sz.dy * y1;
	dst.p2.x = src.p1.x + sz.dx * x2;
	dst.p2.y = src.p1.y + sz.dy * y2;
}

inline int SubBoxIdx(const Bbox2& lim, const Point2& p, int div)
{
	float dx = (p.x - lim.p1.x) / (lim.p2.x - lim.p1.x);
	float dy = (p.y - lim.p1.y) / (lim.p2.y - lim.p1.y);
	
	dx *= (float) div;
	dy *= (float) div;
	
	int idx = max(0, min((int) dx, div-1));
	int idy = max(0, min((int) dy, div-1));
	
	return idx + idy * div;
}

static void BuildPtCache(Pmwx& inMap, const Bbox2& lim, int divs, vector<pair<Bbox2, vector<Point2> > >& outCache)
{
	outCache.clear();
	vector<pair<Bbox2, int> > 		index;
	index.resize(divs * divs);
	outCache.reserve(divs * divs);
	for (int y = 0; y < divs; ++y)
	for (int x = 0; x < divs; ++x)
	{
		int idx = y * divs + x;
		SubBox(lim, (float) x / (float) divs, (float) y / (float) divs, (float) (x+1) / (float) divs, (float) (y+1) / (float) divs, index[idx].first);
		index[idx].second = -1;
	}

	for (Pmwx::Vertex_iterator i = inMap.vertices_begin(); i != inMap.vertices_end(); ++i)
	{
		Point2 p = i->point();
		
		int idx = SubBoxIdx(lim, p, divs);
		if (index[idx].second == -1)
		{
			idx = index[idx].second = outCache.size();
			outCache.push_back(pair<Bbox2, vector<Point2> >());
			outCache[idx].first = p;
			outCache[idx].second.push_back(p);				
		} else {
			idx = index[idx].second;
			outCache[idx].first += p;
			outCache[idx].second.push_back(p);
		}
	}
	
#if DEV
	for (vector<pair<Bbox2, vector<Point2> > >::iterator zone = outCache.begin(); zone != outCache.end(); ++zone)
	for (vector<Point2>::iterator p = zone->second.begin(); p != zone->second.end(); ++p)
		DebugAssert(zone->first.contains(*p));
#endif	
}


static bool	AnyPtInEdgeSpaceCCB(GISHalfedge * e1, GISHalfedge * e2, vector<pair<Bbox2, vector<Point2> > >& cache)
{
	DebugAssert(e1->next() == e2);
	if (e2->next() == e1) return true;
	
	Bbox2	lim(e1->source()->point());
	lim += e1->target()->point();
	lim += e2->target()->point();

	Segment2	s1(e1->source()->point(), e1->target()->point());
	Segment2	s2(e2->source()->point(), e2->target()->point());
	Segment2	s3(e2->target()->point(), e1->source()->point());
	
	DebugAssert(Vector2(s1.p1,s1.p2).signed_area(Vector2(s2.p1,s2.p2)) > 0.0);
	
	Pmwx::Ccb_halfedge_circulator circ, stop;
	
	for (vector<pair<Bbox2, vector<Point2> > >::iterator zone = cache.begin(); zone != cache.end(); ++zone)
	if (lim.overlap(zone->first))
	{
		for (vector<Point2>::iterator pp = zone->second.begin(); pp != zone->second.end(); ++pp)
		{
			Point2 p(*pp);
			if (lim.contains(p))
			if (!s1.on_right_side(p) &&
				!s2.on_right_side(p) &&
				!s3.on_right_side(p))
			if (p != s1.p1 && p != s2.p1 && p != s3.p1)
			{
				return true;
			}			
		}
	}

	return false;
}


void	SimplifyWaterCCB(Pmwx& ioMap, GISHalfedge * edge, vector<pair<Bbox2, vector<Point2> > >&		cache)
{
	bool	is_split = false;
	bool	first_split = false;
	GISHalfedge * stop = edge;	
	bool	first_loop = true;
	bool	last_loop = false;
	do {
	
		DebugAssert(edge->face()->IsWater());
		DebugAssert(!edge->face()->is_unbounded());
	
		if (edge->twin()->face()->is_unbounded() || 
			edge->next()->twin()->face()->is_unbounded() || 
			edge->next()->twin()->next()->twin() != edge ||
			edge->twin()->face()->IsWater())
		{
			edge = edge->next();
			is_split = false;
		} else {
			
			// We want to make sure E is the edge after the split from the first edge.
			if (!is_split)
			{
				edge = ioMap.split_edge(edge, Segment2(edge->source()->point(),edge->target()->point()).midpoint())->next();
			}
			
			if (edge->next() == stop) last_loop = true;
			
			if (!first_split || edge->next() != stop)
				ioMap.split_edge(edge->next(), Segment2(edge->next()->source()->point(),edge->next()->target()->point()).midpoint());

//			Point2				cross_pt;
//			Pmwx::Locate_type	cross_type;

			GISVertex * src_split = edge->source();
			GISVertex * dst_split = edge->next()->target();

			Point2	pt_a = src_split->point();
			Point2	pt_b = edge->target()->point();
			Point2	pt_c = dst_split->point();

			Vector2	v1(pt_a, pt_b);
			Vector2 v2(pt_b, pt_c);
			bool	is_left = v1.left_turn(v2);
			v1.normalize();
			v2.normalize();

			if (v1.dot(v2) < 0.99)
			{
				bool is_left = Vector2(pt_a, pt_b).left_turn(Vector2(pt_b, pt_c));
				bool occupied = is_left ? AnyPtInEdgeSpaceCCB(edge, edge->next(), cache) : AnyPtInEdgeSpaceCCB(edge->next()->twin(), edge->next()->twin()->next(), cache);
			
				if (!occupied)
				{
					ioMap.merge_edges(edge, edge->next());
					is_split = true;
					edge=edge->next();
				/*
					bool	is_left = Vector2(pt_a, pt_b).left_turn(Vector2(pt_b, pt_c));
					
					int terrain_new = is_left ? edge->twin()->face()->mTerrainType : edge->face()->mTerrainType;

					DebugAssert(cross_pt == pt_c);
					
					GISHalfedge * new_edge;
					if (is_left)
						new_edge = ioMap.nox_insert_edge_between_vertices(dst_split, src_split, is_outer_ccb && is_left)->twin();
					else
						new_edge = ioMap.nox_insert_edge_between_vertices(src_split, dst_split, false);

					if (is_left) new_edge->twin()->face()->mTerrainType = terrain_new;
					else		 new_edge->face()->mTerrainType = terrain_new;

//					gMeshLines.push_back(edge->next()->source()->point());
//					gMeshLines.push_back(edge->next()->target()->point());
//					gMeshLines.push_back(edge->source()->point());
//					gMeshLines.push_back(edge->target()->point());					
					
					ioMap.remove_edge(edge->next());
					ioMap.remove_edge(edge);
					is_split = true;
					edge = new_edge->next();
					if (first_loop) first_split = true;
*/					
				} else {
					ioMap.merge_edges(edge->next(), edge->next()->next());
					if (!is_split)
					{
						edge = edge->twin()->next()->twin();
						ioMap.merge_edges(edge, edge->next());
					}
					else
						is_split = false;
					edge = edge->next();
				}

			} else {
			
				ioMap.merge_edges(edge->next(), edge->next()->next());
				if (!is_split)
				{
					edge = edge->twin()->next()->twin();
					ioMap.merge_edges(edge, edge->next());
				}
				else
					is_split = false;
				edge = edge->next();
			}
		
		}
	
		first_loop = false;
	
	} while (edge != stop && !last_loop);
}

void	SimplifyCoastlineFace(Pmwx& ioMap, GISFace * face, vector<pair<Bbox2, vector<Point2> > >& cache)
{
	Bbox2	lim;

	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = face->outer_ccb();
	lim = circ->target()->point();
	do {
		lim += circ->target()->point();
		++circ;
	} while (circ != stop);

	SimplifyWaterCCB(ioMap,face->outer_ccb(), cache);

	set<GISHalfedge *> ee;

	face->copy_holes(ee);			
	
	for (set<GISHalfedge *>::iterator e = ee.begin(); e != ee.end(); ++e)
	{
		SimplifyWaterCCB(ioMap,*e, cache);
	}
}


void	SimplifyCoastlines(Pmwx& ioMap, const Bbox2& bounds, ProgressFunc func)
{
	set<GISFace *>	water;
	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	if (!f->is_unbounded())
	if (f->IsWater())
		water.insert(f);			

	vector<pair<Bbox2, vector<Point2> > >		cache;
	BuildPtCache(ioMap, bounds, 16, cache);
		
	PROGRESS_START(func, 0, 1, "Smoothing coastlines");
	int ctr = 0;
	
	for (set<GISFace *>::iterator i = water.begin(); i != water.end(); ++i, ++ctr)
	{
		PROGRESS_CHECK(func, 0, 1, "Smoothing coastlines", ctr, water.size(), water.size() / 200);
		SimplifyCoastlineFace(ioMap, *i, cache);

	}
	PROGRESS_DONE(func, 0, 1, "Smoothing coastlines");
}


#pragma mark -

// Sets the included area of the shapefile to '1'.
bool	ShapeFileToBoolDem(const char * inShapeFile, DEMGeo& elev)
{
	SHPHandle file = SHPOpen(inShapeFile, "rb");
	if (file == NULL)	return false;
	int	entityCount, shapeType;
	int x, y, rx1, rx2;
	double	bounds_lo[4], bounds_hi[4];
	
	Bbox2	dem_limit(elev.mWest, elev.mSouth, elev.mEast, elev.mNorth);
	
	SHPGetInfo(file, &entityCount, &shapeType, bounds_lo, bounds_hi);
	
	PolyRasterizer	raster;

	for (int n = 0; n < entityCount; ++n)
	{
		SHPObject * obj = SHPReadObject(file, n);

		if (obj->nSHPType == SHPT_POLYGONZ || obj->nSHPType == SHPT_POLYGON || obj->nSHPType == SHPT_POLYGONM)
		if (obj->nVertices > 0)
		{
			Bbox2	shape_limit(Point2(obj->padfX[0],obj->padfY[0]));
			for (int pt = 1; pt < obj->nVertices; ++pt)
			{
				shape_limit += Point2(obj->padfX[pt],obj->padfY[pt]);
			}
		
			if (dem_limit.overlap(shape_limit))
			for (int part = 0; part < obj->nParts; ++part)
			{
				int start_idx = obj->panPartStart[part];
				int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
				Polygon2 pts(stop_idx - start_idx);
				for (int index = start_idx; index < stop_idx; ++index)
				{
					pts[index-start_idx] = Point2(obj->padfX[index],obj->padfY[index]);
				}
				DebugAssert(pts.front() == pts.back());
				pts.pop_back();
				
				for (int i = 0; i < pts.size(); ++i)
				{
					int j = (i + 1) % pts.size();
					double x1 = elev.lon_to_x(pts[i].x);
					double y1 = elev.lat_to_y(pts[i].y);
					double x2 = elev.lon_to_x(pts[j].x);
					double y2 = elev.lat_to_y(pts[j].y);

					if (y1 != y2)
					{
						if (y1 < y2)
							raster.masters.push_back(PolyRasterSeg_t(x1,y1,x2,y2));
						else
							raster.masters.push_back(PolyRasterSeg_t(x2,y2,x1,y1));
					}
				}
			}
		} 

		SHPDestroyObject(obj);	
	}	
	SHPClose(file);
	
	raster.SortMasters();
	
	y = 0;
	raster.StartScanline(y);
	while (!raster.DoneScan())
	{
		while (raster.GetRange(rx1, rx2))
		{
			for (x = max(0,rx1); (x < rx2 && x < elev.mWidth); ++x)
			{
				elev(x,y) = 1;
			}
		}
		++y;
		if (y >= elev.mHeight) 
			break;
		raster.AdvanceScanline(y);		
	}
	return true;
}

bool	MakeWetMask(const char * inShapeDir, int lon, int lat, const char * inMaskDir)
{
	int n, x, y;
	DEMGeo	wet_mask(1201, 1201);
	wet_mask.mWest  = lon  ;
	wet_mask.mEast  = lon+1;
	wet_mask.mSouth = lat  ;
	wet_mask.mNorth = lat+1;
	
	char	fname_me[1024];
	char	fname_r [1024];
	char	fname_t [1024];
	char	fname_l [1024];
	char	fname_b [1024];
	char	fname_tr[1024];
	char	fname_tl[1024];
	char	fname_br[1024];
	char	fname_bl[1024];
	
	sprintf(fname_me,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat  ),  latlon_bucket(lon  ), DIR_STR, lat  , lon  );

	sprintf(fname_r ,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat  ),  latlon_bucket(lon+1), DIR_STR, lat  , lon+1);
	sprintf(fname_t ,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat+1),  latlon_bucket(lon  ), DIR_STR, lat+1, lon  );
	sprintf(fname_l ,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat  ),  latlon_bucket(lon-1), DIR_STR, lat  , lon-1);
	sprintf(fname_b ,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat-1),  latlon_bucket(lon  ), DIR_STR, lat-1, lon  );

	sprintf(fname_tr,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat+1),  latlon_bucket(lon+1), DIR_STR, lat+1, lon+1);
	sprintf(fname_tl,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat+1),  latlon_bucket(lon-1), DIR_STR, lat+1, lon-1);
	sprintf(fname_br,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat-1),  latlon_bucket(lon+1), DIR_STR, lat-1, lon+1);
	sprintf(fname_bl,"%s%+03d%+04d%s%+03d%+04d.shp", inShapeDir,latlon_bucket(lat-1),  latlon_bucket(lon-1), DIR_STR, lat-1, lon-1);

	bool got_me = ShapeFileToBoolDem(fname_me, wet_mask);
	if (got_me)
	{
		bool got_r  = ShapeFileToBoolDem(fname_r , wet_mask);
		bool got_t  = ShapeFileToBoolDem(fname_t , wet_mask);
		bool got_b  = ShapeFileToBoolDem(fname_b , wet_mask);
		bool got_l  = ShapeFileToBoolDem(fname_l , wet_mask);

		bool got_tr = ShapeFileToBoolDem(fname_tr, wet_mask);
		bool got_tl = ShapeFileToBoolDem(fname_tl, wet_mask);
		bool got_br = ShapeFileToBoolDem(fname_br, wet_mask);
		bool got_bl = ShapeFileToBoolDem(fname_bl, wet_mask);

		if (!got_r)
		for (n = 1; n < 1200; ++n)
			wet_mask(1200,n) = wet_mask.get(1199,n);

		if (!got_l)
		for (n = 1; n < 1200; ++n)
			wet_mask(0,n) = wet_mask.get(1,n);
		
		if (!got_t)
		for (n = 1; n < 1200; ++n)
			wet_mask(n,1200) = wet_mask.get(n,1199);

		if (!got_b)
		for (n = 1; n < 1200; ++n)
			wet_mask(n,0) = wet_mask.get(n,1);
		
		if (!got_tr)
		{
				 if (got_t)	wet_mask(1200,1200) = wet_mask(1199, 1200);
			else if (got_r) wet_mask(1200,1200) = wet_mask(1200, 1199);
			else 			wet_mask(1200,1200) = wet_mask(1199, 1199);
		}

		if (!got_tl)
		{
				 if (got_t)	wet_mask(0,1200) = wet_mask(1, 1200);
			else if (got_l) wet_mask(0,1200) = wet_mask(0, 1199);
			else 			wet_mask(0,1200) = wet_mask(1, 1199);
		}

		if (!got_br)
		{
				 if (got_b)	wet_mask(1200,0) = wet_mask(1199, 0);
			else if (got_r) wet_mask(1200,0) = wet_mask(1200, 1);
			else 			wet_mask(1200,0) = wet_mask(1199, 1);
		}

		if (!got_bl)
		{
				 if (got_b)	wet_mask(0,0) = wet_mask(1, 0);
			else if (got_l) wet_mask(0,0) = wet_mask(0, 1);
			else 			wet_mask(0,0) = wet_mask(1, 1);
		}


		
		char	final[1024], internal[1024];
		sprintf(final,"%s%+03d%+04d%s%+03d%+04d.mask.zip", inMaskDir, latlon_bucket(lat  ), latlon_bucket(lon  ), DIR_STR, lat  , lon  );
		sprintf(internal,"%+03d%+04d.mask", lat  , lon  );
		printf("Writing: %s\n", final);
		
		char	buf[1201*1201];
		for (y = 0; y < 1201; ++y)
		for (x = 0; x < 1201; ++x)
			buf[x+y*1201] = wet_mask.get(x,y) > 0.0 ? 1 : 0;
		
		ZipFileWriter	writer(final, internal, platform_BigEndian);
		writer.WriteBulk(buf, sizeof(buf), false);
		
	}
	
	return got_me;
}	

