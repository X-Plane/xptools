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
#include "SDTSReadTVP.h"
#include "CompGeomUtils.h"
#include "MemFileUtils.h"
#include "SDTSRead.h"
#include "MapDefs.h"
#include "GISUtils.h"
#include "WTPM.h"

#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Xref.h>
#include <sdts++/builder/sb_Iden.h>
#include <sdts++/builder/sb_Poly.h>
#include <sdts++/builder/sb_Pnts.h>
#include <sdts++/builder/sb_Line.h>

const char *		kSDTSErrMsgs[] = {
	"Success.",
	"A foreign key cannot be found in the transfer.",
	"Inconsistent topological linkage.",
	"Unreadable file.",
	"Wrong record type in module.",
	"Info missing from record.",
	"Unreadable spatial address.",
	"Logic check failed."
};

struct	CoordTransform {
	bool	UTM;			// Are we in UTM?  If so we need to go to geo.
	int		zone;			// UTM zone number
	double	xscale;
	double	yscale;
	double	xoffset;
	double 	yoffset;
};

struct	StMemFile {
	StMemFile(MFMemFile * mf) : mf_(mf) { }
	~StMemFile() { if (mf_) MemFile_Close(mf_); }
	operator MFMemFile* () const { return mf_; }

	MFMemFile * mf_;
};

typedef vector<ForeignKey>			ForeignKeyVector;
typedef	pair<ForeignKey, bool>		DirectedForeignKey;	// True for pointing at us
typedef	vector<DirectedForeignKey>	DirectedForeignKeyVector;

struct PointRecord : public WTPM_Node {
	char						objType[2];
	bool						isTopo;
};
typedef	hash_map<int, PointRecord>		PointMap;
typedef	hash_map<string, PointMap>		PointTable;

struct	LineRecord : public WTPM_Line {
	char					objType[2];

	ForeignKey				leftFaceKey;
	ForeignKey				rightFaceKey;
	ForeignKey				startNodeKey;
	ForeignKey				endNodeKey;
};
typedef hash_map<int, LineRecord>		LineMap;
typedef	hash_map<string, LineMap>		LineTable;

struct	PolygonRecord : public WTPM_Face {
	char					objType[2];
};
typedef	hash_map<int, PolygonRecord>		PolygonMap;
typedef	hash_map<string, PolygonMap>	PolygonTable;

struct Topology {
	PointTable		points;
	LineTable		lines;
	PolygonTable	polygons;

	PointRecord *			FetchForeignKeyPoint(const ForeignKey& inKey);
	LineRecord *			FetchForeignKeyLine(const ForeignKey& inKey);
	PolygonRecord *			FetchForeignKeyPolygon(const ForeignKey& inKey);

};

SDTSErrorType	ReadLineModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary);
SDTSErrorType	ReadPointModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary);
SDTSErrorType	ReadPolygonModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary);

void	ConvertCoordinates(Topology& ioTopology, const CoordTransform& inCoords);
void	ConvertKeysToLinks(Topology& ioTopology);
void	FindVectors(Topology& ioTopology,
					WTPM_NodeVector& nodes,
					WTPM_LineVector& lines,
					WTPM_FaceVector& faces);






