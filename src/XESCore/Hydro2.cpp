/* 
 * Copyright (c) 2011, Laminar Research.
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

#include "Hydro2.h"

#include "MapRaster.h"
#include "MapOverlay.h"
#include "DEMAlgs.h"
#include "GISTool_Globals.h"
#include "PolyRasterUtils.h"
#include "MeshAlgs.h"
#include "MathUtils.h"
#include "MapTopology.h"
#include "MapHelpers.h"

// This previews cases where water flattening in x-plane are going to seriously deform the DEM!
#define DEBUG_FLATTEN_ERROR 0

static void copy_nearest_splat(DEMGeo& src, DEMGeo& dst, int lu)
{
	for(int y = 0; y < src.mHeight; ++y)
	for(int x = 0; x < src.mWidth; ++x)
	if(src(x,y) == lu)
	{
		int xx = dst.map_x_from(src,x);
		int yy = dst.map_y_from(src,y);
		for(int dy = -1; dy <= 1; ++dy)
		for(int dx = -1; dx <= 1; ++dx)
		{
			if(dst.valid(DEMGeo::coordinates(xx + dx, yy + dy)))
				dst(xx+dx,yy+dy) = lu;
		}
	}
}

static void eliminate_isolated(DEMGeo& dem, int keep_v, int null_v, int min_size_pix)
{
	address_fifo fifo(dem.mWidth * dem.mHeight), wet(dem.mWidth * dem.mHeight);
	DEMGeo	visited(dem.mWidth,dem.mHeight);
	for(DEMGeo::address a = dem.address_begin(); a != dem.address_end(); ++a)
	if(dem[a] == keep_v)
	if(visited[a] == 0.0f)
	{
		fifo.push(a);
		visited[a] = 1.0f;
		
		int lc_count = 0;
		while(!fifo.empty())
		{
			DEMGeo::address w = fifo.pop();
			++lc_count;
			wet.push(w);
			for(DEMGeo::neighbor_iterator<4> n = dem.neighbor_begin<4>(w); n != dem.neighbor_end<4>(w); ++n)
			{
				if(visited[*n] == 0.0 && dem[*n] == keep_v)
				{
					visited[*n] = 1.0;
					fifo.push(*n);
				}
			}
		}
		
		int keep_it = lc_count >= min_size_pix ? keep_v : null_v;
		while(!wet.empty())
		{
			DEMGeo::address w = wet.pop();
			dem[w] = keep_it;
		}
	}
}

template <class Arr>
struct no_sharp_pt {
	bool is_locked(typename Arr::Vertex_handle v) const { 
		typename Arr::Halfedge_handle p = v->incident_halfedges();
		typename Arr::Halfedge_handle pp, n, nn;
		
		pp = p->prev();
		n = p->next();
		nn = n->next();
		
		if(CGAL::angle(pp->source()->point(),p->source()->point(),n->target()->point()) == CGAL::ACUTE)
		{
//			debug_mesh_point(cgal2ben(v->point()),1,0,0);			
			return true;
		}

		if(CGAL::angle(p->source()->point(),n->target()->point(),nn->target()->point()) == CGAL::ACUTE)
		{
//			debug_mesh_point(cgal2ben(v->point()),0,1,0);			
			return true;
		}
	
		return false;
	}
	void remove(typename Arr::Vertex_handle v) const { }
};



void add_missing_water(
			Pmwx&			io_map, 
			DEMGeo&			elev, 
			DEMGeo&			lu,
			int				smallest_water,
			float			zlimit,
			float			simplify)
{
	Pmwx	water;
	
	DEMGeo	water_dem_lu(lu);
	for(DEMGeo::iterator i = water_dem_lu.begin(); i != water_dem_lu.end(); ++i)
	if((*i) == lu_globcover_WATER)
		*i = terrain_Water;
	else {
		*i = NO_VALUE;
	}

	

//	DEMGeo	lhi;
//	DEMGeo	ws_dem;
//	vector<DEMGeo::address> ws;	
//	NeighborHisto(water_dem_lu, lhi, 5);	
//	Watershed(lhi, ws_dem,&ws);
//	MergeMMU(ws_dem,ws,10);
//	SetWatershedsToDominant(water_dem_lu,ws_dem, ws);


	DEMGeo	water_dem(elev);
	water_dem = NO_VALUE;
	copy_nearest_splat(water_dem_lu,water_dem,terrain_Water);
	dem_erode(water_dem,2,NO_VALUE);
	dem_erode(water_dem,1,terrain_Water);

	eliminate_isolated(water_dem, terrain_Water, NO_VALUE, smallest_water);


	

	DEMGeo	existing_water(elev);
	existing_water = 0;

	PolyRasterizer<double> raster;
	SetupWaterRasterizer(io_map, existing_water, raster, terrain_Water);

	int x, y = 0;
	raster.StartScanline(y);
	while (!raster.DoneScan())
	{
		int x1, x2;
		while (raster.GetRange(x1, x2))
		{
			for (x = x1; x < x2; ++x)
			{
				existing_water(x,y) = 1;
			}
		}
		++y;
		if (y >= existing_water.mHeight) break;
		raster.AdvanceScanline(y);
	}

	dem_erode(existing_water, 2, 1);
	
	DEMGeo	existing_vec(elev);
	existing_vec = 0;
	
	for(Pmwx::Halfedge_iterator e = io_map.halfedges_begin(); e != io_map.halfedges_end(); ++e)
	if(!e->face()->is_unbounded())
	if(!e->twin()->face()->is_unbounded())
	if(e->data().HasGroundRoads())
	{
		Segment2 s(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()));
		int x1 = intlim(existing_vec.lon_to_x(s.p1.x()),0,existing_vec.mWidth -1);
		int y1 = intlim(existing_vec.lat_to_y(s.p1.y()),0,existing_vec.mHeight-1);
		int x2 = intlim(existing_vec.lon_to_x(s.p2.x()),0,existing_vec.mWidth -1);
		int y2 = intlim(existing_vec.lat_to_y(s.p2.y()),0,existing_vec.mHeight-1);
		
		int dx = (x2 > x1) ? x2 - x1 : x1 - x2;
		int dy = (y2 > y1) ? y2 - y1 : y1 - y2;
		int x, y;
		if(dx > dy)
		{
			if(x1 > x2) { swap(x1,x2); swap(y1,y2); }
			for(x = x1; x <= x2; ++x)
			{
				y = interp(x1,y1,x2,y2,x);
				existing_vec(x,y) = 1;
			}
		} 
		else 
		{
			if(y1 > y2) { swap(x1,x2); swap(y1,y2); }
			for(y = y1; y <= y2; ++y)
			{
				x = interp(y1,x1,y2,x2,y);
				existing_vec(x,y) = 1;
			}
		}
	}

	dem_erode(existing_vec, 2, 1);

	gDem[dem_Wizard1] = existing_water;
	gDem[dem_Wizard2] = water_dem;
	gDem[dem_Wizard3] = water_dem_lu;
	gDem[dem_Wizard4] = existing_vec;
	
	for(DEMGeo::address a = water_dem.address_begin(); a != water_dem.address_end(); ++a)
	{
		if(existing_water[a] == 1)
			water_dem[a] = NO_VALUE;
		else if(existing_vec[a] == 1)
			water_dem[a] = NO_VALUE;
	}

	eliminate_isolated(water_dem, terrain_Water, NO_VALUE, smallest_water);

	gDem[dem_Wizard5] = water_dem;


	address_fifo fifo(water_dem.mWidth * water_dem.mHeight);
	DEMGeo	visited(water_dem.mWidth,water_dem.mHeight);
	for(DEMGeo::address a = water_dem.address_begin(); a != water_dem.address_end(); ++a)
	if(water_dem[a] == terrain_Water)
	if(visited[a] == 0.0f)
	{
		fifo.push(a);
		visited[a] = 1.0f;
		set<DEMGeo::address> wet;
		
		int lc_count = 0;
		while(!fifo.empty())
		{
			DEMGeo::address w = fifo.pop();
			++lc_count;
			wet.insert(w);
			for(DEMGeo::neighbor_iterator<4> n = water_dem.neighbor_begin<4>(w);  n != water_dem.neighbor_end<4>(w); ++n)
			{
				if(visited[*n] == 0.0 && water_dem[*n] == terrain_Water)
				{
					visited[*n] = 1.0;
					fifo.push(*n);
				}
			}
		}
				
		while(!wet.empty())
		{
			//printf("Looking at lake of %zd pts.\n", wet.size());
			Bbox2	box;
			float zmin = elev[*wet.begin()];
			float zmax = zmin;
			DEMGeo::address top = *wet.begin();
			for(set<DEMGeo::address>::iterator w = wet.begin(); w != wet.end(); ++w)
			{
				DEMGeo::coordinates wc(water_dem.to_coordinates(*w));
				float e = elev[*w];
				box += Point2(water_dem.x_to_lon(wc.first),water_dem.y_to_lat(wc.second));
				if(e > zmax)
				{
					top = *w;
				}
				zmin = min(zmin,e);
				zmax = max(zmax,e);
			}
			
			double DEG_TO_NM_LON = DEG_TO_NM_LAT * cos(CGAL::to_double(box.ymin()) * DEG_TO_RAD);
			double rhs = (pow((box.xmax()-box.xmin())*DEG_TO_NM_LON*NM_TO_MTR,2) + pow((box.ymax()-box.ymin())*DEG_TO_NM_LAT*NM_TO_MTR,2));
			double lhs = pow((double)(zmax-zmin),2);
			//printf("Lake from %lf,%lf to %lf,%lf\n",box.p1.x(),box.p1.y(),box.p2.x(),box.p2.y());
			//printf("Span is %lf, vertical is %lf\n", sqrt(rhs), sqrt(lhs));
			if (zlimit*lhs > rhs) 
			{
				water_dem[top] = NO_VALUE;
				wet.erase(top);
				DEMGeo::coordinates c(water_dem.to_coordinates(top));
//				debug_mesh_point(Point2(water_dem.x_to_lon(c.first),water_dem.y_to_lat(c.second)),1,0,0);
			}
			else
				break;
		}
	}

	gDem[dem_Wizard6] = water_dem;


	MapFromDEM(water_dem, 0, 0, water_dem.mWidth, water_dem.mHeight, 2, NO_VALUE, water, NULL, true);
	for(Pmwx::Edge_iterator e = water.edges_begin(); e != water.edges_end(); ++e)
	{
//		debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,1,0,1,1,0);
//		debug_mesh_point(cgal2ben(e->source()->point()),1,1,0);
	}

//	SimplifyMap(water, false, gProgress);

	arrangement_simplifier<Pmwx,no_sharp_pt<Pmwx> > simplifier;
	simplifier.simplify(water, simplify, arrangement_simplifier<Pmwx, no_sharp_pt<Pmwx> >::traits_type(), gProgress);

	for(Pmwx::Face_handle f  =water.faces_begin(); f != water.faces_end(); ++f)
		f->set_contained(f->data().IsWater());
	
	Pmwx new_map;
	MapOverlay(io_map, water,new_map);
	io_map = new_map;

}

void	build_water_surface_dem(CDT& io_mesh, const DEMGeo& in_elev, DEMGeo& out_water, DEMGeo& io_bath)
{
	out_water.resize(256,256);
	out_water.copy_geo_from(in_elev);
	out_water.mPost = 1;
	for(int y = 0; y < out_water.mHeight; ++y)
	for(int x = 0; x < out_water.mWidth; ++x)
		out_water(x,y) = in_elev.value_linear(out_water.x_to_lon(x),out_water.y_to_lat(y));
	
	out_water = DEM_NO_DATA;
	
	for(CDT::Finite_faces_iterator ffi = io_mesh.finite_faces_begin(); ffi != io_mesh.finite_faces_end(); ++ffi)	
	if(ffi->info().terrain == terrain_Water)
	{
		for(int n = 0; n < 3; ++n)
		{
			Point2 p(cgal2ben(ffi->vertex(n)->point()));
			
			int x, y;
			float hh = out_water.xy_nearest_raw(p.x(),p.y(),x,y);

			out_water(x,y) = MIN_NODATA(hh, ffi->vertex(n)->info().height);

		}
	}
	
	out_water.fill_nearest();



	#if DEBUG_FLATTEN_ERROR
	for(CDT::Finite_vertices_iterator fvi = io_mesh.finite_vertices_begin(); fvi != io_mesh.finite_vertices_end(); ++fvi)
	{
		if(CategorizeVertex(io_mesh, fvi, terrain_Water) < 1)
		{
			double sh = out_water.xy_nearest(CGAL::to_double(fvi->point().x()),CGAL::to_double(fvi->point().y()));
			double err = fabs(sh - fvi->info().height);
			if(err > 15.0)
			debug_mesh_point(cgal2ben(fvi->point()),1,0,0);
			else if(err > 10.0)
			debug_mesh_point(cgal2ben(fvi->point()),1,1,0);
			else if(err > 5.0)
			debug_mesh_point(cgal2ben(fvi->point()),0,1,0);
		}
	}
	#endif

}
