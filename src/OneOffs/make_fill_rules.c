// gcc make_fill_rules.c -o make_fill_rules

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#				zoning				road	variant	height	sidelen	err		major	minor	angle		agb		agb		fac		fac		fac			agb					fac					ags				fil
//#													min	max	min	max			min	max	min	max	min	max		min		slop	width	depth	extra
// FILL_RULE	ind_high_solid		2		-1		0	0	0	0	0		0	0	0	0	0	0		56		2		15		30		7.5			ind_high.agb		ind_high.fac		NO_VALUE		parking.fac


float block_widths[8]	=	{	7.5,	15,		22.5, 	30,		45,		60,		75,		90 };		
float arch_heights[8] =		{	24,		24,		24,		32,		999,	999,	999,	999 };
float block_heights[8] =	{	10,		16,		24,		32,		40,		80,		120,	999 };
float block_chance_30[64]=	{ 	10,		21,		30,		20,		10,		5,		3,		1,	
								12,		30,		30,		20,		5,		2,		1,		0,
								10,		25,		30,		25,		5,		3,		2,		0,
								1,		20,		25,		28,		10,		7,		7,		2,
								0,		10,		25,		30,		15,		10,		5,		5,
								0,		0,		15,		35,		25,		20,		5,		0,
								0,		0,		0,		35,		40,		20,		5,		0,
								0,		0,		0,		35,		40,		20,		5,		0 };
float block_width_60[8] =	{ 	7.5,	15,		22.5, 	30,		45,		60,		75,		90 };		

float block_chance_60[64]=	{	0,		21,		30,		30,		10,		5,		3,		1,
								0,		35,		35,		25,		5,		0,		0,		0,
								10,		25,		30,		25,		5,		5,		0,		0,
								1,		20,		25,		35,		15,		4,		0,		0,
								0,		15,		30,		35,		15,		5,		0,		0,
								0,		5,		15,		35,		40,		5,		0,		0,
								0,		0,		10,		35,		35,		15,		5,		0,
								0,		0,		0,		0,		25,		50,		25,		0 };


							//	0-10	10-16	16-24	24-32	32-40	40-80	80-120	120+
float block_height_table[64]={	100,	0,		0,		0,		0,		0,		0,		0,	// 0-10
								50,		50,		0,		0,		0,		0,		0,		0,	// 10-16
								20,		30,		50,		0,		0,		0,		0,		0,	// 16-24
								5,		25,		35,		35,		0,		0,		0,		0,	// 24-32
								5,		10,		20,		35,		30,		0,		0,		0,	// 32-40
								5,		10,		15,		15,		20,		35,		0,		0,	// 40-80
								0,		10,		10,		10,		10,		20,		40,		0,	// 80-120
								0,		0,		15,		15,		10,		10,		10,		40 };//120+
								

float random_percent(void)
{
	return random() % 100;
}

int	pick_histo(float * table)
{
//	int i;
//	float s = 0;
//	for(i = 0; i < 8; ++i)
//		s += table[i];
//	if(s != 100.0)
//	{
//		for(i = 0; i < 8; ++i)
//			printf("%f\n",table[i]);
//		abort();
//	}
	float percent = random_percent();
	int idx = 0;
	while(percent > table[idx])
	{
		percent -= table[idx];
		++idx;
	}
	if(idx > 7) return 7;
//	if(idx >= 8) 
//	{	
//		for(i = 0; i < 8; ++i)
//			printf("%f\n",table[i]);
//		abort();
//	}
	return idx;
}

float bldg_width_for_height_depth(int height_idx, int depth)
{	
	int pick;
	if(depth >=60)  pick = pick_histo(block_chance_30 + 8 * height_idx);
	else			pick = pick_histo(block_chance_60 + 8 * height_idx);
	return block_widths[pick];
}

int block_height_for_zoning(int zone_height_idx)
{
	return pick_histo(block_height_table + zone_height_idx * 8);
}


