#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************************************************************************
 *
 * UTILITIES
 *
 ****************************************************************************************************************************************/


int 	gCountry;
int		gOnce;

const char * gCountryNames[2] = { "WORLD", "US" };
const char * gCountryPrefix[2] = { "/lib/global8/", "/lib/global8/us/" };

typedef struct {
	int 	width;
	int 	depth;
	int 	height1;
	int 	height2;
	int		road;
	int		fill;
	const char *	feature;
}  ObjSpec_t;

const char * gObjs[10000];
int gCtr = 0;
 
void PrintObj(const char * feature, const char * terrain, int variant, int width1, int width2, int depth1, int depth2, int height1, int height2, int road, int fill, const char * name)
{
	int n;	
	for (n = 0; n < gCtr; ++n)
	{
		if (strcmp(gObjs[n], name)==0)
		{
			if (gOnce) return;
			break;
		}
	}
	if (n == gCtr)
	{
		gObjs[gCtr++] = strdup(name);
	}

	char	tbuf[256];
	
	if (terrain && variant)
		sprintf(tbuf, "terrain/%s%d", terrain, variant);
	else if (terrain)
		sprintf(tbuf, "terrain/%s", terrain);
	else
		strcpy(tbuf, "NO_VALUE");
	
	if (width2 != 0)
	{
		printf("FAC_PROP %-20s %-30s %5d %5d %5d %5d %5d %5d %3d %3d %s\n",
			feature, tbuf, width1, width2, depth1, depth2, height1, height2, road, fill, name);		
	} 
	else if (height2 != 0)
	{
		printf("OBS_PROP %-20s %-30s %5d %5s %5d %5s %5d %5d %3d %3d %s\n",
			feature, tbuf, width1, " ", depth1, " ", height1, height2, road, fill, name);		
	} 
	else
	{
		printf("OBJ_PROP %-20s %-30s %5d %5s %5d %5s %5d %5s %3d %3d %s\n",
			feature, tbuf, width1, " ", depth1, " ", height1, " ", road, fill, name);		
	}
} 
 
void PrintHeader(const char * label, const char * comment)
{
	if (gOnce) return;
	printf("# ---------------------- %s    %s\n", label ? label : "Obstacles",comment ? comment : "");	
	printf("#COMMAND %-20s %-30s %5s %5s %5s %5s %5s %5s %3s %3s %s\n",
		"FEATURE", "TERRAIN", "WID1", "WID2", "DEP1", "DEP2", "HGT1", "HGT2", "RD", "FIL", "OBJ");

}

void PrintSpec(const char * suite, const char * terrain, int variant, const ObjSpec_t spec[])
{
	int n;
	int j;
	const char * sterrain;	
	int j1 = 0, j2 = 0;
	if (variant) { j1 = 1; j2 = 4; }
	for (j = j1; j <= j2; ++j)
	{
		n = 0;
		while (spec[n].width != 0)
		{
			char	buf[256];
			int o = 0;
			if (terrain == NULL)  {
				printf("ERROR: NULL TERRAIN\n"); exit(1); }
			if (suite != NULL && suite[0] != 0)
				o += sprintf(buf+o, "%s_", suite);
			if (spec[n].feature)
				o += sprintf(buf+o, "%s_", spec[n].feature);
			o += sprintf(buf+o,"%d_%d", spec[n].width,spec[n].depth);
			if (spec[n].height2 != 0)
				o += sprintf(buf+o,"_%d", spec[n].height2);
			else if (spec[n].height1 != 0)
				o += sprintf(buf+o,"_%d", spec[n].height1);
			if (spec[n].fill && spec[n].road)
				strcat(buf, "a");
			else if (spec[n].fill)
				strcat(buf, "f");
			else
				strcat(buf, "r");				
			PrintObj(spec[n].feature ? spec[n].feature : "NO_VALUE", terrain, j, spec[n].width,0,spec[n].depth,0,spec[n].height1, spec[n].height2, spec[n].road, spec[n].fill, buf);		
			++n;
		}
	}
}


