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
#include <proj_api.h>	
#include <shapefil.h>
#include "MapOverlay.h"
#include "MapPolygon.h"
#include "ConfigSystem.h"
#include "MapAlgs.h"
//#include "GISTool_Globals.h"
#include "MapTopology.h"
#include "MapHelpers.h"
#include "PolyRasterUtils.h"

//#include <CGAL/Snap_rounding_2.h>
//#include <CGAL/Snap_rounding_traits_2.h>

#include <CGAL/Sweep_line_2_algorithms.h>

// Ben says: this can be modified to printf the points.  If a shape-file import ever blows up,
// we can use it to rapidly generate a numeric dataset - then we send a test program to the CGAL
// team if they want one.
#define ADD_PT_PAIR(a,b,c,d,e)	curves.push_back(Curve_2(Segment_2((a),(b)),(e)));

#if 0
static void ISR(vector<Curve_2>& io_curves)
{
	vector<Curve_2>			results;
	list<list<Point_2> >	poly_lines;

	
	CGAL::snap_rounding_2<CGAL::Snap_rounding_traits_2<FastKernel>, vector<Curve_2>::const_iterator,list<list<Point_2> > >
		(io_curves.begin(), io_curves.end(), poly_lines, 0.00001, false, false, 1);

	int ts = 0;
	for(list<list<Point_2> >::iterator pli = poly_lines.begin(); pli != poly_lines.end(); ++pli)
		ts += (pli->size() - 1);
	results.reserve(ts);
	
	printf("ISR before: %d after %d\n", io_curves.size(), ts);
	
	int ic = 0;
	for(list<list<Point_2> >::iterator pli = poly_lines.begin(); pli != poly_lines.end(); ++pli, ++ic)
	if(pli->size() > 1)
	{
//		DebugAssert(pli->size() > 1);
		list<Point_2>::iterator s, e;
		s = e = pli->begin();
		++e;
		int this_time = 0;
		while(e != pli->end())
		{
			DebugAssert(*s != *e);
			if(*s != *e)
			{
				results.push_back(Curve_2(Segment_2(*s,*e),io_curves[ic].data()));
				++this_time;
			}
			++s;
			++e;
		}
//		DebugAssert(this_time > 0);
	}
	swap(io_curves,results);
}
#endif

// Shape to feature


template <typename __InputIterator, typename __OutputIterator, typename __Functor>
void douglas_peuker(
			__InputIterator			start,
			__InputIterator			stop,
			__OutputIterator		out,
			double					epsi_2,
			__Functor&				lock_check)
{
	if(start == stop)
		return;
	if(*start == *stop)
	{
		int n = stop - start;
		if (n == 1) 
			return;
		
		__InputIterator p(start);
		++p;
		while(p < stop)
		{
			if(lock_check(*p))
			{
				douglas_peuker(start,p,out,epsi_2,lock_check);
				douglas_peuker(p,stop,out,epsi_2,lock_check);
				return;
			}
			++p;
		}
			
		__InputIterator mid(start);
		advance(mid,n/2);
		douglas_peuker(start,mid,out,epsi_2,lock_check);
		douglas_peuker(mid,stop,out,epsi_2,lock_check);
		return;		
	}
	
	
	Segment2	s(*start, *stop);
	__InputIterator p(start);
	++p;
	double max_d=0.0;
	__InputIterator worst(stop);
	while(p < stop)
	{
		if(lock_check(*p))
		{
			douglas_peuker(start,p,out,epsi_2,lock_check);
			douglas_peuker(p,stop,out,epsi_2,lock_check);
			return;
		}
		double d = s.squared_distance_supporting_line(*p);
		if(d > max_d)
		{
			max_d = d;
			worst = p;
		}
		++p;
	}
	if(max_d >= epsi_2)
	{
		douglas_peuker(start,worst,out,epsi_2,lock_check);
		douglas_peuker(worst,stop,out,epsi_2,lock_check);
	}
	else
	{
		*out++ = *start;
	}
}


static double s_crop[4] = { -180.0, -90.0, 180.0, 90.0 };

struct shape_pattern_t {
	vector<string>		columns;
	vector<int>			dbf_id;
	vector<string>		values;
	int					feature;
};
typedef vector<shape_pattern_t> shape_pattern_vector;

static shape_pattern_vector	sShapeRules;
static shape_pattern_vector sLineReverse;
static shape_pattern_vector sLineBridge;
static string				sLayerTag;
static int					sLayerID;

static projPJ 				sProj=NULL;

