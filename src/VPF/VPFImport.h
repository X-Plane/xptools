#ifndef VPFIMPORT_H
#define VPFIMPORT_H

class	Pmwx;

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
