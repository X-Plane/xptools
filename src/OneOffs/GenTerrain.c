#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************************************************************************
 *
 * UTILITIES
 *
 ****************************************************************************************************************************************/



typedef struct {
	int 	width;
	int 	depth;
	int 	height1;
	int 	height2;
	const char *	feature;
}  ObjSpec_t;


 
void PrintObj(const char * feature, const char * terrain, int variant, int width1, int width2, int depth1, int depth2, int height1, int height2, const char * name)
{
	char	tbuf[256];
	
	if (terrain && variant)
		sprintf(tbuf, "terrain/%s%d", terrain, variant);
	else if (terrain)
		sprintf(tbuf, "terrain/%s", terrain);
	else
		strcpy(tbuf, "NO_VALUE");
	
	if (width2 != 0)
	{
		printf("FAC_PROP %-20s %-30s %5d %5d %5d %5d %5d %5d %s\n",
			feature, tbuf, width1, width2, depth1, depth2, height1, height2, name);		
	} 
	else if (height2 != 0)
	{
		printf("OBS_PROP %-20s %-30s %5d %5s %5d %5s %5d %5d %s\n",
			feature, tbuf, width1, " ", depth1, " ", height1, height2, name);		
	} 
	else
	{
		printf("OBJ_PROP %-20s %-30s %5d %5s %5d %5s %5d %5s %s\n",
			feature, tbuf, width1, " ", depth1, " ", height1, " ", name);		
	}
} 
 
void PrintHeader(const char * label, const char * comment)
{
	printf("# ---------------------- %s    %s\n", label ? label : "Obstacles",comment ? comment : "");	
	printf("#COMMAND %-20s %-30s %5s %5s %5s %5s %5s %5s %s\n",
		"FEATURE", "TERRAIN", "WID1", "WID2", "DEP1", "DEP2", "HGT1", "HGT2", "OBJ");

}

void PrintSpec(const char * terrain, int variant, const ObjSpec_t spec[])
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
			o += sprintf(buf+o, "%s", terrain);
			if (spec[n].feature)
				o += sprintf(buf+o, "_%s", spec[n].feature);
			o += sprintf(buf+o,"_%d_%d", spec[n].width,spec[n].depth);
			if (spec[n].height2 == 0)
				o += sprintf(buf+o,"_%d", spec[n].height1);
			PrintObj(spec[n].feature ? spec[n].feature : "NO_VALUE", terrain, j, spec[n].width,0,spec[n].depth,0,spec[n].height1, spec[n].height2, buf);		
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

	spec_Town,		// Town - the outlay transition between city and farm/natural
	spec_Out,		// Outer-town area - basically residential
	spec_In,		// City inner - non-single-unit buildings + sky scrapers limited by height restrictions
	spec_Ind,		// Industrial
	spec_Hill,		// Like out, but with hill patterns	
	spec_Park,		// Parkland and grass/forset areas
	
	spec_Max
};

typedef	void (* TerrainFunc_t)(const char * name, int vari);

void FuncTown(const char *, int);
void FuncOut(const char *, int);
void FuncIn(const char *, int);
void FuncInd(const char *, int);
void FuncHill(const char *, int);
void FuncPark(const char *, int);
void FuncObjs(const char *, int);

TerrainFunc_t	gFuncs[spec_Max] = { 
	FuncTown,
	FuncOut,
	FuncIn,
	FuncInd,
	FuncHill,
	FuncPark,
};

typedef struct {
	const char * 	name;
	int				kind;
	int				vari;
} TerrainItem_t;

