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

#include "TensorRoads.h"
#include "MapDefs.h"
#include "RF_DEMGraphics.h"
#include "DEMDefs.h"
#include "MapAlgs.h"
#include "MapOverlay.h"
#include "MapTopology.h"
#include "PolyRasterUtils.h"
#include "TensorUtils.h"
#include "MathUtils.h"
#include "ParamDefs.h"
#include "GISTool_Globals.h"
#include "PerfUtils.h"
#include "MapCreate.h"
#define XUTILS_EXCLUDE_MAC_CRAP
#include "XUtils.h"

#define	ADVANCE_RATIO 0.0005

#define	PROFILE_PERFORMANCE 1

#if PROFILE_PERFORMANCE
#define TIMER(x)	StElapsedTime	__PerfTimer##x(#x);
#else
#define TIMER(x)
#endif

// SEt this to 1 to see the road restriction DEM - can explain why no roads showed up in a location.
#define SHOW_ROAD_RESTRICT 0

// Make roads EVEN if origin code says not to.  Useful for testing maybe.
#define IGNORE_ORIGIN_CODES 0

// Show roads as debug mesh lines
#define SHOW_GENERATED_ROADS 0

// Ignore urban density - useful for debugging
#define IGNORE_DENSITY 1


/*

	Back before we had OSM, TensorRoads generated the "fake" road grids for urban areas where we lacked true vector data.  This came from
	an academic paper.  Here are the major ideas:
	
	- A "tensor field is a "flow field" - that is, a field of 2-d vector directions.
	- The idea was two-step: make a flow field that loosel corresponds to how we want the road grid to "flow", then turn the tensor field into
	  roads.
	  
	TENSOR FIELDS
	
	A tensor field is a field of 3-d rotations - its "flow" is bent by the rotations.
	
	What's cool about tensors (as opposed to a normal 2-d field of vectors indicating direction, or a 2-d field of angles) is that tensor
	fields are affine transforms and thus we can add them together and scale them.  In other words, the same kinds of "blending" that would
	make an image look good produces VERY reasonable and sane results for tensor fields.
	
	Furthermore, tensor fields can be defined functionally - thus we can do a "composition of functions" (with the functions weighted.
	
	In fact, that is exactly what we do: we can build a tensor out of:
	- The terrain gradient, which will make the road grid follow the terrain.
	- Linear or circular pre-defined functions, which may have attenuation.
	- We can build linear gradients that scale off known edges to enforce the grid along highways.
	
	With the right blending gradients, we get a reasonably sane tensor map that seems to reflect 'local stuff'.
	
	TENSOR 2 ROADS
	
	The way we convert our tensor field is pretty easy.  We create "seed" points along known highways (seed roads) and at those seed points
	we make a small step either along or normal to the flow.  We then drop a seed for our cross-street and keep going.
	
	The result will be a sort of emergent grid that bends around the natural "flow" of the tensor field.  If our tensor field is aesthetically
	pleasing (in otherwords, fly) then the road grid won't look that bad.
	
	There are some heuristics in this seeding to try to weed out and generally sanatize the emerging edges, as well as to keep the process fast.
	For example, we use a raster mask to avoid over-adding roads.  (If the algorithm is allowed to run forever, it will eventually fill in an
	infinite number of infinitely thin lines, for certain tensors.)
	
*/

/************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************/

inline bool RoadsForThisFace(Face_handle f)
{
	return !f->data().IsWater() && f->data().mTerrainType != terrain_Airport
	#if !IGNORE_ORIGIN_CODES
	 && f->data().mParams.count(af_OriginCode)
	 &&	f->data().mParams[af_OriginCode] == 2.0
	#endif
	;
}

