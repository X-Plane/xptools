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
#include "RF_DEMGraphics.h"
#include "ParamDefs.h"
#include "DEMTables.h"
#include "CompGeomDefs3.h"
#include "TensorUtils.h"
#include "CompGeomDefs2.h"
#include "perlin.h"

#define		DDA_STEPS 10.0f
#define		DDA_FACTOR 0.5

#define	ARRAY_COUNT(x)		(sizeof(x) / sizeof(x[0]))

float	kColorBandsRelativeElevation[][4] = {
	0, 255, 0,	-5.0,
	0, 255, 0,   0.0,
	255, 255, 0,   0.3,
	255, 255, 0,   0.7,
	127, 127, 127,   1.0,
	127, 127, 127,   5.0,
	0, 0, 0,   -99.0
};

float kColorBandsElevationRange[][4] = {
0, 0, 0,		0.0,
0, 127, 0,  	100.0,
127, 127, 0,  	500.0,
127, 0, 0,  	1000.0,
127, 0, 127,  	2000.0,
0, 0, 127,  	3000.0,
0, 0, 0,  	-99.0,
};

float	kColorBands[][4] = {
	0.0,			0.0, 1.0, 0.0,
	500.0,			0.0, 1.0, 1.0,
	1000.0,			0.0, 0.0, 1.0,
	1600.0,			1.0, 0.0, 1.0,
	2000.0,			1.0, 0.0, 0.0,
	2400.0,			0.5, 0.5, 0.5,
	-99.0,			0.0, 0.0, 0.0
};
/*

float	kColorBands[][4] = {
	0.0,			0.8, 0.8, 0.8,
	100.0,			0.2, 0.2, 0.2,
	200.0,			0.8, 0.8, 0.8,
	300.0,			0.2, 0.2, 0.2,
	400.0,			0.8, 0.8, 0.8,
	500.0,			0.2, 0.2, 0.2,
	600.0,			0.8, 0.8, 0.8,
	700.0,			0.2, 0.2, 0.2,
	800.0,			0.8, 0.8, 0.8,
	900.0,			0.2, 0.2, 0.2,
	1000.0,			0.8, 0.8, 0.8,
	-99.0,			0.0, 0.0, 0.0
};
*/

float	kColorBandsRainfallYearly[][4] = {
	230,230,230,0   		,
	190,190,190,25  		,
	150,150,150,75  		,
	196,112,110,125 		,
	200,179,150,225 		,
	255,199,117,275 		,
	255,255,84 ,375 		,
	145,255,153,475 		,
	0  ,255,0  ,725 		,
	64 ,199,56 ,975 		,
	13 ,150,5  ,1475		,
	5  ,112,94 ,2475		,
	255,0  ,255,4975		,
	128,0  ,128,7475		,
	0  ,138,255,10005		,
	0  ,0  ,0	,DEM_NO_DATA
};

float	kColorBandsDrainage[][4] = {
	230,230,230,0   		,
	190,190,190,25  		,
	150,150,150,75  		,
	196,112,110,125 		,
	200,179,150,225 		,
	255,199,117,275 		,
	255,255,84 ,375 		,
	145,255,153,475 		,
	0  ,255,0  ,725 		,
	64 ,199,56 ,975 		,
	13 ,150,5  ,1475		,
	5  ,112,94 ,2475		,
	255,0  ,255,4975		,
	128,0  ,128,7475		,
	0  ,138,255,10005		,
	0  ,0  ,0	,DEM_NO_DATA
};

float	kColorBandsBioMass[][4] = {
	230,230,230,0,
	160,160,160,10,
	255,230,150,30,
	255,199,117,50,
	181,112,110,70,
	158,186,90,	110,
	255,255,0,	170,
	160,255,0,	310,
	10,	230,30,	610,
	0,	170,0,	1210,
	5,	112,94,	1510,
	0,	250,255,2010,
	0,	138,255,2510,
	0,	0,	0,	DEM_NO_DATA
};

