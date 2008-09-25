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
#ifndef APTELEV_H
#define APTELEV_H

struct	DEMGeo;

struct	RwyInfo {
	double	lo_lat;
	double	lo_lon;
	double	lo_elev;
	int		recip;
	double	hi_lat;
	double	hi_lon;
	double	hi_elev;
	double	weight;
	string	debug;
};
typedef vector<RwyInfo>	RwyInfoVector;

inline int hash_ll(int lon, int lat) {	return (lon+180) + 360 * (lat+90); }
inline int unhash_lat(int h) { return (h / 360)-90 ; }
inline int unhash_lon(int h) { return (h % 360)-180; }

typedef map<int, RwyInfoVector>	RwyInfoMap;


extern RwyInfoMap	gRwyInfoMap;

bool	ReadAirportRawElevations(const char * inFile);

//bool	ProcessAirportElevations(const char * demdir);
//void	PurgeAirports(void);
bool	WriteAirportElevations(const char * demdir, const char * outdir);

void	BuildDifferentialDegree(const char * file, int west, int south, int hres, int vres, DEMGeo& dem, bool zap);

#endif

