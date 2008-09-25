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
#include "hl_types.h"
#include "road_extrude.h"
#include <algorithm>
#include <math.h>
using std::swap;
using std::find;
#include "CompGeomDefs3.h"
#include "CompGeomDefs2.h"

float	heading_from_one_dir(const Vector2& dir)
{
	Vector2 n(dir);
	n.normalize();
	return atan2(n.dx, -n.dy) * con.radtodeg;
}

float	heading_from_two_dirs(const Vector2& dir1, const Vector2& dir2)
{
	Vector2 n(dir1);
	n += dir2;
	n.normalize();
	return atan2(n.dx, -n.dy) * con.radtodeg;
}

void RoadDef_t::Clear(void)
{
	road_types.clear();
	texes.clear();
	texes_lit.clear();
	offsets.clear();
}

bool RoadDef_t::ReadFromDef(const char * inFile)
{
	bool	has_wires = Xfals;
	road_types.clear();
	texes.clear();
	texes_lit.clear();
	offsets.clear();

	string		file(inFile), path(inFile);
	xbyt*		MAP_add;
	xint		c_use, c_max;
	bool		is_forest = false;
	string		tex, token, tex_path;
	xint		tex_ref;

	float		scale = 1.0;

	set_dir_chars(&file);
	set_dir_chars(&path);
	strip_to_path(&path);

	XMappedFile	mappedFile(file.c_str(), &MAP_add);
	xint cur_id = 0;

	int versions[] = { 800, 0 };
	if (!MAP_check_file_text_typed(&mappedFile, MAP_add, &c_use, &c_max, versions, "ROADS", NULL)) return Xfals;
	while(TXT_MAP_continue(MAP_add,c_use,c_max))
	{
		// TEXTURE <offset> <filename>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "TEXTURE", Xfals))
		{
			offsets.push_back(TXT_MAP_int_scan(MAP_add, &c_use, c_max));
			TXT_MAP_str_scan_space(MAP_add, &c_use, c_max, &tex);
			DSF_LocateResourceSecondary(dsf_Res_Image, dsf_Res_RoadNetwork, path, tex, 1, tex_path);
			if (tex_path.empty()) MACIBM_alert(0, "Error, could not locate image for terrain", "", path, tex, t_exit);
			tex_ref = tman.load_texture(Xtrue,type_tex,t_tex_DIM,tex_anymini	,tex_path);
			if (tex_ref == null_ref) MACIBM_alert(0, "Error, could not load image for terrain", path, tex, tex_path, t_exit);
			texes.push_back(tex_ref);
		} else
		// SCALE <scale>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "SCALE", Xfals))
		{
			scale = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
		} else
		// TEXTURE_LIT <filename>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "TEXTURE_LIT", Xfals))
		{
			// The lit texture is not required.  So try to load it but push a null tex if necessary.  This is
			// because the tex and lit tex vectors must be in sync!!  That way the user can light somes without having to light them all.
			TXT_MAP_str_scan_space(MAP_add, &c_use, c_max, &tex);
			tex_ref = null_ref;
			DSF_LocateResourceSecondary(dsf_Res_Image, dsf_Res_RoadNetwork, path, tex, 1, tex_path);
			if (!tex_path.empty()) tex_ref = tman.load_texture(Xfals,type_tex,t_tex_DIM,tex_anymini	,tex_path);
			texes_lit.push_back(tex_ref);
		} else
		// ROAD_TYPE <id> <width> <length> <tex index>		<r> <g> <b>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "ROAD_TYPE", Xfals))
		{
			cur_id = TXT_MAP_int_scan(MAP_add, &c_use, c_max);


			road_types[cur_id].width = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);

			if(road_types[cur_id].width<=0.0)MACIBM_alert(0,"Road segment not greater than 0!","","","",t_exit);


			road_types[cur_id].length = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].tex_index = TXT_MAP_int_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].color[0] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].color[1] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].color[2] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].has_wires = false;
		} else
		// SEGMENT <index> <lod> <lod> <dx1> <dy1> s1 dx2 dy2 s2
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "SEGMENT", Xfals))
		{
			road_types[cur_id].segments.push_back(SegPart());
//			road_types[cur_id].segments.back().tex_index = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().lod_near = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().lod_far = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);

			road_types[cur_id].segments.back().dx[0] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().dy[0] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().s [0] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().dx[1] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().dy[1] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].segments.back().s [1] = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);

			if (scale != 1.0)
			{
				road_types[cur_id].segments.back().s [0]	/= scale;
				road_types[cur_id].segments.back().s [1]	/= scale;
			}
		} else
		// WIRE <lod> <lod> <dx> <dy> <droop>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "WIRE", Xfals))
		{
			has_wires = true;
			road_types[cur_id].wires.push_back(WireSpec());
			road_types[cur_id].wires.back().lod_near = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].wires.back().lod_far = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].wires.back().dx = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].wires.back().dy = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].wires.back().droop_rat = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].wires.back().min_dis = 100.0;
			road_types[cur_id].wires.back().max_dis = 8.0 * con.nmtomtrs;
			road_types[cur_id].has_wires = true;
		} else
		// OBJECT <model name> <dx> <rotation> <on ground> <dist> <offset>
		if (TXT_MAP_str_match_space(MAP_add, &c_use, c_max, "OBJECT", Xfals))
		{
			road_types[cur_id].objects.push_back(ObjSpec());
			string model_name;
			TXT_MAP_str_scan_space(MAP_add, &c_use, c_max, &model_name);
			string	fullpath;
			DSF_LocateResourceSecondary(dsf_Res_Object, dsf_Res_RoadNetwork, path, model_name, Xtrue, fullpath);
			if (fullpath.empty()) MACIBM_alert(0, "Unable to locate object definition", "", "", model_name, t_exit);
			fullpath.erase(0,app_path.length());
			road_types[cur_id].objects.back().model_index = build_obs(fullpath,Xtrue,Xfals);
			road_types[cur_id].objects.back().dx = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].objects.back().rotation = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].objects.back().on_ground = TXT_MAP_int_scan(MAP_add, &c_use, c_max) > 0;
			road_types[cur_id].objects.back().dist = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
			road_types[cur_id].objects.back().offset = TXT_MAP_flt_scan(MAP_add, &c_use, c_max);
		}  else
			TXT_MAP_str_scan_eoln(MAP_add, &c_use, c_max, NULL);
	}

	if (has_wires)
	{
		offsets.push_back(0);
		if (!texes_lit.empty()) texes_lit.push_back(null_ref);
		texes.push_back(t_powerlines);
	}
