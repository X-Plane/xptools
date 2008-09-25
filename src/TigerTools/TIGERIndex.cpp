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
#include "TIGERRead.h"
#include "TigerProcess.h"

#include <string>
using namespace std;

int	main(int argc, char ** argv)
{
	if ((argc == 2 && !strcmp(argv[1], "-h")) ||
		(argc == 1))
	{
		printf("Usage: TigerIndex filename.RT1 - output square area covered by RT1 and RT2 files.\n");
		printf("       TigerIndex -i index lat lon - output list of files needed for env tile.\n");

		return 0;
	}
	if (argc == 5 && (!strcmp(argv[1], "-i") || !strcmp(argv[1], "-iv")))
	{
		TigerMap	myMap;
		ReadTigerIndex(argv[2], myMap);
		long	hash = atoi(argv[3]) * 360 + atoi(argv[4]);
		FileList&	files = myMap[hash];
		bool	verbose = (!strcmp(argv[1], "-iv"));
		for (FileList::iterator iter = files.begin(); iter != files.end(); ++iter)
		{
			if (verbose)
				printf("%s %f %f %f %f\n",iter->name.c_str(),
					iter->lat_min, iter->lat_max, iter->lon_min, iter->lon_max);
			else
				printf("%s\n",iter->name.c_str());
		}
		return 0;
	}

	for (int n = 1; n < argc; ++n)
	{
		gChains.clear();
		gLandmarks.clear();
		gPolygons.clear();

		string	path = argv[n];
		int		len = path.length();
		string	file = path.substr(len-12, 8);
		string	dir = (len >= 15) ? path.substr(len-15, 2) : string();
		string	rt2 = path.substr(0, len-1) + "2";

		TIGER_LoadRT1(path.c_str());
		TIGER_LoadRT2(rt2.c_str());

		LatLonVector	v;

		double	latMin =  1000.0;
		double	latMax = -1000.0;
		double	lonMin =  1000.0;
		double	lonMax = -1000.0;

		for (ChainInfoMap::iterator i = gChains.begin();
			i != gChains.end(); ++i)
		{
			ConvertLatLons(i->second, v);
			for (LatLonVector::iterator l = v.begin(); l != v.end(); ++l)
			{
				latMin = min(latMin, l->first);
				latMax = max(latMax, l->first);
				lonMin = min(lonMin, l->second);
				lonMax = max(lonMax, l->second);
			}
		}

		printf("%s/%s.RT1 %f %f %f %f\n",
			dir.c_str(), file.c_str(), latMin, latMax, lonMin, lonMax);

	}
	return 0;
}
