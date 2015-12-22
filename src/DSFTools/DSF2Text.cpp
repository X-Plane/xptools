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
#include "DSF2Text.h"
#include "DSFLib.h"
#include <list>

using std::list;

static int sDSF2TEXT_CoordDepth;

static int offset_ter = 0;
static int offset_obj = 0;
static int offset_pol = 0;
static int offset_net = 0;

static int count_ter = 0;
static int count_obj = 0;
static int count_pol = 0;
static int count_net = 0;

string			base_name;
list<string>	dem_names;



int DSF2Text_AcceptTerrainDef(const char * inPartialPath, void * inRef)
{
	++count_ter;
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "TERRAIN_DEF %s\n", inPartialPath);
	return 1;
}

int DSF2Text_AcceptObjectDef(const char * inPartialPath, void * inRef)
{
	++count_obj;
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "OBJECT_DEF %s\n", inPartialPath);
	return 1;
}

int DSF2Text_AcceptPolygonDef(const char * inPartialPath, void * inRef)
{
	++count_pol;
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "POLYGON_DEF %s\n", inPartialPath);
	return 1;
}

int DSF2Text_AcceptNetworkDef(const char * inPartialPath, void * inRef)
{
	++count_net;
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "NETWORK_DEF %s\n", inPartialPath);
	return 1;
}

int DSF2Text_AcceptRasterDef(const char * inPartialPath, void * inRef)
{
	++count_net;
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "RASTER_DEF %s\n", inPartialPath);
	dem_names.push_back(inPartialPath);
	return 1;
}

void DSF2Text_AcceptProperty(const char * inProp, const char * inValue, void * inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "PROPERTY %s %s\n", inProp, inValue);
}

void DSF2Text_BeginPatch(
	unsigned int	inTerrainType,
	double 			inNearLOD,
	double 			inFarLOD,
	unsigned char	inFlags,
	int				inCoordDepth,
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	sDSF2TEXT_CoordDepth = inCoordDepth;
	p->print_func(p->ref, "BEGIN_PATCH %d %lf %lf %d %d\n", inTerrainType + offset_ter, inNearLOD, inFarLOD, inFlags, inCoordDepth);
}

void DSF2Text_BeginPrimitive(
	int				inType,
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "BEGIN_PRIMITIVE %d\n", inType);
}

void DSF2Text_AddPatchVertex(
	double			inCoordinates[],
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "PATCH_VERTEX");
	for (int n = 0; n < sDSF2TEXT_CoordDepth; ++n)
		p->print_func(p->ref, " %.9lf", inCoordinates[n]);
	p->print_func(p->ref, "\n");
}

void DSF2Text_EndPrimitive(
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "END_PRIMITIVE\n");
}

void DSF2Text_EndPatch(
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "END_PATCH\n");
}

void DSF2Text_AddObject(
	unsigned int	inObjectType,
	double			inCoordinates[],
	int				inCoordinateDepth,
	void *			inRef)
{
	if(inObjectType >= count_obj)
		printf("WARNING: out of bounds obj.\n");
	print_funcs_s * p = (print_funcs_s *) inRef;
	if(inCoordinateDepth == 4)
	p->print_func(p->ref, "OBJECT_MSL %d %.9lf %.9lf %.9lf %lf\n", inObjectType + offset_obj, inCoordinates[0], inCoordinates[1], inCoordinates[3], inCoordinates[2]);
	else
	p->print_func(p->ref, "OBJECT %d %.9lf %.9lf %lf\n", inObjectType + offset_obj, inCoordinates[0], inCoordinates[1], inCoordinates[2]);
}

