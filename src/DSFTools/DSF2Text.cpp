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
#include <stdio.h>
#include "DSFLib.h"

static int sDSF2TEXT_CoordDepth;

void DSF2Text_AcceptTerrainDef(const char * inPartialPath, void * inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "TERRAIN_DEF %s\n", inPartialPath);
}

void DSF2Text_AcceptObjectDef(const char * inPartialPath, void * inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "OBJECT_DEF %s\n", inPartialPath);
}

void DSF2Text_AcceptPolygonDef(const char * inPartialPath, void * inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "POLYGON_DEF %s\n", inPartialPath);
}

void DSF2Text_AcceptNetworkDef(const char * inPartialPath, void * inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "NETWORK_DEF %s\n", inPartialPath);
}

void DSF2Text_AcceptProperty(const char * inProp, const char * inValue, void * inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "PROPERTY %s %s\n", inProp, inValue);
}

void DSF2Text_BeginPatch(
	unsigned int	inTerrainType,
	double 			inNearLOD,
	double 			inFarLOD,
	unsigned char	inFlags,
	int				inCoordDepth,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	sDSF2TEXT_CoordDepth = inCoordDepth;
	fprintf(fi, "BEGIN_PATCH %d %lf %lf %d %d\n", inTerrainType, inNearLOD, inFarLOD, inFlags, inCoordDepth);
}

void DSF2Text_BeginPrimitive(
	int				inType,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "BEGIN_PRIMITIVE %d\n", inType);
}

void DSF2Text_AddPatchVertex(
	double			inCoordinates[],
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "PATCH_VERTEX");
	for (int n = 0; n < sDSF2TEXT_CoordDepth; ++n)
		fprintf(fi, " %.8lf", inCoordinates[n]);
	fprintf(fi, "\n");
}

void DSF2Text_EndPrimitive(
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "END_PRIMITIVE\n");
}

void DSF2Text_EndPatch(
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "END_PATCH\n");
}

void DSF2Text_AddObject(
	unsigned int	inObjectType,
	double			inCoordinates[2],
	double			inRotation,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "OBJECT %d %.8lf %.8lf %lf\n", inObjectType, inCoordinates[0], inCoordinates[1], inRotation);
}

void DSF2Text_BeginSegment(
	unsigned int	inNetworkType,
	unsigned int	inNetworkSubtype,
	unsigned int	inStartNodeID,
	double			inCoordinates[6],
	bool			inCurved,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	if (!inCurved)
		fprintf(fi, "BEGIN_SEGMENT %d %d %d %.8lf %.8lf %.8lf\n", inNetworkType, inNetworkSubtype, inStartNodeID,
															inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		fprintf(fi, "BEGIN_SEGMENT_CURVED %d %d %d %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf\n", inNetworkType, inNetworkSubtype, inStartNodeID,
															inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[3],inCoordinates[4],inCoordinates[5]);
}

void DSF2Text_AddSegmentShapePoint(
	double			inCoordinates[6],
	bool			inCurved,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	if (!inCurved)
		fprintf(fi, "SHAPE_POINT %.8lf %.8lf %.8lf\n", inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		fprintf(fi, "SHAPE_POINT_CURVED %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf\n",inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[3],inCoordinates[4],inCoordinates[5]);
}

void DSF2Text_EndSegment(
	unsigned int	inEndNodeID,
	double			inCoordinates[6],
	bool			inCurved,
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	if (!inCurved)
		fprintf(fi, "END_SEGMENT %d %.8lf %.8lf %.8lf\n", inEndNodeID, inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		fprintf(fi, "END_SEGMENT_CURVED %d %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf\n",inEndNodeID, inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[3],inCoordinates[4],inCoordinates[5]);
}

bool DSF2Text_NextPass(int pass, void * ref)
{
	return true;
}

void DSF2Text_BeginPolygon(
	unsigned int	inPolygonType,
	unsigned short	inParam,
	int				inDepth,
	void *			inRef)
{
	sDSF2TEXT_CoordDepth = inDepth;
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "BEGIN_POLYGON %d %d %d\n", inPolygonType, inParam, inDepth);
}

void DSF2Text_BeginPolygonWinding(
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "BEGIN_WINDING\n");
}
void DSF2Text_AddPolygonPoint(
	double			inCoordinates[2],
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "POLYGON_POINT");
	for (int n = 0; n < sDSF2TEXT_CoordDepth; ++n)
		fprintf(fi, " %.8lf", inCoordinates[n]);
	fprintf(fi, "\n");
}

void DSF2Text_EndPolygonWinding(
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "END_WINDING\n");
}

