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
#include "GISTool_Globals.h"
#include "MapTopology.h"
#include "MapHelpers.h"
#include "PolyRasterUtils.h"

// This will cause us to debug-show red/green outlines of the .shp data to see what we imported and what we had to drop.
#define SHOW_FEATURE_IMPORT		0

//#include <CGAL/Snap_rounding_2.h>
//#include <CGAL/Snap_rounding_traits_2.h>
#if !defined(__i386__) && defined(IBM)
#define __i386__
#define __i386__defined 1
#endif
#include <CGAL/Sweep_line_2_algorithms.h>
#if __i386__defined
#undef __i386__
#undef __i386__defined
#endif
// Ben says: this can be modified to printf the points.  If a shape-file import ever blows up,
// we can use it to rapidly generate a numeric dataset - then we send a test program to the CGAL
// team if they want one.
#define ADD_PT_PAIR(a,b,c,d,e)	do { curves.emplace_back(Curve_2(Segment_2((a),(b)),(curves.size()))); curve_feature.push_back((e)); } while (0)


/*
	ShapeIO config files:
	
	SHAPE_FEATURE		<column name list>	<value name list>	feature tag

	If each of the cols matches each of the values, use this feature (and keep the immport).
	Special col values: 
				 *			Match any empty string
				 -			Match null
				 !-			Match any non-null (including empty string)


	SHAPE_ARC_BRIDGE	<column name list>	<value name list>	resulting layer number

	If each col has the respective value, the layer number is applied to the road

	SHAPE_ARC_REVERSE	<column name list>	<value name list>
	
	If each col has the respective value, we reverse our direction from the shapefile direction.
	
	ROAD_LAYER_TAG		<single col name>
	
	This IDs the column where road layers come from.
	
	PROJ <projection params>
	
	This does a reprojection based on proj lib.
	
	COLUMN shape_col rf_key
	
	This maps a column in the shape file to a param of the GIS object.


*/

// Shape to feature

static double s_crop[4] = { -180.0, -90.0, 180.0, 90.0 };

struct shape_import_data {
	explicit shape_import_data(int p) : feature(p) { }
	int					feature;
	GISParamMap			params;
};

struct shape_pattern_t {
	vector<string>		columns;
	vector<int>			dbf_id;
	vector<string>		values;
	int					feature = 0;
};
typedef vector<shape_pattern_t> shape_pattern_vector;

struct import_column_t {
	string				col_name;			// this is the name of the col in the shape file.
	int					dbf_id = -1;
	const int			rf_key;				// This is the enum to key off of for RF.
};
typedef vector<import_column_t>	import_column_vector;

static import_column_vector	sImportColumns;
static shape_pattern_vector	sShapeRules;
static shape_pattern_vector sLineReverse;
static shape_pattern_vector sLineBridge;
static string				sLayerTag;
static int					sLayerID;



static projPJ 				sProj=nullptr;

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
static int want_this_thing(DBFHandle db, int shape_id, const shape_pattern_vector& rules, int * value)
{
	for(const auto & rule : rules)
	{
		bool rule_ok = true;

		for(int n = 0; n < rule.columns.size(); ++n)
		{
			if(rule.dbf_id[n] == -1)														{ rule_ok = false; break; }

			const char * field_val = DBFReadStringAttribute(db,shape_id,rule.dbf_id[n]);
			if(field_val == nullptr && strcmp(rule.values[n].c_str(),"-") == 0)			continue;
			if(field_val != nullptr && strcmp(rule.values[n].c_str(),"!-") == 0)		continue;

			if(field_val == nullptr)													{ rule_ok = false; break; }
			if(strcmp(rule.values[n].c_str(),"*") == 0)
			{
				if(field_val[0] == 0)													{ rule_ok = false; break; }
			}
			else
			{
				if(strcmp(rule.values[n].c_str(),field_val) != 0)						{ rule_ok = false; break; }
			}
		}

		if(rule_ok)
		{
			if(rule.feature == NO_VALUE)
				return 0;
		
			if(value) *value = rule.feature;
			return 1;
		}

	}
	return 0;
}