void DSF2Text_BeginSegment(
	unsigned int	inNetworkType,
	unsigned int	inNetworkSubtype,
	double			inCoordinates[],
	bool			inCurved,
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	if (!inCurved)
		p->print_func(p->ref, "BEGIN_SEGMENT %d %d %d %.9lf %.9lf %.9lf\n", inNetworkType + offset_net, inNetworkSubtype, (int) inCoordinates[3],
															inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		p->print_func(p->ref, "BEGIN_SEGMENT_CURVED %d %d %d %.9lf %.9lf %.9lf %.9lf %.9lf %.9lf\n", inNetworkType, inNetworkSubtype, (int) inCoordinates[3],
															inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[4],inCoordinates[5],inCoordinates[6]);
}

void DSF2Text_AddSegmentShapePoint(
	double			inCoordinates[],
	bool			inCurved,
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	if (!inCurved)
		p->print_func(p->ref, "SHAPE_POINT %.9lf %.9lf %.9lf\n", inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		p->print_func(p->ref, "SHAPE_POINT_CURVED %.9lf %.9lf %.9lf %.9lf %.9lf %.9lf\n",inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[3],inCoordinates[4],inCoordinates[5]);
}

void DSF2Text_EndSegment(
	double			inCoordinates[],
	bool			inCurved,
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	if (!inCurved)
		p->print_func(p->ref, "END_SEGMENT %d %.9lf %.9lf %.9lf\n", (int) inCoordinates[3], inCoordinates[0],inCoordinates[1],inCoordinates[2]);
	else
		p->print_func(p->ref, "END_SEGMENT_CURVED %d %.9lf %.9lf %.9lf %.9lf %.9lf %.9lf\n",(int) inCoordinates[3], inCoordinates[0],inCoordinates[1],inCoordinates[2],
															inCoordinates[4],inCoordinates[5],inCoordinates[6]);
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
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "BEGIN_POLYGON %d %d %d\n", inPolygonType + offset_pol, inParam, inDepth);
}

void DSF2Text_BeginPolygonWinding(
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "BEGIN_WINDING\n");
}
void DSF2Text_AddPolygonPoint(
	double			inCoordinates[2],
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "POLYGON_POINT");
	for (int n = 0; n < sDSF2TEXT_CoordDepth; ++n)
		p->print_func(p->ref, " %.9lf", inCoordinates[n]);
	p->print_func(p->ref, "\n");
}

void DSF2Text_EndPolygonWinding(
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "END_WINDING\n");
}

void DSF2Text_AddRaterData(
					DSFRasterHeader_t *	header,
					void *				data,
					void *				inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref,"RASTER_DATA version=%d bpp=%d flags=%d width=%d height=%d scale=%f offset=%f ",
		header->version, header->bytes_per_pixel, header->flags, header->width, header->height, header->scale,header->offset);

	if(!base_name.empty() && !dem_names.empty())
	{
		string demp(base_name);
		demp += ".";
		demp += dem_names.front();
		demp += ".raw";
		dem_names.pop_front();
		FILE * fb = fopen(demp.c_str(),"wb");
		if(fb)
		{
			p->print_func(p->ref,"%s\n",demp.c_str());
			//p->print_func(p->ref,"# Wrote %d bytes to %s.\n", header->bytes_per_pixel * header->width * header->height, p.c_str());
			fwrite(data,header->bytes_per_pixel * header->width * header->height, 1, fb);
			fclose(fb);
		}
		else
			p->print_func(p->ref,"<write error>\n");
	}
		
}

void DSF2Text_EndPolygon(
	void *			inRef)
{
	print_funcs_s * p = (print_funcs_s *) inRef;
	p->print_func(p->ref, "END_POLYGON\n");
}

void DSF2Text_CreateWriterCallbacks(DSFCallbacks_t * cbs)
{
	cbs->AcceptTerrainDef_f			=DSF2Text_AcceptTerrainDef			;
	cbs->AcceptObjectDef_f			=DSF2Text_AcceptObjectDef			;
	cbs->AcceptPolygonDef_f			=DSF2Text_AcceptPolygonDef			;
	cbs->AcceptNetworkDef_f			=DSF2Text_AcceptNetworkDef			;
	cbs->AcceptRasterDef_f			=DSF2Text_AcceptRasterDef			;
	cbs->AcceptProperty_f			=DSF2Text_AcceptProperty			;
	cbs->BeginPatch_f				=DSF2Text_BeginPatch				;
	cbs->BeginPrimitive_f			=DSF2Text_BeginPrimitive			;
	cbs->AddPatchVertex_f			=DSF2Text_AddPatchVertex			;
	cbs->EndPrimitive_f				=DSF2Text_EndPrimitive				;
	cbs->EndPatch_f					=DSF2Text_EndPatch					;
	cbs->AddObject_f					=DSF2Text_AddObject					;
	cbs->BeginSegment_f				=DSF2Text_BeginSegment				;
	cbs->AddSegmentShapePoint_f		=DSF2Text_AddSegmentShapePoint		;
	cbs->EndSegment_f				=DSF2Text_EndSegment				;
	cbs->BeginPolygon_f				=DSF2Text_BeginPolygon				;
	cbs->BeginPolygonWinding_f		=DSF2Text_BeginPolygonWinding		;
	cbs->AddPolygonPoint_f			=DSF2Text_AddPolygonPoint			;
	cbs->EndPolygonWinding_f			=DSF2Text_EndPolygonWinding			;
	cbs->EndPolygon_f				=DSF2Text_EndPolygon				;
	cbs->AddRasterData_f				=DSF2Text_AddRaterData				;
	cbs->NextPass_f					=DSF2Text_NextPass					;
}