/*
#if DEV
	deverr << "Report on roads " << file << "\n";
	for (int n = 0; n < texes.size(); ++n)deverr << " Tex: " << n << " is: " << texes[n] << " lit: " << (texes_lit.empty() ? 0 : texes_lit[n]) << " offset " << offsets[n] << "\n";
	for (RoadTypeMap::iterator m = road_types.begin(); m != road_types.end(); ++m)
	{
		deverr << " Road type " << m->first << ": width: " << m->second.width << " Length " << m->second.length;
		deverr << " color " << m->second.color[0] << "," << m->second.color[1] << "," << m->second.color[2] << "\n";
		deverr << " has wires " << m->second.has_wires << " tex index " << m->second.tex_index << "\n";
		for (SegVector::iterator s = m->second.segments.begin(); s != m->second.segments.end(); ++s) {
			deverr << "    Near " << s->lod_near << " far " << s->lod_far << "\n";
			deverr << "    Segment from " << s->dx[0] << "," << s->dy[0] << "->" << s->dx[1] << "," << s->dy[1]
				<< " [" << s->s[0] << "," << s->s[1] << "]\n";
		}
		for (WireVector::iterator w = m->second.wires.begin(); w != m->second.wires.end(); ++w) {
			deverr << "   Wire from " << w->dx << "," << w->dy << " LOD " << w->lod_near << "," << w->lod_far << " droop " <<
				w->droop_rat << " dist: " << w->min_dis << "," << w->max_dis << "\n";
		}
		for (ObjVector::iterator o = m->second.objects.begin(); o != m->second.objects.end(); ++o) {
			deverr << "   Object " << o->model_index << " dx = " << o->dx <<
				" on ground " << o->on_ground << " rotation " << o->rotation << " dist "
				<< o->dist << " ofset " << o->offset << "\n";
		}
	}
#endif
*/
	return Xtrue;
}

void	RoadChain_t::reverse(void)
{
	swap(start_node, end_node);
	int vc = shape_points.size() / 3;
	for (int n = 0; n < (vc/2); ++n)
	{
		int on = vc - n - 1;
		swap(shape_points[n*3  ], shape_points[on*3  ]);
		swap(shape_points[n*3+1], shape_points[on*3+1]);
		swap(shape_points[n*3+2], shape_points[on*3+2]);
	}
}

float		RoadChain_t::get_heading(RoadJunction_t * junc, bool force_end)
{
	if (junc == start_node && !force_end)
	{
		if (shape_points.empty()) return atan2(end_node->location[0] - start_node->location[0],
											 end_node->location[2] - start_node->location[2]);
		 return atan2(shape_points[0] - start_node->location[0],
					 shape_points[2] - start_node->location[2]);

	} else if (junc == end_node)
	{
		if (shape_points.empty()) return atan2(start_node->location[0] - end_node->location[0],
											 start_node->location[2] - end_node->location[2]);
		 return atan2(shape_points[shape_points.size()-3] - end_node->location[0],
					 shape_points[shape_points.size()-1] - end_node->location[2]);


	}
#if DEV

	else
		deverr<<"ERROR: get_heading called with bad junction.\n";

#endif


	return 0;
}

float * RoadChain_t::get_nth_pt(int n)
{
	if (n == 0) return start_node->location;
	if (n > (shape_points.size()/3)) return end_node->location;
	return &shape_points[(n-1)*3];
}

void				RoadChain_t::get_nth_dir(int n, float xyz[3])
{
	float * p1 = get_nth_pt(n);
	float * p2 = get_nth_pt(n+1);
	xyz[0] = p2[0] - p1[0];
	xyz[1] = p2[1] - p1[1];
	xyz[2] = p2[2] - p1[2];
}

#if DEV
void				RoadChain_t::check_validity(void)
{
	for (xint n = 0; n < shape_points.size()/3+1; ++n)
	{
		float * p1 = get_nth_pt(n  );
		float * p2 = get_nth_pt(n+1);
		if (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2])
			deverr<<"SANITY CHECK FAILED - dupe point "<<n<<", p1="<<p1[0]<<" "<<p1[1]<<" "<<p1[2]<<" p2="<<p2[0]<<" "<<p2[1]<<" "<<p2[2]<<"\n";
	}
}
#endif