static void reproj(Point2& io_pt)
{
	projXY xy;
	projLP lp;
    xy.u = io_pt.x();
    xy.v = io_pt.y();

	lp = pj_inv( xy, sProj);

	io_pt.x_ = lp.u * RAD_TO_DEG;
	io_pt.y_ = lp.v * RAD_TO_DEG;
}

static void reproj(double io_pt[2])
{
	projXY xy;
	projLP lp;
    xy.u = io_pt[0];
    xy.v = io_pt[1];

	lp = pj_inv( xy, sProj);

	io_pt[0] = lp.u * RAD_TO_DEG;
	io_pt[1] = lp.v * RAD_TO_DEG;
}

bool shape_in_bounds(SHPObject * obj)
{
	Point2	lo(obj->dfXMin,obj->dfYMin);
	Point2	hi(obj->dfXMax,obj->dfYMax);
	if(sProj)
	{
		reproj(lo);
		reproj(hi);
	}
	if(hi.x() < s_crop[0]) return false;
	if(lo.x() > s_crop[2]) return false;
	if(hi.y() < s_crop[1]) return false;
	if(lo.y() > s_crop[3]) return false;
						   return true;
}

/*
inline void DEBUG_POLYGON(const Polygon_2& p, const Point3& c1, const Point3& c2)
{
	for(int n = 0; n < p.size(); ++n)
	{
		gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(p[n]),c1));
		gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(p[(n+1)%p.size()]),c2));
	}
}
*/

// SPECIAL CHARS:
// *			Match any empty string
// -			Match null
// !-			Match any non-null (including empty string)
static int want_this_thing(DBFHandle db, int shape_id, const shape_pattern_vector& rules)
{
	for(shape_pattern_vector::const_iterator r = rules.begin(); r != rules.end(); ++r)
	{
		bool rule_ok = true;

		for(int n = 0; n < r->columns.size(); ++n)
		{
			if(r->dbf_id[n] == -1)														{ rule_ok = false; break; }

			const char * field_val = DBFReadStringAttribute(db,shape_id,r->dbf_id[n]);
			if(field_val == NULL && strcmp(r->values[n].c_str(),"-") == 0)				continue;
			if(field_val != NULL && strcmp(r->values[n].c_str(),"!-") == 0)				continue;
							
			if(field_val == NULL)														{ rule_ok = false; break; }
			if(strcmp(r->values[n].c_str(),"*") == 0)
			{
				if(field_val[0] == 0)													{ rule_ok = false; break; }
			}
			else
			{
				if(strcmp(r->values[n].c_str(),field_val) != 0)							{ rule_ok = false; break; }
			}
		}

		if(rule_ok) return r->feature;
	}
	return -1;
}

static bool ShapeLineImporter(const vector<string>& inTokenLine, void * inRef)
{
	if(inTokenLine[0] == "PROJ")
	{
		vector<char*> args;
		for(int n = 1; n < inTokenLine.size(); ++n)
			args.push_back(strdup(inTokenLine[n].c_str()));
		if(sProj) pj_free(sProj);
		sProj = pj_init(args.size(),&*args.begin());
		for(int n = 0; n < args.size(); ++n)
			free(args[n]);
		return true;
	}
	if(inTokenLine[0] == "SHAPE_FEATURE" || inTokenLine[0] == "SHAPE_ARC_REVERSE" || inTokenLine[0] == "SHAPE_ARC_BRIDGE")
	{
		if((inTokenLine[0] == "SHAPE_ARC_REVERSE" && inTokenLine.size() != 3) ||
		    (inTokenLine[0] != "SHAPE_ARC_REVERSE" && inTokenLine.size() != 4))
		{
			printf("Bad shape import line.\n");
			return false;
		}
		shape_pattern_t		pat;

		if(inTokenLine[0] == "SHAPE_FEATURE")
		{
			if(!TokenizeEnum(inTokenLine[3], pat.feature, "Bad enum"))
				return false;
		} else if(inTokenLine[0] == "SHAPE_ARC_BRIDGE")
			pat.feature = atoi(inTokenLine[3].c_str());
		else
			pat.feature = 1;

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
		
		if(inTokenLine[0] == "SHAPE_ARC_REVERSE")
			sLineReverse.push_back(pat);
		else if (inTokenLine[0] == "SHAPE_ARC_BRIDGE")
			sLineBridge.push_back(pat);
		else
			sShapeRules.push_back(pat);
		return true;
	}
	else if (inTokenLine[0] == "ROAD_LAYER_TAG")
	{
		if(inTokenLine.size() != 2)
		{
			printf("Bad shape import line.\n");
			return false;		
		}
		sLayerTag=inTokenLine[1];
		return true;
	}
	return false;
}

