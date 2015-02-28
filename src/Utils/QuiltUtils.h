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

#ifndef QuiltUtils_H
#define QuiltUtils_H

struct	ImageInfo;

void	quilt_images(
					struct ImageInfo& lhs,
					struct ImageInfo& rhs,
					struct ImageInfo& result);

void	make_texture(
				struct ImageInfo&	src,
				struct ImageInfo&	dst,
				int	tile_size,
				int overlap,
				int trials);
				

inline void 	offset_cuts(vector<int>& cuts, int d)
{
	for(vector<int>::iterator i = cuts.begin(); i != cuts.end(); ++i)
		*i += d;
}

inline unsigned long error_func(const unsigned long * c1, const unsigned long * c2)
{
	int r1 = (*c1 & 0xFF000000) >> 24 ;
	int g1 = (*c1 & 0x00FF0000) >> 16 ;
	int b1 = (*c1 & 0x0000FF00) >>  8 ;
	int a1 = (*c1 & 0x000000FF)		  ;
	
	int r2 = (*c2 & 0xFF000000) >> 24 ;
	int g2 = (*c2 & 0x00FF0000) >> 16 ;
	int b2 = (*c2 & 0x0000FF00) >>  8 ;
	int a2 = (*c2 & 0x000000FF)       ;
	
	return  (r1 - r2) * (r1 - r2) +
	(g1 - g2) * (g1 - g2) +
	(b1 - b2) * (b1 - b2) +
	(a1 - a2) * (a1 - a2);
}

inline unsigned long blend_func(const unsigned long * c1, const unsigned long * c2)
{
#if 0
	return 0xFF0000FF;					// Just return a solid color...makes it easy to see the cut.
#endif
	int r1 = (*c1 & 0xFF000000) >> 24;
	int g1 = (*c1 & 0x00FF0000) >> 16;
	int b1 = (*c1 & 0x0000FF00) >>  8;
	int a1 = (*c1 & 0x000000FF)		;
	
	int r2 = (*c2 & 0xFF000000) >> 24;
	int g2 = (*c2 & 0x00FF0000) >> 16;
	int b2 = (*c2 & 0x0000FF00) >>  8;
	int a2 = (*c2 & 0x000000FF)		 ;
	
	int r = ((r1 + r2) >> 1);
	int g = ((g1 + g2) >> 1);
	int b = ((b1 + b2) >> 1);
	int a = ((a1 + a2) >> 1);
	
	return ((r & 0xFF) << 24) |
	((g & 0xFF) << 16) |
	((b & 0xFF) <<  8) |
	(a & 0xFF);
}

template <typename Pixel>
void copy_rotate(
					const Pixel * s,int	dx1,int dy1,
						  Pixel * d,int	dx2,int dy2,
					int width, int height,
					int dx, int dy)
{
	#define PIXEL1(x,y)		(s+dx1*(x)+dy1*(y))
	#define PIXEL2(x,y)		(d+dx2*(x)+dy2*(y))
	
	for(int y = 0; y < height; ++y)
	for(int x = 0; x < width ; ++x)
	{
		int dest_x = (x + dx + width) % width;
		int dest_y = (y + dy + height) % height;
		
		*PIXEL2(dest_x,dest_y) = *PIXEL1(x,y);
	}	
	
	#undef PIXEL1
	#undef PIXEL2
}

template <typename Pixel>
void rotate_inplace(
					Pixel * s,int	dx1,int dy1,
					int width, int height,
					int dx, int dy)
{
	vector<Pixel>	storage;
	storage.insert(storage.end(), s, s + (width * height));
	copy_rotate(&*storage.begin(), 1, width, s, dx1, dy1, width, height, dx, dy);
}

template <typename Pixel>
void copy_cut_vertical(
					const Pixel * s1,int	dx1,int dy1,
					const Pixel * s2,int	dx2,int dy2,
					Pixel * d3,int	dx3,int dy3,
					int width, int height,
					const vector<int>& path)
{
	#define PIXEL1(x,y)		(s1+dx1*(x)+dy1*(y))
	#define PIXEL2(x,y)		(s2+dx2*(x)+dy2*(y))
	#define PIXEL3(x,y)		(d3+dx3*(x)+dy3*(y))
	
	for(int y = 0; y < height; ++y)
	for(int x = 0; x < width ; ++x)
	{
		if(x < path[y])			*PIXEL3(x,y) = *PIXEL1(x,y);
		else if (x > path[y])	*PIXEL3(x,y) = *PIXEL2(x,y);
		else					*PIXEL3(x,y) = blend_func(PIXEL1(x,y), PIXEL2(x,y));
	}	
	
	#undef PIXEL1
	#undef PIXEL2
	#undef PIXEL3
}

template <typename Pixel>
void copy_cut_horizontal(
					const Pixel * s1,int	dx1,int dy1,
					const Pixel * s2,int	dx2,int dy2,
					Pixel * d3,int	dx3,int dy3,
					int width, int height,
					const vector<int>& path)
{
	return copy_cut_vertical(
			s1,dy1,dx1,
			s2,dy2,dx2,
			d3,dy3,dx3,
			height, width, path);	
}