float	kColorBandsTemperature[][4] = {
160	,160	,160	,-60.0		,
192	,192	,192	,-29.5 		,
220	,220	,220	,-24.5 		,
138	,200	,230	,-14.5 		,
0	,240	,255	,-9.5  		,
138	,240	,145	,-4.5 		,
234	,255	,145	,0.5   		,
255	,255	,0		,5.5  		,
181	,143	,0		,10.5  		,
255	,143	,0		,15.5  		,
255	,84		,0		,20.5  		,
255	,0		,0		,30.5  		,
194	,0		,0		,35.5  		,
97	,0		,0		,40.0		,

/*	Sergo's colors for vegetation bands.
0,	0,		255,	-60.0,
0,	0,		255,	-20.0,

150,150,	250,	-20.0,
150,150,	250,	-10.0,

230,230,	230,	-10.0,
230,230,	230,	  0.0,

0,	255,	0,		  0.0,
0,	255,	0,		 10.0,

255,128,	0,		 10.0,
255,128,	0,		 20.0,

255,0,		0,		 20.0,
255,0,		0,		 60.0,
*/
0	,0		,0		,DEM_NO_DATA
};


float	kColorBandsTemperatureRange[][4] = {
255,	255,	141,	0.0 ,
237,	237,	135,	5.0 ,
219,	219,	85 ,	10.0,
201,	201,	51 ,	15.0,
182,	182,	88 ,	20.0,
197,	164,	78 ,	25.0,
246,	164,	78 ,	30.0,
255,	153,	39 ,	35.0,
255,	145,	119,	40.0,
255,	120,	67 ,	45.0,
207,	135,	149,	50.0,
168,	103,	129,	55.0,
150,	79 ,	90 ,	60.0,
130,	50 ,	50 ,	65.0,
0  ,	0  ,	0  ,	DEM_NO_DATA
};

void	GetColorForTable(float v, ColorBandMap& table, unsigned char col[3])
{
	ColorBandMap::iterator i = table.lower_bound(v);
	if (i == table.end())
	{
		col[0] = col[1] = col[2] = 0;
		return;
	}

	if (v < i->second.lo_value || v > i->second.hi_value)
	{
		col[0] = col[1] = col[2] = 0;
		return;
	}

	float mix2 = (v - i->second.lo_value) / (i->second.hi_value - i->second.lo_value);
	float mix1 = 1.0 - mix2;

	float red = i->second.lo_color.rgb[0] * mix1 + i->second.hi_color.rgb[0] * mix2;
	float grn = i->second.lo_color.rgb[1] * mix1 + i->second.hi_color.rgb[1] * mix2;
	float blu = i->second.lo_color.rgb[2] * mix1 + i->second.hi_color.rgb[2] * mix2;

	col[0] = red * 255.0;
	col[1] = grn * 255.0;
	col[2] = blu * 255.0;
}

void	GetColorForParam(float	v, float mapping[][4], int num_bands, unsigned char col[3])
{
	for (int n = 0; n < (num_bands-1); ++n)
	{
		if (mapping[n  ][3] <= v &&
			mapping[n+1][3] >= v)
		{
			double	scale = (v - mapping[n][3]) /
							(mapping[n+1][3] - mapping[n][3]);
			double	scalei = 1.0 - scale;

			col[0] = mapping[n][0] * scalei + mapping[n+1][0] * scale;
			col[1] = mapping[n][1] * scalei + mapping[n+1][1] * scale;
			col[2] = mapping[n][2] * scalei + mapping[n+1][2] * scale;
			return;
		}
	}
	col[0] = 	mapping[num_bands-1][0];
	col[1] = 	mapping[num_bands-1][1];
	col[2] = 	mapping[num_bands-1][2];
}

void	GetColorForAlt(float alt, unsigned char col[3])
{
	int n = 0;
	while (kColorBands[n][0] != -99.0)
	{
		if (kColorBands[n  ][0] <= alt &&
			kColorBands[n+1][0] >= alt)
		{
			double	scale = (alt - kColorBands[n][0]) /
							(kColorBands[n+1][0] - kColorBands[n][0]);
			double	scalei = 1.0 - scale;

			double	r = kColorBands[n][1] * scalei + kColorBands[n+1][1] * scale;
			double	g = kColorBands[n][2] * scalei + kColorBands[n+1][2] * scale;
			double	b = kColorBands[n][3] * scalei + kColorBands[n+1][3] * scale;

			col[0] = 255.0 * r;
			col[1] = 255.0 * g;
			col[2] = 255.0 * b;
			return;
		}

		++n;
	}
	col[0] = 255;
	col[1] = 0;
	col[2] = 255;
}