/****************************************************************************************************************************************
 *
 * City specification Types
 *
 * These define a profile of objects to be used for a given city.
 *
 ****************************************************************************************************************************************/

enum {

	spec_TownSq,		// Town - the outlay transition between city and farm/natural
	spec_OutSq,		// Outer-town area - basically residential
	spec_InSq,		// City inner - non-single-unit buildings + sky scrapers limited by height restrictions
	spec_IndSq,		// Industrial
	spec_HillSq,		// Like out, but with hill patterns	
	spec_TownIrr,		// Town - the outlay transition between city and farm/natural
	spec_OutIrr,		// Outer-town area - basically residential
	spec_InIrr,		// City inner - non-single-unit buildings + sky scrapers limited by height restrictions
	spec_IndIrr,		// Industrial
	spec_HillIrr,		// Like out, but with hill patterns	
	spec_Park,		// Parkland and grass/forset areas
	
	spec_Max
};

typedef	void (* TerrainFunc_t)(const char * name, int vari);

void FuncTownSq(const char *, int);
void FuncOutSq(const char *, int);
void FuncInSq(const char *, int);
void FuncIndSq(const char *, int);
void FuncHillSq(const char *, int);
void FuncTownIrr(const char *, int);
void FuncOutIrr(const char *, int);
void FuncInIrr(const char *, int);
void FuncIndIrr(const char *, int);
void FuncHillIrr(const char *, int);
void FuncPark(const char *, int);
void FuncObjs(const char *, const char *, int);

TerrainFunc_t	gFuncs[spec_Max] = { 
	FuncTownSq,
	FuncOutSq,
	FuncInSq,
	FuncIndSq,
	FuncHillSq,
	FuncTownIrr,
	FuncOutIrr,
	FuncInIrr,
	FuncIndIrr,
	FuncHillIrr,
	FuncPark,
};

typedef struct {
	const char * 	name;
	int				kind;
	int				vari;
} TerrainItem_t;

TerrainItem_t terrains[] = {
//------------------------ ICE CITY
{"ice_city",				spec_OutIrr,	1},
{"ice_city",				spec_OutIrr,	1},

//------------------------NORTH2 CITY
{"north2_city_irr_ind",		spec_IndIrr,	1},
{"north2_city_irr_in",		spec_InIrr,		1},
{"north2_city_irr_out",		spec_OutIrr,	1},
{"north2_city_irr_twn",		spec_TownIrr,	1},

//------------------------ NORTH CITY
{"north_city_irr_ind",		spec_IndIrr,	1},
{"north_city_irr_in",		spec_InIrr,		1},
{"north_city_irr_out",		spec_OutIrr,	1},
{"north_city_irr_twn",		spec_TownIrr,	1},

//------------------------ WET CITY
{"wet_city_irr_ind",		spec_IndIrr,	1},
{"wet_city_irr_in",			spec_InIrr,		1},
{"wet_city_irr_out",		spec_OutIrr,	1},
{"wet_city_irr_twn",		spec_TownIrr,	1},


//------------------------ TEMPERATE CITY
{"temp_city_park",			spec_Park,		0},
{"temp_city_golf",			spec_Park,		0},

{"temp_city_irr_ind",		spec_IndIrr,	1},
{"temp_city_irr_in",		spec_InIrr,		1},
{"temp_city_irr_out",		spec_OutIrr,	1},
{"temp_city_irr_twn",		spec_TownIrr,	1},

{"temp_city_sq_ind",		spec_IndSq,		1},
{"temp_city_sq_in",			spec_InSq,		1},
{"temp_city_sq_out",		spec_OutSq,		1},
{"temp_city_sq_twn",		spec_TownSq,	1},

{"temp_cityhill_irr",		spec_HillIrr,	1},
{"temp_cityhill_sq",		spec_HillSq,	1},

//------------------------ DRY CITY	
{"dry_city_park",			spec_Park,		0},
{"dry_city_golf",			spec_Park,		0},

{"dry_city_irr_ind",		spec_IndIrr,		1},
{"dry_city_irr_in",			spec_InIrr,		1},
{"dry_city_irr_out",		spec_OutIrr,		1},
{"dry_city_irr_twn",		spec_TownIrr,		1},

{"dry_city_sq_ind",			spec_IndSq,		1},
{"dry_city_sq_in",			spec_InSq,		1},
{"dry_city_sq_out",			spec_OutSq,		1},
{"dry_city_sq_twn",			spec_TownSq,		1},

{"dry_cityhill_irr",		spec_HillIrr,		1},
{"dry_cityhill_sq",			spec_HillSq,		1},

//------------------------ DESERT CITY
{"des_city_park",			spec_Park,		0},
{"des_city_golf",			spec_Park,		0},

{"des_city_irr_ind",		spec_IndIrr,		1},
{"des_city_irr_in",			spec_InIrr,		1},
{"des_city_irr_out",		spec_OutIrr,		1},
{"des_city_irr_twn",		spec_TownIrr,		1},

{"des_city_sq_ind",			spec_IndSq,		1},
{"des_city_sq_in",			spec_InSq,		1},
{"des_city_sq_out",			spec_OutSq,		1},
{"des_city_sq_twn",			spec_TownSq,		1},

{"des_cityhill_irr",		spec_HillIrr,		1},
{"des_cityhill_sq",			spec_HillSq,		1},
{NULL, spec_Max}
};

