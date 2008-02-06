/* 
 * Copyright (c) 2007, Laminar Research.
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

#include "bitmap_match.h"
#include "TclStubs.h"
#include <ac_plugin.h>
#include <stdio.h>
#include <string.h>

static unsigned char * get_image_data(ACImage * im);
static unsigned char * get_image_data(ACImage * im)
{
	int addr;
	if (!ac_entity_get_int_value(im, "data", &addr)) return NULL;
	return (unsigned char *) addr;
}


/*
 * bitmap_match
 *
 * Given two bitmaps, this routine returns true if sub is fully contained
 * within main, and sets h_offset and v_offset to the offsets within those
 * bitmaps.  Sub and main must be of the same bit depth!
 *
 * The algorithm works by brute force...for each possible position of sub,
 * it attempts to check every pixel of sub against main.  Surprisingly, this
 * algorithm executes very rapidly for 512x256 bitmaps on my P-IV, so I haven't
 * bothered to optimize it.  (Authors will only be doing this every once in a
 * while anyway.)  The early exit from a mismatch probably helps speed a lot.
 *
 * One possible optimization: use an adam7-like order of traversals of the pixels
 * in checking for a match.  This would cause us to jump all around the bitmap, 
 * finding mismatches faster even if the upper left local area corner is just 
 * transparent or a solid color in both.
 *
 */ 
int	bitmap_match(
		ACImage *	sub,
		ACImage *	main,
		int *		h_offset,
		int *		v_offset)
{
	int main_width, main_height, main_depth;
	int sub_width, sub_height, sub_depth;
	ac_image_get_dim(main, &main_width, &main_height, &main_depth);
	ac_image_get_dim(sub, &sub_width, &sub_height, &sub_depth);
	
	char * subname, * mainname;
	ac_entity_get_string_value(sub, "name", &subname);
	ac_entity_get_string_value(main, "name", &mainname);
	
	if (sub_depth != main_depth) 
	{
		if (sub_depth == 3 && main_depth == 4)
			message_dialog("Could not match bitmaps because bitmaps '%s' has alpha and '%s' does not.", mainname, subname);
		else if (sub_depth == 4 && main_depth == 3)
			message_dialog("Could not match bitmaps because bitmaps '%s' has alpha and '%s' does not.", subname, mainname);
		else
			message_dialog("Could not match bitmaps because bitmaps '%s' and '%s' have different color depths.", subname, mainname);
		return 0;
	}
	if (sub_width > main_width || sub_height > main_height) 
	{
		message_dialog("Could not match bitmaps because new bitmap '%s' is smaller than old bitmap '%s'.", mainname, subname);
		return 0;
	}
	
	unsigned char * maind = get_image_data(main);
	unsigned char * subd = get_image_data(sub);
	
	for (int x_off = 0; x_off <= (main_width - sub_width); ++x_off)
	for (int y_off = 0; y_off <= (main_height - sub_height); ++y_off)
	{
		int match = 1;
		
		for (int x_pixel = 0; x_pixel < sub_width; ++x_pixel)
		for (int y_pixel = 0; y_pixel < sub_height; ++y_pixel)
		{
			unsigned char * mainp = 
						maind + 
						(x_off + x_pixel) * main_depth +
						(y_off + y_pixel) * main_depth * main_width;

			unsigned char * subp = 
						subd + 
						(x_pixel) * sub_depth +
						(y_pixel) * sub_depth * sub_width;
						
			for (int c = 0; c < sub_depth; ++c)
			if (subp[c] != mainp[c])
			{
				match = 0;
				goto nomatch;
			}					
		}
		*h_offset = x_off;
		*v_offset = y_off;
		return 1;

nomatch:
		match = 0;
		
	}	
	return 0;
}

/*
 * apply_lighting
 *
 * Given a day and night overlay bitmap, this routine adds the night overlay
 * to the day bitmap (they must be the same size), and reduces the day's
 * brightness too, to simulate x-plane night lighting.
 *
 */
int apply_lighting(
		ACImage *	day,
		ACImage *	night)
{
	int day_width, day_height, day_depth;
	int night_width, night_height, night_depth;
	ac_image_get_dim(day, &day_width, &day_height, &day_depth);
	ac_image_get_dim(night, &night_width, &night_height, &night_depth);

	if (day_width != night_width) return 0;
	if (day_height != night_height) return 0;
	if (day_depth != 3 && day_depth != 4) return 0;
	if (night_depth != 3 && night_depth != 4) return 0;
	
	void * new_mem = myalloc(day_width * day_height * day_depth);
	
	unsigned char * dayd = get_image_data(day);
	unsigned char * nightd = get_image_data(night);
	unsigned char * destd = (unsigned char *) new_mem;
	
	int channels = (night_depth > day_depth) ? day_depth : night_depth;

	int x, y, c;
	
	for (y = 0; y < day_height; ++y)
	for (x = 0; x < day_width; ++x)
	{
		unsigned char * dayp = dayd + 
							x * day_depth +
							y * day_depth * day_width;
		unsigned char * nightp = nightd + 
							x * night_depth +
							y * night_depth * night_width;
		unsigned char * destp = destd +
							x * day_depth +
							y * day_depth * day_width;
		
		for (c = 0; c < channels; ++c)
		{
			unsigned long v = (dayp[c] >> 6) + nightp[c];
			if (v > 255) v = 255;
			destp[c] = v;
		}
		if (channels == 3 && day_depth == 4)
		{
			destp[c] = dayp[c];
		}
	}
	
	ac_image_set_data(day, destd);	
	texture_build_for_all_windows(day);
	redraw_all();
	
	return 1;
}		

