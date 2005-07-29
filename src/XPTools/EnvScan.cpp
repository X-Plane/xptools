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
#include "EnvScan.h"
#include "Persistence.h"
#include "EnvParser.h"
#include "XUtils.h"
#include "XGrinderApp.h"

bool	ScanEnv(const char * inPath, const char * inFileName)
{	
	AcceptDimensions(151, 201);
	ClearEnvData();
	
	bool	err = false;
	int		count;
	string	str;
	VertexInfo	v;
	ObjectInfo	o;
	PathInfo	p;
	char	objName[1024];
			
	StTextFileScanner	sc(inPath, false);
	if (sc.done()) return false;
	
	count = 201 * 151;
	while (count--)
	{
		if (!GetNextNoComments(sc, str))
		{
			err = true;
			XGrinder_ShowMessage("Hit end of file while reading vertices.");
			break;
		}
		if (sscanf(str.c_str(), "%lf,%lf,%lf,%hd,%d,%hd,%hd,%hd,%hd,%hd",
				&v.latitude, 	&v.longitude,	&v.elevation,
				&v.landUse, 	&v.custom, 
				&v.rotation, 	&v.scale,
				&v.xOff, 		&v.yOff, 		&v.bodyID) != 10)
		{
			err = true;
			XGrinder_ShowMessage("Expected vertex, got %s", str.c_str());
			break;
		} else
			gVertices.push_back(v);
	}
	
	while (1)
	{
		if (!GetNextNoComments(sc,str))
		{
			XGrinder_ShowMessage("Hit end of file while reading objects.");
			err = true; 
			break;
		}
		if(str=="END")
			break;
		objName[0] = 0;
		if (sscanf(str.c_str(),"%ld,%lf,%lf,%lf,%[^\n]",
			&o.kind, &o.latitude, &o.longitude, &o.elevation, objName) < 4)
		{
			err = true;
			XGrinder_ShowMessage("Expected object, got %s", str.c_str());
			break;
		} else {
			o.name = objName;
			gObjects.push_back(o);
		}				
	}
	
	const char * net_type;
	for (int n = 0; n < 6; ++n)
	{
		vector<PathInfo> *	a;
		switch(n) {
		case 0: 	a = &gRoads; 		net_type = "road";	break;
		case 1: 	a = &gTrails; 		net_type = "trail";	break;
		case 2: 	a = &gTrains; 		net_type = "train";	break;
		case 3: 	a = &gLines; 		net_type = "line";	break;
		case 4: 	a = &gTaxiways; 	net_type = "taxiway";break;
		case 5: 	a = &gRiverVectors; net_type = "river";	break;
		}
		
		while(1)
		{
			if (!GetNextNoComments(sc,str))
			{
				err = true;
				XGrinder_ShowMessage("Hit end of file reading %ss", net_type);
				break;
			}
			if (str=="END")
				break;
			if (sscanf(str.c_str(),"%lf,%lf,%d", &p.latitude,&p.longitude,&p.term) != 3)
			{
				err = true;
				XGrinder_ShowMessage("Expected %s, got %s", net_type, str.c_str());
				break;
			} else {
				a->push_back(p);
			}
		}
	}
	
	while(1)
	{
		if (!GetNextNoComments(sc,str))
		{
			XGrinder_ShowMessage("Hit end of file reading textures.");
			err = true;
			break;
		}
		if (str=="END")
			break;
		gTextures.push_back(str);
	}
	
	return !err;
}
	
