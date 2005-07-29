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
	Point3							location;					// Locations are in absolute MSL spcae - 
	double							ground;						// MSL of ground this road goes on/over
//	Net_JunctionInfoSet				colocated;
	Net_ChainInfoSet				chains;
};

struct	Net_ChainInfo_t {
	Net_JunctionInfo_t *			start_junction;
	Net_JunctionInfo_t *			end_junction;
	int								entity_type;				// A specific road type
	bool							over_water;					// 
	vector<Point3>					shape;
	vector<double>					ground;
	
	void							reverse(void);
	int								pt_count(void);				// Points and segments are identified by index numbers - 0 for the start, then increasing.	
	int								seg_count(void);			// There is one more point than segment.
	Point3							nth_pt(int n);
	double							nth_ground(int n);
	Segment3						nth_seg(int n);	
	
	Vector3							vector_to_junc(Net_JunctionInfo_t * junc);
	Vector2							vector_to_junc_flat(Net_JunctionInfo_t * junc);
	Net_JunctionInfo_t *			other_junc(Net_JunctionInfo_t * junc);
};



void	BuildNetworkTopology(Pmwx& inMap, Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains);
void	CleanupNetworkTopology(Net_JunctionInfoSet& inJunctions, Net_ChainInfoSet& inChains);
void	OptimizeNetwork(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& outChains);

void	DrapeRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains, CDT& inMesh);
void	VerticalPartitionRoads(Net_JunctionInfoSet& ioJunctions, Net_ChainInfoSet& ioChains);

//void	MakeHappyBridges(Net_ChainInfoSet& outChains);

void	ValidateNetworkTopology(Net_JunctionInfoSet& outJunctions, Net_ChainInfoSet& outChains);
void	CountNetwork(const Net_JunctionInfoSet& inJunctions, const Net_ChainInfoSet& inChains);
				
#endif
