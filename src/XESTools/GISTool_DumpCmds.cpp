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

#include "GISTool_DumpCmds.h"
//#include <sdts++/io/sio_ConverterFactory.h>
//#include <sdts++/container/sc_Record.h>
//#include "SDTSRead.h"
#include "GISTool_Utils.h"
#include "SDTSReadTVP.h"
#include "VPFImport.h"
#include "GISTool_Globals.h"
#include <shapefil.h>
#include "VPFTable.h"
#include "MapDefs.h"
#include "FAA_Obs.h"
#include "ParamDefs.h"
#include "DSFLib.h"
#include "MemFileUtils.h"
#include "XChunkyFileUtils.h"
#if OPENGL_MAP
#include <OpenGL/gl.h>
#include "BitmapUtils.h"
#endif

extern int	PrintDSFFile(const char * inPath, FILE * output, bool print_it);

#if 0
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
#endif

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

static int DoDumpAtomic(const vector<const char *>& args)
{
	MFMemFile *	fi = MemFile_Open(args[0]);
	if(fi)
	{
		XAtomContainer	ac;
		ac.begin = (char *) MemFile_GetBegin(fi);
		ac.end = (char *) MemFile_GetEnd(fi);
		XAtom a;
		if(ac.GetFirst(a))
		do
		{
			char c[4];
			*((int *) c) = a.GetID();
			printf("%c%c%c%c: %d\n", c[0],c[1],c[2],c[3], a.GetContentLength());
		} while (a.GetNext(ac,a));
		else printf("File %s contains no atoms.\n", args[0]);
		MemFile_Close(fi);
		return 0;
	} else {
		fprintf(stderr,"Cannot open file %s\n", args[0]);
		return 1;
	}
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
	int err = 0;
	for (int n = 0; n < args.size(); ++n)
	{
		if(DSFCheckSignature(args[n]) != dsf_ErrOK)
		{
			fprintf(stderr, "DSF Checksum failed for %s.\n", args[n]);
			return 1;
		}
		err = PrintDSFFile(args[n], stdout, gVerbose);
		if (err != 0)
		{
			fprintf(stderr, "Bad DSF %s.\n", args[n]);
			return 1;
		}
	}
	return 0;
}


static int DoDumpVPF(const vector<const char *>& args)
{
	dump_vpf_table(args[0]);
	return 0;
}

/*
static int DoDumpSDTS(const vector<const char *>& args)
{
	dump_sdts(args[0], args[1]);
	return 0;
}
*/

static int DoDumpShapeFile(const vector<const char *>& args)
{
	return dump_shape_file(args[0]);
}

static void print_ccb(Halfedge_handle e)
{
	Halfedge_handle circ = e, stop = e;
	int ctr = 0;
	do {
		++ctr;
		circ = circ->next();
	} while (circ != stop);
	circ = e, stop = e;
	printf("%d\n", ctr);
	do {
		printf("%lf %lf\n", CGAL::to_double(circ->target()->point().x()), CGAL::to_double(circ->target()->point().y()));
		circ = circ->next();
	} while (circ != stop);
}

static int DoDumpMap(const vector<const char *>& args)
{
	for (Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		printf("%d\n", distance(f->holes_begin(),f->holes_end()) + 1);
		if (f->is_unbounded())
			printf("0\n");
		else
			print_ccb(f->outer_ccb());
		for (Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
			print_ccb(*h);
	}
	return 0;
}

#if OPENGL_MAP
static int DoScreenShot(const vector<const char *>& args)
{
	static	int	rev = 1;

	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	ImageInfo	cap;
	int err = CreateNewBitmap(viewport[2], viewport[3], 3, &cap);
	if (err == 0)
	{
#if APL
		glReadPixels(0, 0, viewport[2], viewport[3], GL_BGR, GL_UNSIGNED_BYTE, cap.data);
#else
		glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, cap.data);
#endif
		WriteBitmapToPNG(&cap, args[0], NULL, 0);
		DestroyBitmap(&cap);
	}

}
#endif

static	GISTool_RegCmd_t		sDumpCmds[] = {
#if OPENGL_MAP
{ "-screenshot",	1,	1, DoScreenShot ,		"Screenshot of current file.", "" },
#endif
{ "-dumpatomic",	1,	1, DoDumpAtomic ,		"Dump atom table of atomic file.", "" },
{ "-dumpobs", 		0, 0, DoDumpObs, 		"Dump current FAA objects.", "" },
{ "-dumpdsf", 		1, -1, DoDumpDSF, 		"Dump contents of a DSF file.", "" },
{ "-dumpvpf", 		1, 1, DoDumpVPF, 		"Dump a VPF table.", "" },
//{ "-dumpsdts", 		2, 2, DoDumpSDTS, 		"Dump an SDTS module.", "" },
{ "-dumpshp",		1, 1, DoDumpShapeFile,	"Dump an ESRI shape file.", "" },
{ "-dumpmap",		0, 0, DoDumpMap,		"Dump current map as text.", "" },

{ 0, 0, 0, 0, 0, 0 }
};


void RegisterDumpCmds(void)
{
	GISTool_RegisterCommands(sDumpCmds);
}
