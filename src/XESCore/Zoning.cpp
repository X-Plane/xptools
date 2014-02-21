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

#include "MapDefs.h"
#include "Zoning.h"
#include "MapAlgs.h"
#include "DEMDefs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "ParamDefs.h"
#include "MapAlgs.h"
#include "NetTables.h"
#include "NetHelpers.h"
#include "PolyRasterUtils.h"
#include "AptDefs.h"
#include "ObjPlacement2.h"
#include "ConfigSystem.h"
#include "MapTopology.h"
#include "GISTool_Globals.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "GISUtils.h"
#include "CompGeomUtils.h"
#include "BlockFill.h"
#include "BlockAlgs.h"
#include "MathUtils.h"

// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

// This is how big a block can be and still "grow" sky scrapers from one big one, in square meters.  The idea is to keep
// a single tall building in a block from turning into a sea of tall buildings.
#define MAX_OBJ_SPREAD 100000

ZoningRuleTable				gZoningRules;
ZoningInfoTable				gZoningInfo;
EdgeRuleTable				gEdgeRules;
FacadeSpellingTable			gFacadeSpellings;
FillRuleTable				gFillRules;
PointRuleTable				gPointRules;
LandClassInfoTable			gLandClassInfo;
LandFillRuleTable			gLandFillRules;

inline int RegisterAGResource(const string& r)
{
	if (r == "NO_VALUE") return NO_VALUE;
	string res = "lib/g10/autogen/" + r;
	return LookupTokenCreate(res.c_str());
}

static bool ReadLandClassRule(const vector<string>& tokens, void * ref)
{
	LandClassInfo_t info;
	int lc;
	if(TokenizeLine(tokens," eeff", &lc, &info.category, &info.urban_density, &info.veg_density) != 5) return false;

	if(gLandClassInfo.count(lc))
	{
		printf("ERROR: land class %s already in table.\n", FetchTokenString(lc));
		return false;
	}
	gLandClassInfo[lc] = info;
	return true;
}

static bool ReadEdgeRule(const vector<string>& tokens, void * ref)
{
	EdgeRule_t e;
	set<int>	zoning_list;
	set<int>	road_list;
	string res_id;
	if(TokenizeLine(tokens," SiffSsf",&zoning_list, &e.variant, &e.height_min, &e.height_max, &road_list, &res_id, &e.width) != 8) return false;
	e.resource_id = RegisterAGResource(res_id);
	for(set<int>::iterator z = zoning_list.begin(); z != zoning_list.end(); ++z)
	for(set<int>::iterator r = road_list.begin(); r != road_list.end(); ++r)
	{
		e.road_type = *r;
		e.zoning = *z;
		if(gZoningInfo.count(e.zoning) == 0)
			printf("WARNING: zoning type %s, required in edge rule for %s is unknown.\n",
				FetchTokenString(e.zoning),FetchTokenString(e.resource_id));
		gEdgeRules.push_back(e);
	}
	return true;
}

static bool ReadPointFillRule(const vector<string>& tokens, void * ref)
{
	PointRule_t r;
	string fac_rd, fac_ant, fac_free;
	if(TokenizeLine(tokens," eeffffffsffffsffffs",
			&r.zoning,&r.feature,&r.height_min,&r.height_max,
			&r.width_rd,&r.depth_rd,&r.x_width_rd, &r.x_depth_rd,&fac_rd,
			&r.width_ant,&r.depth_ant,&r.x_width_ant, &r.x_depth_ant,&fac_ant,
			&r.width_free,&r.depth_free,&r.x_width_free, &r.x_depth_free,&fac_free
		) != 11+4+5)
		return false;

	r.fac_id_ant = RegisterAGResource(fac_ant);
	r.fac_id_rd = RegisterAGResource(fac_rd);
	r.fac_id_free = RegisterAGResource(fac_free);

	gPointRules.push_back(r);
	return true;
}

static bool ReadFacadeRule(const vector<string>& tokens, void * ref)
{
	if(tokens[0] == "FACADE_SPELLING")
	{
		FacadeSpelling_t spelling;
		if(TokenizeLine(tokens," eiffff",
				&spelling.zoning, &spelling.variant,
				&spelling.height_min, &spelling.height_max,
				&spelling.depth_min, &spelling.depth_max) != 7)
			return false;
		gFacadeSpellings.push_back(spelling);
		return true;
	}

	else if(tokens[0] == "FACADE_TILE")
	{
		string fac_front, fac_back;
		FacadeChoice_t c;
		if(TokenizeLine(tokens," ffffss",&c.width, &c.height_min, &c.height_max, &c.depth_unused, &fac_front, &fac_back) != 7)
			return false;
		c.fac_id_front =  RegisterAGResource(fac_front);
		c.fac_id_back =  RegisterAGResource(fac_back);
		gFacadeSpellings.back().facs.push_back(c);
		return true;
	}
	return false;
}

/*
static void		pick_n(vector<float>& choices, int count, vector<float>& picks)
{
	int r = choices.size();
	while(count--)
	{
		int choice = rand() % r;
		picks.push_back(choices[choice]);
		--r;
		if(choice != r)
			swap(choices[r],choices[choice]);
	}
}
*/
static bool ReadFillRule(const vector<string>& tokens, void * ref)
{
	FillRule_t r;
	string agb, fac, ags, fil;
	if(TokenizeLine(tokens, " eii"
							"fffff"
							"ffffff"
							"fffif"
							"ssss",
			&r.zoning,			&r.road,			&r.variant,
			&r.min_height,&r.max_height,			&r.min_side_len, &r.max_side_len,			&r.block_err_max,
			&r.min_side_major,&r.max_side_major,	&r.min_side_minor,&r.max_side_minor,		&r.ang_min,&r.ang_max,

			&r.agb_min_width,			&r.agb_slop_width,			&r.fac_min_width_unused,	&r.fac_depth_split,			&r.fac_extra,

			&agb,			&fac,			&ags, &fil) != 24)
			return false;

	r.agb_slop_depth = r.agb_slop_width;

	

	r.agb_id = RegisterAGResource(agb);
	r.ags_id = RegisterAGResource(ags);
	r.fac_id = RegisterAGResource(fac);
	r.fil_id = RegisterAGResource(fil);

//	if((r.fac_min_width == 0.0 && r.fac_id != NO_VALUE && r.agb_id != NO_VALUE)
//	{
//		printf("ERROR: FAC %s has no subdiv width but is paired with AGB %s.\n", FetchTokenString(r.fac_id), FetchTokenString(r.agb_id));
//		return false;
//	}
//
//	if(r.fac_extra == 0.0 && r.fac_id != NO_VALUE && r.agb_id != NO_VALUE)
//	{
//		printf("ERROR: FAC %s has no extra width but is paired with AGB %s.\n", FetchTokenString(r.fac_id), FetchTokenString(r.agb_id));
//		return false;
//	}

	if(r.agb_slop_depth == 0.0 && r.agb_id != NO_VALUE)
	{
		printf("ERROR: AGB %s has 0 slop depth.\n", FetchTokenString(r.agb_id));
		return false;
	}

	if(gZoningInfo.count(r.zoning) == 0)
		printf("WARNING: zoning type %s, required in fill rule for %s/%s/%s is unknown.\n",
			FetchTokenString(r.zoning),
			FetchTokenString(r.agb_id),
			FetchTokenString(r.fac_id),
			FetchTokenString(r.ags_id));

	if(r.agb_id != NO_VALUE && strstr(FetchTokenString(r.agb_id),".agb") == NULL)
				printf("WARNING: zoning type %s with AGB %s - illegal agb.\n", FetchTokenString(r.zoning),FetchTokenString(r.agb_id));
	if(r.fac_id != NO_VALUE && strstr(FetchTokenString(r.fac_id),".fac") == NULL)
				printf("WARNING: zoning type %s with FAC %s - illegal fac.\n", FetchTokenString(r.zoning),FetchTokenString(r.fac_id));
	if(r.ags_id != NO_VALUE && strstr(FetchTokenString(r.ags_id),".ags") == NULL)
				printf("WARNING: zoning type %s with AGS %s - illegal ags.\n", FetchTokenString(r.zoning),FetchTokenString(r.ags_id));
	gFillRules.push_back(r);
	return true;
}

static bool ReadZoningInfo(const vector<string>& tokens, void * ref)
{
	ZoningInfo_t	info;
	int				zoning;
	if(TokenizeLine(tokens," efiiiiie", &zoning,&info.max_slope,&info.need_lu,&info.fill_edge,&info.fill_area,&info.fill_points, &info.fill_veg,&info.terrain_type) != 9)
		return false;
	// optimization: if slope will never filter out AND we don't need LU, set slope to 0 to turn the mesh check off entirely!
	if(info.max_slope == 90.0 && info.need_lu == 0)
		info.max_slope = 0.0;
	if(gZoningInfo.count(zoning) != 0)
	{
		printf("WARNING: duplicate zoning info for %s\n", FetchTokenString(zoning));
		return false;
	}
	gZoningInfo[zoning] = info;
	return true;
}