bool DSF2Text(char ** inDSF, int n, const char * inFileName)
{
	FILE * fi = strcmp(inFileName, "-") ? fopen(inFileName, "w") : stdout;
	if (fi == NULL) return false;

	base_name = strcmp(inFileName, "-") ? inFileName : "";
	dem_names.clear();
	
	#if APL
	fprintf(fi, "A\n800\nDSF2TEXT\n\n");
	#elif IBM
	fprintf(fi, "I\n800\nDSF2TEXT\n\n");
	#endif

	DSFCallbacks_t	cbs;
	DSF2Text_CreateWriterCallbacks(&cbs);
	
	print_funcs_s pf;
	pf.print_func = (int (*)(void *,const char *,...)) fprintf;
	pf.ref = fi;

	while(n--)
	{
		fprintf(fi,"# file: %s\n\n",*inDSF);
		int result = DSFReadFile(*inDSF, malloc, free, &cbs, NULL, &pf);

		fprintf(fi, "# Result code: %d\n", result);
		if(result == dsf_ErrNoAtoms || result == dsf_ErrBadCookie || result == dsf_ErrBadVersion)
			fprintf(stderr,"The DFS was not readable.  Perhaps you need to unzip it with 7-zip?\n");

		printf("File %s had %d ter, %d obj, %d pol, %d net.\n", *inDSF,
			count_ter, count_obj,count_pol,count_net);

		++inDSF;
		
		offset_ter += count_ter;
		offset_obj += count_obj;
		offset_pol += count_pol;
		offset_net += count_net;
		
		count_ter = count_obj = count_pol = count_net = 0;
	}

	if (strcmp(inFileName, "-"))
		fclose(fi);
	return true;
}

static char * strip_and_clean(char * raw)
{
	char * r = raw;

	while(*r == ' ' || *r == '\t')					// Advance past any white space, so we can start a line with tab indent.
		++r;

	char * p = r;

	while(*p)										// Convert any CRLF or # to null term, so we don't pick them up.
	{
		if(*p == '#' || *p == '\r' || *p == '\n')
			*p = 0;
		++p;
	}
	return r;
}


