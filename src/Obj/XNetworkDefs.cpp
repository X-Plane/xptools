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
#include "XNetworkDefs.h"

inline	bool	match_types(const vector<int>& pattern, const vector<int>& example)
{
	if (pattern.size() != example.size()) return false;
	for (int n = 0; n < pattern.size(); ++n)
		if (pattern[n] != -1 && pattern[n] != example[n]) return false;
	return true;
}

void NetworkSegment_t::for_lod(float lod_near, float lod_far, NetworkSegmentLOD_t& segment) const
{
	if (lod.empty())
	{
		segment.scale_lon = 0.0;
		return;
	}
	for (int i = 0; i < lod.size(); ++i)
	{
		if (lod[i].lod_near == lod_near && lod[i].lod_far == lod_far)
		{
			segment = lod[i];
			return;
		}
	}
	segment = lod[0];
}


void	JunctionRule_t::rotate()
{
	junction_types.push_back(junction_types.front());
	junction_types.erase(junction_types.begin());
	s_coord.push_back(s_coord.front());;
	s_coord.erase(s_coord.begin());
	t_coord.push_back(t_coord.front());
	t_coord.erase(t_coord.begin());
}



void	NetworkDef_t::find_caps(
					const vector<int>& 	frontJunction,
					const vector<int>& 	backJunction,
					float					lod_near,
					float					lod_far,
					NetworkSegmentLOD_t&	outFrontCap,
					NetworkSegmentLOD_t&	outBackCap) const
{
	outFrontCap.scale_lon = 0.0;
	outFrontCap.chop_point_percent = -1.0;
	outBackCap.scale_lon = 0.0;
	outBackCap.chop_point_percent = -1.0;

	const NetworkCapRules_t& rules = caps[frontJunction[0]];

	bool	found_front = false, found_back = false;

	int	front_count = frontJunction.size();
	int back_count = backJunction.size();

	for (NetworkCapRules_t::const_iterator rule = rules.begin(); rule != rules.end(); ++rule)
	{
		int	rule_count = rule->junction_types.size();
		if ((rule_count == 0 || (rule_count == front_count && match_types(rule->junction_types, frontJunction))) &&  !found_front)
		{
			found_front = true;
			rule->front_cap.for_lod(lod_near, lod_far, outFrontCap);
		}
		if ((rule_count == 0 || (rule_count == back_count && match_types(rule->junction_types, backJunction))) &&  !found_back)
		{
			found_back = true;
			rule->back_cap.for_lod(lod_near, lod_far, outBackCap);
		}
		if (found_front && found_back)
			break;
	}
}

// A perhaps important implementation note: end caps are sorted by the type of road we are
// connecting to, so we can search instantly for the right pattern (e.g. the "start pt" of
// the junction is well defined).  For junctions themselves they are NOT!

bool	NetworkDef_t::find_junction(
					const vector<int>&	junctionPattern,
					JunctionRule_t&		outJunction) const
{
	outJunction.s_coord.clear();
	outJunction.t_coord.clear();
	outJunction.junction_types.clear();

	for (vector<JunctionRule_t>::const_iterator rule = junctions.begin();
		rule != junctions.end(); ++rule)
	{
		if (junctionPattern.size() == rule->junction_types.size())
		{
			JunctionRule_t	localRule = *rule;
			int n = localRule.junction_types.size();
			while (n > 0)
			{
				if (match_types(localRule.junction_types, junctionPattern))
				{
					outJunction = localRule;
					return true;
				}
				localRule.rotate();
				--n;
			}
		}
	}
	return false;
}

bool	NetworkDef_t::find_junction_part(
					int					roadType,
					JunctionRule_t&		outJunction) const
{
	outJunction.s_coord.clear();
	outJunction.t_coord.clear();
	outJunction.junction_types.clear();

	for (vector<JunctionRule_t>::const_iterator rule = junctions.begin();
		rule != junctions.end(); ++rule)
	{
		if (rule->junction_types.size() == 1 && rule->junction_types[0] == roadType)
		{
			outJunction = *rule;
			return true;
		}
	}
	return false;
}

Vector3 NetworkData_t::chain_get_start_tangent(int index) const
{
	if (chains[index].has_start_curve)
		return	Vector3(junctions[chains[index].start_junction].location, chains[index].start_curve);
	if (!chains[index].shape_points.empty())
		return	Vector3(junctions[chains[index].start_junction].location, chains[index].shape_points.front().location);

	return Vector3(junctions[chains[index].start_junction].location, junctions[chains[index].end_junction].location);
}

