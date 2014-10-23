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
#ifndef DEMALGS_H
#define DEMALGS_H

// Now you give dem algs to hypno-nerd...

#include "MapDefs.h"
#include "DEMDefs.h"

//class	Pmwx;

#include "AptDefs.h"
#include "ProgressUtils.h"

enum {
	demFilter_Linear,		// A radial linear ramp from center to edges r
	demFilter_Spread		// Simply spread the image out.
};

struct	DEMPrefs_t {
	int		local_range;
	float	temp_percentile;
	float	rain_disturb;
};

extern DEMPrefs_t	gDemPrefs;

void	CalculateFilter(int dim, float * k, int kind, bool normalize);

// Sample down or up by ratio.  Downsample via average, upsample via data dupe.
void	DownsampleDEM(const DEMGeo& inDem, DEMGeo& outSmaller, int ratio);
void	UpsampleDEM(const DEMGeo& inDem, DEMGeo& outSmaller, int ratio);

// Resample a DEM, changing anything - size, location, postings...does linear interp resample.
// This will alias if src res is more than 2x dst res.
void	ResampleDEM(const DEMGeo& inSrc, DEMGeo& inDst);
void 	ResampleDEMmedian(const DEMGeo& inSrc, DEMGeo& inDst, int radius);

void	InterpDoubleDEM(const DEMGeo& inDEM, DEMGeo& outBigger);
void	ReduceToBorder(const DEMGeo& inDEM, DEMGeo& outDEM);
void	SpreadDEMValues(DEMGeo& ioDem);
void	SpreadDEMValuesTotal(DEMGeo& ioDem);
bool	SpreadDEMValuesIterate(DEMGeo& ioDem);
void	SpreadDEMValues(DEMGeo& ioDem, int dist, int x1, int y1, int x2, int y2);
void	UpsampleFromParamLinear(DEMGeo& masterOrig, DEMGeo& masterDeriv, DEMGeo& slaveOrig, DEMGeo& slaveDeriv);
int		BinaryDEMFromEnum(DEMGeo& dem, float value, float inAccept, float inFail);

/* FFT calculation: we turn a DEM into a series of DEMs - the first is the size
 * of the original DEM and the last is 1x1. */
void	DEMMakeFFT(const DEMGeo& inDEM, vector<DEMGeo>& outFFT);
void	FFTMakeDEM(const vector<DEMGeo>& inFFT, DEMGeo& outDEM);

/* Histogram calculations */
int		DEMMakeHistogram(const DEMGeo& inDEM, map<float, int>& histo, int x1, int y1, int x2, int y2);
float	HistogramGetPercentile(const map<float, int>& histo, int total_samples, float percentile);
void	DEMMakeDifferential(const DEMGeo& inSrc, DEMGeo& dst);

void	CalcSlopeParams(DEMGeoMap& ioDEMs, bool force, ProgressFunc inProg);
void	UpsampleEnvironmentalParams(DEMGeoMap& ioDEMs, ProgressFunc inProg);
void	DeriveDEMs(Pmwx& inMap, DEMGeoMap& ioDEMs, AptVector& ioApts, AptIndex& ioAptIndex, int do_translate, ProgressFunc inProg);

void	MakeTiles(const DEMGeo& inDEM, list<DEMGeo>& outTiles);


void	DifferenceDEM(const DEMGeo& bottom, const DEMGeo& top, DEMGeo& diff);
void	GaussianBlurDEM(DEMGeo& dem, float sigma);

float	IntegLine(const DEMGeo& dem, double x1, double y1, double x2, double y2, int over_sample_ratio);

/* WATERSHED GUNK */

void	NeighborHisto(const DEMGeo& input, DEMGeo& output, int semi_distance);
void	Watershed(DEMGeo& input, DEMGeo& output, vector<DEMGeo::address> * out_watersheds);
void	FindWatersheds(DEMGeo& ws, vector<DEMGeo::address>& out_sheds);
void	MergeMMU(DEMGeo& ws, vector<DEMGeo::address>& io_sheds, int min_mmu_size);
void	VerifySheds(const DEMGeo& ws, vector<DEMGeo::address>& seeds);
void	SetWatershedsToDominant(DEMGeo& underlying, DEMGeo& ws, const vector<DEMGeo::address>& io_sheds);

#endif