// This routine reads one line module from the file
SDTSErrorType	ReadLineModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary)
{
		sc_Record	rawRec;
		sb_Line		line;

	SDTSModuleIterator	modIter(file, dictionary);

	LineMap * vec = NULL;

	while (!modIter.Done())
	{
		modIter.Get(rawRec);

		if (!line.setRecord(rawRec)) throw SDTSException(sdts_WrongRecordType, rawRec, "expected line-type");

		if (vec == NULL)
		{
			if (ioTopology.lines.find(line.getModuleName()) == ioTopology.lines.end())
			{
				ioTopology.lines.insert(LineTable::value_type(line.getModuleName(), LineMap()));
			}
			vec = &(ioTopology.lines[line.getModuleName()]);
		}

		LineRecord	newLine;
		string	objRep;
		sb_ForeignID	lpoly, rpoly, snode, enode;
		if (!line.getObjectRepresentation(objRep))	throw SDTSException(sdts_MissingInfo, rawRec, "need objtype");
		if (objRep.size() != 2) 	throw SDTSException(sdts_LogicError, rawRec, "objrep isn't 2 chars");
		newLine.objType[0] = objRep[0];
		newLine.objType[1] = objRep[1];

		if (line.getPolygonIDLeft(lpoly))	newLine.leftFaceKey = ForeignKey(lpoly);	else throw SDTSException(sdts_MissingInfo, rawRec, "need lpoly");
		if (line.getPolygonIDRight(rpoly))	newLine.rightFaceKey = ForeignKey(rpoly);	else throw SDTSException(sdts_MissingInfo, rawRec, "need rpoly");
		if (line.getStartNodeID(snode))		newLine.startNodeKey = ForeignKey(snode);	else throw SDTSException(sdts_MissingInfo, rawRec, "need snode");
		if (line.getEndNodeID(enode))		newLine.endNodeKey = ForeignKey(enode);	else throw SDTSException(sdts_MissingInfo, rawRec, "need enode");
		sb_Spatials		spatial;
		if (line.getSpatialAddress(spatial))
		{
			for (sb_Spatials::iterator i = spatial.begin(); i != spatial.end(); ++i)
			{
				Point2	p;
				if (i->x().getDouble(p.x) && i->y().getDouble(p.y))
					newLine.shape.push_back(p);
				else
					throw SDTSException(sdts_BadSpatialID, *i);
			}
		}
		if (newLine.shape.size() < 2)
			throw SDTSException(sdts_MissingInfo, rawRec, "need at least two spatial addresses on a line");

		if (newLine.startNodeKey == newLine.endNodeKey && newLine.shape.size() < 3)
			throw SDTSException(sdts_MissingInfo, rawRec, "need at least three spatial addresses for a line with same start and end points");

		for (int n = 1; n < newLine.shape.size(); ++n)
		{
			if (newLine.shape[n-1] == newLine.shape[n])
				throw SDTSException(sdts_MissingInfo, rawRec, "zero length segment defined in line record");
		}

		if (vec->find(line.getRecordID()) != vec->end())
			throw SDTSException(sdts_LogicError, rawRec, "duplicate record id");
		vec->insert(LineMap::value_type(line.getRecordID(), newLine));

		modIter.Increment();
	}

	return modIter.Error() ? sdts_BadRecordFile : sdts_Ok;
}

// This routine reads one point module from the file
SDTSErrorType	ReadPointModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary)
{
		sc_Record	rawRec;
		sb_Pnts		pnts;

	SDTSModuleIterator	modIter(file, dictionary);

	PointMap * vec = NULL;

	while (!modIter.Done())
	{
		modIter.Get(rawRec);

		if (!pnts.setRecord(rawRec)) throw SDTSException(sdts_WrongRecordType, rawRec, "expected pnts-type");

		if (vec == NULL)
		{
			if (ioTopology.points.find(pnts.getModuleName()) == ioTopology.points.end())
			{
				ioTopology.points.insert(PointTable::value_type(pnts.getModuleName(), PointMap()));
			}
			vec = &(ioTopology.points[pnts.getModuleName()]);
		}

		PointRecord	newPt;
		string	objRep;

		if (!pnts.getObjectRepresentation(objRep))	throw SDTSException(sdts_MissingInfo, rawRec, "need objtype");
		if (objRep.size() != 2) 	throw SDTSException(sdts_LogicError, rawRec, "objrep isn't 2 chars");
		newPt.objType[0] = objRep[0];
		newPt.objType[1] = objRep[1];

		newPt.isTopo = newPt.objType[1] == 'O';	// NO = node point, NA = area point (attached to poly), NP = non-topological point

		sb_Spatial		spatial;
		if (!pnts.getSpatialAddress(spatial))	throw SDTSException(sdts_MissingInfo, rawRec, "need one spatial address");
		if (!(spatial.x().getDouble(newPt.location.x) && spatial.y().getDouble(newPt.location.y)))
			throw SDTSException(sdts_BadSpatialID, spatial);

		if (vec->find(pnts.getRecordID()) != vec->end())
			throw SDTSException(sdts_LogicError, rawRec, "duplicate record id");

		vec->insert(PointMap::value_type(pnts.getRecordID(), newPt));

		modIter.Increment();
	}

	return modIter.Error() ? sdts_BadRecordFile : sdts_Ok;
}