void			RoadJunction_t::sort_clockwise(void)
{
	map<float, RoadChain_t *>	sorted_chains;
	set<RoadChain_t *> doubles;	// A chain might go into us twice!  Track this... we'll insert it twice
								// the first time and repress the second.
	for (vector<RoadChain_t *>::iterator ci = chains.begin(); ci != chains.end(); ++ci)
	{
		RoadChain_t * c = *ci;
		if (doubles.find(c) != doubles.end()) continue;

		if (c->start_node == this && c->end_node == this)
		{
			sorted_chains.insert(map<float, RoadChain_t *>::value_type(c->get_heading(this, false), c));
			sorted_chains.insert(map<float, RoadChain_t *>::value_type(c->get_heading(this, true ), c));
			doubles.insert(c);
		} else if (c->start_node == this)
		{
			sorted_chains.insert(map<float, RoadChain_t *>::value_type(c->get_heading(this), c));
		} else if (c->end_node == this)
		{
			sorted_chains.insert(map<float, RoadChain_t *>::value_type(c->get_heading(this), c));
		}
		#if DEV
		else
			deverr<<"ERROR: Road Junction contains chain that it should not.\n";
		#endif
	}
	chains.clear();
	for (map<float, RoadChain_t *>::iterator i = sorted_chains.begin(); i != sorted_chains.end(); ++i)
		chains.push_back(i->second);
}

#pragma mark -

RoadChain_t *	RoadData::AddSimpleSegment(
						int		type,
						int		node1,
						int		node2,
						float	xyz1[3],
						float	xyz2[3],
						bool	no_dupes)
{
	RoadJunctionMap::iterator i1, i2;
	RoadJunction_t * j1, * j2;
	if (node1 >= building_junctions.size())	building_junctions.resize(node1+1);
	if (node2 >= building_junctions.size())	building_junctions.resize(node2+1);
	j1 = building_junctions[node1];
	if (j1 == NULL)
	{
		j1 = new RoadJunction_t;
		j1->location[0] = xyz1[0];
		j1->location[1] = xyz1[1];
		j1->location[2] = xyz1[2];
		j1->id = node1;
		building_junctions[node1] = j1;
		working_junctions.insert(j1);
	}
	j2 = building_junctions[node2];
	if (j2 == NULL)
	{
		j2 = new RoadJunction_t;
		j2->location[0] = xyz2[0];
		j2->location[1] = xyz2[1];
		j2->location[2] = xyz2[2];
		j2->id = node2;
		building_junctions[node2] = j2;
		working_junctions.insert(j2);
	}
	if (no_dupes)
	{
		for (vector<RoadChain_t *>::iterator c = j2->chains.begin(); c != j2->chains.end(); ++c)
		{
			if ((*c)->start_node == j1 && (*c)->end_node == j2) return *c;
			if ((*c)->start_node == j2 && (*c)->end_node == j1) return *c;
		}
	}
	RoadChain_t * c = new RoadChain_t;
	c->start_node = j1;
	c->end_node = j2;
	c->type = type;
	j1->chains.push_back(c);
	j2->chains.push_back(c);
	working_chains.insert(c);
	return c;
}

void			RoadData::AddShapePoint(
						RoadChain_t *	inChain,
						int				count,
						float			xyz[])
{
	inChain->shape_points.insert(inChain->shape_points.end(),xyz,xyz+(count*3));
}

void			RoadData::ProcessChains(
						bool			inSimplify,
						float			x1,
						float			x2,
						float			z1,
						float			z2)
{
	if (inSimplify) MakeShapePoints();
	BucketAll(x1,x2,z1,z2);
///	building_junctions.clear();
	building_junctions.clear();
	working_chains.clear();

	for (int x = 0; x < ROAD_SECTIONS; ++x)
	for (int y = 0; y < ROAD_SECTIONS; ++y)
	for (RoadJunctionSet::iterator j = working_junctions.begin(); j != working_junctions.end(); ++j)
	if (*j)
		(*j)->sort_clockwise();

}

