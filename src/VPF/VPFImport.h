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

#ifndef VPFIMPORT_H
#define VPFIMPORT_H

#include "MapDefs.h"

struct	VPF_LineRule_t {
	const char *		table;			// Name of the table that has this attribute
	const char *		attr_column;	// Name of the column with the enum
	const char *		ref_column;		// Name of column in line  table that refers to attr table
	const char *		strval;			// Matching string enum (or null for int)
	int					ival;			// Matching int enum

	int					he_param;		// Param to add to the halfedge.
	int					he_trans_flags;	// Transportation flags to add - the patterns of these decide the actual transportation to add.
};

struct	VPF_FaceRule_t {
	const char *		table;
	const char *		attr_column;
	const char *		ref_column;
	const char *		strval;
	int					ival;

	int					terrain_type;	// Terrain type to add
	int					area_feature;	// Area feature to add
};

bool	VPFImportTopo3(
					const char * 		inCoverageDir,
					const char * 		inTile,
					Pmwx& 				outMap,
					bool				inHasTopo,
					VPF_LineRule_t * 	inLineRules,
					VPF_FaceRule_t * 	inFaceRules,
					int *				inTransportationTable);	// Table of flags -> edge type, 0 means end.

#endif /* VPFIMPORT_H */
