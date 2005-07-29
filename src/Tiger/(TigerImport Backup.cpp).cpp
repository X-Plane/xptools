#include "TigerImport.h"
#include "ParamDefs.h"
#include "PerfUtils.h"

/*

	CLEAN IMPORT CONCEPTS:
	
	1. Make sure that when we merge two census databases that we:
Ã		- when we see a rerun TLID, count shape pts for safety.
Ã		- when we see a rerun tlid, merge any right/left data from record I
Ã	1a. Make sure that antenna TLIDs are in polygons twice.

	2a. When we're done with all TIGER importing, we also need to collect every TLID that is unbounded on one side and build a GT-polygon
		for the boundary of the whole map.  Do this by inventing a virtual 'world' GT polygon.	Set the unused sides of the edges to 'world'
		and add each of these edges to the list of edges for the world gt polygon.

	2. For every Gt-polygon, sort the perimeter to form a counter-clockwise border based on the TLID direction.
	
		Build an array with each TLID in it.  For TLIDs that have our polyID on _both_ sides we should have it in this array twice.
		index it twice, once by from, once by to.
		
		Find the lower left most segment, we need this to figure out which the hell way is counter clockwise!
			The segment with at least one vertex on the left edge of the polygon is our vertex of choice.
			If we have two vertical segments, the one with the lowest point wins.
			if we have a vertical and non-vertical segment, the vertical segment is good enough.
			if we have two non-vertical segments, pick one that is lower.
			
			then...if the segment goes down or to the right, it is directed around the CCW boundary. If it goes up or left it is
			backwards!!		
		
		from this segment, find the most left turning next segment
			remove this segment, make it current, repeat
			
		when we run out of cases, if we didn't hit our start, we're screwed!
		
		Note: when we pick the left most, we should go by the first segment in the TLID.
		
		The order that we picked the segments is the CCW order.  If we had to link to the target
		and not the source of the vector, it needs to go in backward.
		
	
	3. The import algorithm then goes something like this:
	
		1. For every TLID + shape point in the DB, create a vertex.  Create an index from the tiger strings to the vertices.
			the vertex gets assigned the pt's equivalent point_2.
		2. for every TLID:
			create 2*N half-edges, where N = the number of segments in the TLID counting shape points.
			set each adjacent one to point to the opposite.
			assign each segment along the shape to an edge or its opposite.
			assign each segment's vertex target to the target vertex based on the hash table.
			assign each segment's next to be the next segment along the TLID.  Now only the last halfedge in the TLID
			on the left side and last on the right side (not opposites of each other!!) don't have next's set up.  no one has faces.
			Store this mess in a a hash table from TLID to pair of vector of half-edges, such that each vector runs the half-edges
			first along the left, then the right side of the TLID.
		3. For each GT-Polygon
			Crete a new face.
			Create a map from source vertex to pair of first and last half-edge for the TLID.
			For each TLID around the GT polygon
				fill out the table, with the source vertex for the first half edge and a pair of the first and last half edge.
				set every part of the half edges for this face to point to the face.
			now for each TLID in the table
				set its second half-edges "next" to the first half edge looked up via its target through the table.
			
*/


	/*			
			TODO: ADD COUNTERS OF ALL PHENOMENA
			TODO: build major catagories of land classes....
	*/


/*
 Census Feature Classification Codes - areas.  These codes are applied to points
 or areas and either result in a land class (basically a zoning decision that will
 autogen buildings later) or a point object.  The 'allow_from_point' field allows
 a single point object to apply a land class to the underlying polygon.  This is 
 appropriate for big things where the presence of the object would dominate the 
 block.  It is not appropriate for small things that might just be one single building
 instance in a land use.
 
 */
#define DO_CHECKS 0

struct	AreaInfo_t {
	const char *		cfcc;
	int					land_class;
	int					allow_from_point;
};

