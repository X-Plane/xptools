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
#include "TIGERRead.h"
#include "AssertUtils.h"
#define XUTILS_EXCLUDE_MAC_CRAP 1
#include "XUtils.h"
#include "MemFileUtils.h"
#include "TigerImport.h"
#include "CompGeomUtils.h"

const double PI = 3.14159265358979323846;
const double PI2 = PI * 2.0;

// Normally we purge tiger string IDs to save memory right before we build the CGAL map
// because we can't afford both in RAM at once.  This flag leaves the TIGER info on so
// if we jump in with a debugger, we can get TIGER IDs and check the raw data.
#define 	TIGER_DEBUG_LEAVE_INFO	0

NodeInfoMap			gNodes;
ChainInfoMap		gChains;
LandmarkInfoMap		gLandmarks;
PolygonInfoMap		gPolygons;

inline	Point2 RawCoordToDouble(const RawCoordPair& p) { return Point2(atof(p.second.c_str()) / 1000000.0, atof(p.first.c_str()) / 1000000.0); }

static	int	gPassNum = 0;

// File loading routines.  Call them in the order they are listed in to
// get correct hashing!

void	TIGER_LoadRT1(MFFileSet * inSet, int inFileNumber)
{
	// Important: we basically tag each TLID with a unique num from the file it
	// came from.  This allows us to not load the shape points twice when loading
	// a pair of files.
	gPassNum++;

	char	fname[32];
	sprintf(fname, "TGR%05d.RT1", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{
				TLID		tlid;
				ChainInfo_t	ci;

				tlid = TextScanner_ExtractUnsignedLong(scanner, 5, 15);
				ci.tlid = tlid;
				ci.one_side = TextScanner_ExtractChar(scanner, 15) == 'Y';
				ci.owner = gPassNum;
				ci.reversed = 0;

				RawCoordPair	rawStart, rawEnd;

#if USE_STREET_NAMES
				TextScanner_ExtractString(scanner, 19, 49, ci.name, true);
#endif
				TextScanner_ExtractString(scanner, 55, 58, ci.cfcc, false);
				TextScanner_ExtractString(scanner, 190,200, rawStart.second, true);
				TextScanner_ExtractString(scanner, 200,209, rawStart.first, true);
				TextScanner_ExtractString(scanner, 209,219, rawEnd.second, true);
				TextScanner_ExtractString(scanner, 219,228, rawEnd.first, true);
				ci.start = rawStart.first + rawStart.second;
				ci.end   = rawEnd.first + rawEnd.second;
				ci.shape.push_back(RawCoordToDouble(rawStart));
				ci.shape.push_back(RawCoordToDouble(rawEnd));

				ChainInfoMap::iterator tlidP = gChains.find(tlid);
				if (tlidP == gChains.end())
					gChains.insert(ChainInfoMap::value_type(tlid, ci));
				else {
					// We need to note whether the other file has us reversed.  In one supremely nasty
					// case, we have a closed-loop TLID - in that case, who @#$@#ing knows whether it's reversed??
					if (ci.start == ci.end && tlidP->second.start == tlidP->second.end && tlidP->second.start == ci.start)
						tlidP->second.reversed = 2;
					else if (ci.start == tlidP->second.end && ci.end == tlidP->second.start)
						tlidP->second.reversed = 1;
					else if (ci.start == tlidP->second.start && ci.end == tlidP->second.end)
						tlidP->second.reversed = 0;
					else
						fprintf(stderr,"ERROR: TLID %d has different vertices in this file than the last.\n", tlid);
				}

				NodeInfo_t		node;
				if (gNodes.find(ci.start) == gNodes.end())
				{
					node.location = RawCoordToDouble(rawStart);
					gNodes.insert(NodeInfoMap::value_type(ci.start, node));
				}
				if (gNodes.find(ci.end) == gNodes.end())
				{
					node.location = RawCoordToDouble(rawEnd);
					gNodes.insert(NodeInfoMap::value_type(ci.end, node));
				}
			}

			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}

void	TIGER_LoadRT2(MFFileSet * inSet, int inFileNumber)
{
	char	fname[32];
	sprintf(fname, "TGR%05d.RT2", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{
				TLID		tlid;

				tlid = TextScanner_ExtractUnsignedLong(scanner, 5, 15);

				ChainInfo_t&	ci = gChains[tlid];
				if (ci.owner == gPassNum)
				{
					for (int n = 0; n < 10; ++n)
					{
						RawCoordPair	cp;
						TextScanner_ExtractString(scanner, 18 + 19 * n, 28 + 19 * n, cp.second, true);
						TextScanner_ExtractString(scanner, 28 + 19 * n, 37 + 19 * n, cp.first, true);

						if ((!cp.first.empty()) &&
							(!cp.second.empty()) &&
							(atoi(cp.first.c_str()) != 0) &&
							(atoi(cp.second.c_str()) != 0))
						{
							// IMPORTANT: Little did I realize that the tiger-line data has shape points
							// on chains that are NOT distinct from either adjacent shape points or the end
							// points.  So eliminate these shape points, so that we don't have zero-lengthed
							// halfedges.
							Point2	p(RawCoordToDouble(cp));
							if (p != ci.shape[ci.shape.size()-2])
								ci.shape.insert(ci.shape.end()-1, p);
						}
					}
				}
			}

			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}

void	TIGER_LoadRTP(MFFileSet * inSet, int inFileNumber)
{
	char	fname[32];
	sprintf(fname, "TGR%05d.RTP", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{

				CENID_POLYID	id;
				PolygonInfo_t	info;
				info.isWorld = false;
				RawCoordPair	rawLoc;
				TextScanner_ExtractString(scanner, 10, 25, id, true);
				TextScanner_ExtractString(scanner, 35, 44, rawLoc.first, true);
				TextScanner_ExtractString(scanner, 25, 35, rawLoc.second, true);
				info.water = (TextScanner_ExtractChar(scanner, 44) == '1') ? 1 : 0;
				info.location = RawCoordToDouble(rawLoc);
				gPolygons.insert(PolygonInfoMap::value_type(id, info));
			}
			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}

void	TIGER_LoadRTI(MFFileSet * inSet, int inFileNumber)
{
	char	fname[32];
	sprintf(fname, "TGR%05d.RTI", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{
				CENID_POLYID	rid, lid;
				TLID			tlid;

				TextScanner_ExtractString(scanner, 40, 55, lid, true);
				TextScanner_ExtractString(scanner, 55, 70, rid, true);
				tlid = TextScanner_ExtractUnsignedLong(scanner, 10, 20);

				ChainInfoMap::iterator citer = gChains.find(tlid);
				if (citer == gChains.end())
					fprintf(stderr,"ERROR: unknown TLID %d for polys %s, %s\n", tlid, lid.c_str(), rid.c_str());
				else if (citer->second.reversed == 1)
				{
					swap(rid, lid);
				}
				else if (citer->second.reversed == 2)
				{
					// Tough case - we don't know if this TLID is reversed because the start and end nodes are the same.
					// Well, we must be the second file to have this TLID, so...see which side is filled in.
#if DEV
					if (lid.empty() && rid.empty())
						fprintf(stderr, "ASSERTION failure - no poly for either side of antenna.\n");
					if (!lid.empty() && !rid.empty())
						fprintf(stderr, "ASSERTION failure - we should have at least one empty side if we are shared.\n");
					if (citer->second.lpoly.empty() && citer->second.rpoly.empty())
						fprintf(stderr, "ASSERTION failure - no polygon on either side of the TLID from the last file.\n");
					if (!citer->second.lpoly.empty() && !citer->second.rpoly.empty())
						fprintf(stderr, "ASSERTION failure - no free slot on either side of the TLID from the last file.\n");
#endif
					// If we have no left poly, but we have an empty slot on the TLID already we must be reversed.
					if (lid.empty() && citer->second.lpoly.empty())
						swap(rid,lid);
				}

				if (!lid.empty())
				{
					PolygonInfoMap::iterator liter = gPolygons.find(lid);
					if (liter != gPolygons.end())
					{
						liter->second.border.insert(tlid);
					}
				}

				if (!rid.empty())
				{
					PolygonInfoMap::iterator riter = gPolygons.find(rid);
					if (riter != gPolygons.end())
					{
						riter->second.border.insert(tlid);
					}
				}

				if (citer != gChains.end())
				{
					// Only set the poly IDs for the chain if its
					// not the outside.  This way we will merge adjacent files.
					if (!lid.empty())
					{
						if (!citer->second.lpoly.empty())
							fprintf(stderr,"ERROR: TLID %d has lpoly claimed by %s and %s\n",
								tlid, lid.c_str(), citer->second.lpoly.c_str());
						citer->second.lpoly = lid;
					}
					if (!rid.empty())
					{
						if (!citer->second.rpoly.empty())
							fprintf(stderr,"ERROR: TLID %d has rpoly claimed by %s and %s\n",
								tlid, rid.c_str(), citer->second.rpoly.c_str());
						citer->second.rpoly = rid;
					}
				}
			}
			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}


void	TIGER_LoadRT7(MFFileSet * inSet, int inFileNumber)
{
	char	fname[32];
	sprintf(fname, "TGR%05d.RT7", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{
				LAND			landID;
				LandmarkInfo_t	info_t;

				landID = TextScanner_ExtractUnsignedLong(scanner, 10, 20);
				landID = landID * 100000 + inFileNumber;

				RawCoordPair	rawLoc;
				TextScanner_ExtractString(scanner, 21, 24, info_t.cfcc, false);
#if USE_LANDMARK_NAMES
				TextScanner_ExtractString(scanner, 24, 54, info_t.name, true);
#endif
				TextScanner_ExtractString(scanner, 54, 64, rawLoc.second, true);
				TextScanner_ExtractString(scanner, 64, 73, rawLoc.first, true);
				info_t.location = RawCoordToDouble(rawLoc);
				if (gLandmarks.find(landID) != gLandmarks.end())
				{
					if (gLandmarks[landID].cfcc != info_t.cfcc)
						fprintf(stderr,"ERROR: Landmark type conflict.\n");
				}
				gLandmarks.insert(LandmarkInfoMap::value_type(landID, info_t));
			}
			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}

void	TIGER_LoadRT8(MFFileSet * inSet, int inFileNumber)
{
	char	fname[32];
	sprintf(fname, "TGR%05d.RT8", inFileNumber);
	MFMemFile *	file = FileSet_OpenSpecific(inSet, fname);
	if (file)
	{
		MFTextScanner * scanner = TextScanner_Open(file);
		while (!TextScanner_IsDone(scanner))
		{
			if (TextScanner_GetBegin(scanner) != TextScanner_GetEnd(scanner))
			{
				LAND			landID;

				landID = TextScanner_ExtractUnsignedLong(scanner, 25, 35);
				landID = landID * 100000 + inFileNumber;

				LandmarkInfo_t&	info = gLandmarks[landID];
				CENID_POLYID id;
				TextScanner_ExtractString(scanner, 10, 25, id, true);
				info.cenid_polyid.push_back(id);
			}

			TextScanner_Next(scanner);
		}

		TextScanner_Close(scanner);
		MemFile_Close(file);
	}
}


bool	SegmentOutOfBounds(const Point2& start, const Point2& end,
						double inWest, double inSouth, double inEast, double inNorth)
{
	double	slon = start.x();
	double	slat = start.y();
	double	elon = end.x();
	double	elat = end.y();
	
	if (slon < inWest && elon < inWest) return true;
	if (slon > inEast && elon > inEast) return true;
	if (slat < inSouth && elat < inSouth) return true;
	if (slat > inNorth && elat > inNorth) return true;
	return false;
}

bool	ChainOutOfBounds(const ChainInfo_t& inChain,
						double inWest, double inSouth, double inEast, double inNorth)
{
	for (int i = 1; i < inChain.shape.size(); ++i)
		if (!SegmentOutOfBounds(inChain.shape[i-1], inChain.shape[i], inWest, inSouth, inEast, inNorth))
			return false;
	return true;
}

bool	ChainIsDegenerate(const ChainInfo_t& inChain)
{
	return (inChain.shape.size() == 2) && inChain.start == inChain.end;
}

void	TIGER_EliminateZeroLengthShapePoints(void)
{
	int	nuke = 0;
	for (ChainInfoMap::iterator chain = gChains.begin(); chain != gChains.end(); ++chain)
	{
		for (int n = 1; n < (chain->second.shape.size()-1); ++n)
		{
			if (chain->second.shape[n] == chain->second.shape[n-1])
			{
				chain->second.shape.erase(chain->second.shape.begin()+n);
				--n;
				++nuke;
			}
		}

		if (chain->second.shape.size() > 2 &&
			chain->second.shape[chain->second.shape.size()-1] == chain->second.shape[chain->second.shape.size()-2])
		{
			chain->second.shape.erase(chain->second.shape.begin()+chain->second.shape.size()-2);
			++nuke;
		}

	}
	printf("Deleted %d shape points.\n", nuke);
}

void	TIGER_RoughCull(double inWest, double inSouth, double inEast, double inNorth)
{
	printf("Start of rough cull: %d TLIDs, %d polygons\n", gChains.size(), gPolygons.size());

	/*******************************************************************************************
	 * ROUGH CULL
	 *******************************************************************************************
	 *
	 * This eliminates polygons that are entirely outside of the cull area and are not inside a
	 * polygon that strattles the cull area.  We have to do this via a flood-fill from the outside;
	 * we cannot discover these relationships absolutely because we have no topological data yet -
	 * we have no idea who is a hole, etc.
	 *
	 */

		map<CENID_POLYID, PolygonInfo_t *>		workingPolys;
		map<TLID, ChainInfo_t *>				liberatedChains;
		int										degen = 0;
		CENID_POLYID							poly_id;
		PolygonInfo_t *							poly;
		ChainInfo_t *							chain;
		ChainInfoMap::iterator					chainIter;
		PolygonInfoMap::iterator 				polyIter;
		TLIDSet::iterator			 			tlidIter;

		bool									has_world;
		bool									has_inside;


	/* STEP 1 - MARK ALL CHAINS THAT ARE OUT OF BOUNDS
	 *
	 * Do this once up front and cache the results. */

	for (chainIter = gChains.begin(); chainIter != gChains.end(); ++chainIter)
	{
		chainIter->second.kill = ChainOutOfBounds(chainIter->second, inWest, inSouth, inEast, inNorth);
	}

	/* STEP 2 - FIND THE INITIAL SET OF DEAD POLYGONS
	 *
	 * This is the set of polygons whose entire borders are
	 * eitiher out of bounds or the world poly.  We KNOW
	 * these are dead.  We must have at least one world-poly! */

	for (polyIter = gPolygons.begin(); polyIter != gPolygons.end(); ++polyIter)
	{
		has_world = false;
		has_inside = false;
		for (tlidIter = polyIter->second.border.begin(); tlidIter != polyIter->second.border.end(); ++tlidIter)
		{
			chain = &gChains[*tlidIter];
			if (!chain->kill)
			{
				has_inside = true;
				break;
			}
			if (!has_world && chain->lpoly.empty() || chain->rpoly.empty())
				has_world = true;
		}

		if (has_world && !has_inside)
			workingPolys.insert(map<CENID_POLYID, PolygonInfo_t *>::value_type(polyIter->first, &polyIter->second));
	}

	while (!workingPolys.empty())
	{
		/* STEP 3 - PER POLYGON PROCESSING
		 *
		 * Go through and detach ourselves from our TLIDs.  Any TLIDs
		 * that now have NO faces are simply dead.  Any TLIDs that
		 * have ONE face instead of two now can possibly be culled.
		 * Remove this polgon from the working set.
		 *
		 */

		liberatedChains.clear();
		poly_id = workingPolys.begin()->first;
		poly = workingPolys.begin()->second;

		for (tlidIter = poly->border.begin(); tlidIter != poly->border.end(); ++tlidIter)
		{
			chain = &gChains[*tlidIter];

			if (chain->lpoly == poly_id)
				chain->lpoly.clear();
			if (chain->rpoly == poly_id)
				chain->rpoly.clear();

			if (chain->lpoly.empty() && chain->rpoly.empty())
			{
				gChains.erase(*tlidIter);
				liberatedChains.erase(*tlidIter);
			}
			else if (chain->lpoly.empty() || chain->rpoly.empty())
				liberatedChains.insert(map<TLID,ChainInfo_t *>::value_type(*tlidIter, chain));
		}

		workingPolys.erase(poly_id);
		gPolygons.erase(poly_id);

		/* STEP 4 - RECURSE
		 *
		 * For each TLID that we exposed, examine the polygon it's attached
		 * to.  Again the polygon must pass the test of all out of bounds,
		 * and at least one world-poly. */

		 	set<CENID_POLYID>	possible;

		 for (map<TLID, ChainInfo_t *>::iterator lib = liberatedChains.begin(); lib != liberatedChains.end(); ++lib)
		 {
		 	if (!lib->second->lpoly.empty())
		 		possible.insert(lib->second->lpoly);
		 	if (!lib->second->rpoly.empty())
		 		possible.insert(lib->second->rpoly);
		 }

		 for (set<CENID_POLYID>::iterator posIter = possible.begin(); posIter != possible.end(); ++posIter)
		 {
		 	poly = &gPolygons[*posIter];
			has_world = false;
			has_inside = false;

			for (tlidIter = poly->border.begin(); tlidIter != poly->border.end(); ++tlidIter)
			{
				chain = &gChains[*tlidIter];
				if (!chain->kill)
				{
					has_inside = true;
					break;
				}
				if (!has_world && chain->lpoly.empty() || chain->rpoly.empty())
					has_world = true;
			}

			if (has_world && !has_inside)
				workingPolys.insert(map<CENID_POLYID, PolygonInfo_t *>::value_type(*posIter, poly));

		 }
	}


	/*******************************************************************************************
	 * DEGENERACY
	 *******************************************************************************************
	 *
	 * For some reason there are chains in the tiger data that are effectively zero-length, e.g.
	 * they have the same start and end and no shape points.  Nuke them now - they're not really
	 * needed.  We don't need to worry about border - it won't be used again.
	 *
	 */

	set<TLID>							deadEdges;


	for (ChainInfoMap::iterator chainIter = gChains.begin(); chainIter != gChains.end(); ++chainIter)
	if (ChainIsDegenerate(chainIter->second))
	{
#if DEV
		// Note: degenerate TLIDs are not really a problem - we just don't include them in the boundaries - after all
		// they aren't topologically necessary IF we are spatially valid.  However, in dev mode it's nice to know they're
		// there so we can see what array of problems are in one file.
		fprintf(stderr,"Warning: Degenerate TLID ID %d\n", chainIter->first);
#endif
		deadEdges.insert(chainIter->first);
		++degen;
	}

	// Ok nuke city
	for (set<TLID>::iterator tlid = deadEdges.begin(); tlid != deadEdges.end(); ++tlid)
		gChains.erase(*tlid);

	printf("End of rough cull: %d TLIDs, %d polygons, %d chains were degenerate\n", gChains.size(), gPolygons.size(), degen);
}

void	TIGER_PostProcess(Pmwx& ioMap)
{
	/******* PART 1 - TRANSLATE KEYS TO LINKS AND BUILD TABLES *********/
		WTPM_NodeVector		nodes;
		WTPM_LineVector		lines;
		WTPM_FaceVector		faces;
		PolygonInfo_t		worldPoly;

	// First build the world polygon
	worldPoly.isWorld = true;
	worldPoly.water = 1;
	gPolygons.insert(PolygonInfoMap::value_type(WORLD_POLY, worldPoly));

	// Do per-per-chain processing.
	for (ChainInfoMap::iterator chainIter = gChains.begin(); chainIter != gChains.end(); ++chainIter)
	{
		// Mark any unknown polygons as the world polygon.
		if (chainIter->second.lpoly.empty())
			chainIter->second.lpoly = WORLD_POLY;
		if (chainIter->second.rpoly.empty())
			chainIter->second.rpoly = WORLD_POLY;

		// Build links
		PolygonInfoMap::iterator leftIter = gPolygons.find(chainIter->second.lpoly);
		if (leftIter == gPolygons.end())
			fprintf(stderr,"ASSERTION FAILED: Unknown left polygon %s for chain %d\n", chainIter->second.lpoly.c_str(), chainIter->first);
		chainIter->second.leftFace = &leftIter->second;

		PolygonInfoMap::iterator rightIter = gPolygons.find(chainIter->second.rpoly);
		if (rightIter == gPolygons.end())
			fprintf(stderr,"ASSERTION FAILED: Unknown right polygon %s for chain %d\n", chainIter->second.rpoly.c_str(), chainIter->first);
		chainIter->second.rightFace = &rightIter->second;

		NodeInfoMap::iterator startIter = gNodes.find(chainIter->second.start);
		if (startIter == gNodes.end())
			fprintf(stderr,"ASSERTON FAILED: Unknown start node %sfor chain %d\n", chainIter->second.start.c_str(), chainIter->first);
		chainIter->second.startNode = &startIter->second;

		NodeInfoMap::iterator endIter = gNodes.find(chainIter->second.end);
		if (endIter == gNodes.end())
			fprintf(stderr,"ASSERTON FAILED: Unknown end node %sfor chain %d\n", chainIter->second.end.c_str(), chainIter->first);
		chainIter->second.endNode = &endIter->second;

		lines.push_back(&chainIter->second);

#if !TIGER_DEBUG_LEAVE_INFO
		chainIter->second.lpoly.clear();
		chainIter->second.rpoly.clear();
		chainIter->second.start.clear();
		chainIter->second.end.clear();
#endif
	}

	for (PolygonInfoMap::iterator polyIter = gPolygons.begin(); polyIter != gPolygons.end(); ++polyIter)
	{
		faces.push_back(&polyIter->second);
	}

	/***** BUILD TOPOLOGY ***********/

	WTPM_CreateBackLinks(lines);

	for (PolygonInfoMap::iterator polyIter = gPolygons.begin(); polyIter != gPolygons.end(); ++polyIter)
	{
#if !TIGER_DEBUG_LEAVE_INFO
		polyIter->second.border.clear();
#endif
	}

	// Now that we've back-linked, cull nodes.  If some halfedges were removed, we'll have stale nodes.
	// Collect them, then destory them.

		set<RawCoordKey>	nuke;

	for (NodeInfoMap::iterator nodeIter = gNodes.begin(); nodeIter != gNodes.end(); ++nodeIter)
	{
		if (nodeIter->second.lines.empty())
			nuke.insert(nodeIter->first);
	}
	for (set<RawCoordKey>::iterator nukeIter = nuke.begin(); nukeIter != nuke.end(); ++nukeIter)
		gNodes.erase(*nukeIter);

	// Now we build the node table, with only valid nodes.
	for (NodeInfoMap::iterator nodeIter = gNodes.begin(); nodeIter != gNodes.end(); ++nodeIter)
		nodes.push_back(&nodeIter->second);

	WTPM_RestoreTopology(nodes, lines, faces);
	if (gWTPMErrors)
		fprintf(stderr,"ERROR: there are spatial/topological problems with this file!!!\n");

	WTPM_ExportToMap(nodes, lines, faces, ioMap);
}
