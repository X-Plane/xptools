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

#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>

//******************************************************************************************************************************************************************************
// GENERAL MATH PROCS
//******************************************************************************************************************************************************************************
inline int		sign		(const int in																){													return (in<0   )?-1   :1   ;}
inline float	sign		(const float in																){													return (in<0.0f)?-1.0f:1.0f;}
inline int		intround	(const float in																){													return (in<0.0f)?(int)(in-0.5f):(int)(in+0.5f);}
inline float	sqr			(const float in																){													return in*in;}
inline double	dob_sqr		(const double in															){													return in*in;}
inline float	signsqr		(const float in																){													return (in>=0.0f)?sqr (in):-sqr ( in);}
inline float	signsqrt	(const float in																){													return (in>=0.0f)?sqrtf(in):-sqrtf(-in);}
inline float	pythag		(const float p,const float q												){													return sqrtf(p*p+q*q	 );}
inline float	pythag		(const float p,const float q,const float r									){													return sqrtf(p*p+q*q+r*r);}
inline float	pythag_sqr  (const float p,const float q												){													return (p*p+q*q);}
inline float	pythag_sqr	(const float p,const float q,const float r									){													return (p*p+q*q+r*r);}
inline float	fltnear		(      float in,const float nearest											){in/=nearest;	in=intround(in);					return in*nearest;}
inline int		intnear		(const int in,const int nearest												){													return (int)fltnear((float)in,(float)nearest);}
inline double	dobmin2		(const double x1,const double x2											){													return (x1<x2)?x1:x2;}
inline double	dobmax2		(const double x1,const double x2											){													return (x1>x2)?x1:x2;}
inline int		intmin2		(const int x1,const int x2													){													return (x1<x2)?x1:x2;}
inline int		intmax2		(const int x1,const int x2													){													return (x1>x2)?x1:x2;}
inline float	fltmin2		(const float x1,const float x2												){													return (x1<x2)?x1:x2;}
inline float	fltmax2		(const float x1,const float x2												){													return (x1>x2)?x1:x2;}
inline int		intmin3		(const int x1,const int x2,const int x3										){int x4=(x1<x2)?x1:x2;								return (x3<x4)?x3:x4;}
inline int		intmax3		(const int x1,const int x2,const int x3										){int x5=(x1>x2)?x1:x2;								return (x3>x5)?x3:x5;}
inline float	fltmin3		(const float x1,const float x2,const float x3								){float x4=(x1<x2)?x1:x2;							return (x3<x4)?x3:x4;}
inline float	fltmax3		(const float x1,const float x2,const float x3								){float x5=(x1>x2)?x1:x2;							return (x3>x5)?x3:x5;}
inline float	fltmin4		(const float x1,const float x2,const float x3,const float x4				){float x5=(x1<x2)?x1:x2;	float x6=(x3<x4)?x3:x4;	return (x5<x6)?x5:x6;}
inline float	fltmax4		(const float x1,const float x2,const float x3,const float x4				){float x5=(x1>x2)?x1:x2;	float x6=(x3>x4)?x3:x4;	return (x5>x6)?x5:x6;}
inline int		intmin4		(const int x1,const int x2,const int x3,const int x4						){int x5=(x1<x2)?x1:x2;	int x6=(x3<x4)?x3:x4;		return (x5<x6)?x5:x6;}
inline int		intmax4		(const int x1,const int x2,const int x3,const int x4						){int x5=(x1>x2)?x1:x2;	int x6=(x3>x4)?x3:x4;		return (x5>x6)?x5:x6;}
inline float	fltmin5		(const float x1,const float x2,const float x3,const float x4,const float x5	){float x6=fltmin3(x1,x2,x3);float x7=fltmin2(x4,x5);	return (x6<x7)?x6:x7;}
inline float	fltmax5		(const float x1,const float x2,const float x3,const float x4,const float x5	){float x6=fltmax3(x1,x2,x3);float x7=fltmax2(x4,x5);	return (x6>x7)?x6:x7;}
inline float	fltmin6		(const float x1,const float x2,const float x3,const float x4,const float x5,const float x6){return fltmin2(fltmin3(x1,x2,x3),fltmin3(x4,x5,x6));}
inline float	fltmax6		(const float x1,const float x2,const float x3,const float x4,const float x5,const float x6){return fltmax2(fltmax3(x1,x2,x3),fltmax3(x4,x5,x6));}