void RoadData::MakeShapePoints(void)
{
	int total_merged = 0;
	int total_removed = 0;

	// First go through and consider every jucntion...if it has 2 items and they're the same and not a
	// loop, we can coalesce.  Look out for a colocated junctions.
	for (RoadJunctionSet::iterator junc = working_junctions.begin(); junc != working_junctions.end(); ++junc)
	{
		RoadJunction_t * me = (*junc);
		if (me->chains.size() == 2)
		{
			RoadChain_t * sc = me->chains[0];
			RoadChain_t * ec = me->chains[1];

			if (sc != ec && sc->type == ec->type &&
				sc->start_node != sc->end_node &&
				ec->start_node != ec->end_node)
			{
				// Organize so start chain feeds into end chain directionally, with
				// us as the middle junction.
				if (sc->end_node != me)		sc->reverse();
				if (ec->start_node != me)	ec->reverse();
#if DEV
				if (sc->end_node != me)
					deverr<<"Topology error.\n";
				if (ec->start_node != me)
					deverr<<"Topology error.\n";
#endif
				// These junctions cap the new complete chain.
				RoadJunction_t * sj = sc->start_node;
				RoadJunction_t * ej = ec->end_node;
#if DEV
				if (sj == me)
					deverr<<"Topology error.\n";
				if (ej == me)
					deverr<<"Topology error.\n";
#endif
				// Accumulate all shape points.
				sc->shape_points.push_back(me->location[0]);
				sc->shape_points.push_back(me->location[1]);
				sc->shape_points.push_back(me->location[2]);
				sc->shape_points.insert(sc->shape_points.end(), ec->shape_points.begin(),ec->shape_points.end());
				// We no longer have points.
				me->chains.clear();
				// End junction now has first chain instead of second.
				ej->chains.erase(find(ej->chains.begin(), ej->chains.end(), ec));
				ej->chains.push_back(sc);
				// Start chain now ends at end junction, not us.
				sc->end_node = ej;
				// Nuke second chain, it's useless.
				delete ec;
				working_chains.erase(ec);
				++total_merged;
			}
		}
	}

	// Now go through and take out the trash...schedule for deletion every 0-valence
	// junction and also undo the colocation loops.
	RoadJunctionSet	deadJuncs;
	for (RoadJunctionSet::iterator junc = working_junctions.begin(); junc != working_junctions.end(); ++junc)
	{
		if ((*junc)->chains.empty())
		{
			deadJuncs.insert(*junc);
			++total_removed;
		}
	}

	// Finally do the actual deletion.
	for (RoadJunctionSet::iterator junc = deadJuncs.begin(); junc != deadJuncs.end(); ++junc)
	{
		working_junctions.erase(*junc);
		delete (*junc);
	}
}

void		RoadData::BucketAll(float x1, float x2, float z1, float z2)
{
	float dx, dz;
	int	  ix, iz;
/*
	if (working_junctions.empty() && !building_junctions.empty())
	{
		for (RoadJunctionMap::iterator j = building_junctions.begin(); j != building_junctions.end(); ++j)
		{
			dx = ((*j)->location[0] - x1) / (x2 - x1);
			dz = ((*j)->location[2] - z1) / (z2 - z1);
			dx *= (float) ROAD_SECTIONS;
			dz *= (float) ROAD_SECTIONS;
			ix = (int)(dx + 0.5);
			iz = (int)(dz + 0.5);
			if (ix < 0) ix = 0;
			if (iz < 0) iz = 0;
			if (ix >= ROAD_SECTIONS) ix = ROAD_SECTIONS-1;
			if (iz >= ROAD_SECTIONS) iz = ROAD_SECTIONS-1;
			final_junctions[ix][iz].push_back(*j);
		}
	} else {
	for (RoadJunctionSet::iterator j = working_junctions.begin(); j != working_junctions.end(); ++j)
	{
		dx = ((*j)->location[0] - x1) / (x2 - x1);
		dz = ((*j)->location[2] - z1) / (z2 - z1);
		dx *= (float) ROAD_SECTIONS;
		dz *= (float) ROAD_SECTIONS;
		ix = (int)(dx + 0.5);
		iz = (int)(dz + 0.5);
		if (ix < 0) ix = 0;
		if (iz < 0) iz = 0;
		if (ix >= ROAD_SECTIONS) ix = ROAD_SECTIONS-1;
		if (iz >= ROAD_SECTIONS) iz = ROAD_SECTIONS-1;
		final_junctions[ix][iz].push_back(*j);
	}
	}
*/

	int	chain_counts[ROAD_SECTIONS][ROAD_SECTIONS];
#if !LIN
	Metrowerks::hash_map<xint, RoadChain_t *>	ptr_migrate;
#else
	hash_map<xint, RoadChain_t *>			ptr_migrate;
#endif
	memset(chain_counts, 0, sizeof(chain_counts));
	for (RoadChainSet::iterator c = working_chains.begin(); c != working_chains.end(); ++c)
	{
		dx = ((*c)->start_node->location[0] - x1) / (x2 - x1);
		dz = ((*c)->start_node->location[2] - z1) / (z2 - z1);
		dx *= (float) ROAD_SECTIONS;
		dz *= (float) ROAD_SECTIONS;
		ix = (int)(dx + 0.5);
		iz = (int)(dz + 0.5);
		if (ix < 0) ix = 0;
		if (iz < 0) iz = 0;
		if (ix >= ROAD_SECTIONS) ix = ROAD_SECTIONS-1;
		if (iz >= ROAD_SECTIONS) iz = ROAD_SECTIONS-1;
		chain_counts[ix][iz]++;
	}
	for (int x = 0; x < ROAD_SECTIONS; ++x)
	for (int y = 0; y < ROAD_SECTIONS; ++y)
		final_chains[x][y].resize(chain_counts[x][y]);
	memset(chain_counts, 0, sizeof(chain_counts));

	for (RoadChainSet::iterator c = working_chains.begin(); c != working_chains.end(); ++c)
	{
		dx = ((*c)->start_node->location[0] - x1) / (x2 - x1);
		dz = ((*c)->start_node->location[2] - z1) / (z2 - z1);
		dx *= (float) ROAD_SECTIONS;
		dz *= (float) ROAD_SECTIONS;
		ix = (int)(dx + 0.5);
		iz = (int)(dz + 0.5);
		if (ix < 0) ix = 0;
		if (iz < 0) iz = 0;
		if (ix >= ROAD_SECTIONS) ix = ROAD_SECTIONS-1;
		if (iz >= ROAD_SECTIONS) iz = ROAD_SECTIONS-1;
		int idx = chain_counts[ix][iz]++;
		final_chains[ix][iz][idx] = **c;
		ptr_migrate[(xint) *c] = &final_chains[ix][iz][idx];
	}

	for (RoadJunctionSet::iterator j = working_junctions.begin(); j != working_junctions.end(); ++j)
	if (*j)
	{
		for (vector<RoadChain_t *>::iterator i = (*j)->chains.begin(); i != (*j)->chains.end(); ++i)
		{
			RoadChain_t **	ptr = &*i;
			RoadChain_t * old = *i;
			if (ptr_migrate.count((xint) old) == 0)
				MACIBM_alert(0, "Topology error in chains", "", "", "", t_alert);
			*ptr = ptr_migrate[(xint) old];
		}
	}
}