class toggle_properties_visitor : public MapBFSVisitor<set<int>, Pmwx > {
public:

	typedef	set<int>	Prop_t;

	const vector<int> *	feature_map;

	virtual	void	initialize_properties(Prop_t& io_properties)
	{
		io_properties.clear();
	}
	
	virtual	void	adjust_properties(Pmwx::Halfedge_handle edge, Prop_t& io_properties)
	{
		for(EdgeKey_iterator k = edge->curve().data().begin(); k != edge->curve().data().end(); ++k)
		{
			int key = *k;
			if(key >= 0 && key < feature_map->size())
			{
				if(io_properties.count(key))	io_properties.erase(key);
				else							io_properties.insert(key);
			}
		}
	}
	
	virtual	void	mark_face(const Prop_t& in_properties, Face_handle face)
	{
		face->set_contained(!in_properties.empty());
		if(!in_properties.empty())
		{
			face->data().mTerrainType = (*feature_map)[*(--in_properties.end())];
		}	
	}
};

static void round_grid(Point2& io_pt, int steps)
{
	int x_steps = round((io_pt.x() - s_crop[0]) * (double) steps / (s_crop[2] - s_crop[0]));
	int y_steps = round((io_pt.y() - s_crop[1]) * (double) steps / (s_crop[3] - s_crop[1]));
	
	io_pt.x_ = s_crop[0] + (double) x_steps * (s_crop[2] - s_crop[0]) / (double) steps;
	io_pt.y_ = s_crop[1] + (double) y_steps * (s_crop[3] - s_crop[1]) / (double) steps;
}

struct shape_lock_traits {
	bool is_locked(Pmwx::Vertex_handle v) const { 

		Pmwx::Halfedge_handle h1(v->incident_halfedges());
		Pmwx::Halfedge_handle h2(h1->next());

		Pmwx::Halfedge_handle t1(h1->twin());
		Pmwx::Halfedge_handle t2(h2->twin());
		
		// Verify our assumption; that all road segments are "flat" on import, that is, source and target height are the same.
		// This is how shape data comes in these days because we get one layer per SEGMENT.  If we ever get a sr/dst height pair,
		// our alg will fail, these asserts will squawk, and we will go to the #if 0 code.
	#if DEV
		GISNetworkSegmentVector::iterator r;
		for(r=h1->data().mSegments.begin();r!=h1->data().mSegments.end();++r)
			DebugAssert(r->mSourceHeight == r->mTargetHeight);
		for(r=t1->data().mSegments.begin();r!=t1->data().mSegments.end();++r)
			DebugAssert(r->mSourceHeight == r->mTargetHeight);
		for(r=h2->data().mSegments.begin();r!=h2->data().mSegments.end();++r)
			DebugAssert(r->mSourceHeight == r->mTargetHeight);
		for(r=t2->data().mSegments.begin();r!=t2->data().mSegments.end();++r)
			DebugAssert(r->mSourceHeight == r->mTargetHeight);
	#endif
		
		// If we don't have matched segments, lock the point so we don't lose the road type data.
		if(h1->data().mSegments == h2->data().mSegments &&
		   t1->data().mSegments == t2->data().mSegments)	return false;

		if(h1->data().mSegments == t2->data().mSegments &&
		   t1->data().mSegments == h2->data().mSegments)	return false;
		
//		debug_mesh_point(cgal2ben(v->point()),1,0,0);
		return true;
	

	// This is a more correct version of the lock alg: if attempts to check what is happening at the
	// vertex to be eliminated.  If we ever have 'ramped' data (src height != dst height) we can use this.
	// Note that in that case when we merge an edge, we would have to 'merge' the segment data's height info too!

#if 0

		double	e1,e2;
		int		f1, f2;

		int r1 = h1->data().mSegments.size() + t1->data().mSegments.size();
		int r2 = h2->data().mSegments.size() + t2->data().mSegments.size();
		if(r1 != r2)
		{
			debug_mesh_point(cgal2ben(v->point()),1,0,0);
			return true;
		}
		
		if(r1 == 0 || r2 == 0)
			return false;
			
		if(r1 > 1 || r2 > 1)
		{
			debug_mesh_point(cgal2ben(v->point()),1,0,1);
			return true;
		}

		if(!h1->data().mSegments.empty())
		{
			e1 = h1->data().mSegments.front().mTargetHeight;
			r1 = h1->data().mSegments.front().mFeatType;
		}
		else if(!t1->data().mSegments.empty())
		{
			e1 = t1->data().mSegments.front().mSourceHeight;
			r1 = t1->data().mSegments.front().mFeatType;
		}
		else
		{
			DebugAssert(!"Logic error.");
			return false;
		}

		if(!h2->data().mSegments.empty())
		{
			e2 = h2->data().mSegments.front().mTargetHeight;
			r2 = h2->data().mSegments.front().mFeatType;
		}
		else if(!t2->data().mSegments.empty())
		{
			e2 = t2->data().mSegments.front().mSourceHeight;
			r2 = t2->data().mSegments.front().mFeatType;
		}
		else
		{
			DebugAssert(!"Logic error.");
			return false;
		}

		if (e1 != e2)
		{
			debug_mesh_point(cgal2ben(v->point()), 0, 0, 1);
			return true;
		}
		if(f1 != f2)
		{
			debug_mesh_point(cgal2ben(v->point()), 0, 1, 0);
			return true;
		}
		return false;
#endif
	}
	void remove(Pmwx::Vertex_handle v) const { 
//		debug_mesh_point(cgal2ben(v->point()),0.6,0.6,0.6);
	}
};

