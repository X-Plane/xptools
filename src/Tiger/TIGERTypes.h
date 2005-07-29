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
#ifndef TIGERTYPES_H
#define TIGERTYPES_H

#include "WTPM.h"

#define USE_STREET_NAMES 0
#define USE_LANDMARK_NAMES 0

#define WORLD_POLY "WORLD_POLY"

#include <string>
#include <vector>
#include <hash_map>
#include <map>


using namespace std;

// A Tiger Line ID is a 32-bit int that identifies a single complete chain.
// They are global to the whole TIGER/Line set
typedef	unsigned long	TLID;

// CFCCs are 3-char codes that describe what a feature is
typedef	string			CFCC;

// Until we finish processing and correlating the data, we store lat/lons as strings
// so that we get a perfect match (no risk of hashing problems)
typedef	string			RawCoord;

// Land codes are ints that identify a landmark; hash with county ID to be safe
typedef	unsigned long	LAND;

// Census ID codes are alphanumeric, and Polygon ID codes are non-sequential ints.
// So better to just merge this whole mess into a string and hash; it's the only
// safe way to do it.
typedef	string			CENID_POLYID;

typedef	string			RawCoordKey;

typedef	pair<RawCoord, RawCoord>	RawCoordPair;
//typedef	vector<TLID>				TLIDVector;
typedef set<TLID>					TLIDSet;
//typedef	pair<TLID, bool>			DirectedTLID;
//typedef	vector<DirectedTLID>		DirectedTLIDVector;

struct	NodeInfo_t : public WTPM_Node {

};

typedef	hash_map<RawCoordKey, NodeInfo_t>	NodeInfoMap;

struct	ChainInfo_t : public WTPM_Line {
	int				tlid;
	bool			one_side;
#if USE_STREET_NAMES
	string			name;
#endif	
	CFCC			cfcc;			// Census feature classifcation code
	RawCoordKey		start;
	RawCoordKey		end;
	int				owner;		// File pass # for the owner!
	char			reversed;	// Current owner has vertex order opposite the one that made it 0=false1=true2=???
	CENID_POLYID	lpoly;
	CENID_POLYID	rpoly;
	
	bool			kill;		// Marker flag for culling
};

// Chain Infos are usually referenced by TLID
typedef	hash_map<TLID, ChainInfo_t>	ChainInfoMap;	

struct	LandmarkInfo_t {
#if USE_LANDMARK_NAMES
	string			name;
#endif	
	CFCC			cfcc;
	vector<CENID_POLYID>	cenid_polyid;
	Point2			location;
};
typedef	hash_map<LAND, LandmarkInfo_t>	LandmarkInfoMap;

struct	PolygonInfo_t : public WTPM_Face {
	TLIDSet						border;		// NOTE: this only exists to allow us to do rough
											// culling.  WTPM handles back-links for us for topo integration.
	Point2						location;	// Some point within the entity
	int							water;		// Water code - is this polygon wet?
	
	bool						kill;		// Marker flag for culling
};

typedef hash_map<CENID_POLYID, PolygonInfo_t>	PolygonInfoMap;

#endif
