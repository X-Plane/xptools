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
#if 0
#include "Env2DSF.h"
#include "Persistence.h"
#include "DSFLib.h"
#include "EnvDefs.h"

/*
	NOT DONE YET: 
	Water behind custom land use textures
	Randomizing land use textures start offset
	Handling land uses with more than 1 rep (e.g. not just a 1x1 square)	
	Handling projected land use like water
	Building tri strips and other optimizations in mesh
	
	handling default objects!
	
*/

int	kObjectGranularity	= 10;								// New object every 10 meters
const double	kMaxLODForTerrain = 25.0 * 6080.0 / 3.2;	// 25 nm visibility max
const int		kMeshPatchSize = 10;						// Number of quads per terrain section, must be divisor of 150 & 200


const	char *	kCompatibilityNetworkName = "XP6Network";
const int	kNetworkTypeRoad = 0;
const int	kNetworkTypeTrain = 1;
const int	kNetworkTypeTrail = 2;


const	char *	kCompatibilityProtoypeNames[] = {
	"XP6ControlTower",		// 1 in X-Plnae!!
	"XP6Skyscraper",		// 2  "   "
	"XP6RadioTower",		// 3  "   "
	"XP6PowerTower",		// etc.
	"XP6CoolingTower",
	"XP6SmokeStacks",
	0
};
	

string	TerrainDefFromLandUse(int inLanduse)
{
	static	char	buf[31];
	sprintf(buf,"Landuse_%d", inLanduse);
	return string(buf);
}

string	TerrainDefFromCustex(const string& inTex)
{
	return string("CustomTex_") + inTex;
}

string	DefaultObjFromTypeAndHeight(int type, double height)
{
	int sheight = height;
	sheight /= kObjectGranularity;
	static	char	buf[63];
	sprintf(buf, "%s_%d", kCompatibilityProtoypeNames[type], sheight);
	return string(buf);
}

////////// MESH DEFINITIONS ////////////////


// These massive arrays are used to expand the terrain mesh!
// All vertex indices go from southwest clockwise
float	gMeshXYZ[151][201][3];			// quad X, quad Y, coord (XYZ) index				Coordinates of the vertices
short	gDefn[150][200][4];				// quad X, quad Y, layer #							Definition index for this layer or -1 if unused
float	gMeshST1[150][200][4][4][2];	// quad X, quad Y, layer #, vertex #, ST index		S&T tex coords of this layer
float	gMeshST2[150][200][4][4][2];	// quad X, quad Y, layer #, vertex #, ST index		S&T mask coords of this layer
char	gMeshMulti[150][200][4];		// quad X, quad Y, layer #							Is this layer multitextured?

unsigned int		gMeshIndex[151][201];			// vertex X, vertex Y
unsigned int		gST1Index[150][200][4][4];		// quad X, quad Y, layer #, vertex #
unsigned int		gST2Index[150][200][4][4];		// quad X, quad Y, layer #, vertex #

typedef long long INDEX_t;

inline int		ENCODE_LL_LON(double lon) { return lon * (double) (1 << 23); }
inline int		ENCODE_LL_LAT(double lat) { return lat * (double) (1 << 23); }
inline INDEX_t 	ENCODE_LL_INDEX(double lon, double lat) { return (((INDEX_t) ENCODE_LL_LON(lon)) << 32) | ((INDEX_t) ENCODE_LL_LAT(lat)); }

inline int		ENCODE_LLE_LON(double lon) { return lon * (double) (1 << 23); }
inline int		ENCODE_LLE_LAT(double lat) { return lat * (double) (1 << 23); }
inline int		ENCODE_LLE_ALT(double alt) { return alt * (double) (1 << 16); }
inline INDEX_t 	ENCODE_LLE_INDEX(float lon, float lat, float alt) { return (((INDEX_t) ENCODE_LLE_LON(lon)) << 32) | ((INDEX_t) ENCODE_LLE_LAT(lat)) | (((INDEX_t) ENCODE_LLE_ALT(alt)) << 24); }

inline short	ENCODE_ST_S(float s) { return s * (float) (1 << 14); }
inline short	ENCODE_ST_T(float t) { return t * (float) (1 << 14); }
inline INDEX_t 	ENCODE_ST_INDEX(float s, float t) { return (((INDEX_t) ENCODE_ST_S(s)) << 32) | ((INDEX_t) ENCODE_ST_T(t)); }


