/*
 * Copyright (c) 2007, Laminar Research.
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

#include "VPFImport.h"
#include "MapDefs.h"
#include "AssertUtils.h"
#include "VPFTable.h"
#include "WTPM.h"
#include "PlatformUtils.h"
#include "ProgressUtils.h"
#include "ParamDefs.h"

// A NOTE ON STRANGE ROUNDING ERRORS:
// VPF coordinates at 0.0 are not exactly 0 - but they are close, so we must round them down.

// A note on skipping: it is possible for a node to be in a VPF tile but not link to any actual entity in the tile (e.g. not link to any
// halfedge) because it is a corner that touches the cut line and there is no boundary inserted around the tile.  Therefore
// we need to blow this node off - skip allows us to track this.

struct VPF_Node : public WTPM_Node {
	int		edg_index;
	bool	skip;
};

struct VPF_Line : public WTPM_Line {
	int		start_cnd_index;
	int		end_cnd_index;
	int		left_fac_index;
	int		right_fac_index;
	int		left_edg_index;
	int		right_edg_index;
	Point2	start_node_pt;			// Some VPF data is missing node coordinates on the nodes themselves.
	Point2	end_node_pt;			// Copy it from the lines via this.

	bool	is_loop(void) const 			{ return start_cnd_index == end_cnd_index; }

	int			he_param;
	int			he_trans_flags;
};

struct VPF_Face : public WTPM_Face {
	vector<int>	edg_index;

	int			terrain_type;
	set<int>	area_features;
};

struct	StMemFile {
	StMemFile(const char * p) : f_(MemFile_Open(p))
	{
		if (f_ == NULL)
		{
			string fname(p);
			string::size_type dchr = fname.find_last_of("\\/:");
			if (f_ == NULL)
			{
				if (dchr != fname.npos)
				{
					fname[dchr-1] = tolower(fname[dchr-1]);
					f_ = MemFile_Open(fname.c_str());
				}
			}
			if (fname.find('.', dchr+1) == fname.npos)
			{
				fname += '.';
				f_ = MemFile_Open(fname.c_str());
			}
		}
	}
	~StMemFile() { if (f_) MemFile_Close(f_); }
	operator MFMemFile * () const { return f_; }
	bool operator()() const { return f_ != NULL; }
	MFMemFile * f_;
};

enum {
	require_Int,
	require_Link,
	require_Coords,
	require_String
};

static void NukeDupePts(vector<Point2>& pts)
{
	for (int n = 1; n < (pts.size()-1); ++n)
	{
		if (pts[n] == pts[n-1])
		{
			pts.erase(pts.begin()+n);
			--n;
		}
	}
}

class	RememberHalfedge : public CGAL::Arr_observer<Arrangement_2> {
public:
	WTPM_Line::EdgePair* slot;
	void set_slot(WTPM_Line::EdgePair * islot) { slot = islot; }
	virtual void after_create_edge(Halfedge_handle he)
	{
		
	}
	virtual void after_split_edge(Halfedge_handle e1, Halfedge_handle e2)
	{	
		// If we got here, the data is probably not network topology - that is, there are non-disjoint line segments.
		DebugAssert(!"Should not be here!");	
	}
};

/*static void RememberHalfedge(Halfedge_handle old_he, Halfedge_handle new_he, void * ref)
{
	WTPM_Line::EdgePair * slot = (WTPM_Line::EdgePair *) ref;
	Halfedge_handle he = (new_he ? new_he : old_he);
	slot->first.push_back(he);
	slot->second.push_back(he->twin());
}*/

static bool FindColumn(const VPF_TableDef& header, const char * column, int& index, int validate, const char * file)
{
	index = header.GetColumnByName(column);
	if (index == -1) {
		printf("Could not find column '%s' in file '%s'\n", column, file);
		return false;
	}
	switch(validate) {
	case require_String:
		if (!header.IsFieldString(index)) {
			printf("Column %d ('%s') is not a string in file '%s'\n", index, column, file);
			return false;
		}
		break;
	case require_Int:
		if (!header.IsFieldInt(index)) {
			printf("Column %d ('%s') is not an int in file '%s'\n", index, column, file);
			return false;
		}
		break;
	case require_Link:
		if (!header.IsFieldInt(index) && !header.IsFieldTripletKey(index)) {
			printf("Column %d ('%s') is not an int or triplet-key in file '%s'\n", index, column, file);
			return false;
		}
		break;
	case require_Coords:
		if (!header.IsFieldArray(index) || (!header.IsFieldTwoTuple(index) && !header.IsFieldThreeTuple(index))) {
			printf("Column %d ('%s') is not a corod array in file '%s'\n", index, column, file);
			return false;
		}
	}
	return true;
}

