// To compile:
// gcc -o gen_tiles gen_tiles.c
//
// To use: ./gen_tiles tex_x tex_y tile_x tile_y [<frequency bias>]
// This generates a tex_x x tex_y swizzling texture to swizzle an albedo of tile_x x tile_y tiles.
// Generally you want tex_x and tex_y much larger than tile_x and tile_y.
// Generally no tile is repeated next to each other (including diagonals.)
// If you include an optional bias (< 1.0) the bottom tiles are used more; repeats in the FIRST tile
// are not fixed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Define 4 to allow diagonal repeats in the 2-d case
#define NUM_NEIGHBORS 8

// Set to 1 to get binary raw for photoshop, 0 for debu text
#define WANT_BINARY 1

int			xs, ys, ts;
float		pow_curve;
char *		mem;

char chars[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int pick_tile(int num_choices)
{
	float r;
	if(pow_curve == 1.0)
		return rand() % num_choices;
	r = (float) rand() / (float) RAND_MAX;
	r = pow(r,pow_curve);
	return num_choices * r;
}

char	neighbor(int i, int d)
{
	int xo[8] = { 0,0,-1,1,-1,-1,1,1 };
	int yo[8] = { -1,1,0,0,-1,1,-1,1 };
	int x = i % xs;
	int y = i / xs;
	x = (x + xo[d] + xs) % xs;
	y = (y + yo[d] + ys) % ys;
	return mem[x + y * xs];
	
}

int main(int argc, const char * argv[])
{
	xs = atoi(argv[1]);
	ys = atoi(argv[2]);
	ts = xs*ys;
	int tx = atoi(argv[3]);
	int ty = atoi(argv[4]);
	pow_curve = (argc == 6) ? atof(argv[5]) : 1.0;
	int tc = tx * ty;
	int x, y, i, d;

	mem = (char *) malloc(ts);
	memset(mem,0,ts);

	fprintf(stderr,"will generate %d x %d with %d tiles.\n", xs,ys,tc);
	if(pow_curve != 1.0)
		fprintf(stderr,"Usage bias: %f\n", pow_curve);

	srand((unsigned)time(NULL));

	for(i = 0; i < ts; ++i)
		mem[i] = pick_tile(tc);
	
	
retry:	
	{
		int offset = rand() % ts;
		for(i = 0; i < ts; ++i)
		{
			int ii = (i + offset) % ts;
			int nstart = (ty == 1) ? 2 : 0;
			int nend = (tx == 1) ? 2 : NUM_NEIGHBORS;
			for(d = nstart; d < nend; ++d)
			if(mem[ii] == neighbor(ii,d))
			if(pow_curve == 1.0f || mem[ii] != tc-1)
			{
				mem[ii] = pick_tile(tc);
				goto retry;
			}
		}
	}
	
	i = 0;
	for(y = 0; y < ys; ++y)
	{
		for(x = 0; x < xs; ++x)
		{
#if WANT_BINARY		
			int tcx = mem[i] % tx;
			int tcy = mem[i] / tx;
			int r_level = 255 * tcx / tx;
			int g_level = 255 * tcy / ty;
			fputc(r_level,stdout);
			fputc(g_level,stdout);
			fputc(0,stdout);
#endif
			fprintf(stderr,"%c", chars[mem[i]]);
			++i;
		}
		fprintf(stderr,"\n");
	}
	{
		int * histo = (int *) malloc(sizeof(int) * tc);
		memset(histo,0,tc * sizeof(int));
		for(i = 0; i < ts; ++i)
			histo[mem[i]]++;
		for(i = 0; i < tc; ++i)
			fprintf(stderr,"%d: %f (%f)\n", i,(float) histo[i] / (float) (ts/tc), (float) histo[i] / (float) (ts));
	}
	
	return 0;
	
	
}

