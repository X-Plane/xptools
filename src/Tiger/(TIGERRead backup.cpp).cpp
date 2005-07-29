#include "TIGERRead.h"
#define XUTILS_EXCLUDE_MAC_CRAP 1
#include "XUtils.h"
#include "MapDefs.h"
#include "TigerImport.h"

ChainInfoMap		gChains;
LandmarkInfoMap		gLandmarks;
PolygonInfoMap		gPolygons;

static	int	gPassNum = 0;

// File loading routines.  Call them in the order they are listed in to 
// get correct hashing!

void	TIGER_LoadRT1(const char * inFileName)
{
	// Important: we basically tag each TLID with a unique num from the file it
	// came from.  This allows us to not load the shape points twice when loading
	// a pair of files.
	gPassNum++;
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		TLID		tlid;		
		ChainInfo_t	ci;
		
		ExtractFixedRecordUnsignedLong(line, 6, 15, tlid);
		ci.one_side = line[15] == 'Y';
		ci.owner = gPassNum;
		ExtractFixedRecordString(line, 20, 49, ci.name);
		ExtractFixedRecordString(line, 56, 58, ci.cfcc);
		ExtractFixedRecordString(line, 191,200, ci.start.second);
		ExtractFixedRecordString(line, 201,209, ci.start.first);
		ExtractFixedRecordString(line, 210,219, ci.end.second);
		ExtractFixedRecordString(line, 220,228, ci.end.first);
		
		if (gChains.find(tlid) == gChains.end())
			gChains.insert(ChainInfoMap::value_type(tlid, ci));
	}
}

void	TIGER_LoadRT2(const char * inFileName)
{
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		TLID		tlid;		

		ExtractFixedRecordUnsignedLong(line, 6, 15, tlid);

		ChainInfo_t&	ci = gChains[tlid];
		if (ci.owner == gPassNum)
		{		
			for (int n = 0; n < 10; ++n)
			{
				RawCoordPair	cp;
				ExtractFixedRecordString(line, 19 + 19 * n, 28 + 19 * n, cp.second);
				ExtractFixedRecordString(line, 29 + 19 * n, 37 + 19 * n, cp.first);

				if ((!cp.first.empty()) && 
					(!cp.second.empty()) &&
					(atoi(cp.first.c_str()) != 0) &&
					(atoi(cp.second.c_str()) != 0))
				{
					ci.shape.push_back(cp);
				}
			}
		}
	}
}	

void	TIGER_LoadRTP(const char * inFileName)
{
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		CENID_POLYID	id;
		PolygonInfo_t	info;
		info.antennas = 0;
		ExtractFixedRecordString(line, 11, 25, id);
		ExtractFixedRecordString(line, 36, 44, info.location.first);
		ExtractFixedRecordString(line, 26, 35, info.location.second);
		info.water = (line[44] == '1') ? 1 : 0;
		
		gPolygons.insert(PolygonInfoMap::value_type(id, info));
	}
}

void	TIGER_LoadRTI(const char * inFileName)
{
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		CENID_POLYID	rid, lid;
		TLID			tlid;

		ExtractFixedRecordString(line, 41, 55, lid);
		ExtractFixedRecordString(line, 56, 70, rid);
		ExtractFixedRecordUnsignedLong(line, 11, 20, tlid);
		
		if (!lid.empty())
		{
			PolygonInfoMap::iterator liter = gPolygons.find(lid);		
			if (liter != gPolygons.end())
			{
				liter->second.border.push_back(DirectedTLID(tlid, false));
				
				if (rid == lid)
					liter->second.antennas++;
			}
		}
		
		if (!rid.empty() && (lid != rid))
		{
			PolygonInfoMap::iterator riter = gPolygons.find(rid);
			if (riter != gPolygons.end())
			{
				riter->second.border.push_back(DirectedTLID(tlid, false));
			}
		}
		
		ChainInfoMap::iterator citer = gChains.find(tlid);
		if (citer != gChains.end())
		{
			// Only set the poly IDs for the chain if its 
			// not the outside.  This way we will merge adjacent files.
			if (!lid.empty()) citer->second.lpoly = lid;
			if (!rid.empty()) citer->second.rpoly = rid;
		}
	}
}


void	TIGER_LoadRT7(const char * inFileName)
{
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		LAND			landID;
		LandmarkInfo_t	info_t;
		
		ExtractFixedRecordUnsignedLong(line, 11, 20, landID);
		
		ExtractFixedRecordString(line, 22, 24, info_t.cfcc);
		ExtractFixedRecordString(line, 25, 54, info_t.name);
		ExtractFixedRecordString(line, 55, 64, info_t.location.second);
		ExtractFixedRecordString(line, 65, 73, info_t.location.first);

		gLandmarks.insert(LandmarkInfoMap::value_type(landID, info_t));
	}
}

