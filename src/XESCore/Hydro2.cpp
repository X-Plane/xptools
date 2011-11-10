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
			DEMGeo::coordinates wc(dem.to_coordinates(w));
			for(int dx = -1; dx <= 1; ++dx)
			for(int dy = -1; dy <= 1; ++dy)
			{
				DEMGeo::address n = dem.to_address(DEMGeo::coordinates(wc.first + dx, wc.second + dy));
				if(dem.valid(n) && visited[n] == 0.0 && dem[n] == keep_v)
				{
					visited[n] = 1.0;
					fifo.push(n);
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


void add_missing_water(Pmwx& io_map, DEMGeo& elev, DEMGeo& lu)
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

	eliminate_isolated(water_dem, terrain_Water, NO_VALUE, 20);


	

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

	gDem[dem_Wizard5] = water_dem;
	
	eliminate_isolated(water_dem, terrain_Water, NO_VALUE, 20);

	gDem[dem_Wizard6] = water_dem;

//	MapFromDEM(water_dem, 0, 0, water_dem.mWidth, water_dem.mHeight, NO_VALUE, water, NULL);


//	for(Pmwx::Face_handle f  =water.faces_begin(); f != water.faces_end(); ++f)
//		f->set_contained(f->data().IsWater());
		
//	Pmwx new_map;
//	MapOverlay(io_map, water,new_map);
//	io_map = new_map;

}