int height_max[] = {	  999, 120, 80,  40,		32,		24,		16,		10, 0, 0		};
int	road_codes[] = { 2, 1, 0 };
const char * road_suffix[] = { "_f", "_h", NULL };

int is_lib = 0;

struct depth_info_t {
	int		block_min;
	int		block_max;
	int		sub_div;
	int		agb;
	int		fac;
	float	min_div;
};

struct depth_info_t depth_residential [] =  {
	30,	45,	1,	30,	60,	10.0,
	45,	60,	2,	30,	30,	7.5,
	60,	120,2,	60,	60,	15.0,
	0,	0,	0,	0,	0,	0.0
};

struct depth_info_t depth_industrial [] = { 
	30, 45,		1,	30,	60,	10.0,
	45, 60,		2,	30,	30,	7.5,
	60, 90,		2,	60,	60,	15.0,
	90, 180,	2,	90,	90,	22.5,
	0,	0,		0,	0,	0,	0.0
};

int facade_schedule[] = { 25, 30, 35, 40, 45, 50, 80, 9999, 0 };

void output_one_rule(int h, int r, int v, const struct depth_info_t * ds, const char * zoning)
{
	int w, fac_height, s;
	int d1 = ds->block_min;
	int d2 = ds->block_max;
	int splits = ds->sub_div;
	int df1 = ds->fac / 2;
	int df2 = ds->fac;

	
	if(!is_lib)
	{
		printf("FILL_RULE\t%s\t",zoning);																		// Fill rule and zoning
		printf("%d\t%d\t%d\t%d\t0\t0\t0\t\t",road_codes[r],v,height_max[h+1],		height_max[h  ]);			// road code, variant code, height codes, sidelen, block err, 
		printf("%d\t9999\t%d\t%d\t0\t0\t\t",ds->agb,d1,d2);															// range for major, minor, angle
		printf("%d\t2\t%d\t\%d\t%.1f\t\t",ds->agb,30,splits,ds->min_div);											// AGB params: width, slop,fac min unused, fac_split, fac_extra
	}
	
	if(is_lib)
	{
		if(height_max[h] == height_max[h+1])
		{
			printf("EXPORT lib/g10/autogen/%s%s_%d_v%d.agb\t\t\tfoo.agb\n",zoning, road_suffix[r], ds->agb, v);
		} else {
			printf("EXPORT lib/g10/autogen/%s%s_%dx%d_v%d.agbt\t\tfoo.agb\n",zoning, road_suffix[r], ds->agb, height_max[h], v);
		}
	}
	else 
	{
		if(height_max[h] == height_max[h+1])
		{
			printf("%s%s_%d_v%d.agb\t",zoning, road_suffix[r], ds->agb, v);
//			printf("%s%s_%d_v%d.fac\t",zoning, road_suffix[r], ds->fac, v);	
			printf("NO_VALUE\t");				
			printf("NO_VALUE	%s_parking.fac\n",zoning);				
		} else {
			printf("%s%s_%dx%d_v%d.agb\t",zoning, road_suffix[r], ds->agb, height_max[h], v);
//			printf("%s%s_%dx%d_v%d.fac\t",zoning, road_suffix[r], ds->fac, height_max[h], v);
			printf("NO_VALUE\t");
			printf("NO_VALUE	%s_parking.fac\n",zoning);
		}
	}

	fac_height = 7 - h;
	if(height_max[h] == height_max[h+1]) fac_height = 0;

	if(r == 1 && splits > 1)
	{
		int bh_bottom = (height_max[h] == height_max[h+1] || fac_height == 0) ? 0 : block_heights[fac_height-1];
		int bh_top = (height_max[h] == height_max[h+1]						) ? 0 : block_heights[fac_height  ];
		
		int tiles, tries;
		
		for(tiles = 1; tiles < 15; ++tiles)
		{
			int num_tries = (tiles < 4) ? 8 : 4;
			if (tiles > 10) num_tries = 2;
			for(tries = 1; tries <= num_tries; ++ tries)
			{
		
				printf("FACADE_SPELLING\t%s\t%d\t\t%d\t%d\t%d\t%d\n",
					zoning,v,bh_bottom, bh_top,df1,df2);
		//		printf("%s %f %d\n", zoning, block_heights[fac_height], d1);
				for(s = 0; s < tiles; ++s)
				{
					int fh = block_height_for_zoning(fac_height);
					int ah = arch_heights[fac_height];
					int pick;
					float width;
					if(df2 > 30)	pick = pick_histo(block_chance_30 + 8 * fac_height);
					else			pick = pick_histo(block_chance_60 + 8 * fac_height);
					if(tiles == 1) pick = tries - 1;
					width =  block_widths[pick];
					
					
					printf("FACADE_TILE\t%f\t%f\t%f\t%d\t\t",
							width, fh ? block_heights[fh-1] : 8.0f, block_heights[fh], df2);

					if(height_max[h] != height_max[h+1])
					{
						printf("%s_%d_%dx%dx%d%c.fac\t",
								zoning, ah, (int) width, (int) block_heights[fh], df2, s % 2 ? 'b' : 'a');
						
						printf("%s_%d_%dx%dx%d%c.fac\n",
								zoning, ah, (int) width, (int) block_heights[fh], df2, s % 2 ? 'a' : 'b');
					}
					else
					{
						printf("%s_%dx%d%c.fac\t",
								zoning, (int) width, df2, s % 2 ? 'b' : 'a');
						
						printf("%s_%dx%d%c.fac\n",
								zoning, (int) width, df2, s % 2 ? 'a' : 'b');
					}
				}
			}
		}
	}
}