bool	ReadShapeFile(const char * in_file, Pmwx& io_map, shp_Flags flags, const char * feature_desc, double bounds[4], double simplify_mtr, int grid_steps, ProgressFunc	inFunc)
{
		int		killed = 0, total = 0;
		int		entity_count;
		int		shape_type;
		double	bounds_lo[4], bounds_hi[4];
		static bool first_time = true;

	if(sProj) pj_free(sProj);sProj=NULL;


	for(int n = 0; n < 4; ++n)
		s_crop[n] = bounds[n];

	if(first_time)
	{
		RegisterLineHandler("SHAPE_FEATURE", ShapeLineImporter, NULL);
		RegisterLineHandler("ROAD_LAYER_TAG",ShapeLineImporter, NULL);
		RegisterLineHandler("SHAPE_ARC_REVERSE",ShapeLineImporter, NULL);
		RegisterLineHandler("SHAPE_ARC_BRIDGE",ShapeLineImporter, NULL);
		RegisterLineHandler("PROJ",ShapeLineImporter, NULL);
		first_time = false;
	}

	SHPHandle file =  SHPOpen(in_file, "rb");
	if(!file) 
		return false;
		
	// Gotta do this before we crop out the whole file.  Why?  Cuz we might wnat the projection info!
	if(flags & shp_Mode_Map)
	{
		sShapeRules.clear();
		sLineReverse.clear();
		sLineBridge.clear();
		sLayerTag.clear();
		if (!LoadConfigFile(feature_desc))
		{
			printf("Could not load shape mapping file %s\n", feature_desc);
			SHPClose(file);
			return false;
		}
	}
	
	SHPGetInfo(file, &entity_count, &shape_type, bounds_lo, bounds_hi);
	if(sProj) reproj(bounds_lo);
	if(sProj) reproj(bounds_hi);

	if(flags & shp_Use_Crop)
	{
		if(bounds_lo[0] > bounds[2] ||
		   bounds_hi[0] < bounds[0] ||
		   bounds_lo[1] > bounds[3] ||
		   bounds_hi[1] < bounds[1])
		{
			SHPClose(file);
			return true;
		}
	}

	if((flags & shp_Overlay) == 0)	// If we are not overlaying or err checking, nuke the map now.  In _some_ modes (road curve insert,
		io_map.clear();								// one-by-one burn in) we are going to work on the final map, so this is needed.

		vector<Curve_2>							curves;
	
	int feat = NO_VALUE;
	if(flags & shp_Mode_Simple)
		feat = LookupToken(feature_desc);

	DBFHandle db = NULL;
	if(flags & shp_Mode_Map)
	{
		db = DBFOpen(in_file,"rb");
		if(db == NULL)
		{
			printf("Could not open shape DB file.n\n");
			SHPClose(file);
			return false;
		}

		for(shape_pattern_vector::iterator r = sShapeRules.begin(); r != sShapeRules.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));

		for(shape_pattern_vector::iterator r = sLineReverse.begin(); r != sLineReverse.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));

		for(shape_pattern_vector::iterator r = sLineBridge.begin(); r != sLineBridge.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));
		
		
		if(!sLayerTag.empty())
			sLayerID = DBFGetFieldIndex(db, sLayerTag.c_str());

	}


	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
		e->curve().set_data(-1);

	bounds[0] = bounds_lo[0];
	bounds[1] = bounds_lo[1];

	bounds[2] = bounds_hi[0];
	bounds[3] = bounds_hi[1];

	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")

	vector<int>	feature_map, feature_rev, feature_lay;
	feature_map.resize(entity_count,NO_VALUE);
	feature_rev.resize(entity_count,0);
	feature_lay.resize(entity_count,0);
	
	/************************************************************************************************************************************
	 * MAIN SHAPE READING LOOP
	 ************************************************************************************************************************************/