int		AccumPoint2(float lon, float lat, vector<int>& ioPts, map<INDEX_t, int>& ioIndex);
int		AccumPoint3(float lon, float lat, float alt, vector<int>& ioPts, map<INDEX_t, int>& ioIndex);
int		AccumPointST(float s, float t, vector<short>& ioPts, map<INDEX_t, int>& ioIndex);

const	float	kLayerMasks[16][2] = {	// Bitmask, S&T index
/* 0	*/	{	-1.0, -1.0 },

/* 1	*/	{	1.0, 3.0 },
/* 2	*/	{	1.0, 2.0 },
/* 3	*/	{	2.0, 3.0 },
/* 4	*/	{	0.0, 2.0 },

/* 5	*/	{	1.0, 3.0 },
/* 6	*/	{	1.0, 3.0 },
/* 7	*/	{	1.0, 3.0 },
/* 8	*/	{	1.0, 3.0 },

/* 9	*/	{	1.0, 3.0 },
/* 10	*/	{	1.0, 3.0 },
/* 11	*/	{	1.0, 3.0 },
/* 12	*/	{	1.0, 3.0 },
/* 13	*/	{	1.0, 3.0 },

/* 14	*/	{	1.0, 3.0 },
/* 15	*/	{	-1.0, -1.0 } };

int	kQuadDeltaX[4] = { 0, 0, 1, 1 };
int kQuadDeltaY[4] = { 0, 1, 1, 0 };

//////////// NETWORK DEFINITIONS /////////////////////

struct	NetworkNode_t {
	float		lon;
	float		lat;
	unsigned int	index;
	unsigned int	index3;
	set<int>	segs;
};

struct	NetworkSeg_t {
	int		start;
	int		end;
	int		kind;
	bool	erased;
	vector<float>	shapeLon;
	vector<float>	shapeLat;
	vector<unsigned int>		shape;
	vector<unsigned int>		shapeCurve;
};

void	AccumSegment(	float lon1, float lat1, 
						float lon2, float lat2, int kind,	
						vector<NetworkNode_t>&		nodes,
						map<float, int>&			nodeIndex,
						vector<NetworkSeg_t>&		segments);

bool	CanMerge(		int nodeNum,
						int& seg1, int& seg2,
						vector<NetworkNode_t>&		nodes,
						vector<NetworkSeg_t>&		segments);

void	MergeSegs(		int inSeg1, int inSeg2,
						vector<NetworkNode_t>&		nodes,
						vector<NetworkSeg_t>&		segments);