void output_rule_with_height(const char * zoning, const struct depth_info_t * depth_schedule)
{
	int h , r, v, d;
	for(h = 0; height_max[h]; ++h)
	for(r = 0; road_suffix[r]; ++r)
	for(d = 0; depth_schedule[d].block_min; ++d)
	for(v = 0; v < 4; ++v)
	{
		output_one_rule(h,r,v,depth_schedule+d, zoning);
	}
}

void output_rule_basic(const char * zoning, const struct depth_info_t * depth_schedule)
{
	int h , r, v, d;
	for(h = 0; height_max[h]; ++h) { }
	
	for(r = 0; road_suffix[r]; ++r)
	for(d = 0; depth_schedule[d].block_min; ++d)
	for(v = 0; v < 4; ++v)
	{
		output_one_rule(h,r,v, depth_schedule+d, zoning);
	}
}

void adjust_table(float table[64],float widths[8])
{
	int i, j;
//	for(i = 0; i < 64; ++i)
//	{
//		if(i % 8 == 0) printf("\n");
//		printf(" %f",table[i]);
//		
//	}
	
	for(i = 0; i < 64; ++i)
	{
		table[i] /= widths[i % 8];
	}
	for(i = 0; i < 8; ++i)
	{
		float s = 0;
		for(j = 0; j < 8; ++j)
			s += table[i*8+j];
		s = 100.0 / s;
		for(j = 0; j < 8; ++j)
			table[i*8+j] *= s;
	}
//	for(i = 0; i < 64; ++i)
//	{
//		if(i % 8 == 0) printf("\n");
//		printf(" %f",table[i]);
//		
//	}
}


int main(int argc, const char * argv[])
{
	adjust_table(block_chance_30, block_widths);
	adjust_table(block_chance_60, block_widths);

	int p = 1;
	int h = 0;
	int i = 0;
	if(strcmp(argv[p],"-l")==0)
	{
		is_lib =1;
		++p;
	}
	if(strcmp(argv[p],"-h")==0)
	{
		h = 1;
		++p;
	}
	if(strcmp(argv[p],"-i")==0)
	{
		i = 1;
		++p;
	}
	
	
	const char * zoning = argv[p];
	if(h) output_rule_with_height(zoning, i ? depth_industrial : depth_residential);
	else  output_rule_basic(zoning, i ? depth_industrial : depth_residential);
	
	return 0;
}