//	if (shape_type == SHPT_ARC ||
//		shape_type == SHPT_ARCZ ||
//		shape_type == SHPT_ARCM)
//	for(int n = 0; n < entity_count; ++n)
//	{
//		SHPObject * obj = SHPReadObject(file, n);
//		if((flags & shp_Use_Crop) == 0 || shape_in_bounds(obj))
//		if(!db || ((feat = want_this_thing(db, obj->nShapeId, sShapeRules)) != -1))
//		for (int part = 0; part < obj->nParts; ++part)
//		{
//			int start_idx = obj->panPartStart[part];
//			int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
//			for (int i = start_idx; i < stop_idx; ++i)
//			{
//				Point2 pt(obj->padfX[i],obj->padfY[i]);
//				if(sProj)		reproj(pt);
//				if(grid_steps)  round_grid(pt, grid_steps);
//				nodes[pt]++;
//			}
//		}
//		SHPDestroyObject(obj);
//	}
//	for(map<Point2,int,lesser_y_then_x>::iterator i = nodes.begin(); i != nodes.end();)
//	{
//		if(i->second < 2)
//		{
//			map<Point2,int>::iterator k = i;
//			++i;
//			nodes.erase(k);
//		}
//		else
//			++i;
//	}
//	printf("%llu nodes locked.\n", (unsigned long long)nodes.size());

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);
		if((flags & shp_Use_Crop) == 0 || shape_in_bounds(obj))
		if(!db || ((feat = want_this_thing(db, obj->nShapeId, sShapeRules)) != -1))
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
				if(db) {
					feature_rev[n] = want_this_thing(db,obj->nShapeId, sLineReverse) != -1 ? 1 : 0;
					if(!sLayerTag.empty() && sLayerID != -1)
						feature_lay[n] = DBFReadIntegerAttribute(db, obj->nShapeId, sLayerID);
					if(sLayerTag.empty() || sLayerID == -1 || DBFIsAttributeNULL(db, obj->nShapeId, sLayerID))
					if(want_this_thing(db,obj->nShapeId,sLineBridge) != -1)
						feature_lay[n] = want_this_thing(db,obj->nShapeId,sLineBridge);
				}	
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					vector<Point2>	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point2 pt(obj->padfX[i],obj->padfY[i]);
						if(sProj)	   reproj(pt);
						if(grid_steps) round_grid(pt, grid_steps);						
						if(p.empty() || pt != p.back())
						{
							p.push_back(pt);
						}
					}
					vector<Point2> reduced;
					swap(p,reduced);
					
					/* 
					// we could see what points we killed.
					for(int dp=0;dp<p.size();++dp)
					if(find(reduced.begin(),reduced.end(),p[dp])==reduced.end())
						debug_mesh_point(p[dp],1,1,1);
					*/
					
					total += (p.size());
