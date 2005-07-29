#include "GISTool_MiscCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PerfUtils.h"
#include "SceneryPackages.h"
#include "DEMTables.h"

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


static	GISTool_RegCmd_t		sMiscCmds[] = {
{ "-obj2config", 	2, -1, 	DoObjToConfig, 			"Import SDTS VTP vector map.", "" },
{ "-checkdem",		0, 0,  DoCheckSpreadsheet,		"Check spreadsheet coverage.", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterMiscCmds(void)
{
	GISTool_RegisterCommands(sMiscCmds);
}