static bool	ReadZoningRule(const vector<string>& tokens, void * ref)
{
	ZoningRule_t	r;
	if(TokenizeLine(tokens," effffffffffffffiiefefiiiiiifffffffSSe",
		&r.terrain,
		&r.size_min,			&r.size_max,
		&r.slope_min,			&r.slope_max,
		&r.urban_avg_min,		&r.urban_avg_max,
		&r.forest_avg_min,		&r.forest_avg_max,
		&r.park_avg_min,		&r.park_avg_max,
		&r.bldg_min,			&r.bldg_max,
		&r.ang_min,				&r.ang_max,
		&r.sides_min, &r.sides_max,

		&r.req_cat1, &r.req_cat1_min,
		&r.req_cat2, &r.req_cat2_min,

		&r.req_water,
		&r.req_train,
		&r.req_road,
		&r.hole_ok,
		&r.crud_ok,
		&r.want_prim,
		&r.block_err_max,
		&r.min_side_len,
		&r.max_side_len,
		&r.min_side_major,
		&r.max_side_major,
		&r.min_side_minor,
		&r.max_side_minor,
		&r.require_features,
		&r.consume_features,
		&r.zoning) != 26+7+5)	return false;

	r.slope_min = 1.0 - cos(r.slope_min * DEG_TO_RAD);
	r.slope_max = 1.0 - cos(r.slope_max * DEG_TO_RAD);


	if(gZoningInfo.count(r.zoning) < 1 && r.zoning != NO_VALUE)
		printf("WARNING: zoning rule output %s is unknown zoning type.\n", FetchTokenString(r.zoning));
	gZoningRules.push_back(r);

	return true;
}

static bool ReadLandFillRule(const vector<string>& tokens, void * ref)
{
	LandFillRule_t r;
	if(TokenizeLine(tokens," Sie",&r.required_zoning,&r.color, &r.terrain) != 4)
		return false;

	gLandFillRules.push_back(r);

	return true;
}

void LoadZoningRules(void)
{
	gLandClassInfo.clear();
	gZoningRules.clear();
	gZoningInfo.clear();
	gEdgeRules.clear();
	gFacadeSpellings.clear();
	gFillRules.clear();
	gPointRules.clear();
	gLandFillRules.clear();

	RegisterLineHandler("LANDCLASS_INFO", ReadLandClassRule, NULL);
	RegisterLineHandler("ZONING_RULE", ReadZoningRule, NULL);
	RegisterLineHandler("ZONING_INFO", ReadZoningInfo, NULL);
	RegisterLineHandler("EDGE_RULE", ReadEdgeRule, NULL);
	RegisterLineHandler("FILL_RULE", ReadFillRule, NULL);
	RegisterLineHandler("LANDFILL_RULE", ReadLandFillRule, NULL);
	RegisterLineHandler("POINT_FILL", ReadPointFillRule, NULL);
	RegisterLineHandler("FACADE_TILE", ReadFacadeRule, NULL);
	RegisterLineHandler("FACADE_SPELLING", ReadFacadeRule, NULL);

	LoadConfigFile("zoning.txt");

	for(FacadeSpellingTable::iterator sp = gFacadeSpellings.begin(); sp != gFacadeSpellings.end(); ++sp)
	{
		sp->width_real = 0.0;
		for(vector<FacadeChoice_t>::iterator fc = sp->facs.begin(); fc != sp->facs.end(); ++fc)
		{
			sp->width_real += fc->width;
		}
		sp->width_min = sp->width_real - 10.0;
		sp->width_max = sp->width_real + 20.0;
	}
}

template <typename T>
inline bool	check_rule(T minv, T maxv, T actv)
{
	return ((minv == maxv) && (minv == 0) || (actv >= minv && actv <= maxv));
}

inline bool any_match(const set<int>& lhs, const set<int>& rhs)
{
	set<int>::const_iterator l = lhs.begin(), r = rhs.begin();

	while(l != lhs.end() && r != rhs.end())	// While we're not off the end of either
	{
		if(*l < *r)							// Advance the lesser
			++l;
		else if (*l > *r)
			++r;
		else
			return true;					// Any match = a win
	}
	return false;							// if one is EOF we're not going to get a match.
}

inline bool all_match(const set<int>& lhs, const set<int>& rhs)
{
	if(lhs.size() != rhs.size()) return false;

	set<int>::const_iterator l = lhs.begin(), r = rhs.begin();

	while(l != lhs.end() && r != rhs.end())
	{
		if(*l < *r) return false;			// Mismatch?  Bail.
		if(*l > *r) return false;
		++l, ++r;							// match, continue.
	}
	return l == lhs.end() && r == rhs.end();// We match if we ran out of BOTH at the same time.
}

inline void remove_these(set<int>& stuff, const set<int>& nuke_these)
{
	for(set<int>::const_iterator i = nuke_these.begin(); i != nuke_these.end(); ++i)
		stuff.erase(*i);
}

static int		PickZoningRule(
						int			terrain,
						float		area,
						int			num_sides,
						float		max_slope,
						float		urban_avg,
						float		forest_avg,
						float		park_avg,
						float		bldg_hgt,
						float		min_ang,
						float		max_ang,
						int			cat1,
						float		rat1,
						int			cat2,
						float		rat2,
						int			has_water,			// 0 = none, 1 = some, 2 = all
						int			has_train,			// 0 = none, 1 = some, 2 = all
						int			has_road,			// 0 = none, 1 = some, 2 = all
						int			has_hole,			// 0 = no, 1 = yes
						int			has_prim,			// ...

						float		block_err,			// Worst distance in meters of any point from the "frame" of the supporting grid block structure.  Low for very REGULAR blocks.
						float		short_side,			// Length in meters of the shortest side.
						float		long_side,			// Length  in meters of the longest side.
						float		major_length,		// Length along the "long" axis of the block
						float		minor_length,		// Length along the "short" axis of the block.
						set<int>&	features)
{
	for(ZoningRuleTable::const_iterator r = gZoningRules.begin(); r != gZoningRules.end(); ++r)
	{
		if(r->terrain == NO_VALUE || r->terrain == terrain)
		if(0 == r->sides_max || (r->sides_min <= num_sides && num_sides <= r->sides_max))
		if(check_rule(r->size_min, r->size_max, area))
		if(check_rule(r->slope_min, r->slope_max, max_slope))
		if(check_rule(r->urban_avg_min, r->urban_avg_max, urban_avg))
		if(check_rule(r->forest_avg_min, r->forest_avg_max, forest_avg))
		if(check_rule(r->park_avg_min, r->park_avg_max, park_avg))
		if(check_rule(r->bldg_min, r->bldg_max, bldg_hgt))
		if(r->ang_min == r->ang_max || (r->ang_min < min_ang && max_ang < r->ang_max))
		if(r->req_cat1 == NO_VALUE || (r->req_cat1 == cat1 && r->req_cat1_min <= rat1))
		if(r->req_cat2 == NO_VALUE || cat2 == NO_VALUE || (r->req_cat2 == cat2 && r->req_cat2_min <= rat2))
		if(has_water >= r->req_water)
		if(has_train >= r->req_train)
		if(has_road >= r->req_road)
		if(!has_hole || r->hole_ok)
		if(!r->want_prim || has_prim)
		if(block_err <= r->block_err_max || r->block_err_max == 0.0)
		if(check_rule(r->min_side_major,r->max_side_major,major_length))
		if(check_rule(r->min_side_minor,r->max_side_minor,minor_length))
		if(r->min_side_len  == r->max_side_len || (r->min_side_len <= short_side && long_side <= r->max_side_len))
		if(r->require_features.empty() || any_match(r->require_features, features))
		if(r->crud_ok || is_subset(features, r->consume_features))
		{
			remove_these(features, r->consume_features);
			return r->zoning;
		}
	}
	return NO_VALUE;
}



//----------------------------------------------------------------------------------------------------------------------------------------

struct zone_borders_t {
	set<double>		lock_pts[4];
};

int mark_is_locked(Vertex_handle v, NT coord, const set<double>& locked_pts)
{
	if(locked_pts.empty())
	{
		v->data().mNeighborBurned = false;
		v->data().mNeighborNotBurned = true;
		return 0;
	}

	double coordf = CGAL::to_double(coord);

	set<double>::const_iterator k = locked_pts.lower_bound(coordf);
	double k1, k2;
	if(locked_pts.size() == 1)
		k1 = k2 = *k;
	else if (k == locked_pts.begin())
		k1 = k2 = *k;
	else if (k == locked_pts.end())
	{
		--k;
		k1 = k2 = *k;
	} else
	{
		k2 = *k;
		--k;
		k1 = *k;
	}

	const double epsi = 1.0 / 10000000.0;
	if(fabs(coordf - k1) < epsi || fabs(coordf - k2) < epsi)
	{
		v->data().mNeighborBurned = true;
		v->data().mNeighborNotBurned = false;
		return 1;
	}
	else
	{
		v->data().mNeighborBurned = false;
		v->data().mNeighborNotBurned = true;
		return 0;
	}
}