//					DebugAssert(reduced.size() >= 2);
					for(int i = 1; i < reduced.size(); ++i)
					{
						DebugAssert(reduced[i-1] != reduced[i]);
						bool oob = false;
						if(flags & shp_Use_Crop)
						if ((reduced[i-1].x() < s_crop[0]  && reduced[i].x() < s_crop[0] ) ||
							(reduced[i-1].x() > s_crop[2]  && reduced[i].x() > s_crop[2] ) ||
							(reduced[i-1].y() < s_crop[1] && reduced[i].y() < s_crop[1] ) ||
							(reduced[i-1].y() > s_crop[3] && reduced[i].y() > s_crop[3] ))
							oob = true;
						if(!oob)
						ADD_PT_PAIR(
							ben2cgal(reduced[i-1]),
							ben2cgal(reduced[i]),
							reduced[i-1],
							reduced[i],
							n);
					}
				}
			}
			break;
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
		case SHPT_POLYGONM:
			if (obj->nVertices > 0)
			{
				feature_map[n] = feat;				
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					Polygon_2	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(grid_steps || sProj) 
						{
							Point2 raw_pt(obj->padfX[i],obj->padfY[i]);
							if(sProj) reproj(raw_pt);
							if(grid_steps) round_grid(raw_pt, grid_steps);
							pt = ben2cgal(raw_pt);
						}
						if(p.is_empty() || pt != p.vertex(p.size()-1))					// Do not add point if it equals the prev!
							p.push_back(pt);
					}
					
					DebugAssert(p[0] == p[p.size()-1]);
					while(p.size() > 0 && p[0] == p[p.size()-1])
						p.erase(p.vertices_end()-1);

					if(p.size() > 2)
					{
						if(p.is_simple())
						{
							for(int s = 0; s < p.size(); ++s)
							{
								curves.push_back(Curve_2(p.edge(s), n));
							}

						} else {

							vector<Polygon_2>	simple_ones;
							MakePolygonSimple(p,simple_ones);
							#if DEV
							for(vector<Polygon_2>::iterator t = simple_ones.begin(); t != simple_ones.end(); ++t)
							{
								DebugAssert(t->is_simple());
								DebugAssert(t->is_counterclockwise_oriented());
							}
							#endif
							for(vector<Polygon_2>::iterator t = simple_ones.begin(); t != simple_ones.end(); ++t)
							{
								for(int s = 0; s < t->size(); ++s)
								{
									curves.push_back(Curve_2(t->edge(s), n));
								}
							}
						}
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

	PROGRESS_DONE(inFunc, 0, 1, "Reading shape file...")
	
	/************************************************************************************************************************************
	 * CROP, INSERT AND TRIM
	 ************************************************************************************************************************************/

	if(flags & shp_ErrCheck)
	if(curves.empty())
	{
		printf("Aborting because the shape file is empty within our insert area.\n");
		return false;
	}


	if((flags & shp_Use_Crop))
	{
		ADD_PT_PAIR(
							Point_2(s_crop[0],s_crop[1]),
							Point_2(s_crop[2],s_crop[1]),
							Point2(s_crop[0],s_crop[1]),
							Point2(s_crop[2],s_crop[1]),
							entity_count);

		ADD_PT_PAIR(
							Point_2(s_crop[2],s_crop[1]),
							Point_2(s_crop[2],s_crop[3]),
							Point2(s_crop[2],s_crop[1]),
							Point2(s_crop[2],s_crop[3]),
							entity_count+1);

		ADD_PT_PAIR(
							Point_2(s_crop[2],s_crop[3]),
							Point_2(s_crop[0],s_crop[3]),
							Point2(s_crop[2],s_crop[3]),
							Point2(s_crop[0],s_crop[3]),
							entity_count+2);

		ADD_PT_PAIR(
							Point_2(s_crop[0],s_crop[3]),
							Point_2(s_crop[0],s_crop[1]),
							Point2(s_crop[0],s_crop[3]),
							Point2(s_crop[0],s_crop[1]),
							entity_count+3);
	}

	Pmwx	local;
	Pmwx *	targ = (flags & shp_Overlay) ? &local : &io_map;
	
	if(flags & shp_ErrCheck)
	{
		Traits_2			tr;
		vector<Point_2>		errs;
		CGAL::compute_intersection_points(curves.begin(), curves.end()-4, back_inserter(errs), false, tr);
		if(!errs.empty())
		{
			printf("File skipped because it contains intersections.\n");
			return false;
		}
	}

	SHPClose(file);
	if(db)	DBFClose(db);
	
//	printf("Inserting %d curves into %d.\n", curves.size(), targ->number_of_edges());
//	ISR(curves);
	CGAL::insert(*targ, curves.begin(), curves.end());

	nuke_container(curves);
	Point_2 sw(s_crop[0],s_crop[1]);
	Point_2 ne(s_crop[2],s_crop[3]);

	/************************************************************************************************************************************
	 * ATTRIBUTE APPLY!
	 ************************************************************************************************************************************/

	switch(shape_type) {
	case SHPT_POLYGON:
	case SHPT_POLYGONZ:
	case SHPT_POLYGONM:
		{
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
				f->set_visited(false);

			toggle_properties_visitor	visitor;
			visitor.feature_map = &feature_map;
			
			visitor.Visit(targ);

			int count = 0;
			// map: crop last!  Otherwise closed polygons "leak" into open area...we are NOT clever enough
			// to tag the open area by membership, so BFS first.
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
					++count;
				}
			}
			if(count) printf("Removed: %d edges.\n", count);
			
			if(simplify_mtr)
			{
				printf("Before import simplify: %d. ", targ->number_of_halfedges());
//				MapSimplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT);
				arrangement_simplifier<Pmwx, shape_lock_traits> simplifier;
				simplifier.simplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT, shape_lock_traits(), inFunc);

				printf("After import simplify: %d.\n", targ->number_of_halfedges());
			}
			
			if(flags & shp_Overlay)
			{
				nuke_container(feature_map);
				nuke_container(feature_rev);
				nuke_container(feature_lay);			
			
				Pmwx	src(io_map);
				io_map.clear();
				MapOverlay(src,local,io_map);
			}

		}
		break;
	
	
	
