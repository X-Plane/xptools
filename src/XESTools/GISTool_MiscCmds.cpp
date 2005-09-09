#include "GISTool_MiscCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PerfUtils.h"
#include "SceneryPackages.h"
#include "DEMTables.h"

#include "MapDefs.h"
#include "GISUtils.h"

static double calc_water_area(void)
{

	double total = 0.0;
	for (Pmwx::Face_iterator face = gMap.faces_begin(); face != gMap.faces_end(); ++face)
	if (!face->is_unbounded())
	if (face->IsWater())
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			total += (Vector2(stop->source()->point(), circ->source()->point()).signed_area(
					  Vector2(stop->source()->point(), circ->target()->point())));
			++circ;
		} while (circ != stop);
		
		for (Pmwx::Holes_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
		{
			circ = stop = *hole;
			do {
				total += (Vector2(stop->source()->point(), circ->source()->point()).signed_area(
						  Vector2(stop->source()->point(), circ->target()->point())));
				++circ;
			} while (circ != stop);
		}
	}
	return total;
}


static int	DoObjToConfig(const vector<const char *>& args)
{
	const char * ouf = args[args.size()-1];
	FILE * fi = strcmp(ouf, "-") ? fopen(ouf, "w") : stdout;
	if (fi == NULL)
	{
		fprintf(stderr, "Could not open %s for writing.\n", args[args.size()-1]);
		return 1;
	}
	
	
	for (int n = 0;  n < (args.size()-1); ++n)
	{
		SpreadsheetForObject(args[n], fi);
	}	
	
	if (fi != stdout)
		fclose(fi);
	
	return 0;
}

static int	DoCheckSpreadsheet(const vector<const char *>& args)
{
	CheckDEMRuleCoverage();
	return 0;
}

static int DoCheckWaterConform(const vector<const char *>& args)
{
	// XES source, SHP source, output
	
	FILE * im = fopen(args[2], "wb");
	
	for (int y = 30; y < 40; ++y)
	for (int x = -130; x < -120; ++x)
	{
		char	shp_fname[255];
		char	xes_fname[255];
		sprintf(xes_fname,"%s%+03d%+04d/%+03d%+04d.xes", args[0], latlon_bucket(y), latlon_bucket(x), y, x);
		sprintf(shp_fname,"%s%+03d%+04d/%+03d%+04d.shp", args[1], latlon_bucket(y), latlon_bucket(x), y, x);
		
		vector<const char *> cmd;
		cmd.push_back("-load");
		cmd.push_back(xes_fname);
		if (GISTool_ParseCommands(cmd)) 
		{
			fputc(0, im);
			fputc(0, im);
			fputc(0, im);
			continue;
		}

		double wet_xes = calc_water_area();		
		
		cmd[0] = "-shapefile";
		cmd[1] = shp_fname;

		if (GISTool_ParseCommands(cmd)) 
		{
			fputc(0, im);
			fputc(0, im);
			fputc(0, im);
			continue;
		}

		double wet_shp = calc_water_area();		
		
		double err = fabs(wet_shp - wet_xes);
		double sat = max(wet_shp, wet_xes);
		double rel = (sat == 0.0) ? 0.0 : (err / sat);

		printf("%d,%d: SHP=%lf,XES=%lf   err=%lf, sat=%lf, rel=%lf\n", x, y, wet_shp, wet_xes, err, sat, rel);
		
		unsigned char err_c = err * 255.0 * 1000.0;
		unsigned char sat_c = sat * 255.0 * 1.0;
		unsigned char rel_c = rel * 255.0 * 10.0;
		
		fputc(err_c, im);
		fputc(sat_c, im);
		fputc(rel_c, im);
	}
	fclose(im);
	return 1;
}

static	GISTool_RegCmd_t		sMiscCmds[] = {
{ "-obj2config", 	2, -1, 	DoObjToConfig, 			"Import SDTS VTP vector map.", "" },
{ "-checkdem",		0, 0,  DoCheckSpreadsheet,		"Check spreadsheet coverage.", "" },
{ "-checkwaterconform", 3, 3, DoCheckWaterConform, 	"Check water matchup", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterMiscCmds(void)
{
	GISTool_RegisterCommands(sMiscCmds);
}