void DSF2Text_EndPolygon(
	void *			inRef)
{
	FILE * fi = (FILE *) inRef;
	fprintf(fi, "END_POLYGON\n");
}

bool DSF2Text(const char * inDSF, const char * inFileName)
{
	FILE * fi = strcmp(inFileName, "-") ? fopen(inFileName, "w") : stdout;
	if (fi == NULL) return false;

	#if APL
	fprintf(fi, "A\n800\nDSF2TEXT\n\n#%s\n\n", inDSF);
	#elif IBM
	fprintf(fi, "I\n800\nDSF2TEXT\n\n#%s\n\n", inDSF);
	#endif

	DSFCallbacks_t	cbs;
	cbs.AcceptTerrainDef_f			=DSF2Text_AcceptTerrainDef			;
	cbs.AcceptObjectDef_f			=DSF2Text_AcceptObjectDef			;
	cbs.AcceptPolygonDef_f			=DSF2Text_AcceptPolygonDef			;
	cbs.AcceptNetworkDef_f			=DSF2Text_AcceptNetworkDef			;
	cbs.AcceptProperty_f			=DSF2Text_AcceptProperty			;
	cbs.BeginPatch_f				=DSF2Text_BeginPatch				;
	cbs.BeginPrimitive_f			=DSF2Text_BeginPrimitive			;
	cbs.AddPatchVertex_f			=DSF2Text_AddPatchVertex			;
	cbs.EndPrimitive_f				=DSF2Text_EndPrimitive				;
	cbs.EndPatch_f					=DSF2Text_EndPatch					;
	cbs.AddObject_f					=DSF2Text_AddObject					;
	cbs.BeginSegment_f				=DSF2Text_BeginSegment				;
	cbs.AddSegmentShapePoint_f		=DSF2Text_AddSegmentShapePoint		;
	cbs.EndSegment_f				=DSF2Text_EndSegment				;
	cbs.BeginPolygon_f				=DSF2Text_BeginPolygon				;
	cbs.BeginPolygonWinding_f		=DSF2Text_BeginPolygonWinding		;
	cbs.AddPolygonPoint_f			=DSF2Text_AddPolygonPoint			;
	cbs.EndPolygonWinding_f			=DSF2Text_EndPolygonWinding			;
	cbs.EndPolygon_f				=DSF2Text_EndPolygon				;
	cbs.NextPass_f					=DSF2Text_NextPass					;


	int result = DSFReadFile(inDSF, &cbs, NULL, fi);

	fprintf(fi, "# Result code: %d\n", result);

	if (strcmp(inFileName, "-"))
		fclose(fi);
	return true;
}