Vector3 NetworkData_t::chain_get_end_tangent(int index) const
{
	if (chains[index].has_end_curve)
		return	Vector3(junctions[chains[index].end_junction].location, chains[index].end_curve);
	if (!chains[index].shape_points.empty())
		return	Vector3(chains[index].shape_points.back().location, junctions[chains[index].end_junction].location);

	return Vector3(junctions[chains[index].start_junction].location, junctions[chains[index].end_junction].location);
}

Vector3		NetworkData_t::junction_get_spur_heading(NetworkJunction_t::spur chain) const
{
	if (chain.second)
		return chain_get_end_tangent(chain.first) * -1.0;
	else
		return chain_get_start_tangent(chain.first);
}


typedef	map<double, NetworkJunction_t::spur>	JunctionSorter_t;
typedef	JunctionSorter_t::value_type			JunctionSorterItem_t;
typedef	vector<JunctionSorter_t>				JunctionSorterVector_t;

void		NetworkData_t::build_back_links(void)
{
	JunctionSorterVector_t	orders;
	int					junction, chain;

	orders.resize(junctions.size());

	for (chain = 0; chain < chains.size(); ++chain)
	{
		Vector3	start_out = chain_get_start_tangent(chain);
		Vector3 end_out = chain_get_end_tangent(chain);
		end_out *= -1.0;

		double	start_angle = atan2(start_out.dx, -start_out.dz);
		double	end_angle = atan2(end_out.dx, -end_out.dz);

		orders[chains[chain].start_junction].insert(
			JunctionSorterItem_t(start_angle, NetworkJunction_t::spur(chain, false)));
		orders[chains[chain].end_junction].insert(
			JunctionSorterItem_t(end_angle, NetworkJunction_t::spur(chain, true)));
	}

	for (junction = 0; junction < junctions.size(); ++junction)
	{
		junctions[junction].spurs.clear();
		for (JunctionSorter_t::iterator spur = orders[junction].begin();
			spur != orders[junction].end(); ++spur)
		{
			junctions[junction].spurs.push_back(spur->second);
		}
	}
}

static	int	find_spur_index(const NetworkData_t& roads, const NetworkJunction_t::spur& spur)
{
	int junction = spur.second ? roads.chains[spur.first].end_junction : roads.chains[spur.first].start_junction;

	for (int n = 0; n < roads.junctions[junction].spurs.size(); ++n)
	{
		if (roads.junctions[junction].spurs[n] == spur)
			return n;
	}
	return -1;
}

NetworkJunction_t::spur
NetworkData_t::previous_chain(NetworkJunction_t::spur chain) const
{
	int junction = chain.second ? chains[chain.first].end_junction : chains[chain.first].start_junction;

	int index = find_spur_index(*this, chain);
	if (index == -1) return chain;
	--index;
	if (index < 0)
		index += junctions[junction].spurs.size();
	return junctions[junction].spurs[index];
}

NetworkJunction_t::spur
NetworkData_t::next_chain(NetworkJunction_t::spur chain) const
{
	int junction = chain.second ? chains[chain.first].end_junction : chains[chain.first].start_junction;

	int index = find_spur_index(*this, chain);
	if (index == -1) return chain;
	++index;
	if (index >= junctions[junction].spurs.size())
		index = 0;
	return junctions[junction].spurs[index];
}

void
NetworkData_t::get_junction_types(NetworkJunction_t::spur chain, vector<int>& out_types) const
{
		int		offset;

	int junction = chain.second ? chains[chain.first].end_junction : chains[chain.first].start_junction;

	vector<NetworkJunction_t::spur>::const_iterator i = find(junctions[junction].spurs.begin(), junctions[junction].spurs.end(), chain);
	if (i == junctions[junction].spurs.end())
	{
#if DEV
		MACIBM_alert(0, "Error: spur not in junction, some kind of data corruption?!?", "", "", "", t_exit);
#endif
		return;
	}
	offset = distance(junctions[junction].spurs.begin(), i);

	get_junction_types(junction, out_types);

	while(offset--)
	{
		out_types.push_back(out_types.front());
		out_types.erase(out_types.begin());
	}
}

void
NetworkData_t::get_junction_types(int junction, vector<int>& out_types) const
{
		int		n;

	out_types.clear();

	for (n = 0; n < junctions[junction].spurs.size(); ++n)
		out_types.push_back(chains[junctions[junction].spurs[n].first].chain_type);

}