bool CrossCheck(const Segment2& s1, const Segment2& s2, Point2& p)
{
	if(s1.p1 == s2.p1 ||
	   s1.p1 == s2.p2 ||
	   s1.p2 == s2.p1 ||
	   s1.p2 == s2.p2)			return false;
	if(!s1.could_intersect(s2))	return false;

	if(s1.p1.y() == s2.p1.y())
	{
		DebugAssert(s1.p1.x() != s2.p2.x());
		if (s1.p1.x() < s2.p1.x())
		{
			if (s1.intersect(s2,p) && p != s1.p1 && p != s1.p2)
			{
//				printf("Generated %lf,%lf from %lf,%lf->%lf,%lf x %lf,%lf->%lf,%lf\n",p.x,p.y,
//						s1.p1.x,s1.p1.y,s1.p2.x,s1.p2.y,
//						s2.p1.x,s2.p1.y,s2.p2.x,s2.p2.y);
				return true;
			} else return false;
		}
		else
		{
			if (s2.intersect(s1,p))
			{
//				printf("Generated %lf,%lf from %lf,%lf->%lf,%lf x %lf,%lf->%lf,%lf\n",p.x,p.y,
//						s2.p1.x,s2.p1.y,s2.p2.x,s2.p2.y,
//						s1.p1.x,s1.p1.y,s1.p2.x,s1.p2.y);
				return true;
			} else return false;
		}
	}
	else
	{
		if (s1.p1.y() < s2.p1.y())
		{
			if (s1.intersect(s2,p))
			{
//				printf("Generated %lf,%lf from %lf,%lf->%lf,%lf x %lf,%lf->%lf,%lf\n",p.x,p.y,
//						s1.p1.x,s1.p1.y,s1.p2.x,s1.p2.y,
//						s2.p1.x,s2.p1.y,s2.p2.x,s2.p2.y);
				return true;
			} else return false;
		}
		else
		{
			if (s2.intersect(s1,p))
			{
//				printf("Generated %lf,%lf from %lf,%lf->%lf,%lf x %lf,%lf->%lf,%lf\n",p.x,p.y,
//						s2.p1.x,s2.p1.y,s2.p2.x,s2.p2.y,
//						s1.p1.x,s1.p1.y,s1.p2.x,s1.p2.y);
				return true;
			} else return false;
		}
	}
}

Halfedge_handle InsertOneEdge(const Point_2& p1, const Point_2& p2, Pmwx& io_map, Locator& io_locator)
{
	DebugAssert(p1 != p2);
	Vertex_handle	v1, v2;
	Face_handle		f;
	CGAL::Object	obj1 = io_locator.locate(p1);
	CGAL::Object	obj2 = io_locator.locate(p2);

	bool has_v1 = CGAL::assign(v1, obj1);
	bool has_v2 = CGAL::assign(v2, obj2);

	Curve_2 s(Segment_2(p1,p2));

	if(has_v1 && has_v2)		return io_map.insert_at_vertices(s, v1,v2);
	else if (has_v1)			return io_map.insert_from_left_vertex(s, v1);
	else if (has_v2)			return io_map.insert_from_right_vertex(s, v2);
	else if(CGAL::assign(f,obj1))	return io_map.insert_in_face_interior(s,f);
	else {
	//return CGAL::insert_curve(io_map,s);
		DebugAssert(!"Not disjoint!!");
		return Halfedge_handle();
	}
}

/*
void	SetRoadProps(Halfedge_handle e)
{
	if (!e->data().mDominant) e = e->twin();
	e->data().mSegments.push_back(GISNetworkSegment_t());
	e->data().mSegments.back().mFeatType = road_LocalUnsep;
}
*/

inline bool LessEdgesThan(Face_handle f, int c)
{
	if(f->is_unbounded()) return false;

	Pmwx::Ccb_halfedge_circulator circ,stop;
	circ = stop = f->outer_ccb();
	do {
		if(c-- <= 0) return false;
		++circ;
	} while(circ != stop);

	for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
			if(c-- <= 0) return false;
			++circ;
		} while(circ != stop);
	}
	return true;
}

void BulkZapRoads(const DEMGeo& inUrbanDensity, Pmwx& io_map)
{
	printf("BEFORE ZAP: %llu generated roads.\n",(unsigned long long)io_map.number_of_halfedges() / 2);
	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); )
	{
		if((e->source()->degree() > 2 && e->target()->degree() > 2 &&		// If we have a real intersection on both sides AND
		   e->face() != e->twin()->face() &&								// We divide two DIFFERENT faces (so we don't make an island) AND
		   LessEdgesThan(e->face(), 6) &&									// The faces aren't too complex
		   LessEdgesThan(e->twin()->face(), 6)) ||

		   (e->source()->degree() == 1 || e->target()->degree() == 1)		// Or if we're an antenna
			)
		{
			Point_2	mp(CGAL::midpoint(e->source()->point(),e->target()->point()));
			double d = inUrbanDensity.get(inUrbanDensity.lon_to_x(CGAL::to_double(mp.x())),inUrbanDensity.lat_to_y(CGAL::to_double(mp.y())));
			if(e->source()->degree() == 1 || e->target()->degree() == 1)
				d = min(d,0.5);		// always think of nuking antennas
			#if IGNORE_DENSITY
				d =1.0;
			#endif
			if(!RollDice(d))
			{
				Halfedge_handle k = e;
				++e;
				io_map.remove_edge(k);
			}
			else
				++e;
		} else
			++e;
	}
	printf("AFTER ZAP: %llu generated roads.\n",(unsigned long long)io_map.number_of_halfedges() / 2);
}