/*			Pmwx	local;
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
*/
	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_ARCM:
		{
			// Lines: crop first - just get rid of data, save time.
			int count = 0;
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
					++count;
					targ->remove_edge(k);
				}
			}
			if(count) printf("Removed: %d edges.\n", count);
			
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
			for(Pmwx::Edge_iterator eit = targ->edges_begin(); eit != targ->edges_end(); ++eit)
			for(EdgeKey_iterator k = eit->curve().data().begin(); k != eit->curve().data().end(); ++k)
			if(*k < entity_count)
			{
				r.mFeatType = feature_map[*k];
				r.mSourceHeight = r.mTargetHeight = feature_lay[*k];
				if(feature_rev[*k])
				{
					if(he_is_same_direction(eit))
						eit->twin()->data().mSegments.push_back(r);
					else
						eit->data().mSegments.push_back(r);
				}
				else
				{
					if(he_is_same_direction(eit))
						eit->data().mSegments.push_back(r);
					else
						eit->twin()->data().mSegments.push_back(r);
				}
			}

			if(flags & shp_Mode_Coastline)
			{
				int feature = 0;
				if(flags & shp_Mode_Simple)
					feature = feat;
				if(flags & shp_Mode_Map)
					feature = sShapeRules.front().feature;
		
				bool hosed = false;
				for(Pmwx::Edge_iterator e = targ->edges_begin(); e != targ->edges_end(); ++e)
				if(*e->curve().data().begin() < entity_count)
				{
					Pmwx::Halfedge_handle ee = he_get_same_direction(Halfedge_handle(e));
					ee->twin()->face()->set_contained(true);					
					ee->twin()->face()->data().mTerrainType = feature;
					if(ee->face()->contained())
						hosed = true;
				}
			}

			if(simplify_mtr)
			{
				printf("Before import simplify: %d. ", targ->number_of_halfedges());
//				MapSimplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT);
				arrangement_simplifier<Pmwx, shape_lock_traits> simplifier;
				simplifier.simplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT,shape_lock_traits(), inFunc);
				printf("After import simplify: %d.\n", targ->number_of_halfedges());
			}
			
			if(flags & shp_Overlay)
			{
				nuke_container(feature_map);
				nuke_container(feature_rev);
				nuke_container(feature_lay);	
				int n1 = SimplifyMap(io_map,false,NULL);
				int n2 = SimplifyMap(local,false,NULL);
				printf("Simplify: %d and %d\n", n1, n2);
				Pmwx	src(io_map);
				io_map.clear();
				MapMerge(src,local,io_map);
			}
		}
		break;
	}
	printf("DP killed off %d points of %d.\n", killed, total);

	return true;
}

