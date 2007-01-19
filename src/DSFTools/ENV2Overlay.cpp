#include "DSFLib.h"
#include "EnvParser.h"
#include "EnvDefs.h"
#include "Persistence.h"
#include <math.h>


bool ENV2Overlay(const char * inFileName, const char * inDSF)
{
	ClearEnvData();
	
	if (ReadEnvFile(inFileName) != 0) 
	{
		fprintf(stdout, "ERROR: ENV file read failed for %s\n", inFileName);
		return false;
	}
	
	int divisions = 8;

	DSFCallbacks_t	cbs;
	void * writer;

	int props_got = 0;

	double	lat_min = 90.0, lat_max = -90.0, lon_min = 180.0, lon_max = -180.0;
	
	for (int v = 0; v < gVertices.size(); ++v)
	{
		lat_min = min(lat_min, gVertices[v].latitude);
		lat_max = max(lat_max, gVertices[v].latitude);
		lon_min = min(lon_min, gVertices[v].longitude);
		lon_max = max(lon_max, gVertices[v].longitude);
	}

	lon_min = floor(lon_min);
	lat_min = floor(lat_min);
	lon_max = ceil (lon_max);
	lat_max = ceil (lat_max);

	writer = DSFCreateWriter(lon_min, lat_min, lon_max, lat_max, divisions);
	DSFGetWriterCallbacks(&cbs);
	
	
	char north[32], west[32],south[32],east[32];
	
	sprintf(north,"%d", (int) lat_max);
	sprintf(south,"%d", (int) lat_min);
	sprintf( east,"%d", (int) lon_max);
	sprintf( west,"%d", (int) lon_min);
	
	cbs.AcceptProperty_f("sim/planet","earth", writer);
	cbs.AcceptProperty_f("sim/overlay","1", writer);
	cbs.AcceptProperty_f("sim/require_object","1/0", writer);
	cbs.AcceptProperty_f("sim/west" ,west , writer);
	cbs.AcceptProperty_f("sim/east" ,east , writer);
	cbs.AcceptProperty_f("sim/north",north, writer);
	cbs.AcceptProperty_f("sim/south",south, writer);

	int o;
	
	map<string, int>	defs;
	
	for (o = 0; o < gObjects.size(); ++o)
	if (gObjects[o].kind == kObstacleTypeCustom)
		defs.insert(map<string,int>::value_type(gObjects[o].name,0));

	o = 0;
	for (map<string, int>::iterator i = defs.begin(); i != defs.end(); ++i,++o)
	{
		i->second = o;
		string nn = i->first + ".obj";
		cbs.AcceptObjectDef_f(nn.c_str(), writer);
	}

	for (o = 0; o < gObjects.size(); ++o)
	if (gObjects[o].kind == kObstacleTypeCustom)
	{
		double coords[3];
		coords[0] = gObjects[o].longitude;
		coords[1] = gObjects[o].latitude ;
		coords[2] = gObjects[o].elevation;
		cbs.AddObject_f(defs[gObjects[o].name], coords, coords[2], writer);
	}
	
	DSFWriteToFile(inDSF, writer);
	DSFDestroyWriter(writer);
	return true;	
}