void BulkInsertRoads(vector<Segment2>	roads, Pmwx& io_map)
{
	GIS_halfedge_data	hed;
	hed.mSegments.push_back(GISNetworkSegment_t());
	hed.mSegments.back().mFeatType = road_LocalUnsep;

	vector<Segment_2>			road_vec(roads.size());
	vector<GIS_halfedge_data>	data_vec(roads.size(),hed);

	for(int n = 0; n < roads.size(); ++n)
		road_vec[n] = Segment_2(ben2cgal(roads[n].p1),ben2cgal(roads[n].p2));

	Map_CreateWithLineData(io_map, road_vec, data_vec);
}

int ThinLine(list<Point2>& pts, double max_dist_move, double max_dist_seg)
{
	max_dist_move *= max_dist_move;
	max_dist_seg *= max_dist_seg;
	int t=0;
	while(1)
	{
		list<Point2>::iterator best_dead = pts.end();
		double d_sq = 0.0f;

		for(list<Point2>::iterator i = pts.begin(); i != pts.end(); ++i)
		{
			if(i != pts.begin())
			{
				list<Point2>::iterator p(i), n(i);
				--p;
				++n;
				if(n != pts.end())
				{
					Segment2 span(*p, *n);
					if(span.squared_length() < max_dist_seg)
					{
						Point2 pp(span.projection(*i));
						double my_dist = Segment2(pp, *i).squared_length();
						if(my_dist < max_dist_move && my_dist > d_sq)
						{
							best_dead = i;
							d_sq = my_dist;
						}
					}
				}
			}
		}
		if(best_dead == pts.end()) break;

		pts.erase(best_dead);
		++t;
	}
	return t;
}

/************************************************************************************************************************************************
 *
 ************************************************************************************************************************************************/

RoadPrefs_t gRoadPrefs = { 10.0, 50000.0, 0.8, 1.0 };

struct	TensorSeed {
	Point2		p;
	bool		major;
	int			x;
	int			y;
};
typedef list<TensorSeed>		SeedQueue;



struct Tensor_info {
	const DEMGeo *	elev;
	const DEMGeo *	grdx;
	const DEMGeo *	grdy;
	const DEMGeo *	uden;
	const DEMGeo *	urad;
	const DEMGeo *	usqr;
	Bbox2			bounds;
};

inline int	dem_get(const DEMGeo& d, int x, int y)
{
	float e[9];
	e[0] = d.get(x-1,y-1);
	e[1] = d.get(x  ,y-1);
	e[2] = d.get(x+1,y-1);
	e[3] = d.get(x-1,y  );
	e[4] = d.get(x  ,y  );
	e[5] = d.get(x+1,y  );
	e[6] = d.get(x-1,y+1);
	e[7] = d.get(x  ,y+1);
	e[8] = d.get(x+1,y+1);
	if(e[4] == DEM_NO_DATA) return DEM_NO_DATA;
	int f = 0;
	bool h = false;
	for(int n = 0; n < 9; ++n)
	if(e[n] != DEM_NO_DATA)
	{
		h = true;
		f |= (int) e[n];
	}
	return h ? f : DEM_NO_DATA;
}