bool	RasterShapeFile(
			const char *			inFile,
			DEMGeo&					dem,
			shp_Flags				flags,
			const char *			feature_desc,
			ProgressFunc			inFunc)
{
		int		entity_count;
		int		shape_type;
		double	bounds_lo[4], bounds_hi[4];
		static	bool first_time = true;

	if(sProj) pj_free(sProj);sProj=NULL;

	s_crop[0] = dem.mWest;
	s_crop[1] = dem.mSouth;
	s_crop[2] = dem.mEast;
	s_crop[3] = dem.mNorth;
	
	if(first_time)
	{
		RegisterLineHandler("SHAPE_FEATURE", ShapeLineImporter, NULL);
		RegisterLineHandler("ROAD_LAYER_TAG",ShapeLineImporter, NULL);
		RegisterLineHandler("SHAPE_ARC_REVERSE",ShapeLineImporter, NULL);
		RegisterLineHandler("SHAPE_ARC_BRIDGE",ShapeLineImporter, NULL);
		RegisterLineHandler("PROJ",ShapeLineImporter, NULL);
		first_time = false;
	}

	SHPHandle file =  SHPOpen(inFile, "rb");
	if(!file) 
		return false;
		
	// Gotta do this before we crop out the whole file.  Why?  Cuz we might wnat the projection info!
	if(flags & shp_Mode_Map)
	{
		sShapeRules.clear();
		sLineReverse.clear();
		sLineBridge.clear();
		sLayerTag.clear();
		if (!LoadConfigFile(feature_desc))
		{
			printf("Could not load shape mapping file %s\n", feature_desc);
			SHPClose(file);
			return false;
		}
	}
	
	SHPGetInfo(file, &entity_count, &shape_type, bounds_lo, bounds_hi);
	
	if(sProj) reproj(bounds_lo);
	if(sProj) reproj(bounds_hi);

	if(flags & shp_Use_Crop)
	{
		if(bounds_lo[0] > dem.mEast  ||
		   bounds_hi[0] < dem.mWest  ||
		   bounds_lo[1] > dem.mNorth ||
		   bounds_hi[1] < dem.mSouth)
		{
			SHPClose(file);
			return true;
		}
	}

	map<int, PolyRasterizer<double> >	rasterizers;
	
	int feat = NO_VALUE;
	if(flags & shp_Mode_Simple)
		feat = LookupToken(feature_desc);
	else
		feat = atoi(feature_desc);

	DBFHandle db = NULL;
	if(flags & shp_Mode_Map)
	{
		db = DBFOpen(inFile,"rb");
		if(db == NULL)
		{
			printf("Could not open shape DB file.n\n");
			SHPClose(file);
			return false;
		}

		for(shape_pattern_vector::iterator r = sShapeRules.begin(); r != sShapeRules.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));

		for(shape_pattern_vector::iterator r = sLineReverse.begin(); r != sLineReverse.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));

		for(shape_pattern_vector::iterator r = sLineBridge.begin(); r != sLineBridge.end(); ++r)
		for(vector<string>::iterator c = r->columns.begin(); c != r->columns.end(); ++c)
			r->dbf_id.push_back(DBFGetFieldIndex(db,c->c_str()));
		
		
		if(!sLayerTag.empty())
			sLayerID = DBFGetFieldIndex(db, sLayerTag.c_str());

	}

	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")

	/************************************************************************************************************************************
	 * MAIN SHAPE READING LOOP
	 ************************************************************************************************************************************/

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);
		if((flags & shp_Use_Crop) == 0 || shape_in_bounds(obj))
		if(!db || ((feat = want_this_thing(db, obj->nShapeId, sShapeRules)) != -1))
		switch(obj->nSHPType) {
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
		case SHPT_POLYGONM:
			if (obj->nVertices > 0)
			{
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					Polygon2	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point2 pt(obj->padfX[i],obj->padfY[i]);
						if(sProj) 
						{
							if(sProj) reproj(pt);
						}
						if(p.empty() || pt != p[p.size()-1])					// Do not add point if it equals the prev!
							p.push_back(pt);
					}
					
					DebugAssert(p[0] == p[p.size()-1]);
					while(p.size() > 0 && p[0] == p[p.size()-1])
						p.erase(p.end()-1);

					PolyRasterizer<double>& rasterizer(rasterizers[feat]);

					if(p.size() > 2)
					{
						for(int s = 0; s < p.size(); ++s)
						{
							Segment2 si(p.side(s));
							rasterizer.AddEdge(	dem.lon_to_x(si.p1.x()),
												dem.lat_to_y(si.p1.y()),
												dem.lon_to_x(si.p2.x()),
												dem.lat_to_y(si.p2.y()));
						}
						
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

	PROGRESS_DONE(inFunc, 0, 1, "Reading shape file...")
	
	/************************************************************************************************************************************
	 * PROCESS
	 ************************************************************************************************************************************/
	SHPClose(file);
	if(db)	DBFClose(db);
	
	for(map<int, PolyRasterizer<double> >::iterator r = rasterizers.begin(); r != rasterizers.end(); ++r)
	{
		r->second.SortMasters();		
		int x, y = 0;
		r->second.StartScanline(y);
		while (!r->second.DoneScan())
		{
			DebugAssert(y >= 0);
			if (y >= dem.mHeight) 
				break;

			int x1, x2;
			while (r->second.GetRange(x1, x2))
			{
				x1 = max(x1,0);
				x2 = min(x2,dem.mWidth);
				
				for (x = x1; x < x2; ++x)
				{
					dem(x,y) = r->first;
				}
			}
			++y;
			r->second.AdvanceScanline(y);
		}
	}

	return true;
}