/****************************************************************************************************************************************
 *
 * CITY SPEC BUILDERS
 *
 * These functions list the actual objects we must build
 *
 ****************************************************************************************************************************************/

void FuncTownSq(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  Widen smaller pieces to create
		{	90, 40, 0, 0, 1, 0, NULL },			
		{	90, 30, 0, 0, 1, 0, NULL },			
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};

	PrintHeader(t, "(Town-type square)");
	PrintSpec("town_sq", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncOutSq(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	 40,  20,  40, 0,    1, 0, NULL },	// B2-B
		{	 20,  20,  40, 0,    1, 0, NULL },
	
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  Widen smaller pieces to create
		{	90, 30, 0, 0, 1, 0, NULL },			// sparseness
		{	90, 20, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	60, 20, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },
		{	30, 20, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(OuterCity-type irregular)");
	PrintSpec("out_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
	PrintHeader(t, "(OuterCity-type square)");
	PrintSpec("out_sq", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncInSq(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	 45,  30, 150, 0,    1, 0, NULL },	// S1-H group
		{	 65,  25, 100, 0,    1, 0, NULL },	// S2-A group
		{	 45,  20, 100, 0,    1, 0, NULL },	// S2-A group
		{	 60,  30,  80, 0,    1, 0, NULL },	// S2-B group
		{	 25,  25,  80, 0,    1, 0, NULL },
		{	 60,  30,  65, 0,    1, 0, NULL },	// B2-E
		{	 30,  25,  65, 0,    1, 0, NULL },
		{	 60,  30,  55, 0,    1, 0, NULL },	// B2-D
		{	 30,  25,  55, 0,    1, 0, NULL },
		{	 80,  30,  45, 0,    1, 0, NULL },	// B2-C
		{	 30,  20,  45, 0,    1, 0, NULL },
		{	 40,  20,  40, 0,    1, 0, NULL },	// B2-B
		{	 20,  20,  40, 0,    1, 0, NULL },
	
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  High granularity for precise placement.
		{	120, 15, 0, 0, 1, 0, NULL },
		{	90, 30, 0, 0, 1, 0, NULL },
		{	90, 15, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	60, 15, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },
		{	30, 15, 0, 0, 1, 0, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(InnerCity-type - square)");
	PrintSpec("in_sq",t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncIndSq(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	200, 200, 0, 0, 1, 0, NULL },
		{	200, 100, 0, 0, 1, 0, NULL },
		{	100, 60, 0, 0, 1, 0, NULL },
		{	60, 60, 0, 0, 1, 0, NULL },
		{	90, 30, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(Industrial-type square)");
	PrintSpec("ind_sq", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncHillSq(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	120, 30, 0, 0, 1, 0, NULL },		// Smaller buildings for hill-friendliness
		{	90, 30, 0, 0, 1, 0, NULL },			// Smaller buildings for hill-friendliness
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 1, NULL },	// Small objs backin Europe for hill!
		{	50, 50, 0, 0, 1, 1, NULL },		

		{	0,	0,	0, 0, 1, 1, NULL }
	};

	PrintHeader(t, "(Hill-type square)");
	PrintSpec("hill_sq",t, vari, gCountry ? the_spec_us : the_spec_world);
}


void FuncTownIrr(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  Widen smaller pieces to create
		{	90, 40, 0, 0, 1, 0, NULL },			
		{	90, 30, 0, 0, 1, 0, NULL },			
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(OuterCity-type irregular)");
	PrintSpec("out_irr", t, vari, gCountry ? the_spec_us : the_spec_world);

	PrintHeader(t, "(Town-type Irrgular)");
	PrintSpec("town_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncOutIrr(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	 40,  20,  40, 0,    1, 0, NULL },	// B2-B
		{	 20,  20,  40, 0,    1, 0, NULL },
	
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  Widen smaller pieces to create
		{	90, 30, 0, 0, 1, 0, NULL },			// sparseness
		{	90, 20, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	60, 20, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },
		{	30, 20, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(OuterCity-type irregular)");
	PrintSpec("out_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncInIrr(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	 45,  30, 150, 0,    1, 0, NULL },	// S1-H group
		{	 65,  25, 100, 0,    1, 0, NULL },	// S2-A group
		{	 45,  20, 100, 0,    1, 0, NULL },	// S2-A group
		{	 60,  30,  80, 0,    1, 0, NULL },	// S2-B group
		{	 25,  25,  80, 0,    1, 0, NULL },
		{	 60,  30,  65, 0,    1, 0, NULL },	// B2-E
		{	 30,  25,  65, 0,    1, 0, NULL },
		{	 60,  30,  55, 0,    1, 0, NULL },	// B2-D
		{	 30,  25,  55, 0,    1, 0, NULL },
		{	 80,  30,  45, 0,    1, 0, NULL },	// B2-C
		{	 30,  20,  45, 0,    1, 0, NULL },
		{	 40,  20,  40, 0,    1, 0, NULL },	// B2-B
		{	 20,  20,  40, 0,    1, 0, NULL },
	
		{	200, 200, 0, 0, 1, 0, NULL },		// Splat a huge block - use for big-box or other meta-areas
		{	120, 30, 0, 0, 1, 0, NULL },		// Now build long-thin pieces.  High granularity for precise placement.
		{	120, 15, 0, 0, 1, 0, NULL },
		{	90, 30, 0, 0, 1, 0, NULL },
		{	90, 15, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	60, 15, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },
		{	30, 15, 0, 0, 1, 0, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};
	PrintHeader(t, "(InnerCity-type irregular)");
	PrintSpec("in_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncIndIrr(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	200, 200, 0, 0, 1, 0, NULL },
		{	200, 100, 0, 0, 1, 0, NULL },
		{	100, 60, 0, 0, 1, 0, NULL },
		{	60, 60, 0, 0, 1, 0, NULL },
		{	90, 30, 0, 0, 1, 0, NULL },
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	500, 500, 0, 0, 1, 1, NULL },		// Europe - big blocks to try to make a pseudo radial grid wthin highways.
		{	500, 250, 0, 0, 1, 1, NULL },
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 0, NULL },		// Hack: do NOT use a smallest block on the inside..makes it look too atomized.
//		{	60, 60, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 1, 1, NULL }
	};

	PrintHeader(t, "(Industrial-type Irregular)");
	PrintSpec("ind_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncHillIrr(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	120, 30, 0, 0, 1, 0, NULL },		// Smaller buildings for hill-friendliness
		{	90, 30, 0, 0, 1, 0, NULL },			// Smaller buildings for hill-friendliness
		{	60, 30, 0, 0, 1, 0, NULL },
		{	30, 30, 0, 0, 1, 0, NULL },

		{	60, 60, 0, 0, 0, 1, NULL },			// Vegetation
		{	60, 30, 0, 0, 0, 1, NULL },
		{	30, 30, 0, 0, 0, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	250, 250, 0, 0, 1, 1, NULL },
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 1, NULL },	// Small objs backin Europe for hill!
		{	50, 50, 0, 0, 1, 1, NULL },		

		{	0,	0,	0, 0, 1, 1, NULL }
	};

	PrintHeader(t, "(Hill-type Irregular)");
	PrintSpec("hill_irr", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncPark(const char * t, int vari)
{
	static ObjSpec_t	the_spec_us[] = {
		{	500, 250, 0, 0, 1, 1, NULL },		// Park-like vevgtation and stuff
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 60, 0, 0, 1, 1, NULL },
		{	90, 30, 0, 0, 1, 1, NULL },
		{	60, 30, 0, 0, 1, 1, NULL },
		{	0,	0,	0, 0, 1, 1, NULL }
	};

	static ObjSpec_t	the_spec_world[] = {
		{	250, 250, 0, 0, 1, 1, NULL },	// Simplified european splat
		{	250, 120, 0, 0, 1, 1, NULL },
		{	120, 50, 0, 0, 1, 1, NULL },

		{	0,	0,	0, 0, 0, 0, NULL }
	};

	PrintHeader(t, "(Park-type)");

	PrintSpec("park", t, vari, gCountry ? the_spec_us : the_spec_world);
}

void FuncObjs(const char * s, const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
	{  10,  10, 20, 650,1,0, "feat_RadioTower" 	},
	{   5,   5, 20, 650,1,0, "feat_RadioTower" 	},	// 0-628
	{  40,  40, 20, 200,1,0, "feat_Crane" 	 	},	// 9-192
	{  50,  40, 20, 600,1,0, "feat_Building"	},	// 2-527
	{  50,  40, 20, 140,1,0, "feat_Windmill"	},	// 11-123
	{ 100, 100, 20, 140,1,0, "feat_Refinery"	},	// 49-113
	{ 100, 100, 20, 350,1,0, "feat_Tank"		},	// 0-334
	{ 100, 100, 20, 400,1,0, "feat_Smokestack"  },	// 7-381
	{ 100, 100, 20, 400,1,0, "feat_Smokestacks" },	// 12-366
	{ 100, 100, 20, 300,1,0, "feat_Plant"		},	// 7-251

	{  50,  50, 20, 350,1,0, "feat_CoolingTower"},	// 41-306
	{  20,  20, 20, 400,1,0, "feat_Monument"	},	// 13-350
/*
                      feat_Dam       9     227
                  feat_Tramway       9     259
                     feat_Pole       0     164
                 feat_Elevator      10     101
                     feat_Arch      52     192
                    feat_Spire      17     137
                     feat_Dome      18      93
                     feat_Sign       3      62
                 feat_RadarASR       0     102
                feat_RadarARSR       0       0
                 feat_Building       2     527
*/
	{ 0, 0, 0, 0,0,0, NULL }
	};
	PrintSpec(s, t, vari, the_spec);
}




int main(int argc, char ** argv)
{
	gOnce = 0;
	if (argc < 2) { fprintf(stderr, "PLEASE ENTER A COUNTRY CODE 0 = world 1 = us\n"); exit(1); }
	int ctr = 1;
	if (strcmp(argv[ctr], "-once")==0) { gOnce = 1; ++ctr; }
	if (ctr >= argc) { fprintf(stderr, "PLEASE ENTER A COUNTRY CODE 0 = world 1 = us\n"); exit(1); } 
	gCountry = atoi(argv[ctr]);	
	printf("## OBJECTS FOR COUNTRY %s\n",gCountryNames[gCountry]);
	printf("OBJ_PREFIX %s\n", gCountryPrefix[gCountry]);
	int		t = 0;
	while (terrains[t].kind != spec_Max)
	{
		FuncObjs("", terrains[t].name, terrains[t].vari);
		gFuncs[terrains[t].kind](terrains[t].name, terrains[t].vari);
		++t;
	}
	printf("\n# Requires %d unique objects.\n", gCtr);
	return 0;
}


