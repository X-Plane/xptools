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
#include "perlin.h"
#include <math.h>

float gRandomRats	[RAN_RAT_DIM];	// seed random numbers

struct	init_random_t {
	init_random_t() { 
		for (int n = 0; n < RAN_RAT_DIM; ++n)
		{
			gRandomRats[n] = (float) rand() / (float) RAND_MAX;
		}
	}
};

static init_random_t	init_random;

inline float interp_rat	(const float a,const float b,const float rat )
{
	return a*(1.0-rat)+b*rat;
}

inline float interp_noise_1d(const float x,const int seed)
{
	int integer_X	=(int)x;
	float fractional_X=x-integer_X;
	float v1=gRandomRats[(integer_X  +seed)%RAN_RAT_DIM];
	float v2=gRandomRats[(integer_X+1+seed)%RAN_RAT_DIM];
	return  interp_rat(v1,v2,fractional_X);
}

inline float interp_noise_2d(const float x,const float y,const int seed)
{
	int integer_X	=(int)x;
	int integer_Y	=(int)y;
	float fractional_X=x-integer_X;
	float fractional_Y=y-integer_Y;
	float v1=gRandomRats[(integer_X  +(integer_Y  )*8932+seed)%RAN_RAT_DIM];
	float v2=gRandomRats[(integer_X+1+(integer_Y  )*8932+seed)%RAN_RAT_DIM];
	float v3=gRandomRats[(integer_X  +(integer_Y+1)*8932+seed)%RAN_RAT_DIM];
	float v4=gRandomRats[(integer_X+1+(integer_Y+1)*8932+seed)%RAN_RAT_DIM];
	float i1=interp_rat(v1,v2,fractional_X);
	float i2=interp_rat(v3,v4,fractional_X);
	return  interp_rat(i1,i2,fractional_Y);	// scale from 0.0->1.0 to -1.0->1.0
}

float perlin_1d(float x, int min_level, int max_level, float persistence, int seed)
{
	float result = 0.0;
	for (int level = min_level; level <= max_level; ++level)
		result += pow(persistence,level) * interp_noise_1d(x * (1 << level), seed);
	return result;
}

float perlin_2d(float x, float y, int min_level, int max_level, float persistence, int seed)
{
	float result = 0.0;
	for (int level = min_level; level <= max_level; ++level)
		result += pow(persistence,level) * interp_noise_2d(x * (1 << level), y * (1 << level), seed);
	return result;
}