void	Env2DSF(const char * filename)
{
		int				n, x, y, layer, dx, dy, landuse;

	/********************************************************************
	 * COLLECT DEFINITIONS NEEDED FOR THIS ENV FILE
	 ********************************************************************/
	 
	// There are no 'built in' definitions in a DSF file, so we must reference
	// a library of definitions that provide compatibility with built-in OBJ6
	// entities.  We also may have to wrap some OBJ6 concepts (like custom terrain 
	// types and land uses) in DSF definitions.  This code goes through all of
	// the definitions used in this file and builds up definition lists and
	// indices for fast output.


	// BUILD SET OF TERRAIN DEFINITIONS NEEDED FOR THIS FILE

		set<short>			neededLandUses;		// The land uses really used in this file
		map<short, int>		landUseToTerrain;	// An index from land use to def index
		map<string, int>	customTexToTerrain;	// An idnex from custom texture to def index
		vector<string>		terrainDefs;
		
	for (n = 0; n < gVertices.size(); ++n)
	if (!gVertices[n].custom)
		neededLandUses.insert(gVertices[n].landUse);
	
	n = 0;
	for (set<short>::iterator lu = neededLandUses.begin();
		lu != neededLandUses.end(); ++lu, ++n)
	{
		terrainDefs.push_back(TerrainDefFromLandUse(*lu));
		landUseToTerrain.insert(map<short, int>::value_type(*lu, n));
	}
	
	n = neededLandUses.size();
	for (vector<string>::iterator tex = gTextures.begin();
		tex != gTextures.end(); ++tex, ++n)
	{
		terrainDefs.push_back(TerrainDefFromCustex(*tex));
		customTexToTerrain.insert(map<string, int>::value_type(*tex, n));
	}
	
	// BUILD SET OF OBJECT DEFINITIONS FOR CUSTOM OBJECTS
	
		set<string>			customObjects;			// All custom objects we use
		map<string, int>	customObjectIndex;		// An index from custom object to def index
		vector<string>		customObjectDefinitions;// Our custom object defs we need
		
	for (n = 0; n < gObjects.size(); ++n)
	if (gObjects[n].kind == kObstacleTypeCustom)
		customObjects.insert(gObjects[n].name);
	else
		customObjects.insert(DefaultObjFromTypeAndHeight(gObjects[n].kind, gObjects[n].elevation));
	
	n = 0;
	for(set<string>::iterator obj = customObjects.begin();
		obj != customObjects.end(); ++obj, ++n)
	{
		customObjectDefinitions.push_back(*obj);
		customObjectIndex.insert(map<string,int>::value_type(*obj, n));
	}
		
	// BUILD A DEFAULT SET OF PROTOTYPES FOR DEFAULT OBJECTS
	
		vector<string>		prototypeDefs;
	
	// BUILD A SET OF DEFAULT PROTOTYPES FOR ROADS, ETC.
	
		vector<string>		networkDefinitions;
	
	networkDefinitions.push_back(kCompatibilityNetworkName);

	/********************************************************************
	 * EXPAND THE TERRAIN MESH
	 ********************************************************************/
	
	// We really need to expand the terrain mesh...some interpretation is done
	// based on combinations of nearby land use.  Best to work this out once.

	for (y = 0; y < 201; ++y)
	for (x = 0; x < 151; ++x)
	{
		gMeshXYZ[x][y][0] = gVertices[x+y*151].longitude;
		gMeshXYZ[x][y][1] = gVertices[x+y*151].latitude;
		gMeshXYZ[x][y][2] = gVertices[x+y*151].elevation;
	}
	
	for (y = 0; y < 200; ++y)
	for (x = 0; x < 150; ++x)
	{
		gDefn[x][y][0] = -1;
		gDefn[x][y][1] = -1;
		gDefn[x][y][2] = -1;
		gDefn[x][y][3] = -1;

		n = x+y*151;		
		
		if (gVertices[n].custom)
		{
			gDefn[x][y][0] = customTexToTerrain[gTextures[gVertices[n].landUse]];
			gMeshMulti[x][y][0] = 0;
			gMeshST1[x][y][0][0][0] = (float)  gVertices[n].xOff    / (float) gVertices[n].scale;
			gMeshST1[x][y][0][1][0] = (float)  gVertices[n].xOff    / (float) gVertices[n].scale;
			gMeshST1[x][y][0][2][0] = (float) (gVertices[n].xOff+1) / (float) gVertices[n].scale;
			gMeshST1[x][y][0][3][0] = (float) (gVertices[n].xOff+1) / (float) gVertices[n].scale;
			gMeshST1[x][y][0][0][1] = (float)  gVertices[n].yOff    / (float) gVertices[n].scale;
			gMeshST1[x][y][0][1][1] = (float) (gVertices[n].yOff+1) / (float) gVertices[n].scale;
			gMeshST1[x][y][0][2][1] = (float) (gVertices[n].yOff+1) / (float) gVertices[n].scale;
			gMeshST1[x][y][0][3][1] = (float)  gVertices[n].yOff    / (float) gVertices[n].scale;
		} else {
			if (gVertices[n+1].custom || gVertices[n+151].custom || gVertices[n+152].custom)
			{
				gDefn[x][y][0] = landUseToTerrain[gVertices[n].landUse];
				gMeshMulti[x][y][0] = 0;
				gMeshST1[x][y][0][0][0] = 0.0;
				gMeshST1[x][y][0][1][0] = 0.0;
				gMeshST1[x][y][0][2][0] = 1.0;
				gMeshST1[x][y][0][3][0] = 1.0;
				gMeshST1[x][y][0][0][1] = 0.0;
				gMeshST1[x][y][0][1][1] = 1.0;
				gMeshST1[x][y][0][2][1] = 1.0;
				gMeshST1[x][y][0][3][1] = 0.0;
			} else {
				for (set<short>::iterator lu = neededLandUses.begin(); lu != neededLandUses.end(); ++lu)
				{
					char	mask = 0;
					if (gVertices[n    ].landUse == *lu)	mask |= 0x1;
					if (gVertices[n+151].landUse == *lu)	mask |= 0x2;
					if (gVertices[n+152].landUse == *lu)	mask |= 0x4;
					if (gVertices[n+1  ].landUse == *lu)	mask |= 0x8;
					if (mask != 0)
					{
						int layer = 0;
						while (gDefn[x][y][layer] != -1)	++layer;
						
						gDefn[x][y][layer] = landUseToTerrain[*lu];
						gMeshMulti[x][y][layer] = (layer != 0);
						
						gMeshST1[x][y][layer][0][0] = 0.0;
						gMeshST1[x][y][layer][1][0] = 0.0;
						gMeshST1[x][y][layer][2][0] = 1.0;
						gMeshST1[x][y][layer][3][0] = 1.0;
						gMeshST1[x][y][layer][0][1] = 0.0;
						gMeshST1[x][y][layer][1][1] = 1.0;
						gMeshST1[x][y][layer][2][1] = 1.0;
						gMeshST1[x][y][layer][3][1] = 0.0;
						
						if (layer != 0)
						{
							gMeshST2[x][y][layer][0][0] = kLayerMasks[mask][0] / 4.0 + 0.0;
							gMeshST2[x][y][layer][1][0] = kLayerMasks[mask][0] / 4.0 + 0.0;
							gMeshST2[x][y][layer][2][0] = kLayerMasks[mask][0] / 4.0 + 0.25;
							gMeshST2[x][y][layer][3][0] = kLayerMasks[mask][0] / 4.0 + 0.25;
							gMeshST2[x][y][layer][0][1] = kLayerMasks[mask][1] / 4.0 + 0.0;
							gMeshST2[x][y][layer][1][1] = kLayerMasks[mask][1] / 4.0 + 0.25;
							gMeshST2[x][y][layer][2][1] = kLayerMasks[mask][1] / 4.0 + 0.25;
							gMeshST2[x][y][layer][3][1] = kLayerMasks[mask][1] / 4.0 + 0.0;
						}
					}
				}
			}
		}
	}

	/********************************************************************
	 * RECONNECT THE NETWORKS!
	 ********************************************************************/
	
	// We need to go through and rebuild the networks.  Fortunately there are no unvertexed crossings
	// in the network data, thanks to its original source (topology-level 2 VMAP0 VPF).
	
	// Make a node for every vertex.
	// Make a segment for every line.
	// For every vertex with valence 2 and equal types, consolidate the lines with a shape node
	// (When we build this thing we'll skip valence 0 nodes!)
	
		vector<NetworkNode_t>		nodes;
		map<float, int>				nodeIndex;	// Maps coords to nodes for fast building
		vector<NetworkSeg_t>		segments;

	for (n = 1; n < gRoads.size(); ++n)
	if (!gRoads[n-1].term)
		AccumSegment(gRoads[n-1].longitude, gRoads[n-1].latitude,
						 gRoads[n  ].longitude, gRoads[n  ].latitude,
						 kNetworkTypeRoad, 
						 nodes,
						 nodeIndex,
						 segments);						 

	for (n = 1; n < gTrains.size(); ++n)
	if (!gTrains[n-1].term)
		AccumSegment(gTrains[n-1].longitude, gTrains[n-1].latitude,
						 gTrains[n  ].longitude, gTrains[n  ].latitude,
						 kNetworkTypeTrain, 
						 nodes,
						 nodeIndex,
						 segments);						 

	for (n = 1; n < gTrails.size(); ++n)
	if (!gTrails[n-1].term)
		AccumSegment(gTrails[n-1].longitude, gTrails[n-1].latitude,
						 gTrails[n  ].longitude, gTrails[n  ].latitude,
						 kNetworkTypeTrail, 
						 nodes,
						 nodeIndex,
						 segments);						 

	// Recursively merge any adjacent segments of same type at a 2-valence vertex
	// down into a single segment with a shape point.

	bool	didMerge = false;
	do {
		didMerge = false;
		for (n = 0; n < nodes.size(); ++n)
		{
			int s1, s2;
			if (CanMerge(n, s1, s2, nodes, segments))
			{
				MergeSegs(s1, s2, nodes, segments);
				didMerge = true;
			}
		}
	} while (didMerge);

	/********************************************************************
	 * COLLECT GEOMETRY POINTS FOR THIS FILE
	 ********************************************************************/

	// Now we go through and collect all points in this file, replacing the points with index numbers.
	
	// BUILD VERTEX INDICES FOR MESH

		vector<int>			points2d, points3d;
		vector<short>		pointsST;
		map<INDEX_t, int>	pIndex2d, pIndex3d, pIndexST;
		vector<unsigned int>		objLocationIndex;

	for (x = 0; x < 151; ++x)
	for (y = 0; y < 201; ++y)
	{
		n = x + y * 151;
		gMeshIndex[x][y] = AccumPoint3(gVertices[n].longitude, gVertices[n].latitude, gVertices[n].elevation, points3d, pIndex3d);
	}
	for (x = 0; x < 150; ++x)
	for (y = 0; y < 200; ++y)
	for (layer = 0; layer < 4; ++layer)
	for (n = 0; n < 4; ++n)
	{
		if (gDefn[x][y][layer] != -1)
		{		
			gST1Index[x][y][layer][n] = AccumPointST(gMeshST1[x][y][layer][n][0], gMeshST1[x][y][layer][n][1], pointsST, pIndexST);
			if (gMeshMulti[x][y][layer])
			{
				gST2Index[x][y][layer][n] = AccumPointST(gMeshST2[x][y][layer][n][0], gMeshST2[x][y][layer][n][1], pointsST, pIndexST);
			}
		}
	}	

	// BUILD VERTEX INDICES FOR OBJECTS
	
	for (n = 0; n < gObjects.size(); ++n)
	{
		objLocationIndex.push_back(AccumPoint2(gObjects[n].longitude, gObjects[n].latitude, points2d, pIndex2d));
	}
	
	// BUILD VERTEX INDICES FOR PATHS
	
	for (n = 0; n < nodes.size(); ++n)
	if (!nodes[n].segs.empty())
	{
		nodes[n].index = AccumPoint2(nodes[n].lon, nodes[n].lat, points2d, pIndex2d);
		nodes[n].index3 = AccumPoint3(nodes[n].lon, nodes[n].lat, 0.0, points3d, pIndex3d);
	}
		
	for (n = 0; n < segments.size(); ++n)
	if (!segments[n].erased)
	for (x = 0; x < segments[n].shapeLat.size(); ++x)
	{
		segments[n].shape.push_back(AccumPoint3(segments[n].shapeLon[x], segments[n].shapeLat[x],0.0,  points3d, pIndex3d));
		segments[n].shapeCurve.push_back(0xFFFFFFFF);
	}

	/********************************************************************
	 * WRITE THE ACTUAL DSF FILE
	 ********************************************************************/

		DSFFileWriter	dsf(filename);

	// WRITE OUT DEFINITIONS
	
	dsf.AddDefinitions(terrainDefs, customObjectDefinitions, prototypeDefs, networkDefinitions);
	
	// WRITE OUT GEOMETRY
	
	dsf.AddGeometry(points2d.size() / 2, &*points2d.begin(), 
					points3d.size() / 3, &*points3d.begin(),
					pointsST.size() / 2, &*pointsST.begin());
		
	// WRITE OUT TERRAIN MESH


	for (y = 0; y < 200; y += kMeshPatchSize)
	for (x = 0; x < 150; x += kMeshPatchSize)
	{
		dsf.AddTerrainLODCmd(0, /*0.0, */kMaxLODForTerrain);
		for (layer = 0; layer < 4; ++layer)
		for (landuse = 0; landuse < terrainDefs.size(); ++landuse)
		for (dy = 0; dy < kMeshPatchSize; ++dy)
		for (dx = 0; dx < kMeshPatchSize; ++dx)
		if (gDefn[x+dx][y+dy][layer] == landuse)
		{
			dsf.AddGeometryCmd(
						gDefn[x+dx][y+dy][layer], 
						(layer == 0) ? dsf_Flag_ZBuffer_Write : dsf_Flag_ZBuffer_Overlay, 
						(layer == 0) ? dsf_Flag_Hardness_Solid : dsf_Flag_Hardness_Overlay, 
						gMeshMulti[x+dx][y+dy][layer] ? 3 : 2);
			dsf.AddGeometryPolygon(dsf_Geo_Quad);
			for(n = 0; n < 4; ++n)
			{
				if (gMeshMulti[x+dx][y+dy][layer])
					dsf.AddGeometryVertexMasked(gMeshIndex[x+dx+kQuadDeltaX[n]][y+dy+kQuadDeltaY[n]],
											gST1Index[x+dx][y+dy][layer][n],
											gST2Index[x+dy][y+dy][layer][n]);
				else		
					dsf.AddGeometryVertexTextured(gMeshIndex[x+dx+kQuadDeltaX[n]][y+dy+kQuadDeltaY[n]],
											gST1Index[x+dx][y+dy][layer][n]);
			}
		}
	}

	// WRITE OUT ROAD NETWORK

		vector<unsigned int>		nodePoints;
		map<int, int>				usedNodeIndex;

	for (n = 0; n < nodes.size(); ++n)
	if (!nodes[n].segs.empty())
	{
		nodePoints.push_back(nodes[n].index);
		usedNodeIndex.insert(map<int,int>::value_type(n, nodePoints.size()-1));
	}
	
	dsf.AddNetworkJunctionTableCmd(0, &*nodePoints.begin(), &*nodePoints.end());

	for (n = 0; n < segments.size(); ++n)
	if (!segments[n].erased)
		dsf.AddNetworkChainCmd(0,
				usedNodeIndex[segments[n].start],
				usedNodeIndex[segments[n].end],
				segments[n].kind,
				0xFFFFFFFF, // nodes[segments[n].start].index3,	// NO SHAPING TO
				0xFFFFFFFF, // nodes[segments[n].end].index3,	// OUR END POINTS!
				&*segments[n].shape.begin(),
				&*segments[n].shape.end(),
				&*segments[n].shapeCurve.begin(),
				&*segments[n].shapeCurve.end());

	// WRITE OUT CUSTOM OBJECTS

	for (n = 0; n < gObjects.size(); ++n)
	{
		string	objName = (gObjects[n].kind == kObstacleTypeCustom) ? gObjects[n].name : DefaultObjFromTypeAndHeight(gObjects[n].kind, gObjects[n].elevation);
		dsf.AddObjectCmd(customObjectIndex[objName],
						objLocationIndex[n],
						gObjects[n].elevation);
	} 
}