// the code below takes us from 125.0 to 125.8 fps in a no-scenery case
// the tricks are:
// 1: we do NOT want <=, but instead want <... it's faster to do > than >=
// 2: we do NOT want >=, but instead want >... it's faster to do > than >=
// 3: we want early-exit
inline int intrange(const int x ,const int x1,const int x2){
	if(x<x1)return false;
	if(x>x2)return false;
			return true;}

inline int intbox (const int x ,const int y ,const int x1,const int y1,const int x2,const int y2){
	if(x<x1)return false;
	if(x>x2)return false;
	if(y<y1)return false;
	if(y>y2)return false;
			return true;}

inline int fltrange(const float x ,const float x1,const float x2){
	if(x<x1)return false;
	if(x>x2)return false;
			return true;}

inline int fltbox (const float x ,const float y ,const float x1,const float y1,const float x2,const float y2){
	if(x<x1)return false;
	if(x>x2)return false;
	if(y<y1)return false;
	if(y>y2)return false;
			return true;}

inline float flt_abs	(const float in){return (in>=0.0f)?in:-in;}		// TYPICAL case: fabs() takes 6.3% of the CPU time, according to profiling!
inline int int_abs	(const int in){return (in>=0   )?in:-in;}		// going to this proc instead takes 0.1%! 6.2% speed boost with no performance loss! BAM!

inline int intlim(const int in,const int min,const int max){
	if(in<min)return min;
	if(in>max)return max;
	return in;}

inline float fltlim(const float in,const float min,const float max){
	if(in<min)return min;
	if(in>max)return max;
	return in;}

inline double doblim(const double in,const double min,const double max){
	if(in<min)return min;
	if(in>max)return max;
	return in;}

inline int intwrap(int in,const int min,const int max){	// we DON'T send 0 and 360. That is REDUNDANT AT DUE
	int del=(max-min+1);									// NORTH! That is REALLY a range of 361. We send 0->359
	if(del<=0)return min;
	while(in<min)in+=del;
	while(in>max)in-=del;
	return in;}

inline float fltwrap(float in,const float min,const float max){	// FLOATING POINT MUST NOT TAKE 0->359, cause that
	float del=(max-min);										// leaves a whole degree uncharted!!! Argh!
	if(del<=0.0f)return min;
	while(in<min)in+=del;
	while(in>max)in-=del;
	return in;}

inline double dobwrap(double in,const double min,const double max){	// FLOATING POINT MUST NOT TAKE 0->359, cause that
	double del=(max-min);										// leaves a whole degree uncharted!!! Argh!
	if(del<=0.0f)return min;
	while(in<min)in+=del;
	while(in>max)in-=del;
	return in;}

inline float interp(const float x1,const float y1,const float x2,const float y2,const float x)
{
	if(x1==x2)	return (y1+y2)*0.5f;
				return fltlim(y1+((y2-y1)/(x2-x1))*(x-x1),fltmin2(y1,y2),fltmax2(y1,y2));
}

inline float interp360(const float x1,const float y1,const float x2,const float y2,const float x)
{
	float ratio=interp (x1,0.0 , x2,1.0 , x);	// this lets us do an interp on heading
	float delta=fltwrap(y2-y1, -180.0,180.0);	// without a stupid wrap-around problem going from 350 to 10!
	return fltwrap(y1+delta*ratio,0.0,360.0);
}

