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
#include "ParamDefs.h"
#include "PerfUtils.h"
#include "ProgressUtils.h"
#include "AssertUtils.h"
#include "XESInit.h"
#include "GISTool_Globals.h"
#include "CompGeomDefs2.h"
#include "GISTool_Utils.h"
#include "GISTooL_ObsCmds.h"
#include "GISTool_DemCmds.h"
#include "GISTool_CoreCmds.h"
#include "GISTool_DumpCmds.h"
#include "GISTool_MiscCmds.h"
#include "GISTool_ProcessingCmds.h"
#include "GISTool_VectorCmds.h"

extern void	SelfTestAll(void);

void	CGALFailure(
        const char* what, const char* expr, const char* file, int line, const char* msg)
{
	printf("Terminating due to a CGAL exception.\n");
	fprintf(stderr,"%s: %s (%s:%d).%s\n",
		what, expr, file, line, msg ? msg : "");
	exit(1);
}        

static int DoHelp(const vector<const char *>& args)
{
	int ok = 0;
	if (args.size() == 0)
		GISTool_PrintHelpSummary();
	for (int n = 0; n < args.size(); ++n)
	{
		string	foo("-");
		foo += args[n];
		if (GISTool_PrintHelpCommand(foo.c_str()))
			ok = 1;
	}	
	return ok;
}

static int DoSelfTest(const vector<const char *>& args)		{	SelfTestAll(); 	return 0; 	}
static int DoVerbose(const vector<const char *>& args)		{	gVerbose = 1;	return 0;	}
static int DoQuiet(const vector<const char *>& args)		{	gVerbose = 0;	return 0;	}
static int DoTiming(const vector<const char *>& args)		{	gTiming = 1;	return 0;	}
static int DoNoTiming(const vector<const char *>& args)		{	gTiming = 0;	return 0;	}
static int DoProgress(const vector<const char *>& args)		{	gProgress = ConsoleProgressFunc;	return 0;	}
static int DoNoProgress(const vector<const char *>& args)	{	gProgress = NULL;					return 0;	}

static	GISTool_RegCmd_t		sUtilCmds[] = {
{ "-help",			0, 1, DoHelp, "Prints help info for a command.", "" },
{ "-verbose",		0, 0, DoVerbose, "Enables loggging messages.", "" },
{ "-quiet",			0, 0, DoQuiet, "Disables logging messages.", "" },
{ "-timing",		0, 0, DoTiming, "Enables performance timing.", "" },
{ "-notiming",		0, 0, DoNoTiming, "Disables performance timing.", "" },
{ "-progress",		0, 0, DoProgress, "Shows progress bars", "" },
{ "-noprogress",	0, 0, DoNoProgress, "Disables progress bars", "" },
{ "-selftest",		0, 0, DoSelfTest, "Self test internal algorithms.", "" },
{ 0, 0, 0, 0, 0, 0 }
};


int	main(int argc, char * argv[])
{
//	char * args[] = { "./GISTool_d", "-vpf", "/Volumes/GIS/data/vmap0/v0noa/vmaplv0/noamer/", "bnd,hydro,trans", "-90", "30", "-validate", "-save", "test.xes" };
//	char * args[] = { "./GISTool_d", "-vpf", "/Volumes/GIS/data/vmap0/v0eur/vmaplv0/eurnasia/", "bnd,hydro,trans", "0", "30", "-validate", "-save", "test.xes" };
//	char * args[] = { "./GISTool_d", "-apt", "/Users/bsupnik/Desktop/AptNav200503XP810/apt.dat", "-quiet", "-apttest" };
//	char * args[] = { "./GISTool", "-vpf", "/Volumes/GIS/data/vmap0/v0noa/vmaplv0/noamer/", "bnd", "-120", "30", "-validate", "-bbox", "-save", "foo.xes" };
//	char * args[] = { "./GISTool", "-extent", "-130", "20", "-60", "50", "-gshhs", "/Volumes/GIS/data/GSHHS/gshhs_1.3/gshhs_c.b", "-bbox", "-save", "foo.xes", "-validate" };
//	char * args[] = { "./GISTool_d", "-apt", "/Volumes/GIS/data/apt.dat", "-apttest" };

//	argv = args;
//	argc = sizeof(args) / sizeof(args[0]);

	if (argc == 1)	SelfTestAll();
	
	int result;
	try {
		
//		StElapsedTime *	total = new StElapsedTime("Total time for operation");

		// Set CGAL to throw an exception rather than just
		// call exit!
		CGAL::set_error_handler(CGALFailure);

		int start_arg = 1;
		{
//			StElapsedTime	timer("Init");
			if (argc < 2 || strcmp(argv[1], "-noinit"))
				XESInit();
			else
				++start_arg;	
		}
		
		GISTool_RegisterCommands(sUtilCmds);

		RegisterDemCmds();
		RegisterCoreCmds();
		RegisterDumpCmds();
		RegisterVectorCmds();
		RegisterProcessingCmds();
		RegisterObsCmds();
		RegisterMiscCmds();
		
		vector<const char *>	args;		
		for (int n = start_arg; n < argc; ++n)
		{
			args.push_back(argv[n]);			
		}
		
		result = GISTool_ParseCommands(args);

//		delete total;

		exit(0);
		return result;	

	} catch (exception& e) {
		fprintf(stderr,"Caught unknown exception %s.  Exiting.\n", e.what());
		exit(1);
	} catch (...) {
		fprintf(stderr,"Caught unknown exception.  Exiting.\n");
		exit(1);
	}
}