void	TIGER_LoadRT8(const char * inFileName)
{
	for (StTextFileScanner scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		
		LAND			landID;

		ExtractFixedRecordUnsignedLong(line, 26, 35, landID);
		
		LandmarkInfo_t&	info = gLandmarks[landID];
		ExtractFixedRecordString(line, 11, 25, info.cenid_polyid);
	}
}

void TIGER_SanityCheckPerimeter(PolygonInfo_t& poly, const char * id)
{
	for (int n = 0; n < poly.border.size(); ++n)
	{
		int nxt = n + 1;
		if (nxt >= poly.border.size()) nxt = 0;
		ChainInfo_t&	thisTLID = gChains[poly.border[n].first];
		ChainInfo_t&	nextTLID = gChains[poly.border[nxt].first];
		
		RawCoordPair thisOutgoing = (poly.border[n  ].second) ? thisTLID.start : thisTLID.end;
		RawCoordPair nextIncoming = (poly.border[nxt].second) ? nextTLID.end : nextTLID.start;
		
		if (thisOutgoing != nextIncoming)
		{
			printf("ERROR: Border check failed on polygon, CENID/POLYID = %s!\n", id);
			return;
		}
	}
}

void	TIGER_SortPerimeter(PolygonInfo_t&	poly, const char * id)
{
	// This is hokey as hell, but it does work.
	
	typedef	map<Pmwx::Halfedge *, DirectedTLID>	TLIDMap;
	
	VertexIndex	ptAssistCache;
	
	Pmwx	planar_map;
	TLIDMap	index;
	set<TLID>	old_borders;
	int	old_border_dupes = 0;
	for (DirectedTLIDVector::iterator i = poly.border.begin(); i != poly.border.end(); ++i)
	{
		if (old_borders.find(i->first) != old_borders.end()) ++old_border_dupes;
		old_borders.insert(i->first);
	}
	
	poly.original_border_count = poly.border.size();
	int	dupes = 0;
	for (DirectedTLIDVector::iterator tlid = poly.border.begin(); 
		tlid != poly.border.end(); ++tlid)
	{
		ChainInfoMap::iterator	tlidP = gChains.find(tlid->first);
		if (tlidP == gChains.end())
			printf("ERROR: unknown tlid in poly perimeter!!\n");
		else {
			vector<RawCoordPair>	pts(tlidP->second.shape);
			pts.insert(pts.begin(), tlidP->second.start);
			pts.insert(pts.end(), tlidP->second.end);
			
			Pmwx::Halfedge_handle	he = Pmwx::Halfedge_handle();
			
			for (int n = 1; n < pts.size(); ++n)
			{
				PM_Curve_2	seg(
					Point_2(
						atof(pts[n-1].first.c_str()),
						atof(pts[n-1].second.c_str())),
					Point_2(
						atof(pts[n  ].first.c_str()),
						atof(pts[n  ].second.c_str())));
						
//				Pmwx::Halfedge_handle	he = InsertOneSegment(pts[n-1], pts[n],
//							ptAssistCache, planar_map);

				Pmwx::Halfedge_handle he = planar_map.non_intersecting_insert(seg);
				if (he != Pmwx::Halfedge_handle())
				{
					index.insert(TLIDMap::value_type(&*he, DirectedTLID(tlid->first, false)));
					index.insert(TLIDMap::value_type(&*(he->twin()), DirectedTLID(tlid->first, true)));
				} else
					printf("WARNING: hit a dupe!\n");
			}
		}
	}
	
	
	poly.border.clear();
	Pmwx::Face_handle f = planar_map.unbounded_face();
	if (distance(f->holes_begin(), f->holes_end()) < 1)
	{
		printf("ERROR: one poly made a map with no holes!!\n");
	} else {
		
		Pmwx::Face_handle	tf = Pmwx::Face_handle();
		
		for (TLIDMap::iterator i = index.begin(); i != index.end(); ++i)
		{
			CENID_POLYID	rpoly = gChains[i->second.first].rpoly;
			CENID_POLYID	lpoly = gChains[i->second.first].lpoly;
			if (i->second.second)
			{
				// We have a half-edge that goes opposite the TLID's direction.
				if (lpoly == id)
				{
					tf = i->first->face();
					break;
				}
			} else {
				if (rpoly == id)
				{
					tf = i->first->face();
					break;
				}
			}
		}
		if (tf == Pmwx::Face_handle())
		{
			printf("ERROR: couldn't find our face in one of the holes!\n");
			return;
		}
		Pmwx::Ccb_halfedge_circulator cur, last;
		if (tf->is_unbounded() && string(id) != WORLD_POLY)
			printf("WARNING: unbounded poly: %s\n", id);
		if (!tf->is_unbounded() && string(id) == WORLD_POLY)
			printf("WARNING: world poly has bounds.\n");
		if (!tf->is_unbounded())
		{
			cur = last = tf->outer_ccb();
			do {
				DirectedTLID us = index[&*cur];
				if (poly.border.empty() || (poly.border.back() != us && poly.border.front() != us))
					poly.border.push_back(us);
				++cur;
			} while (cur != last);
		}		
		for (Pmwx::Holes_iterator hole = tf->holes_begin(); hole != tf->holes_end(); ++hole)
		{
			DirectedTLIDVector	d;
			cur = last = *hole;
			do {
				DirectedTLID us = index[&*cur];
				if (d.empty() || (d.back() != us && d.front() != us))
					d.push_back(us);
				++cur;
			} while (cur != last);
			poly.holes.push_back(d);
		}		
	}

	int	total_tlids = poly.border.size();
	for (DirectedTLIDVectorVector::iterator v = poly.holes.begin(); v != poly.holes.end(); ++v)
	{
		total_tlids += v->size();
	}
	
	if ((poly.original_border_count + poly.antennas) != (total_tlids))
	{
		printf("ERROR: changed border segment count, old = %d, new = %d, antennas = %d, halfedges = %d.\n", poly.original_border_count, total_tlids, poly.antennas, index.size());
		set<TLID>	new_borders;
		int		new_border_dupes = 0;
		for (DirectedTLIDVector::iterator i = poly.border.begin(); i != poly.border.end(); ++i)
		{
			if (new_borders.find(i->first) != new_borders.end()) ++new_border_dupes;
			new_borders.insert(i->first);
		}
		for (DirectedTLIDVectorVector::iterator v = poly.holes.begin(); v != poly.holes.end(); ++v)
			for (DirectedTLIDVector::iterator i = v->begin(); i != v->end(); ++i)
			{
				if (new_borders.find(i->first) != new_borders.end()) ++new_border_dupes;
				new_borders.insert(i->first);
			}
		printf("Old %d unique %d dupes, new %d unique %d dupes.\n", old_borders.size(), old_border_dupes, new_borders.size(), new_border_dupes);
		vector<TLID>	dif;
		set_difference(old_borders.begin(), old_borders.end(), new_borders.begin(), new_borders.end(),
			back_insert_iterator<vector<TLID> >(dif));
		printf("Polygon %s AWOL TLIDs are:\n", id);
		for (vector<TLID>::iterator i = dif.begin(); i != dif.end(); ++i)
		{
			printf("  TLID=%d, Start=(%s,%s), end = (%s,%s)\n", *i,
				gChains[*i].start.first.c_str(),
				gChains[*i].start.second.c_str(),
				gChains[*i].end.first.c_str(),
				gChains[*i].end.second.c_str());
				
		}	
	}	
}

