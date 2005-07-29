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
#include "XGrinderApp.h"
#include "TIGERRead.h"
#include "EnvParser.h"
#include "EnvWrite.h"
#include "TIGERProcess.h"
#include "Persistence.h"
#include "XUtils.h"

void	XGrindFiles(const vector<string>& files)
{
	for (vector<string>::const_iterator f = files.begin();
		f != files.end(); ++f)
	{
		if (HasExtNoCase(*f, ".RT1"))
			TIGER_LoadRT1(f->c_str());
	}
	
	for (vector<string>::const_iterator f = files.begin();
		f != files.end(); ++f)
	{
		if (HasExtNoCase(*f, ".RT2"))
			TIGER_LoadRT2(f->c_str());
	}

		
	for (vector<string>::const_iterator f = files.begin();
		f != files.end(); ++f)
	{
		if (HasExtNoCase(*f, ".env"))
		{
			ClearEnvData();
			if (ReadEnvFile(f->c_str()) == 0)
			{
				gRoads.clear();
				gRiverVectors.clear();
				gTrains.clear();
				gLines.clear();
				
				for (ChainInfoMap::iterator i = gChains.begin(); i != gChains.end(); ++i)
				{
					string	cfcc = i->second.cfcc.substr(0, 2);
					if (cfcc[0] == 'A')
					{
						LatLonVector	v;
						ConvertLatLons(i->second, v);
						bool	ok = true;
						for (int n = 0; n < v.size(); ++n)
						{
							if (v[n].first < 32.0)	 ok = false;
							if (v[n].first > 33.0)	 ok = false;
							if (v[n].second < -118.0)ok = false;
							if (v[n].second > -117.0)ok = false;
						}
						
						if (ok)
						{
							for (int n = 0; n < v.size(); ++n)
							{
								PathInfo	p;
								p.latitude = v[n].first;
								p.longitude = v[n].second;
								p.term = ((n+1)==v.size()) ? 1 : 0;
								gRoads.push_back(p);
							}
						}
					}
					
					
					if (cfcc[0] == 'H')
					{
						LatLonVector	v;
						ConvertLatLons(i->second, v);
						bool	ok = true;
						for (int n = 0; n < v.size(); ++n)
						{
							if (v[n].first < 32.0)	 ok = false;
							if (v[n].first > 33.0)	 ok = false;
							if (v[n].second < -118.0)ok = false;
							if (v[n].second > -117.0)ok = false;
						}
						
						if (ok)
						{
							for (int n = 0; n < v.size(); ++n)
							{
								PathInfo	p;
								p.latitude = v[n].first;
								p.longitude = v[n].second;
								p.term = ((n+1)==v.size()) ? 1 : 0;
								gRiverVectors.push_back(p);
							}
						}
					}					

					if (cfcc[0] == 'B')
					{
						LatLonVector	v;
						ConvertLatLons(i->second, v);
						bool	ok = true;
						for (int n = 0; n < v.size(); ++n)
						{
							if (v[n].first < 32.0)	 ok = false;
							if (v[n].first > 33.0)	 ok = false;
							if (v[n].second < -118.0)ok = false;
							if (v[n].second > -117.0)ok = false;
						}
						
						if (ok)
						{
							for (int n = 0; n < v.size(); ++n)
							{
								PathInfo	p;
								p.latitude = v[n].first;
								p.longitude = v[n].second;
								p.term = ((n+1)==v.size()) ? 1 : 0;
								gTrains.push_back(p);
							}
						}
					}										

					if (cfcc == "C20")
					{
						LatLonVector	v;
						ConvertLatLons(i->second, v);
						bool	ok = true;
						for (int n = 0; n < v.size(); ++n)
						{
							if (v[n].first < 32.0)	 ok = false;
							if (v[n].first > 33.0)	 ok = false;
							if (v[n].second < -118.0)ok = false;
							if (v[n].second > -117.0)ok = false;
						}
						
						if (ok)
						{
							for (int n = 0; n < v.size(); ++n)
							{
								PathInfo	p;
								p.latitude = v[n].first;
								p.longitude = v[n].second;
								p.term = ((n+1)==v.size()) ? 1 : 0;
								gLines.push_back(p);
							}
						}
					}										

					
				}
				

				string	newname = f->substr(0, f->length() - 4) + "_new.env";
				EnvWrite(newname.c_str());
				
				XGrinder_ShowMessage("Created %d road segs, %d hydro lines, %d rails, %d power lines", gRoads.size(), gRiverVectors.size(), gTrains.size(), gLines.size());
				
			}
		}
	}
	
}

void	XGrindInit(string& t)
{
	t = "TigerTool";
	XGrinder_ShowMessage("Drag .rt1, .rt2 and .env files to replace roads.");
}	