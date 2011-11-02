// gcc make_fill_rules.c -o make_fill_rules

//#				zoning				road	variant	height	sidelen	err		major	minor	angle		agb		agb		fac		fac		fac			agb					fac					ags
//#													min	max	min	max			min	max	min	max	min	max		min		slop	width	depth	extra
// FILL_RULE	ind_high_solid		2		-1		0	0	0	0	0		0	0	0	0	0	0		56		2		15		30		7.5			ind_high.agb		ind_high.fac		NO_VALUE


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int height_max[] = {	  9999,		40,		32,		24,		16,		10, 0, 0		};
int	road_codes[] = { 2, 1, 0 };
const char * road_suffix[] = { "_f", "_h", NULL };

int is_lib = 0;

int depth_residential [] =  { 30, 60, 120, 0 };
int depth_industrial [] =  { 30, 60, 90, 240, 0 };

void output_one_rule(int h, int r, int v, int d1, int d2, const char * zoning)
{
	if(!is_lib)
	{
		printf("FILL_RULE\t%s\t",zoning);																		// Fill rule and zoning
		printf("%d\t%d\t%d\t%d\t0\t0\t0\t\t",road_codes[r],v,height_max[h+1],		height_max[h  ]);			// road code, variant code, height codes, sidelen, block err, 
		printf("%d\t9999\t%d\t%d\t0\t0\t\t",d1,d1,d2);														// range for major, minor, angle
		printf("%d\t2\t15\t30\t7.5\t\t",d1);																	// AGB params
	}
	
	if(is_lib)
	{
		if(height_max[h] == height_max[h+1])
		{
			printf("EXPORT lib/g10/autogen/%s%s_%d_v%d.agb\t\t\tfoo.agb\n",zoning, road_suffix[r], d1, v);
			printf("EXPORT lib/g01/autogen/%s%s_%d_v%d.fac\t\t\tfoo.fac\n",zoning, road_suffix[r], d1, v);	
		} else {
			printf("EXPORT lib/g10/autogen/%s%s_%dx%d_v%d.agbt\t\tfoo.agb\n",zoning, road_suffix[r], d1, height_max[h], v);
			printf("EXPORT lib/g10/autogen/%s%s_%dx%d_v%d.fact\t\tfoo.fac\n",zoning, road_suffix[r], d1, height_max[h], v);
		}
	}
	else {
		if(height_max[h] == height_max[h+1])
		{
			printf("%s%s_%d_v%d.agb\t",zoning, road_suffix[r], d1, v);
			printf("%s%s_%d_v%d.fac\t",zoning, road_suffix[r], d1, v);	
			printf("NO_VALUE\n");				
		} else {
			printf("%s%s_%dx%d_v%d.agb\t",zoning, road_suffix[r], d1, height_max[h], v);
			printf("%s%s_%dx%d_v%d.fac\t",zoning, road_suffix[r], d1, height_max[h], v);
			printf("NO_VALUE\n");
		}
	}
}

void output_rule_with_height(const char * zoning, const int * depth_schedule)
{
	int h , r, v, d;
	for(h = 0; height_max[h]; ++h)
	for(r = 0; road_suffix[r]; ++r)
	for(d = 0; depth_schedule[d+1]; ++d)
	for(v = 0; v < 4; ++v)
	{
		output_one_rule(h,r,v,depth_schedule[d], depth_schedule[d+1], zoning);
	}
}

void output_rule_basic(const char * zoning, const int * depth_schedule)
{
	int h , r, v, d;
	for(h = 0; height_max[h]; ++h) { }
	
	for(r = 0; road_suffix[r]; ++r)
	for(d = 0; depth_schedule[d+1]; ++d)
	for(v = 0; v < 4; ++v)
	{
		output_one_rule(h,r,v, depth_schedule[d], depth_schedule[d+1], zoning);
	}
}


int main(int argc, const char * argv[])
{
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