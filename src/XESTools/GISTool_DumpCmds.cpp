#include "GISTool_DumpCmds.h"
#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/container/sc_Record.h>
#include "SDTSRead.h"
#include "GISTool_Utils.h"
#include "SDTSReadTVP.h"
#include "VPFImport.h"
#include "GISTool_Globals.h"
#include <shapefil.h>
#include "VPFTable.h"
#include "FAA_Obs.h"
#include "ParamDefs.h"

extern void	PrintDSFFile(const char * inPath, FILE * output);


static void	dump_sdts(const char *  ifs_name, const char * modName)
{
	printf("Showiwng %s module %s\n", ifs_name, modName);
	string last_mnemonic;
	string subfield_mnemonic;

	sio_8211_converter_dictionary converters; // hints for reader for binary data

	bool totals = false;

	SDTSDirectory dir(ifs_name, NULL);	/* no ex ref yet! */
	if (!dir.DidLoad())
		return;
		
	AddConverters(dir, converters);

	MFMemFile *	mf = dir.OpenModule(modName);
	if (!mf) return;

 	sc_Record record;


  int records = 0;
	SDTSModuleIterator	iter(mf, &converters);
  while ( !iter.Done() )
    {
      iter.Get( record );

      if ( ! totals )
        {
          cout << record << "\n";
        }

      iter.Increment();
      ++records;
    }
	if (iter.Error())
		fprintf(stderr,"Hit error reading module.\n");

      const char* singular = " record";
      const char* plural   = " records";
      const char* message = (records > 1 || 0 == records ) ? plural : singular;

      cout << ifs_name << " has " << records << message << "\n";

  MemFile_Close(mf);
}

static void dump_vpf_table(const char * inFileName)
{
	MFMemFile * mf = MemFile_Open(inFileName);
	if (!mf) { fprintf(stderr,"Could not open %s\n", inFileName); return; }
	
	VPF_TableDef	def;
	if (ReadVPFTableHeader(mf, def))
	{
		DumpVPFTableHeader(def);
		DumpVPFTable(mf, def);
	} else {
		fprintf(stderr,"File %s not readable as VPF table.\n", inFileName);
	}
	
	MemFile_Close(mf);
}

static int dump_shape_file(const char * inFileName)
{
	SHPHandle file = SHPOpen(inFileName, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Could not open shape file %s\n", inFileName);
		return 1;
	}
	
	int	entityCount, shapeType;
	double	bounds_lo[4], bounds_hi[4];
	
	SHPGetInfo(file, &entityCount, &shapeType, bounds_lo, bounds_hi);
	
	if (gVerbose)
	{
		printf("File contains %d entities of type %d\n", entityCount, shapeType);
		for (int n = 0; n < 4; ++n)
			printf("%lf -> %lf\n", bounds_lo[n], bounds_hi[n]);
	}
	
	for (int n = 0; n < entityCount; ++n)
	{
		SHPObject * obj = SHPReadObject(file, n);
		printf("Obj %d: type=%d, ID=%d, parts=%d, vertices=%d\n",
			n, obj->nSHPType, obj->nShapeId, obj->nParts, obj->nVertices);
		
		for (int i = 0; i < obj->nParts; ++i)
			printf("  Part %d index = %d, type = %d\n",
				i, obj->panPartStart[i], obj->panPartType[i]);
		
		if (gVerbose)
		for (int i = 0; i < obj->nVertices; ++i)
			printf("   Pt %d: %lf, %lf\n",
				i, obj->padfX[i], obj->padfY[i]);
		
		SHPDestroyObject(obj);	
	}
	
	
	SHPClose(file);
	return 0;
}

static int DoDumpObs(const vector<const char *>& args)
{
	for (FAAObsTable::iterator iter = gFAAObs.begin(); iter != gFAAObs.end(); ++iter)
	{
		FAAObs_t&	obs = iter->second;
		printf("   %lf,%lf   %f (%f)    %d (%s)       '%s' (%s)\n",
			obs.lon, obs.lat, obs.msl, obs.agl,
			obs.kind, FetchTokenString(obs.kind),
			obs.kind_str.c_str(), obs.freq.c_str());			
	}
	return 0;
}
		
static int DoDumpDSF(const vector<const char *>& args)
{
	for (int n = 0; n < args.size(); ++n)
	{
		PrintDSFFile(args[n], stdout);
	}
	return 0;
}


static int DoDumpVPF(const vector<const char *>& args)
{
	dump_vpf_table(args[0]);
	return 0;
}
		
static int DoDumpSDTS(const vector<const char *>& args)
{
	dump_sdts(args[0], args[1]);
	return 0;
}

static int DoDumpShapeFile(const vector<const char *>& args)
{
	return dump_shape_file(args[0]);
}

static	GISTool_RegCmd_t		sDumpCmds[] = {
{ "-dumpobs", 		0, 0, DoDumpObs, 		"Dump current FAA objects.", "" },
{ "-dumpdsf", 		1, -1, DoDumpDSF, 		"Dump contents of a DSF file.", "" },
{ "-dumpvpf", 		1, 1, DoDumpVPF, 		"Dump a VPF table.", "" },
{ "-dumpsdts", 		2, 2, DoDumpSDTS, 		"Dump an SDTS module.", "" },
{ "-dumpshp",		1, 1, DoDumpShapeFile,	"Dump an ESRI shape file.", "" },

{ 0, 0, 0, 0, 0, 0 }
};


void RegisterDumpCmds(void)
{
	GISTool_RegisterCommands(sDumpCmds);
}