TerrainItem_t terrains[] = {
//------------------------ ICE CITY
{"ice_city",				spec_Out,		1},
{"ice_city",				spec_Out,		1},

//------------------------ NORTH CITY
{"north_city_irr_ind",		spec_Ind,		1},
{"north_city_irr_in",		spec_In,		1},
{"north_city_irr_out",		spec_Out,		1},
{"north_city_irr_twn",		spec_Town,		1},

//------------------------ TEMPERATE CITY
{"temp_city_park",			spec_Park,		0},
{"temp_city_park",			spec_Park,		0},
{"temp_city_golf",			spec_Park,		0},

{"temp_city_irr_ind",		spec_Ind,		1},
{"temp_city_irr_in",		spec_In,		1},
{"temp_city_irr_out",		spec_Out,		1},
{"temp_city_irr_twn",		spec_Town,		1},

{"temp_city_sq_ind",		spec_Ind,		1},
{"temp_city_sq_in",			spec_In,		1},
{"temp_city_sq_out",		spec_Out,		1},
{"temp_city_sq_twn",		spec_Town,		1},

{"temp_cityhill_irr",		spec_Hill,		1},
{"temp_cityhill_sq",		spec_Hill,		1},

//------------------------ DRY CITY	
{"dry_city_park",			spec_Park,		0},
{"dry_city_park",			spec_Park,		0},
{"dry_city_golf",			spec_Park,		0},

{"dry_city_irr_ind",		spec_Ind,		1},
{"dry_city_irr_in",			spec_In,		1},
{"dry_city_irr_out",		spec_Out,		1},
{"dry_city_irr_twn",		spec_Town,		1},

{"dry_city_sq_ind",			spec_Ind,		1},
{"dry_city_sq_in",			spec_In,		1},
{"dry_city_sq_out",			spec_Out,		1},
{"dry_city_sq_twn",			spec_Town,		1},

{"dry_cityhill_irr",		spec_Hill,		1},
{"dry_cityhill_sq",			spec_Hill,		1},

//------------------------ DESERT CITY
{"des_city_park",			spec_Park,		0},
{"des_city_park",			spec_Park,		0},
{"des_city_golf",			spec_Park,		0},

{"des_city_irr_ind",		spec_Ind,		1},
{"des_city_irr_in",			spec_In,		1},
{"des_city_irr_out",		spec_Out,		1},
{"des_city_irr_twn",		spec_Town,		1},

{"des_city_sq_ind",			spec_Ind,		1},
{"des_city_sq_in",			spec_In,		1},
{"des_city_sq_out",			spec_Out,		1},
{"des_city_sq_twn",			spec_Town,		1},

{"des_cityhill_irr",		spec_Hill,		1},
{"des_cityhill_sq",			spec_Hill,		1},
{NULL, spec_Max}
};

/****************************************************************************************************************************************
 *
 * CITY SPEC BUILDERS
 *
 * These functions list the actual objects we must build
 *
 ****************************************************************************************************************************************/


void FuncTown(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	60, 30, 0, 0, NULL },
		{	40, 30, 0, 0, NULL },
		{	20, 20, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(Town-type)");
	PrintSpec(t, vari, the_spec);
}

void FuncOut(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	120, 30, 0, 0, NULL },
		{	90, 30, 0, 0, NULL },
		{	60, 30, 0, 0, NULL },
		{	30, 30, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(OuterCity-type)");
	PrintSpec(t, vari, the_spec);
}

void FuncIn(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	200, 20, 0, 0, NULL },
		{	100, 20, 0, 0, NULL },
		{	75, 20, 0, 0, NULL },
		{	60, 20, 0, 0, NULL },
		{	50, 20, 0, 0, NULL },
		{	40, 20, 0, 0, NULL },
		{	35, 20, 0, 0, NULL },
		{	25, 20, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(InnerCity-type)");
	PrintSpec(t, vari, the_spec);
}

void FuncInd(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	300, 300, 0, 0, NULL },
		{	200, 200, 0, 0, NULL },
		{	100, 100, 0, 0, NULL },
		{	100, 50, 0, 0, NULL },
		{	100, 30, 0, 0, NULL },
		{	50, 30, 0, 0, NULL },
		{	30, 30, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(Industrial-type)");
	PrintSpec(t, vari, the_spec);
}

void FuncHill(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	30, 30, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(Hill-type)");
	PrintSpec(t, vari, the_spec);
}

void FuncPark(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
		{	200, 200, 0, 0, NULL },
		{	100, 100, 0, 0, NULL },
		{	50, 50, 0, 0, NULL },
		{	50, 20, 0, 0, NULL },
		{	30, 30, 0, 0, NULL },
		{	0,	0,	0, 0, NULL }
	};
	PrintHeader(t, "(Park-type)");

	PrintSpec(t, vari, the_spec);
}

void FuncObjs(const char * t, int vari)
{
	static ObjSpec_t	the_spec[] = {
	{  10,  10, 20, 650, "feat_RadioTower" 	},
	{   5,   5, 20, 650, "feat_RadioTower" 	},	// 0-628
	{  40,  40, 20, 200, "feat_Crane" 	 	},	// 9-192
	{  50,  40, 20, 600, "feat_Building"	},	// 2-527
	{  50,  40, 20, 140, "feat_Windmill"	},	// 11-123
	{ 100, 100, 20, 140, "feat_Refinery"	},	// 49-113
	{ 100, 100, 20, 350, "feat_Tank"		},	// 0-334
	{ 100, 100, 20, 400, "feat_Smokestack"  },	// 7-381
	{ 100, 100, 20, 400, "feat_Smokestacks" },	// 12-366
	{ 100, 100, 20, 300, "feat_Plant"		},	// 7-251

	{  50,  50, 20, 350, "feat_CoolingTower"},	// 41-306
	{  20,  20, 20, 400, "feat_Monument"	},	// 13-350
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
	{ 0, 0, 0, 0, NULL }
	};
	PrintSpec(t, vari, the_spec);
}




int main(int argc, char ** argv)
{
	int		t = 0;
	while (terrains[t].kind != spec_Max)
	{
		gFuncs[terrains[t].kind](terrains[t].name, terrains[t].vari);
		FuncObjs(terrains[t].name, terrains[t].vari);
		++t;
	}
	return 0;
}