int	AccumPoint2(float lon, float lat, vector<int>& ioPts, map<INDEX_t, int>& ioIndex)
{
	INDEX_t	pindex = ENCODE_LL_INDEX(lon, lat);
	map<INDEX_t, int>::iterator i = ioIndex.find(pindex);
	if (i != ioIndex.end())
		return i->second;
	ioPts.push_back(ENCODE_LL_LON(lon));
	ioPts.push_back(ENCODE_LL_LAT(lat));
	ioIndex.insert(map<INDEX_t, int>::value_type(pindex, (ioPts.size()) / 2 - 1));
	return (ioPts.size()) / 2 - 1;
}

int	AccumPoint3(float lon, float lat, float alt, vector<int>& ioPts, map<INDEX_t, int>& ioIndex)
{
	INDEX_t	pindex = ENCODE_LLE_INDEX(lon, lat, alt);
	map<INDEX_t, int>::iterator i = ioIndex.find(pindex);
	if (i != ioIndex.end())
		return i->second;
	ioPts.push_back(ENCODE_LLE_LON(lon));
	ioPts.push_back(ENCODE_LLE_LAT(lat));
	ioPts.push_back(ENCODE_LLE_ALT(alt));
	ioIndex.insert(map<INDEX_t, int>::value_type(pindex, (ioPts.size()) / 3 - 1));
	return (ioPts.size()) / 3 - 1;
}

