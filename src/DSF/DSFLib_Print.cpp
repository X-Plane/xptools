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
#ifndef USE_MEM_FILE
#define USE_MEM_FILE 1
#endif

#include "DSFLib.h"
#include "DSFDefs.h"
#if USE_MEM_FILE
#include "MemFileUtils.h"
#endif

#define PRINT_IT 0
#define PRINT_IT_DEF 1
#define CHECK_IT 0
#define OBJ_HISTO 1

static	int sDSF_Patches = 0;
static	int sDSF_Tris = 0;
static	int sDSF_Polys = 0;
static	int sDSF_Objs = 0;
static	int sDSF_Chains = 0;
static	int sDSF_ShapePoints = 0;

static	double	sWest = 180.0;
static	double	sEast = -180.0;
static	double	sNorth = -90.0;
static	double	sSouth = 90.0;

static int sBad = 0;
#if CHECK_IT

static	int	sDSF_CheckType = -1;
static	int sDSF_VertNum = 0;
static	double	sTri_SaveLast[2];
static	double	sTri_SaveFirst[2];
#endif

#if OBJ_HISTO

static vector<string>	obj_names;
static vector<int>		obj_usages;
static int				obj_total;

#endif

#define	REF	(FILE *) inRef

bool DSFPrint_NextPass(int pass, void * ref)
{
	return 1;
}

void DSFPrint_AcceptTerrainDef(const char * inPartialPath, void * inRef)
{
#if PRINT_IT_DEF
	fprintf(REF, "Terrain Def: %s\n", inPartialPath);
#endif
}

void DSFPrint_AcceptObjectDef(const char * inPartialPath, void * inRef)
{
#if OBJ_HISTO
obj_names.push_back(inPartialPath);
obj_usages.push_back(0);
#endif

#if PRINT_IT_DEF
	fprintf(REF, "Object Def: %s\n", inPartialPath);
#endif
}

void DSFPrint_AcceptPolygonDef(const char * inPartialPath, void * inRef)
{
#if PRINT_IT_DEF
	fprintf(REF, "Polygon Def: %s\n", inPartialPath);
#endif
}

void DSFPrint_AcceptNetworkDef(const char * inPartialPath, void * inRef)
{
#if PRINT_IT_DEF
	fprintf(REF, "Network Def: %s\n", inPartialPath);
#endif
}

void DSFPrint_AcceptProperty(const char * inProp, const char * inValue, void * inRef)
{
	if (strcmp(inProp,"sim/west")==0)	sWest = atoi(inValue);
	if (strcmp(inProp,"sim/east")==0)	sEast = atoi(inValue);
	if (strcmp(inProp,"sim/north")==0)	sNorth = atoi(inValue);
	if (strcmp(inProp,"sim/south")==0)	sSouth = atoi(inValue);
#if PRINT_IT_DEF
	fprintf(REF, "Property %s=%s\n", inProp, inValue);
#endif
}

static	int	dsf_print_depth = 0;


void DSFPrint_BeginPatch(
				unsigned int	inTerrainType,
				double 			inNearLOD,
				double 			inFarLOD,
				unsigned char	inFlags,
				int				depth,
				void *			inRef)
{
	dsf_print_depth = depth;
#if PRINT_IT
	fprintf(REF,"Begin patch terrain=%d,LOD=[%f-%f],flags=0x%02d,depth=%d\n",
		inTerrainType, inNearLOD, inFarLOD, inFlags, depth);
#endif
}

void DSFPrint_BeginPrimitive(int type, void * inRef)
{
#if CHECK_IT
	sDSF_CheckType = type;
	sDSF_VertNum = 0;
#endif
#if PRINT_IT
	fprintf(REF, "  Primitive type=%d\n", type);
#endif
}