#pragma mark -

void			RoadData::Preallocate(int max_node_id, int num_chains[ROAD_SECTIONS][ROAD_SECTIONS])
{
	building_junctions.resize(max_node_id+1);
	for (int n = 1; n <= max_node_id; ++n)
	{
		building_junctions[n] = new RoadJunction_t;
		building_junctions[n]->id = n;
	}

	for (int x = 0; x < ROAD_SECTIONS; ++x)
	for (int y = 0; y < ROAD_SECTIONS; ++y)
	{
		final_chains[x][y].resize(num_chains[x][y]);
	}
}

RoadChain_t *	RoadData::AddSimpleSegmentDirect(
						int		x_bucket,
						int		y_bucket,
						int		index,		// Zero based index for bucket
						int		type,
						int		node1,
						int		node2,
						float	xyz1[3],
						float	xyz2[3])
{
	RoadJunction_t * j1, * j2;

	j1 = building_junctions[node1];
	if (j1->chains.empty())
	{
		j1->location[0] = xyz1[0];
		j1->location[1] = xyz1[1];
		j1->location[2] = xyz1[2];
	}
	j2 = building_junctions[node2];
	if (j2->chains.empty())
	{
		j2->location[0] = xyz2[0];
		j2->location[1] = xyz2[1];
		j2->location[2] = xyz2[2];
	}
	RoadChain_t * c = &final_chains[x_bucket][y_bucket][index];
	c->start_node = j1;
	c->end_node = j2;
	c->type = type;
	j1->chains.push_back(c);
	j2->chains.push_back(c);
	return c;
}

void			RoadData::AddShapePointVectorAndClear(
						RoadChain_t *	inChain,
						vector<float>&	ioVec)
{
	inChain->shape_points.swap(ioVec);
}

void			RoadData::ProcessChainsPreallocated(void)
{
	for (int x = 0; x < ROAD_SECTIONS; ++x)
	for (int y = 0; y < ROAD_SECTIONS; ++y)
	for (RoadJunctionVector::iterator j = building_junctions.begin(); j != building_junctions.end(); ++j)
	if (*j)
		(*j)->sort_clockwise();
}


#pragma mark -

#if DEV
void RoadData::Dump(void)
{
	for (int y = 0; y < ROAD_SECTIONS; ++y)
	for (int x = 0; x < ROAD_SECTIONS; ++x)
	{
		deverr<<"BUCKET SECTION "<<x<<" y="<<y<<"\n";

/*
		deverr<<" Junctions:\n";
		for (int i = 0; i < final_junctions[x][y].size(); ++i)
		{
			deverr<<"  [%3d]  Junction %d - Location %f,%f,%f\n", i, final_junctions[x][y][i]->id,
				final_junctions[x][y][i]->location[0],final_junctions[x][y][i]->location[1],final_junctions[x][y][i]->location[2]);

			for (int j = 0; j < final_junctions[x][y][i]->chains.size(); ++j)
				deverr<<"     Connects with chain 0x%08X (%d -> %d)\n", final_junctions[x][y][i]->chains[j],
					final_junctions[x][y][i]->chains[j]->start_node->id,
					final_junctions[x][y][i]->chains[j]->end_node->id);
		}

		deverr<<" Chains:\n";
		for (int i = 0; i < final_chains[x][y].size(); ++i)
		{
			deverr<<"  [%3d]  Chain 0x%08X - from nodes %d to %d, type %d\n",
				i,final_chains[x][y][i],
				final_chains[x][y][i]->start_node->id, final_chains[x][y][i]->end_node->id, final_chains[x][y][i]->type);
			for (int j = 0; j < final_chains[x][y][i]->shape_points.size(); j += 3)
				deverr<<"      Shape point %f,%f,%f\n",
					final_chains[x][y][i]->shape_points[j  ],
					final_chains[x][y][i]->shape_points[j+1],
					final_chains[x][y][i]->shape_points[j+2]);
		}
*/
	}
}
#endif

void		RoadData::Clear(void)
{
	for (int x = 0; x < ROAD_SECTIONS; ++x)
	for (int y = 0; y < ROAD_SECTIONS; ++y)
	{
		final_chains[x][y].clear();
//		final_junctions.clear();
	}
	if (!working_junctions.empty())
	for (RoadJunctionSet::iterator j = working_junctions.begin(); j != working_junctions.end(); ++j)
		delete *j;
	else
	for (RoadJunctionVector::iterator j = building_junctions.begin(); j != building_junctions.end(); ++j)
	if (*j)
			delete *j;
	working_chains.clear();
	working_junctions.clear();
	building_junctions.clear();

}