void GetColorForLU(float alt, unsigned char col[3])
{
	RGBColor_t&	c = gEnumColors[alt];
	col[0] = 255.0 * c.rgb[0];
	col[1] = 255.0 * c.rgb[1];
	col[2] = 255.0 * c.rgb[2];
}


int	DEMToBitmap(
				const DEMGeo& 	inDEM,
				ImageInfo&		outImage,
				int				inMode)
{
	int x, y, ch;
	int	err = CreateNewBitmap(inDEM.mWidth, inDEM.mHeight, 3, &outImage);
	if (err) return err;
	outImage.pad = 0;

	float	dh_max = 0;

	float h, ha, hr, dh, smin, smax, scaled;

	float (*vp)[4];
	int		cnt;
	switch(inMode) {
	case dem_StrataBiomass:
		vp = kColorBandsBioMass;
		cnt = ARRAY_COUNT(kColorBandsBioMass);
		break;
	case dem_StrataRainfallYearly:
		vp = kColorBandsRainfallYearly;
		cnt = ARRAY_COUNT(kColorBandsRainfallYearly);
		break;
	case dem_StrataTemperature:
		vp = kColorBandsTemperature;
		cnt = ARRAY_COUNT(kColorBandsTemperature);
		break;
	case dem_StrataTemperatureRange:
		vp = kColorBandsTemperatureRange;
		cnt = ARRAY_COUNT(kColorBandsTemperatureRange);
		break;
	case dem_StrataElevationRange:
		vp = kColorBandsElevationRange;
		cnt = ARRAY_COUNT(kColorBandsElevationRange);
		break;
	case dem_StrataRelativeElevation:
		vp = kColorBandsRelativeElevation;
		cnt = ARRAY_COUNT(kColorBandsRelativeElevation);
		break;
	case dem_StrataDrainage:
		vp = kColorBandsDrainage;
		cnt = ARRAY_COUNT(kColorBandsDrainage);
		break;
	}

	if (gColorBands.count(inMode) != 0)
	{
		ColorBandMap& table(gColorBands[inMode]);
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float h = inDEM(x,y);
			unsigned char col[3];
			GetColorForTable(h, table, col);
			outImage.data[(x + y * outImage.width) * outImage.channels  ] = col[2];
			outImage.data[(x + y * outImage.width) * outImage.channels+1] = col[1];
			outImage.data[(x + y * outImage.width) * outImage.channels+2] = col[0];
		}
		return 0;
	}


	switch(inMode) {
	case dem_StrataBiomass:
	case dem_StrataRainfallYearly:
	case dem_StrataTemperature:
	case dem_StrataTemperatureRange:
	case dem_StrataElevationRange:
	case dem_StrataRelativeElevation:
	case dem_StrataDrainage:
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float h = inDEM(x,y);
			unsigned char col[3];
			GetColorForParam(h, vp, cnt, col);

			outImage.data[(x + y * outImage.width) * outImage.channels  ] = col[2];
			outImage.data[(x + y * outImage.width) * outImage.channels+1] = col[1];
			outImage.data[(x + y * outImage.width) * outImage.channels+2] = col[0];
		}
		break;
	case dem_Enum:
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float h = inDEM(x,y);
			unsigned char col[3];
			GetColorForLU(h, col);
			outImage.data[(x + y * outImage.width) * outImage.channels  ] = col[2];
			outImage.data[(x + y * outImage.width) * outImage.channels+1] = col[1];
			outImage.data[(x + y * outImage.width) * outImage.channels+2] = col[0];
		}
		break;
	case dem_Strata:
		smin = 9.9e9;
		smax = -9.9e9;
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float h = inDEM(x,y);
			if (h != DEM_NO_DATA && h < smin) smin = h;
			if (h != DEM_NO_DATA && h > smax) smax = h;
		}
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float h = inDEM(x,y);
			if (h != DEM_NO_DATA && smin != smax) h = ((h - smin) * 1000.0 / (smax - smin));
			unsigned char col[3];
			GetColorForAlt(h, col);

			outImage.data[(x + y * outImage.width) * outImage.channels  ] = col[0];
			outImage.data[(x + y * outImage.width) * outImage.channels+1] = col[1];
			outImage.data[(x + y * outImage.width) * outImage.channels+2] = col[2];
		}
		break;
	case dem_Normals:
		for (y = 0; y < (inDEM.mHeight-1); ++y)
		for (x = 0; x < (inDEM.mWidth-1); ++x)
		{
			h = inDEM(x,y);
			ha = inDEM(x,y+1);
			hr = inDEM(x+1,y);

			Point3	p_h(0,0,h);
			Point3	p_ha(0, inDEM.y_dist_to_m(1), ha);
			Point3	p_hr(inDEM.x_dist_to_m(1), 0, hr);

			Vector3	to_a(p_h, p_ha);
			Vector3	to_r(p_h, p_hr);

			Vector3	n(to_r.cross(to_a));
			n.normalize();

			if (h == DEM_NO_DATA || ha == DEM_NO_DATA || hr == DEM_NO_DATA)
			{
				outImage.data[(x + y * outImage.width) * outImage.channels  ] = 0x80;
				outImage.data[(x + y * outImage.width) * outImage.channels+1] = 0x80;
				outImage.data[(x + y * outImage.width) * outImage.channels+2] = 0x80;
			} else {
				outImage.data[(x + y * outImage.width) * outImage.channels  ] = n.dz * 127.0 + 127.0;
				outImage.data[(x + y * outImage.width) * outImage.channels+1] = n.dy * 127.0 + 127.0;
				outImage.data[(x + y * outImage.width) * outImage.channels+2] = n.dx * 127.0 + 127.0;
			}
		}
		break;

	case dem_Shaded:
		for (y = 0; y < (inDEM.mHeight-1); ++y)
		for (x = 0; x < (inDEM.mWidth-1); ++x)
		{
			h = inDEM(x,y);
			ha = inDEM(x,y+1);
			hr = inDEM(x+1, y);
			if (h == DEM_NO_DATA || ha == DEM_NO_DATA || hr == DEM_NO_DATA) continue;
			ha -= h;
			hr -= h;
			dh = fabs(ha + hr);
			if (dh > dh_max)
				dh_max = dh;
		}
		for (y = 0; y < (inDEM.mHeight-1); ++y)
		for (x = 0; x < (inDEM.mWidth-1); ++x)
		{
			h = inDEM(x,y);
			ha = inDEM(x,y+1);
			hr = inDEM(x+1,y);
			if (h == DEM_NO_DATA || ha == DEM_NO_DATA || hr == DEM_NO_DATA)
				dh = 0.0;
			else {
				ha -= h;
				hr -= h;
				dh = ha + hr;
			}
			scaled = (dh_max > 0.0) ? (dh / dh_max) : 0.0;
			scaled = (scaled * 0.5 + 0.5) * 255.0;

			if (h == DEM_NO_DATA)
			{
				outImage.data[(x + y * outImage.width) * outImage.channels  ] = scaled;
				outImage.data[(x + y * outImage.width) * outImage.channels+1] = 0;
				outImage.data[(x + y * outImage.width) * outImage.channels+2] = 0;
			} else {
				outImage.data[(x + y * outImage.width) * outImage.channels  ] = scaled;
				outImage.data[(x + y * outImage.width) * outImage.channels+1] = scaled;
				outImage.data[(x + y * outImage.width) * outImage.channels+2] = scaled;
			}
		}
		for (y = 0; y < (inDEM.mHeight-1); ++y)
		for (ch = 0; ch < 3; ++ch)
			outImage.data[(outImage.width-1 + y * outImage.width) * outImage.channels + ch] =
			outImage.data[(outImage.width-2 + y * outImage.width) * outImage.channels + ch];
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			outImage.data[(x + (outImage.height-1) * outImage.width) * outImage.channels + ch] =
			outImage.data[(x + (outImage.height-2) * outImage.width) * outImage.channels + ch];
		}
		break;