template <typename Pixel>
void copy_cut_edges(
					const Pixel * s1, int dx1, int dy1,
					const Pixel * s2, int dx2, int dy2,
						  Pixel * d3, int dx3, int dy3,
					int width, int height,
					const vector<int>& cut_l,
					const vector<int>& cut_b,
					const vector<int>& cut_r,
					const vector<int>& cut_t)
{
	#define PIXEL1(x,y)		(s1+dx1*(x)+dy1*(y))
	#define PIXEL2(x,y)		(s2+dx2*(x)+dy2*(y))
	#define PIXEL3(x,y)		(d3+dx3*(x)+dy3*(y))
	
	for(int y = 0; y < height; ++y)
	for(int x = 0; x < width ; ++x)
	{
		int cl = cut_l.empty() ? -1    : cut_l[y];
		int cr = cut_r.empty() ? width : cut_r[y];
		int cb = cut_b.empty() ? -1    : cut_b[x];
		int ct = cut_t.empty() ? height: cut_t[x];
		
		// If we are out of bounds on any side or the cuts have squeezed the middle to zero, use base tex.
		if(x < cl || x > cr || y < cb || y > ct || cl == cr || cb == ct)
			*PIXEL3(x,y) = *PIXEL1(x,y);
		// If we hit directly on a side, blend
		else if (x == cl || x == cr || y == cb || y == ct)
			*PIXEL3(x,y) = blend_func(PIXEL1(x,y), PIXEL2(x,y));
		// else we must be in the middle.

		else
			*PIXEL3(x,y) = *PIXEL2(x,y);
	}	
	
	#undef PIXEL1
	#undef PIXEL2
	#undef PIXEL3
}
// Calculate error for two regions, of known size.
template <typename Pixel, typename ErrorMetric>
ErrorMetric calc_total_error(
					const Pixel * p1,int dx1,int		dy1,			// Ptr to the SUBREGION of an image we are using (lower left corner) "Jump" in pixels for each image to go to the right one, and up one. (Note that this means extra storage per row must be a multiple of the pixel size.
					const Pixel * p2,int		dx2,int		dy2,
					int		width, int height)				// Total size in pixels of the region.
{
	#define PIXEL1(x,y)		(p1+dx1*(x)+dy1*(y))
	#define PIXEL2(x,y)		(p2+dx2*(x)+dy2*(y))

	ErrorMetric e = 0;

	for(int y = 0; y < height; ++y)
	for(int x = 0; x < width ; ++x)
		e += error_func(PIXEL1(x,y),PIXEL2(x,y));
	#undef PIXEL1
	#undef PIXEL2
	return e;	
}

// Calculate error.  x1,y1,x2,y2 define an arbitrary rect in the coordinate systems of p1/p2.
template <typename Pixel, typename ErrorMetric>
ErrorMetric calc_total_error_rgn(
					const Pixel * p1, int		dx1, int		dy1,		
					const Pixel * p2, int		dx2, int		dy2,
					int		x1, int		y1, int		x2, int		y2)
{
	return calc_total_error<Pixel, ErrorMetric>(
							 p1 + dx1 * x1 + dy1 * y1,	dx1, dy1,
							 p2 + dx2 * x1 + dy2 * y1,	dx2, dy2,
							 x2 - x1, y2 - y1);
}					

// Given a region we are going to drop over another one, and an amount of "border" on 4 sides (0 = no border), measure
// total area of ALL of the O, U, or I shaped border region(s).
template <typename Pixel, typename ErrorMetric>
ErrorMetric calc_overlay_error(
					const Pixel * base,			// Ptr to the SUBREGION of an image we are using (lower left corner)
					int		dx1,				// "Jump" in pixels for each image to go to the right one, and up one.
					int		dy1,				// (Note that this means extra storage per row must be a multiple of the pixel size.
					const Pixel * dub,
					int		dx2,
					int		dy2,
					int		width,				// Total size in pixels of the region.
					int		height,
					int		left,				// This is the number of pixels on each side of the overlay region that are going to overlap.
					int		bottom,
					int		right,
					int		top)
{
	ErrorMetric e = 0;

	right = width - right;
	top = height - top;

	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		0,		0,	left, bottom);
	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		left,	0,	right, bottom);
	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		right,	0,	width, bottom);

	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		0,		bottom,	left, top);
//	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		left,	bottom,	right, top);
	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		right,	bottom,	width, top);

	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		0,		top,	left, height);
	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		left,	top,	right, height);
	e += calc_total_error_rgn<Pixel, ErrorMetric>(base,dx1,dy1,dub,dx2,dy2,		right,	top,	width, height);

	return e;						
}
					

