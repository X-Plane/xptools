// gcc make_fill_rules.c -o make_fill_rules

//#				zoning				road	variant	height	sidelen	err		major	minor	angle		agb		agb		fac		fac		fac			agb					fac					ags
//#													min	max	min	max			min	max	min	max	min	max		min		slop	width	depth	extra
// FILL_RULE	ind_high_solid		2		-1		0	0	0	0	0		0	0	0	0	0	0		56		2		15		30		7.5			ind_high.agb		ind_high.fac		NO_VALUE


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int height_min[] = {		40,		32,		24,		16,		10,		0		};
int height_max[] = {	  9999,		40,		32,		24,		16,		10		};

int	road_codes[] = { 2, 1 };

const char * road_suffix[] = { "", "_thin" };

int main(int argc, const char * argv[])
{
	int v,h,r;
	const char * zoning = argv[1];


	for(h = 0; h < 6; ++h)
	for(r = 0; r < 2; ++r)
	for(v = 0; v < 4; ++v)
	
	printf(
		"FILL_RULE\t%s\t"											// Fill rule and zoning
		"%d\t%d\t%d\t%d\t"											// road code, variant code, height codes
		"0\t0\t0\t0\t0\t\t0\t0\t0\t0\t\t"						// sidelen, block err, range for major, minor, angle
		"56\t2\t15\t30\t7.5\t"
		"%s%s_%d_%d.agb\t%s%s_%d_%d.fac\tNO_VALUE\n",
		zoning,
		road_codes[r],
		v,
		height_min[h],
		height_max[h],
		zoning, road_suffix[r], height_max[h], v,
		zoning, road_suffix[r], height_max[h], v);
	return 0;
}