static bool GetVPFLink(VPFTableIterator& iter, const VPF_TableDef& header, int row, int& value)
{
	if (header.IsFieldInt(row)) return iter.GetNthFieldAsInt(row, value);
	VPF_TripletKey key;

	if (header.IsFieldTripletKey(row))
	{
		if (iter.GetNthFieldAsTripletKey(row, key))
		{
			value = key.row_id;
			return true;
		}
	}

	return false;
}

bool	VPFImportTopo3(
					const char * 		inCoverageDir,
					const char * 		inTile,
					Pmwx& 				ioMap,
					bool				inHasTopo,
					VPF_LineRule_t * 	inLineRules,
					VPF_FaceRule_t * 	inFaceRules,
					int *				inTransTable)
{
	char	tilePath[1024], thePath[1024];
	char	tileMajorX[2] = { inTile[0], 0 };
	char	tileMajorY[2] = { inTile[1], 0 };
	bool		ok = true;
	MFMemFile *	mem = NULL;
	int		expected_row, row_num, foreign_key, i, j;

	strcpy(tilePath, inCoverageDir);
	strcat(tilePath, DIR_STR);
	strcat(tilePath, tileMajorX);
	strcat(tilePath, DIR_STR);
	strcat(tilePath, tileMajorY);
	strcat(tilePath, DIR_STR);
	if (inTile[2] != 0)
	{
		char	tileMinorX[2] = { inTile[2], 0 };
		char	tileMinorY[2] = { inTile[3], 0 };
		strcat(tilePath, tileMinorX);
		strcat(tilePath, DIR_STR);
		strcat(tilePath, tileMinorY);
		strcat(tilePath, DIR_STR);

	}

	vector<VPF_Node>	nodes;
	vector<VPF_Line>	lines;
	vector<VPF_Face>	faces;

	/****************************************************************************************
	 * BUILD UP FEATURES
	 ****************************************************************************************/

	// For each rule we apply, keep a set of all of the raw primitive record numbers
	// that have this entity.
	vector<set<int> >	lineMatches;
	vector<set<int> >	faceMatches;
	vector<int>			lineColumns;	// The column number that a given rule
	vector<int>			faceColumns;	// is searching for.
	int					numLines = 0;
	int					numFaces = 0;
	{
		set<string>			tables;			// Every attribute table we need to read.
		string				strAttr;
		int					intAttr;
		int					id_col;

		while(inLineRules[numLines].table)
		{
			tables.insert(inLineRules[numLines].table);
			++numLines;
		}
		while(inFaceRules[numFaces].table)
		{
			tables.insert(inFaceRules[numFaces].table);
			++numFaces;
		}

		lineMatches.resize(numLines);
		faceMatches.resize(numFaces);
		lineColumns.resize(numLines);
		faceColumns.resize(numFaces);

		for(set<string>::iterator tableIter = tables.begin(); tableIter != tables.end(); ++tableIter)
		{
			strcpy(thePath, inCoverageDir);
			strcat(thePath, DIR_STR);
			strcat(thePath, tableIter->c_str());
			StMemFile		theTable(thePath);
			VPF_TableDef	tableDef;

			if (!theTable()) { printf("Could not open '%s'\n", thePath); return false; }
			if (!ReadVPFTableHeader(theTable, tableDef)) { printf("Could not read VPF header for '%s'\n", thePath); return false; }
			if (!FindColumn(tableDef, "id", id_col, require_Int, thePath)) return false;

			for (i = 0; i < numLines; ++i)
			if (*tableIter == inLineRules[i].table)
			{
				if (inLineRules[i].strval) {
					if (!FindColumn(tableDef, inLineRules[i].attr_column, lineColumns[i], require_String, thePath)) return false;
				} else {
					if (!FindColumn(tableDef, inLineRules[i].attr_column, lineColumns[i], require_Int, thePath)) return false;
				}
			}

			for (i = 0; i < numFaces; ++i)
			if (*tableIter == inFaceRules[i].table)
			{
				if (inFaceRules[i].strval) {
					if (!FindColumn(tableDef, inFaceRules[i].attr_column, faceColumns[i], require_String, thePath)) return false;
				} else {
					if (!FindColumn(tableDef, inFaceRules[i].attr_column, faceColumns[i], require_Int, thePath)) return false;
				}
			}

			expected_row = 1;
			for (VPFTableIterator tableRowIter(theTable, tableDef); !tableRowIter.Done(); tableRowIter.Next(), ++expected_row)
			{
				if (!tableRowIter.GetNthFieldAsInt(id_col, intAttr)) 	{ printf("Could not read int row id, col=%d, row=%d, file=%s\n", lineColumns[i], expected_row, thePath); return false; }
				if (intAttr != expected_row) 						{ printf("Expected row %d, got row %d, file=%s\n", expected_row, intAttr, thePath); return false; }
				for (i = 0; i < numLines; ++i)
				if (*tableIter == inLineRules[i].table)
				{
					if (inLineRules[i].strval)
					{
						if (!tableRowIter.GetNthFieldAsString(lineColumns[i], strAttr)) { printf("Could not read string attribute, col=%d, row=%d, file=%s\n", lineColumns[i], expected_row, thePath); return false; }
						if (strAttr == inLineRules[i].strval)
							lineMatches[i].insert(expected_row);
					} else {
						if (!tableRowIter.GetNthFieldAsInt(lineColumns[i], intAttr)) { printf("Could not read int attribute, col=%d, row=%d, file=%s\n", lineColumns[i], expected_row, thePath); return false; }
						if (intAttr == inLineRules[i].ival)
							lineMatches[i].insert(expected_row);
					}
				}

				for (i = 0; i < numFaces; ++i)
				if (*tableIter == inFaceRules[i].table)
				{
					if (inFaceRules[i].strval)
					{
						if (!tableRowIter.GetNthFieldAsString(faceColumns[i], strAttr)) { printf("Could not read string attribute, col=%d, row=%d, file=%s\n", faceColumns[i], expected_row, thePath); return false; }
						if (strAttr == inFaceRules[i].strval)
							faceMatches[i].insert(expected_row);
					} else {
						if (!tableRowIter.GetNthFieldAsInt(faceColumns[i], intAttr)) { printf("Could not read int attribute, col=%d, row=%d, file=%s\n", faceColumns[i], expected_row, thePath); return false; }
						if (intAttr == inFaceRules[i].ival)
							faceMatches[i].insert(expected_row);
					}
				}
			}
		}
	}

	/****************************************************************************************
	 * CONNECTED NODES (CND)
	 ****************************************************************************************/
	{
		printf("Importing points...\n");
		strcpy(thePath, tilePath);
		strcat(thePath, "cnd");

		StMemFile			cnd(thePath);
		VPF_TableDef		cndDef;
		if (!cnd()) { printf("Could not open '%s'\n", thePath); return false; }

		if (!ReadVPFTableHeader(cnd, cndDef)) { printf("Could not read VPF header for '%s'\n", thePath); return false; }

		int cnd_id;
		int cnd_first_edge;

		if (!FindColumn(cndDef, "id", cnd_id, require_Int, thePath)) return false;
		if (!FindColumn(cndDef, "first_edge", cnd_first_edge, require_Link, thePath)) return false;

		expected_row = 1;
		for (VPFTableIterator cndIter(cnd, cndDef); !cndIter.Done(); cndIter.Next(), ++expected_row)
		{
			if (!cndIter.GetNthFieldAsInt(cnd_id, row_num)) { printf("Could not read id on row %d of '%s'\n", expected_row, thePath); return false; }
			if (row_num != expected_row) { printf("Row numbers in table '%s' are not consecutive one-based.\n", thePath); return false; }
			if (!GetVPFLink(cndIter, cndDef, cnd_first_edge, foreign_key)) { printf("Could not read first_edge on row %d of '%s'\n", expected_row, thePath); return false; }

			nodes.push_back(VPF_Node());
			nodes.back().edg_index = foreign_key-1;
			nodes.back().skip = foreign_key == 0;

		}
	}

	/****************************************************************************************
	 * EDGES (EDG)
	 ****************************************************************************************/
	{
		printf("Importing lines...\n");
		strcpy(thePath, tilePath);
		strcat(thePath, "edg");

		StMemFile			edg(thePath);
		VPF_TableDef		edgDef;
		vector<Point2>		edgPts;
		if (!edg()) { printf("Could not open '%s'\n", thePath); return false; }

		if (!ReadVPFTableHeader(edg, edgDef)) { printf("Could not read VPF header for '%s'\n", thePath); return false; }

		int edg_id, edg_start_node, edg_end_node, edg_right_face, edg_left_face, edg_coordinates;
		int edg_right_edge, edg_left_edge, lin_attr_ref;
		if (!FindColumn(edgDef, "id", edg_id, require_Int, thePath)) return false;
		if (!FindColumn(edgDef, "start_node", edg_start_node, require_Link, thePath)) return false;
		if (!FindColumn(edgDef, "end_node", edg_end_node, require_Link, thePath)) return false;
		if (inHasTopo) if (!FindColumn(edgDef, "right_face", edg_right_face, require_Link, thePath)) return false;
		if (inHasTopo) if (!FindColumn(edgDef, "left_face", edg_left_face, require_Link, thePath)) return false;
		if (!FindColumn(edgDef, "right_edge", edg_right_edge, require_Link, thePath)) return false;
		if (!FindColumn(edgDef, "left_edge", edg_left_edge, require_Link, thePath)) return false;
		if (!FindColumn(edgDef, "coordinates", edg_coordinates, require_Coords, thePath)) return false;

		for (i = 0; i < numLines; ++i)
			if (!FindColumn(edgDef, inLineRules[i].ref_column, lineColumns[i], require_Int, thePath)) return false;

		expected_row = 1;
		for (VPFTableIterator edgIter(edg, edgDef); !edgIter.Done(); edgIter.Next(), ++expected_row)
		{
			if (!edgIter.GetNthFieldAsInt(edg_id, row_num)) { printf("Could not read id on row %d of '%s'\n", expected_row, thePath); return false; }
			if (row_num != expected_row) { printf("Row numbers in table '%s' are not consecutive one-based.\n", thePath); return false; }
			lines.push_back(VPF_Line());

			if (inHasTopo) {
			if (!GetVPFLink(edgIter, edgDef, edg_left_face, foreign_key)) { printf("Could not read left_face on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().left_fac_index = foreign_key-1;
			if (!GetVPFLink(edgIter, edgDef, edg_right_face, foreign_key)) { printf("Could not read right_face on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().right_fac_index = foreign_key-1;
			}
			if (!GetVPFLink(edgIter, edgDef, edg_left_edge, foreign_key)) { printf("Could not read left_edge on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().left_edg_index = foreign_key-1;
			if (!GetVPFLink(edgIter, edgDef, edg_right_edge, foreign_key)) { printf("Could not read right_edge on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().right_edg_index = foreign_key-1;
			if (!GetVPFLink(edgIter, edgDef, edg_start_node, foreign_key)) { printf("Could not read start_node on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().start_cnd_index = foreign_key-1;
			if (!GetVPFLink(edgIter, edgDef, edg_end_node, foreign_key)) { printf("Could not read end_node on row %d of '%s'\n", expected_row, thePath); return false; }
			lines.back().end_cnd_index = foreign_key-1;

			if (!edgIter.GetNthFieldAsCoordPairArray(edg_coordinates, edgPts)) { printf("Could not read edge coordinates on row %s of '%s'\n", expected_row, thePath); return false; }
			if (edgPts.size() < 2) { printf("Less than two pts for edge on row %s of '%s'\n", expected_row, thePath); return false; }

			lines.back().start_node_pt = edgPts.front();
			lines.back().end_node_pt = edgPts.back();
			lines.back().shape = edgPts;
			NukeDupePts(lines.back().shape);
			lines.back().he_trans_flags = 0;
			lines.back().he_param = NO_VALUE;

			for (i = 0; i < numLines; ++i)
			{
				if (!edgIter.GetNthFieldAsInt(lineColumns[i], lin_attr_ref)) { printf("Could not read col %d of row %d as int in file %s\n", lineColumns[i], expected_row, thePath); return false; }
				if (lineMatches[i].count(lin_attr_ref))
				{
//					if (inLineRules[i].he_param)
					DebugAssert(inLineRules[i].he_param >= 0 && inLineRules[i].he_param < gTokens.size());
						lines.back().he_param = inLineRules[i].he_param;
					if (inLineRules[i].he_trans_flags)
						lines.back().he_trans_flags |= inLineRules[i].he_trans_flags;
				}
			}

		}
	}

	/****************************************************************************************
	 * FACES (FAC)
	 ****************************************************************************************/
	if (inHasTopo)
	{
		printf("Importing faces...\n");
		strcpy(thePath, tilePath);
		strcat(thePath, "fac");

		StMemFile			fac(thePath);
		VPF_TableDef		facDef;
		if (!fac()) { printf("Could not open '%s'\n", thePath); return false; }

		if (!ReadVPFTableHeader(fac, facDef)) { printf("Could not read VPF header for '%s'\n", thePath); return false; }

		int fac_id, fac_attr_ref;

		if (!FindColumn(facDef, "id", fac_id, require_Int, thePath)) return false;
		for (i = 0; i < numFaces; ++i)
			if (!FindColumn(facDef, inFaceRules[i].ref_column, faceColumns[i], require_Int, thePath)) return false;

		expected_row = 1;
		for (VPFTableIterator facIter(fac, facDef); !facIter.Done(); facIter.Next(), ++expected_row)
		{
			if (!facIter.GetNthFieldAsInt(fac_id, row_num)) { printf("Could not read id on row %d of '%s'\n", expected_row, thePath); return false; }
			if (row_num != expected_row) { printf("Row numbers in table '%s' are not consecutive one-based.\n", thePath); return false; }

			faces.push_back(VPF_Face());
			faces.back().isWorld = (expected_row == 1);
			faces.back().terrain_type = terrain_Natural;

			for (i = 0; i < numFaces; ++i)
			{
				if (!facIter.GetNthFieldAsInt(faceColumns[i], fac_attr_ref)) { printf("Could not read col %d of row %d as int in file %s\n", faceColumns[i], expected_row, thePath); return false; }
				if (faceMatches[i].count(fac_attr_ref))
				{
					if (inFaceRules[i].terrain_type)
						faces.back().terrain_type = inFaceRules[i].terrain_type;
					if (inFaceRules[i].area_feature)
						faces.back().area_features.insert(inFaceRules[i].area_feature);
				}
			}
		}
	}
	/****************************************************************************************
	 * RINGS (RNG)
	 ****************************************************************************************/
	if (inHasTopo)
	{
		printf("Importing Rings...\n");
		strcpy(thePath, tilePath);
		strcat(thePath, "rng");

		StMemFile			rng(thePath);
		VPF_TableDef		rngDef;
		if (!rng()) { printf("Could not open '%s'\n", thePath); return false; }

		if (!ReadVPFTableHeader(rng, rngDef)) { printf("Could not read VPF header for '%s'\n", thePath); return false; }

		int face_id, start_edge_index;

		if (!FindColumn(rngDef, "face_id", face_id, require_Int, thePath)) return false;
		if (!FindColumn(rngDef, "start_edge", start_edge_index, require_Int, thePath)) return false;

		for (VPFTableIterator rngIter(rng, rngDef); !rngIter.Done(); rngIter.Next())
		{
			int	the_face, the_edge;
			if (!rngIter.GetNthFieldAsInt(face_id, the_face)) { printf("Could not read face_id on row %d of '%s'\n", expected_row, thePath); return false; }
			if (!rngIter.GetNthFieldAsInt(start_edge_index, the_edge)) { printf("Could not start_edge face_id on row %d of '%s'\n", expected_row, thePath); return false; }

			if (the_edge != 0x80000000)
				faces[the_face-1].edg_index.push_back(the_edge-1);
		}
	}

	/****************************************************************************************
	 * TOPOLOGICAL IMPORT
	 ****************************************************************************************/
	if (inHasTopo)
	{
		WTPM_LineVector		wtpm_lines(lines.size());
		WTPM_NodeVector		wtpm_nodes(nodes.size());
		WTPM_FaceVector		wtpm_faces(faces.size());

		printf("Setting up pointers...\n");
		for (i = 0; i < lines.size(); ++i)
		{
			wtpm_lines[i] = &lines[i];

			VPF_Line&	me = 	lines[i];
			VPF_Line&	left =	lines[lines[i].left_edg_index];
			VPF_Line&	right =	lines[lines[i].right_edg_index];

			lines[i].leftFace = &faces[lines[i].left_fac_index];
			lines[i].rightFace = &faces[lines[i].right_fac_index];

			lines[i].startNode = &nodes[lines[i].start_cnd_index];
			lines[i].endNode = &nodes[lines[i].end_cnd_index];

			lines[i].startNode->location = lines[i].start_node_pt;
			lines[i].endNode->location = lines[i].end_node_pt;

			// this is a little bit tricky - VPF's idea of "right and left" are OPPOSITE of ours -
			// basically the one-way pointers are reversed.

			// Figure out linkage from my "next left" ptr.
			if (left.end_cnd_index == me.start_cnd_index && left.start_cnd_index == me.start_cnd_index)
			{
				if (me.left_fac_index == left.left_fac_index)
					left.nextLeft = WTPM_DirectedLinePtr(&me, true);
				else
					left.nextRight = WTPM_DirectedLinePtr(&me, true);
			}
			else if (left.end_cnd_index == me.start_cnd_index)
			{
				left.nextLeft = WTPM_DirectedLinePtr(&me, true);
			}
			else if (left.start_cnd_index == me.start_cnd_index)
			{
				left.nextRight = WTPM_DirectedLinePtr(&me, true);
			}
			else
			{
				printf("ERROR: left edge does not share a vertex with my start node.\n");
			}

			// Figure out linkage from my "next right" ptr
			if (right.end_cnd_index == me.end_cnd_index && right.start_cnd_index == me.end_cnd_index)
			{
				if (me.right_fac_index == right.right_fac_index)
					right.nextRight = WTPM_DirectedLinePtr(&me, false);
				else
					right.nextLeft = WTPM_DirectedLinePtr(&me, false);
			}
			else if (right.end_cnd_index == me.end_cnd_index)
			{
				right.nextLeft = WTPM_DirectedLinePtr(&me, false);
			}
			else if (right.start_cnd_index == me.end_cnd_index)
			{
				right.nextRight = WTPM_DirectedLinePtr(&me, false);
			}
			else
			{
				printf("ERROR: right edge does not share a vertex with my end node.\n");
			}
		}

		for (i = 0; i < faces.size(); ++i)
		{
			wtpm_faces[i] = &faces[i];
			if (faces[i].edg_index.empty() && !faces[i].isWorld)
				printf("ERROR - NO EDGES FOR FACE %i\n", i);
			for (int k = 0; k < faces[i].edg_index.size(); ++k)
			{
				// This is NOT what you'd expect - for the world poly we do use
				// the outer ring slot for a hole.  I do NOT know what I was thinking.
				if (k == 0)
					faces[i].outerRing = &lines[faces[i].edg_index[k]];
				else
					faces[i].innerRings.push_back(&lines[faces[i].edg_index[k]]);

			}
		}

		for (i = 0, j = 0; i < nodes.size(); ++i)
		{
			if (!nodes[i].skip)
				wtpm_nodes[j++] = &nodes[i];
	//		if ((nodes[i].edg_index) < 0 || (nodes[i].edg_index) >= lines.size())
	//			printf("Bad key %d on node %d.\n", nodes[i].edg_index, i);
		}
		wtpm_nodes.resize(j);


		printf("Building map...\n");
		WTPM_ExportToMap(wtpm_nodes, wtpm_lines, wtpm_faces, ioMap);

		printf("Read %d nodes.\n", nodes.size());
		printf("Read %d lines.\n", lines.size());
		printf("Read %d faces.\n", faces.size());
	}

	/****************************************************************************************
	 * NON-TOPOLOGICAL IMPORT
	 ****************************************************************************************/
	if (!inHasTopo)
	{
		int	slow = 0, fast = 0;
		for (i = 0; i < nodes.size(); ++i)
		{
			nodes[i].pm_vertex = NULL;
		}

		for (i = 0; i < lines.size(); ++i)
		{
			lines[i].startNode = &nodes[lines[i].start_cnd_index];
			lines[i].endNode = &nodes[lines[i].end_cnd_index];

			lines[i].startNode->location = lines[i].start_node_pt;
			lines[i].endNode->location = lines[i].end_node_pt;
		}

		RememberHalfedge	observer;
		observer.attach(ioMap);

		printf("Inserting lines...\n");
		for (i = 0; i < lines.size(); ++i)
		{
			if ((i % 100) == 0)
				ConsoleProgressFunc(0, 1, "Organizing", (float) i / (float) lines.size());

//			if (i == 1800) break;

			for (int  j = 1; j < lines[i].shape.size(); ++j)
			{
				observer.set_slot(&lines[i].pm_edges);
				CGAL::insert_curve(ioMap, Curve_2(Segment_2(
									Point_2(lines[i].shape[j-1].x(),lines[i].shape[j-1].y()),
									Point_2(lines[i].shape[j].x(),lines[i].shape[j].y()))));
			}

			if (lines[i].startNode->pm_vertex == NULL)
				lines[i].startNode->pm_vertex = lines[i].pm_edges.first.front()->opposite()->vertex();
			if (lines[i].endNode->pm_vertex == NULL)
				lines[i].endNode->pm_vertex = lines[i].pm_edges.first.back()->vertex();
		}
		observer.detach();
		printf("Added %d slow segs, %d fast segs.\n", slow, fast);
	}

	/****************************************************************************************
	 * APPLY ATTRIBUTE CRAP
	 ****************************************************************************************/

	printf("Applying attributes to %d faces, %d lines...\n", faces.size(), lines.size());
	for (i = 0; i < faces.size(); ++i)
	{
		if (faces[i].terrain_type != terrain_Natural)
			faces[i].pm_face->data().mTerrainType = faces[i].terrain_type;
		if (!faces[i].area_features.empty())
			faces[i].pm_face->data().mAreaFeature.mFeatType = *faces[i].area_features.begin();
	}

	for (i = 0; i < lines.size(); ++i)
	{
		WTPM_Line::HalfedgeVector::iterator he;

		if (lines[i].he_param != NO_VALUE)
		{
			DebugAssert(lines[i].he_param >= 0 && lines[i].he_param < gTokens.size());
			for (he = lines[i].pm_edges.first.begin(); he != lines[i].pm_edges.first.end(); ++he)
			if ((*he)->data().mDominant)
				(*he)->data().mParams[lines[i].he_param] = 0.0;
			for (he = lines[i].pm_edges.second.begin(); he != lines[i].pm_edges.second.end(); ++he)
			if ((*he)->data().mDominant)
				(*he)->data().mParams[lines[i].he_param] = 0.0;
		}		

		if (inTransTable && lines[i].he_trans_flags)
		{
			int * rule = inTransTable;
			while (*rule)
			{
				int the_rule = *rule++;
				int the_val = *rule++;
				if (lines[i].he_trans_flags == the_rule)
				{
					GISNetworkSegment_t seg;
					seg.mRepType = 0;
					seg.mSourceHeight = seg.mTargetHeight = 0.0;
					seg.mFeatType = the_val;
					for (he = lines[i].pm_edges.first.begin(); he != lines[i].pm_edges.first.end(); ++he)
					if ((*he)->data().mDominant)
						(*he)->data().mSegments.push_back(seg);
					for (he = lines[i].pm_edges.second.begin(); he != lines[i].pm_edges.second.end(); ++he)
					if ((*he)->data().mDominant)
						(*he)->data().mSegments.push_back(seg);
				}
			}
		}
	}



	return true;
}


/*
edge table
  0 id                I    1 P Row Identifier                                     -              -              -
   1 aquecanl.lft_id   I    1 N Line Feature Table ID                              -              -              -
   2 watrcrsl.lft_id   I    1 N Line Feature Table ID                              -              -              -
   3 start_node        I    1 N Start/Left Node                                    -              -              -
   4 end_node          I    1 N End/Right Node                                     -              -              -
   5 right_face        K    1 N Right Face                                         -              -              -
   6 left_face         K    1 N Left Face                                          -              -              -
   7 right_edge        K    1 N Right Edge from End Node                           -              -              -
   8 left_edge         K    1 N Left Edge from Start Node                          -              -              -
   9 coordinates       Z    0 N Coordinates of Edge                                -              -              -

*/