int	AccumPointST(float s, float t, vector<short>& ioPts, map<INDEX_t, int>& ioIndex)
{
	INDEX_t	pindex = ENCODE_ST_INDEX(s,t);
	map<INDEX_t, int>::iterator i = ioIndex.find(pindex);
	if (i != ioIndex.end())
		return i->second;
	ioPts.push_back(ENCODE_ST_S(s));
	ioPts.push_back(ENCODE_ST_T(t));
	ioIndex.insert(map<INDEX_t, int>::value_type(pindex, (ioPts.size()) / 2 - 1));
	return (ioPts.size()) / 2 - 1;
}

void	AccumSegment(	float lon1, float lat1, 
						float lon2, float lat2, int kind,	
						vector<NetworkNode_t>&		nodes,
						map<float, int>&			nodeIndex,
						vector<NetworkSeg_t>&		segments)
{
	NetworkSeg_t	seg;
	NetworkNode_t	newnode;

	// First either find our nodes, or create them if they
	// don't exist.

	float	pindex1 = lon1 + lat1 * 360.0;
	float	pindex2 = lon2 + lat2 * 360.0;
	
	map<float, int>::iterator iter1 = nodeIndex.find(pindex1);
	map<float, int>::iterator iter2 = nodeIndex.find(pindex2);
	if (iter1 == nodeIndex.end())
	{
		newnode.lat = lat1;
		newnode.lon = lon1;
		nodes.push_back(newnode);
		nodeIndex.insert(map<float, int>::value_type(pindex1, nodes.size()-1));
		seg.start = nodes.size() - 1;
	} else
		seg.start = iter1->second;

	if (iter2 == nodeIndex.end())
	{
		newnode.lat = lat2;
		newnode.lon = lon2;
		nodes.push_back(newnode);
		nodeIndex.insert(map<float, int>::value_type(pindex2, nodes.size()-1));
		seg.end = nodes.size() - 1;
	} else
		seg.end = iter1->second;

	// Create a new segment
	
	seg.kind = kind;
	seg.erased = false;
	segments.push_back(seg);

	// Reference this segment with our nodes.

	nodes[seg.start].segs.insert(segments.size() - 1);
	nodes[seg.end].segs.insert(segments.size() - 1);
}		