// This routine reads one polygon module from the file
SDTSErrorType	ReadPolygonModule(Topology& ioTopology, MFMemFile * file, sio_8211_converter_dictionary * dictionary)
{
		sc_Record	rawRec;
		sb_Poly		poly;

	SDTSModuleIterator	modIter(file, dictionary);

	PolygonMap * vec = NULL;

	while (!modIter.Done())
	{
		modIter.Get(rawRec);

		if (!poly.setRecord(rawRec)) throw SDTSException(sdts_WrongRecordType, rawRec, "expected poly-type");

		if (vec == NULL)
		{
			if (ioTopology.polygons.find(poly.getModuleName()) == ioTopology.polygons.end())
			{
				ioTopology.polygons.insert(PolygonTable::value_type(poly.getModuleName(), PolygonMap()));
			}
			vec = &(ioTopology.polygons[poly.getModuleName()]);
		}

		PolygonRecord	newPoly;
		string	objRep;

		if (!poly.getObjectRepresentation(objRep))	throw SDTSException(sdts_MissingInfo, rawRec, "need objtype");
		if (objRep.size() != 2) 	throw SDTSException(sdts_LogicError, rawRec, "objrep isn't 2 chars");
		newPoly.objType[0] = objRep[0];
		newPoly.objType[1] = objRep[1];

		newPoly.isWorld = (newPoly.objType[1] == 'W');

		if (vec->find(poly.getRecordID()) != vec->end())
			throw SDTSException(sdts_LogicError, rawRec, "duplicate record id");
		vec->insert(PolygonMap::value_type(poly.getRecordID(), newPoly));

		modIter.Increment();
	}

	return modIter.Error() ? sdts_BadRecordFile : sdts_Ok;
}

// This routine reads all modules from the file that we need.  Read the metadata first - if it's bad, don't waste time reading 10,000 geometry
// records fo rnothing!
void	ReadTransfer(Topology& ioTopology, SDTSDirectory * inDirectory, sio_8211_converter_dictionary * inDictionary, CoordTransform&	transform)
{
		vector<string>	mods;
		sc_Record		rawRec;
		sb_Xref			xref;
		sb_Iref			iref;
		string			sys, zoneStr;

	{
		StMemFile	xrefMem(inDirectory->OpenModule("XREF"));
		if (xrefMem == NULL) throw SDTSException(sdts_BadRecordFile, "unable to open module", "xref");

		SDTSModuleIterator	modIter(xrefMem, inDictionary);
		modIter.Get(rawRec);
		if (modIter.Error()) throw SDTSException(sdts_BadRecordFile, "unable to read module record", "xref");
		if (!xref.setRecord(rawRec)) throw SDTSException(sdts_WrongRecordType, rawRec, "expected xref");

		if (!xref.getReferenceSystemName(sys))	throw SDTSException(sdts_MissingInfo, rawRec, "reference system");
		transform.UTM = (sys == "UTM");
		if (transform.UTM)
		{
			if (!xref.getZoneReferenceNumber(zoneStr))	throw SDTSException(sdts_MissingInfo, rawRec, "zone number");
			transform.zone = atoi(zoneStr.c_str());
		}
	}

	{
		StMemFile	irefMem(inDirectory->OpenModule("IREF"));
		if (irefMem == NULL) throw SDTSException(sdts_BadRecordFile, "unable to open module", "iref");

		SDTSModuleIterator modIter(irefMem, inDictionary);
		modIter.Get(rawRec);
		if (modIter.Error()) throw SDTSException(sdts_BadRecordFile, "unable to rea dmodule record", "iref");
		if (!iref.setRecord(rawRec)) throw SDTSException(sdts_WrongRecordType, rawRec, "expected iref");

		if (!iref.getScaleFactorX(transform.xscale)) transform.xscale = 1.0;
		if (!iref.getScaleFactorY(transform.yscale)) transform.yscale = 1.0;
		if (!iref.getXOrigin(transform.xoffset)) transform.xoffset = 0.0;
		if (!iref.getYOrigin(transform.yoffset)) transform.yoffset = 0.0;
	}

	inDirectory->GetAllModuleNames(mods);
	for (vector<string>::iterator modName = mods.begin(); modName != mods.end(); ++modName)
	{
		if (modName->size() == 4)
		{
			string	modType = modName->substr(0,2);
			int	modNum = atoi(modName->substr(2,4).c_str());
			if (modType == "LE" && modNum > 0)
			{
				StMemFile mf(inDirectory->OpenModule(*modName));
				if (!mf) throw SDTSException(sdts_BadRecordFile, "cannot open module", modType.c_str());
				SDTSErrorType err = ReadLineModule(ioTopology, mf, inDictionary);
				if (err != sdts_Ok) throw SDTSException(err, "error reading records from module", modType.c_str());
			}
			if ((modType == "NA" || modType == "NO" || modType == "NP") && modNum > 0)
			{
				StMemFile mf(inDirectory->OpenModule(*modName));
				if (!mf) throw SDTSException(sdts_BadRecordFile, "cannot open module", modType.c_str());
				SDTSErrorType err = ReadPointModule(ioTopology, mf, inDictionary);
				if (err != sdts_Ok) throw SDTSException(err, "error reading records from module", modType.c_str());
			}
			if (modType == "PC" && modNum > 0)
			{
				StMemFile mf(inDirectory->OpenModule(*modName));
				if (!mf) throw SDTSException(sdts_BadRecordFile, "cannot open module", modType.c_str());
				SDTSErrorType err = ReadPolygonModule(ioTopology, mf, inDictionary);
				if (err != sdts_Ok) throw SDTSException(err, "error reading records from module", modType.c_str());
			}
		}
	}
}