/*
	case dem_DDA:
		for (y = 0; y < inDEM.mHeight; ++y)
		for (x = 0; x < inDEM.mWidth; ++x)
		{
			float gx = inDEM.gradient_x_bilinear(x,y);
			float gy = inDEM.gradient_y_bilinear(x,y);
			Vector2	e = Tensor2Eigen(Gradient2Tensor(Vector2(gx,gy)));
//			Vector2	e = Tensor2Eigen(Linear_Tensor(Point2(600,600),Vector2(0.717, -0.717), 0.0,Point2(x,y)));
//			Vector2	e = Tensor2Eigen(Radial_Tensor(Point2(600,600),0.0,Point2(x,y)));

			float v = 0.0f;
			Point2 p(x,y);
			for (int n = -DDA_STEPS; n <= DDA_STEPS; ++n)
			{
				Point2 s(p + e * n);
				v += interp_noise_2d(s.x() * DDA_FACTOR,s.y() * DDA_FACTOR,0);
			}
			v /= (DDA_STEPS*2.0f+1.0f);
//			v = e.dy * 0.5 + 0.5;
			outImage.data[(x + y * outImage.width) * outImage.channels  ] = v * 255.0f;
			outImage.data[(x + y * outImage.width) * outImage.channels+1] = v * 255.0f;
			outImage.data[(x + y * outImage.width) * outImage.channels+2] = v * 255.0f;
		}
		break;
*/		
	}
	return 0;
}

