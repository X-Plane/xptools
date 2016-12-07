//
//  GISTool_ImageCmds.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 11/12/16.
//
//

#include "GISTool_ImageCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "BitmapUtils.h"
#include "AssertUtils.h"
#include "MathUtils.h"
#include "GISUtils.h"

static void inc_rgba(int rgba8, float io_rgba[4])
{
	int r = (rgba8 & 0xFF000000) >> 24;
	int g = (rgba8 & 0x00FF0000) >> 16;
	int b = (rgba8 & 0x0000FF00) >>  8;
	int a = (rgba8 & 0x000000FF);
	io_rgba[0] += ((float) r / 255.0f);
	io_rgba[1] += ((float) g / 255.0f);
	io_rgba[2] += ((float) b / 255.0f);
	io_rgba[3] += ((float) a / 255.0f);
}

static void write_rgba(const float in_rgba[4], int * dest)
{
	int r = in_rgba[0] * 255.0f;
	int g = in_rgba[1] * 255.0f;
	int b = in_rgba[2] * 255.0f;
	int a = in_rgba[3] * 255.0f;
	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(a > 255) a = 255;
	*dest = (r << 24) | (g << 16) | (b << 8) | a;
}

void FloodFill(ImageInfo * image, int alpha_color, int fill_color, int steps)
{
	Assert(image->channels == 4);
	Assert(image->pad == 0);
	int h = image->height;
	int w = image->width;
	int t = w * h;
	int * s = (int *) image->data;
	int * e = s + t;
	
	int dirs[8] = { -1, w-1, w, w+1, 1, -w+1, -w, -w-1 };
	
	float fill_float[4] = { 0 };
	inc_rgba(fill_color, fill_float);
	
	vector<int *>	todo;
	
	for(int * p = s; p < e; ++p)
	{
		if(*p != alpha_color)
		for(int ni = 0; ni < 8; ++ni)
		{
			int * n = p + dirs[ni];
			if(n >= s && n < e)
				if (*n == alpha_color)
					todo.push_back(n);
		}
	}

	for(int st = 0; st < steps; ++st)
	{
		if(todo.empty())
			break;
		vector<int *> more;
		for(vector<int *>::iterator i = todo.begin(); i != todo.end(); ++i)
		{
			float w = 0.0f;
			float rgba[4] = { 0 };
			int * d = *i;
			
			if(*d == alpha_color)
			{
				for(int ni = 0; ni < 8; ++ni)
				{
					int * n = d + dirs[ni];
					if(n >= s && n < e)
					{
						int nc = *n;
						if(nc == alpha_color)
						{
							more.push_back(n);
						}
						else
						{
							w += 1.0f;
							inc_rgba(nc, rgba);
						}
					}
				}
				DebugAssert(w > 0.0f);
				rgba[0] /= w;
				rgba[1] /= w;
				rgba[2] /= w;
				rgba[3] /= w;
				
				float fill_rat = 1.0f / (float) (steps - st);
				float keep_rat = 1.0f - fill_rat;
				for(int ch = 0; ch < 4; ++ch)
				rgba[ch] = rgba[ch] * keep_rat + fill_float[ch] * fill_rat;
				
				write_rgba(rgba, d);
			}
		}
		
		todo.swap(more);
	}
	
	for(int * p = s; p < e; ++p)
		if(*p == alpha_color)
			*p = fill_color;
}

static int FillWaterImage(const vector<const char *>& args)
{
	ImageInfo i;
	
	int w = atoi(args[0]);
	int s = atoi(args[1]);
	
	int err = CreateNewBitmap(240*12,240*12,4,&i);
	if(err != 0)
		return err;
	
	for(int y = 0; y < 12; ++y)
	for(int x = 0; x < 12; ++x)
	{
		int ww = w - 1 + x;
		int ss = s - 1 + y;
		
		if(ww < -180) ww += 360;
		if(ww >= 180) ww -= 360;
		
		if(ss < -90) ss = -90;
		if(ss >= 90) ss = 89;
		
		int www = latlon_bucket(ww);
		int sss = latlon_bucket(ss);
		
		char buf[50];
		sprintf(buf,"%+03d%+04d/%+03d%+04d.tif",sss,www,ss,ww);
		
		string p(args[2]);
		p += buf;
		
		ImageInfo ii;
		err = CreateBitmapFromTIF(p.c_str(), &ii);
		if(err)
		{
			printf("Could not open: %s\n", p.c_str());
			return err;
		}
		
		CopyBitmapSectionDirect(ii, i, 0, 0, x * 240, y * 240, 240, 240);
		DestroyBitmap(&ii);
		
	}
	
	
	FloodFill(&i, 0xFFFFFFFF, 0xFF020413, 16);
	
	ImageInfo j;
	err = CreateNewBitmap(2048, 2048, 4, &j);
	if (err != 0)
		return err;
	
	CopyBitmapSection(&i, &j, 240, 240,
						240*11, 240*11, 0, 0, 2048, 2048);
	
//	CopyBitmapSectionDirect(i, j, 240, 240, 0, 0, 240 * 10, 240 * 10);
	
	
	char buf[50];
	sprintf(buf,"%+03d%+04d.png", s, w);
	string d(args[2]);
	d += buf;
	
	
	WriteBitmapToPNG(&j, d.c_str(), NULL, 0, 0.0f);
	DestroyBitmap(&i);
	DestroyBitmap(&j);
	

	
	return 0;
}

static	GISTool_RegCmd_t		sImageCmds[] = {
	{ "-fill_water_image", 3, 3, FillWaterImage,"","" },
	{ 0, 0, 0, 0, 0, 0 }
};





void RegisterImageCmds(void)
{
	GISTool_RegisterCommands(sImageCmds);
}