bool Text2DSF(const char * inFileName, const char * inDSF)
{
	FILE * fi = (strcmp(inFileName, "-") ? fopen(inFileName, "r") : stdin);
	if (!fi) return NULL;

	int divisions = 8;
	float west = 999.0, south = 999.0, north = 999.0, east = 999.0;

	char	buf[512];
	char	prop_id[512];
	char	prop_value[512];

	DSFCallbacks_t	cbs;
	void * writer;

	int props_got = 0;

	vector<pair<string, string> >		properties;

	printf("Scanning for dimension properties...\n");

	while (fgets(buf, sizeof(buf), fi))
	{
		if (sscanf(buf, "PROPERTY %s %[^\r\n]", prop_id, prop_value) == 2)
			properties.push_back(pair<string, string>(prop_id, prop_value));

		if (sscanf(buf, "PROPERTY sim/west %f", &west) == 1) ++props_got;
		if (sscanf(buf, "PROPERTY sim/east %f", &east) == 1) ++props_got;
		if (sscanf(buf, "PROPERTY sim/north %f", &north) == 1) ++props_got;
		if (sscanf(buf, "PROPERTY sim/south %f", &south) == 1) ++props_got;
		sscanf(buf, "DIVISIONS %d", &divisions);
		if (props_got >= 4 && west != 999.0 && east != 999.0 && north != 999.0 & south != 999.0) break;
	}

	if (west >= 180.0 || west < -180.0 ||
		east > 180.0 || east <= -180.0 ||
		south >= 90.0 || south < -90.0 ||
		north > 90.0 || north <= -90.0)
	{
		fprintf(stdout, "ERROR: the DSF boundaries are out of range.  This can indicate a missing or corrupt sim/dimension properties.\n");
		return false;
	}

	printf("Got dimension properties, establishing file writer...\n");

	writer = DSFCreateWriter(west, south, east, north, divisions);
	DSFGetWriterCallbacks(&cbs);

	for (int p = 0; p < properties.size(); ++p)
		cbs.AcceptProperty_f(properties[p].first.c_str(), properties[p].second.c_str(), writer);

	int		ptype, subtype, flags, depth = 99, nodeid, param;
	double	lod_near, lod_far;

	double	coords[10];

	while (fgets(buf, sizeof(buf), fi))
	{

			 if (sscanf(buf, "PATCH_VERTEX %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5], &coords[6], &coords[7], &coords[8], &coords[9]) == depth)		cbs.AddPatchVertex_f(coords, writer);
		else if (sscanf(buf, "OBJECT %d %lf %lf %lf", &ptype, &coords[0],&coords[1],&coords[2]) == 4)		cbs.AddObject_f(ptype, coords, coords[2], writer);

		else if (sscanf(buf,"BEGIN_SEGMENT %d %d %d %lf %lf %lf", &ptype, &subtype, &nodeid,  &coords[0],&coords[1],&coords[2]) == 6)							cbs.BeginSegment_f(ptype, subtype, nodeid, coords, false, writer);
		else if (sscanf(buf,"SHAPE_POINT %lf %lf %lf", &coords[0], &coords[1], &coords[2])== 3) 			cbs.AddSegmentShapePoint_f(coords, false, writer);
		else if (sscanf(buf,"END_SEGMENT %d %lf %lf %lf", &nodeid, &coords[0], &coords[1], &coords[2])== 4) cbs.EndSegment_f(nodeid, coords, false, writer);

		else if (sscanf(buf, "BEGIN_PRIMITIVE %d", &ptype) == 1)												cbs.BeginPrimitive_f(ptype, writer);
		else if (!strncmp(buf, "END_PRIMITIVE", strlen("END_PRIMITIVE")))										cbs.EndPrimitive_f(writer);
		else if (sscanf(buf,"BEGIN_PATCH %d %lf %lf %d %d", &ptype, &lod_near, &lod_far, &flags, &depth) == 5) 	cbs.BeginPatch_f(ptype, lod_near, lod_far, flags, depth, writer);
		else if (!strncmp(buf, "END_PATCH", strlen("END_PATCH")))												{ cbs.EndPatch_f(writer); depth = 99; }

		else if (sscanf(buf, "POLYGON_POINT %lf %lf %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5], &coords[6], &coords[7])==depth)			cbs.AddPolygonPoint_f(coords, writer);
		else if (!strncmp(buf, "BEGIN_WINDING", strlen("BEGIN_WINDING")))					cbs.BeginPolygonWinding_f(writer);
		else if (!strncmp(buf, "END_WINDING", strlen("END_WINDING")))						cbs.EndPolygonWinding_f(writer);
		else if (sscanf(buf,"BEGIN_POLYGON %d %d %d", &ptype, &param, &depth)==3)			cbs.BeginPolygon_f(ptype, param, depth, writer);
		else if (sscanf(buf,"BEGIN_POLYGON %d %d %d", &ptype, &param, &depth)==2)			cbs.BeginPolygon_f(ptype, param, 2, 	writer);
		else if (!strncmp(buf, "END_POLYGON", strlen("END_POLYGON")))						cbs.EndPolygon_f(writer);


		else if (sscanf(buf, "TERRAIN_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptTerrainDef_f(prop_id, writer);
		else if (sscanf(buf, "OBJECT_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptObjectDef_f(prop_id, writer);
		else if (sscanf(buf, "POLYGON_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptPolygonDef_f(prop_id, writer);
		else if (sscanf(buf, "NETWORK_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptNetworkDef_f(prop_id, writer);

		else if (sscanf(buf,"BEGIN_SEGMENT_CURVED %d %d %d %lf %lf %lf %lf %lf %lf", &ptype, &subtype, &nodeid, &coords[0],&coords[1],&coords[2],&coords[3],&coords[4],&coords[5]) == 9) cbs.BeginSegment_f(ptype, subtype, nodeid, coords, true, writer);
		else if (sscanf(buf,"SHAPE_POINT_CURVED %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5])== 6) cbs.AddSegmentShapePoint_f(coords, true, writer);
		else if (sscanf(buf,"SHAPE_POINT_CURVED %d %lf %lf %lf %lf %lf %lf ", &nodeid, &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5])== 7) cbs.EndSegment_f(nodeid, coords, true, writer);
	}

	if (strcmp(inFileName, "-"))
		fclose(fi);

	printf("Got entire file, processing and creating DSF.\n");

	DSFWriteToFile(inDSF, writer);
	DSFDestroyWriter(writer);
	return true;
}