const AreaInfo_t	kAreaCodes[] = {
	"D10",				lc_MilitaryBase,		1,	
	"D20",				lc_TransientQuarters,	1,
	"D21",				lc_ApartmentComplex,	1,
	"D23",				lc_TrailerPark,			1,
	"D24",				lc_Marina,				1,
	"D26",				lc_WorkerHousing,		1,
	"D27",				lc_Hotel,				0,
	"D28",				lc_CampGround,			1,
	"D29",				lc_ShelterMission,		0,

	"D30",				lc_CustodialFacility,	1,
	"D31",				lc_Hospital,			1,
	"D32",				lc_TransientQuarters,	0,
	"D33",				lc_NursingHome,			1,
	"D34",				lc_PoorFarm,			1,
	"D35",				lc_Orphanage,			1,
	"D36",				lc_Jail,				0,	// Only promote federal jails to whole area
	"D37",				lc_Jail,				1,

	"D40",				lc_EducationalInstitution,	1,
	"D42",				lc_Monastary,				0,
	"D43",				lc_EducationalInstitution,	1,
	"D44",				lc_ReligiousInstitution,	0,
	
	"D50",				lc_TransportationTerminal,	0,
	"D51",				lc_Airport,					1,
	"D52",				lc_TrainStation,			0,
	"D53",				lc_BusTerminal,				0,
	"D54",				lc_MarineTerminal,			1,
	"D55",				lc_SeaplanePort,			1,
	"D57",				lc_Airport,					1,

	"D60",				lc_EmploymentCenter,		1,
	"D61",				lc_ShoppingCenter,			1,
	"D62",				lc_IndustrialPark,			1,
	"D63",				lc_OfficeBuilding,			0,
	"D64",				lc_AmusementCenter,			1,
	"D65",				lc_GovernmentCenter,		0,
	"D66",				lc_EmploymentCenter,		1,

	"D80",				lc_OpenSpace,				1,
	"D81",				lc_GolfCourse,				1,
	"D82",				lc_Cemetary,				1,
	"D83",				lc_Park,					1,
	"D84",				lc_NationalForest,			1,
	"D85",				lc_Park,					1,
	
	"D91",				lc_PostOffice,				0,
	
	"E23",				lc_GenericLand,				0,
	
	"H00",				lc_GenericWater,			1,
	"H11",				lc_Stream,					1,
	"H12",				lc_Stream,					1,
	"H13",				lc_Stream,					1,
	"H21",				lc_Canal,					1,
	"H22",				lc_Canal,					1,
	"H31",				lc_Pond,					1,
	"H32",				lc_Pond,					1,
	"H41",				lc_Resevoir,				1,
	"H41",				lc_Resevoir,				1,
	"H50",				lc_Bay,						1,
	"H51",				lc_Bay,						1,
	"H53",				lc_Ocean,					1,
	"H60",				lc_FilledQuaryPit,			1,
	"H80",				lc_GenericWater,			1,
	"H81",				lc_Glacier,					1,

	NULL, 0, 0
};

struct	RoadInfo_t {
	const char *		cfcc;
	int					network_type;
	int					underpassing;
};

const RoadInfo_t	kRoadCodes[] = {
	"A11",	road_PrimaryLimUnsep,		0,
	"A13",	road_PrimaryLimUnsep,		1,
	"A14",	road_PrimaryLimUnsepRail,	0,
	
	"A15",	road_PrimaryLimSep,			0,
	"A17",	road_PrimaryLimSep,			1,
	"A18",	road_PrimaryLimSepRail,		0,
	
	"A21",	road_PrimaryUnsep,			0,
	"A23",	road_PrimaryUnsep,			1,
	"A24",	road_PrimaryUnsepRail,		0,
	"A25",	road_PrimarySep,			0,
	"A27",	road_PrimarySep,			1,
	"A28",	road_PrimarySepRail,		0,
	
	"A31",	road_SecondUnsep,			0,
	"A33",	road_PrimarySep,			1,
	"A34",	road_SecondUnsepRail,		0,
	"A35",	road_SecondSep,				0,
	"A37",	road_SecondSep,				1,
	"A38",	road_SecondSepRail,			0,
	
	"A41",	road_LocalUnsep,			0,
	"A43",	road_LocalUnsep,			1,
	"A44",	road_LocalUnsepRail,		0,
	"A45",	road_LocalSep,				0,
	"A47",	road_LocalSep,				1,
	"A48",	road_LocalSepRail,			0,
	
	"A51",	road_4WDUnsep,				0,
	"A53",	road_4WDUnsep,				1,
	
	"A60",	road_Unknown,				0,
	"A61",	road_Culdesac,				0,
	"A62",	road_TrafficCircle,			0,
	"A63",	road_Ramp,					0,
	"A64",	road_Service,				0,	// Should we set some of these underpassings?!?
	
	"A70",	walk_Unknown,				0,
	"A71",	walk_Trail,					0,
	"A72",	walk_Stairway,				0,
	"A73",	road_Alley,					0,
	"A74",	road_Driveway,				0,
	NULL, 0, 0
};	

int	LookupAreaCFCC(const char * inCode)
{
	int n = 0;
	while (kAreaCodes[n].cfcc)
	{
		if (!strcmp(kAreaCodes[n].cfcc, inCode)) return n;
		++n;
	}
	return -1;
}

