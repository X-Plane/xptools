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
#include "AptElev.h"
#include "MemFileUtils.h"
#include "DEMDefs.h"
#include "DEMIO.h"
#include "XESConstants.h"

#include <list>

int	lon_offsets[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
int	lat_offsets[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };

RwyInfoMap	gRwyInfoMap;

static int	bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}




bool	ReadAirportRawElevations(const char * inFile)
{
	bool	ret = false;
	string 	my_friend, me;
	MFMemFile *		f = NULL;
	MFTextScanner *	s = NULL;
	int n = 0;
	map<string, double>	db_lat, db_lon, db_elev;
	float	elev = -32768.0;
	char	last_icao[10] = { 0 };

	f = MemFile_Open(inFile);		if (f == NULL) goto bail;
	s = TextScanner_Open(f);		if (s == NULL) goto bail;

	while (!TextScanner_IsDone(s))
	{
		RwyInfo	info;
		char	icao[10], rec_code[10];

		if (TextScanner_FormatScan(s, "tt", icao, rec_code) == 2)
		{
			if (strcmp(icao, last_icao))
			{
				db_lat.clear();
				db_lon.clear();
				db_elev.clear();
			}
			strcpy(last_icao, icao);

			if (!strcmp(rec_code, "APT"))
			{
				if (TextScanner_FormatScan(s, "     f", &elev) != 6)
				{
					printf("Bad APT line.\n");
					return false;
				}
				elev *= FT_TO_MTR;
			}
			else if (!strcmp(rec_code, "TWY"))
			{
				if (elev == -32768.0)
				{
					printf("Error, no elevation!");
					return false;
				}
				double	cent_lat, cent_lon, rot, len;
				string	nam;
				if (TextScanner_FormatScan(s, "  dddd", &len, &cent_lat, &cent_lon, &rot) != 6)
				{
					printf("Couldn't read 10 line in apt.dat.\n");
					return false;
				}
				double	dx_m = (len * 0.5 * FT_TO_MTR) * sin(rot * DEG_TO_RAD);
				double	dy_m = (len * 0.5 * FT_TO_MTR) * cos(rot * DEG_TO_RAD);

				info.debug = string(icao) + " " + "TWY";
				info.lo_elev = elev;
				double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * cos(cent_lat * DEG_TO_RAD);
				info.lo_lat = cent_lat - dy_m / DEG_TO_MTR_LAT;
				info.lo_lon = cent_lon - dx_m / DEG_TO_MTR_LON;
				info.weight = 0.25;
				info.hi_lat = 0.0;
				info.hi_lon = 0.0;
				info.hi_elev = 0.0;
				info.recip = 0;
				set<int>	hashes;
				for (int n = 0; n < 9; ++n)
				{
					hashes.insert(hash_ll(
							floor(info.lo_lon)+lon_offsets[n],
							floor(info.lo_lat)+lat_offsets[n]));
				}
				for (set<int>::iterator i = hashes.begin(); i != hashes.end(); ++i)
				{
					gRwyInfoMap[*i].push_back(info);
				}

				info.hi_lon = info.lo_lon;
				info.hi_lat = info.lo_lat;
				info.hi_elev = info.lo_elev;
				info.debug = string(icao) + " " + "TWY_recip";
				info.recip = 1;
				info.lo_lon = cent_lon + dx_m / DEG_TO_MTR_LON;
				info.lo_lat = cent_lat + dy_m / DEG_TO_MTR_LAT;
				hashes.clear();
				for (int n = 0; n < 9; ++n)
				{
					hashes.insert(hash_ll(
							floor(info.lo_lon)+lon_offsets[n],
							floor(info.lo_lat)+lat_offsets[n]));
				}
				for (set<int>::iterator i = hashes.begin(); i != hashes.end(); ++i)
				{
					gRwyInfoMap[*i].push_back(info);
				}

			} else {

				char	id[10], alt[10];
				if (TextScanner_FormatScan(s, "     t", alt) == 6)
				if (!strcmp(alt, "NULL"))
				{
					TextScanner_Next(s);
					continue;
				}

				if (TextScanner_FormatScan(s, "tttddd", icao, id, alt, &info.lo_lat, &info.lo_lon, &info.lo_elev) != 6)
				{
					printf("Bad record.\n");
					return false;
				}

				my_friend = string(icao) + " " + alt;
				if (db_lat.count(my_friend))
				{
					info.hi_lat = db_lat[my_friend];
					info.hi_lon =  db_lon[my_friend];
					info.hi_elev = db_elev[my_friend];
					info.recip = 1;
					info.weight = 1.0;
				} else {
					info.hi_elev = 0.0;
					info.hi_lat = 0.0;
					info.hi_lon = 0.0;
					info.recip = 0;
					info.weight = 0.50;
				}

				info.lo_elev *= FT_TO_MTR;

				me = string(icao) + " " + id;
				db_lat[me] = info.lo_lat;
				db_lon[me] = info.lo_lon;
				db_elev[me] = info.lo_elev;

				info.debug = me + "/" + alt;

				set<int>	hashes;
				for (int n = 0; n < 9; ++n)
				{
					hashes.insert(hash_ll(
							floor(info.lo_lon)+lon_offsets[n],
							floor(info.lo_lat)+lat_offsets[n]));
				}
				for (set<int>::iterator i = hashes.begin(); i != hashes.end(); ++i)
				{
					gRwyInfoMap[*i].push_back(info);
				}

			}
		}
		TextScanner_Next(s);
	}
	ret = true;
bail:
	if (s) TextScanner_Close(s);
	if (f) MemFile_Close(f);
	return ret;
}



