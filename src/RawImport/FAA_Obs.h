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
#ifndef FAA_OBS_H
#define FAA_OBS_H

#include <math.h>

class	Pmwx;

struct	FAAObs_t {
	string		kind_str;
	string		freq;
	double		lon;
	double		lat;
	float		msl;
	float		agl;
	int			kind;
};

typedef hash_multimap<int, FAAObs_t>		FAAObsTable;

extern		FAAObsTable		gFAAObs;
extern		const char *	gObsNames[];

inline int		HashLonLat(double lon, double lat)
{
	return ((int) floor(lon)) + 360 * ((int) floor(lat));
}

bool	LoadFAAObsFile(const char * inFile);
bool	LoadLegacyObjectArchive(const char * inFile);
bool	LoadFAARadarFile(const char * inFile, bool isApproach);

bool	ReadDegFile(const char * inFile);
bool	WriteDegFile(const char * inFile, int lon, int lat);

bool	ReadAPTNavAsObs(const char * inFile);

int		GetObjMinMaxHeights(map<int, float>& mins, map<int, float>& maxs);

void ApplyObjects(Pmwx& ioMap);

#endif