void	ConvertCoordinates(Topology& sdts, const CoordTransform& coords)
{
	for (PointTable::iterator ptable = sdts.points.begin(); ptable != sdts.points.end(); ++ptable)
	for (PointMap::iterator pt = ptable->second.begin(); pt != ptable->second.end(); ++pt)
	{
		pt->second.location.x *= coords.xscale;
		pt->second.location.y *= coords.yscale;
		pt->second.location.x += coords.xoffset;
		pt->second.location.y += coords.yoffset;
		if (coords.UTM)
			UTMToLonLat(pt->second.location.x, pt->second.location.y, coords.zone, &pt->second.location.x, &pt->second.location.y);
	}

	for (LineTable::iterator ltable = sdts.lines.begin(); ltable != sdts.lines.end(); ++ltable)
	for (LineMap::iterator line = ltable->second.begin(); line != ltable->second.end(); ++line)
	for (vector<Point2>::iterator pt = line->second.shape.begin(); pt != line->second.shape.end(); ++pt)
	{
		pt->x *= coords.xscale;
		pt->y *= coords.yscale;
		pt->x += coords.xoffset;
		pt->y += coords.yoffset;
		if (coords.UTM)
			UTMToLonLat(pt->x, pt->y, coords.zone, &pt->x, &pt->y);
	}
}

void	ConvertKeysToLinks(Topology& sdts)
{
	for (LineTable::iterator table = sdts.lines.begin(); table != sdts.lines.end(); ++table)
	for (LineMap::iterator line = table->second.begin(); line != table->second.end(); ++line)
	{
		line->second.leftFace = sdts.FetchForeignKeyPolygon(line->second.leftFaceKey);
		line->second.rightFace = sdts.FetchForeignKeyPolygon(line->second.rightFaceKey);
		line->second.startNode = sdts.FetchForeignKeyPoint(line->second.startNodeKey);
		line->second.endNode = sdts.FetchForeignKeyPoint(line->second.endNodeKey);
	}
}