DEMGeo *	ReadFloatHGTCached(const char * fname)
{
	typedef pair<string, DEMGeo *>		CacheItem;
	typedef list<CacheItem> CacheItemList;
	static	CacheItemList cache;

	for (CacheItemList::iterator i = cache.begin(); i != cache.end(); ++i)
	{
		if (i->first == fname)
		{
			CacheItem item = *i;
			cache.erase(i);
			cache.push_back(item);
//			printf("Cache hit with %s\n", fname);
			return item.second;
		}
	}
	DEMGeo * dem = new DEMGeo;
//	printf("Loading %s into cache.\n", fname);
	if (!ReadFloatHGT(*dem, fname))
	{
		delete dem;
		return NULL;
	}
	while (cache.size() > 50)
	{
//		printf("Purging %s from cache.\n", cache.front().first.c_str());
		delete cache.front().second;
		cache.pop_front();
	}
	cache.push_back(CacheItem(fname, dem));
	return dem;
}

#if 0
bool	ProcessAirportElevations(const char * demdir)
{
	DEMGeo *	dems[9];
	float		h;

	multimap<double, string>	worst;
	set<string>					seen;

	for (RwyInfoMap::iterator b = gRwyInfoMap.begin(); b != gRwyInfoMap.end(); ++b)
	{
		int lon = unhash_lon(b->first);
		int lat = unhash_lat(b->first);
		printf("Processing degree %d,%d\n", lon, lat);
		for (int n = 0; n < 9; ++n)
		{
			char	fname[1024];
			sprintf(fname, "%s%+03d%+04d/%+03d%+04d.DEM", demdir,
				bucket(lat+lat_offsets[n]), bucket(lon + lon_offsets[n]),
				lat+lat_offsets[n], lon + lon_offsets[n]);
			dems[n] = ReadFloatHGTCached(fname);
		}

		for (RwyInfoVector::iterator rwy = b->second.begin(); rwy != b->second.end(); ++rwy)
		{
			for (int n = 0; n < 9; ++n)
			if (dems[n])
			{
				h = dems[n]->value_linear(rwy->lo_lon, rwy->lo_lat);
				if (h != NO_DATA)
				{
					rwy->lo_diff = rwy->lo_elev - h;
				}
			}
		}
	}

	FILE * fi = fopen("worst.txt", "w");
	for (multimap<double,string>::iterator i = worst.begin(); i != worst.end(); ++i)
	{
		fprintf(fi, "%f %s\n", i->first, i->second.c_str());
	}

	return true;
}
#endif

bool	WriteAirportElevations(const char * demdir, const char * outdir)
{
	for (RwyInfoMap::iterator b = gRwyInfoMap.begin(); b != gRwyInfoMap.end(); ++b)
	{
		int lon = unhash_lon(b->first);
		int lat = unhash_lat(b->first);
		char	fname[1024], dname[1024];
		sprintf(fname, "%s%+03d%+04d/%+03d%+04d.elv", outdir, bucket(lat), bucket(lon), lat, lon);
		sprintf(dname, "%s%+03d%+04d/%+03d%+04d.DEM", demdir, bucket(lat), bucket(lon), lat, lon);
		if (!b->second.empty())
		{
			FILE * f = fopen(dname, "r");
			if (f != NULL)
			{
				fclose(f);

				FILE * fi = fopen(fname, "w");
				if (fi == NULL) {
					printf("Problem Writing %d airports to %s\n", b->second.size(), fname);
					return false;
				}
				for (RwyInfoVector::iterator rwy = b->second.begin(); rwy != b->second.end(); ++rwy)
				{
					fprintf(fi, "%lf %lf %lf %d %lf %lf %lf %lf %s\n",
						rwy->lo_lon, rwy->lo_lat, rwy->lo_elev, rwy->recip,
						rwy->hi_lon, rwy->hi_lat, rwy->hi_elev, rwy->weight, rwy->debug.c_str());
				}
				fclose(fi);
			}
		}
	}
	return true;
}