template <typename Pixel, typename ErrorMetric>
void calc_quilt_vertical(
					const Pixel * p1,			// Ptr to the SUBREGION of an image we are using (lower left corner)
					int		dx1,				// "Jump" in pixels for each image to go to the right one, and up one.
					int		dy1,				// (Note that this means extra storage per row must be a multiple of the pixel size.
					const Pixel * p2,
					int		dx2,
					int		dy2,
					int		width,				// Total size in pixels of the region.
					int		height,
					vector<int>& out_path)		// "Best" path.  This is in whole pixels, e.g. 0 means the left-most pixel is the 'transition' pixel to blend.
{
	// Here's the key idea: we want to find a path of steps to the left or right that minimizes the "conflict" at our border.
	// Rather than evaluate EVERY possible path, we do this: the error at each pixel will include its own error PLUS the error it took
	// to GET there from below.  This is sort of like an array version of Djikstra.  So by the time we get to the top, each of those 
	// pixels contains the FULL cost, and the cheapest top pixel is the best case IN TOTAL.
	
	out_path.resize(height);
	vector<ErrorMetric>	err_matrix(width*height);
	
	#define PIXEL1(x,y)		(p1+dx1*(x)+dy1*(y))
	#define PIXEL2(x,y)		(p2+dx2*(x)+dy2*(y))

	#define ERR(x,y)		(err_matrix[(x) + (y) * width])

	
	int x, y;
	for(x = 0; x < width; ++x)
		ERR(x,0) = error_func(PIXEL1(x,0),PIXEL2(x,0));		// Bottom row (start), error is JUST the cost of one pixel.
	
	for(y = 1; y < height; ++y)
	{

		// Note that when we accum our previous row below us to ourselves, we take the minimum of the 2 or 3 pixels that "feed" us...
		// because we would always take the most efficient path to ourselves.
	
		ERR(0,y) = min(ERR(0,y-1),ERR(1,y-1)) + error_func(PIXEL1(0,y),PIXEL2(0,y));
		ERR(width-1,y) = min(ERR(width-1,y-1),ERR(width-2,y-1)) + error_func(PIXEL1(width-1,y),PIXEL2(width-1,y));

		for(x = 1; x < (width-1); ++x)
			ERR(x,y) = min(min(ERR(x-1,y-1),ERR(x+1,y-1)),ERR(x,y-1)) + error_func(PIXEL1(x,y),PIXEL2(x,y));		
	}

	int c = 0, r = height-1;									// Find the least error at the top.
	for(x = 1; x < width; ++x)
	if(ERR(x,r) < ERR(c,r))
		c = x;
	
	out_path[r] = c;
	while(r > 0)
	{
		--r;													// Step down a row.
			 if(c >  0        && ERR(c-1,r) < ERR(c,r))	--c;	// Figure out if moving 1 to the left or right gets us less error.
		else if(c < (width-1) && ERR(c+1,r) < ERR(c,r)) ++c;
		out_path[r] = c;	
	}
	
	#undef PIXEL1
	#undef PIXEL22
	#undef ERR
}


template <typename Pixel, typename ErrorMetric>
void calc_quilt_horizontal(
					const Pixel * p1,			// Ptr to the SUBREGION of an image we are using (lower left corner)
					int		dx1,				// "Jump" in pixels for each image to go to the right one, and up one.
					int		dy1,				// (Note that this means extra storage per row must be a multiple of the pixel size.
					const Pixel * p2,
					int		dx2,
					int		dy2,
					int		width,				// Total size in pixels of the region.
					int		height,
					vector<int>& out_path)		// "Best" path.  This is in whole pixels, e.g. 0 means the left-most pixel is the 'transition' pixel to blend.
{
	calc_quilt_vertical<Pixel,ErrorMetric>(p1,dy1,dx1,p2,dy2,dx2,height,width, out_path);
}


template <typename Pixel, typename ErrorMetric>
void calc_four_cuts(
					const Pixel * s1, int dx1, int dy1,
					const Pixel * s2, int dx2, int dy2,
					int width, int height,
					int l, int b, int r, int t,
					vector<int>& cut_l,
					vector<int>& cut_b,
					vector<int>& cut_r,
					vector<int>& cut_t)
{
	cut_l.clear();
	cut_b.clear();
	cut_r.clear();
	cut_t.clear();
	
	if(l > 0)
	{
		calc_quilt_vertical<Pixel, ErrorMetric>(
					s1,dx1,dy1,
					s2,dx2,dy2,
					l, height,
					cut_l);
	}
	if(r > 0)
	{
		calc_quilt_vertical<Pixel, ErrorMetric>(
					s1 + dx1 * (width-r), dx1, dy1,
					s2 + dx2 * (width-r), dx2, dy2,
					r, height,
					cut_r);
		offset_cuts(cut_r, width-r);
	}

	
	if(b > 0)
	{
		calc_quilt_horizontal<Pixel, ErrorMetric>(
					s1,dx1,dy1,
					s2,dx2,dy2,
					width, b,
					cut_b);
	}
	if(t > 0)
	{
		calc_quilt_horizontal<Pixel, ErrorMetric>(
					s1 + dy1 * (height-t), dx1, dy1,
					s2 + dy2 * (height-t), dx2, dy2,
					width, t,
					cut_t);
		offset_cuts(cut_t, height-t);
	}
	
}



#endif /* QuiltUtils_H */
