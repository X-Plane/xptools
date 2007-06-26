#ifndef BITMAP_MATCH_H
#define BITMAP_MATCH_H

#include <ac_plugin.h>

int	bitmap_match(		
		ACImage *	sub,
		ACImage *	main,
		int *		h_offset,
		int *		v_offset);
		
int apply_lighting(
		ACImage *	day,
		ACImage *	night);

int make_transparent(ACImage * im);

void	tex_reload(int tex_id);

void bitmap_subcopy(
		ACImage * src,
		ACImage * dst,
		int l,
		int b,
		int r,
		int t);

#endif