/*
 * Given a non-alpha-channel 24 bit image, this routine converts it to a 32-bit
 * ARGB image and makes pure magenta (FF00FF) pixels transparent, allowing users
 * to preview transparency for BMP textured objects.
 *
 * WARNING: this routine does not properly implement color smearing near magenta
 * pixels, so there may be artifacts around transparent areas.  Frankly this is
 * acceptable, it's just a preview, and authors should be working with PNG anyway.
 *
 */
int make_transparent(ACImage * im)
{
	int im_width, im_height, im_depth;
	ac_image_get_dim(im, &im_width, &im_height, &im_depth);
	
	if (im_depth != 3) 
	{
		message_dialog("Bitmap already has an alpha channel.");
		printf("Image is not depth 3.\n");
		return 0;
	}
	
	void * new_mem = myalloc(im_width * im_height * 4);
	unsigned char * srcd = get_image_data(im);
	unsigned char * dstd = (unsigned char *) new_mem;
	
	int transparent = 0;
	
	for (int y = 0; y < im_height; ++y)
	for (int x = 0; x < im_width; ++ x)
	{
		unsigned char * srcp = srcd + x * 3 + y * im_width * 3;
		unsigned char * dstp = dstd + x * 4 + y * im_width * 4;
		
		if (srcp[0] == 255 && srcp[1] == 0 && srcp[2] == 255)
		{
			++transparent;
			dstp[0] = dstp[1] = dstp[2] = dstp[3] = 0;
		} else {
			dstp[0] = srcp[0];
			dstp[1] = srcp[1];
			dstp[2] = srcp[2];
			dstp[3] = 255;
		}		
	}
	
	ac_image_set_alpha_mask(im, ALPHA_TRANSP);
	ac_image_set_dim(im, im_width, im_height, 4);
	ac_image_set_data(im, dstd);	
	texture_build_for_all_windows(im);
	redraw_all();
	
	if (transparent == 0)
		message_dialog("No magenta pixels were found.");
	printf("Rendered %d pixels transparent.\n", transparent);
	return 1;
}

void	tex_reload(int tex_id)
{
	char * fname = texture_id_to_name(tex_id);
//	int im_width, im_height, im_depth;
	
	texture_build_for_all_windows(texture_id_to_image(add_new_texture_reload(fname,fname)));
	redraw_all();
	return;
/*	
	ACImage * old_image = texture_id_to_image(tex_id);
	ACImage * new_image = new_acimage(fname);
	if (new_image == NULL)
	{
		message_dialog("Error: could not load %s.\n", fname);
		return;
	}
	
	
	ac_image_get_dim(new_image, &im_width, &im_height, &im_depth);	
	ac_image_set_dim(old_image, im_width, im_height, im_depth);
	
	void * new_mem = myalloc(im_width * im_height * im_depth);
	memcpy(new_mem, get_image_data(new_image), im_width * im_height * im_depth);

	ac_image_set_data(old_image, (unsigned char *) new_mem);

	free_acimage(new_image);
	texture_build_for_all_windows(old_image);
	redraw_all();
*/
}

void bitmap_subcopy(
		ACImage * src,
		ACImage * dst,
		int l,
		int b,
		int r,
		int t)
{
	int im_width, im_height, im_depth;
	ac_image_get_dim(src, &im_width, &im_height, &im_depth);
	
	void * new_mem = myalloc((r-l) * (t-b) * im_depth);

	unsigned char * srcd = get_image_data(src);
	unsigned char * dstd = (unsigned char *) new_mem;
	
	for (int y = b; y < t; ++y)
	for (int x = l; x < r; ++ x)
	{
		unsigned char * srcp = srcd +  x    * im_depth +  y    * im_width * im_depth;
		unsigned char * dstp = dstd + (x-l) * im_depth + (y-b) * (r-l)    * im_depth;

		int c = im_depth;
		while(c--)
			*dstp++ = *srcp++;		
	}
	
	ac_image_set_dim(dst, r-l, t-b, im_depth);
	ac_image_set_data(dst, dstd);	
	texture_build_for_all_windows(dst);
	redraw_all();

}