static bool load_zone_border(const char * fname, zone_borders_t& border)
{
	for(int n = 0; n < 4; ++n)
		border.lock_pts[n].clear();
	FILE * fi = fopen(fname,"r");
	if(fi)
	{
		char buf[256];
		while(fgets(buf,sizeof(buf), fi))
		{
			double l;
			if(sscanf(buf,"LOCK_WEST %lf",&l)==1)
				border.lock_pts[0].insert(l);
			if(sscanf(buf,"LOCK_SOUTH %lf",&l)==1)
				border.lock_pts[1].insert(l);
			if(sscanf(buf,"LOCK_EAST %lf",&l)==1)
				border.lock_pts[2].insert(l);
			if(sscanf(buf,"LOCK_NORTH %lf",&l)==1)
				border.lock_pts[3].insert(l);
		}
		fclose(fi);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------


typedef int(* Feature_Is_f)(int);

template<Feature_Is_f evaluator>
int evaluate_he(Pmwx::Halfedge_handle he)
{
	GISNetworkSegmentVector::iterator i;
	for(i = he->data().mSegments.begin(); i != he->data().mSegments.end(); ++i)
		if(evaluator(i->mFeatType))
			return 1;
	for(i = he->twin()->data().mSegments.begin(); i != he->twin()->data().mSegments.end(); ++i)
		if(evaluator(i->mFeatType))
			return 1;
	return 0;
}


struct is_same_terrain_p {
	int terrain_;
	is_same_terrain_p(int terrain) : terrain_(terrain) { }
	bool operator()(Face_handle f) const { return !f->is_unbounded() && f->data().mTerrainType == terrain_; }
};



void	ColorFaces(set<Face_handle>&	io_faces);

void kill_antennas(Pmwx& io_map, Pmwx::Face_handle f, float max_len)
{
	Pmwx::Ccb_halfedge_circulator circ, stop, x,y, ae;
	circ = stop = f->outer_ccb();
	do {
		x = circ;
		--x;
		if(x->twin() == x->prev())
		if(he_has_any_roads(x))
		if(get_he_is_on_ground(x))
		if(get_he_street(x))
		{
			ae = x;
			while(ae->twin()->face() == f)
				++ae;
			double len = 0;
			y = x;
			while(y != ae)
			{
				Segment2 me(cgal2ben(y->source()->point()),cgal2ben(y->target()->point()));
				len += LonLatDistMeters(me.p1.x(),me.p1.y(),me.p2.x(),me.p2.y());
				++y;
			}

			if (len < max_len)
			{
				if(x == stop)
					++stop;
				else if(x->prev() == stop)
				{
					++stop; ++stop;
				}
//				debug_mesh_line(cgal2ben(x->source()->point()), cgal2ben(x->target()->point()), 1,0,0,  1,0,0);
				DebugAssert(x != stop);
				DebugAssert(x->twin() != stop);
				io_map.remove_edge(x);
			}
		}
	} while (++circ != stop);
}

static void ZoneOneFace(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
				const DEMGeo&		inPark,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				const DEMGeo&		urban_density_from_lu,
				Pmwx::Face_handle	face)
{
	//--------------------------------------------------------------------------------------------------------------------------------
	// BASIC BLOCK INFO - AREA, RASTER FEATURES
	//--------------------------------------------------------------------------------------------------------------------------------

	double mfam = GetMapFaceAreaMeters(face);
	double	max_height = 0.0;
	set<int>	my_pt_features;

	if (mfam < MAX_OBJ_SPREAD)
	for (GISPointFeatureVector::iterator feat = face->data().mPointFeatures.begin(); feat != face->data().mPointFeatures.end(); ++feat)
	{
		my_pt_features.insert(feat->mFeatType);
		if (feat->mFeatType == feat_Building)
		{
			if (feat->mParams.count(pf_Height))
			{
				max_height = max(max_height, feat->mParams[pf_Height]);
			}
		} else {
			printf("Has other feature: %s\n", FetchTokenString(feat->mFeatType));
		}
	}

	int has_water = 0;
	int has_non_water = 0;
	int has_train = 0;
	int has_prim = 0;
	int	has_non_train = 0;
	int has_local = 0;
	int has_non_local = 0;
	Bbox2 face_extent;

	PolyRasterizer<double>	r;
	int x, y, x1, x2;
	y = SetupRasterizerForDEM(face, inLanduse, r);
	r.StartScanline(y);
	float count = 0, total_forest = 0, total_urban = 0, total_park = 0;
	map<int, int>		histo;

	while (!r.DoneScan())
	{
		while (r.GetRange(x1, x2))
		{
			for (x = x1; x < x2; ++x)
			{
				float e = inLanduse.get(x,y);
				float f = inForest.get(x,y);
//				float p = inPark.get(x,y);
				float p = inPark.get(
					inPark.map_x_from(inLanduse, x),
					inPark.map_y_from(inLanduse,y));
				float d = urban_density_from_lu.get(x,y);
				count++;

				total_urban += d;

				if(gLandClassInfo.count(e))
				{
					LandClassInfo_t& i(gLandClassInfo[e]);
					histo[i.category]++;
					total_forest += i.veg_density;

					// THIS IS INTENTIONAL!  We are ONLY accepting park raster points that CONVERT
					// urban to park.  The reason: a giant state forest is tagged "forest park" in OSM
					// but does NOT get special zoning treatment.
					if(p != NO_VALUE)
						total_park += 1.0;
				}
				else
				{
					histo[terrain_Natural]++;
					if(f != NO_VALUE)
						total_forest += 1.0;
				}

			}
		}
		++y;
		if (y >= inLanduse.mHeight) break;
		r.AdvanceScanline(y);
	}

	if(count == 0)
	{
		Point2 any = cgal2ben(face->outer_ccb()->source()->point());
		float e = inLanduse.xy_nearest(any.x(),any.y());
		float f = inForest.xy_nearest(any.x(),any.y());
		float p = inPark.xy_nearest(any.x(),any.y());
		float d = urban_density_from_lu.value_linear(any.x(), any.y());

		count++;

		total_urban += d;
		if(gLandClassInfo.count(e))
		{
			LandClassInfo_t& i(gLandClassInfo[e]);
			histo[i.category]++;
			total_forest += i.veg_density;
			if(p != NO_VALUE)
				total_park += 1.0;
		}
		else
		{
			histo[terrain_Natural]++;
			if(f != NO_VALUE)
				total_forest += 1.0;
		}

	}

	if(count)
	if((total_urban / count) > 0.5)
	if(face->number_of_holes() == 0)
		kill_antennas(ioMap,face,  ((total_urban / count) > 0.75) ? 35.0 : 20.0);

	multimap<int, int, greater<int> > histo2;
	for(map<int,int>::iterator i = histo.begin(); i != histo.end(); ++i)
		histo2.insert(multimap<int,int, greater<int> >::value_type(i->second,i->first));

	multimap<int, int, greater<int> >::iterator i = histo2.begin();
	if(histo2.size() > 0)
	{
		face->data().mParams[af_Cat1] = i->second;
		face->data().mParams[af_Cat1Rat] = (float) i->first / (float) count;
		++i;

		if(histo2.size() > 1)
		{
			face->data().mParams[af_Cat2] = i->second;
			face->data().mParams[af_Cat2Rat] = (float) i->first / (float) count;
			++i;

			if(histo2.size() > 2)
			{
				face->data().mParams[af_Cat3] = i->second;
				face->data().mParams[af_Cat3Rat] = (float) i->first / (float) count;
				++i;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------
	// ROAD ANALYSIS
	//--------------------------------------------------------------------------------------------------------------------------------

	Pmwx::Ccb_halfedge_circulator circ, stop;
	circ = stop = face->outer_ccb();
	do {
		face_extent += cgal2ben(circ->source()->point());
		if(evaluate_he<Road_IsTrain>(circ))
			has_train = 1;
		else
			has_non_train = 1;
		if((evaluate_he<Road_IsLocal>(circ) || evaluate_he<Road_IsMainDrag>(circ)) &&
			(circ->data().HasGroundRoads() || circ->twin()->data().HasGroundRoads()))
		{
			has_local = 1;
			int ft = get_he_feat_type(circ);
			if(ft == road_Primary || ft == road_PrimaryOneway || ft == road_Secondary || ft == road_SecondaryOneway)
				has_prim = 1;
		} else
			has_non_local = 1;
		if(!circ->twin()->face()->is_unbounded() && circ->twin()->face()->data().IsWater())
			has_water = 1;
		else
			has_non_water;
	} while (++circ != stop);

	if(has_water && !has_non_water) has_water = 2;
	if(has_train && !has_non_train) has_train = 2;
	if(has_local && !has_non_local)	has_local = 2;


	PolyRasterizer<double>  r2;
	y = SetupRasterizerForDEM(face, inSlope, r2);
	r2.StartScanline(y);
	int scount = 0;
	float max_slope = 0.0;
	while (!r2.DoneScan())
	{
		while (r2.GetRange(x1, x2))
		{
			for (x = x1; x < x2; ++x)
			{
				float s = inSlope.get(x,y);
				max_slope = max(max_slope, s);
				++scount;
			}
		}
		++y;
		if (y >= inLanduse.mHeight) break;
		r2.AdvanceScanline(y);
	}

	if(scount == 0)
	{
		Point2 any = cgal2ben(face->outer_ccb()->source()->point());
		float s = inSlope.xy_nearest(any.x(),any.y());
		max_slope = s;
	}

	//--------------------------------------------------------------------------------------------------------------------------------
	// DETAILED EXAMINATION: VECTOR ANALYSIS IN METRIC SPACE
	//--------------------------------------------------------------------------------------------------------------------------------


	CoordTranslator2	trans;
	CreateTranslatorForBounds(face_extent,trans);
	vector<block_pt>		outer_border;
	if(!block_pts_from_ccb(face->outer_ccb(), trans, outer_border, 1.0,false))
	{
		return;
	}

	float len_train = 0.0, len_local = 0.0, len_total = 0.0;
	for(int i = 0; i < outer_border.size(); ++i)
	{
		int j = (i + 1) % outer_border.size();
		float len = sqrt(Segment2(outer_border[i].loc,outer_border[j].loc).squared_length());
		len_total += len;
		if(ground_road_access_for_he(outer_border[i].orig))
			len_local += len;
		if(ground_train_access_for_he(outer_border[i].orig))
			len_train += len;
	}

	int num_sides = 0;
	float min_angle = 0.0;
	float max_angle = 0.0;
	bool has_holes = face->number_of_holes() > 0;
	double max_err = 0.0;
	double short_side = 0.0;
	double long_side = 0.0;
	double short_axis_length = 0.0;
	double long_axis_length = 0.0;

	if(outer_border.size() < 30)
	{
		Segment2	major;
		double	bounds[4];
		Vector2	v_x,v_y;

		find_major_axis(outer_border, &major,&v_x,&v_y,bounds);

//		This shows the early major axis in red.
//		debug_mesh_line(trans.Reverse(major.p1 + v_y),trans.Reverse(major.p2 + v_y),1,0,0,1,0,0);

		short_side = sqrt(major.squared_length());
		long_side = short_side;

		for(vector<block_pt>::iterator p = outer_border.begin(); p != outer_border.end(); ++p)
		{
			vector<block_pt>::iterator n(p);
			++n;
			if(n == outer_border.end()) n = outer_border.begin();
			double slen = sqrt(p->loc.squared_distance(n->loc));
			short_side = min(short_side,slen);
			long_side = max(long_side,slen);

			double cx = v_x.dot(Vector2(p->loc));
			double cy = v_y.dot(Vector2(p->loc));
			double x_err = min(fabs(cx-bounds[0]),fabs(cx-bounds[2]));
			double y_err = min(fabs(cy-bounds[1]),fabs(cy-bounds[3]));
			max_err = max(max_err,min(x_err,y_err));
		}

		short_axis_length = fabs(bounds[3] - bounds[1]);
		long_axis_length = fabs(bounds[2] - bounds[0]);

		min_angle = 180.0;
		max_angle = -180.0;

		for(int i = 0; i < outer_border.size(); ++i)
		{
			int j = (i + 1) % outer_border.size();
			int k = (i + 2) % outer_border.size();
			Vector2	v1(outer_border[i].loc,outer_border[j].loc);
			Vector2	v2(outer_border[j].loc,outer_border[k].loc);
			v1.normalize();
			v2.normalize();
			float ang = doblim(acos(doblim(v1.dot(v2),-1.0,1.0)) * RAD_TO_DEG,-180.0,180.0);
			if(v1.right_turn(v2))
				ang = -ang;
			if(outer_border[i].loc == outer_border[k].loc)
				ang = -180.0;

			// Nearly straight angle AND a road size change?  Assume we're going to get a "jaggy" and be a bit screwed.
			if(WidthForSegment(outer_border[i].edge_type) != WidthForSegment(outer_border[j].edge_type) &&
				(ang > -30 && ang < 30))
			{
				//debug_mesh_point(trans.Reverse(outer_border[j].loc),1,0,0);
				min_angle = min(ang-90.0f,min_angle);
				max_angle = max(ang+90.0f,max_angle);

				min_angle = max(min_angle,-180.0f);
				max_angle = min(max_angle, 180.0f);
			}

			min_angle = min(ang,min_angle);
			max_angle = max(ang,max_angle);

		}

		vector<pair<Pmwx::Halfedge_handle, Pmwx::Halfedge_handle> >				sides;
		Polygon2																mbounds;

		if(!has_holes)
		if(short_axis_length > 1.0 && long_axis_length > 1.0)
		if(build_convex_polygon(face->outer_ccb(),sides,trans,mbounds, 2.0, 2.0))
		{
			num_sides = mbounds.size();

			int major = pick_major_axis(sides,mbounds, v_x,v_y);
			if(major != -1)
			{
				// This shows the AG final major axis in green.
				Segment2 real_major = mbounds.side(major);
//				debug_mesh_line(trans.Reverse(real_major.p1/* + v_y*/),trans.Reverse(real_major.p2/* + v_y*/),0,1,0,0,1,0);
				
				bounds[0] = bounds[2] = v_x.dot(Vector2(mbounds[0]));
				bounds[1] = bounds[3] = v_y.dot(Vector2(mbounds[0]));	
				for(int n = 1; n < mbounds.size(); ++n)
				{
					double x = v_x.dot(Vector2(mbounds[n]));
					double y = v_y.dot(Vector2(mbounds[n]));
					bounds[0] = min(bounds[0],x);
					bounds[1] = min(bounds[1],y);
					bounds[2] = max(bounds[2],x);
					bounds[3] = max(bounds[3],y);
				}

				double worst_l[4] = { 0 };

				for(int n = 0; n < mbounds.size(); ++n)
				{
					double x = v_x.dot(Vector2(mbounds[n]));
					double y = v_y.dot(Vector2(mbounds[n]));

					double dif[4] = {
						x-bounds[0],
						bounds[2]-x,
						y-bounds[1],
						bounds[3]-y };
					
					for(int s = 0; s < 4; ++s)
					{																									
						DebugAssert(dif[s] >= 0.0f);
						if(dif[s] < 2.0f)
							worst_l[s] = max(worst_l[s],dif[s]);
					}
				}
				
				short_axis_length = fabs(bounds[3] - bounds[1]) - worst_l[0] - worst_l[2];
				long_axis_length = fabs(bounds[2] - bounds[0]) - worst_l[1] - worst_l[3];

			}

			min_angle = 180.0;
			max_angle = -180.0;

			for(int i = 0; i < mbounds.size(); ++i)
			{
				int j = (i + 1) % mbounds.size();
				int k = (i + 2) % mbounds.size();
				Vector2	v1(mbounds[i],mbounds[j]);
				Vector2	v2(mbounds[j],mbounds[k]);
				v1.normalize();
				v2.normalize();
				float ang = doblim(acos(doblim(v1.dot(v2),-1.0,1.0)) * RAD_TO_DEG,-180.0,180.0);
				min_angle = min(ang,min_angle);
				max_angle = max(ang,max_angle);

			}

			int old_local = has_local;
			has_local = has_non_local = 0;
			for(int i = 0; i < sides.size(); ++i)
			if(ground_road_access_for_he(sides[i].first))
			{
				Vector2	vside(mbounds.side(i).p1,mbounds.side(i).p2);
				vside.normalize();
				if(fabs(v_x.dot(vside)) > 0.996)
					has_local = 1;
			} else
				has_non_local = 1;
			if(has_local && !has_non_local) has_local = 2;

			if((has_local == 0) != (old_local == 0))
			{
				num_sides = 0;
				has_local = old_local;
			}

		}
	}

	face->data().mParams[af_AGSides] = num_sides;

	//--------------------------------------------------------------------------------------------------------------------------------
	// LET US MAKE A FREAKING DECISION!!!
	//--------------------------------------------------------------------------------------------------------------------------------

	int zone = PickZoningRule(
					face->data().mTerrainType,
					mfam,
					num_sides,
					max_slope,
					total_urban/(float)count,total_forest/(float)count,total_park/(float)count,
					max_height,
					min_angle,
					max_angle,
					face->data().mParams[af_Cat1],
					face->data().mParams[af_Cat1Rat],
					face->data().mParams[af_Cat2],
					face->data().mParams[af_Cat1Rat] + face->data().mParams[af_Cat2Rat],	// Really?  Yes.  This is the "high water mark" of BOTH cat 1 + cat 2.  That way
					has_water,																// We can say "80% industrial, 90% urban, and we cover 80I+10U and 90I+0U.  In other
					has_train,																// words when we can accept a mix, this lets the DOMINANT type crowd out the secondary.
					has_local,
					has_holes,
					has_prim,

					max_err,

					short_side,
					long_side,

					long_axis_length,
					short_axis_length,

					my_pt_features);

	if(zone != NO_VALUE)
	{
		face->data().SetZoning(zone);
		int wanted_terrain = gZoningInfo[zone].terrain_type;
		if(wanted_terrain != NO_VALUE)
			face->data().mTerrainType = wanted_terrain;
	}
	face->data().mParams[af_HeightObjs] = max_height;

	face->data().mParams[af_UrbanAverage] = total_urban / (float) count;
	face->data().mParams[af_ForestAverage] = total_forest / (float) count;
	face->data().mParams[af_ParkAverage] = total_park / (float) count;
	face->data().mParams[af_SlopeMax] = max_slope;
	face->data().mParams[af_AreaMeters] = mfam;


	face->data().mParams[af_ShortestSide]		= short_side;
	face->data().mParams[af_LongestSide]		= long_side;
	face->data().mParams[af_ShortAxisLength]	= short_axis_length;
	face->data().mParams[af_LongAxisLength]		= long_axis_length;
	face->data().mParams[af_BlockErr]			= max_err;

	face->data().mParams[af_MinAngle]			= min_angle;
	face->data().mParams[af_MaxAngle]			= max_angle;

	face->data().mParams[af_WaterEdge]	=	has_water;
	face->data().mParams[af_RoadEdge]	=	has_local;
	face->data().mParams[af_RailEdge]	=	has_train;
	face->data().mParams[af_PrimaryEdge]=	has_prim;

	face->data().mParams[af_LocalPercent] = len_local / len_total;
	face->data().mParams[af_RailPercent] = len_train / len_total;

	// FEATURE ASSIGNMENT - first go and assign any features we might have.
	face->data().mTemp1 = NO_VALUE;
	face->data().mTemp2 = 0;


	if(((len_local / len_total) < 0.1 && mfam < 10000.0) ||
		(short_axis_length > 0.0 && short_axis_length < 20.0) ||
		mfam < 900.0)
	{
		face->data().mParams[af_Median] = 2;
	}

}


void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
				const DEMGeo&		inPark,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				Pmwx::Face_handle	inDebug,
				ProgressFunc		inProg)
{
		Pmwx::Face_iterator face;

	PROGRESS_START(inProg, 0, 3, "Zoning terrain...")

	int total = ioMap.number_of_faces() * 2;
	int check = total / 100;
	int ctr = 0;

	/*****************************************************************************
	 * PRE-COMP - DERIVE SOME LANDUSE INFO
	 *****************************************************************************/
	DEMGeo	urban_density_from_lu(inLanduse);
	for(DEMGeo::iterator i = urban_density_from_lu.begin(); i != urban_density_from_lu.end(); ++i)
	{
		LandClassInfoTable::iterator lu = gLandClassInfo.find(*i);
		if(lu == gLandClassInfo.end())
			*i = 0;
		else
			*i = lu->second.urban_density;
	}

	GaussianBlurDEM(urban_density_from_lu, 1.0);

//	gDem[dem_Wizard] = urban_density_from_lu;

	/*****************************************************************************
	 * PASS 1 - ZONING ASSIGNMENT VIA LAD USE DATA + FEATURES
	 *****************************************************************************/
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if(!face->data().IsWater())
	if(inDebug == Pmwx::Face_handle() || face == inDebug)
	{
		PROGRESS_CHECK(inProg, 0, 3, "Zoning terrain...", ctr, total, check)
		ZoneOneFace(
					ioMap,
					inLanduse,
					inForest,
					inPark,
					inSlope,
					inApts,
					urban_density_from_lu,
		
			face);
	}

#define HEIGHT_SPREAD_FACTOR 0.5
#define MIN_HEIGHT_TO_SPREAD 16.0

	//--------------------------------------------------------------------------------------------------------------------------------
	// HEIGHT SPREAD
	//--------------------------------------------------------------------------------------------------------------------------------

		set<Pmwx::Face_handle>	to_visit, visited;

	for(Pmwx::Face_handle f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	if(!f->is_unbounded())
	if(f->data().GetZoning() != NO_VALUE)
	if(!f->data().IsWater())
	if(f->data().mParams.count(af_HeightObjs))
	if(f->data().GetParam(af_AreaMeters,0) < MAX_OBJ_SPREAD)
	{
		double h = f->data().mParams[af_HeightObjs] * HEIGHT_SPREAD_FACTOR;
		if(h > MIN_HEIGHT_TO_SPREAD)
			to_visit.insert(f);
	}

	while(!to_visit.empty())
	{
		Pmwx::Face_handle me = *to_visit.begin();
		to_visit.erase(to_visit.begin());

		double h = me->data().mParams[af_HeightObjs] * HEIGHT_SPREAD_FACTOR;
		if(h > MIN_HEIGHT_TO_SPREAD)
		{
			set<Pmwx::Face_handle>	neighbors;
			FindAdjacentFaces<Pmwx>(me, neighbors);
			for(set<Pmwx::Face_handle>::iterator n = neighbors.begin(); n != neighbors.end(); ++n)
			if(!(*n)->is_unbounded())
			if(!(*n)->data().IsWater())
			if((*n)->data().GetParam(af_AreaMeters,0) < MAX_OBJ_SPREAD)
			if((*n)->data().GetZoning() != NO_VALUE)
			{
				float my_height = (*n)->data().GetParam(af_HeightObjs,0.0);
				if(h > my_height)
				{
					(*n)->data().mParams[af_HeightObjs] = h;
					to_visit.insert(*n);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------
	// DEAD CODE - APPROACH PATHS
	//--------------------------------------------------------------------------------------------------------------------------------



	PROGRESS_DONE(inProg, 0, 3, "Zoning terrain...")
#if 0
	PROGRESS_START(inProg, 1, 3, "Checking approach paths...")

	ctr = 0;
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (face->data().mTerrainType != terrain_Airport)
	if (!face->data().IsWater())
	{
		PROGRESS_CHECK(inProg, 1, 3, "Checking approach paths...", ctr, total, check)
		set<Face_handle>	neighbors;
		//FindAdjacentFaces(face, neighbors);
		{
			neighbors.clear();
			set<Halfedge_handle> e;
			FindEdgesForFace(face, e);
			for (set<Halfedge_handle>::iterator he = e.begin(); he != e.end(); ++he)
				if ((*he)->twin()->face() != face)
					neighbors.insert((*he)->twin()->face());
		}
		Polygon_2 me;
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			me.push_back(circ->target()->point());
			++circ;
		} while (circ != stop);

		Point_2	myloc = centroid(me);

		double	my_agl = face->data().mParams[af_HeightObjs];
		double	max_agl = my_agl;

		for (set<Face_handle>::iterator niter = neighbors.begin(); niter != neighbors.end(); ++niter)
		{
			max_agl = max(max_agl, (*niter)->data().mParams[af_HeightObjs] * 0.5);
		}

		for (AptVector::const_iterator apt = inApts.begin(); apt != inApts.end(); ++apt)
		if (apt->kind_code == apt_airport)
		if (!apt->pavements.empty())
		{
			Point_2 midp = CGAL::midpoint(apt->pavements.front().ends.source(),apt->pavements.front().ends.target());
			double dist = LonLatDistMeters(midp.x(), midp.y(), myloc.x(), myloc.y());
			if (dist < 15000.0)
			for (AptPavementVector::const_iterator rwy = apt->pavements.begin(); rwy != apt->pavements.end(); ++rwy)
			if (rwy->name != "xxx")
			{
				midp = CGAL::midpoint(rwy->ends.source(), rwy->ends.target());
				dist = LonLatDistMeters(midp.x(), midp.y(), myloc.x(), myloc.y());

				Vector_2	azi_rwy = normalize(Vector_2(rwy->ends.source(), rwy->ends.target()));
				Vector_2 azi_me = normalize(Vector_2(midp, myloc));

				double dot = azi_rwy * azi_me;

				double gs_elev = dist / 18.0;
				if (dot > 0.8 && dist < 700.0)
					max_agl = min(max_agl, gs_elev);
			}
		}

		my_agl = max(my_agl, max_agl);
		face->data().mParams[af_Height] = max_agl;
	}
	PROGRESS_DONE(inProg, 1, 3, "Checking approach paths...")
#endif

	//--------------------------------------------------------------------------------------------------------------------------------
	// WATER ANALYSIS
	//--------------------------------------------------------------------------------------------------------------------------------


	PROGRESS_START(inProg, 2, 3, "Checking Water")
	ctr = 0;
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (face->data().IsWater())
	{
		bool is_open = false;
		PROGRESS_CHECK(inProg, 2, 3, "Checking Water", ctr, total, check)
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			if (circ->twin()->face()->is_unbounded())
			{
				is_open = true;
				break;
			}
			++circ;
		} while (circ != stop);

		face->data().mParams[af_WaterOpen] = is_open ? 1.0 : 0.0;
		face->data().mParams[af_WaterArea] = GetMapFaceAreaMeters(face);

	}
	PROGRESS_DONE(inProg, 2, 3, "Checking Water")

	//--------------------------------------------------------------------------------------------------------------------------------
	// ZONING DEBUG OUTPUT, HISTOGRAMS, ALL THAT GOOD STUFF.
	//--------------------------------------------------------------------------------------------------------------------------------

	NT west(inLanduse.mWest);
	NT east(inLanduse.mEast);
	NT north(inLanduse.mNorth);
	NT south(inLanduse.mSouth);


	{
		zone_borders_t	n_west, n_south, n_east, n_north;

		char path[1024];

		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest-1, inLanduse.mSouth,"zoning", path);
		bool has_west = load_zone_border(path, n_west);
		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest+1, inLanduse.mSouth,"zoning", path);
		bool has_east = load_zone_border(path, n_east);

		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth-1,"zoning", path);
		bool has_south = load_zone_border(path, n_south);
		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth+1,"zoning", path);
		bool has_north = load_zone_border(path, n_north);

		int marked = 0;

		for(Pmwx::Vertex_iterator v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++v)
		{
			Point_2 p(v->point());
			if(p.x() == west && has_west)
				marked += mark_is_locked(v, p.y(), n_west.lock_pts[2]);
			else if (p.x() == east && has_east)
				marked += mark_is_locked(v, p.y(), n_east.lock_pts[0]);
			else if (p.y() == south && has_south)
				marked += mark_is_locked(v, p.x(), n_south.lock_pts[3]);
			else if (p.y() == north && has_north)
				marked += mark_is_locked(v, p.x(), n_north.lock_pts[1]);
			else
				v->data().mNeighborBurned = v->data().mNeighborNotBurned = false;
		}

		printf("Marked: %d. Imported: %zd.\n", marked, n_west.lock_pts[2].size() + n_east.lock_pts[0].size() + n_south.lock_pts[3].size() + n_north.lock_pts[1].size());
	}


	for(face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face)
	if(!face->is_unbounded())
	if(!face->data().IsWater())
	if(!face->data().HasParam(af_Variant))
	{
		set<Face_handle>	the_blob;
		CollectionVisitor<Pmwx,Face_handle,is_same_terrain_p>	col(&the_blob, is_same_terrain_p(face->data().mTerrainType));
		VisitContiguousFaces<Pmwx,CollectionVisitor<Pmwx,Face_handle,is_same_terrain_p> >(face, col);
		ColorFaces(the_blob);
	}

	char zbpath[1024];
	make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth,"zoning", zbpath);

	FILE * bf = fopen(zbpath,"w");
	if(bf)
	{
		set<double>	edge_l, edge_r, edge_b, edge_t;

		Pmwx::Ccb_halfedge_circulator circ, stop;
		DebugAssert(ioMap.unbounded_face()->number_of_holes() == 1);
		circ = stop = *ioMap.unbounded_face()->holes_begin();//->outer_ccb();
		do
		{
			if(must_burn_v(circ->target()))
			{
				Point_2 p(circ->target()->point());
				if(p.x() == west)
					edge_l.insert(CGAL::to_double(p.y()));
				else if(p.x() == east)
					edge_r.insert(CGAL::to_double(p.y()));
				else if(p.y() == south)
					edge_b.insert(CGAL::to_double(p.x()));
				else if(p.y() == north)
					edge_t.insert(CGAL::to_double(p.x()));
				else
				{
					printf("Bad point: %lf,%lf\n",CGAL::to_double(p.x()),CGAL::to_double(p.y()));
					DebugAssert(!"Point is not on an edge.");
				}
//				debug_mesh_point(cgal2ben(circ->target()->point()),1,1,1);
			}
		} while(++circ != stop);

		set<double>::iterator i;
		for(i = edge_l.begin(); i != edge_l.end(); ++i)
			fprintf(bf,"LOCK_WEST %.12lf\n",*i);
		for(i = edge_b.begin(); i != edge_b.end(); ++i)
			fprintf(bf,"LOCK_SOUTH %.12lf\n",*i);
		for(i = edge_r.begin(); i != edge_r.end(); ++i)
			fprintf(bf,"LOCK_EAST %.12lf\n",*i);
		for(i = edge_t.begin(); i != edge_t.end(); ++i)
			fprintf(bf,"LOCK_NORTH %.12lf\n",*i);
		fclose(bf);
	}


	/************************
	 * GO BACK AND FIX ZONING
	 ************************/

	for(Pmwx::Edge_iterator i = ioMap.edges_begin(); i != ioMap.edges_end(); ++i)
	if(he_has_any_roads(i))
	{
		Pmwx::Halfedge_handle he =  get_he_with_roads(i);
		Pmwx::Face_handle fl = he->face();
		Pmwx::Face_handle fr = he->twin()->face();
		int zl = fl->is_unbounded() ? NO_VALUE : fl->data().GetZoning();
		int zr = fr->is_unbounded() ? NO_VALUE : fr->data().GetZoning();

		bool zone_left = gPromotedZoningSet.count(zl);
		bool zone_right = gPromotedZoningSet.count(zr);
		if(zone_left || zone_right)
		{
			int rt = get_he_rep_type(he);
			ZonePromoteTable::iterator p = gZonePromote.find(rt);
			if(p != gZonePromote.end())
			{
				if(zone_left && zone_right)
					set_he_rep_type(he,p->second.promote_both);
				else if (zone_left)
					set_he_rep_type(he,p->second.promote_left);
				else if (zone_right)
					set_he_rep_type(he,p->second.promote_right);
			}
		}
	}

}

/***
 *
 **/
#pragma mark -

inline int MIN_NOVALUE(int a, int b)
{
	if(a == NO_VALUE) return b;
	if(b == NO_VALUE) return a;
	return min(a,b);
}


struct EdgeNode_t;

typedef multimap<float, EdgeNode_t *, greater<float> >	EdgeQ;

struct	FaceNode_t {

	FaceNode_t *			prev;
	FaceNode_t *			next;

	list<Face_handle>		real_faces;
	list<EdgeNode_t *>		edges;
	float					area;
	Bbox2					bounds;
	set<int>				zoning;
	int						color;

};

struct EdgeNode_t {
	EdgeNode_t *	prev;
	EdgeNode_t *	next;

	FaceNode_t *	f1;
	FaceNode_t *	f2;
	float			length;		// Length of edge in meters
	int				count;		// Number of edges
	int				road_type;	// Lowest enum overlying road on this edge
	bool			must_lock;
	bool			must_merge;
	EdgeQ::iterator	self;

	FaceNode_t *	other(FaceNode_t * f) const { DebugAssert(f == f1 || f == f2); return (f == f1) ? f2 : f1; }
};

struct edge_contains_f {
	FaceNode_t * f_;
	edge_contains_f(FaceNode_t * f) : f_(f) { }
	bool operator()(const EdgeNode_t *& e) const { return e->f1 == f_ || e->f2 == f_; }
};

struct FaceGraph_t {

	FaceGraph_t() { edges = NULL; faces = NULL; }

	EdgeNode_t *	edges;
	FaceNode_t *	faces;
	EdgeQ			queue;

	~FaceGraph_t() { clear(); }
	void			clear();
	FaceNode_t *	new_face();
	EdgeNode_t *	new_edge();
	void			delete_face(FaceNode_t * f);
	void			delete_edge(EdgeNode_t * e);
	bool			connected(FaceNode_t * f1, FaceNode_t * f2);
	void			merge(FaceNode_t * f1, FaceNode_t * f2, list<EdgeNode_t *>& merged_neighbors);

	void			enqueue(EdgeNode_t * en,float (* cost_func)(EdgeNode_t * en));
	EdgeNode_t *	pop(void);

};

FaceNode_t *	FaceGraph_t::new_face()
{
	FaceNode_t * n = new FaceNode_t;
	n->prev = NULL;
	n->next = faces;
	faces = n;
	if(n->next)
		n->next->prev = n;
	return n;
}

EdgeNode_t *	FaceGraph_t::new_edge()
{
	EdgeNode_t * n = new EdgeNode_t;
	n->prev = NULL;
	n->next = edges;
	if(n->next)
		n->next->prev = n;
	edges = n;
	return n;
}


void			FaceGraph_t::delete_face(FaceNode_t * f)
{
	if(f == faces)
		faces = f->next;
	if(f->prev)	f->prev->next = f->next;
	if(f->next) f->next->prev = f->prev;
	delete f;
}

void			FaceGraph_t::delete_edge(EdgeNode_t * e)
{
	if(e->self != queue.end())
		queue.erase(e->self);

	if(e == edges)
		edges = e->next;
	if(e->prev)	e->prev->next = e->next;
	if(e->next) e->next->prev = e->prev;
	delete e;
}

bool			FaceGraph_t::connected(FaceNode_t * f1, FaceNode_t * f2)
{
	if(f1->edges.size() > f2->edges.size())
		swap(f1,f2);
	for(list<EdgeNode_t *>::iterator e = f1->edges.begin(); e != f1->edges.end(); ++e)
	if(((*e)->f1 == f1 && (*e)->f2 == f2) ||
	   ((*e)->f1 == f2 && (*e)->f2 == f1))
		return true;
	return false;
}

void			FaceGraph_t::merge(FaceNode_t * f1, FaceNode_t * f2, list<EdgeNode_t *>& out_neighbors)
{
	f1->area += f2->area;
	f1->real_faces.splice(f1->real_faces.end(),f2->real_faces);
	copy(f2->zoning.begin(),f2->zoning.end(),set_inserter(f1->zoning));

	f1->bounds += f2->bounds;

	map<FaceNode_t *, EdgeNode_t *>						neighbors;
	map<FaceNode_t *, EdgeNode_t *>::iterator			ni;
	list<EdgeNode_t *>::iterator ei;
	EdgeNode_t * en;
	FaceNode_t * fn;


	// First: grab up all edges that we may keep.  This is our neighbor set.
	// We have one edge to f2 which we will cope with later.
	for(list<EdgeNode_t *>::iterator ei = f1->edges.begin(); ei != f1->edges.end(); ++ei)
	{
		en = * ei;
		fn = en->other(f1);
		if(fn != f2)
		{
			DebugAssert(neighbors.count(fn) == 0);
			neighbors[fn] = en;
		}
	}

	// Now we can "merge in" F2's edges.
	for(list<EdgeNode_t *>::iterator ei = f2->edges.begin(); ei != f2->edges.end(); ++ei)
	{
		en = *ei;
		fn = en->other(f2);
		if(fn == f1)
		{
			// This is the f1-f2 edge - it's GONE.
			DebugAssert(!en->must_lock);
			f1->edges.remove(en);
			delete_edge(en);
		}
		else if (neighbors.count(fn) > 0)
		{
			// Both f1 and f2 were connected to this face.  We need to merge these edges.
			EdgeNode_t * oe = neighbors[fn];
			oe->length += en->length;
			oe->count += en->count;
			if(en->must_lock)
				oe->must_lock = true;
			if(en->must_merge)
				oe->must_merge = true;
//			DebugAssert(!(oe->must_merge && oe->must_lock));
			if(oe->must_lock)
				oe->must_merge = false;
//			#if !DEV
//				#error temp hack
//			#endif
			oe->road_type = MIN_NOVALUE(oe->road_type,en->road_type);
			fn->edges.remove(en);
			delete_edge(en);
			out_neighbors.push_back(oe);
		}
		else
		{
			// This is a face adjacent to f2 but not f1.  So now that f1 and f2 are merged,
			// f1 accumulates it in its neighborhood.
			neighbors[fn] = en;
			// Migrate links over...edge is literally moved, so this is the easy case.
			if(en->f1 == f2)
				en->f1 = f1;
			else if(en->f2 == f2)
				en->f2 = f1;
			f1->edges.push_back(en);
			out_neighbors.push_back(en);
		}
	}

	delete_face(f2);
}

void			FaceGraph_t::enqueue(EdgeNode_t * en, float (* cost_func)(EdgeNode_t * en))
{
	if(queue.end() != en->self)
		queue.erase(en->self);
	float cost_now = cost_func(en);
	if(cost_now >= 0.0f)
	{
		en->self = queue.insert(EdgeQ::value_type(cost_now,en));
	}
	else
		en->self = queue.end();
}


EdgeNode_t *	FaceGraph_t::pop(void)
{
	if(queue.empty())
		return NULL;
	EdgeNode_t * ret = queue.begin()->second;
	queue.erase(queue.begin());
	ret->self = queue.end();
	return ret;
}





void		FaceGraph_t::clear(void)
{
	while(faces)
	{
		FaceNode_t * f = faces;
		faces = faces->next;
		delete f;
	}
	while(edges)
	{
		EdgeNode_t * e = edges;
		edges = edges->next;
		delete e;
	}
}

bool fill_for_zoning(const set<int>& z)
{
	if(z.empty()) return false;
	
	for(LandFillRuleTable::iterator r = gLandFillRules.begin(); r != gLandFillRules.end(); ++r)
	if(is_subset(z,r->required_zoning))
		return true;
	return false;
}

#define MAX_AREA (1000.0 * 1000.0)
#define MAX_DIM 0.01
float	edge_cost_func(EdgeNode_t * en)
{
	if(en->must_lock) return -1.0f;
	if(en->must_merge) return 9.9e9f;
	if((en->f1->area + en->f2->area) > MAX_AREA)	return -1.0f;

	if(
		fill_for_zoning(en->f1->zoning) ||
		fill_for_zoning(en->f2->zoning))
	{
		if(en->f1->zoning.empty() || en->f2->zoning.empty())
			return -1.0f;

		set<int> zc;
		set_union(
			en->f1->zoning.begin(),en->f1->zoning.end(),
			en->f2->zoning.begin(),en->f2->zoning.end(),
			set_inserter(zc));
		if(!fill_for_zoning(zc))
		{
			return -1.0f;
		}
	}
		


	Bbox2	nb = en->f1->bounds;
	nb += en->f2->bounds;
	DebugAssert(nb.area() > 0.0);

//	if(nb.xspan() > MAX_DIM || nb.yspan() > MAX_DIM)
//		return -1.0f;

	DebugAssert(nb.xspan() > 0.0);
	DebugAssert(nb.yspan() > 0.0);
	double max_aspect = max(nb.xspan() / nb.yspan(), nb.yspan() / nb.xspan());

	int diversity = set_union_length(en->f1->zoning,en->f2->zoning);

	if(en->f1->zoning.empty() && en->f2->zoning.empty())
		return en->length / max_aspect;
	else {
		return en->length / (max_aspect * (double) (1 << diversity));
	}

//	if(en->f1->zoning == en->f2->zoning)
//		return 10.0 * en->length / max_aspect;
//	else
//		return en->length / max_aspect;



//	f(en->f1->zoning == en->f2->zoning)
//		return (en->f1->bounds.area() + en->f2->bounds.area()) ;
//	else
//		return en->f1->bounds.area() + en->f2->bounds.area();
}

struct UnlinkedFace_p {
	set<Face_handle> *						universe_;
	UnlinkedFace_p(set<Face_handle> * universe) : universe_(universe) { }
	bool operator()(Face_handle who) const {
		return universe_->count(who) > 0;
	}
};

void	ColorFaces(set<Face_handle>&	io_faces)
{
		FaceGraph_t								graph;
		set<Face_handle>::iterator				f, ff;
		FaceNode_t *							fn;
		EdgeNode_t *							en;
		map<Face_handle, FaceNode_t *>			face_mapping;

	/* First: build our graph structure */
	for(f = io_faces.begin(); f != io_faces.end(); ++f)
	{
		fn = graph.new_face();
		fn->real_faces.push_back(*f);
		Bbox2 me;
		fn->area = GetMapFaceAreaMeters(*f,&me);
		fn->bounds = me;
		fn->zoning.insert((*f)->data().GetZoning());
		fn->color = -1;
		face_mapping[*f] = fn;
	}

	for(f = io_faces.begin(); f != io_faces.end(); ++f)
	{
		set<Face_handle>	friends;
		CollectionVisitor<Pmwx,Face_handle,UnlinkedFace_p> face_collector(&friends, UnlinkedFace_p(&io_faces));
		VisitAdjacentFaces<Pmwx, CollectionVisitor<Pmwx,Face_handle,UnlinkedFace_p> >(*f, face_collector);
		for(ff = friends.begin(); ff != friends.end(); ++ff)
		{
			DebugAssert(face_mapping.count(*f ));
			DebugAssert(face_mapping.count(*ff));
			FaceNode_t * f1 = face_mapping[*f ];
			FaceNode_t * f2 = face_mapping[*ff];
			if(!graph.connected(f1,f2))
			{
				en = graph.new_edge();
				en->f1 = f1;
				en->f2 = f2;
				f1->edges.push_back(en);
				f2->edges.push_back(en);
				en->self = graph.queue.end();
				en->length = 0.0f;
				en->count = 0;
				en->must_merge = false;
				en->must_lock = false;
				en->road_type = NO_VALUE;
				set<Halfedge_handle>	edges;
				FindEdgesForFace<Pmwx>(*f, edges);
				for(set<Halfedge_handle>::iterator i = edges.begin(); i != edges.end(); ++i)
				if((*i)->twin()->face() == *ff)
				{
					en->length += GetMapEdgeLengthMeters(*i);
					en->count++;
					if((*i)->data().HasRoads())
					{
						en->road_type = MIN_NOVALUE(en->road_type, (*i)->data().mSegments.front().mFeatType);
						if((*i)->data().mSegments.front().mFeatType == powerline_Generic)
							en->must_merge = true;
					}
					if((*i)->twin()->data().HasRoads())
					{
						en->road_type = MIN_NOVALUE(en->road_type, (*i)->twin()->data().mSegments.front().mFeatType);
						if((*i)->twin()->data().mSegments.front().mFeatType == powerline_Generic)
							en->must_merge = true;
					}
					if((*i)->source()->data().mNeighborBurned ||
					   (*i)->target()->data().mNeighborBurned)
					{
						en->must_merge = false;
						en->must_lock = true;
					}
					if((*i)->source()->data().mNeighborNotBurned ||
					   (*i)->target()->data().mNeighborNotBurned)
					{
						en->must_merge = true;
						en->must_lock = false;
					}
				}

//				for(set<Halfedge_handle>::iterator i = edges.begin(); i != edges.end(); ++i)
//				if((*i)->twin()->face() == *ff)
//				if(en->must_merge)
//					debug_mesh_line(cgal2ben((*i)->source()->point()),cgal2ben((*i)->target()->point()),1,1,0,1,1,0);

			}
		}
	}

	/* Second: seed the initial PQ with edges. */

	for(en = graph.edges; en; en = en->next)
		graph.enqueue(en, edge_cost_func);

	/* Third: run the PQ until we have done our "merges". */
	while((en = graph.pop()) != NULL)
	{
		list<EdgeNode_t *>	neighbors;
		graph.merge(en->f1, en->f2, neighbors);
		for(list<EdgeNode_t *>::iterator n = neighbors.begin(); n != neighbors.end(); ++n)
			graph.enqueue(*n, edge_cost_func);
	}

	int highest = 0, total = 0, over_4 = 0;

	multimap<int, FaceNode_t *, greater<int> >		nodes_by_degree;

	for(en = graph.edges; en; en = en->next)
	{
		DebugAssert(!en->must_merge);
	}

	/* Fourth: color the remaining edges. */
	for(fn = graph.faces; fn; fn = fn->next)
	{

//		set<Face_handle>		faces;
//		set<Halfedge_handle>	edges;
//		faces.insert(fn->real_faces.begin(),fn->real_faces.end());
//		FindEdgesForFaceSet<Pmwx>(faces,edges);
//		for(set<Halfedge_handle>::iterator e = edges.begin(); e != edges.end(); ++e)
//			debug_mesh_line(cgal2ben((*e)->source()->point()),cgal2ben((*e)->target()->point()),1,0,0,0,1,0);

		nodes_by_degree.insert(multimap<int, FaceNode_t *, greater<int> >::value_type(fn->edges.size(),fn));

	}

	for(multimap<int, FaceNode_t *, greater<int> >::iterator i = nodes_by_degree.begin(); i != nodes_by_degree.end(); ++i)
	{
		set<int> used;
		i->second->color = 0;
		for(list<EdgeNode_t *>::iterator ei = i->second->edges.begin(); ei != i->second->edges.end(); ++ei)
		{
			en = *ei;
			fn = en->other(i->second);
			if(fn->color != -1)	used.insert(fn->color);
		}

		vector<int>	avail;
		for(int n = 0; n < 4; ++n)
			if(used.count(n) == 0) avail.push_back(n);

		if(!avail.empty())
		{
			i->second->color = avail[rand() % avail.size()];
		}
		else
		{
			while(used.count(i->second->color))
				++i->second->color;
		}

		highest = max(highest,i->second->color);
		++total;
		if(i->second->color > 3)
		{
			++over_4;
			i->second->color = i->second->color % 4;
		}

		set<int> zoning_we_have;

		for(list<Face_handle>::iterator fh = i->second->real_faces.begin(); fh != i->second->real_faces.end(); ++fh)
		{
			(*fh)->data().mParams[af_Variant] = i->second->color;
			zoning_we_have.insert((*fh)->data().GetZoning());
		}

		set<int>	my_possible_terrains;
		for(LandFillRuleTable::iterator r = gLandFillRules.begin(); r != gLandFillRules.end(); ++r)
		if(is_subset(zoning_we_have,r->required_zoning))
			my_possible_terrains.insert(r->terrain);

		for(LandFillRuleTable::iterator r = gLandFillRules.begin(); r != gLandFillRules.end(); ++r)
		if(is_subset(zoning_we_have,r->required_zoning) && r->color == i->second->color)
		{
			for(list<Face_handle>::iterator fh = i->second->real_faces.begin(); fh != i->second->real_faces.end(); ++fh)
			if((*fh)->data().mTerrainType == NO_VALUE || my_possible_terrains.count((*fh)->data().mTerrainType))
				(*fh)->data().mTerrainType = r->terrain;
			break;
		}
	}
//	printf("Highest color: %d.  Total before: %d.  Total after: %d.  Over_4: %d.\n", highest, io_faces.size(), total,over_4);
}



FillRule_t * GetFillRuleForBlock(Pmwx::Face_handle f)
{
	int z = f->data().GetZoning();
	if(z == NO_VALUE) return NULL;

	float h = f->data().GetParam(af_HeightObjs,0.0f);

	float short_side = f->data().GetParam(af_ShortestSide,-1.0f);
	float long_side = f->data().GetParam(af_LongestSide,-1.0f);
	float short_axis = f->data().GetParam(af_ShortAxisLength,-1.0f);
	float long_axis = f->data().GetParam(af_LongAxisLength,-1.0f);
	float block_err = f->data().GetParam(af_BlockErr,-1.0f);
	float	ang_min = f->data().GetParam(af_MinAngle,-1.0f);
	float	ang_max = f->data().GetParam(af_MaxAngle,-1.0f);
	int road_edge = f->data().GetParam(af_RoadEdge,0);
	int variant = f->data().GetParam(af_Variant,0);

	for(FillRuleTable::iterator r = gFillRules.begin(); r != gFillRules.end(); ++r)
	if(r->zoning == z)
	if(r->road == 0 || r->road == road_edge)
	if(r->min_side_len == r->max_side_len || (r->min_side_len <= short_side && long_side <= r->max_side_len))
	if(r->block_err_max == 0.0 || block_err < r->block_err_max)
	if(r->min_side_major == r->max_side_major || (r->min_side_major <= long_axis && long_axis <= r->max_side_major))
	if(r->min_side_minor == r->max_side_minor || (r->min_side_minor <= short_axis && short_axis <= r->max_side_minor))
	if(r->ang_min == r->ang_max || (r->ang_min <= ang_min && ang_max < r->ang_max))
	if(r->min_height == r->max_height || (r->min_height <= h && h <= r->max_height))
	if(r->variant == -1 || r->variant == variant)
		return &*r;

	return NULL;
}

PointRule_t * GetPointRuleForFeature(int zoning, const GISPointFeature_t& f)
{
	for(PointRuleTable::iterator r = gPointRules.begin(); r != gPointRules.end(); ++r)
	if(r->zoning == NO_VALUE || r->zoning == zoning)
	if(r->feature == f.mFeatType)
	if(r->height_min == r->height_max || (f.mParams.count(pf_Height) && r->height_min <= f.mParams.find(pf_Height)->second && f.mParams.find(pf_Height)->second <= r->height_max))
	{
		return &*r;

	}
	return NULL;
}

FacadeSpelling_t * GetFacadeRule(int zoning, int variant, double front_wall_len, double height, double depth_one_fac)
{
	vector<FacadeSpelling_t *>	possible;
	FacadeSpelling_t * emerg = NULL;
	float emerg_dist = 0;
	for(FacadeSpellingTable::iterator r = gFacadeSpellings.begin(); r != gFacadeSpellings.end(); ++r)
	if(r->zoning == NO_VALUE || r->zoning == zoning)
	if(r->variant == -1 || r->variant == variant)
	if(r->height_min == r->height_max || (r->height_min <= height && height <= r->height_max))
	if(r->depth_min == r->depth_max || (r->depth_min <= depth_one_fac && depth_one_fac <= r->depth_max))
	{
		{
			double dist_to_this = 0;
			if(r->width_min > front_wall_len)
				dist_to_this = r->width_min - front_wall_len;
			if(r->width_max < front_wall_len)
				dist_to_this = front_wall_len - r->width_max;

			if(emerg == NULL)
			{
				emerg = &*r;
				emerg_dist = dist_to_this;
			}
			else if(dist_to_this < emerg_dist)
			{
				emerg = &*r;
				emerg_dist = dist_to_this;
			}
		}
		if(r->width_min == r->width_max || (r->width_min <= front_wall_len && front_wall_len <= r->width_max))
			possible.push_back(&*r);
	}
	if(possible.empty() && emerg)
	{
		printf("Wanted %lf for %s.  Best was: %lf, %lf\n",
			front_wall_len,
			FetchTokenString(zoning),
			emerg->width_min,emerg->width_max);
		return emerg;
	}
	if(!possible.empty())
	{
		return possible[rand() % possible.size()];
	}

	#if DEV
		printf("WARNING: no facade rule for %s v%d at %lf x %lf\n", FetchTokenString(zoning), variant, front_wall_len,height);
	#endif
	return NULL;
}