void		RoadData::ExtrudeArea(
					    int					tex_index,
						const RoadDef_t&	defs,
						int					x_bucket,
						int					y_bucket,
						ExtrudeFunc_f		func,
						ReceiveObj_f		objFunc,
						void *				ref,
						void *				objRef,
						void *				objRef2)
{

#if DEV
	double	stime = clock();
	int	quant = 0, skipq = 0, skipm = 0, ocount = 0;
#endif

	int n;
	vector<float>	final_pts, local_pts, final_sts;
	for (RoadChainVector::iterator c = final_chains[x_bucket][y_bucket].begin();
		c != final_chains[x_bucket][y_bucket].end(); ++c)
	{
		RoadDef_t::RoadTypeMap::const_iterator road_type_it = defs.road_types.find(c->type);
		if (road_type_it == defs.road_types.end()) { /*skipm++;*/ continue; }
#if DEV
		if (c->start_node == c->end_node && c->shape_points.size() < 6)
			deverr << "WARNING: segment with end nodes ID " <<
				c->start_node->id << " has " << c->shape_points.size() / 3 << " shape points.\n";
#endif


		const RoadDef_t::RoadType& road_type(road_type_it->second);

		// We only need to run this case if (1) this is our texture type or (2) we have wires and it's powerline time.  That's it.
		// (This means we do make TWO passes over a powerline + road vector..the first one puts down the pavement and objects, the
		// second one does the wires.
		if (road_type.tex_index != tex_index && !(road_type.has_wires && defs.texes[tex_index] == t_powerlines)) { /*skipq++;*/ continue; }

		double	dw0 = road_type.width * -0.5;
		double	dw1 = road_type.width *  0.5;

		int	num_pts = (c->shape_points.size() / 3) + 2;
		int num_segs = num_pts-1;
		vector<Vector2>	dirs(num_segs);
		vector<Point3> pts(num_pts);
		float xyz[3];
		for (n = 0; n < num_segs; ++n)
		{
			c->get_nth_dir(n, xyz);
			dirs[n] = Vector2(xyz[0], xyz[2]);
			dirs[n].normalize();
		}
		for (n = 0; n < num_pts; ++n)
		{
			float * p = c->get_nth_pt(n);
			pts[n] = Point3(p[0], p[1], p[2]);

#if DEV
if(check_nan(p[0]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(p[1]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(p[2]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

		}
		final_pts.resize(num_pts*6);
		Vector2 d_left, d_right;
		Vector2 d_left1, d_right1;
		Vector2 d_left2, d_right2;
		for (n = 0; n < num_pts; ++n)
		{

#if DEV
if(check_nan(pts[n].x))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(pts[n].y))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(pts[n].z))MACIBM_alert(0,"NAN value!","","","",t_exit);

if(check_nan( d_left.dx))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan( d_left.dy))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(d_right.dx))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(d_right.dy))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

			if (n == 0)
			{
				d_right = d_left = dirs[n].perpendicular_cw();
				d_left *= dw0;
				d_right *= dw1;
				final_pts[n*6  ]=pts[n].x + d_left.dx;
				final_pts[n*6+1]=pts[n].y;
				final_pts[n*6+2]=pts[n].z + d_left.dy;
				final_pts[n*6+3]=pts[n].x + d_right.dx;
				final_pts[n*6+4]=pts[n].y;
				final_pts[n*6+5]=pts[n].z + d_right.dy;


#if DEV
if(check_nan(final_pts[n*6  ]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+1]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+2]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+3]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+4]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+5]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif




			} else if (n == num_segs)
			{
				d_right = d_left = dirs[n-1].perpendicular_cw();
				d_left *= dw0;
				d_right *= dw1;
				final_pts[n*6  ]=pts[n].x + d_left.dx;
				final_pts[n*6+1]=pts[n].y;
				final_pts[n*6+2]=pts[n].z + d_left.dy;
				final_pts[n*6+3]=pts[n].x + d_right.dx;
				final_pts[n*6+4]=pts[n].y;
				final_pts[n*6+5]=pts[n].z + d_right.dy;

#if DEV
if(check_nan(final_pts[n*6  ]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+1]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+2]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+3]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+4]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+5]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

			}
			else if (dirs[n-1].dot(dirs[n]) <= -1.0)
			{
				#if DEV
				deverr << "Dot product of the direction vectors is: " << dirs[n-1].dot(dirs[n]) << "\n";
				for (int de = 0; de < pts.size(); ++de)
				{
					deverr << "  Pts " << de << " is: " << pts[de].x << "," << pts[de].y << "," << pts[de].z << "\n";
					if (de != 0)
						deverr << "Dir is : " << dirs[de-1].dx << "," << dirs[de-1].dy << "\n";
				}
//				MACIBM_alert(0, "PINCH!", "", "", "", t_exit);
				#endif

			} else if (dirs[n-1].dot(dirs[n]) > 0.9) {
				d_left = dirs[n-1].perpendicular_cw();
				d_left += dirs[n].perpendicular_cw();
				d_left.normalize();
				d_right = d_left;
				d_left *= dw0;
				d_right *= dw1;
				final_pts[n*6  ]=pts[n].x + d_left.dx;
				final_pts[n*6+1]=pts[n].y;
				final_pts[n*6+2]=pts[n].z + d_left.dy;
				final_pts[n*6+3]=pts[n].x + d_right.dx;
				final_pts[n*6+4]=pts[n].y;
				final_pts[n*6+5]=pts[n].z + d_right.dy;
#if DEV
if(check_nan(final_pts[n*6  ]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+1]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+2]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+3]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+4]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+5]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

			} else {
				d_right1 = d_left1 = dirs[n-1].perpendicular_cw();
				d_left1 *= dw0;
				d_right1 *= dw1;
				d_right2 = d_left2 = dirs[n].perpendicular_cw();
				d_left2 *= dw0;
				d_right2 *= dw1;
				Segment2 s1l(Point2(pts[n-1].x, pts[n-1].z),Point2(pts[n  ].x,pts[n  ].z));
				Segment2 s2l(Point2(pts[n  ].x, pts[n  ].z),Point2(pts[n+1].x,pts[n+1].z));
				Segment2 s1r(s1l);
				Segment2 s2r(s2l);
				s1l += d_left1;
				s1r += d_right1;
				s2l += d_left2;
				s2r += d_right2;
				Point2	pl, pr;
				if (Line2(s1l).intersect(Line2(s2l), pl))
				{
					final_pts[n*6  ] = pl.x;
					final_pts[n*6+1] = pts[n].y;
					final_pts[n*6+2] = pl.y;

#if DEV
if(check_nan(final_pts[n*6  ]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+1]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+2]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

				}
				#if DEV
				else
					deverr<<"ERROR NO INTERSECTION.\n";
				#endif

				if (Line2(s1r).intersect(Line2(s2r), pr))
				{
					final_pts[n*6+3] = pr.x;
					final_pts[n*6+4] = pts[n].y;
					final_pts[n*6+5] = pr.y;

#if DEV
if(check_nan(final_pts[n*6+3]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+4]))MACIBM_alert(0,"NAN value!","","","",t_exit);
if(check_nan(final_pts[n*6+5]))MACIBM_alert(0,"NAN value!","","","",t_exit);
#endif

				}
				#if DEV
				else
					deverr<<"ERROR NO INTERSECTION.\n";
				#endif

			}

		} // End of point->quad extrusion loop

		bool	called_start = false;

		if (road_type.tex_index == tex_index)
		for (int d = 0; d < road_type.segments.size(); ++d)
		{
			if (!called_start)
				func(ext_Start_Obj, 0, NULL, NULL, 0.0, -1.0, ref);
			double dx0 = road_type.segments[d].dx[0];
			double dx1 = road_type.segments[d].dx[1];
			double dy0 = road_type.segments[d].dy[0];
			double dy1 = road_type.segments[d].dy[1];
			double s0 = road_type.segments[d].s[0];
			double s1 = road_type.segments[d].s[1];

			called_start = true;
			local_pts = final_pts;
			final_sts.resize(num_pts*4);
			final_sts[0] = s0;
			final_sts[1] = 0.0;
			final_sts[2] = s1;
			final_sts[3] = 0.0;
			double	last = 0.0;
			for (n = 0; n < num_pts; ++n)
			{
				double len = (n > 0) ? sqrt(Segment3(pts[n-1], pts[n]).squared_length()) : 0.0;
				last += (len / road_type.length);
				final_sts[n*4  ] = s0;
				final_sts[n*4+1] = last;
				final_sts[n*4+2] = s1;
				final_sts[n*4+3] = last;
				if (last > 1.0) last -= floor(last);

				local_pts[n*6  ] = extrap(0.0, final_pts[n*6  ], 1.0, final_pts[n*6+3], dx0);
				local_pts[n*6+1] = final_pts[n*6+1] + dy0;
				local_pts[n*6+2] = extrap(0.0, final_pts[n*6+2], 1.0, final_pts[n*6+5], dx0);
				local_pts[n*6+3] = extrap(0.0, final_pts[n*6  ], 1.0, final_pts[n*6+3], dx1);
				local_pts[n*6+4] = final_pts[n*6+4] + dy1;
				local_pts[n*6+5] = extrap(0.0, final_pts[n*6+2], 1.0, final_pts[n*6+5], dx1);
			}
			func(ext_Poly_QuadStrip, num_pts*2, &*local_pts.begin(), &*final_sts.begin(), road_type.segments[d].lod_near,road_type.segments[d].lod_far, ref);
		}
		if (called_start)
			func(ext_Stop_Obj, 0, NULL, NULL, 0.0, -1.0, ref);
		called_start = false;

		if (defs.texes[tex_index] == t_powerlines)