void ColorForValue(
				int				dem_type,
				float			value,
				unsigned char	rgb[3])
{
	if (gColorBands.count(dem_type) != 0)
	{
		ColorBandMap& table(gColorBands[dem_type]);
		GetColorForTable(value, table, rgb);
		return;
	}
	if (gEnumDEMs.count(dem_type))
	{
		GetColorForLU(value, rgb);
		return;
	}

	RGBColor_t * c = NULL;
	switch(dem_type) {
	case dem_Elevation:
	case dem_LandUse:
	case dem_Climate:
//	case dem_NudeColor:
		if (gEnumColors.count(value) > 0)	c = &gEnumColors[value];
		if (c) { rgb[0] = c->rgb[0] * 255.0; rgb[1] = c->rgb[1] * 255.0; rgb[2] = c->rgb[2] * 255.0; }
		else   { rgb[0] = rgb[2] = 255.0; rgb[1] = 0.0; }
		return;

	case dem_Temperature:		GetColorForParam(value, kColorBandsTemperature, 		ARRAY_COUNT(kColorBandsTemperature), 		rgb); return;
	case dem_TemperatureRange:	GetColorForParam(value, kColorBandsTemperatureRange, 	ARRAY_COUNT(kColorBandsTemperatureRange), 	rgb); return;
	case dem_Rainfall:			GetColorForParam(value, kColorBandsRainfallYearly, 		ARRAY_COUNT(kColorBandsRainfallYearly), 	rgb); return;
	case dem_Biomass:			GetColorForParam(value, kColorBandsBioMass,				ARRAY_COUNT(kColorBandsBioMass),			rgb); return;

	default:
		if (value > 0.5)
		{
			rgb[0] = 1.0; rgb[1] = 255.0 * (2.0 * (1.0 - value)); rgb[2] = 0.0;
		} else {
			rgb[0] = 255.0 * 2.0 * value; rgb[1] = 1.0; rgb[2] = 0.0;
		}
	}
}

void TensorDDA(
			ImageInfo&	ioImage,
			Vector2 (*	tensor_func)(const Point2& p, void * ref),
			void *		ref)
{
	for (int y = 0; y < ioImage.height; ++y)
	for (int x = 0; x < ioImage.width;  ++x)
	{
		Point2 p((double) x / (double) ioImage.width, (double) y / (double) ioImage.height);
		Vector2 t(tensor_func(p,ref));
		Vector2 e(Tensor2Eigen(t));
		float v = 0.0f;
		Point2 pi(x,y);
		for (int n = -DDA_STEPS; n <= DDA_STEPS; ++n)
		{
			Point2 s(pi + e * n);
			v += interp_noise_2d(s.x() * DDA_FACTOR,s.y() * DDA_FACTOR,0);
		}
		v /= (DDA_STEPS*2.0f+1.0f);
		for(int c = 0; c < ioImage.channels; ++c)
			ioImage.data[(x + y * ioImage.width) * ioImage.channels+c] = v * 255.0f;
//		ioImage.data[(x + y * ioImage.width) * ioImage.channels+0] = 255.0 * e.dx;
//		ioImage.data[(x + y * ioImage.width) * ioImage.channels+1] = 255.0 * e.dy;
//		ioImage.data[(x + y * ioImage.width) * ioImage.channels+2] = 0;
	}
}
