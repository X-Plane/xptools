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
#ifndef NETPLACEMENT_H
#define NETPLACEMENT_H

#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "ProgressUtils.h"

class	CDT;
class	Pmwx;
struct	DEMGeo;

void	CalcRoadTypes(Pmwx& ioMap, const DEMGeo& inElevation, const DEMGeo& inUrbanDensity, ProgressFunc inProg);

/******************************************************************************************
 * FORMING NETWORK TOPOLOGY FROM GT-POLYGONS
 ******************************************************************************************/

struct Net_JunctionInfo_t ;
struct Net_ChainInfo_t;
typedef set<Net_JunctionInfo_t *>	Net_JunctionInfoSet;
typedef set<Net_ChainInfo_t *>		Net_ChainInfoSet;

struct	Net_JunctionInfo_t {
	int								index;
	Point3							location;					// Locations are in absolute MSL space -
	double							agl;						// agl of point over ground
	int								power_crossing;				// Crossing of powerline with a road??
	Net_ChainInfoSet				chains;
	bool							vertical_locked;			// Is this loc's vertical pre-determined already?  (If so, don't mess with it!)

	double							GetMatchingAngle(Net_ChainInfo_t * chain, Net_ChainInfo_t * match);
	Net_ChainInfo_t *				get_other(Net_ChainInfo_t * me);
};

struct	Net_ChainInfo_t {
	Net_JunctionInfo_t *			start_junction;				// Start and end junction ptrs
	Net_JunctionInfo_t *			end_junction;

	int								entity_type;				// A specific road type
	int								export_type;				// The type x-plane sees
	bool							over_water;					// Is this segment over water?
	vector<Point3>					shape;						// Intermediate shaping pts - 3d loc and
	vector<double>					agl;						// AGL height
	vector<int>						power_crossing;				// Does a powerline cross a road here?

	void							reverse(void);
	int								pt_count(void);				// Points and segments are identified by index numbers - 0 for the start, then increasing.
	int								seg_count(void);			// There is one more point than segment.
	Point3							nth_pt(int n);
	double							nth_agl(int n);
	Segment3						nth_seg(int n);

	Vector3							vector_to_junc(Net_JunctionInfo_t * junc);
	Vector2							vector_to_junc_flat(Net_JunctionInfo_t * junc);
	Net_JunctionInfo_t *			other_junc(Net_JunctionInfo_t * junc);

	double							meter_length(int pt_start, int pt_stop);	// 0, pts-1 gives total length, 0 1 gives first seg len
	double							dot_angle(int ctr_pt);						// dot product for turn at this pt.  Do not pass 0 or max pt #
	void							split_seg(int n, double rat);

};



void	BuildNetworkTopology(Pmwx& inMap, Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains);
void	CleanupNetworkTopology(Net_JunctionInfoSet& inJunctions, Net_ChainInfoSet& inChains);
void	OptimizeNetwork(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& outChains, bool water_only);

// Given the road network, insert shaping points to make sure we don't go underground.
void	DrapeRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, CDT& inMesh);
// Turn shape points into nodes
void	PromoteShapePoints(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Split up junctions vertically.
void	VerticalPartitionRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Build vertical bridge sloping
void	VerticalBuildBridges(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Build in all intermediate road heights
void	InterpolateRoadHeights(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Assign actual export types to all roadways.
void	AssignExportTypes(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Delete any chain whose export type is -1
void DeleteBlankChains(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);
// Minimize powerline segmnets!
void	SpacePowerlines(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, double ideal_dist_m, double max_dip);

void	ValidateNetworkTopology(Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains);
void	CountNetwork(const Net_JunctionInfoSet& inJunctions, const Net_ChainInfoSet& inChains);

#endif