static bool Text2DSFWithWriterAny(const char * inFileName, const char * inDSF, DSFCallbacks_t * in_cbs, void * in_writer)
{
	bool is_pipe = strcmp(inFileName, "-") == 0;
	FILE * fi = (!is_pipe) ? fopen(inFileName, "r") : stdin;
	if (!fi) return NULL;

	int divisions = 8;
	float west = 999.0, south = 999.0, north = 999.0, east = 999.0;

	DSFRasterHeader_t	rheader;

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
		char * ptr = strip_and_clean(buf);
		if (sscanf(ptr, "PROPERTY %s %[^\r\n]", prop_id, prop_value) == 2)
			properties.push_back(pair<string, string>(prop_id, prop_value));

		if (sscanf(ptr, "PROPERTY sim/west %f", &west) == 1) ++props_got;
		if (sscanf(ptr, "PROPERTY sim/east %f", &east) == 1) ++props_got;
		if (sscanf(ptr, "PROPERTY sim/north %f", &north) == 1) ++props_got;
		if (sscanf(ptr, "PROPERTY sim/south %f", &south) == 1) ++props_got;
		sscanf(ptr, "DIVISIONS %d", &divisions);

		if(is_pipe)
		if (strncmp(ptr,"DIVISIONS",9) != 0 &&
		   strncmp(ptr,"PROPERTY",8) != 0 &&
		   strncmp(ptr,"I",1) != 0 &&
		   strncmp(ptr,"A",1) != 0 &&
		   strncmp(ptr,"800",3) != 0 &&
		   strncmp(ptr,"DSF2TEXT",8) != 0 &&
		   ptr[0] != 0)
			break;
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

	if(!is_pipe)
		fseek(fi,0,SEEK_SET);

	if(in_cbs)
	{
		memcpy(&cbs,in_cbs,sizeof(cbs));
		writer = in_writer;
	}
	else
	{
		writer = DSFCreateWriter(west, south, east, north, -32768.0, 32767.0, divisions);
		DSFGetWriterCallbacks(&cbs);
	}

	for (int p = 0; p < properties.size(); ++p)
		cbs.AcceptProperty_f(properties[p].first.c_str(), properties[p].second.c_str(), writer);

	int		ptype, subtype, flags, depth = 99, param;
	double	lod_near, lod_far;

	double	coords[10];

	if(!is_pipe)
	{
		fseek(fi,0,SEEK_SET);
		while(fgets(buf, sizeof(buf), fi))
		{
			char * ptr = strip_and_clean(buf);
				 if (!is_pipe && sscanf(ptr, "TERRAIN_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptTerrainDef_f(prop_id, writer);
			else if (!is_pipe && sscanf(ptr, "OBJECT_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptObjectDef_f(prop_id, writer);
			else if (!is_pipe && sscanf(ptr, "POLYGON_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptPolygonDef_f(prop_id, writer);
			else if (!is_pipe && sscanf(ptr, "NETWORK_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptNetworkDef_f(prop_id, writer);
			else if (!is_pipe && sscanf(ptr, "RASTER_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptRasterDef_f(prop_id, writer);

		}

		fseek(fi,0,SEEK_SET);
		fgets(buf, sizeof(buf), fi);


	}
	

	do 
	{
		char * ptr = strip_and_clean(buf);
			 if (sscanf(ptr, "PATCH_VERTEX %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5], &coords[6], &coords[7], &coords[8], &coords[9]) == depth)		cbs.AddPatchVertex_f(coords, writer);
		else if (sscanf(ptr, "OBJECT %d %lf %lf %lf", &ptype, &coords[0],&coords[1],&coords[2]) == 4)		cbs.AddObject_f(ptype, coords, 3, writer);
		else if (sscanf(ptr, "OBJECT_MSL %d %lf %lf %lf %lf", &ptype, &coords[0],&coords[1],&coords[3],&coords[2]) == 5)		cbs.AddObject_f(ptype, coords, 4, writer);

		else if (sscanf(ptr,"BEGIN_SEGMENT %d %d %lf %lf %lf %lf", &ptype, &subtype, &coords[3], &coords[0],&coords[1],&coords[2]) == 6)							cbs.BeginSegment_f(ptype, subtype, coords, false, writer);
		else if (sscanf(ptr,"SHAPE_POINT %lf %lf %lf", &coords[0], &coords[1], &coords[2])== 3) 			cbs.AddSegmentShapePoint_f(coords, false, writer);
		else if (sscanf(ptr,"END_SEGMENT %lf %lf %lf %lf", &coords[3], &coords[0], &coords[1], &coords[2])== 4) cbs.EndSegment_f(coords, false, writer);

		else if (sscanf(ptr, "BEGIN_PRIMITIVE %d", &ptype) == 1)												cbs.BeginPrimitive_f(ptype, writer);
		else if (!strncmp(ptr, "END_PRIMITIVE", strlen("END_PRIMITIVE")))										cbs.EndPrimitive_f(writer);
		else if (sscanf(ptr,"BEGIN_PATCH %d %lf %lf %d %d", &ptype, &lod_near, &lod_far, &flags, &depth) == 5) 	cbs.BeginPatch_f(ptype, lod_near, lod_far, flags, depth, writer);
		else if (!strncmp(ptr, "END_PATCH", strlen("END_PATCH")))												{ cbs.EndPatch_f(writer); depth = 99; }

		else if (sscanf(ptr, "POLYGON_POINT %lf %lf %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5], &coords[6], &coords[7])==depth)			cbs.AddPolygonPoint_f(coords, writer);
		else if (!strncmp(ptr, "BEGIN_WINDING", strlen("BEGIN_WINDING")))					cbs.BeginPolygonWinding_f(writer);
		else if (!strncmp(ptr, "END_WINDING", strlen("END_WINDING")))						cbs.EndPolygonWinding_f(writer);
		else if (sscanf(ptr,"BEGIN_POLYGON %d %d %d", &ptype, &param, &depth)==3)			cbs.BeginPolygon_f(ptype, param, depth, writer);
		else if (sscanf(ptr,"BEGIN_POLYGON %d %d %d", &ptype, &param, &depth)==2)			cbs.BeginPolygon_f(ptype, param, 2, 	writer);
		else if (!strncmp(ptr, "END_POLYGON", strlen("END_POLYGON")))						cbs.EndPolygon_f(writer);

		else if (is_pipe && sscanf(ptr, "TERRAIN_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptTerrainDef_f(prop_id, writer);
		else if (is_pipe && sscanf(ptr, "OBJECT_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptObjectDef_f(prop_id, writer);
		else if (is_pipe && sscanf(ptr, "POLYGON_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptPolygonDef_f(prop_id, writer);
		else if (is_pipe && sscanf(ptr, "NETWORK_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptNetworkDef_f(prop_id, writer);
		else if (is_pipe && sscanf(ptr, "RASTER_DEF %[^\r\n]", prop_id) == 1)							cbs.AcceptRasterDef_f(prop_id, writer);

		else if (sscanf(ptr,"BEGIN_SEGMENT_CURVED %d %d %lf %lf %lf %lf %lf %lf %lf", &ptype, &subtype, &coords[3], &coords[0],&coords[1],&coords[2],&coords[4],&coords[5],&coords[6]) == 9) cbs.BeginSegment_f(ptype, subtype, coords, true, writer);
		else if (sscanf(ptr,"SHAPE_POINT_CURVED %lf %lf %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3], &coords[4], &coords[5])== 6) cbs.AddSegmentShapePoint_f(coords, true, writer);
		else if (sscanf(ptr,"SHAPE_POINT_CURVED %lf %lf %lf %lf %lf %lf %lf ", &coords[3], &coords[0], &coords[1], &coords[2], &coords[4], &coords[5], &coords[6])== 7) cbs.EndSegment_f(coords, true, writer);

		else if (sscanf(ptr,"RASTER_DATA version=%hhu bpp=%hhu flags=%hu width=%u height=%u scale=%f offset=%f %[^\r\n]",
							&rheader.version,&rheader.bytes_per_pixel,&rheader.flags,&rheader.width,&rheader.height,&rheader.scale,&rheader.offset,prop_id) == 8)
		{
			int ds = rheader.bytes_per_pixel * rheader.width * rheader.height;
			char * data = (char *) malloc(ds);
			FILE * sf = fopen(prop_id,"rb");
			if(sf)
			{
				if(fread(data,1,ds,sf) == ds)
				{
					cbs.AddRasterData_f(&rheader,data,writer);
				} 
				else
				{
					fprintf(stdout, "ERROR: could not write %d bytes to file %s\n", ds, prop_id);
					fclose(sf);
					return false;
				}
				fclose(sf);
			} else {
				fprintf(stdout, "ERROR: could not open file %s\n", prop_id);
				return false;
			}

		}
	}
	while(fgets(buf, sizeof(buf), fi));


	if (!is_pipe)
		fclose(fi);

	printf("Got entire file, processing and creating DSF.\n");

	if(!in_cbs)
	{
		DSFWriteToFile(inDSF, writer);
		DSFDestroyWriter(writer);
	}
	return true;
}

bool Text2DSFWithWriter(const char * inFileName, DSFCallbacks_t * cbs, void * writer)
{
	return Text2DSFWithWriterAny(inFileName, NULL, cbs, writer);

}
bool Text2DSF(const char * inFileName, const char * inDSF)
{
	return Text2DSFWithWriterAny(inFileName, inDSF, NULL, NULL);

}