void	FindVectors(Topology& sdts,
					WTPM_NodeVector& nodes,
					WTPM_LineVector& lines,
					WTPM_FaceVector& faces)
{
	for (PointTable::iterator table = sdts.points.begin(); table != sdts.points.end(); ++table)
	for (PointMap::iterator row = table->second.begin(); row != table->second.end(); ++row)
	if (row->second.isTopo)
		nodes.push_back(&row->second);

	for (LineTable::iterator table = sdts.lines.begin(); table != sdts.lines.end(); ++table)
	for (LineMap::iterator row = table->second.begin(); row != table->second.end(); ++row)
	lines.push_back(&row->second);

	for (PolygonTable::iterator table = sdts.polygons.begin(); table != sdts.polygons.end(); ++table)
	for (PolygonMap::iterator row = table->second.begin(); row != table->second.end(); ++row)
	faces.push_back(&row->second);

}


PointRecord *			Topology::FetchForeignKeyPoint(const ForeignKey& inKey)
{
	PointTable::iterator table = points.find(inKey.first);
	if (table == points.end()) return NULL;
	PointMap::iterator row = table->second.find(inKey.second);
	if (row == table->second.end()) return NULL;
		return &row->second;
}

LineRecord *			Topology::FetchForeignKeyLine(const ForeignKey& inKey)
{
	LineTable::iterator table = lines.find(inKey.first);
	if (table == lines.end()) return NULL;
	LineMap::iterator row = table->second.find(inKey.second);
	if (row == table->second.end()) return NULL;
		return &row->second;
}

PolygonRecord *			Topology::FetchForeignKeyPolygon(const ForeignKey& inKey)
{
	PolygonTable::iterator table = polygons.find(inKey.first);
	if (table == polygons.end()) return NULL;
	PolygonMap::iterator row = table->second.find(inKey.second);
	if (row == table->second.end()) return NULL;
		return &row->second;
}

ForeignKey::ForeignKey() : pair<string,int>()
{
}

ForeignKey::ForeignKey(const sb_ForeignID& id) : pair<string, int>(id.moduleName(), id.recordID())
{
}

ForeignKey::ForeignKey(const string& s, int d) : pair<string, int>(s, d)
{
}

ForeignKey::ForeignKey(const ForeignKey& x) : pair<string, int>(x)
{
}


void	ImportSDTSTransferTVP(const char * path, const char * ext, Pmwx& pmwx)
{
	SDTSDirectory	dir(path, ext);
	if (dir.DidLoad())
	{

		CoordTransform	transform;
		Topology	topology;
		WTPM_NodeVector	nodes;
		WTPM_LineVector	lines;
		WTPM_FaceVector	faces;

		sio_8211_converter_dictionary	dict;
		AddConverters(dir, dict);

		ReadTransfer(topology, &dir, &dict, transform);
		ConvertKeysToLinks(topology);
		FindVectors(topology, nodes, lines, faces);
		WTPM_CreateBackLinks(lines);
		WTPM_RestoreTopology(nodes, lines, faces);
		ConvertCoordinates(topology, transform);
		WTPM_ExportToMap(nodes, lines, faces, pmwx);
	}
}



SDTSException::SDTSException(SDTSErrorType errType, const ForeignKey& theKey, const ForeignKey& us)
{
	sprintf(mBuf, "%s %s/%d %s/%d", kSDTSErrMsgs[errType], theKey.first.c_str(), theKey.second, us.first.c_str(), us.second);
}

SDTSException::SDTSException(SDTSErrorType errType, const ForeignKey& theKey, const char * info)
{
	sprintf(mBuf, "%s %s/%d %s", kSDTSErrMsgs[errType], theKey.first.c_str(), theKey.second, info);
}

SDTSException::SDTSException(SDTSErrorType errType, const sc_Record& theRecord, const char * info)
{
	sprintf(mBuf, "%s %s", kSDTSErrMsgs[errType], info);
}

SDTSException::SDTSException(SDTSErrorType errType, const sb_Spatial& addr)
{
	sprintf(mBuf, "%s", kSDTSErrMsgs[errType]);
}

SDTSException::SDTSException(SDTSErrorType errType, const char * s1 , const char * s2)
{
	sprintf(mBuf, "%s %s %s", kSDTSErrMsgs[errType], s1, s2);
}