int LookupNetCFCC(const char * inCode)
{
	int n = 0;
	while(kRoadCodes[n].cfcc)
	{
		if (!strcmp(kRoadCodes[n].cfcc, inCode)) return n;
		++n;
	}
	return -1;
}
	
inline	string	RawCoordToKey(const RawCoordPair& p) { return p.first + p.second; }
inline	Point_2 RawCoordToCoord(const RawCoordPair& p) { return Point_2(atof(p.second.c_str())/* / 1000000.0*/, atof(p.first.c_str())/* / 1000000.0*/); }

;
typedef	MAP_TYPE<TLID, Pmwx::Halfedge_handle>		EdgeIndex;
typedef	MAP_TYPE<CENID_POLYID, Pmwx::Face_handle>	FaceIndex;


PerfTimer	zeroV("In face Insertion");
PerfTimer	oneV("One Vertex Insertion");
PerfTimer	twoV("Two Vertex Insertion");


Pmwx::Halfedge_handle	InsertOneSegment(
							const RawCoordPair& p1, 
							const RawCoordPair& p2,
							VertexIndex&		index,
							Pmwx&				ioMap)
{
	string	key1 = RawCoordToKey(p1);
	Point_2	pt1  = RawCoordToCoord(p1);
	string	key2 = RawCoordToKey(p2);
	Point_2	pt2  = RawCoordToCoord(p2);

	VertexIndex::iterator i1 = index.find(key1);
	VertexIndex::iterator i2 = index.find(key2);
		
	Pmwx::Halfedge_handle	he = Pmwx::Halfedge_handle();

#if 0	
	Pmwx::Locate_type loc1, loc2;
	ioMap.locate(pt1, loc1);
	ioMap.locate(pt2, loc2);
	CGAL_precondition_msg(loc1 != Pmwx::EDGE, "Pt1 on an edge, will cause CHAOS");
	CGAL_precondition_msg(loc2 != Pmwx::EDGE, "Pt2 on an edge, will cause CHAOS");
	if (i1 == index.end())
		CGAL_precondition_msg(loc1 != Pmwx::VERTEX, "Pt1 on an unindexed vertex, will cause CHAOS");
	if (i2 == index.end())
		CGAL_precondition_msg(loc2 != Pmwx::VERTEX, "Pt2 on an unindexed vertex, will cause CHAOS");
#endif

	if (i1 == index.end())
	{
		if (i2 == index.end())
		{
			// Totally unknown segment.
			Pmwx::Locate_type lt;
			zeroV.Start();
			he = ioMap.locate(pt1, lt);
			CGAL_precondition_msg(lt == Pmwx::FACE || lt == Pmwx::UNBOUNDED_FACE, "Inserting a segment in unknown territory but it's NOT on a face!!");
			Pmwx::Face_handle	fe = (lt == Pmwx::UNBOUNDED_FACE) ? ioMap.unbounded_face() : he->face();
//			he = ioMap.non_intersecting_insert(PM_Curve_2(pt1, pt2));
			he = ioMap.insert_in_face_interior(PM_Curve_2(pt1, pt2), fe);
			zeroV.Stop();
			if (he != Pmwx::Halfedge_handle())
			{
				index[key1] = he->source();
				index[key2] = he->target();			
			}
		} else {
			// We know pt 2 but pt 1 is floating.  Make a vector
			// using the vertex handle from 2 and 1's raw value.
			oneV.Start();
			he = ioMap.Planar_map_2::insert_from_vertex(
					PM_Curve_2(i2->second->point(), pt1),
					i2->second);
			oneV.Stop();
			// Now pt 1 gets stored...it is the target of the new halfedge.
			if (he != Pmwx::Halfedge_handle())
			{
				index[key1] = he->target();
				he = he->twin();	// This halfedge goes from 2 to 1, turn it around!
			}
		}
	} else {
		if (i2 == index.end())
		{
			oneV.Start();
			// We know pt 1 but not pt 2
			he = ioMap.Planar_map_2::insert_from_vertex(
					PM_Curve_2(i1->second->point(), pt2),
					i1->second);
			oneV.Stop();
			// Now pt 1 gets stored...it is the target of the new halfedge.
			if (he != Pmwx::Halfedge_handle())
				index[key2] = he->target();
		} else {
			twoV.Start();
			// Both pts are known
				he = ioMap.Planar_map_2::insert_at_vertices(
					PM_Curve_2(i1->second->point(), i2->second->point()),
					i1->second,
					i2->second);
			twoV.Stop();
		}
	}

	if (he == Pmwx::Halfedge_handle())
	{
		return ioMap.halfedges_end();
	}

	// Whenever we create a half edge we have to pick dominance...this works.
	he->mDominant = true;
	return he;
}					

