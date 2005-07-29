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
#include "BitmapUtils.h"

int	start_items[4][4] = { { 8, 11, 9, 10 }, { 4, 7, 5, 6 }, { 0, 3, 1, 2 }, { 12, 13, -1, -1 } };

float	st_start_lookup[4][14][4][2];

int	final_items[16] = {
	12, 11,  8,  0,  6,  3,  1,  4,
	13, 10,  9,  2,  7,  3,  1,  5 };
	
float	st_lookup_final[14][4][2];

void	CopyEasy(ImageInfo * src, ImageInfo * dst, 
				int sx1, int sy1, int sx2, int sy2,
				int dx1, int dy1, int dx2, int dy2)
{
	int sx, sy, dx, dy;
	for (sy = sy1, dy = dy1; sy < sy2; ++sy, ++dy)
	for (sx = sx1, dx = dx1; sx < sx2; ++sx, ++dx)
	{
		unsigned char * sp = src->data + (sx * src->channels + sy * (src->channels * src->width + src->pad));
		unsigned char * dp = dst->data + (dx * dst->channels + dy * (dst->channels * dst->width + dst->pad));
		
		for (int i = 0; i < src->channels; ++i)
			dp[i] = sp[i];
	}
}

void	HackFixTextures(const char * fnames[4], const char * fname)
{
	for (int y = 0; y < 2; ++y)
	for (int x = 0; x < 8; ++x)
	{
		int cell_id = y * 8 + x;
		int tex_num = final_items[cell_id];
		
		float	s1 = (float) x / 8.0;
		float 	s2 = (float) (x+1) / 8.0;
		float	t1 = (float) y / 2.0;
		float 	t2 = (float) (y+1) / 2.0;
		
		// Lower Left
		st_lookup_final[tex_num][0][0] = s1;
		st_lookup_final[tex_num][0][1] = t1;

		// Upper Left
		st_lookup_final[tex_num][1][0] = s1;
		st_lookup_final[tex_num][1][1] = t2;
		
		// Upper Right
		st_lookup_final[tex_num][2][0] = s2;
		st_lookup_final[tex_num][2][1] = t2;
		
		// Lower Right
		st_lookup_final[tex_num][3][0] = s2;
		st_lookup_final[tex_num][3][1] = t1;
	}
	for (int n = 0; n < 14; ++n)
	{
		printf("{ ");
		for (int m = 0; m < 4; ++m)
		{
			printf("{ %f, %f }, ", st_lookup_final[n][m][0],st_lookup_final[n][4][1]);
		}
		printf("},\n");
	}
	
	for (int i = 0; i < 4; ++i)
	{
		for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
		{
			int cell_id = y * 2 + x;
			int tex_num = start_items[i][cell_id];

			if (tex_num == -1) continue;
			float	s1 = (float) x / 2.0;
			float 	s2 = (float) (x+1) / 2.0;
			float	t1 = (float) y / 2.0;
			float 	t2 = (float) (y+1) / 2.0;
			if (i == 3) t1 = 0.0, t2 = 1.0;

			// Lower Left
			st_start_lookup[i][tex_num][0][0] = s1;
			st_start_lookup[i][tex_num][0][1] = t1;

			// Upper Left
			st_start_lookup[i][tex_num][1][0] = s1;
			st_start_lookup[i][tex_num][1][1] = t2;
			
			// Upper Right
			st_start_lookup[i][tex_num][2][0] = s2;
			st_start_lookup[i][tex_num][2][1] = t2;
			
			// Lower Right
			st_start_lookup[i][tex_num][3][0] = s2;
			st_start_lookup[i][tex_num][3][1] = t1;
		}
	}
	
	ImageInfo	src[4];
	ImageInfo	final;
	for (int i = 0; i < 4; ++i)
	{
		CreateBitmapFromFile(fnames[i], &src[i]);		
	}
	CreateNewBitmap(1024, 256, 3, &final);
	
	for (int tex_id = 0; tex_id < 14; ++tex_id)
	{
		for (int srcn = 0; srcn < 4; ++srcn)
		{			
			if (st_start_lookup[srcn][tex_id][0][0] != st_start_lookup[srcn][tex_id][2][0])
			{
				CopyEasy(&src[srcn], &final,
					256.0 * st_start_lookup[srcn][tex_id][0][0],
					((srcn == 3) ? 128.0 : 256.0) * st_start_lookup[srcn][tex_id][0][1],
					256.0 * st_start_lookup[srcn][tex_id][2][0],
					((srcn == 3) ? 128.0 : 256.0) * st_start_lookup[srcn][tex_id][2][1],

					1024.0 * st_lookup_final	[tex_id][0][0],
					 256.0 * st_lookup_final	[tex_id][0][1],
					1024.0 * st_lookup_final	[tex_id][2][0],
					 256.0 * st_lookup_final	[tex_id][2][1]);
			}
		}
	}
	CopyEasy(&final, &final, 
						640, 128, 896, 256,
						640, 0, 896, 128);
	WriteBitmapToFile(&final, fname);
	DestroyBitmap(&final);
	for (int n = 0; n < 4; ++n)
		DestroyBitmap(&src[n]);		
}