#define WIRE_TESS_FACTOR	50.0
		for (int w = 0; w < road_type.wires.size(); ++w)
		{
			xflt droop = road_type.wires[w].droop_rat;
			xflt dx = road_type.wires[w].dx;
			xflt dy = road_type.wires[w].dy;
			func(ext_Start_Obj, 0, NULL, NULL, 0.0, -1.0, ref);
			local_pts.resize((WIRE_TESS_FACTOR*(num_pts-1)+1)*3);
			for (int i = 0; i < (num_pts-1); ++i)
			{
				xflt srcx,srcz,srcy,dsty,dstx,dstz;
				srcx=extrap(0.0, final_pts[i*6  ], 1.0, final_pts[i*6+ 3], dx);
				dstx=extrap(0.0, final_pts[i*6+6], 1.0, final_pts[i*6+ 9], dx);
				srcy=extrap(0.0, final_pts[i*6+1], 1.0, final_pts[i*6+ 4], dx);
				dsty=extrap(0.0, final_pts[i*6+7], 1.0, final_pts[i*6+10], dx);
				srcz=extrap(0.0, final_pts[i*6+2], 1.0, final_pts[i*6+ 5], dx);
				dstz=extrap(0.0, final_pts[i*6+8], 1.0, final_pts[i*6+11], dx);
				for (int j = 0; j <= WIRE_TESS_FACTOR; ++j)
				{
					float rat = (float) j / WIRE_TESS_FACTOR;
					float index = WIRE_TESS_FACTOR*i+j;

					local_pts[3*index  ]=interp(0.0,srcx,1.0, dstx, rat);
					local_pts[3*index+1]=interp(0.0,srcy,1.0, dsty, rat)+dy*(1.0-droop)+dy*droop*sqr(2.0*rat-1.0);
					local_pts[3*index+2]=interp(0.0,srcz,1.0, dstz, rat);
				}
			}
			func(ext_Line_Strip, WIRE_TESS_FACTOR*(num_pts-1)+1, &*local_pts.begin(), NULL, road_type.wires[w].lod_near, road_type.wires[w].lod_far, ref);
			func(ext_Stop_Obj, 0, NULL, NULL, 0, 0, ref);
		}

		// When do we put down our objects?  We better do it exactly once.  To accomplish that, all
		// objects must have a texture type.  We will run this pass and get objects no matter what.
		if (tex_index == road_type.tex_index)
		for (int o = 0; o < road_type.objects.size(); ++o)
		{
			xflt ox = road_type.objects[o].dx;
			float	cume = road_type.objects[o].offset;
			float	total = 0.0;
			xflt heading_seg;
			xflt heading_first = heading_from_one_dir(dirs.front());
			xflt heading_last = heading_from_one_dir(dirs.back());
			xflt heading_prev;
			xflt heading_next;
			for (int i = 0; i < num_segs; ++i)
			{
				heading_seg = heading_from_one_dir(dirs[i]);
				if (i == 0) heading_prev = heading_seg;
				else		heading_prev = heading_from_two_dirs(dirs[i-1],dirs[i]);
				if (i==(num_segs-1)) heading_next = heading_seg;
				else			   heading_next = heading_from_two_dirs(dirs[i],dirs[i+1]);

				xflt srcx,srcz,srcy,dsty,dstx,dstz;
				srcx=extrap(0.0, final_pts[i*6  ], 1.0, final_pts[i*6+ 3], ox);
				dstx=extrap(0.0, final_pts[i*6+6], 1.0, final_pts[i*6+ 9], ox);
				srcy=extrap(0.0, final_pts[i*6+1], 1.0, final_pts[i*6+ 4], ox);
				dsty=extrap(0.0, final_pts[i*6+7], 1.0, final_pts[i*6+10], ox);
				srcz=extrap(0.0, final_pts[i*6+2], 1.0, final_pts[i*6+ 5], ox);
				dstz=extrap(0.0, final_pts[i*6+8], 1.0, final_pts[i*6+11], ox);
				float seglen = pythag(dstx-srcx,dsty-srcy,dstz-srcz);
				float old_total = total;
				total += seglen;
				if (road_type.objects[o].dist <= 0.0)
				{
					if (i == 0)
					{
						objFunc(srcx,srcy,srcz,
							heading_first + road_type.objects[o].rotation,
							road_type.objects[0].on_ground,
							road_type.objects[o].model_index, objRef, objRef2);
#if DEV
						++ocount;
#endif
					}
					objFunc(dstx,dsty,dstz,
							heading_next + road_type.objects[o].rotation,
							road_type.objects[0].on_ground,
							road_type.objects[o].model_index, objRef, objRef2);
#if DEV
					++ocount;
#endif
				}
				else
				{
					while (cume < total)
					{
						float rat = (cume - old_total) / seglen;
						xflt heading = heading_seg;
						if (rat == 0.0) heading = heading_prev;
						if (rat == 1.0) heading = heading_next;
						xflt x = interp(0.0, srcx, 1.0, dstx, rat);
						xflt y = interp(0.0, srcy, 1.0, dsty, rat);
						xflt z = interp(0.0, srcz, 1.0, dstz, rat);
						objFunc(x,y,z,
							heading + road_type.objects[o].rotation,
							road_type.objects[0].on_ground,
							road_type.objects[o].model_index, objRef, objRef2);
						cume += road_type.objects[o].dist;
#if DEV
						++ocount;
#endif
					}
				}
			}
		}
#if DEV
		++quant;
#endif
	}

#if DEV
	stime = clock() - stime;
	stime /= (float) CLOCKS_PER_SEC;
//	deverr << " Extruded " << quant << " road chains in " << stime << " seconds with " << ocount << "objects.\n";
//	deverr << " Skipped " << skipq << " due to wrong tex, index now is " << tex_index << " skipped " << skipm << " because no def.\n";
#endif
}