static bool ShapeLineImporter(const vector<string>& inTokenLine, void * inRef)
{
	if(inTokenLine[0] == "COLUMN")
	{
		if(inTokenLine.size() != 3)
			return false;
		
		import_column_t ic {
			inTokenLine[1],
			-1,
			LookupTokenCreate(inTokenLine[2].c_str()),
		};
		sImportColumns.push_back(ic);
		return true;
	}
	if(inTokenLine[0] == "PROJ")
	{
		vector<char*> args;
		for(int n = 1; n < inTokenLine.size(); ++n)
			args.push_back(const_cast<char*>(inTokenLine[n].c_str()));
		if(sProj) pj_free(sProj);
		sProj = pj_init(args.size(),&*args.begin());
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
			pat.columns.emplace_back(s,e);
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
			pat.values.emplace_back(s,e);
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

	const vector<int> *					curve_feature = nullptr;
	const vector<shape_import_data> *	feature_map = nullptr;

	void initialize_properties(Prop_t& io_properties) override
	{
		io_properties.clear();
	}

	void adjust_properties(Pmwx::Halfedge_handle edge, Prop_t& io_properties) override
	{
		for(const auto& key : edge->curve().data())
		{
			if (key < 0 || key >= curve_feature->size())
				continue;

			const auto feature = (*curve_feature)[key];
			if(feature >= 0 && feature < feature_map->size())
			{
				if(io_properties.count(feature))	io_properties.erase(feature);
				else								io_properties.insert(feature);
			}
		}
	}

	void mark_face(const Prop_t& in_properties, Face_handle face) override
	{
		face->set_contained(!in_properties.empty());
		if(!in_properties.empty())
		{
			face->data().mTerrainType = (*feature_map)[*(--in_properties.end())].feature;
			face->data().mParams = (*feature_map)[*(--in_properties.end())].params;
		}
	}
};

static bool desliver_border(Point2& io_pt)
{
	auto snap_near = [](double& v, double b) -> bool
	{
		const double epsi = 0.0000001;
		if(fabs(v - b) < epsi)
		{
			v = b;
			return true;
		}
		return false;
	};
	if(snap_near(io_pt.x_, s_crop[0]))
		return true;
	if(snap_near(io_pt.x_, s_crop[2]))
		return true;
	if(snap_near(io_pt.y_, s_crop[1]))
		return true;
	if(snap_near(io_pt.y_, s_crop[3]))
		return true;
	return false;
}

static void dump_desliver(const Point2& fixed, double raw_x, double raw_y)
{
	printf("Fixed %.10lf, %.10lf (%16llx, %16llx) -> %.10lf, %.10lf (%16llx, %16llx)", raw_x,raw_y,*((unsigned long long *) &raw_x), *((unsigned long long *) &raw_y),
				fixed.x_, fixed.y_, *((unsigned long long *) &fixed.x_), *((unsigned long long *) &fixed.y_));
}


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
	int		killed = 0;
	size_t	total = 0;
	int		entity_count;
	int		shape_type;
	double	bounds_lo[4], bounds_hi[4];
	static bool first_time = true;
	const bool read_z = (flags & shp_Altitude) == shp_Altitude;

	if(sProj) pj_free(sProj);sProj=nullptr;


	for(int n = 0; n < 4; ++n)
		s_crop[n] = bounds[n];

	if(first_time)
	{
		RegisterLineHandler("SHAPE_FEATURE", ShapeLineImporter, nullptr);
		RegisterLineHandler("ROAD_LAYER_TAG",ShapeLineImporter, nullptr);
		RegisterLineHandler("SHAPE_ARC_REVERSE",ShapeLineImporter, nullptr);
		RegisterLineHandler("SHAPE_ARC_BRIDGE",ShapeLineImporter, nullptr);
		RegisterLineHandler("PROJ",ShapeLineImporter, nullptr);
		RegisterLineHandler("COLUMN", ShapeLineImporter, nullptr);
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
		sImportColumns.clear();
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

//	if(flags & shp_Use_Crop)
//	{
//		if(bounds_lo[0] > bounds[2] ||
//		   bounds_hi[0] < bounds[0] ||
//		   bounds_lo[1] > bounds[3] ||
//		   bounds_hi[1] < bounds[1])
//		{
//			SHPClose(file);
//			return true;
//		}
//	}

	if((flags & shp_Overlay) == 0)	// If we are not overlaying or err checking, nuke the map now.  In _some_ modes (road curve insert,
		io_map.clear();								// one-by-one burn in) we are going to work on the final map, so this is needed.

	vector<Curve_2>							curves;
	// Curve_2 -> feature_map index
	vector<int>							curve_feature;
	vector<tuple<boost::optional<double>,boost::optional<double>>> curve_elevation;

	int feat = NO_VALUE;
	if(flags & shp_Mode_Simple)
		feat = LookupToken(feature_desc);

	DBFHandle db = nullptr;
	if(flags & shp_Mode_Map)
	{
		db = DBFOpen(in_file,"rb");
		if(db == nullptr)
		{
			printf("Could not open shape DB file.n\n");
			SHPClose(file);
			return false;
		}

		for(auto & r : sShapeRules)
		for(const auto & c : r.columns)
			r.dbf_id.push_back(DBFGetFieldIndex(db,c.c_str()));

		for(auto & r : sLineReverse)
		for(const auto & c : r.columns)
			r.dbf_id.push_back(DBFGetFieldIndex(db,c.c_str()));

		for(auto & r : sLineBridge)
		for(const auto & c : r.columns)
			r.dbf_id.push_back(DBFGetFieldIndex(db,c.c_str()));


		if(!sLayerTag.empty())
			sLayerID = DBFGetFieldIndex(db, sLayerTag.c_str());
		for(auto & r : sImportColumns)
			r.dbf_id = DBFGetFieldIndex(db, r.col_name.c_str());

	}


	for(Pmwx::Edge_iterator e = io_map.edges_begin(); e != io_map.edges_end(); ++e)
		e->curve().set_data(-1);

	bounds[0] = bounds_lo[0];
	bounds[1] = bounds_lo[1];

	bounds[2] = bounds_hi[0];
	bounds[3] = bounds_hi[1];

	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")

	vector<shape_import_data>	feature_map;
	vector<int>					feature_rev, 
								feature_lay;
	feature_map.resize(entity_count,shape_import_data(NO_VALUE));
	feature_rev.resize(entity_count,0);
	feature_lay.resize(entity_count,0);

	/************************************************************************************************************************************
	 * MAIN SHAPE READING LOOP
	 ************************************************************************************************************************************/

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);
		if((flags & shp_Use_Crop) == 0 || shape_in_bounds(obj))
		if(!db || want_this_thing(db, obj->nShapeId, sShapeRules, &feat))
		switch(obj->nSHPType) {
		case SHPT_POINT:
		case SHPT_POINTZ:
		case SHPT_POINTM:

		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
			if (obj->nVertices > 1)
			{
				feature_map[n].feature = feat;
				if(db) {
					feature_rev[n] = want_this_thing(db,obj->nShapeId, sLineReverse, nullptr) ? 1 :0;
					if(!sLayerTag.empty() && sLayerID != -1)
						feature_lay[n] = DBFReadIntegerAttribute(db, obj->nShapeId, sLayerID);
					if(sLayerTag.empty() || sLayerID == -1 || DBFIsAttributeNULL(db, obj->nShapeId, sLayerID))
					want_this_thing(db,obj->nShapeId,sLineBridge, &feature_lay[n]);
				}
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					vector<Point2>	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point2 pt(obj->padfX[i],obj->padfY[i]);

						if(flags & shp_Use_Crop)
						{
							if(desliver_border(pt))
								dump_desliver(pt,obj->padfX[i], obj->padfY[i]);
						}
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
						const auto is_outside_crop = [](const Point2& a, const Point2& b) -> bool {
							return ((a.x() < s_crop[0] && b.x() < s_crop[0] ) ||
									(a.x() > s_crop[2] && b.x() > s_crop[2] ) ||
									(a.y() < s_crop[1] && b.y() < s_crop[1] ) ||
									(a.y() > s_crop[3] && b.y() > s_crop[3] ));
						};

						bool oob = false;
						if(flags & shp_Use_Crop)
							oob = is_outside_crop(reduced[i-1], reduced[i]);
						if(!oob)
							ADD_PT_PAIR(
								ben2cgal<Point_2>(reduced[i-1]),
								ben2cgal<Point_2>(reduced[i]),
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
				feature_map[n].feature = feat;
				for(auto & r : sImportColumns)
				if(r.dbf_id != -1)
				{
					const char * field_val = DBFReadStringAttribute(db,obj->nShapeId,r.dbf_id);
					if(field_val && field_val[0])
					{
						float f = TokenizeFloatWithEnum(field_val);
						feature_map[n].params[r.rf_key] = f;
					}
				}

				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					vector<tuple<Point_2, boost::optional<double>>> p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);

						boost::optional<double> pt_z;
						if (read_z && obj->padfZ)
							pt_z = obj->padfZ[i];

						if(grid_steps || sProj || flags & shp_Use_Crop)
						{
							Point2 raw_pt(obj->padfX[i],obj->padfY[i]);

							if(flags & shp_Use_Crop)
							{
								if(desliver_border(raw_pt))
									dump_desliver(raw_pt,obj->padfX[i], obj->padfY[i]);
							}
							
							if(sProj) reproj(raw_pt);
							if(grid_steps) round_grid(raw_pt, grid_steps);
							pt = ben2cgal<Point_2>(raw_pt);
						}

						// Do not add point if it equals the prev!
						if (p.empty() || pt != get<0>(p.back()))
						{
							p.emplace_back(pt, pt_z);
						}
					}

					DebugAssert(p[0] == p[p.size()-1]);
					// Ignore any Z value for start & end merging
					while(!p.empty() && std::get<0>(p[0]) == std::get<0>(p.back()))
						p.erase(p.end()-1);

					if(p.size() > 2)
					{
						// Convert part from a sequence of points to a sequence of curves.
						for (size_t i = 0, j = 1; i < p.size(); ++i)
						{
							j = (i + 1) % p.size();

							if (read_z && obj->nSHPType == SHPT_POLYGONZ)
							{
								DebugAssert(get<1>(p[i]).has_value());
								DebugAssert(get<1>(p[j]).has_value());
								curve_elevation.emplace_back(get<1>(p[i]), get<1>(p[j]));
							}
							else
							{
								curve_elevation.emplace_back();
							}

							ADD_PT_PAIR(
									get<0>(p[i]),
									get<0>(p[j]),
									get<0>(p[i]),
									get<0>(p[j]),
									n);
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
	{
		if (curves.empty())
		{
			printf("Aborting because the shape file is empty within our insert area.\n");
			return false;
		}
	}

	if((flags & shp_Use_Crop))
	{
		ADD_PT_PAIR(
							Point_2(s_crop[0],s_crop[1]),
							Point_2(s_crop[2],s_crop[1]),
							Point2(s_crop[0],s_crop[1]),
							Point2(s_crop[2],s_crop[1]),
							-1);

		ADD_PT_PAIR(
							Point_2(s_crop[2],s_crop[1]),
							Point_2(s_crop[2],s_crop[3]),
							Point2(s_crop[2],s_crop[1]),
							Point2(s_crop[2],s_crop[3]),
							-1);

		ADD_PT_PAIR(
							Point_2(s_crop[2],s_crop[3]),
							Point_2(s_crop[0],s_crop[3]),
							Point2(s_crop[2],s_crop[3]),
							Point2(s_crop[0],s_crop[3]),
							-1);

		ADD_PT_PAIR(
							Point_2(s_crop[0],s_crop[3]),
							Point_2(s_crop[0],s_crop[1]),
							Point2(s_crop[0],s_crop[3]),
							Point2(s_crop[0],s_crop[1]),
							-1);
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

	CGAL::insert(*targ, curves.begin(), curves.end());

	nuke_container(curves);
	Point_2 sw(s_crop[0],s_crop[1]);
	Point_2 ne(s_crop[2],s_crop[3]);

	/************************************************************************************************************************************
	 * ATTRIBUTE APPLY!
	 ************************************************************************************************************************************/

	switch(shape_type) {
	default: break;
	case SHPT_POLYGON:
	case SHPT_POLYGONZ:
	case SHPT_POLYGONM:
		{
			for(Pmwx::Face_iterator f = targ->faces_begin(); f != targ->faces_end(); ++f)
				f->set_visited(false);

			toggle_properties_visitor	visitor;
			visitor.curve_feature = &curve_feature;
			visitor.feature_map = &feature_map;

			visitor.Visit(targ);

			int count = 0;
			// map: crop last!  Otherwise closed polygons "leak" into open area...we are NOT clever enough
			// to tag the open area by membership, so BFS first.
			if(flags & shp_Use_Crop)
			{
				for (Pmwx::Edge_iterator e = targ->edges_begin(); e != targ->edges_end();)
				{
					Pmwx::Edge_iterator k(e);
					++e;

					if (CGAL::compare_x(k->source()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_x(k->target()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_x(k->source()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_x(k->target()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_y(k->source()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_y(k->target()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_y(k->source()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_y(k->target()->point(), ne) == CGAL::LARGER)
					{
						targ->remove_edge(k);
						++count;
					}
				}
			}
			if(count) printf("Removed: %d edges.\n", count);

			if(simplify_mtr > 0.0)
			{
				printf("Before import simplify: %zd. ", targ->number_of_halfedges());
//				MapSimplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT);
				arrangement_simplifier<Pmwx, shape_lock_traits> simplifier;
				simplifier.simplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT, shape_lock_traits(), inFunc);

				printf("After import simplify: %zd.\n", targ->number_of_halfedges());
			}

			for (auto he = targ->halfedges_begin(); he != targ->halfedges_end(); ++he)
			{
				for (const auto& key : he->curve().data())
				{
					if (key < 0 || key >= curve_elevation.size())
						continue;

					if (read_z)
					{
						const auto elevation = std::get<0>(curve_elevation[key]);
						he->source()->data().mElevation = elevation;
						if (elevation && !he->face()->is_unbounded())
						{
							// Edges with elevation must be burned to preserve the contour
							he->data().mParams.emplace(he_MustBurn, 1.0);
							he->face()->data().mHasElevation = true;
						}
					}
				}
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

	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_ARCM:
		{
			// Lines: crop first - just get rid of data, save time.
			int count = 0;
			if(flags & shp_Use_Crop)
			{
				for (Pmwx::Edge_iterator e = targ->edges_begin(); e != targ->edges_end();)
				{
					Pmwx::Edge_iterator k(e);
					++e;

					if (CGAL::compare_x(k->source()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_x(k->target()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_x(k->source()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_x(k->target()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_y(k->source()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_y(k->target()->point(), sw) == CGAL::SMALLER ||
						CGAL::compare_y(k->source()->point(), ne) == CGAL::LARGER ||
						CGAL::compare_y(k->target()->point(), ne) == CGAL::LARGER)
					{
						++count;
						targ->remove_edge(k);
					}
				}
			}
			if(count) printf("Removed: %d edges.\n", count);

			GISNetworkSegment_t r {};
			r.mFeatType = feat;
			r.mRepType = NO_VALUE;

			if ((flags & shp_Mode_Simple) && (flags & shp_Mode_Road))
				for(Pmwx::Halfedge_iterator e = targ->halfedges_begin(); e != targ->halfedges_end(); ++e)
					if(he_is_same_direction(e))
						e->data().mSegments.push_back(r);

			if ((flags & shp_Mode_Map) && (flags & shp_Mode_Road))
				for(Pmwx::Edge_iterator eit = targ->edges_begin(); eit != targ->edges_end(); ++eit)
					for(const auto& key : eit->curve().data())
					{
						const auto feature = curve_feature[key];
						if (feature > 0 && feature < entity_count)
						{
							r.mFeatType = feature_map[feature].feature;
							r.mSourceHeight = r.mTargetHeight = feature_lay[feature];
							if (feature_rev[feature])
							{
								if (he_is_same_direction(eit))
									eit->twin()->data().mSegments.push_back(r);
								else
									eit->data().mSegments.push_back(r);
							} else
							{
								if (he_is_same_direction(eit))
									eit->data().mSegments.push_back(r);
								else
									eit->twin()->data().mSegments.push_back(r);
							}
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
				{
					const auto key = *e->curve().data().begin();
					if (key > 0 && key < entity_count)
					{
						Pmwx::Halfedge_handle ee = he_get_same_direction(Halfedge_handle(e));
						ee->twin()->face()->set_contained(true);
						ee->twin()->face()->data().mTerrainType = feature;
						if(ee->face()->contained())
							hosed = true;
					}
				}
			}

			if(simplify_mtr > 0.0)
			{
				printf("Before import simplify: %zd. ", targ->number_of_halfedges());
//				MapSimplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT);
				arrangement_simplifier<Pmwx, shape_lock_traits> simplifier;
				simplifier.simplify(*targ, simplify_mtr*MTR_TO_NM * NM_TO_DEG_LAT,shape_lock_traits(), inFunc);
				printf("After import simplify: %zd.\n", targ->number_of_halfedges());
			}

			if(flags & shp_Overlay)
			{
				nuke_container(feature_map);
				nuke_container(feature_rev);
				nuke_container(feature_lay);
				int n1 = SimplifyMap(io_map,false,nullptr);
				int n2 = SimplifyMap(local,false,nullptr);
				printf("Simplify: %d and %d\n", n1, n2);
				Pmwx	src(io_map);
				io_map.clear();
				MapMerge(src,local,io_map);
			}
		}
		break;
	}
	printf("DP killed off %d points of %zu.\n", killed, total);

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
		sImportColumns.clear();
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

	DEMGeo orig;
	if(flags & shp_Overlay)
	{
		orig = dem;
		dem = DEM_NO_DATA;
	}

	map<int, PolyRasterizer<double> >	rasterizers;
	map<int, vector<Segment2> >			edges;

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
		for(import_column_vector::iterator r = sImportColumns.begin(); r != sImportColumns.end(); ++r)
			r->dbf_id = DBFGetFieldIndex(db, r->col_name.c_str());

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
		if(!db || want_this_thing(db, obj->nShapeId, sShapeRules, &feat))
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
					vector<Segment2>& edge_list(edges[feat]);

					if(p.size() > 2)
					{
						for(int s = 0; s < p.size(); ++s)
						{
							Segment2 si(p.side(s));

							if(flags & shp_Outline)
							edge_list.push_back(Segment2(
													Point2(
														dem.lon_to_x(si.p1.x()),
														dem.lat_to_y(si.p1.y())),
													Point2(
														dem.lon_to_x(si.p2.x()),
														dem.lat_to_y(si.p2.y()))));
			
//							debug_mesh_line(si.p1,si.p2,9,0,1,0,0,1);
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

	for(map<int, vector<Segment2> >::iterator edge_list = edges.begin(); edge_list != edges.end(); ++edge_list)
	{
		for(vector<Segment2>::iterator e = edge_list->second.begin(); e != edge_list->second.end(); ++e)
		{
			double count = ceil(sqrt(e->squared_length())) * 4.0;
			
			for(double c = 0; c <= count; ++c)
			{
				double x = interp(0,e->p1.x(),count,e->p2.x(),c);
				double y = interp(0,e->p1.y(),count,e->p2.y(),c);
				int xx = round(x);
				int yy = round(y);
				if(flags & shp_Overlay)
				{
					dem(xx,yy) = orig(xx,yy);
				}
				else
				{
					dem(xx,yy) = edge_list->first;
				}
			}
		}
	}

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

				if(flags & shp_Overlay)
				{
					for (x = x1; x < x2; ++x)
					{
						dem(x,y) = orig(x,y);
					}
				}
				else
				{
					for (x = x1; x < x2; ++x)
					{
						dem(x,y) = r->first;
					}
				}
			}
			++y;
			r->second.AdvanceScanline(y);
		}
	}

	return true;
}

bool	ReadShapeFile(
			const char *			in_file,
			Pmwx&					io_map,
			double					lim_west,
			double					lim_south,
			double					lim_east,
			double					lim_north,
			ProgressFunc			inFunc)
{
	SHPHandle file =  SHPOpen(in_file, "rb");
	if(!file)
	{
		fprintf(stderr,"Unable to open shape file '%s'\n", in_file);
		return false;
	}
	DBFHandle db = DBFOpen(in_file,"rb");
	if(!db)
	{
		fprintf(stderr,"Unable to open shape file DB '%s'\n", in_file);
		SHPClose(file);
		return false;
	}
	
	for(int f = 0; f < DBFGetFieldCount(db); ++f)
	{
		char fname[256];
		int width, dec;
		DBFGetFieldInfo(db, f, fname, &width, &dec);
		printf("%d: %s (%d/%d)\n", f, fname, width,dec);
	}

	int dsf_param = DBFGetFieldIndex(db,"param");
	int dsf_asset_name = DBFGetFieldIndex(db,"asset_name");

	if(dsf_param == -1 || dsf_asset_name == -1)
	{
		if(dsf_param == -1)
			fprintf(stderr,"Error: unable to find field 'param' for SHP DB.\n");
		if(dsf_asset_name == -1)
			fprintf(stderr,"Error: unable to find field 'asset_name' for SHP DB.\n");
		DBFClose(db);
		SHPClose(file);
		return false;
	}

		int		entity_count;
		int		shape_type;
		double	bounds_lo[4], bounds_hi[4];

	SHPGetInfo(file, &entity_count, &shape_type, bounds_lo, bounds_hi);

	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")

	CGAL::Arr_walk_along_line_point_location<Arrangement_2>	locator(io_map);

	bool is_pts = shape_type == SHPT_POINT || shape_type == SHPT_POINTZ || shape_type == SHPT_POINTM;
	printf("Shape point is %d, contians %d entities.\n", shape_type, entity_count);

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);

		if(is_pts)
		{
			GISObjPlacement_t no;
			no.mRepType = LookupTokenCreate(DBFReadStringAttribute(db,obj->nShapeId,dsf_asset_name));
			no.mHeading = DBFReadIntegerAttribute(db,obj->nShapeId,dsf_param);

			no.mLocation = Point2(obj->padfX[0],obj->padfY[0]);
			Assert(no.mLocation.x() >= lim_west );
			Assert(no.mLocation.x() <= lim_east );
			Assert(no.mLocation.y() >= lim_south);
			Assert(no.mLocation.y() <= lim_north);

			SHPDestroyObject(obj);

			CGAL::Object lobj = locator.locate(ben2cgal<Pmwx::Point_2>(no.mLocation));
			Face_const_handle ff;
			if(CGAL::assign(ff,lobj))
			{
				Face_handle f = io_map.non_const_handle(ff);
				f->data().mObjs.push_back(no);
#if SHOW_FEATURE_IMPORT
				debug_mesh_point(no.mLocation,0,1,0);
#endif

			}
			else
			{
#if SHOW_FEATURE_IMPORT
				debug_mesh_point(no.mLocation,1,0,0);
#endif
				fprintf(stderr,"WARNING: point %d could not be placed.\n", n);
			}
		}
		else
		{
			GISPolyObjPlacement_t np ;
			np.mRepType = LookupTokenCreate(DBFReadStringAttribute(db,obj->nShapeId,dsf_asset_name));
			np.mParam = DBFReadIntegerAttribute(db,obj->nShapeId,dsf_param);
			
			for (int part = 0; part < obj->nParts; ++part)
			{
				int start_idx = obj->panPartStart[part];
				int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
				Polygon2	p;
				for (int i = start_idx; i < stop_idx; ++i)
				{
					Point2 pt(obj->padfX[i],obj->padfY[i]);
					p.push_back(pt);
				}
				np.mShape.push_back(p);
			}

			SHPDestroyObject(obj);

			if(np.mShape.empty() || np.mShape[0].empty())
				continue;

			CGAL::Object lobj = locator.locate(ben2cgal<Pmwx::Point_2>(np.mShape[0][0]));
			Face_const_handle ff;
			if(CGAL::assign(ff,lobj))
			{
				Face_handle f = io_map.non_const_handle(ff);
				f->data().mPolyObjs.push_back(np);
	#if SHOW_FEATURE_IMPORT
				for(vector<Polygon2>::iterator p = np.mShape.begin(); p != np.mShape.end(); ++p)
				{
					for(int i = 1; i < p->size(); ++i)
						debug_mesh_line(p->at(i-1), p->at(i),0,0.5,0,0,1,0);
				}
	#endif

			}
			else
			{
	#if SHOW_FEATURE_IMPORT
				for(vector<Polygon2>::iterator p = np.mShape.begin(); p != np.mShape.end(); ++p)
				{
					for(int i = 1; i < p->size(); ++i)
						debug_mesh_line(p->at(i-1), p->at(i),0.5,0,0,1,0,0);
				}
	#endif
				fprintf(stderr,"WARNING: polygon %d could not be placed.\n", n);
			}
		}
	}

	PROGRESS_DONE(inFunc, 0, 1, "Reading shape file...")

	SHPClose(file);
	DBFClose(db);

	return true;
}

static int accum_ccb(
					Pmwx::Ccb_halfedge_circulator	circ,
					vector<double>&					x,
					vector<double>&					y)
{
	int ret = x.size();
	Pmwx::Ccb_halfedge_circulator stop = circ;
	do {

		Point2	p = cgal2ben(circ->source()->point());
		
		x.push_back(p.x());
		y.push_back(p.y());
		
	} while(--circ != stop);
	
	x.push_back(x[ret]);
	y.push_back(y[ret]);
	return ret;
}

bool	WriteShapefile(
			const char *			in_file,
			Pmwx&					in_map,
			int						terrain_type,
			bool					write_terrain_types,
			ProgressFunc			inFunc)
{
	SHPHandle file = SHPCreate(in_file, SHPT_POLYGON);
	if(!file)
		return false;
	DBFHandle dfile = write_terrain_types ? DBFCreate(in_file) : nullptr;
	if(write_terrain_types && !dfile)
	{
		SHPClose(file);
		return false;
	}
	
	int field_id = -1;
	
	if(dfile)
	{
      field_id = DBFAddField(dfile, "terrain_type", FTString, 64, 0);
	}

	PROGRESS_START(inFunc, 0, 1, "Writing shape file...")

	
	int entity_count = in_map.number_of_faces();
	int step = entity_count ? (entity_count / 150) : 2;
	int n = 0;
	int sid = 0;

	for(Pmwx::Face_handle f = in_map.faces_begin(); f != in_map.faces_end(); ++f, ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Writing shape file...", n, entity_count, step)

		if(!f->is_unbounded())
		if((terrain_type == -1 && f->data().mTerrainType != NO_VALUE) || f->data().mTerrainType == terrain_type)
		{
			vector<double>	v_x, v_y;
			vector<int>		offsets;

			int s = accum_ccb(f->outer_ccb(), v_x,v_y);
			offsets.push_back(s);
			
			for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
			{
				int s = accum_ccb(*h, v_x,v_y);
				offsets.push_back(s);
			}		
		
			SHPObject * sobj = 
				 SHPCreateObject( 
						SHPT_POLYGON, sid++, 
						offsets.size(),
						&offsets[0],
						NULL,
						v_x.size(), &v_x[0], &v_y[0], NULL, NULL);

			int shape_idx = SHPWriteObject(file,-1,sobj);
			SHPDestroyObject(sobj);
			
			if(dfile)
				DBFWriteStringAttribute(dfile, shape_idx, field_id, FetchTokenString(f->data().mTerrainType));
		}
	}
	
	PROGRESS_DONE(inFunc, 0, 1, "Writing shape file...")
	

	SHPClose(file);
	if(dfile)
	DBFClose(dfile);
	return true;
}
