/*
 * Copyright (c) 2010, Laminar Research.
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

#include <assert.h>
#include "QuiltUtils.h"
#include "BitmapUtils.h"

#if IBM
#define random rand
#endif

void test(void)
{
		unsigned long * a, * b;
	vector<int> path;
	calc_quilt_vertical<unsigned long, unsigned long>(a, 1, 40, b, 1, 50, 40, 40, path);

}

void	quilt_images(
					struct ImageInfo& lhs,
					struct ImageInfo& rhs,
					struct ImageInfo& res)
{
	// Make sure we have 3 equally sized 4-channel images.
	assert(lhs.channels == 4);
	assert(rhs.channels == 4);
	assert(res.channels == 4);
	assert(lhs.pad % 4 == 0);
	assert(rhs.pad % 4 == 0);
	assert(res.pad % 4 == 0);
	assert(lhs.width==rhs.width);
	assert(lhs.height==rhs.height);
	assert(lhs.width==res.width);
	assert(lhs.height==res.height);

	const unsigned long * s1 = (const unsigned long *) lhs.data;
	const unsigned long * s2 = (const unsigned long *) rhs.data;
	unsigned long * d  = (unsigned long *) res.data;

	vector<int> l, b, r, t;

	calc_four_cuts<unsigned long, unsigned long>(
						s1, 1, lhs.width + lhs.pad / 4,
						s2, 1, rhs.width + rhs.pad / 4,
						lhs.width, lhs.height,

					100, 100, 100, 100,

//					lhs.width / 3, lhs.height / 3,
//					lhs.width / 3, lhs.height / 3,
					l,b,r,t);


	copy_cut_edges<unsigned long>(
									s1, 1, lhs.width + lhs.pad / 4,
									s2, 1, rhs.width + rhs.pad / 4,
									d, 1, res.width + res.pad / 4,
									lhs.width, lhs.height,
							l, b, r, t);

}

const unsigned long *	grab_patch(const unsigned long * base, int dx, int dy, int w, int h, int tile_size)
{
	int xo = random() % (w - tile_size);
	int yo = random() % (h - tile_size);
	return base + xo * dx + yo * dy;
}

void	splat_for_spot(
				const unsigned long *	image_src, int sdx, int sdy, int src_w, int src_h,
					  unsigned long *	image_dst, int ddx, int ddy,			// dest pre-offset location
					  int splat_s,												// size o splat
					  int l, int b, int r, int t,								// amount of overlap
					  int trials)
{
	const unsigned long * best = grab_patch(image_src,sdx,sdy, src_w, src_h, splat_s);
	unsigned long best_e = calc_overlay_error<unsigned long, unsigned long>(
									image_dst, ddx, ddy,
									best, sdx, sdy,
									splat_s, splat_s,
									l, b, r, t);
	while(trials > 0 && best_e > 0)
	{
		const unsigned long * trial = grab_patch(image_src,sdx,sdy, src_w, src_h, splat_s);
		unsigned long trial_e = calc_overlay_error<unsigned long, unsigned long>(
									image_dst, ddx, ddy,
									best, sdx, sdy,
									splat_s, splat_s,
									l, b, r, t);

		if(trial_e < best_e)
		{
			best_e = trial_e;
			best = trial;
		}
		--trials;
	}

	vector<int> cl, cb, cr, ct;

	calc_four_cuts<unsigned long, unsigned long>(
						image_dst, ddx,ddy,
						best, sdx, sdy,
						splat_s, splat_s,
						l, b, r, t,
						cl, cb, cr, ct);

	copy_cut_edges<unsigned long>(
						image_dst, ddx,ddy,
						best, sdx, sdy,
						image_dst, ddx,ddy,
						splat_s, splat_s,
						cl, cb, cr, ct);
}


void	make_texture(
				struct ImageInfo&	src,
				struct ImageInfo&	dst,
				int	tile_size,
				int overlap,
				int trials)
{
	assert(src.channels==4);
	assert(src.pad % 4 == 0);
	assert(dst.channels==4);
	assert(dst.pad % 4 == 0);

	int sdy = src.width + src.pad / 4;
	int ddy = dst.width + src.pad / 4;

	const unsigned long * srcp = (const unsigned long *) src.data;
		  unsigned long * dstp = (      unsigned long *) dst.data;

	int scoot = tile_size - overlap;

	assert(dst.width % scoot == 0);
	assert(dst.height % scoot == 0);

	int tiles_x = dst.width / scoot;
	int tiles_y = dst.height / scoot;

	splat_for_spot(
						srcp, 1, sdy, src.width, src.height,
						dstp, 1, ddy,
						tile_size,
						0, 0, 0, 0, trials);
	int n;
	for(n = 1; n < tiles_x / 2; ++n)
	{
		splat_for_spot(
						srcp, 1, sdy, src.width, src.height,
						dstp + n * scoot, 1, ddy,
						tile_size,
						overlap, 0, 0, 0, trials);
	}
	for(n = 1; n < tiles_y / 2; ++n)
	{
		splat_for_spot(
						srcp, 1, sdy, src.width, src.height,
						dstp + n * scoot * ddy, 1, ddy,
						tile_size,
						0, overlap, 0, 0, trials);
	}

	rotate_inplace(dstp, 1, ddy, dst.width, dst.height, -scoot * 2, -scoot * 2);

	for(n = tiles_x / 2; n < tiles_x; ++n)
	{
		splat_for_spot(
						srcp, 1, sdy, src.width, src.height,
						dstp + (n-2) * scoot + ddy * (dst.height - scoot * 2), 1, ddy,
						tile_size,
						overlap, 0, (n == tiles_x-1) ? overlap : 0, 0, trials);
	}

	for(n = tiles_y / 2; n < tiles_y; ++n)
	{
		splat_for_spot(
						srcp, 1, sdy, src.width, src.height,
						dstp + (n-2) * scoot * ddy + dst.width - scoot * 2, 1, ddy,
						tile_size,
						0, overlap, 0, (n == tiles_y-1) ? overlap : 0, trials);
	}

	int rdx = scoot;// / 2;
	int rdy = scoot;// / 2;

	rotate_inplace(dstp, 1, ddy, dst.width, dst.height, scoot * 2 - rdx, scoot * 2 - rdy);

	for(int y = 1; y < tiles_y; ++y)
	for(int x = 1; x < tiles_x; ++x)
	{
		splat_for_spot(
					srcp, 1, sdy, src.width, src.height,
					dstp + x * scoot - rdx + (y * scoot - rdy) * ddy, 1, ddy,
					tile_size,
					overlap,
					overlap,
					(x == tiles_x-1) ? overlap : 0,
					(y == tiles_y-1) ? overlap : 0,
					trials);
	}
}
