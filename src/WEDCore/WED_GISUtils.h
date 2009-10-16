/*
 *  WED_GISUtils.h
 *  SceneryTools
 *
 *  Created by bsupnik on 5/27/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WED_GISUTILS_H
#define WED_GISUTILS_H

#include "MapDefs.h"
class IGISPointSequence;
class IGISEntity;
class IGISQuad;
class IGISPolygon;
class WED_Thing;

// Return a polygon for a WED point ring.
// Return false for point sequence that has bezier parts.
bool	WED_PolygonForPointSequence(IGISPointSequence * in_seq, Polygon_2& out_pol, Polygon_2 * out_uv);

// Returns a polygon set that contains the surface area of a GIS entity.
// Returns false if a GIS entity contains a curved polygon, which is not yet supported.
bool	WED_PolygonSetForEntity(IGISEntity * in_entity, Polygon_set_2& out_pgs);

typedef vector<FastKernel::Triangle_2>		UVMap_t;

void	WED_MakeUVMap(
						const Polygon_2&		uv_map_ll,
						const Polygon_2&		uv_map_uv,
						UVMap_t&				out_map);

void	WED_MakeUVMap(
						IGISQuad *				in_quad,
						UVMap_t&				out_map);

void	WED_MakeUVMap(
						IGISPolygon *			in_quad,
						UVMap_t&				out_map);


bool	WED_MapPoint(const UVMap_t&	in_map, const Point_2& ll, Point_2& uv);
bool	WED_MapPolygon(const UVMap_t&	in_map, const Polygon_2& ll, Polygon_2& uv);
bool	WED_MapPolygonWithHoles(const UVMap_t&	in_map, const Polygon_with_holes_2& ll, Polygon_with_holes_2& uv);

/********************************************************************************************************************************************
 * MORE COMPLEX IGIS-BASED OPERATIONS
 ********************************************************************************************************************************************/
 
// This will attempt to merge the above points, connecting any attached edges.
bool	WED_MergePoints(const vector<IGISEntity *>& in_points);

// This splits an edge at a given internal pt, forming two new edges.
bool	WED_SplitEdgeIfNeeded(WED_Thing * pt, const string& in_cross_name);

#endif