bool	CanMerge(		int nodeNum,
						int& s1, int& s2,
						vector<NetworkNode_t>&		nodes,
						vector<NetworkSeg_t>&		segments)
{
	if (nodes[nodeNum].segs.size() != 2) return false;
	set<int>::iterator i = nodes[nodeNum].segs.begin();
	
	s1 = *i;
	++i;
	s2 = *i;
	
	return segments[s1].kind == segments[s2].kind;
}						

void	MergeSegs(		int inSeg1, int inSeg2,
						vector<NetworkNode_t>&		nodes,
						vector<NetworkSeg_t>&		segments)
{		
	NetworkSeg_t&	seg1 = segments[inSeg1];
	NetworkSeg_t&	seg2 = segments[inSeg2];
	
	// One or both of our segments may be 'backward' in orientation!
	if (seg2.start != seg1.end)
	{
		if (seg2.end == seg1.start || seg2.end == seg1.end)
		{
			// flip seg 2
			swap(seg2.start, seg2.end);
			reverse(seg2.shapeLat.begin(), seg2.shapeLat.end());
			reverse(seg2.shapeLon.begin(), seg2.shapeLon.end());
		}
		
		
		if (seg1.start == seg2.start && seg1.end != seg2.start)
		{
			// flip seg1
			swap(seg1.start, seg1.end);
			reverse(seg1.shapeLat.begin(), seg1.shapeLat.end());
			reverse(seg1.shapeLon.begin(), seg1.shapeLon.end());
		}		
	}
	
	// Copy the shape geometry from 2 to 1, including the shared node.
	seg1.shapeLon.push_back(nodes[seg1.end].lon);
	seg1.shapeLat.push_back(nodes[seg1.end].lat);
	seg1.shapeLon.insert(seg1.shapeLon.end(), seg2.shapeLon.begin(), seg2.shapeLon.end());
	seg1.shapeLat.insert(seg1.shapeLat.end(), seg2.shapeLat.begin(), seg2.shapeLat.end());

	// Erase seg 2
	seg2.erased = true;

	// Remove seg2 from its nodes
	nodes[seg2.start].segs.erase(inSeg2);
	nodes[seg2.end].segs.erase(inSeg2);

	// Move seg1's end from seg1's end to seg2's end.
	nodes[seg1.end].segs.erase(inSeg1);
	nodes[seg2.end].segs.insert(inSeg1);
	seg1.end = seg2.end;
}

#endif