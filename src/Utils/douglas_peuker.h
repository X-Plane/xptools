/* 
 * Copyright (c) 2011, Laminar Research.
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

#ifndef douglas_peuker_H
#define douglas_peuker_H

/*

	This is a templated douglas_peuker implementation.  Pass in:
	
		start and stop are iterators to the end points of a poly-line.  Note that unlike most STL iterators,
		stop must point _to_ the last point, not past the end of it.
		
		out is an output iterator that receives only the points that are required to meet the error tolerance.
		start is always pushed, and stop is never pushed.  (This way, you can push multiple segments that share
		an end point.)
		
		epsi is the error at which point we must not exclude a point - that is, the end curve is within epsi
		of the original.
		
		lock-check is a functor that takes points and returns true if they should never be removed.  (This is 
		not needed to ensure that the end points are kept.)
		
		measure is a functor that returns the amount of error if x is removed and approximated with the line
		through a and b.
	
	If start and stop point to the same spatial location (e.g. a loop) then a single lock point will be found
	(or picked) if none exists to ensure that two half-loops are processed.  This ensures that loops won't
	collapse down to only one point.  (The loop may become two points, forming a line out and line back.)

*/

template <typename __InputIterator, typename __OutputIterator, typename __LockFunctor, typename __MeasureFunctor>
void douglas_peuker(
			__InputIterator			start,
			__InputIterator			stop,
			__OutputIterator		out,
			double					epsi,
			const __LockFunctor&	lock_check,
			const __MeasureFunctor& measure)
{
	if(start == stop)
		return;
	if(*start == *stop)
	{
		int n = stop - start;
		if (n == 1) 
			return;
		
		__InputIterator p(start);
		++p;
		while(p < stop)
		{
			if(lock_check(*p))
			{
				douglas_peuker(start,p,out,epsi,lock_check,measure);
				douglas_peuker(p,stop,out,epsi,lock_check,measure);
				return;
			}
			++p;
		}
			
		__InputIterator mid(start);
		advance(mid,n/2);
		douglas_peuker(start,mid,out,epsi,lock_check,measure);
		douglas_peuker(mid,stop,out,epsi,lock_check,measure);
		return;		
	}
	
	
	__InputIterator p(start);
	++p;
	double max_d=0.0;
	__InputIterator worst(stop);
	while(p < stop)
	{
		if(lock_check(*p))
		{
			douglas_peuker(start,p,out,epsi,lock_check,measure);
			douglas_peuker(p,stop,out,epsi,lock_check,measure);
			return;
		}
		double d = measure(*start, *stop, *p);
		if(d > max_d)
		{
			max_d = d;
			worst = p;
		}
		++p;
	}
	if(max_d >= epsi)
	{
		douglas_peuker(start,worst,out,epsi,lock_check,measure);
		douglas_peuker(worst,stop,out,epsi,lock_check,measure);
	}
	else
	{
		*out++ = *start;
	}
}


#endif /* douglas_peuker_H */