#if 0
void	PurgeAirports(void)
{
	set<int>	nuke;
	for (RwyInfoMap::iterator b = gRwyInfoMap.begin(); b != gRwyInfoMap.end(); ++b)
	{
		for (RwyInfoVector::iterator rwy = b->second.begin(); rwy != b->second.end(); )
		{
			if (rwy->lo_diff == NO_DATA)
			{
//				printf("No info about runway %s\n", rwy->debug.c_str());
				rwy = b->second.erase(rwy);
			}
			else
				++rwy;

		}
		if (b->second.empty())
			nuke.insert(b->first);
	}
	for (set<int>::iterator i = nuke.begin(); i != nuke.end(); ++i)
	{
//		printf("Purging tile %d, %d\n", unhash_lon(*i), unhash_lat(*i));
		gRwyInfoMap.erase(*i);
	}
}
#endif

#define	RADIUS 22

void	AssympApply(DEMGeo& ioDem, int x, int y, float v, float w, bool zap)
{
	for (int dx = x-1; dx <= x+1; ++dx)
	for (int dy = y-1; dy <= y+1; ++dy)
	{
		if (zap)
		{
			if (ioDem.get(dx, dy) != DEM_NO_DATA)
				ioDem(dx, dy) = 0.0;
		} else {
			float ww = (dx == x && dy == y) ? (w * 1.0) : (w * 0.5);

			float h = ioDem.get(dx,dy);
			if (h != DEM_NO_DATA)
			{
				ioDem(dx,dy) = h * (1.0 - ww) + v * ww;
			}
		}
	}
}

void	BuildDifferentialDegree(const char * file, int west, int south, int hres, int vres, DEMGeo& ioDem, bool zap)
{
	DEMGeo	dem(hres, vres);
	dem.resize(hres, vres);
	dem.mWest = west;
	dem.mSouth = south;
	dem.mNorth = south + 1;
	dem.mEast = west + 1;
	FILE * fi = fopen(file, "r");
	vector<RwyInfo>	infos;
	if (fi)
	{
		char	buf[1024];
		while (fgets(buf, 1024, fi))
		{
			RwyInfo	info;
			sscanf(buf, "%lf %lf %lf %d %lf %lf %lf %lf",
				&info.lo_lon, &info.lo_lat, &info.lo_elev, &info.recip, &info.hi_lon, &info.hi_lat, &info.hi_elev, &info.weight);
			infos.push_back(info);
		}
	}
	fclose(fi);
	float rad_fac = sqrt((float)RADIUS*RADIUS);
	for (int n = 0; n < infos.size(); ++n)
	{
		int x1 = ioDem.lon_to_x(infos[n].lo_lon);
		int y1 = ioDem.lat_to_y(infos[n].lo_lat);

		AssympApply(ioDem,x1,y1,infos[n].lo_elev,infos[n].weight, zap);

		if (infos[n].recip)
		{
			int x2 = ioDem.lon_to_x(infos[n].hi_lon);
			int y2 = ioDem.lat_to_y(infos[n].hi_lat);

			AssympApply(ioDem,x2,y2,infos[n].hi_elev, infos[n].weight, zap);

			int steps = sqrt((float)(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))+1;

			for (int t = 0; t <= steps; ++t)
			{
				float rat = ((float) t / (float) steps);
				float   e = rat * infos[n].hi_elev + (1.0 - rat) * infos[n].lo_elev;
				float	lon = rat * infos[n].hi_lon  + (1.0 - rat) * infos[n].lo_lon;
				float	lat = rat * infos[n].hi_lat  + (1.0 - rat) * infos[n].lo_lat;

				int x3 = ioDem.lon_to_x(lon);
				int y3 = ioDem.lat_to_y(lat);

				AssympApply(ioDem,x3,y3,e,0.75 + 0.5 * fabs(rat-0.5) * infos[n].weight, zap);
			}
		}
	}

//	for (int xp = 0; xp < ioDem.mWidth; xp++)
//	for (int yp = 0; yp < ioDem.mHeight; yp++)
//	{
//		float h = ioDem(xp, yp);
//		if (h != DEM_NO_DATA)
//		{
//			h += dem(xp,yp);
//		}
//	}
}