void	TIGER_PostProcess(void)
{
	// Now we go through and create the virtual 'world' face.  This is a slightly
	// weird polygon in that its boundary will end up CCW, but secretly it is an INNER
	// boundary.  Weird.
	
	PolygonInfo_t	worldPoly;
	worldPoly.water = 1;
	worldPoly.antennas = 0;
	for (ChainInfoMap::iterator chain = gChains.begin(); chain != gChains.end(); ++chain)
	{
		if (chain->second.lpoly.empty())
		{
			chain->second.lpoly = WORLD_POLY;
			worldPoly.border.push_back(DirectedTLID(chain->first, false));
		}
		if (chain->second.rpoly.empty())
		{
			chain->second.rpoly = WORLD_POLY;
			worldPoly.border.push_back(DirectedTLID(chain->first, false));
		}
	}
	
	gPolygons.insert(PolygonInfoMap::value_type(CENID_POLYID(WORLD_POLY), worldPoly));
	
	// Now go through and resort the boundaries of every polygon so that it is counter
	// clockwise and self-consistent.
	
	for (PolygonInfoMap::iterator poly = gPolygons.begin(); poly != gPolygons.end(); ++poly)
	{
		TIGER_SortPerimeter(poly->second, poly->first.c_str());
		TIGER_SanityCheckPerimeter(poly->second, poly->first.c_str());
	}
}