static Vector2	Tensor_Func(const Point2& p, void * ref)
{
	Tensor_info * i = (Tensor_info *) ref;

	double lon = interp(0,i->bounds.p1.x(),1,i->bounds.p2.x(),p.x());
	double lat = interp(0,i->bounds.p1.y(),1,i->bounds.p2.y(),p.y());

	double xe = i->elev->lon_to_x(lon);
	double ye = i->elev->lat_to_y(lat);
	double xr = i->urad->lon_to_x(lon);
	double yr = i->urad->lat_to_y(lat);
	double xu = i->usqr->lon_to_x(lon);
	double yu = i->usqr->lat_to_y(lat);
	double xg = i->grdx->lon_to_x(lon);
	double yg = i->grdx->lat_to_y(lat);

	double	sq_w = 0.0f;
	double	ir_w = 1.0f;
	float sqv = i->usqr->get(xu,yu);
	if (sqv == 1.0) sq_w = 1.f, ir_w = 0.0f;
	if (sqv == 2.0) sq_w = 0.f, ir_w = 1.0f;

	Vector2	basis  = (Gradient2Tensor(Vector2(i->elev->gradient_x_bilinear(xe,ye),i->elev->gradient_y_bilinear(xe,ye))) * gRoadPrefs.elevation_weight);
	if(ir_w > 0.0f)
			basis += (Gradient2Tensor(Vector2(i->urad->gradient_x_bilinear(xr,yr),i->urad->gradient_y_bilinear(xr,yr))) * gRoadPrefs.radial_weight * ir_w);

	if (sq_w > 0.0f)
		basis += Vector2(i->grdx->get(xg,yg),i->grdy->get(xg,yg));

	return basis;


}

bool	CheckSeed(
				const TensorSeed&	s,
					  DEMGeo&		d)
{
	int old = dem_get(d,s.x,s.y);
	if(old==DEM_NO_DATA) return false;
	int mask = s.major ? 1 : 2;
	if ((old & mask) == 0)
	{
		return true;
	}
	return false;
}

void	QueueSeed(
				const Point2&	p,
				bool			major,
				DEMGeo&			dem,
				SeedQueue&		q)
{
	TensorSeed	s;
	s.p = p;
	s.major = major;
	s.x = dem.lon_to_x(p.x());
	s.y = dem.lat_to_y(p.y());

	int old = dem_get(dem,s.x,s.y);
	if(old==DEM_NO_DATA)return;
	int mask = s.major ? 5 : 6;
	if ((old & mask) == 0)
	{
		old |= 4;
		dem(s.x,s.y) = old;
		q.push_back(s);
	}
}

bool CheckStats(const Point2& p, const Point2& p_old, const DEMGeo& elev, const DEMGeo& slope, const DEMGeo& density, float amp)
{
//	return true;
	float d = density.get(density.lon_to_x(p.x()),density.lat_to_y(p.y()));
	float s = slope.get(slope.lon_to_x(p.x()),slope.lat_to_y(p.y()));
	if(d == DEM_NO_DATA) return false;
	d += amp;
	if (!RollDice(d)) return false;
//	if (!RollDice((d*gRoadPrefs.density_amp))) return false;
	float ss = fabs(elev.value_linear(p.x(),p.y())-elev.value_linear(p_old.x(),p_old.y()));
	float rr = pythag(elev.x_dist_to_m(elev.lon_to_x(p.x())-elev.lon_to_x(p_old.x())),
					 elev.y_dist_to_m(elev.lat_to_y(p.y())-elev.lat_to_y(p_old.y())));
	if(rr==0.0) return true;
	if(RollDice(ss / rr)*gRoadPrefs.slope_amp)return false;
//	if(RollDice(s * gRoadPrefs.slope_amp)) return false;
	return true;
}

bool CheckAndRegister(
			const Point2&		p,
			DEMGeo&				dem,
			int&				ox,
			int&				oy,
			int&				ctr,
			bool				major)
{
	int x = intlim(dem.lon_to_x(p.x()),0,dem.mWidth -1);
	int y = intlim(dem.lat_to_y(p.y()),0,dem.mHeight-1);
	if(x == ox && y == oy)
		return (ctr-- > 0);
	ox =x;
	oy =y;
	ctr = 10;
	int old = dem.get(x,y);
	if(old==DEM_NO_DATA)return false;
	int mask = major ? 1 : 2;
	if ((old & mask) == 0)
	{
		old |= mask;
		dem(x,y) = old;
		return true;
	}
	return false;
}