void DSFPrint_AddPatchVertex(double * inData, void * inRef)
{
#if PRINT_IT
	fprintf(REF, "       ");
	for (int n = 0; n < dsf_print_depth; ++n)
		fprintf(REF, "%lf    ", inData[n]);
	fprintf(REF, "\n");
#endif
	++sDSF_Tris;

	if (inData[0] < sWest || inData[0] > sEast ||
		inData[1] < sSouth || inData[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inData[0], inData[1]);
		sBad = true;
	}

#if CHECK_IT
	// DSF triangle validity checks.
	switch(sDSF_CheckType) {
	case dsf_Tri:
		// If we are a tri, no set of three can be the same.
		if (sDSF_VertNum % 3)
		{
			if (sTri_SaveLast[0] == inData[0] &&
				sTri_SaveLast[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri (previous).\n");
			}

			if (sTri_SaveFirst[0] == inData[0] &&
				sTri_SaveFirst[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri (first).\n");
			}

		} else {
			sTri_SaveFirst[0] = inData[0];
			sTri_SaveFirst[1] = inData[1];
		}
		sTri_SaveLast[0] = inData[0];
		sTri_SaveLast[1] = inData[1];
		break;

	case dsf_TriStrip:
		if (sDSF_VertNum > 0)
		{
			if (sTri_SaveLast[0] == inData[0] &&
				sTri_SaveLast[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri strip (back 1).\n");
			}
		}
		if (sDSF_VertNum > 1)
		{
			if (sTri_SaveFirst[0] == inData[0] &&
				sTri_SaveFirst[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri strip (back 2).\n");
			}
		}

		sTri_SaveFirst[0] = sTri_SaveLast[0];
		sTri_SaveFirst[1] = sTri_SaveLast[1];
		sTri_SaveLast[0] = inData[0];
		sTri_SaveLast[1] = inData[1];
		break;

	case dsf_TriFan:
		if (sDSF_VertNum > 2)
		{
			if (sTri_SaveLast[0] == inData[0] &&
				sTri_SaveLast[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri fan adjacent.\n");
			}

			if (sTri_SaveFirst[0] == inData[0] &&
				sTri_SaveFirst[1] == inData[1])
			{
				fprintf(stderr,"DSF Duplicate vertex check failed on tri fan adjacent.\n");
			}
		}
		if (sDSF_VertNum == 0)
		{
			sTri_SaveFirst[0] = inData[0];
			sTri_SaveFirst[1] = inData[1];
		}
		sTri_SaveLast[0] = inData[0];
		sTri_SaveLast[1] = inData[1];
		break;
	}

	sDSF_VertNum++;
#endif
}

void DSFPrint_EndPrimitive(
				void *			inRef)
{
#if PRINT_IT
	fprintf(REF, "End of primitive.\n");
#endif
}


void DSFPrint_EndPatch(
				void *			inRef)
{
	++sDSF_Patches;
#if PRINT_IT
	fprintf(REF, "End of patch.\n");
#endif
}

void DSFPrint_AddObject(
				unsigned int	inObjectType,
				double			inCoordinates[2],
				double			inRotation,
				void *			inRef)
{
#if OBJ_HISTO
	obj_usages[inObjectType]++;
	obj_total++;
#endif
	if (inCoordinates[0] < sWest || inCoordinates[0] > sEast ||
		inCoordinates[1] < sSouth || inCoordinates[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inCoordinates[0], inCoordinates[1]);
		sBad = true;
	}

#if PRINT_IT
	fprintf(REF, "Got object type %d, loc %lf,%lf rotate %lf\n",
		inObjectType, inCoordinates[0],inCoordinates[1],inRotation);
#endif
	++sDSF_Objs;
}

void DSFPrint_BeginSegment(
				unsigned int	inNetworkType,
				unsigned int	inNetworkSubtype,
				unsigned int	inStartNodeID,
				double			inCoordinates[6],
				bool			inCurved,
				void *			inRef)
{
	if (inCoordinates[0] < sWest || inCoordinates[0] > sEast ||
		inCoordinates[1] < sSouth || inCoordinates[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inCoordinates[0], inCoordinates[1]);
		sBad = true;
	}
	++sDSF_Chains;
#if PRINT_IT
	fprintf(REF,"Start segment type=%d,subtype=%d, from %d ",
		inNetworkType, inNetworkSubtype, inStartNodeID);
	if (inCurved)
		fprintf(REF,"%f,%f,%f (%f,%f,%f)\n",inCoordinates[0],inCoordinates[1],inCoordinates[2],inCoordinates[3],inCoordinates[4],inCoordinates[5]);
	else
		fprintf(REF,"%f,%f,%f\n",inCoordinates[0],inCoordinates[1],inCoordinates[2]);
#endif
}

void DSFPrint_AddSegmentShapePoint(
				double			inCoordinates[6],
				bool			inCurved,
				void *			inRef)
{
	if (inCoordinates[0] < sWest || inCoordinates[0] > sEast ||
		inCoordinates[1] < sSouth || inCoordinates[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inCoordinates[0], inCoordinates[1]);
		sBad = true;
	}
	++sDSF_ShapePoints;
#if PRINT_IT
	if (inCurved)
		fprintf(REF,"       %f,%f,%f (%f,%f,%f)\n",inCoordinates[0],inCoordinates[1],inCoordinates[2],inCoordinates[3],inCoordinates[4],inCoordinates[5]);
	else
		fprintf(REF,"       %f,%f,%f\n",inCoordinates[0],inCoordinates[1],inCoordinates[2]);
#endif
}

void DSFPrint_EndSegment(
				unsigned int	inEndNodeID,
				double			inCoordinates[6],
				bool			inCurved,
				void *			inRef)
{
	if (inCoordinates[0] < sWest || inCoordinates[0] > sEast ||
		inCoordinates[1] < sSouth || inCoordinates[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inCoordinates[0], inCoordinates[1]);
		sBad = true;
	}
#if PRINT_IT
	fprintf(REF,"   End segment to %d ",inEndNodeID);
	if (inCurved)
		fprintf(REF,"%f,%f,%f (%f,%f,%f)\n",inCoordinates[0],inCoordinates[1],inCoordinates[2],inCoordinates[3],inCoordinates[4],inCoordinates[5]);
	else
		fprintf(REF,"%f,%f,%f\n",inCoordinates[0],inCoordinates[1],inCoordinates[2]);
#endif
}

void DSFPrint_BeginPolygon(
				unsigned int	inPolygonType,
				unsigned short	inParam,
				int				inDepth,
				void *			inRef)
{
#if PRINT_IT
	fprintf(REF,"Polygon type=%d, param=0x%04x\n", inPolygonType, (int) inParam);
#endif
	dsf_print_depth = inDepth;
}
void DSFPrint_BeginPolygonWinding(
				void *			inRef)
{
#if PRINT_IT
	fprintf(REF, "  Begin winding.\n");
#endif
}
void DSFPrint_AddPolygonPoint(
				double *		inCoordinates,
				void *			inRef)
{
	if (inCoordinates[0] < sWest || inCoordinates[0] > sEast ||
		inCoordinates[1] < sSouth || inCoordinates[1] > sNorth)
	{
		printf("ERROR: out of bounds pt %lf, %lf\n", inCoordinates[0], inCoordinates[1]);
		sBad = true;
	}
#if PRINT_IT
	fprintf(REF, "       ");
	for (int n = 0; n < dsf_print_depth; ++n)
		fprintf(REF, "%lf    ", inData[n]);
	fprintf(REF, "\n");
#endif
}

void DSFPrint_EndPolygonWinding(
				void *			inRef)
{
#if PRINT_IT
	fprintf(REF, "  End winding.\n");
#endif
}
void DSFPrint_EndPolygon(
				void *			inRef)
{
#if PRINT_IT
	fprintf(REF, "End polygon.\n");
#endif
	++sDSF_Polys;
}
#undef REF

int	PrintDSFFile(const char * inPath, FILE * output, bool print_it)
{
	sDSF_Patches = 0;
	sDSF_Tris = 0;
	sDSF_Polys = 0;
	sDSF_Objs = 0;
	sDSF_Chains = 0;
	sDSF_ShapePoints = 0;
	sBad = 0;

#if OBJ_HISTO
obj_names.clear();
obj_usages.clear();
obj_total = 0;
#endif

	if (print_it) fprintf(output, "Dumping file %s\n", inPath);
	DSFCallbacks_t	callbacks;
	callbacks.NextPass_f = DSFPrint_NextPass;
	callbacks.AcceptTerrainDef_f = DSFPrint_AcceptTerrainDef;
	callbacks.AcceptObjectDef_f = DSFPrint_AcceptObjectDef;
	callbacks.AcceptPolygonDef_f = DSFPrint_AcceptPolygonDef;
	callbacks.AcceptNetworkDef_f = DSFPrint_AcceptNetworkDef;
	callbacks.AcceptProperty_f = DSFPrint_AcceptProperty;
	callbacks.BeginPatch_f = DSFPrint_BeginPatch;
	callbacks.BeginPrimitive_f = DSFPrint_BeginPrimitive;
	callbacks.AddPatchVertex_f = DSFPrint_AddPatchVertex;
	callbacks.EndPrimitive_f = DSFPrint_EndPrimitive;
	callbacks.EndPatch_f = DSFPrint_EndPatch;
	callbacks.AddObject_f = DSFPrint_AddObject;
	callbacks.BeginSegment_f = DSFPrint_BeginSegment;
	callbacks.AddSegmentShapePoint_f = DSFPrint_AddSegmentShapePoint;
	callbacks.EndSegment_f = DSFPrint_EndSegment;
	callbacks.BeginPolygon_f = DSFPrint_BeginPolygon;
	callbacks.BeginPolygonWinding_f = DSFPrint_BeginPolygonWinding;
	callbacks.AddPolygonPoint_f = DSFPrint_AddPolygonPoint;
	callbacks.EndPolygonWinding_f = DSFPrint_EndPolygonWinding;
	callbacks.EndPolygon_f = DSFPrint_EndPolygon;
#if USE_MEM_FILE
	int err = 0;
	MFMemFile *	mf = MemFile_Open(inPath);
	if (mf)
	{
		err = DSFReadMem(MemFile_GetBegin(mf),MemFile_GetEnd(mf), &callbacks, NULL, output);
		MemFile_Close(mf);
	}
#else
	int err = DSFReadFile(inPath, &callbacks, NULL, output);
#endif
	if (print_it) fprintf(output,"Done - error = %d (%s) ", err, dsfErrorMessages[err]);
	if (print_it) fprintf(output,"Patches=%d, Tris=%d, polys=%d, objs=%d ",
				sDSF_Patches,sDSF_Tris / 3,sDSF_Polys,sDSF_Objs);
	if (print_it) fprintf(output, "Chains=%d, Shape Points=%d\n",sDSF_Chains, sDSF_ShapePoints);
	if (err == 0 && sBad) return 1;

#if OBJ_HISTO
	if (obj_total)
	{
		multimap<int, string>	histo;
		for (int n = 0; n < obj_names.size(); ++n)
		{
			histo.insert(multimap<int, string>::value_type(obj_usages[n], obj_names[n]));
		}

		for (multimap<int, string>::iterator iter = histo.begin(); iter != histo.end(); ++iter)
		{
			printf("%5d %3.2f %s\n", iter->first, (float) iter->first / (float) obj_total, iter->second.c_str());
		}
	}
#endif
	return err;
}
