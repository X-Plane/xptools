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

THIS IS STUFF THAT IS GENERALLY NO LONGER USED IN MESH ALGS

/* This defines the ratio of skip to ok points for vector rivers.  It's
   a real pain to do a perfect computation of vital river points, and 
   frankly it doesn't matter...every river point is DEM-wise useful and
   for the most part the data is organized to keep adjacent rivers nearby
   in the map.  So skipping a few produces surprisingly pleasing results 
   for like, no work. */
#define RIVER_SKIP 6
#define FAR_TEX_DIVISIONS 2


// About 80 degrees
#define ROCK_SLOPE_CUTOFF	0.17364	


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
				