void	TIGERImport(
			const	ChainInfoMap&		chains,
			const	LandmarkInfoMap&	landmarks,
			const	PolygonInfoMap&		polygons,
			Pmwx&						outMap)
{
	// Our planar map MUST be empty!
	
	// First we go in and insert every segment from the TIGER database into our map.
	// We keep a table from coordinates into the map so we can avoid doing a search
	// when we have a segment that is already at least partially inserted.
	
	VertexIndex		vertices;
	EdgeIndex		edges;
	FaceIndex		faces;
	
	set<TLID>		badTLIDs;
	
	int	gSegs = 0, gBad = 0, gDupes = 0;
	set<TLID>	tlids;
#if DO_CHECKS	
	map<string, TLID>	lines;
#endif
	
	for (ChainInfoMap::const_iterator chain = chains.begin(); chain != chains.end(); ++chain)
	{
		int i = LookupNetCFCC(chain->second.cfcc.c_str());
		
		vector<RawCoordPair>	pts = chain->second.shape;
		pts.insert(pts.begin(), chain->second.start);
		pts.insert(pts.end(), chain->second.end);

		if (tlids.find(chain->first) != tlids.end())
		{
//			printf("WARNING: about to dupe a TLID!\n");
			continue;
		}
		tlids.insert(chain->first);
		
		for (int n = 1; n < pts.size(); ++n)
		{
			++gSegs;

#if DO_CHECKS	
			string	masterkey, k1 = RawCoordToKey(pts[n-1]), k2 = RawCoordToKey(pts[n]);
			if (k1 < k2)
				masterkey = k1 + k2;
			else
				masterkey = k2 + k1;
			
			map<string, TLID>::iterator tlidCheck = lines.find(masterkey);
			if (tlidCheck != lines.end())
			{
				printf("WARNING: already did this seg (by key), old TLID = %ul, new TLID = %ul!\n", tlidCheck->second, chain->first);
				printf("Sequence in question is: %s\n", tlidCheck->first.c_str());
				continue;
			}
			lines.insert(map<string, TLID>::value_type(masterkey, chain->first));
#endif

			try {
		
				Pmwx::Halfedge_handle he = InsertOneSegment(pts[n-1], pts[n], vertices, outMap);
				if (he != outMap.halfedges_end())
				{				
					// InsertOneSegment always returns the dominant half-edge.  Tag it with
					// our road type, underpassing info, and our TLID.
					if (i != -1)
					{
						GISNetworkSegment_t nl;
						nl.type = kRoadCodes[i].network_type;
						he->mSegments.push_back(nl);
						he->mParams[gis_TIGER_IsUnderpassing] = kRoadCodes[i].underpassing;
					}
					he->mParams[gis_TIGER_TLID] = chain->first;
					he->twin()->mParams[gis_TIGER_TLID] = chain->first;
					edges[chain->first] = he;
				} else {
					printf("Got dupe seg, CFCC = %s, name = %s, tlid = %d\n", chain->second.cfcc.c_str(), chain->second.name.c_str(), chain->first);
					++gDupes;
				}
				
			} catch (...) {
				++gBad;
				printf("Got bad seg, CFCC = %s, name = %s, tlid = %d\n", chain->second.cfcc.c_str(), chain->second.name.c_str(), chain->first);
				badTLIDs.insert(chain->first);
			}

			if ((gSegs % 10000) == 0)
			{
				fprintf(stdout, ".");
				fflush(stdout);
			}
		}
	}
	std::cout << "\nTotal " << gSegs << " bad " << gBad << " dupes " << gDupes << "\n";

	double	elapsed;
	unsigned long	calls;
	double	ave;

	zeroV.GetStats(elapsed, calls);
	ave = elapsed / (double) calls;
	printf("In-face insertion: %f total, %d calls, %f average.\n", elapsed, calls, ave);

	oneV.GetStats(elapsed, calls);
	ave = elapsed / (double) calls;
	printf("One-V Insertion: %f total, %d calls, %f average.\n", elapsed, calls, ave);

	twoV.GetStats(elapsed, calls);
	ave = elapsed / (double) calls;
	printf("Two-V insertion: %f total, %d calls, %f average.\n", elapsed, calls, ave);
	
	
	// Now we go in and apply our polygon data.  We have set the dominant flag to be the halfedge 
	// that goes in the same direction as the tiger database.  Since CGAL faces have CCW outer 
	// boundaries, that means that the left hand poly of a TLID is adjacent to the dominant 
	// halfedge.  
	
	
	
	int	gPolys = 0, gMissingTLIDs = 0, gBadEdges = 0, gBadBackLink = 0, gDeadTLID = 0;
	
	for (PolygonInfoMap::const_iterator poly = polygons.begin(); poly != polygons.end(); ++poly)
	{
		if (poly->first == WORLD_POLY)
			continue;
		++gPolys;
		set<TLID>		ourTLIDs;
		for (DirectedTLIDVector::const_iterator t = poly->second.border.begin(); t != poly->second.border.end(); ++t)
			ourTLIDs.insert(t->first);
		vector<TLID>	ourBads;
		set_intersection(ourTLIDs.begin(), ourTLIDs.end(), badTLIDs.begin(), badTLIDs.end(), 
						back_insert_iterator<vector<TLID> >(ourBads));
		if (!ourBads.empty())
		{
			printf("Skipped Polygon because one of its TLIDs is missing from the DB!\n");
			++gDeadTLID;
			continue;
		}
		
		EdgeIndex::iterator edgeIter = edges.find(*ourTLIDs.begin());
		ChainInfoMap::const_iterator tlidIter = chains.find(*ourTLIDs.begin());
	
		if (edgeIter != edges.end() && tlidIter != chains.end())
		{
			Pmwx::Face_handle		our_face = outMap.faces_end();
			Pmwx::Halfedge_handle	he = edgeIter->second;
			if (!he->mDominant) he = he->twin();
			if (!he->mDominant)
			{
				++gBadEdges;
				printf("WARNING: Halfedge with no dominance!!\n");
				continue;
			}
			
			if (poly->first == tlidIter->second.lpoly)
			{
				our_face = he->face();
			} else if (poly->first == tlidIter->second.rpoly)
			{
				our_face = he->twin()->face();
			} else  {
				printf("WARNING: TLID from poly not backlinked to our poly!\n");
				++gBadBackLink;
			}

			if (our_face != outMap.faces_end())
			{
				if (poly->second.water)
				{
					our_face->mLandClass = lc_GenericWater;
				}
				faces[poly->first] = our_face;
			} 			
		} else
			++gMissingTLIDs;
	}
	

	printf("Polygons: %d, missing TLIDs from indices: %d, edges with no dominance: %d, bad back links: %d, dead TLIDS: %d\n",
			gPolys, gMissingTLIDs, gBadEdges, gBadBackLink, gDeadTLID);
	




	int	gLand = 0, gNoID = 0, gNoLocAtAll = 0, gPtOnEdge = 0;
	for (LandmarkInfoMap::const_iterator landmark = landmarks.begin();
		landmark != landmarks.end(); ++landmark)	
	{
		++gLand;
		int cfcc = LookupAreaCFCC(landmark->second.cfcc.c_str());
		if (cfcc != -1)
		{
			if (!landmark->second.cenid_polyid.empty())
			{
				FaceIndex::iterator theFace = faces.find(landmark->second.cenid_polyid);
				if (theFace != faces.end())
				{
					theFace->second->mLandClass = kAreaCodes[cfcc].land_class;
				} else {
					fprintf(stderr, "WARNING: Cenid/polyid not found.\n");
					++gNoID;
				}
				
			} else if (!landmark->second.location.first.empty()) {
			
				if (kAreaCodes[cfcc].allow_from_point)
				{
					try {
						Pmwx::Locate_type	lt;
						
						Pmwx::Halfedge_handle h = 
							outMap.locate(RawCoordToCoord(landmark->second.location), lt);
							
						if (lt == Pmwx::EDGE || lt == Pmwx::FACE)
						{
							h->face()->mLandClass = kAreaCodes[cfcc].land_class;
						} else {
							++gPtOnEdge;
							fprintf(stderr, "WARNING: Pt land mark on vertex or out of map.\n");
						}
					} catch (...) {
						++gPtOnEdge;
					}
				} else {
					// TODO: Add pt object
				}			
			} else {
				fprintf(stderr, "Warning: landmark without polygon or pt.\n");
				gNoLocAtAll++;
			}
		}
	}
	printf("Total landmarks = %d, total with unknown CENID/POLYID = %d, no Loc = %d, pt on edge = %d\n", gLand, gNoID, gNoLocAtAll, gPtOnEdge);
}