void	TensorForFace(
					const DEMGeo&	inElevation,
					const DEMGeo&	inUrbanDensity,
					const DEMGeo&	inUrbanRadial,
					const DEMGeo&	inUrbanSquare,
					const DEMGeo&	inGridX,
					const DEMGeo&	inGridY,
					Tensor_info&	t)
{

	t.elev = &inElevation;
	t.urad = &inUrbanRadial;
	t.usqr = &inUrbanSquare;
	t.uden = &inUrbanDensity;
	t.grdx = &inGridX;
	t.grdy = &inGridY;
	t.bounds.p1.x_ = inElevation.mWest ;
	t.bounds.p1.y_ = inElevation.mSouth;
	t.bounds.p2.x_ = inElevation.mEast ;
	t.bounds.p2.y_ = inElevation.mNorth;
}

void	RasterEdge(
					Halfedge_handle		e,
					DEMGeo&				dem,
					Vector2 (*			tensorFunc)(const Point2& p, void * ref),
					void *				tensorRef)
{
	int x1 = intlim(dem.lon_to_x(CGAL::to_double(e->source()->point().x())),0,dem.mWidth-1);
	int x2 = intlim(dem.lon_to_x(CGAL::to_double(e->target()->point().x())),0,dem.mWidth-1);
	int y1 = intlim(dem.lat_to_y(CGAL::to_double(e->source()->point().y())),0,dem.mHeight-1);
	int y2 = intlim(dem.lat_to_y(CGAL::to_double(e->target()->point().y())),0,dem.mHeight-1);

	Vector2	road_dir(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()));
	road_dir.normalize();

	if(std::abs(x2-x1) > std::abs(y2-y1))
	{
		// "Horizontal line"
		if(x2 < x1)
		{
			swap(x1,x2);
			swap(y1,y2);
		}
		for(int x = x1; x <= x2; ++x)
		{
			int y = intlim(interp(x1,y1,x2,y2,x),0,dem.mHeight-1);
			Vector2	e(Tensor2Eigen(tensorFunc(
										Point2(interp(0,0,dem.mWidth -1,1,x),
											   interp(0,0,dem.mHeight-1,1,y)),tensorRef)));
			double align_major = fabs(road_dir.dot(e));
			e = e.perpendicular_ccw();
			double align_minor = fabs(road_dir.dot(e));

			int old = dem.get(x,y);
			if(align_major > 0.7) old |= 1;
			if(align_minor > 0.7) old |= 2;
			dem(x,y)=old;
		}
	}
	else
	{
		// "Vertical line"
		if(y2 < y1)
		{
			swap(x1,x2);
			swap(y1,y2);
		}
		for(int y = y1; y < y2; ++y)
		{
			int x = intlim(interp(y1,x1,y2,x2,y),0,dem.mWidth-1);
			Vector2	e(Tensor2Eigen(tensorFunc(
										Point2(interp(0,0,dem.mWidth -1,1,x),
											   interp(0,0,dem.mHeight-1,1,y)),tensorRef)));
			double align_major = fabs(road_dir.dot(e));
			e = e.perpendicular_ccw();
			double align_minor = fabs(road_dir.dot(e));

			int old = dem.get(x,y);
			if(align_major > 0.8) old |= 1;
			if(align_minor > 0.8) old |= 2;
			dem(x,y)=old;
		}
	}
}