inline float interp(	const float x1,const float y1,
					const float x2,const float y2,
					const float x3,const float y3,const float x)
{
	if(x<x2){if(x1==x2)return (y1+y2)*0.5f;	return fltlim(y1+((y2-y1)/(x2-x1))*(x-x1),fltmin2(y1,y2),fltmax2(y1,y2));}
	else	{if(x2==x3)return (y2+y3)*0.5f;	return fltlim(y2+((y3-y2)/(x3-x2))*(x-x2),fltmin2(y2,y3),fltmax2(y2,y3));}
}

inline double double_interp(const double x1,const double y1,const double x2,const double y2,const double x)
{
	if(x1==x2)	return (y1+y2)*0.5;
				return fltlim(y1+((y2-y1)/(x2-x1))*(x-x1),dobmin2(y1,y2),dobmax2(y1,y2));
}

inline float extrap(const float x1,const float y1,const float x2,const float y2,const float x)
{
	if(x1==x2)	return (y1+y2)*0.5f;
				return	y1+((y2-y1)/(x2-x1))*(x-x1);
}

inline double double_extrap(const double x1,const double y1,const double x2,const double y2,const double x)
{
	if(x1==x2)	return (y1+y2)*0.5;
				return	y1+((y2-y1)/(x2-x1))*(x-x1);
}


inline float bezier_interp(float p0, float p1, float p2, float p3, float t)
{
	if(t==0)return p0;
	if(t==1)return p3;

	float ot = 1.0 - t;
	return         ot * ot * ot * p0 +
			3.0f * ot * ot *  t * p1 +
			3.0f * ot *  t *  t * p2 +
			        t *  t *  t * p3;
}

inline float signzero(const float in){
	if(in> 0.00001f)return  1.0f;
	if(in<-0.00001f)return -1.0f;
				    return  0.0f;}

inline float interp2(const float rat_x,
						  float rat_z,const float ya,const float yb,const float yc,const float yd){
	float y1=interp(0.0f,ya , 1.0f,yb , rat_x);
	float y2=interp(0.0f,yc , 1.0f,yd , rat_x);
	return  interp(0.0f,y1 , 1.0f,y2 , rat_z);}

inline float fallout(const float in,const float lo,const float hi){
	if(in<			  lo)return in;
	if(in>			  hi)return in;
	if(in<((lo+hi)*0.5f))return lo;
					     return hi;}

inline float closer_assymp(const float nowval,const float termval,const float speedrat){return nowval*(1.0f-speedrat)+termval*speedrat;}
inline float closer_linear(const float nowval,const float termval,const float maxrate ){return nowval+fltlim(termval-nowval,-maxrate,maxrate);}

inline float closer_360   (const float nowval,const float termval,const float ratnew  )
{
	float delta=fltwrap(termval-nowval		,-180.0,180.0);
	return     fltwrap(nowval +delta*ratnew	,   0.0,360.0);	// this gives a proper 359->1 wrap
}

inline float closer_180   (const float nowval,const float termval,const float ratnew  )
{
	float delta=fltwrap(termval-nowval		,-180.0,180.0);
	return     fltwrap(nowval +delta*ratnew	,-180.0,180.0);	// this gives a proper -180->180 wrap
}

inline float xpow(float input,float power)
{
		 if(input>0.0f)	return  pow( input,power);	// when do we get negative or zero values? airspeed at zero, rpm at negative as the prop kicks back, stuff like that!
	else if(input<0.0f)	return -pow(-input,power);	// windows machines cannot handle negative input! so i handle it this way... this is a physics sim after all.
	else				return  0.0f;				// windows machines cannot handle 0 input! amazing and strange but true!
}

inline int dec_needed(const float val){
	if(flt_abs(val)>9999.9)return 0;
	if(flt_abs(val)> 999.9)return 1;
	if(flt_abs(val)>  99.9)return 2;
	if(flt_abs(val)>   9.9)return 3;
						   return 4;}

#endif