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
#include "EnvPrint.h"
#include "XGrinderApp.h"
#include <stdio.h>
#include "Persistence.h"

bool	PrintENVData(const char * newPath)
{
	FILE * fi = fopen(newPath, "w");
	if (fi)
	{
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# MESH" CRLF);
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# 201 x 151 vector nodes, from SW to NE, adjacent=east" CRLF);
		fprintf(fi,"# lat,lon,elevation,landuse,custom,rotation,scale,xoffset,yoffset,joincode" CRLF);			

		for (vector<VertexInfo>::iterator v = gVertices.begin();
			v != gVertices.end(); ++v)
		{
			fprintf(fi,"%lf,%lf,%lf,%d,%d,%d,%d,%d,%d,%d" CRLF,
				v->latitude, v->longitude, v->elevation,
				v->landUse, v->custom, 
				v->rotation, v->scale,
				v->xOff, v->yOff, v->bodyID);
		}
		
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# OBJECTS" CRLF);
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# Variable number of objects, ends with 'END'" CRLF);
		fprintf(fi,"# kind,lat,lon,elevation/heading,cust obj name" CRLF);
		
		for (vector<ObjectInfo>::iterator o = gObjects.begin();
			o != gObjects.end(); ++o)
		{
			fprintf(fi,"%ld,%lf,%lf,%lf,%s" CRLF,
				o->kind,o->latitude, o->longitude,o->elevation,
				o->name.c_str());
		}
		fprintf(fi,"END" CRLF);
		
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# VECTORS" CRLF);
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# Variable number of vector sets, each ends with END" CRLF);
		fprintf(fi,"# lat, lon, is last" CRLF);
						
		vector<PathInfo>::iterator p;
		
		fprintf(fi,"# Roads" CRLF);
		for (p = gRoads.begin(); p != gRoads.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);
		fprintf(fi,"# Trails" CRLF);
		for (p = gTrails.begin(); p != gTrails.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);
		fprintf(fi,"# Railways" CRLF);
		for (p = gTrains.begin(); p != gTrains.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);
		fprintf(fi,"# Lines" CRLF);
		for (p = gLines.begin(); p != gLines.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);
		fprintf(fi,"# Taxiways" CRLF);
		for (p = gTaxiways.begin(); p != gTaxiways.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);
		fprintf(fi,"# Rivers" CRLF);
		for (p = gRiverVectors.begin(); p != gRiverVectors.end(); ++p)
		{
			fprintf(fi,"%f,%f,%d" CRLF,p->latitude,p->longitude,p->term);
		}
		fprintf(fi,"END" CRLF);

		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# TEXTURES" CRLF);
		fprintf(fi,"###################################################" CRLF);
		fprintf(fi,"# Variable number of texture file names, ends with END" CRLF);
	
		for (vector<string>::iterator t = gTextures.begin();
			t != gTextures.end(); ++t)
		{
			fprintf(fi,"%s" CRLF,t->c_str());
		}
		fprintf(fi,"END" CRLF);
		
		fclose(fi);
		return true;
	}
	return false;
}
