/* 
 * Copyright (c) 2008, Laminar Research.
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

#include "ShapeIO.h"
#include <shapefil.h>
#include "MapOverlay.h"
#include "GISTool_Globals.h"
#include "ConfigSystem.h"
#include "MapAlgs.h"
#if !DEV
	#error factor this out
#endif

// Ben says: this can be modified to printf the points.  If a shape-file import ever blows up,
// we can use it to rapidly generate a numeric dataset - then we send a test program to the CGAL
// team if they want one.
#define ADD_PT_PAIR(a,b,c,d,e)	curves.push_back(Curve_2(Segment_2((a),(b)),(e)));

// Shape to feature 

struct shape_pattern_t {
	vector<string>		columns;
	vector<int>			dbf_id;
	vector<string>		values;
	int					feature;
};
typedef vector<shape_pattern_t> shape_pattern_vector;

static shape_pattern_vector	sShapeRules;

bool shape_in_bounds(SHPObject * obj)
{
	if(obj->dfXMax < gMapWest)	return false;
	if(obj->dfXMin > gMapEast)	return false;
	if(obj->dfYMax < gMapSouth)	return false;
	if(obj->dfYMin > gMapNorth) return false;
								return true;
}



static int want_this_thing(DBFHandle db, int shape_id)
{
	for(shape_pattern_vector::iterator r = sShapeRules.begin(); r != sShapeRules.end(); ++r)
	{
		bool rule_ok = true;
		
		for(int n = 0; n < r->columns.size(); ++n)
		{
			if(r->dbf_id[n] == -1)														{ rule_ok = false; break; }				
			
				const char * field_val = DBFReadStringAttribute(db,shape_id,r->dbf_id[n]);
				if(field_val == NULL)														{ rule_ok = false; break; }
				if(strcmp(r->values[n].c_str(),"*") == 0)
				{
					if(field_val[0] == 0)													 { rule_ok = false; break; }
				}
				else
				{
					if(strcmp(r->values[n].c_str(),field_val) != 0)								{ rule_ok = false; break; }			
				}
		}
		
		if(rule_ok) return r->feature;
	}
	return NO_VALUE;
}

static bool ShapeLineImporter(const vector<string>& inTokenLine, void * inRef)
{
	if(inTokenLine.size() != 4)
	{
		printf("Bad shape import line.\n");
		return false;
	}
	shape_pattern_t		pat;
	
	if(!TokenizeEnum(inTokenLine[3], pat.feature, "Bad enum"))
		return false;
		
	string::const_iterator e, s = inTokenLine[1].begin();
	while(s != inTokenLine[1].end())
	{
		e=s;
		while(e != inTokenLine[1].end() && *e != ',') ++e;
		pat.columns.push_back(string(s,e));
		if(e == inTokenLine[1].end())
			s = e;
		else
			s = e+1;
	}

	s = inTokenLine[2].begin();
	while(s != inTokenLine[2].end())
	{
		e=s;
		while(e != inTokenLine[2].end() && *e != ',') ++e;
		pat.values.push_back(string(s,e));
		if(e == inTokenLine[2].end())
			s = e;
		else
			s = e+1;
	}
	
	if(pat.values.size() != pat.columns.size())
	{
		printf("mismatch in number of columns vs. patterns.\n");
		return false;
	}	
	sShapeRules.push_back(pat);
	return true;
}


bool	ReadShapeFile(const char * in_file, Pmwx& io_map, shp_Flags flags, const char * feature_desc, double bounds[4], ProgressFunc	inFunc)
{
		int		entity_count;
		int		shape_type;
		double	bounds_lo[4], bounds_hi[4];
		static bool first_time = true;
	
	if(first_time)
	{	
		RegisterLineHandler("SHAPE_FEATURE", ShapeLineImporter, NULL);
		first_time = false;
	}

	int feat = NO_VALUE;
	if(flags & shp_Mode_Simple)
		feat = LookupToken(feature_desc);

	if(flags & shp_Mode_Map)
	{
		sShapeRules.clear();
		if (!LoadConfigFile(feature_desc))
		{
			printf("Could not load shape mapping file %s\n", feature_desc);
			return false;
		}
	}

	if((flags & shp_Overlay) == 0)					// If we are not overlaying, nuke the map now.  In _some_ modes (road curve insert,
		io_map.clear();								// one-by-one burn in) we are going to work on the final map, so this is needed.

		list<Polygon_2>	boundaries, holes;
		Polygon_set_2	poly_set;
		vector<Curve_2>	curves;

	SHPHandle file =  SHPOpen(in_file, "rb");
	if(!file) return false;
	DBFHandle db = NULL;
	if(flags & shp_Mode_Map)
	{
		db = DBFOpen(in_file,"rb");
		if(db == NULL)
		{
			SHPClose(file);
			return false;
		}

		for(shape_pattern_vector::iterator r = sShapeRules.begin(); r != sShapeRules.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));
	}
	
	SHPGetInfo(file, &entity_count, &shape_type, bounds_lo, bounds_hi);

	switch(shape_type) {
	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_ARCM:
		for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
			e->curve().set_data(-1);
	}

	bounds[0] = bounds_lo[0];
	bounds[1] = bounds_lo[1];

	bounds[2] = bounds_hi[0];
	bounds[3] = bounds_hi[1];

	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")
	
	vector<int>	feature_map;
	feature_map.resize(entity_count,NO_VALUE);

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);
		if((flags & shp_Use_Crop) == 0 || shape_in_bounds(obj))
		if(!db || ((feat = want_this_thing(db, obj->nShapeId)) != NO_VALUE))
		switch(obj->nSHPType) {
		case SHPT_POINT:
		case SHPT_POINTZ:
		case SHPT_POINTM:
		
		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
			if (obj->nVertices > 1)
			{		
				feature_map[n] = feat;			
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					vector<Point_2>	p;
					vector<Point2>	p_raw;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(p.empty() || pt != p.back())
						{
							p.push_back(pt);
							p_raw.push_back(Point2(obj->padfX[i],obj->padfY[i]));
						}
					}
//					DebugAssert(p.size() >= 2);
					for(int i = 1; i < p.size(); ++i)
					{
						DebugAssert(p[i-1] != p[i]);
						bool oob = false;
						if(flags & shp_Use_Crop)
						if ((p_raw[i-1].x() < gMapWest  && p_raw[i].x() < gMapWest ) ||
							(p_raw[i-1].x() > gMapEast  && p_raw[i].x() > gMapEast ) ||
							(p_raw[i-1].y() < gMapSouth && p_raw[i].y() < gMapSouth ) ||
							(p_raw[i-1].y() > gMapNorth && p_raw[i].y() > gMapNorth ))
							oob = true;
						if(!oob)
						ADD_PT_PAIR(p[i-1],p[i],p_raw[i-1],p_raw[i],n);
					}
				}
			}
			break;
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
		case SHPT_POLYGONM:
			if (obj->nVertices > 0)
			{
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					Polygon_2	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(p.is_empty() || pt != p.vertex(p.size()-1))					// Do not add point if it equals the prev!
							p.push_back(pt);
					}
					DebugAssert(p.size() >= 4);
					DebugAssert(p[0] == p[p.size()-1]);
					while(p[0] == p[p.size()-1])
						p.erase(p.vertices_end()-1);
					DebugAssert(p.size() == 0 || p.size() >= 3);
					if(p.size() > 0)
					{
						DebugAssert(p.is_simple());
						if(p.is_counterclockwise_oriented())
						{
							holes.push_back(p);
						} else {
							p.reverse_orientation();
							boundaries.push_back(p);					
						}
					}
				}
				
				if((flags & shp_Fast) == 0)
				{				
					for(list<Polygon_2>::iterator i = boundaries.begin(); i != boundaries.end(); ++i)
						poly_set.join(*i);
					for(list<Polygon_2>::iterator i = holes.begin(); i != holes.end(); ++i)
						poly_set.difference(*i);				
					boundaries.clear();
					holes.clear();

					if(flags & shp_Mode_Map)
					{
						set<Face_handle> faces;
						MapOverlayPolygonSet(io_map, poly_set, NULL, &faces);

						if(flags & shp_Mode_Landuse)
						for(set<Face_handle>::iterator f = faces.begin(); f != faces.end(); ++f)						
							(*f)->data().mTerrainType = feat;
						
						if(flags & shp_Mode_Feature)
						for(set<Face_handle>::iterator f = faces.begin(); f != faces.end(); ++f)						
							(*f)->data().mAreaFeature.mFeatType = feat;

						poly_set.clear();
					}
				}
			}
			break;
		case SHPT_MULTIPOINT:
		case SHPT_MULTIPOINTZ:
		case SHPT_MULTIPOINTM:
		case SHPT_MULTIPATCH:
			break;
		}
		SHPDestroyObject(obj);
	}
	switch(shape_type) {
	case SHPT_POLYGON:
	case SHPT_POLYGONZ:
	case SHPT_POLYGONM:
		
		if((flags & shp_Fast))
		{
			poly_set.join(boundaries.begin(),boundaries.end(),holes.begin(),holes.end());
			holes.clear();
			boundaries.clear();
		}
		
		// Ben says: crop must be done later, EVEN in fast mode...if we try to batch the crop with the
		// union, it gets "lost" - something about the order of ops of the true bulk cropper.  Ah, well, such is life.		
		if((flags & shp_Use_Crop) != 0)
		{
			Polygon_2	crop_box;
			if(gMapWest > -180)
			{
				crop_box.push_back(Point_2(-180,-90));
				crop_box.push_back(Point_2(gMapWest,-90));
				crop_box.push_back(Point_2(gMapWest,90));
				crop_box.push_back(Point_2(-180,90));
				poly_set.difference(crop_box);
				crop_box.clear();
			}

			if(gMapEast < 180)
			{
				crop_box.push_back(Point_2(gMapEast,-90));
				crop_box.push_back(Point_2(180,-90));
				crop_box.push_back(Point_2(180,90));
				crop_box.push_back(Point_2(gMapEast,90));
				poly_set.difference(crop_box);
				crop_box.clear();
			}
		
			if(gMapSouth > -90)
			{
				crop_box.push_back(Point_2(-180, -90));
				crop_box.push_back(Point_2(180, -90));
				crop_box.push_back(Point_2(180, gMapSouth));
				crop_box.push_back(Point_2(-180, gMapSouth));
				poly_set.difference(crop_box);
				crop_box.clear();
			}

			if(gMapNorth < 90)
			{
				crop_box.push_back(Point_2(-180, gMapNorth));
				crop_box.push_back(Point_2(180, gMapNorth));
				crop_box.push_back(Point_2(180, 90));
				crop_box.push_back(Point_2(-180, 90));
				poly_set.difference(crop_box);
				crop_box.clear();
			}
		}

		if((flags & shp_Fast) || (flags & shp_Mode_Map) == 0)		
		{
			Pmwx	local;
			Pmwx *	targ = (flags & shp_Overlay) ? &local : &io_map;
			
			*targ = poly_set.arrangement();

			if(flags & shp_Mode_Simple)
			if(flags & shp_Mode_Landuse)
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
			if(f->contained())
				f->data().mTerrainType = feat;

			if(flags & shp_Mode_Simple)
			if(flags & shp_Mode_Feature)
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
			if(f->contained())
				f->data().mAreaFeature.mFeatType = feat;

			if(flags & shp_Mode_Map)
			if(flags & shp_Mode_Landuse)
			if(!sShapeRules.empty())
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
			if(f->contained())
				f->data().mTerrainType = sShapeRules.front().feature;

			if(flags & shp_Mode_Map)
			if(flags & shp_Mode_Feature)
			if(!sShapeRules.empty())
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
			if(f->contained())
				f->data().mAreaFeature.mFeatType = sShapeRules.front().feature;
				
			if(flags & shp_Overlay)
			{
				Pmwx	src(io_map);				
				MapOverlay(src,local,io_map);
			}
		}
		break;
		
	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_ARCM:

		if((flags & shp_Use_Crop))
		{		
			ADD_PT_PAIR(
								Point_2(gMapWest,gMapSouth),
								Point_2(gMapEast,gMapSouth),
								Point2(gMapWest,gMapSouth),
								Point2(gMapEast,gMapSouth),
								entity_count);

			ADD_PT_PAIR(
								Point_2(gMapEast,gMapSouth),
								Point_2(gMapEast,gMapNorth),
								Point2(gMapEast,gMapSouth),
								Point2(gMapEast,gMapNorth),
								entity_count+1);

			ADD_PT_PAIR(
								Point_2(gMapEast,gMapNorth),
								Point_2(gMapWest,gMapNorth),
								Point2(gMapEast,gMapNorth),
								Point2(gMapWest,gMapNorth),
								entity_count+2);

			ADD_PT_PAIR(
								Point_2(gMapWest,gMapNorth),
								Point_2(gMapWest,gMapSouth),
								Point2(gMapWest,gMapNorth),
								Point2(gMapWest,gMapSouth),
								entity_count+3);
		}
		{
			Pmwx	local;
			Pmwx *	targ = (flags & shp_Overlay) ? &local : &io_map;
			
			CGAL::insert_curves(*targ, curves.begin(), curves.end());
			ValidateMapDominance(*targ);
			Point_2 sw(gMapWest,gMapSouth);
			Point_2 ne(gMapEast,gMapNorth);

			if(flags & shp_Use_Crop)
			for(Pmwx::Edge_iterator e = targ->edges_begin(); e != targ->edges_end();)
			{
				Pmwx::Edge_iterator k(e);
				++e;
				
				if (CGAL::compare_x(k->source()->point(),sw) == CGAL::SMALLER ||
					CGAL::compare_x(k->target()->point(),sw) == CGAL::SMALLER ||
					CGAL::compare_x(k->source()->point(),ne) == CGAL::LARGER ||
					CGAL::compare_x(k->target()->point(),ne) == CGAL::LARGER ||					
					CGAL::compare_y(k->source()->point(),sw) == CGAL::SMALLER ||
					CGAL::compare_y(k->target()->point(),sw) == CGAL::SMALLER ||
					CGAL::compare_y(k->source()->point(),ne) == CGAL::LARGER ||
					CGAL::compare_y(k->target()->point(),ne) == CGAL::LARGER)
				{
					targ->remove_edge(k);
				}
			}

			GISNetworkSegment_t r;
			r.mFeatType = feat;
			r.mRepType = NO_VALUE;
			
			if(flags & shp_Mode_Simple)
			if(flags & shp_Mode_Road)
			for(Pmwx::Halfedge_iterator e = targ->halfedges_begin(); e != targ->halfedges_end(); ++e)
			if(he_is_same_direction(e))
				e->data().mSegments.push_back(r);

			if(flags & shp_Mode_Map)
			if(flags & shp_Mode_Road)
			for(Pmwx::Halfedge_iterator e = targ->halfedges_begin(); e != targ->halfedges_end(); ++e)
			if(he_is_same_direction(e))
			if(e->curve().data().front() >= 0 && e->curve().data().front() < feature_map.size())
			{
				r.mFeatType = feature_map[e->curve().data().front()];
				e->data().mSegments.push_back(r);
			}	
			
			if(flags & shp_Overlay)
			{
				Pmwx	src(io_map);
				MapMerge(src,local,io_map);
			}
		}
		break;
	}
	SHPClose(file);
	if(db)	DBFClose(db);
	PROGRESS_DONE(inFunc, 0, 1, "Reading shape file...")

	return true;
}