void	BuildRoadsForFace(
					Pmwx&			ioMap,
					const DEMGeo&	inElevation,
					const DEMGeo&	inSlope,
					const DEMGeo&	inUrbanDensity,
					const DEMGeo&	inUrbanRadial,
					const DEMGeo&	inUrbanSquare,
					Face_handle		inFace,
					ProgressFunc	inProg,
					ImageInfo *		ioTensorImage,
					double			outTensorBounds[4])
{
	Pmwx::Face_iterator		f;
	int						rx1, rx2, x, y;
	Tensor_info				t;

//	gMeshLines.clear();
//	gMeshPoints.clear();

	DEMGeo	road_restrict(inElevation.mWidth,inElevation.mHeight);
	DEMGeo	grid_x(inElevation.mWidth,inElevation.mHeight);
	DEMGeo	grid_y(inElevation.mWidth,inElevation.mHeight);
	road_restrict.copy_geo_from(inElevation);
	grid_x.copy_geo_from(inElevation);
	grid_y.copy_geo_from(inElevation);

	/**********************************************************************************************************************************
	 * INITIALIZE THE ROAD RESTRICTION GRID USING WATER AND OTHER NON-PASSABLES!
	**********************************************************************************************************************************/

	// Best to zap out a lot here since more possiblep points means more time in the alg.
	{
		TIMER(burn_water)

		set<Face_handle>			no_road_faces;
		set<Halfedge_handle>		bounds;
		PolyRasterizer				raster;
		for(f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
		if (!f->is_unbounded())
		if(!RoadsForThisFace(f))
			no_road_faces.insert(f);

		FindEdgesForFaceSet<Pmwx>(no_road_faces, bounds);
		y = SetupRasterizerForDEM(bounds, road_restrict, raster);

		raster.StartScanline(y);
		while (!raster.DoneScan())
		{
			while (raster.GetRange(rx1, rx2))
			{
				rx1 = intlim(rx1,0,road_restrict.mWidth-1);
				rx2 = intlim(rx2,0,road_restrict.mWidth-1);
				for (x = rx1; x < rx2; ++x)
				{
					road_restrict(x,y)=3.0;
				}
			}
			++y;
			if (y >= road_restrict.mHeight)
				break;
			raster.AdvanceScanline(y);
		}

		{
			DEMGeo	temp(road_restrict);
			for(y = 0; y < temp.mHeight; ++y)
			for(x = 0; x < temp.mWidth ; ++x)
			{
				if(temp.get_radial(x,y,1,0.0) != 0.0)
					road_restrict(x,y) = 3.0;
			}
		}
	}

	/**********************************************************************************************************************************
	 * BURN EACH VECTOR INTO THE RESTRICTION GRID TOO
	**********************************************************************************************************************************/

	{
		TIMER(burn_roads)
			TensorForFace(
				inElevation,
				inUrbanDensity,
				inUrbanRadial,
				inUrbanSquare,
				grid_x,
				grid_y,
				t);

		for(Pmwx::Edge_iterator e = ioMap.edges_begin(); e != ioMap.edges_end(); ++e)
			RasterEdge(e, road_restrict, Tensor_Func, &t);
	}

#if DEV
//	gDem[dem_Wizard] = road_restrict;
#endif
	/**********************************************************************************************************************************
	 * BUILD GRID TENSOR FIELD
	**********************************************************************************************************************************/

	// Running a tensor func that accesses every polygon vertex in its evaluator would be unacceptably slow.  So we simply rasterize
	// each polygon's interior using its own internal tensor func, which simplifies the cost of building this.  This lowers the accuracy
	// of the grid tensor field, but we don't care that much anyway.

	{
		TIMER(calc_linear_tensors)
//		int tcalcs = 0;

		for(f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
		if (!f->is_unbounded())
		if(RoadsForThisFace(f))
		{
			// First build a polygon with tensor weights for the face we're working on.
			vector<Point2>		poly;
			vector<Vector2>		tensors;
			PolyRasterizer		raster;


			Pmwx::Ccb_halfedge_circulator	circ = f->outer_ccb();
			Pmwx::Ccb_halfedge_circulator	start = circ;
			Bbox2	bounds(cgal2ben(circ->source()->point()));
			do {
				poly.push_back(cgal2ben(circ->target()->point()));
				bounds += cgal2ben(circ->target()->point());
				Vector2 prev(	cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point()));
				Vector2 next(	cgal2ben(circ->next()->source()->point()),cgal2ben(circ->next()->target()->point()));
				prev.normalize();
				next.normalize();
				Vector2 v(prev+next);
				v.normalize();
				tensors.push_back(/*Eigen2Tensor*/(v));
				++circ;
			} while (circ != start);

			for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
			{
				Pmwx::Ccb_halfedge_circulator	circ(*h);
				Pmwx::Ccb_halfedge_circulator	start = circ;
				do {
					poly.push_back(cgal2ben(circ->target()->point()));
					Vector2 prev(	cgal2ben(circ->source()->point()),cgal2ben(circ->target()->point()));
					Vector2 next(	cgal2ben(circ->next()->source()->point()),cgal2ben(circ->next()->target()->point()));
					prev.normalize();
					next.normalize();
					Vector2 v(prev+next);
					v.normalize();
					tensors.push_back(/*Eigen2Tensor*/(v));
					++circ;
				} while(circ != start);
			}

			// Now rasterize into the polygon...
			double sz = (bounds.p2.y() - bounds.p1.y()) * (bounds.p2.x() - bounds.p1.x());
			y = SetupRasterizerForDEM(f, road_restrict, raster);
			raster.StartScanline(y);
			while (!raster.DoneScan())
			{
				while (raster.GetRange(rx1, rx2))
				{
					rx1 = intlim(rx1,0,road_restrict.mWidth-1);
					rx2 = intlim(rx2,0,road_restrict.mWidth-1);
					for (x = rx1; x < rx2; ++x)
					if(road_restrict.get(x,y) != 3.0)
					{
						float sq = inUrbanSquare.get(
									inUrbanSquare.lon_to_x(grid_x.x_to_lon(x)),
									inUrbanSquare.lat_to_y(grid_x.y_to_lat(y)));
						if(sq == 1.0)
						{
							Vector2	t(grid_x.get(x,y),grid_y.get(x,y));
							for (int n = 0; n < poly.size(); ++n)
							{
//								++tcalcs;
								t += (Linear_Tensor(poly[n],tensors[n], 4.0 / sz, Point2(road_restrict.x_to_lon(x),road_restrict.y_to_lat(y))));
							}
							grid_x(x,y) = t.dx;
							grid_y(x,y) = t.dy;
						}
					}
				}
				++y;
				if (y >= road_restrict.mHeight)
					break;
				raster.AdvanceScanline(y);
			}
		}

//		printf("Total tensor calcs for road grid: %d\n",tcalcs);
	}


	/**********************************************************************************************************************************
	 * SEED THE QUEUE!
	**********************************************************************************************************************************/
	SeedQueue		seedQ;

	{
		TIMER(build_seedQ)

		for(Pmwx::Vertex_iterator v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++v)
		{
			bool has_road = false;
			Pmwx::Halfedge_around_vertex_circulator circ(v->incident_halfedges());
			Pmwx::Halfedge_around_vertex_circulator stop(circ);
			do {
				if (!circ->data().mSegments.empty() ||
					!circ->twin()->data().mSegments.empty())
				{
					has_road = true;
					break;
				}
				++circ;
			} while (circ != stop);
			if(has_road)
			{
				QueueSeed(cgal2ben(v->point()),false,road_restrict,seedQ);
			}
		}

		printf("Queued %llu seeds origially.\n", (unsigned long long)seedQ.size());
	}

	/**********************************************************************************************************************************
	 * RUN THROUGH THE QUEUE, BUILDING ROADS
	**********************************************************************************************************************************/

	vector<Segment2> roads;
	int ctr=0;
	{
		TIMER(eval_seed_Q)
		while(!seedQ.empty())
		{
			++ctr;
			if(CheckSeed(seedQ.front(),road_restrict))
			{
				list<Point2>	pts;
				Point2	fp(seedQ.front().p);
				Point2	bp(fp);
				pts.push_back(fp);
				bool front_alive = true;
				bool back_alive = true;
				bool major = seedQ.front().major;
				int fx(seedQ.front().x);
				int fy(seedQ.front().y);
				int fc=10,bc=10;
				int bx = fx, by = fy;
				Vector2	fe(0.0,0.0);
				Vector2	be(0.0,0.0);
				do {
					if (front_alive)
					{
						Vector2	e = Tensor2Eigen(Tensor_Func(
											Point2(interp(road_restrict.mWest , 0, road_restrict.mEast ,1,fp.x()),
												   interp(road_restrict.mSouth, 0, road_restrict.mNorth,1,fp.y())),&t));
						if (!seedQ.front().major) e = e.perpendicular_ccw();
						if(e.dot(fe) < 0) e = -e;
						fe = e;
						e *= ADVANCE_RATIO;
						fp += e;
						front_alive = CheckAndRegister(fp,road_restrict,fx, fy, fc, major);
						if(front_alive) front_alive = CheckStats(fp,pts.front(),inElevation,inSlope, inUrbanDensity, gRoadPrefs.density_amp);
						if(front_alive)
						{
							pts.push_front(fp);
							if (CheckStats(fp,pts.front(),inElevation,inSlope, inUrbanDensity, 0.0f))
								QueueSeed(fp,!major,road_restrict,seedQ);
						}
					}
					if (back_alive)
					{
						Vector2	e = -Tensor2Eigen(Tensor_Func(
											Point2(interp(road_restrict.mWest , 0, road_restrict.mEast ,1,bp.x()),
												   interp(road_restrict.mSouth, 0, road_restrict.mNorth,1,bp.y())),&t));
						if (!seedQ.front().major) e = e.perpendicular_ccw();
						if (e.dot(be) < 0) e = -e;
						be = e;
						e *= ADVANCE_RATIO;
						bp+= e;
						back_alive = CheckAndRegister(bp,road_restrict,bx, by, bc, major);
						if(back_alive) back_alive = CheckStats(bp,pts.back(),inElevation,inSlope, inUrbanDensity, gRoadPrefs.density_amp);
						if(back_alive) {
							pts.push_back(bp);
							if(CheckStats(bp,pts.back(),inElevation,inSlope, inUrbanDensity, 0.0f))
								QueueSeed(bp,!major,road_restrict,seedQ);
						}
					}
				} while (front_alive || back_alive);

				Point3	c(1,1,0);
				if(!major)c.y = 0;

				int k = ThinLine(pts, 10.0 * MTR_TO_NM * NM_TO_DEG_LAT, 500 * MTR_TO_NM * NM_TO_DEG_LAT);
	//			printf("Killed %d points, kept %d points.\n", k, pts.size());
				for(list<Point2>::iterator i = pts.begin(); i != pts.end(); ++i)
				{
	//				gMeshPoints.push_back(pair<Point2,Point3>(*i,c));
					if(i != pts.begin())
					{
						list<Point2>::iterator j(i);
						--j;
	//					gMeshLines.push_back(pair<Point2,Point3>(*j,c));
	//					gMeshLines.push_back(pair<Point2,Point3>(*i,c));
					// can't  do this - makes a point cloud of roads - TOTALLY gross.
	//					if(RollDice(max(inUrbanDensity.value_linear(j->x,j->y),inUrbanDensity.value_linear(i->x,i->y))))
							roads.push_back(Segment2(*j,*i));
					}
				}
			}
			seedQ.pop_front();
	//		if((ctr%1000)==0)
	//			printf("Q contains: %d, pts: %d\n", seedQ.size(), gMeshPoints.size());
		}
	}

	{
		TIMER(build_real_roads)
		Pmwx	sub;
		sub.unbounded_face()->data().mTerrainType = terrain_Natural;
		BulkInsertRoads(roads, sub);
		BulkZapRoads(inUrbanDensity, sub);
		DebugAssert(sub.is_valid());
//		TopoIntegrateMaps(&ioMap, &sub);
	//	for(Pmwx::Face_iterator sf = sub.faces-begin(); sf != sub.faces_end(); ++sf)
	//		sf->mTerrainType = terrain_Natural;
		DebugAssert(ioMap.is_valid());
		DebugAssert(sub.is_valid());

		#if SHOW_GENERATED_ROADS
		for(Pmwx::Edge_iterator eit = sub.edges_begin(); eit != sub.edges_end(); ++eit)
			debug_mesh_line(cgal2ben(eit->source()->point()),cgal2ben(eit->target()->point()),1,0,0, 0,1,0);
		#endif

		if(!sub.is_empty())
			MergeMaps_legacy(ioMap, sub, false, NULL, true, inProg);
	}

	/**********************************************************************************************************************************
	 * DEBUG OUTPUT
	**********************************************************************************************************************************/

#if OPENGL_MAP
	if(inFace != Face_handle() && ioTensorImage)
	{
		Pmwx::Ccb_halfedge_circulator	circ = inFace->outer_ccb();
		Pmwx::Ccb_halfedge_circulator	start = circ;
		t.bounds = Bbox2(cgal2ben(circ->source()->point()));
		do {
			t.bounds += cgal2ben(circ->source()->point());
			++circ;
		} while (circ != start);


		TensorDDA(*ioTensorImage,Tensor_Func,&t);
		outTensorBounds[0] = t.bounds.p1.x();
		outTensorBounds[1] = t.bounds.p1.y();
		outTensorBounds[2] = t.bounds.p2.x();
		outTensorBounds[3] = t.bounds.p2.y();
	}
#endif
#if SHOW_ROAD_RESTRICT
	gDem[dem_Wizard] = road_restrict;
#endif

}
