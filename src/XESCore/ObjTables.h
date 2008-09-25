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
#ifndef OBJTABLES_H
#define OBJTABLES_H

/*
 *	HOW DOES THIS WORK?
 *
 *	We have one record for an object of fac.  BUT for objects with var-height, the spreadsheet lists the range and the objs are generated on load-in.
 *	Thus the spreadsheet really contains like 20 radio towers.
 *
 */

enum {
	rep_Obj,
	rep_Fac
};

struct	RepInfo_t {

	// RULEZ
	int		feature;
	int		terrain;

//	float	temp_min;
//	float	temp_max;
//	float	rain_min;
//	float	rain_max;
//	float	slope_min;
//	float	slope_max;
//	float	urban_dense_min;
//	float	urban_dense_max;
//	float	urban_radial_min;
//	float	urban_radial_max;
//	float	urban_trans_min;
//	float	urban_trans_max;

	// AUTOGEN
//	float	freq;
//	int		max_num;

	int		road;
	int		fill;

	// OBJECT
	int		obj_type;
	int		obj_name;

	// Dims
	float	width_min;
	float	width_max;
	float	depth_min;
	float	depth_max;
	float	height_min;
	float	height_max;
};
typedef	vector<RepInfo_t>							RepTable;
typedef hash_map<int, pair<int, int> >				RepTableTerrainIndex;

typedef hash_map<int, int>							RepFeatureIndex;
typedef multimap<float, int, greater<float> >		RepAreaIndex;
typedef hash_map<int, int>							RepUsageTable;
extern	RepTable		gRepTable;			// This is the actual master table
extern	RepFeatureIndex	gRepFeatureIndex;	// This indexes based on feature type (the object enum.)

//extern	RepAreaIndex	gFacadeAreaIndex;	// This sorts facades by min area, big to small
//extern	RepAreaIndex	gObjectAreaIndex;	// This sorts objects by area, big to small
extern	RepUsageTable			gRepUsage;			// This is a table of usages.
extern	int						gRepUsageTotal;
extern 	RepTableTerrainIndex	gRepTableTerrainIndex;
struct	FeatureInfo {
	float		property_value;
	int			terrain_type;
};
typedef hash_map<int, FeatureInfo>		FeatureInfoTable;
extern	FeatureInfoTable				gFeatures;

void	LoadObjTables(void);

// This routines returns facades that fit this profile sorted from biggest
// to smallest.  Note that they give you table indices, not feature types!
int	QueryUsableFacsBySize(
					// Rule inputs!
					int				feature,
					int				terrain,

//					float			temp,
//					float			rain,
//					float			slope,
//					float			urban_dense,
//					float			urban_radial,
//					float			urban_trans,

					float			inLongSide,
					float			inShortSide,
					float			inTargetHeight,

//					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int *			outResults,
					int				inMaxResults);

// This routine returns objects that fit this profile sorted from biggest
// to smallest.

int QueryUsableObjsBySize(
					// Rule inputs!
					int				feature,
					int				terrain,

//					float			temp,
//					float			rain,
//					float			slope,
//					float			urban_dense,
//					float			urban_radial,
//					float			urban_trans,

					float			inWidth,
					float			inDepth,
					float			inHeightMax,	// If min = max, we want an exact height!

//					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int				road,
					int				fill,

					int *			outResults,
					int				inMaxResults);

void IncrementRepUsage(int inRep);
void ResetUsages(void);
bool IsWellKnownFeature(int inFeat);
bool IsFeatureObject(int inName);
void GetObjTerrainTypes(set<int>& outTypes);

void CheckObjTable(void);

extern	string	gObjPlacementFile;
extern string	gObjLibPrefix;

#endif /* OBJTABLES_H */
